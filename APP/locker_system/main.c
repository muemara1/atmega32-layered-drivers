/******************************************************************************
 * Application : Locker system
 * Description : PIN-protected locker.
 *               - First boot: the user sets a 4-digit PIN (entered twice)
 *               - PIN, magic byte and failed-attempt counter live in the
 *                 external 24C16 EEPROM, so a power cycle cannot reset the
 *                 attempt counter to bypass the lockout
 *               - Correct PIN: lock relay opens for 5 s ('=' relocks early,
 *                 '*' changes the PIN while open)
 *               - 3 wrong PINs: 30 s lockout with buzzer
 *               - Every event is logged over UART with an RTC time stamp;
 *                 the LCD shows the time while idle
 * Drivers     : DIO, TWI, UART, TIMER2, GIE, LCD, KPD, EEPROM, RTC,
 *               SCHED, LOG
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#include "DIO_interface.h"
#include "TWI_interface.h"
#include "UART_interface.h"
#include "GIE_interface.h"

#include "LCD_interface.h"
#include "KPD_interface.h"
#include "EEPROM_interface.h"
#include "RTC_interface.h"

#include "SCHED_interface.h"
#include "LOG_interface.h"

/*------------------------------ Hardware ----------------------------------*/
#define APP_LOCK_PORT           DIO_PORTA
#define APP_LOCK_PIN            DIO_PIN0        /* relay / solenoid, HIGH = open */
#define APP_BUZZER_PORT         DIO_PORTA
#define APP_BUZZER_PIN          DIO_PIN1
#define APP_GREEN_LED_PORT      DIO_PORTA
#define APP_GREEN_LED_PIN       DIO_PIN2
#define APP_RED_LED_PORT        DIO_PORTA
#define APP_RED_LED_PIN         DIO_PIN3

/*------------------------------ Behaviour ---------------------------------*/
#define APP_PIN_LENGTH          ((u8)4U)
#define APP_MAX_ATTEMPTS        ((u8)3U)
#define APP_UNLOCK_TIME_MS      ((u32)5000UL)
#define APP_LOCKOUT_TIME_MS     ((u32)30000UL)
#define APP_BUZZER_TIME_MS      ((u32)3000UL)
#define APP_MESSAGE_TIME_MS     ((u32)1500UL)

/*------------------------------ EEPROM map --------------------------------*/
#define APP_EE_MAGIC_ADDR       ((u16)0x000U)
#define APP_EE_PIN_ADDR         ((u16)0x001U)   /* 4 bytes */
#define APP_EE_ATTEMPTS_ADDR    ((u16)0x005U)
#define APP_EE_MAGIC_VALUE      ((u8)0xA5U)

typedef enum
{
    APP_STATE_SET_PIN = 0,      /* type a new PIN                  */
    APP_STATE_CONFIRM_PIN,      /* type it again                   */
    APP_STATE_ENTER_PIN,        /* locked, waiting for the PIN     */
    APP_STATE_MESSAGE,          /* short message, then next state  */
    APP_STATE_UNLOCKED,
    APP_STATE_LOCKOUT,
    APP_STATE_ERROR             /* storage failure, stays locked   */
} APP_State_t;

static APP_State_t APP_enuState          = APP_STATE_ENTER_PIN;
static APP_State_t APP_enuStateAfterMsg  = APP_STATE_ENTER_PIN;
static u32         APP_u32StateDeadline  = 0UL;

static u8 APP_au8StoredPin[APP_PIN_LENGTH];
static u8 APP_au8NewPin[APP_PIN_LENGTH];
static u8 APP_au8Entry[APP_PIN_LENGTH];
static u8 APP_u8EntryLength = 0U;
static u8 APP_u8Attempts    = 0U;

