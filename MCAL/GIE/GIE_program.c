/******************************************************************************
 * File        : GIE_program.c
 * Layer       : MCAL
 * Description : Global interrupt enable - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "GIE_interface.h"

void GIE_voidEnable(void)
{
    SET_BIT(SREG, SREG_I);
}

void GIE_voidDisable(void)
{
    CLR_BIT(SREG, SREG_I);
}

u8 GIE_u8EnterCritical(void)
{
    u8 Local_u8Sreg = SREG;
    CLR_BIT(SREG, SREG_I);
    return Local_u8Sreg;
}

void GIE_voidRestore(u8 Copy_u8SavedSreg)
{
    SREG = Copy_u8SavedSreg;
}
