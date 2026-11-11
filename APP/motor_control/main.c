/******************************************************************************
 * Application : Motor control
 * Description : - Potentiometer (ADC0) sets the DC motor speed 0..100 % (PWM)
 *               - Button on INT0 reverses the DC motor
 *               - Button on INT1 reverses the stepper motor
 *               - Stepper runs continuously; LCD shows speed and directions
 * Drivers     : DIO, ADC, TIMER0 (PWM), TIMER2 (tick), EXTI, GIE, LCD, DCM,
 *               STP, SCHED
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#include "DIO_interface.h"
#include "ADC_interface.h"
#include "EXTI_interface.h"
#include "GIE_interface.h"

#include "LCD_interface.h"
#include "DCM_interface.h"
#include "STP_interface.h"

#include "SCHED_interface.h"

#define APP_POT_CHANNEL         ADC_CHANNEL0
#define APP_BUTTON_LOCKOUT_MS   ((u32)250UL)    /* ignores contact bounce */
#define APP_SPEED_DEADBAND      ((u8)3U)        /* below this, stop the motor */

static volatile u8  APP_u8DcmReverseRequest = STD_FALSE;
static volatile u8  APP_u8StpReverseRequest = STD_FALSE;
static volatile u32 APP_u32LastInt0Ms       = 0UL;
static volatile u32 APP_u32LastInt1Ms       = 0UL;

static u8 APP_u8SpeedPercent = 0U;
static u8 APP_u8StpDirection = STP_DIR_CW;

/*------------------------------ ISR callbacks -----------------------------*/
static void APP_voidOnInt0(void)
{
    u32 Local_u32Now = SCHED_u32GetTicks();

    if ((Local_u32Now - APP_u32LastInt0Ms) >= APP_BUTTON_LOCKOUT_MS)
    {
        APP_u32LastInt0Ms       = Local_u32Now;
        APP_u8DcmReverseRequest = STD_TRUE;     /* handled in a task: the
                                                   reverse needs a dead time */
    }
}

static void APP_voidOnInt1(void)
{
    u32 Local_u32Now = SCHED_u32GetTicks();

    if ((Local_u32Now - APP_u32LastInt1Ms) >= APP_BUTTON_LOCKOUT_MS)
    {
        APP_u32LastInt1Ms       = Local_u32Now;
        APP_u8StpReverseRequest = STD_TRUE;
    }
}

/*------------------------------ Tasks -------------------------------------*/
static void APP_voidTaskSpeed(void)          /* every 50 ms */
{
    u16 Local_u16Raw = 0U;

    if (ADC_enuReadChannelSync(APP_POT_CHANNEL, &Local_u16Raw) == ES_OK)
    {
        u8 Local_u8Percent = (u8)(((u32)Local_u16Raw * 100UL) / (u32)ADC_MAX_VALUE);

        if (Local_u8Percent < APP_SPEED_DEADBAND)
        {
            Local_u8Percent = 0U;
        }

        if (Local_u8Percent != APP_u8SpeedPercent)
        {
            APP_u8SpeedPercent = Local_u8Percent;
            (void)DCM_enuSetSpeed(APP_u8SpeedPercent);
        }
    }
}

static void APP_voidTaskButtons(void)        /* every 20 ms */
{
    if (APP_u8DcmReverseRequest == STD_TRUE)
    {
        APP_u8DcmReverseRequest = STD_FALSE;
        (void)DCM_enuToggleDirection();
    }

    if (APP_u8StpReverseRequest == STD_TRUE)
    {
        APP_u8StpReverseRequest = STD_FALSE;
        APP_u8StpDirection = (APP_u8StpDirection == STP_DIR_CW) ? STP_DIR_CCW : STP_DIR_CW;
    }
}

static void APP_voidTaskStepper(void)        /* every 3 ms = one half-step */
{
    (void)STP_enuStep(APP_u8StpDirection);
}

static void APP_voidTaskDisplay(void)        /* every 250 ms */
{
    /* "DC  100%  CW   " / "Stepper: CCW    " */
    char Local_acLine0[LCD_COLUMNS + 1U] = "DC     %       ";
    char Local_acNumber[4];
    u8   Local_u8Length;
    u8   Local_u8Index;
    u8   Local_u8DcmDirection = DCM_DIR_CW;

    Local_u8Length = UTILS_u8U32ToString((u32)APP_u8SpeedPercent, Local_acNumber, (u8)sizeof(Local_acNumber));
    for (Local_u8Index = 0U; Local_u8Index < Local_u8Length; Local_u8Index++)
    {
        /* right-align the number in columns 4..6 */
        Local_acLine0[(u8)(7U - Local_u8Length) + Local_u8Index] = Local_acNumber[Local_u8Index];
    }

    (void)DCM_enuGetDirection(&Local_u8DcmDirection);
    Local_acLine0[10] = 'C';
    if (Local_u8DcmDirection == DCM_DIR_CW)
    {
        Local_acLine0[11] = 'W';
    }
    else
    {
        Local_acLine0[11] = 'C';
        Local_acLine0[12] = 'W';
    }

    (void)LCD_enuWriteLine(LCD_ROW0, Local_acLine0);
    (void)LCD_enuWriteLine(LCD_ROW1, (APP_u8StpDirection == STP_DIR_CW) ? "Stepper: CW" : "Stepper: CCW");
}

/*------------------------------ main --------------------------------------*/
int main(void)
{
    (void)DIO_enuInit();        /* buttons: PD2, PD3 inputs with pull-up */
    (void)ADC_enuInit();
    (void)LCD_enuInit();
    (void)DCM_enuInit();
    (void)STP_enuInit();

    (void)SCHED_enuInit();
    (void)SCHED_enuAddTask(&APP_voidTaskStepper, 3U,   0U);
    (void)SCHED_enuAddTask(&APP_voidTaskButtons, 20U,  1U);
    (void)SCHED_enuAddTask(&APP_voidTaskSpeed,   50U,  2U);
    (void)SCHED_enuAddTask(&APP_voidTaskDisplay, 250U, 4U);

    (void)EXTI_enuSetCallback(EXTI_INT0, &APP_voidOnInt0);
    (void)EXTI_enuSetCallback(EXTI_INT1, &APP_voidOnInt1);
    (void)EXTI_enuInit();       /* falling edge, enabled (see config) */

    (void)LCD_enuWriteLine(LCD_ROW0, "Motor control");
    (void)LCD_enuWriteLine(LCD_ROW1, "Starting...");

    (void)SCHED_enuStart();
    GIE_voidEnable();

    for (;;)
    {
        SCHED_voidDispatch();
    }

    return 0;
}
