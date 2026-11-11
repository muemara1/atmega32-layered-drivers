/******************************************************************************
 * File        : DCM_program.c
 * Layer       : HAL
 * Description : DC motor driver - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "DELAY.h"

#include "DIO_interface.h"
#include "TIMER_interface.h"

#include "DCM_interface.h"
#include "DCM_config.h"

static u8 DCM_u8Direction = DCM_DIR_CW;
static u8 DCM_u8Speed     = 0U;

static void DCM_voidApplyDirection(void)
{
    if (DCM_u8Direction == DCM_DIR_CW)
    {
        (void)DIO_enuSetPinValue(DCM_IN2_PORT, DCM_IN2_PIN, DIO_LOW);
        (void)DIO_enuSetPinValue(DCM_IN1_PORT, DCM_IN1_PIN, DIO_HIGH);
    }
    else
    {
        (void)DIO_enuSetPinValue(DCM_IN1_PORT, DCM_IN1_PIN, DIO_LOW);
        (void)DIO_enuSetPinValue(DCM_IN2_PORT, DCM_IN2_PIN, DIO_HIGH);
    }
}

ES_t DCM_enuInit(void)
{
    ES_t Local_enuErrorState;

    (void)DIO_enuSetPinValue(DCM_IN1_PORT, DCM_IN1_PIN, DIO_LOW);
    (void)DIO_enuSetPinValue(DCM_IN2_PORT, DCM_IN2_PIN, DIO_LOW);
    (void)DIO_enuSetPinDirection(DCM_IN1_PORT, DCM_IN1_PIN, DIO_OUTPUT);
    (void)DIO_enuSetPinDirection(DCM_IN2_PORT, DCM_IN2_PIN, DIO_OUTPUT);

    Local_enuErrorState = TIMER0_enuInit();     /* Fast PWM per TIMER_config.h */
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TIMER0_enuSetDutyCycle(0U);
    }
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TIMER0_enuStart();
    }

    DCM_u8Direction = DCM_DIR_CW;
    DCM_u8Speed     = 0U;

    return Local_enuErrorState;
}

ES_t DCM_enuSetSpeed(u8 Copy_u8SpeedPercent)
{
    ES_t Local_enuErrorState = TIMER0_enuSetDutyCycle(Copy_u8SpeedPercent);

    if (Local_enuErrorState == ES_OK)
    {
        DCM_u8Speed = Copy_u8SpeedPercent;
        if (Copy_u8SpeedPercent > 0U)
        {
            DCM_voidApplyDirection();
        }
    }

    return Local_enuErrorState;
}

ES_t DCM_enuSetDirection(u8 Copy_u8Direction)
{
    ES_t Local_enuErrorState = ES_OK;

    if ((Copy_u8Direction != DCM_DIR_CW) && (Copy_u8Direction != DCM_DIR_CCW))
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else if (Copy_u8Direction != DCM_u8Direction)
    {
        /* Brief coast before reversing protects the H-bridge and gearbox */
        (void)DIO_enuSetPinValue(DCM_IN1_PORT, DCM_IN1_PIN, DIO_LOW);
        (void)DIO_enuSetPinValue(DCM_IN2_PORT, DCM_IN2_PIN, DIO_LOW);
        DELAY_MS(DCM_REVERSE_DEAD_TIME_MS);

        DCM_u8Direction = Copy_u8Direction;
        if (DCM_u8Speed > 0U)
        {
            DCM_voidApplyDirection();
        }
    }
    else
    {
        /* Already in the requested direction */
    }

    return Local_enuErrorState;
}

ES_t DCM_enuToggleDirection(void)
{
    u8 Local_u8NewDirection = (DCM_u8Direction == DCM_DIR_CW) ? DCM_DIR_CCW : DCM_DIR_CW;
    return DCM_enuSetDirection(Local_u8NewDirection);
}

ES_t DCM_enuGetDirection(u8 *Copy_pu8Direction)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pu8Direction == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        *Copy_pu8Direction = DCM_u8Direction;
    }

    return Local_enuErrorState;
}

ES_t DCM_enuStop(void)
{
    (void)DIO_enuSetPinValue(DCM_IN1_PORT, DCM_IN1_PIN, DIO_LOW);
    (void)DIO_enuSetPinValue(DCM_IN2_PORT, DCM_IN2_PIN, DIO_LOW);
    DCM_u8Speed = 0U;

    return TIMER0_enuSetDutyCycle(0U);
}
