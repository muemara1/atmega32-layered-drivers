/******************************************************************************
 * File        : TIMER_private.h
 * Layer       : MCAL
 *****************************************************************************/
#ifndef TIMER_PRIVATE_H_
#define TIMER_PRIVATE_H_

#define TIMER_DUTY_MAX          ((u8)100U)
#define TIMER_TOP_8BIT          ((u16)255U)
#define TIMER_COM_SHIFT         (4U)

#define TIMER0_OC0_PORT         DIO_PORTB
#define TIMER0_OC0_PIN          DIO_PIN3

#endif /* TIMER_PRIVATE_H_ */
