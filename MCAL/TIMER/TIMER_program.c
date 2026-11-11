/******************************************************************************
 * File        : TIMER_program.c
 * Layer       : MCAL
 * Description : Timer0 and Timer2 drivers - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "DIO_interface.h"
#include "TIMER_interface.h"
#include "TIMER_private.h"
#include "TIMER_config.h"

static void (*TIMER0_pfOverflowCallback)(void) = NULL;
static void (*TIMER0_pfCompareCallback)(void)  = NULL;
static void (*TIMER2_pfTickCallback)(void)     = NULL;

/*=============================== Timer0 ===================================*/
ES_t TIMER0_enuInit(void)
{
    ES_t Local_enuErrorState = ES_OK;

    TCCR0 = 0U;     /* stop the timer while configuring */
    TCNT0 = 0U;
    OCR0  = 0U;

#if (TIMER0_MODE == TIMER0_MODE_NORMAL)
    SET_BIT(TIMSK, TIMSK_TOIE0);
#elif (TIMER0_MODE == TIMER0_MODE_CTC)
    SET_BIT(TCCR0, TCCR0_WGM01);
    OCR0 = TIMER0_COMPARE_VALUE;
    SET_BIT(TIMSK, TIMSK_OCIE0);
#elif (TIMER0_MODE == TIMER0_MODE_PHASE_PWM) || (TIMER0_MODE == TIMER0_MODE_FAST_PWM)
    SET_BIT(TCCR0, TCCR0_WGM00);
    #if (TIMER0_MODE == TIMER0_MODE_FAST_PWM)
    SET_BIT(TCCR0, TCCR0_WGM01);
    #endif
    /* OC0 stays disconnected until a non-zero duty is requested */
    (void)DIO_enuSetPinValue(TIMER0_OC0_PORT, TIMER0_OC0_PIN, DIO_LOW);
    (void)DIO_enuSetPinDirection(TIMER0_OC0_PORT, TIMER0_OC0_PIN, DIO_OUTPUT);
#else
    #error "TIMER0_MODE has an invalid value"
#endif

    return Local_enuErrorState;
}

ES_t TIMER0_enuStart(void)
{
    TCCR0 = (u8)((TCCR0 & TIMER_CS_MASK) | (u8)TIMER0_PRESCALER);
    return ES_OK;
}

ES_t TIMER0_enuStop(void)
{
    TCCR0 &= TIMER_CS_MASK;
    return ES_OK;
}

ES_t TIMER0_enuSetCompareValue(u8 Copy_u8Value)
{
    OCR0 = Copy_u8Value;
    return ES_OK;
}

ES_t TIMER0_enuSetDutyCycle(u8 Copy_u8DutyPercent)
{
    ES_t Local_enuErrorState = ES_OK;

#if (TIMER0_MODE == TIMER0_MODE_PHASE_PWM) || (TIMER0_MODE == TIMER0_MODE_FAST_PWM)
    if (Copy_u8DutyPercent > TIMER_DUTY_MAX)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else if (Copy_u8DutyPercent == 0U)
    {
        /* Fast PWM still emits a 1-tick pulse with OCR0 = 0, so disconnect
         * OC0 and let the port latch hold the pin low instead. */
        CLR_BIT(TCCR0, TCCR0_COM01);
        CLR_BIT(TCCR0, TCCR0_COM00);
        OCR0 = 0U;
    }
    else
    {
        OCR0  = (u8)(((u16)Copy_u8DutyPercent * TIMER_TOP_8BIT) / (u16)TIMER_DUTY_MAX);
        TCCR0 = (u8)((TCCR0 & (u8)0xCFU) | (u8)(TIMER0_PWM_OUTPUT << TIMER_COM_SHIFT));
    }
#else
    (void)Copy_u8DutyPercent;
    Local_enuErrorState = ES_NOK;   /* not in a PWM mode */
#endif

    return Local_enuErrorState;
}

ES_t TIMER0_enuSetCallback(u8 Copy_u8Event, void (*Copy_pfCallback)(void))
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pfCallback == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u8Event == TIMER0_EVENT_OVERFLOW)
    {
        TIMER0_pfOverflowCallback = Copy_pfCallback;
    }
    else if (Copy_u8Event == TIMER0_EVENT_COMPARE)
    {
        TIMER0_pfCompareCallback = Copy_pfCallback;
    }
    else
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    return Local_enuErrorState;
}

/*=============================== Timer2 ===================================*/
ES_t TIMER2_enuInitTick(void)
{
    TCCR2 = 0U;
    TCNT2 = 0U;
    OCR2  = TIMER2_COMPARE_VALUE;
    SET_BIT(TCCR2, TCCR2_WGM21);    /* CTC, OC2 disconnected */
    SET_BIT(TIMSK, TIMSK_OCIE2);

    return ES_OK;
}

ES_t TIMER2_enuStart(void)
{
    TCCR2 = (u8)((TCCR2 & TIMER_CS_MASK) | (u8)TIMER2_PRESCALER);
    return ES_OK;
}

ES_t TIMER2_enuStop(void)
{
    TCCR2 &= TIMER_CS_MASK;
    return ES_OK;
}

ES_t TIMER2_enuSetCallback(void (*Copy_pfCallback)(void))
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pfCallback == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        TIMER2_pfTickCallback = Copy_pfCallback;
    }

    return Local_enuErrorState;
}

/*=============================== ISRs =====================================*/
ISR_DEFINE(VECT_TIMER0_OVF)
{
    if (TIMER0_pfOverflowCallback != NULL)
    {
        TIMER0_pfOverflowCallback();
    }
}

ISR_DEFINE(VECT_TIMER0_COMP)
{
    if (TIMER0_pfCompareCallback != NULL)
    {
        TIMER0_pfCompareCallback();
    }
}

ISR_DEFINE(VECT_TIMER2_COMP)
{
    if (TIMER2_pfTickCallback != NULL)
    {
        TIMER2_pfTickCallback();
    }
}
