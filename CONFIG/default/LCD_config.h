/******************************************************************************
 * File        : LCD_config.h  (default)
 * Layer       : HAL configuration
 * Description : LCD wiring. RW must be tied to GND.
 *****************************************************************************/
#ifndef LCD_CONFIG_H_
#define LCD_CONFIG_H_

#define LCD_RS_PORT     DIO_PORTB
#define LCD_RS_PIN      DIO_PIN0

#define LCD_EN_PORT     DIO_PORTB
#define LCD_EN_PIN      DIO_PIN1

#define LCD_D4_PORT     DIO_PORTB
#define LCD_D4_PIN      DIO_PIN4
#define LCD_D5_PORT     DIO_PORTB
#define LCD_D5_PIN      DIO_PIN5
#define LCD_D6_PORT     DIO_PORTB
#define LCD_D6_PIN      DIO_PIN6
#define LCD_D7_PORT     DIO_PORTB
#define LCD_D7_PIN      DIO_PIN7

#endif /* LCD_CONFIG_H_ */
