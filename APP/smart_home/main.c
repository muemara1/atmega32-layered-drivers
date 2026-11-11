/******************************************************************************
 * Application : Smart home with fire alarm
 * Description : Temperature sensing, cooling control and fire protection.
 *
 *   NORMAL : LM35 is read every 200 ms. The fan speed follows temperature:
 *            off below 25 C (turns off again only below 24 C, hysteresis),
 *            30 % at 25 C rising linearly to 100 % at 35 C.
 *   FIRE   : entered when the flame sensor fires (INT0, active low) or the
 *            temperature reaches 55 C. Fan OFF (do not feed the fire),
 *            sprinkler pump relay ON, buzzer ON, red LED blinks, LCD alarm.
 *            The alarm is latched: it clears only when the reset button
 *            (INT1) is pressed AND temperature < 45 C AND no flame.
 *
 * Drivers     : DIO, ADC, EXTI, GIE, TIMER0 (PWM), TIMER2 (tick), LCD, DCM,
 *               LM35, SCHED
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#include "DIO_interface.h"
#include "ADC_interface.h"
#include "EXTI_interface.h"
#include "GIE_interface.h"

#include "LCD_interface.h"
#include "DCM_interface.h"
#include "LM35_interface.h"

#include "SCHED_interface.h"

/*------------------------------ Hardware ----------------------------------*/
#define APP_FLAME_PORT          DIO_PORTD
#define APP_FLAME_PIN           DIO_PIN2        /* INT0, LOW = flame        */
#define APP_BUZZER_PORT         DIO_PORTC
#define APP_BUZZER_PIN          DIO_PIN2
#define APP_ALARM_LED_PORT      DIO_PORTC
#define APP_ALARM_LED_PIN       DIO_PIN3
#define APP_PUMP_PORT           DIO_PORTC
#define APP_PUMP_PIN            DIO_PIN4        /* sprinkler relay          */
#define APP_OK_LED_PORT         DIO_PORTC
#define APP_OK_LED_PIN          DIO_PIN5

/*------------------------------ Thresholds (tenths of a degree) -----------*/
#define APP_FAN_ON_TENTHS       ((u16)250U)     /* 25.0 C */
#define APP_FAN_OFF_TENTHS      ((u16)240U)     /* 24.0 C */
#define APP_FAN_FULL_TENTHS     ((u16)350U)     /* 35.0 C */
#define APP_FAN_MIN_PERCENT     ((u8)30U)       /* below this the fan stalls */
#define APP_FIRE_TENTHS         ((u16)550U)     /* 55.0 C */
#define APP_FIRE_CLEAR_TENTHS   ((u16)450U)     /* 45.0 C */

#define APP_BUTTON_LOCKOUT_MS   ((u32)250UL)
#define APP_DEGREE_GLYPH        ((char)0)       /* CGRAM slot 0 */

typedef enum
{
    APP_MODE_NORMAL = 0,
    APP_MODE_FIRE
} APP_Mode_t;

static volatile u8  APP_u8FlameEvent   = STD_FALSE;
static volatile u8  APP_u8ResetRequest = STD_FALSE;
static volatile u32 APP_u32LastResetMs = 0UL;

static APP_Mode_t APP_enuMode         = APP_MODE_NORMAL;
static u16        APP_u16TempTenths   = 0U;
static u8         APP_u8SensorOk      = STD_FALSE;
static u8         APP_u8FanPercent    = 0U;

static const u8 APP_au8DegreeGlyph[8] = { 0x06U, 0x09U, 0x09U, 0x06U, 0x00U, 0x00U, 0x00U, 0x00U };

/*------------------------------ ISR callbacks -----------------------------*/
static void APP_voidOnFlame(void)
{
    APP_u8FlameEvent = STD_TRUE;    /* no debounce: a fire is never ignored */
}

static void APP_voidOnResetButton(void)
{
    u32 Local_u32Now = SCHED_u32GetTicks();

    if ((Local_u32Now - APP_u32LastResetMs) >= APP_BUTTON_LOCKOUT_MS)
    {
        APP_u32LastResetMs = Local_u32Now;
        APP_u8ResetRequest = STD_TRUE;
    }
}

/*------------------------------ Helpers -----------------------------------*/
static u8 APP_u8FlamePresent(void)
{
    u8 Local_u8Level = DIO_HIGH;

    (void)DIO_enuGetPinValue(APP_FLAME_PORT, APP_FLAME_PIN, &Local_u8Level);

    return (Local_u8Level == DIO_LOW) ? STD_TRUE : STD_FALSE;
}

