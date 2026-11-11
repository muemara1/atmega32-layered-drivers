/******************************************************************************
 * File        : ADC_config.h  (default)
 * Layer       : MCAL configuration
 * Options     : ADC_VREF      : ADC_VREF_AREF, ADC_VREF_AVCC, ADC_VREF_INTERNAL
 *               ADC_PRESCALER : ADC_PRESCALER_2 .. ADC_PRESCALER_128
 *****************************************************************************/
#ifndef ADC_CONFIG_H_
#define ADC_CONFIG_H_

#define ADC_VREF        ADC_VREF_AVCC

/* 8 MHz / 64 = 125 kHz ADC clock */
#define ADC_PRESCALER   ADC_PRESCALER_64

/* Supply / external reference in millivolts, used for scaling only */
#define ADC_AVCC_MV     5000UL
#define ADC_AREF_MV     5000UL

#endif /* ADC_CONFIG_H_ */
