/******************************************************************************
 * File        : TIMER_interface.h
 * Layer       : MCAL
 * Description : Timer0 (PWM on OC0 = PB3, or CTC/normal with callbacks) and
 *               Timer2 (CTC periodic tick, used by the scheduler service).
 *****************************************************************************/
#ifndef TIMER_INTERFACE_H_
#define TIMER_INTERFACE_H_

#include "STD_TYPES.h"

/* Timer0 modes (TIMER_config.h) */
#define TIMER0_MODE_NORMAL          0U
#define TIMER0_MODE_PHASE_PWM       1U
#define TIMER0_MODE_CTC             2U
#define TIMER0_MODE_FAST_PWM        3U

/* Timer0 OC0 pin behaviour in PWM modes */
#define TIMER0_PWM_NON_INVERTING    2U
#define TIMER0_PWM_INVERTING        3U

/* Timer0 clock select */
#define TIMER0_STOPPED              0U
#define TIMER0_PRESCALER_1          1U
#define TIMER0_PRESCALER_8          2U
#define TIMER0_PRESCALER_64         3U
#define TIMER0_PRESCALER_256        4U
#define TIMER0_PRESCALER_1024       5U

/* Timer2 clock select (different encoding from Timer0) */
#define TIMER2_STOPPED              0U
#define TIMER2_PRESCALER_1          1U
#define TIMER2_PRESCALER_8          2U
#define TIMER2_PRESCALER_32         3U
#define TIMER2_PRESCALER_64         4U
#define TIMER2_PRESCALER_128        5U
#define TIMER2_PRESCALER_256        6U
#define TIMER2_PRESCALER_1024       7U

/* Callback sources for Timer0 */
#define TIMER0_EVENT_OVERFLOW       ((u8)0U)
#define TIMER0_EVENT_COMPARE        ((u8)1U)

/*------------------------------- Timer0 -----------------------------------*/
ES_t TIMER0_enuInit(void);
ES_t TIMER0_enuStart(void);
ES_t TIMER0_enuStop(void);
ES_t TIMER0_enuSetCompareValue(u8 Copy_u8Value);

/* PWM modes only: 0..100 %. 0 % forces the output low (no glitch pulse) */
ES_t TIMER0_enuSetDutyCycle(u8 Copy_u8DutyPercent);

ES_t TIMER0_enuSetCallback(u8 Copy_u8Event, void (*Copy_pfCallback)(void));

/*------------------------------- Timer2 -----------------------------------*/
/* CTC mode, interrupt every (OCR2 + 1) * prescaler / F_CPU seconds */
ES_t TIMER2_enuInitTick(void);
ES_t TIMER2_enuStart(void);
ES_t TIMER2_enuStop(void);
ES_t TIMER2_enuSetCallback(void (*Copy_pfCallback)(void));

#endif /* TIMER_INTERFACE_H_ */
