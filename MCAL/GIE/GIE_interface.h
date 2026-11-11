/******************************************************************************
 * File        : GIE_interface.h
 * Layer       : MCAL
 * Description : Global interrupt enable (I-bit in SREG).
 *****************************************************************************/
#ifndef GIE_INTERFACE_H_
#define GIE_INTERFACE_H_

#include "STD_TYPES.h"

void GIE_voidEnable(void);
void GIE_voidDisable(void);

/* Disables interrupts and returns the previous SREG so a critical section
 * can restore the original state with GIE_voidRestore(). */
u8   GIE_u8EnterCritical(void);
void GIE_voidRestore(u8 Copy_u8SavedSreg);

#endif /* GIE_INTERFACE_H_ */
