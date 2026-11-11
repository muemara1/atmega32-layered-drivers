/******************************************************************************
 * File        : DIO_config.h  (locker_system)
 * Description : PA0 lock relay, PA1 buzzer, PA2 green LED, PA3 red LED
 *               (all outputs, start LOW = locked, silent, LEDs off).
 *               LCD, keypad, UART and TWI pins are set by their drivers.
 *****************************************************************************/
#ifndef DIO_CONFIG_H_
#define DIO_CONFIG_H_

#include "BIT_MATH.h"

#define I   0U
#define O   1U
#define L   0U
#define H   1U

/*                                      P7 P6 P5 P4 P3 P2 P1 P0 */
#define DIO_PORTA_DIRECTION   CONC_BIT( I, I, I, I, O, O, O, O )
#define DIO_PORTA_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTB_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTB_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTC_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTC_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTD_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTD_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#endif /* DIO_CONFIG_H_ */
