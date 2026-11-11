/******************************************************************************
 * Application : Car parking system
 * Description : - IR sensor at the entry (INT0) and exit (INT1), active low
 *               - Entry gate opens for 2 s if a slot is free, otherwise the
 *                 buzzer beeps and the "FULL" LED lights
 *               - Seven-segment shows free slots ('F' when full)
 *               - LCD shows free slots and the RTC time
 *               - Occupancy is saved in the 24C16 EEPROM, so a power cut
 *                 does not lose the count
 *               - Every car in/out is logged over UART with a time stamp
 * Drivers     : DIO, EXTI, GIE, TWI, UART, TIMER2, LCD, SSD, EEPROM, RTC,
 *               SCHED, LOG
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#include "DIO_interface.h"
#include "EXTI_interface.h"
#include "GIE_interface.h"
#include "TWI_interface.h"
#include "UART_interface.h"

#include "LCD_interface.h"
#include "SSD_interface.h"
#include "EEPROM_interface.h"
#include "RTC_interface.h"

#include "SCHED_interface.h"
#include "LOG_interface.h"

/*------------------------------ Hardware ----------------------------------*/
#define APP_ENTRY_GATE_PORT     DIO_PORTD
#define APP_ENTRY_GATE_PIN      DIO_PIN4
#define APP_EXIT_GATE_PORT      DIO_PORTD
#define APP_EXIT_GATE_PIN       DIO_PIN5
#define APP_FULL_LED_PORT       DIO_PORTD
#define APP_FULL_LED_PIN        DIO_PIN6
#define APP_BUZZER_PORT         DIO_PORTD
#define APP_BUZZER_PIN          DIO_PIN7

/*------------------------------ Behaviour ---------------------------------*/
#define APP_CAPACITY            ((u8)9U)        /* fits one SSD digit */
#define APP_GATE_OPEN_MS        ((u32)2000UL)
#define APP_BEEP_MS             ((u32)500UL)
#define APP_SENSOR_LOCKOUT_MS   ((u32)1500UL)   /* one car = one event */

#define APP_EE_OCCUPIED_ADDR    ((u16)0x010U)
#define APP_EE_CHECK_ADDR       ((u16)0x011U)   /* bit-inverted copy */

static volatile u8  APP_u8EntryEvent  = STD_FALSE;
static volatile u8  APP_u8ExitEvent   = STD_FALSE;
static volatile u32 APP_u32LastEntryMs = 0UL;
static volatile u32 APP_u32LastExitMs  = 0UL;

static u8  APP_u8Occupied       = 0U;
static u32 APP_u32EntryCloseAt  = 0UL;
static u32 APP_u32ExitCloseAt   = 0UL;
static u32 APP_u32BuzzerOffAt   = 0UL;
static u8  APP_u8EntryOpen      = STD_FALSE;
static u8  APP_u8ExitOpen       = STD_FALSE;
static u8  APP_u8BuzzerOn       = STD_FALSE;

/*------------------------------ ISR callbacks -----------------------------*/
static void APP_voidOnEntrySensor(void)
{
    u32 Local_u32Now = SCHED_u32GetTicks();

    if ((Local_u32Now - APP_u32LastEntryMs) >= APP_SENSOR_LOCKOUT_MS)
    {
        APP_u32LastEntryMs = Local_u32Now;
        APP_u8EntryEvent   = STD_TRUE;
    }
}

static void APP_voidOnExitSensor(void)
{
    u32 Local_u32Now = SCHED_u32GetTicks();

    if ((Local_u32Now - APP_u32LastExitMs) >= APP_SENSOR_LOCKOUT_MS)
    {
        APP_u32LastExitMs = Local_u32Now;
        APP_u8ExitEvent   = STD_TRUE;
    }
}

/*------------------------------ Helpers -----------------------------------*/
static u8 APP_u8IsDue(u32 Copy_u32Deadline)
{
    return ((s32)(SCHED_u32GetTicks() - Copy_u32Deadline) >= 0L) ? STD_TRUE : STD_FALSE;
}

