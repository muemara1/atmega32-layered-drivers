/******************************************************************************
 * File        : LM35_program.c
 * Layer       : HAL
 * Description : LM35 temperature sensor - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"

#include "ADC_interface.h"

#include "LM35_interface.h"
#include "LM35_config.h"

ES_t LM35_enuReadTenths(u16 *Copy_pu16TempTenths)
{
    ES_t Local_enuErrorState = ES_OK;
    u32  Local_u32Sum        = 0UL;
    u16  Local_u16Raw        = 0U;
    u8   Local_u8Sample;

    if (Copy_pu16TempTenths == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        for (Local_u8Sample = 0U; (Local_u8Sample < LM35_SAMPLES) && (Local_enuErrorState == ES_OK); Local_u8Sample++)
        {
            Local_enuErrorState = ADC_enuReadChannelSync(LM35_ADC_CHANNEL, &Local_u16Raw);
            Local_u32Sum += (u32)Local_u16Raw;
        }

        if (Local_enuErrorState == ES_OK)
        {
            u16 Local_u16AvgRaw = (u16)(Local_u32Sum / (u32)LM35_SAMPLES);

            /* 10 mV per degC, so millivolts == tenths of a degree */
            *Copy_pu16TempTenths = ADC_u16RawToMilliVolt(Local_u16AvgRaw);
        }
    }

    return Local_enuErrorState;
}

ES_t LM35_enuReadCelsius(u8 *Copy_pu8TempCelsius)
{
    ES_t Local_enuErrorState = ES_OK;
    u16  Local_u16Tenths     = 0U;

    if (Copy_pu8TempCelsius == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        Local_enuErrorState = LM35_enuReadTenths(&Local_u16Tenths);
        if (Local_enuErrorState == ES_OK)
        {
            u16 Local_u16Celsius = (u16)((Local_u16Tenths + 5U) / 10U);
            *Copy_pu8TempCelsius = (Local_u16Celsius > 255U) ? 255U : (u8)Local_u16Celsius;
        }
    }

    return Local_enuErrorState;
}
