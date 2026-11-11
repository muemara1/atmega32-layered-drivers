/******************************************************************************
 * File        : EXTI_private.h
 * Layer       : MCAL
 *****************************************************************************/
#ifndef EXTI_PRIVATE_H_
#define EXTI_PRIVATE_H_

#define EXTI_COUNT          ((u8)3U)

/* ISCx1:ISCx0 field masks in MCUCR */
#define EXTI_INT0_ISC_MASK  ((u8)0xFCU)
#define EXTI_INT1_ISC_MASK  ((u8)0xF3U)
#define EXTI_INT1_ISC_SHIFT (2U)

#endif /* EXTI_PRIVATE_H_ */
