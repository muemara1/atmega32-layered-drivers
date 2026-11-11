/******************************************************************************
 * File        : ADC_private.h
 * Layer       : MCAL
 *****************************************************************************/
#ifndef ADC_PRIVATE_H_
#define ADC_PRIVATE_H_

#define ADC_CHANNEL_COUNT       ((u8)8U)
#define ADC_MUX_MASK            ((u8)0xE0U)   /* keeps REFS1:0 and ADLAR */
#define ADC_PRESCALER_MASK      ((u8)0xF8U)
#define ADC_REFS_SHIFT          (6U)
#define ADC_TIMEOUT_COUNT       ((u32)50000UL)

#define ADC_STATE_IDLE          ((u8)0U)
#define ADC_STATE_BUSY          ((u8)1U)

/* Reference voltage in millivolts, selected at build time */
#if   ADC_VREF == ADC_VREF_INTERNAL
    #define ADC_VREF_MV         ((u32)2560UL)
#elif ADC_VREF == ADC_VREF_AVCC
    #define ADC_VREF_MV         ((u32)ADC_AVCC_MV)
#else
    #define ADC_VREF_MV         ((u32)ADC_AREF_MV)
#endif

#endif /* ADC_PRIVATE_H_ */
