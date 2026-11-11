/******************************************************************************
 * File        : EXTI_interface.h
 * Layer       : MCAL
 * Description : External interrupts INT0 (PD2), INT1 (PD3), INT2 (PB2).
 *               Configure the pin as input (with pull-up for a button to GND)
 *               in DIO_config.h before enabling the interrupt.
 *****************************************************************************/
#ifndef EXTI_INTERFACE_H_
#define EXTI_INTERFACE_H_

#include "STD_TYPES.h"

#define EXTI_INT0           ((u8)0U)
#define EXTI_INT1           ((u8)1U)
#define EXTI_INT2           ((u8)2U)

#define EXTI_LOW_LEVEL      ((u8)0U)    /* INT0 / INT1 only */
#define EXTI_ANY_CHANGE     ((u8)1U)    /* INT0 / INT1 only */
#define EXTI_FALLING_EDGE   ((u8)2U)
#define EXTI_RISING_EDGE    ((u8)3U)

#define EXTI_DISABLED       ((u8)0U)
#define EXTI_ENABLED        ((u8)1U)

/* Applies sense control and enable state from EXTI_config.h */
ES_t EXTI_enuInit(void);

ES_t EXTI_enuSetSenseControl(u8 Copy_u8IntId, u8 Copy_u8Sense);
ES_t EXTI_enuEnable(u8 Copy_u8IntId);
ES_t EXTI_enuDisable(u8 Copy_u8IntId);
ES_t EXTI_enuSetCallback(u8 Copy_u8IntId, void (*Copy_pfCallback)(void));

#endif /* EXTI_INTERFACE_H_ */
