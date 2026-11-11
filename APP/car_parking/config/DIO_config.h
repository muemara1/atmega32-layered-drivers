/******************************************************************************
 * File        : DIO_config.h  (car_parking)
 * Description : PD2 = entry IR sensor (INT0), PD3 = exit IR sensor (INT1),
 *               inputs with pull-up (sensors pull LOW when a car is there)
 *               PD4 = entry gate, PD5 = exit gate, PD6 = FULL LED,
 *               PD7 = buzzer (outputs, start LOW)
 *               LCD, SSD, UART and TWI pins are set by their drivers.
 *****************************************************************************/
#ifndef DIO_CONFIG_H_
#define DIO_CONFIG_H_

#include "BIT_MATH.h"

#define I   0U
#define O   1U
#define L   0U
#define H   1U

/*                                      P7 P6 P5 P4 P3 P2 P1 P0 */
#define DIO_PORTA_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTA_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTB_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTB_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTC_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTC_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTD_DIRECTION   CONC_BIT( O, O, O, O, I, I, I, I )
#define DIO_PORTD_INIT_VALUE  CONC_BIT( L, L, L, L, H, H, L, L )

#endif /* DIO_CONFIG_H_ */