/*------------------------------ Helpers -----------------------------------*/
static void APP_voidSetOutputs(u8 Copy_u8LockOpen, u8 Copy_u8Green, u8 Copy_u8Red, u8 Copy_u8Buzzer)
{
    (void)DIO_enuSetPinValue(APP_LOCK_PORT,      APP_LOCK_PIN,      Copy_u8LockOpen);
    (void)DIO_enuSetPinValue(APP_GREEN_LED_PORT, APP_GREEN_LED_PIN, Copy_u8Green);
    (void)DIO_enuSetPinValue(APP_RED_LED_PORT,   APP_RED_LED_PIN,   Copy_u8Red);
    (void)DIO_enuSetPinValue(APP_BUZZER_PORT,    APP_BUZZER_PIN,    Copy_u8Buzzer);
}

static void APP_voidClearEntry(void)
{
    u8 Local_u8Index;

    for (Local_u8Index = 0U; Local_u8Index < APP_PIN_LENGTH; Local_u8Index++)
    {
        APP_au8Entry[Local_u8Index] = 0U;
    }
    APP_u8EntryLength = 0U;
}

static void APP_voidShowStars(void)
{
    char Local_acStars[APP_PIN_LENGTH + 1U];
    u8   Local_u8Index;

    for (Local_u8Index = 0U; Local_u8Index < APP_u8EntryLength; Local_u8Index++)
    {
        Local_acStars[Local_u8Index] = '*';
    }
    Local_acStars[APP_u8EntryLength] = '\0';

    (void)LCD_enuWriteLine(LCD_ROW1, Local_acStars);
}

static void APP_voidShowMessage(const char *Copy_pcLine0, const char *Copy_pcLine1, APP_State_t Copy_enuNext)
{
    (void)LCD_enuWriteLine(LCD_ROW0, Copy_pcLine0);
    (void)LCD_enuWriteLine(LCD_ROW1, Copy_pcLine1);
    APP_enuStateAfterMsg = Copy_enuNext;
    APP_u32StateDeadline = SCHED_u32GetTicks() + APP_MESSAGE_TIME_MS;
    APP_enuState         = APP_STATE_MESSAGE;
}

static void APP_voidSaveAttempts(void)
{
    if (EEPROM_enuWriteByte(APP_EE_ATTEMPTS_ADDR, APP_u8Attempts) != ES_OK)
    {
        (void)LOG_enuWrite("EEPROM write failed (attempts)");
    }
}

/*------------------------------ State entries -----------------------------*/
static void APP_voidEnterSetPin(void)
{
    APP_voidClearEntry();
    APP_voidSetOutputs(DIO_LOW, DIO_LOW, DIO_LOW, DIO_LOW);
    (void)LCD_enuWriteLine(LCD_ROW0, "New PIN:");
    (void)LCD_enuWriteLine(LCD_ROW1, "");
    APP_enuState = APP_STATE_SET_PIN;
}

static void APP_voidEnterLocked(void)
{
    APP_voidClearEntry();
    APP_voidSetOutputs(DIO_LOW, DIO_LOW, DIO_LOW, DIO_LOW);
    (void)LCD_enuWriteLine(LCD_ROW0, "Enter PIN:");
    (void)LCD_enuWriteLine(LCD_ROW1, "");
    APP_enuState = APP_STATE_ENTER_PIN;
}

static void APP_voidEnterUnlocked(void)
{
    APP_voidClearEntry();
    APP_voidSetOutputs(DIO_HIGH, DIO_HIGH, DIO_LOW, DIO_LOW);
    (void)LCD_enuWriteLine(LCD_ROW0, "Unlocked");
    (void)LCD_enuWriteLine(LCD_ROW1, "=lock *new PIN");
    APP_u32StateDeadline = SCHED_u32GetTicks() + APP_UNLOCK_TIME_MS;
    APP_enuState         = APP_STATE_UNLOCKED;
    (void)LOG_enuWrite("Unlocked");
}

static void APP_voidEnterLockout(void)
{
    APP_voidClearEntry();
    APP_voidSetOutputs(DIO_LOW, DIO_LOW, DIO_HIGH, DIO_HIGH);
    (void)LCD_enuWriteLine(LCD_ROW0, "LOCKED OUT");
    APP_u32StateDeadline = SCHED_u32GetTicks() + APP_LOCKOUT_TIME_MS;
    APP_enuState         = APP_STATE_LOCKOUT;
    (void)LOG_enuWrite("ALARM: too many wrong PINs, lockout");
}

