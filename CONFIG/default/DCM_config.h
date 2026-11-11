/******************************************************************************
 * File        : DCM_config.h  (default)
 * Layer       : HAL configuration
 * Description : EN is fixed to OC0 (PB3). Only the direction pins are free.
 *****************************************************************************/
#ifndef DCM_CONFIG_H_
#define DCM_CONFIG_H_

#define DCM_IN1_PORT                DIO_PORTD
#define DCM_IN1_PIN                 DIO_PIN4
#define DCM_IN2_PORT                DIO_PORTD
#define DCM_IN2_PIN                 DIO_PIN5

#define DCM_REVERSE_DEAD_TIME_MS    100

#endif /* DCM_CONFIG_H_ */