static void APP_voidSaveOccupancy(void)
{
    u8 Local_au8Record[2];

    Local_au8Record[0] = APP_u8Occupied;
    Local_au8Record[1] = (u8)(~APP_u8Occupied);

    if (EEPROM_enuWriteBlock(APP_EE_OCCUPIED_ADDR, Local_au8Record, 2U) != ES_OK)
    {
        (void)LOG_enuWrite("EEPROM write failed");
    }
}

static void APP_voidLoadOccupancy(void)
{
    u8 Local_au8Record[2] = { 0U, 0U };

    if ((EEPROM_enuReadBlock(APP_EE_OCCUPIED_ADDR, Local_au8Record, 2U) == ES_OK) &&
        (Local_au8Record[1] == (u8)(~Local_au8Record[0])) &&
        (Local_au8Record[0] <= APP_CAPACITY))
    {
        APP_u8Occupied = Local_au8Record[0];
        (void)LOG_enuWriteValue("Restored occupancy: ", (s32)APP_u8Occupied);
    }
    else
    {
        /* Blank or corrupted record (or no EEPROM): start empty */
        APP_u8Occupied = 0U;
        (void)LOG_enuWrite("No valid occupancy record, starting empty");
        APP_voidSaveOccupancy();
    }
}

static void APP_voidUpdateDisplays(void)
{
    u8   Local_u8Free = (u8)(APP_CAPACITY - APP_u8Occupied);
    char Local_acLine[LCD_COLUMNS + 1U] = "Free:  /";

    if (Local_u8Free == 0U)
    {
        (void)SSD_enuDisplayPattern(SSD_SYMBOL_F);
        (void)DIO_enuSetPinValue(APP_FULL_LED_PORT, APP_FULL_LED_PIN, DIO_HIGH);
        (void)LCD_enuWriteLine(LCD_ROW0, "Parking FULL");
    }
    else
    {
        (void)SSD_enuDisplayDigit(Local_u8Free);
        (void)DIO_enuSetPinValue(APP_FULL_LED_PORT, APP_FULL_LED_PIN, DIO_LOW);
        Local_acLine[6] = (char)('0' + (char)Local_u8Free);
        Local_acLine[8] = (char)('0' + (char)APP_CAPACITY);
        (void)LCD_enuWriteLine(LCD_ROW0, Local_acLine);
    }
}

static void APP_voidBeep(void)
{
    (void)DIO_enuSetPinValue(APP_BUZZER_PORT, APP_BUZZER_PIN, DIO_HIGH);
    APP_u8BuzzerOn     = STD_TRUE;
    APP_u32BuzzerOffAt = SCHED_u32GetTicks() + APP_BEEP_MS;
}

/*------------------------------ Tasks -------------------------------------*/
static void APP_voidTaskEvents(void)            /* every 20 ms */
{
    if (APP_u8EntryEvent == STD_TRUE)
    {
        APP_u8EntryEvent = STD_FALSE;

        if (APP_u8Occupied < APP_CAPACITY)
        {
            APP_u8Occupied++;
            APP_voidSaveOccupancy();
            (void)DIO_enuSetPinValue(APP_ENTRY_GATE_PORT, APP_ENTRY_GATE_PIN, DIO_HIGH);
            APP_u8EntryOpen     = STD_TRUE;
            APP_u32EntryCloseAt = SCHED_u32GetTicks() + APP_GATE_OPEN_MS;
            (void)LOG_enuWriteValue("Car in, free slots: ", (s32)(APP_CAPACITY - APP_u8Occupied));
        }
        else
        {
            APP_voidBeep();
            (void)LOG_enuWrite("Entry refused, parking full");
        }
        APP_voidUpdateDisplays();
    }

    if (APP_u8ExitEvent == STD_TRUE)
    {
        APP_u8ExitEvent = STD_FALSE;

        if (APP_u8Occupied > 0U)
        {
            APP_u8Occupied--;
            APP_voidSaveOccupancy();
            (void)DIO_enuSetPinValue(APP_EXIT_GATE_PORT, APP_EXIT_GATE_PIN, DIO_HIGH);
            APP_u8ExitOpen     = STD_TRUE;
            APP_u32ExitCloseAt = SCHED_u32GetTicks() + APP_GATE_OPEN_MS;
            (void)LOG_enuWriteValue("Car out, free slots: ", (s32)(APP_CAPACITY - APP_u8Occupied));
        }
        else
        {
            /* Exit sensor fired with an empty lot: count mismatch */
            (void)LOG_enuWrite("Exit with 0 cars counted, check sensor");
        }
        APP_voidUpdateDisplays();
    }
}

