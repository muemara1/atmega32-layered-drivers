/******************************************************************************
 * File        : BIT_MATH.h
 * Layer       : LIB
 * Description : Bit manipulation helpers for 8-bit registers.
 *****************************************************************************/
#ifndef BIT_MATH_H_
#define BIT_MATH_H_

#define SET_BIT(REG, BIT)   ((REG) |= (u8)(1U << (BIT)))
#define CLR_BIT(REG, BIT)   ((REG) &= (u8)(~(u8)(1U << (BIT))))
#define TOG_BIT(REG, BIT)   ((REG) ^= (u8)(1U << (BIT)))
#define GET_BIT(REG, BIT)   ((u8)(((u8)(REG) >> (BIT)) & 1U))

/* Writes VAL (0 or 1) into bit BIT of REG */
#define WRT_BIT(REG, BIT, VAL) \
    ((REG) = (u8)(((REG) & (u8)(~(u8)(1U << (BIT)))) | (u8)(((u8)(VAL) & 1U) << (BIT))))

/* Builds an 8-bit value from 8 single bits, b7 is the MSB */
#define CONC_BIT(b7, b6, b5, b4, b3, b2, b1, b0) \
    ((u8)(((u8)(b7) << 7) | ((u8)(b6) << 6) | ((u8)(b5) << 5) | ((u8)(b4) << 4) | \
          ((u8)(b3) << 3) | ((u8)(b2) << 2) | ((u8)(b1) << 1) | ((u8)(b0))))

#endif /* BIT_MATH_H_ */
