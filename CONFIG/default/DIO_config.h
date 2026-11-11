/******************************************************************************
 * File        : DIO_config.h  (default)
 * Layer       : MCAL configuration
 * Description : Initial direction and level of every pin, applied by
 *               DIO_enuInit(). Arguments are listed from pin 7 down to pin 0.
 *               Default: every pin input, no pull-ups (safe reset state).
 *               HAL drivers configure their own pins in their init functions,
 *               so an application only overrides this file for raw pins
 *               (LEDs, buzzers, relays, push-buttons).
 *****************************************************************************/
#ifndef DIO_CONFIG_H_
#define DIO_CONFIG_H_

#include "BIT_MATH.h"

/* Short aliases for readability */
#define I   0U  /* input            */
#define O   1U  /* output           */
#define L   0U  /* low / no pull-up */
#define H   1U  /* high / pull-up   */

/*                                      P7 P6 P5 P4 P3 P2 P1 P0 */
#define DIO_PORTA_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTA_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTB_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTB_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTC_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTC_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#define DIO_PORTD_DIRECTION   CONC_BIT( I, I, I, I, I, I, I, I )
#define DIO_PORTD_INIT_VALUE  CONC_BIT( L, L, L, L, L, L, L, L )

#endif /* DIO_CONFIG_H_ */
