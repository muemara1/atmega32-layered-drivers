/******************************************************************************
 * File        : LM35_interface.h
 * Layer       : HAL
 * Description : LM35 temperature sensor (10 mV / degC) on an ADC channel.
 *               Best resolution with the internal 2.56 V reference (0.25 degC).
 *               Call ADC_enuInit() before using this driver.
 *****************************************************************************/
#ifndef LM35_INTERFACE_H_
#define LM35_INTERFACE_H_

#include "STD_TYPES.h"

/* Temperature in tenths of a degree (e.g. 253 = 25.3 degC), averaged over
 * LM35_SAMPLES conversions to reduce noise */
ES_t LM35_enuReadTenths(u16 *Copy_pu16TempTenths);

/* Whole degrees, rounded */
ES_t LM35_enuReadCelsius(u8 *Copy_pu8TempCelsius);

#endif /* LM35_INTERFACE_H_ */
