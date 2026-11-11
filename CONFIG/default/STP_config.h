/******************************************************************************
 * File        : STP_config.h  (default)
 * Layer       : HAL configuration
 *****************************************************************************/
#ifndef STP_CONFIG_H_
#define STP_CONFIG_H_

#define STP_A_PORT          DIO_PORTC
#define STP_A_PIN           DIO_PIN2
#define STP_B_PORT          DIO_PORTC
#define STP_B_PIN           DIO_PIN3
#define STP_C_PORT          DIO_PORTC
#define STP_C_PIN           DIO_PIN4
#define STP_D_PORT          DIO_PORTC
#define STP_D_PIN           DIO_PIN5

/* 28BYJ-48 handles about 1 half-step per 2 ms reliably */
#define STP_STEP_DELAY_MS   2

#endif /* STP_CONFIG_H_ */