static void APP_voidEnterError(const char *Copy_pcReason)
{
    APP_voidSetOutputs(DIO_LOW, DIO_LOW, DIO_HIGH, DIO_LOW);
    (void)LCD_enuWriteLine(LCD_ROW0, "Storage error");
    (void)LCD_enuWriteLine(LCD_ROW1, "Check EEPROM");
    (void)LOG_enuWrite(Copy_pcReason);
    APP_enuState = APP_STATE_ERROR;
}

/*------------------------------ PIN handling ------------------------------*/
static void APP_voidOnPinComplete(void)
{
    u8 Local_u8Index;

    switch (APP_enuState)
    {
        case APP_STATE_SET_PIN:
            for (Local_u8Index = 0U; Local_u8Index < APP_PIN_LENGTH; Local_u8Index++)
            {
                APP_au8NewPin[Local_u8Index] = APP_au8Entry[Local_u8Index];
            }
            APP_voidClearEntry();
            (void)LCD_enuWriteLine(LCD_ROW0, "Confirm PIN:");
            (void)LCD_enuWriteLine(LCD_ROW1, "");
            APP_enuState = APP_STATE_CONFIRM_PIN;
            break;

        case APP_STATE_CONFIRM_PIN:
            if (UTILS_u8BytesEqual(APP_au8NewPin, APP_au8Entry, APP_PIN_LENGTH) == STD_TRUE)
            {
                /* Order matters: PIN and a zeroed attempt counter first, the
                 * magic byte last, so a power cut mid-way leaves no half-valid
                 * record (a blank EEPROM reads 0xFF, which would otherwise
                 * look like a lockout on the next boot). */
                ES_t Local_enuStatus = EEPROM_enuWriteBlock(APP_EE_PIN_ADDR, APP_au8NewPin, APP_PIN_LENGTH);
                if (Local_enuStatus == ES_OK)
                {
                    APP_u8Attempts  = 0U;
                    Local_enuStatus = EEPROM_enuWriteByte(APP_EE_ATTEMPTS_ADDR, APP_u8Attempts);
                }
                if (Local_enuStatus == ES_OK)
                {
                    Local_enuStatus = EEPROM_enuWriteByte(APP_EE_MAGIC_ADDR, APP_EE_MAGIC_VALUE);
                }

                if (Local_enuStatus == ES_OK)
                {
                    for (Local_u8Index = 0U; Local_u8Index < APP_PIN_LENGTH; Local_u8Index++)
                    {
                        APP_au8StoredPin[Local_u8Index] = APP_au8NewPin[Local_u8Index];
                    }
                    (void)LOG_enuWrite("PIN changed");
                    APP_voidShowMessage("PIN saved", "", APP_STATE_ENTER_PIN);
                }
                else
                {
                    APP_voidEnterError("EEPROM write failed (PIN)");
                }
            }
            else
            {
                APP_voidShowMessage("PINs differ", "Try again", APP_STATE_SET_PIN);
            }
            break;

        case APP_STATE_ENTER_PIN:
            if (UTILS_u8BytesEqual(APP_au8StoredPin, APP_au8Entry, APP_PIN_LENGTH) == STD_TRUE)
            {
                if (APP_u8Attempts != 0U)
                {
                    APP_u8Attempts = 0U;
                    APP_voidSaveAttempts();
                }
                APP_voidEnterUnlocked();
            }
            else
            {
                APP_u8Attempts++;
                APP_voidSaveAttempts();
                (void)LOG_enuWriteValue("Wrong PIN, attempt ", (s32)APP_u8Attempts);

                if (APP_u8Attempts >= APP_MAX_ATTEMPTS)
                {
                    APP_voidEnterLockout();
                }
                else
                {
                    (void)DIO_enuSetPinValue(APP_RED_LED_PORT, APP_RED_LED_PIN, DIO_HIGH);
                    APP_voidShowMessage("Wrong PIN", (APP_u8Attempts == 1U) ? "2 tries left" : "1 try left",
                                        APP_STATE_ENTER_PIN);
                }
            }
            APP_voidClearEntry();
            break;

        default:
            /* PIN entry is not accepted in other states */
            break;
    }
}

