/******************************************************************************
 * File        : DIO_config.h  (smart_home)
 * Description : PA1 = LM35 output (analog, no pull-up)
 *               PD2 = flame sensor DO (INT0), pull-up, LOW = flame
 *               PD3 = alarm reset button (INT1) to GND, pull-up
 *               PC2 = buzzer, PC3 = alarm LED, PC4 = sprinkler pump relay,
 *               PC5 = "system OK" LED (outputs, start LOW)
 *               LCD and fan (DC motor) pins are set by their drivers.
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

#define DIO_PORTC_DIRECTION   CONC_BIT( I, I, O, O, O, O, I, I )
#define DIO_PORTC_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTD_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTD_INIT_VALUE  CONC_BIT( L, L, L, L, H, H, L, L )

#endif /* DIO_CONFIG_H_ */
