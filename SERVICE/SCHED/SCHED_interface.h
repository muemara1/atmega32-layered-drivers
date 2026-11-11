/******************************************************************************
 * File        : SCHED_interface.h
 * Layer       : SERVICE
 * Description : Cooperative time-triggered scheduler. Timer2 generates a
 *               1 ms tick; the ISR only marks tasks as due, and
 *               SCHED_voidDispatch() (called from the main loop) runs them.
 *               Tasks must be short and must never block.
 *****************************************************************************/
#ifndef SCHED_INTERFACE_H_
#define SCHED_INTERFACE_H_

#include "STD_TYPES.h"

/* Configures Timer2 for the tick; call before adding tasks */
ES_t SCHED_enuInit(void);

/* Period and first-run offset are in ticks (ms). Offsets spread the load. */
ES_t SCHED_enuAddTask(void (*Copy_pfTask)(void), u16 Copy_u16PeriodMs, u16 Copy_u16OffsetMs);

/* Starts the tick (enable global interrupts separately) */
ES_t SCHED_enuStart(void);

/* Runs every task that is due; call it continuously from while(1) */
void SCHED_voidDispatch(void);

/* Milliseconds since SCHED_enuStart (wraps after ~49 days) */
u32  SCHED_u32GetTicks(void);

#endif /* SCHED_INTERFACE_H_ */
