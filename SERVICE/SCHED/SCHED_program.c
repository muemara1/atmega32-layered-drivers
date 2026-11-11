/******************************************************************************
 * File        : SCHED_program.c
 * Layer       : SERVICE
 * Description : Cooperative scheduler - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"

#include "GIE_interface.h"
#include "TIMER_interface.h"

#include "SCHED_interface.h"
#include "SCHED_config.h"

typedef struct
{
    void (*pfTask)(void);
    u16  u16Period;
    u16  u16Countdown;
    u8   u8Ready;       /* set by the tick ISR, cleared by the dispatcher.
                           A run missed while another task overran is
                           dropped, not queued, so a late task never
                           fires in a burst (important for steppers). */
} SCHED_Task_t;

static SCHED_Task_t    SCHED_astrTasks[SCHED_MAX_TASKS];
static u8              SCHED_u8TaskCount = 0U;
static volatile u32    SCHED_u32Ticks    = 0UL;

/* Called from the Timer2 compare ISR every 1 ms */
static void SCHED_voidTick(void)
{
    u8 Local_u8Index;

    SCHED_u32Ticks++;

    for (Local_u8Index = 0U; Local_u8Index < SCHED_u8TaskCount; Local_u8Index++)
    {
        if (SCHED_astrTasks[Local_u8Index].u16Countdown > 0U)
        {
            SCHED_astrTasks[Local_u8Index].u16Countdown--;
        }

        if (SCHED_astrTasks[Local_u8Index].u16Countdown == 0U)
        {
            SCHED_astrTasks[Local_u8Index].u16Countdown = SCHED_astrTasks[Local_u8Index].u16Period;
            SCHED_astrTasks[Local_u8Index].u8Ready      = STD_TRUE;
        }
    }
}

ES_t SCHED_enuInit(void)
{
    ES_t Local_enuErrorState;

    SCHED_u8TaskCount = 0U;
    SCHED_u32Ticks    = 0UL;

    Local_enuErrorState = TIMER2_enuInitTick();
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TIMER2_enuSetCallback(&SCHED_voidTick);
    }

    return Local_enuErrorState;
}

ES_t SCHED_enuAddTask(void (*Copy_pfTask)(void), u16 Copy_u16PeriodMs, u16 Copy_u16OffsetMs)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_u8Sreg;

    if (Copy_pfTask == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if ((Copy_u16PeriodMs == 0U) || (SCHED_u8TaskCount >= SCHED_MAX_TASKS))
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        Local_u8Sreg = GIE_u8EnterCritical();

        SCHED_astrTasks[SCHED_u8TaskCount].pfTask       = Copy_pfTask;
        SCHED_astrTasks[SCHED_u8TaskCount].u16Period    = Copy_u16PeriodMs;
        SCHED_astrTasks[SCHED_u8TaskCount].u16Countdown = (u16)(Copy_u16OffsetMs + 1U);
        SCHED_astrTasks[SCHED_u8TaskCount].u8Ready      = STD_FALSE;
        SCHED_u8TaskCount++;

        GIE_voidRestore(Local_u8Sreg);
    }

    return Local_enuErrorState;
}

ES_t SCHED_enuStart(void)
{
    return TIMER2_enuStart();
}

void SCHED_voidDispatch(void)
{
    u8 Local_u8Index;
    u8 Local_u8Run;
    u8 Local_u8Sreg;

    for (Local_u8Index = 0U; Local_u8Index < SCHED_u8TaskCount; Local_u8Index++)
    {
        /* Test-and-clear must not race with the tick ISR */
        Local_u8Sreg = GIE_u8EnterCritical();
        Local_u8Run  = SCHED_astrTasks[Local_u8Index].u8Ready;
        SCHED_astrTasks[Local_u8Index].u8Ready = STD_FALSE;
        GIE_voidRestore(Local_u8Sreg);

        if (Local_u8Run == STD_TRUE)
        {
            SCHED_astrTasks[Local_u8Index].pfTask();
        }
    }
}

u32 SCHED_u32GetTicks(void)
{
    u32 Local_u32Ticks;
    u8  Local_u8Sreg = GIE_u8EnterCritical();   /* 32-bit read is not atomic on AVR */

    Local_u32Ticks = SCHED_u32Ticks;
    GIE_voidRestore(Local_u8Sreg);

    return Local_u32Ticks;
}
