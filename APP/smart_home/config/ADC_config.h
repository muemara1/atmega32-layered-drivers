/******************************************************************************
 * File        : ADC_config.h  (smart_home)
 * Description : Internal 2.56 V reference gives 2.5 mV per step, i.e.
 *               0.25 degC resolution for the LM35 (vs 0.49 degC with AVCC).
 *               Put a 100 nF capacitor from AREF to GND.
 *****************************************************************************/
#ifndef ADC_CONFIG_H_
#define ADC_CONFIG_H_

#define ADC_VREF        ADC_VREF_INTERNAL
#define ADC_PRESCALER   ADC_PRESCALER_64

#define ADC_AVCC_MV     5000UL
#define ADC_AREF_MV     5000UL

#endif /* ADC_CONFIG_H_ */
