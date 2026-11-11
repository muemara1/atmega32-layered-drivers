/******************************************************************************
 * File        : UTILS.h
 * Layer       : LIB
 * Description : Small, allocation-free string helpers shared by all layers
 *               (no dependency on the C library, keeps flash usage low).
 *****************************************************************************/
#ifndef UTILS_H_
#define UTILS_H_

#include "STD_TYPES.h"

/* Converts Copy_u32Value to a NUL-terminated decimal string.
 * Returns the number of digits written, or 0 if the buffer is too small. */
u8 UTILS_u8U32ToString(u32 Copy_u32Value, char *Copy_pcBuffer, u8 Copy_u8BufferSize);

/* Same for signed values (adds a leading '-') */
u8 UTILS_u8S32ToString(s32 Copy_s32Value, char *Copy_pcBuffer, u8 Copy_u8BufferSize);

/* Writes exactly two digits ("07"), no terminator. Values > 99 give "99" */
void UTILS_voidTwoDigits(u8 Copy_u8Value, char *Copy_pcOut);

/* Compares Copy_u8Length bytes; returns STD_TRUE when equal.
 * Runs in constant time so password checks do not leak timing. */
u8 UTILS_u8BytesEqual(const u8 *Copy_pu8A, const u8 *Copy_pu8B, u8 Copy_u8Length);

/* BCD <-> binary for RTC registers */
u8 UTILS_u8BcdToBin(u8 Copy_u8Bcd);
u8 UTILS_u8BinToBcd(u8 Copy_u8Bin);

#endif /* UTILS_H_ */