/* Fan duty for a temperature, including hysteresis around the on point */
static u8 APP_u8FanPercentFor(u16 Copy_u16Tenths, u8 Copy_u8CurrentPercent)
{
    u8 Local_u8Percent;

    if (Copy_u16Tenths >= APP_FAN_FULL_TENTHS)
    {
        Local_u8Percent = 100U;
    }
    else if (Copy_u16Tenths >= APP_FAN_ON_TENTHS)
    {
        /* Linear 30 % .. 100 % between 25.0 C and 35.0 C */
        Local_u8Percent = (u8)(APP_FAN_MIN_PERCENT +
            (u8)(((u32)(Copy_u16Tenths - APP_FAN_ON_TENTHS) * (u32)(100U - APP_FAN_MIN_PERCENT)) /
                 (u32)(APP_FAN_FULL_TENTHS - APP_FAN_ON_TENTHS)));
    }
    else if ((Copy_u16Tenths >= APP_FAN_OFF_TENTHS) && (Copy_u8CurrentPercent > 0U))
    {
        Local_u8Percent = APP_FAN_MIN_PERCENT;   /* inside hysteresis band */
    }
    else
    {
        Local_u8Percent = 0U;
    }

    return Local_u8Percent;
}

static void APP_voidEnterFire(const char *Copy_pcCause)
{
    APP_enuMode      = APP_MODE_FIRE;
    APP_u8FanPercent = 0U;
    (void)DCM_enuStop();

    (void)DIO_enuSetPinValue(APP_PUMP_PORT,      APP_PUMP_PIN,      DIO_HIGH);
    (void)DIO_enuSetPinValue(APP_BUZZER_PORT,    APP_BUZZER_PIN,    DIO_HIGH);
    (void)DIO_enuSetPinValue(APP_OK_LED_PORT,    APP_OK_LED_PIN,    DIO_LOW);
    (void)DIO_enuSetPinValue(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, DIO_HIGH);

    (void)LCD_enuWriteLine(LCD_ROW0, "!! FIRE ALARM !!");
    (void)LCD_enuWriteLine(LCD_ROW1, Copy_pcCause);
}

static void APP_voidEnterNormal(void)
{
    APP_enuMode = APP_MODE_NORMAL;

    (void)DIO_enuSetPinValue(APP_PUMP_PORT,      APP_PUMP_PIN,      DIO_LOW);
    (void)DIO_enuSetPinValue(APP_BUZZER_PORT,    APP_BUZZER_PIN,    DIO_LOW);
    (void)DIO_enuSetPinValue(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN, DIO_LOW);
    (void)DIO_enuSetPinValue(APP_OK_LED_PORT,    APP_OK_LED_PIN,    DIO_HIGH);

    (void)LCD_enuClear();
}

/*------------------------------ Tasks -------------------------------------*/
static void APP_voidTaskSense(void)             /* every 200 ms */
{
    u16 Local_u16Tenths = 0U;

    if (LM35_enuReadTenths(&Local_u16Tenths) == ES_OK)
    {
        APP_u16TempTenths = Local_u16Tenths;
        APP_u8SensorOk    = STD_TRUE;
    }
    else
    {
        APP_u8SensorOk = STD_FALSE;
    }

    if (APP_enuMode == APP_MODE_NORMAL)
    {
        if (APP_u8FlameEvent == STD_TRUE)
        {
            APP_voidEnterFire("Flame detected");
        }
        else if ((APP_u8SensorOk == STD_TRUE) && (APP_u16TempTenths >= APP_FIRE_TENTHS))
        {
            APP_voidEnterFire("Over-temperature");
        }
        else
        {
            u8 Local_u8NewPercent = (APP_u8SensorOk == STD_TRUE)
                                    ? APP_u8FanPercentFor(APP_u16TempTenths, APP_u8FanPercent)
                                    : 100U;     /* sensor fault: fail safe, cool */
            if (Local_u8NewPercent != APP_u8FanPercent)
            {
                APP_u8FanPercent = Local_u8NewPercent;
                (void)DCM_enuSetSpeed(APP_u8FanPercent);
            }
        }
    }
}

