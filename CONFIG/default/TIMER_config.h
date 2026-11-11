/******************************************************************************
 * File        : TIMER_config.h  (default)
 * Layer       : MCAL configuration
 *****************************************************************************/
#ifndef TIMER_CONFIG_H_
#define TIMER_CONFIG_H_

/*------------------------------- Timer0 -----------------------------------
 * TIMER0_MODE       : TIMER0_MODE_NORMAL, TIMER0_MODE_CTC,
 *                     TIMER0_MODE_PHASE_PWM, TIMER0_MODE_FAST_PWM
 * TIMER0_PWM_OUTPUT : TIMER0_PWM_NON_INVERTING, TIMER0_PWM_INVERTING
 * TIMER0_PRESCALER  : TIMER0_PRESCALER_1 / 8 / 64 / 256 / 1024
 *
 * Fast PWM frequency = F_CPU / (prescaler * 256)
 *                    = 8 MHz / (8 * 256) = 3.9 kHz  (quiet for DC motors)
 *--------------------------------------------------------------------------*/
#define TIMER0_MODE             TIMER0_MODE_FAST_PWM
#define TIMER0_PWM_OUTPUT       TIMER0_PWM_NON_INVERTING
#define TIMER0_PRESCALER        TIMER0_PRESCALER_8
#define TIMER0_COMPARE_VALUE    ((u8)0U)        /* CTC mode only */

/*------------------------------- Timer2 -----------------------------------
 * Tick period = (TIMER2_COMPARE_VALUE + 1) * prescaler / F_CPU
 *             = 125 * 64 / 8 MHz = 1 ms
 *--------------------------------------------------------------------------*/
#define TIMER2_PRESCALER        TIMER2_PRESCALER_64
#define TIMER2_COMPARE_VALUE    ((u8)124U)

#endif /* TIMER_CONFIG_H_ */
