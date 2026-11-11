/******************************************************************************
 * File        : SSD_config.h  (default)
 * Layer       : HAL configuration
 * Options     : SSD_TYPE : SSD_COMMON_CATHODE / SSD_COMMON_ANODE
 *****************************************************************************/
#ifndef SSD_CONFIG_H_
#define SSD_CONFIG_H_

#define SSD_TYPE        SSD_COMMON_CATHODE

/* Segments a..g on PA1..PA7 (PA0 stays free for the ADC) */
#define SSD_A_PORT      DIO_PORTA
#define SSD_A_PIN       DIO_PIN1
#define SSD_B_PORT      DIO_PORTA
#define SSD_B_PIN       DIO_PIN2
#define SSD_C_PORT      DIO_PORTA
#define SSD_C_PIN       DIO_PIN3
#define SSD_D_PORT      DIO_PORTA
#define SSD_D_PIN       DIO_PIN4
#define SSD_E_PORT      DIO_PORTA
#define SSD_E_PIN       DIO_PIN5
#define SSD_F_PORT      DIO_PORTA
#define SSD_F_PIN       DIO_PIN6
#define SSD_G_PORT      DIO_PORTA
#define SSD_G_PIN       DIO_PIN7

#endif /* SSD_CONFIG_H_ */