static void APP_voidTaskAlarm(void)             /* every 250 ms */
{
    if (APP_enuMode == APP_MODE_FIRE)
    {
        (void)DIO_enuTogglePinValue(APP_ALARM_LED_PORT, APP_ALARM_LED_PIN);

        if (APP_u8ResetRequest == STD_TRUE)
        {
            APP_u8ResetRequest = STD_FALSE;

            if ((APP_u8FlamePresent() == STD_FALSE) && (APP_u8SensorOk == STD_TRUE) &&
                (APP_u16TempTenths < APP_FIRE_CLEAR_TENTHS))
            {
                APP_u8FlameEvent = STD_FALSE;
                APP_voidEnterNormal();
            }
            else
            {
                (void)LCD_enuWriteLine(LCD_ROW1, "Still unsafe!");
            }
        }
    }
    else
    {
        APP_u8ResetRequest = STD_FALSE;     /* reset has no meaning here */
    }
}

static void APP_voidTaskDisplay(void)           /* every 500 ms */
{
    char Local_acLine[LCD_COLUMNS + 1U] = "Temp: ";
    char Local_acNumber[6];
    u8   Local_u8Pos = 6U;
    u8   Local_u8Len;
    u8   Local_u8Index;

    if (APP_enuMode == APP_MODE_NORMAL)
    {
        if (APP_u8SensorOk == STD_TRUE)
        {
            /* "Temp: 26.5<deg>C" */
            Local_u8Len = UTILS_u8U32ToString((u32)(APP_u16TempTenths / 10U), Local_acNumber, (u8)sizeof(Local_acNumber));
            for (Local_u8Index = 0U; Local_u8Index < Local_u8Len; Local_u8Index++)
            {
                Local_acLine[Local_u8Pos] = Local_acNumber[Local_u8Index];
                Local_u8Pos++;
            }
            Local_acLine[Local_u8Pos]      = '.';
            Local_acLine[Local_u8Pos + 1U] = (char)('0' + (char)(APP_u16TempTenths % 10U));
            Local_acLine[Local_u8Pos + 2U] = '\0';

            /* The degree glyph is character code 0, which would terminate
             * a C string, so it is written separately after the number. */
            (void)LCD_enuWriteLine(LCD_ROW0, Local_acLine);
            (void)LCD_enuGoTo(LCD_ROW0, (u8)(Local_u8Pos + 2U));
            (void)LCD_enuWriteChar(APP_DEGREE_GLYPH);
            (void)LCD_enuWriteChar('C');
        }
        else
        {
            (void)LCD_enuWriteLine(LCD_ROW0, "Sensor fault!");
        }

        {
            char Local_acFan[LCD_COLUMNS + 1U] = "Fan: ";

            if (APP_u8FanPercent == 0U)
            {
                Local_acFan[5] = 'O';
                Local_acFan[6] = 'F';
                Local_acFan[7] = 'F';
            }
            else
            {
                Local_u8Len = UTILS_u8U32ToString((u32)APP_u8FanPercent, &Local_acFan[5], 4U);
                Local_acFan[5U + Local_u8Len] = '%';
            }
            (void)LCD_enuWriteLine(LCD_ROW1, Local_acFan);
        }
    }
}

/*------------------------------ main --------------------------------------*/
int main(void)
{
    (void)DIO_enuInit();        /* PD2/PD3 pull-up inputs, PC2..PC5 outputs */
    (void)ADC_enuInit();        /* internal 2.56 V reference for the LM35 */
    (void)LCD_enuInit();
    (void)LCD_enuCreateCustomChar(0U, APP_au8DegreeGlyph);
    (void)DCM_enuInit();

    (void)LCD_enuWriteLine(LCD_ROW0, "Smart home");
    (void)LCD_enuWriteLine(LCD_ROW1, "Starting...");

    (void)SCHED_enuInit();
    (void)SCHED_enuAddTask(&APP_voidTaskSense,   200U, 0U);
    (void)SCHED_enuAddTask(&APP_voidTaskAlarm,   250U, 7U);
    (void)SCHED_enuAddTask(&APP_voidTaskDisplay, 500U, 13U);

    (void)EXTI_enuSetCallback(EXTI_INT0, &APP_voidOnFlame);
    (void)EXTI_enuSetCallback(EXTI_INT1, &APP_voidOnResetButton);
    (void)EXTI_enuInit();

    APP_voidEnterNormal();

    /* A flame already present at power-up produces no edge: check the level */
    if (APP_u8FlamePresent() == STD_TRUE)
    {
        APP_u8FlameEvent = STD_TRUE;
    }

    (void)SCHED_enuStart();
    GIE_voidEnable();

    for (;;)
    {
        SCHED_voidDispatch();
    }

    return 0;
}