static void APP_voidHandleKey(u8 Copy_u8Key)
{
    u8 Local_u8IsDigit = ((Copy_u8Key >= (u8)'0') && (Copy_u8Key <= (u8)'9')) ? STD_TRUE : STD_FALSE;

    switch (APP_enuState)
    {
        case APP_STATE_SET_PIN:
        case APP_STATE_CONFIRM_PIN:
        case APP_STATE_ENTER_PIN:
            if (Local_u8IsDigit == STD_TRUE)
            {
                APP_au8Entry[APP_u8EntryLength] = (u8)(Copy_u8Key - (u8)'0');
                APP_u8EntryLength++;
                APP_voidShowStars();
                if (APP_u8EntryLength == APP_PIN_LENGTH)
                {
                    APP_voidOnPinComplete();
                }
            }
            else if (Copy_u8Key == (u8)'C')
            {
                APP_voidClearEntry();
                APP_voidShowStars();
            }
            else
            {
                /* other keys ignored while typing a PIN */
            }
            break;

        case APP_STATE_UNLOCKED:
            if (Copy_u8Key == (u8)'=')
            {
                (void)LOG_enuWrite("Locked by user");
                APP_voidEnterLocked();
            }
            else if (Copy_u8Key == (u8)'*')
            {
                APP_voidEnterSetPin();
            }
            else
            {
                /* ignored */
            }
            break;

        default:
            /* MESSAGE, LOCKOUT, ERROR: keypad disabled */
            break;
    }
}

/*------------------------------ Tasks -------------------------------------*/
static void APP_voidTaskKeypad(void)            /* every 10 ms */
{
    u8 Local_u8Key = KPD_NO_KEY;

    if ((KPD_enuGetPressedKey(&Local_u8Key) == ES_OK) && (Local_u8Key != KPD_NO_KEY))
    {
        APP_voidHandleKey(Local_u8Key);
    }
}

static void APP_voidTaskTimeouts(void)          /* every 100 ms */
{
    u32 Local_u32Now = SCHED_u32GetTicks();

    /* Signed difference handles tick wrap-around */
    u8 Local_u8Expired = ((s32)(Local_u32Now - APP_u32StateDeadline) >= 0L) ? STD_TRUE : STD_FALSE;

    switch (APP_enuState)
    {
        case APP_STATE_MESSAGE:
            if (Local_u8Expired == STD_TRUE)
            {
                if (APP_enuStateAfterMsg == APP_STATE_SET_PIN)
                {
                    APP_voidEnterSetPin();
                }
                else
                {
                    APP_voidEnterLocked();
                }
            }
            break;

        case APP_STATE_UNLOCKED:
            if (Local_u8Expired == STD_TRUE)
            {
                (void)LOG_enuWrite("Auto-locked");
                APP_voidEnterLocked();
            }
            break;

        case APP_STATE_LOCKOUT:
            if (Local_u8Expired == STD_TRUE)
            {
                APP_u8Attempts = 0U;
                APP_voidSaveAttempts();
                (void)LOG_enuWrite("Lockout ended");
                APP_voidEnterLocked();
            }
            else
            {
                u32  Local_u32Left = (APP_u32StateDeadline - Local_u32Now) / 1000UL;
                char Local_acLine[LCD_COLUMNS + 1U] = "Wait ";
                u8   Local_u8Len = UTILS_u8U32ToString(Local_u32Left + 1UL, &Local_acLine[5], 4U);

                Local_acLine[5U + Local_u8Len] = ' ';
                Local_acLine[6U + Local_u8Len] = 's';
                Local_acLine[7U + Local_u8Len] = '\0';
                (void)LCD_enuWriteLine(LCD_ROW1, Local_acLine);

                /* Buzzer only for the first seconds of the lockout */
                if ((APP_LOCKOUT_TIME_MS - (APP_u32StateDeadline - Local_u32Now)) >= APP_BUZZER_TIME_MS)
                {
                    (void)DIO_enuSetPinValue(APP_BUZZER_PORT, APP_BUZZER_PIN, DIO_LOW);
                }
            }
            break;

        default:
            break;
    }
}

