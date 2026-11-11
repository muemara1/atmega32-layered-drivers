/******************************************************************************
 * File        : ADC_interface.h
 * Layer       : MCAL
 * Description : 10-bit analog-to-digital converter, single-ended channels
 *               ADC0..ADC7 on PA0..PA7.
 *****************************************************************************/
#ifndef ADC_INTERFACE_H_
#define ADC_INTERFACE_H_

#include "STD_TYPES.h"

#define ADC_CHANNEL0        ((u8)0U)
#define ADC_CHANNEL1        ((u8)1U)
#define ADC_CHANNEL2        ((u8)2U)
#define ADC_CHANNEL3        ((u8)3U)
#define ADC_CHANNEL4        ((u8)4U)
#define ADC_CHANNEL5        ((u8)5U)
#define ADC_CHANNEL6        ((u8)6U)
#define ADC_CHANNEL7        ((u8)7U)

/* Reference voltage options (ADC_config.h).
 * Plain integer literals: these are compared in #if directives. */
#define ADC_VREF_AREF       0U
#define ADC_VREF_AVCC       1U
#define ADC_VREF_INTERNAL   3U      /* 2.56 V */

/* Prescaler options (ADC_config.h). Keep ADC clock in 50..200 kHz */
#define ADC_PRESCALER_2     ((u8)1U)
#define ADC_PRESCALER_4     ((u8)2U)
#define ADC_PRESCALER_8     ((u8)3U)
#define ADC_PRESCALER_16    ((u8)4U)
#define ADC_PRESCALER_32    ((u8)5U)
#define ADC_PRESCALER_64    ((u8)6U)
#define ADC_PRESCALER_128   ((u8)7U)

#define ADC_MAX_VALUE       ((u16)1023U)

ES_t ADC_enuInit(void);

/* Blocking conversion with timeout */
ES_t ADC_enuReadChannelSync(u8 Copy_u8Channel, u16 *Copy_pu16Result);

/* Non-blocking conversion; the callback receives the result from the ISR.
 * Returns ES_NOK if a conversion is already running. */
ES_t ADC_enuStartConversionAsync(u8 Copy_u8Channel, void (*Copy_pfNotification)(u16));

/* Scales a raw reading to millivolts for the configured reference */
u16  ADC_u16RawToMilliVolt(u16 Copy_u16Raw);

#endif /* ADC_INTERFACE_H_ */
