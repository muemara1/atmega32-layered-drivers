/******************************************************************************
 * File        : STP_interface.h
 * Layer       : HAL
 * Description : Unipolar stepper motor (e.g. 28BYJ-48 + ULN2003) on four
 *               DIO pins, half-step drive. STP_enuStep() is non-blocking so
 *               the application decides the step rate.
 *****************************************************************************/
#ifndef STP_INTERFACE_H_
#define STP_INTERFACE_H_

#include "STD_TYPES.h"

#define STP_DIR_CW      ((u8)0U)
#define STP_DIR_CCW     ((u8)1U)

ES_t STP_enuInit(void);

/* Advances the motor one half-step in the given direction */
ES_t STP_enuStep(u8 Copy_u8Direction);

/* Blocking: rotates by Copy_u16Steps half-steps at the configured rate */
ES_t STP_enuRotateSteps(u16 Copy_u16Steps, u8 Copy_u8Direction);

/* De-energizes all coils (no holding torque, no heating) */
ES_t STP_enuRelease(void);

#endif /* STP_INTERFACE_H_ */
