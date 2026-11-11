/******************************************************************************
 * File        : ADC_program.c
 * Layer       : MCAL
 * Description : ADC driver - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "ADC_interface.h"
#include "ADC_config.h"
#include "ADC_private.h"

static volatile u8 ADC_u8State = ADC_STATE_IDLE;
static void (*ADC_pfNotification)(u16) = NULL;

static u16 ADC_u16ReadDataRegister(void)
{
    /* ADCL must be read first; reading ADCH releases the data register */
    u16 Local_u16Low  = (u16)ADCL;
    u16 Local_u16High = (u16)ADCH;
    return (u16)((Local_u16High << 8) | Local_u16Low);
}

ES_t ADC_enuInit(void)
{
    /* Reference voltage, right-adjusted result, channel 0 */
    ADMUX = (u8)(ADC_VREF << ADC_REFS_SHIFT);

    /* Single conversion mode, interrupt disabled, prescaler from config */
    ADCSRA = ADC_PRESCALER;
    SET_BIT(ADCSRA, ADCSRA_ADEN);

    ADC_u8State = ADC_STATE_IDLE;

    return ES_OK;
}

ES_t ADC_enuReadChannelSync(u8 Copy_u8Channel, u16 *Copy_pu16Result)
{
    ES_t Local_enuErrorState = ES_OK;
    u32  Local_u32Counter    = 0UL;

    if (Copy_pu16Result == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u8Channel >= ADC_CHANNEL_COUNT)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else if (ADC_u8State == ADC_STATE_BUSY)
    {
        Local_enuErrorState = ES_NOK;
    }
    else
    {
        ADMUX = (u8)((ADMUX & ADC_MUX_MASK) | Copy_u8Channel);
        SET_BIT(ADCSRA, ADCSRA_ADSC);

        while ((GET_BIT(ADCSRA, ADCSRA_ADIF) == 0U) && (Local_u32Counter < ADC_TIMEOUT_COUNT))
        {
            Local_u32Counter++;
        }

        if (Local_u32Counter >= ADC_TIMEOUT_COUNT)
        {
            Local_enuErrorState = ES_TIMEOUT;
        }
        else
        {
            SET_BIT(ADCSRA, ADCSRA_ADIF);   /* write 1 to clear the flag */
            *Copy_pu16Result = ADC_u16ReadDataRegister();
        }
    }

    return Local_enuErrorState;
}

ES_t ADC_enuStartConversionAsync(u8 Copy_u8Channel, void (*Copy_pfNotification)(u16))
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pfNotification == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u8Channel >= ADC_CHANNEL_COUNT)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else if (ADC_u8State == ADC_STATE_BUSY)
    {
        Local_enuErrorState = ES_NOK;
    }
    else
    {
        ADC_u8State        = ADC_STATE_BUSY;
        ADC_pfNotification = Copy_pfNotification;

        ADMUX = (u8)((ADMUX & ADC_MUX_MASK) | Copy_u8Channel);
        SET_BIT(ADCSRA, ADCSRA_ADIE);
        SET_BIT(ADCSRA, ADCSRA_ADSC);
    }

    return Local_enuErrorState;
}

u16 ADC_u16RawToMilliVolt(u16 Copy_u16Raw)
{
    return (u16)(((u32)Copy_u16Raw * ADC_VREF_MV) / ((u32)ADC_MAX_VALUE + 1UL));
}

ISR_DEFINE(VECT_ADC)
{
    u16 Local_u16Result = ADC_u16ReadDataRegister();

    CLR_BIT(ADCSRA, ADCSRA_ADIE);
    ADC_u8State = ADC_STATE_IDLE;

    if (ADC_pfNotification != NULL)
    {
        ADC_pfNotification(Local_u16Result);
    }
}
