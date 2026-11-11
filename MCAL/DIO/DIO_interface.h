/******************************************************************************
 * File        : DIO_interface.h
 * Layer       : MCAL
 * Description : Digital input/output driver - public API.
 *****************************************************************************/
#ifndef DIO_INTERFACE_H_
#define DIO_INTERFACE_H_

#include "STD_TYPES.h"

/* Ports */
#define DIO_PORTA       ((u8)0U)
#define DIO_PORTB       ((u8)1U)
#define DIO_PORTC       ((u8)2U)
#define DIO_PORTD       ((u8)3U)

/* Pins */
#define DIO_PIN0        ((u8)0U)
#define DIO_PIN1        ((u8)1U)
#define DIO_PIN2        ((u8)2U)
#define DIO_PIN3        ((u8)3U)
#define DIO_PIN4        ((u8)4U)
#define DIO_PIN5        ((u8)5U)
#define DIO_PIN6        ((u8)6U)
#define DIO_PIN7        ((u8)7U)

/* Directions */
#define DIO_INPUT       ((u8)0U)
#define DIO_OUTPUT      ((u8)1U)

/* Levels. For an input pin, writing DIO_HIGH enables the internal pull-up */
#define DIO_LOW         ((u8)0U)
#define DIO_HIGH        ((u8)1U)
#define DIO_FLOAT       DIO_LOW
#define DIO_PULLUP      DIO_HIGH

/* Whole-port values */
#define DIO_PORT_INPUT  ((u8)0x00U)
#define DIO_PORT_OUTPUT ((u8)0xFFU)
#define DIO_PORT_LOW    ((u8)0x00U)
#define DIO_PORT_HIGH   ((u8)0xFFU)

/* Applies the pre-build configuration in DIO_config.h to all four ports */
ES_t DIO_enuInit(void);

ES_t DIO_enuSetPinDirection(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Direction);
ES_t DIO_enuSetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Value);
ES_t DIO_enuGetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin, u8 *Copy_pu8Value);
ES_t DIO_enuTogglePinValue(u8 Copy_u8Port, u8 Copy_u8Pin);

ES_t DIO_enuSetPortDirection(u8 Copy_u8Port, u8 Copy_u8Direction);
ES_t DIO_enuSetPortValue(u8 Copy_u8Port, u8 Copy_u8Value);
ES_t DIO_enuGetPortValue(u8 Copy_u8Port, u8 *Copy_pu8Value);

#endif /* DIO_INTERFACE_H_ */