static void APP_voidTaskOutputs(void)           /* every 50 ms */
{
    if ((APP_u8EntryOpen == STD_TRUE) && (APP_u8IsDue(APP_u32EntryCloseAt) == STD_TRUE))
    {
        (void)DIO_enuSetPinValue(APP_ENTRY_GATE_PORT, APP_ENTRY_GATE_PIN, DIO_LOW);
        APP_u8EntryOpen = STD_FALSE;
    }

    if ((APP_u8ExitOpen == STD_TRUE) && (APP_u8IsDue(APP_u32ExitCloseAt) == STD_TRUE))
    {
        (void)DIO_enuSetPinValue(APP_EXIT_GATE_PORT, APP_EXIT_GATE_PIN, DIO_LOW);
        APP_u8ExitOpen = STD_FALSE;
    }

    if ((APP_u8BuzzerOn == STD_TRUE) && (APP_u8IsDue(APP_u32BuzzerOffAt) == STD_TRUE))
    {
        (void)DIO_enuSetPinValue(APP_BUZZER_PORT, APP_BUZZER_PIN, DIO_LOW);
        APP_u8BuzzerOn = STD_FALSE;
    }
}

static void APP_voidTaskClock(void)             /* every 500 ms */
{
    RTC_Time_t Local_strTime;
    char       Local_acLine[LCD_COLUMNS + 1U] = "Time --:--:--";

    if (RTC_enuGetTime(&Local_strTime) == ES_OK)
    {
        UTILS_voidTwoDigits(Local_strTime.Hours,   &Local_acLine[5]);
        UTILS_voidTwoDigits(Local_strTime.Minutes, &Local_acLine[8]);
        UTILS_voidTwoDigits(Local_strTime.Seconds, &Local_acLine[11]);
    }
    (void)LCD_enuWriteLine(LCD_ROW1, Local_acLine);
}

static void APP_voidTaskConsole(void)           /* every 100 ms */
{
    LOG_voidProcessConsole();
}

/*------------------------------ main --------------------------------------*/
int main(void)
{
    (void)DIO_enuInit();        /* sensors PD2/PD3 pull-up, PD4..PD7 outputs */
    (void)UART_enuInit();
    (void)TWI_enuInitMaster();
    (void)LCD_enuInit();
    (void)SSD_enuInit();

    if (RTC_enuInit() != ES_OK)
    {
        (void)UART_enuSendString("RTC not responding\r\n");
    }
    (void)LOG_enuInitConsole();
    (void)LOG_enuWrite("Car parking system started");

    APP_voidLoadOccupancy();
    APP_voidUpdateDisplays();

    (void)SCHED_enuInit();
    (void)SCHED_enuAddTask(&APP_voidTaskEvents,  20U,  0U);
    (void)SCHED_enuAddTask(&APP_voidTaskOutputs, 50U,  3U);
    (void)SCHED_enuAddTask(&APP_voidTaskConsole, 100U, 7U);
    (void)SCHED_enuAddTask(&APP_voidTaskClock,   500U, 11U);

    (void)EXTI_enuSetCallback(EXTI_INT0, &APP_voidOnEntrySensor);
    (void)EXTI_enuSetCallback(EXTI_INT1, &APP_voidOnExitSensor);
    (void)EXTI_enuInit();

    (void)SCHED_enuStart();
    GIE_voidEnable();

    for (;;)
    {
        SCHED_voidDispatch();
    }

    return 0;
}