static void APP_voidTaskClock(void)             /* every 500 ms */
{
    RTC_Time_t Local_strTime;
    char       Local_acLine[LCD_COLUMNS + 1U] = "    --:--:--";

    /* Show the clock only while idle, so it never hides typed stars */
    if ((APP_enuState == APP_STATE_ENTER_PIN) && (APP_u8EntryLength == 0U))
    {
        if (RTC_enuGetTime(&Local_strTime) == ES_OK)
        {
            UTILS_voidTwoDigits(Local_strTime.Hours,   &Local_acLine[4]);
            UTILS_voidTwoDigits(Local_strTime.Minutes, &Local_acLine[7]);
            UTILS_voidTwoDigits(Local_strTime.Seconds, &Local_acLine[10]);
        }
        (void)LCD_enuWriteLine(LCD_ROW1, Local_acLine);
    }
}

static void APP_voidTaskConsole(void)           /* every 100 ms */
{
    LOG_voidProcessConsole();
}

/*------------------------------ Start-up ----------------------------------*/
static void APP_voidLoadSettings(void)
{
    u8 Local_u8Magic = 0U;

    if (EEPROM_enuReadByte(APP_EE_MAGIC_ADDR, &Local_u8Magic) != ES_OK)
    {
        APP_voidEnterError("EEPROM not responding");
    }
    else if (Local_u8Magic != APP_EE_MAGIC_VALUE)
    {
        (void)LOG_enuWrite("No PIN stored, first-time setup");
        APP_u8Attempts = 0U;
        APP_voidEnterSetPin();
    }
    else if ((EEPROM_enuReadBlock(APP_EE_PIN_ADDR, APP_au8StoredPin, APP_PIN_LENGTH) != ES_OK) ||
             (EEPROM_enuReadByte(APP_EE_ATTEMPTS_ADDR, &APP_u8Attempts) != ES_OK))
    {
        APP_voidEnterError("EEPROM read failed");
    }
    else if (APP_u8Attempts >= APP_MAX_ATTEMPTS)
    {
        /* Power was cycled during a lockout: continue it */
        APP_voidEnterLockout();
    }
    else
    {
        APP_voidEnterLocked();
    }
}

int main(void)
{
    (void)DIO_enuInit();        /* PA0..PA3 outputs, low */
    (void)UART_enuInit();
    (void)TWI_enuInitMaster();
    (void)LCD_enuInit();
    (void)KPD_enuInit();

    (void)LCD_enuWriteLine(LCD_ROW0, "Locker system");
    (void)LCD_enuWriteLine(LCD_ROW1, "Starting...");

    if (RTC_enuInit() != ES_OK)
    {
        (void)UART_enuSendString("RTC not responding\r\n");
    }
    (void)LOG_enuInitConsole();
    (void)LOG_enuWrite("Locker system started");

    (void)SCHED_enuInit();
    (void)SCHED_enuAddTask(&APP_voidTaskKeypad,   10U,  0U);
    (void)SCHED_enuAddTask(&APP_voidTaskTimeouts, 100U, 3U);
    (void)SCHED_enuAddTask(&APP_voidTaskConsole,  100U, 7U);
    (void)SCHED_enuAddTask(&APP_voidTaskClock,    500U, 5U);

    /* Interrupts are needed for SCHED_u32GetTicks() deadlines and the
     * console, so start them before the first state is entered. */
    (void)SCHED_enuStart();
    GIE_voidEnable();

    APP_voidLoadSettings();

    for (;;)
    {
        SCHED_voidDispatch();
    }

    return 0;
}
