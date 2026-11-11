/******************************************************************************
 * File        : DCM_interface.h
 * Layer       : HAL
 * Description : DC motor through an H-bridge (L293D / L298N).
 *               EN  <- OC0 (PB3) PWM from Timer0 (speed)
 *               IN1, IN2 <- DIO pins (direction)
 *               This driver owns Timer0: do not use Timer0 elsewhere.
 *****************************************************************************/
#ifndef DCM_INTERFACE_H_
#define DCM_INTERFACE_H_

#include "STD_TYPES.h"

#define DCM_DIR_CW      ((u8)0U)
#define DCM_DIR_CCW     ((u8)1U)

ES_t DCM_enuInit(void);
ES_t DCM_enuSetSpeed(u8 Copy_u8SpeedPercent);   /* 0..100 */
ES_t DCM_enuSetDirection(u8 Copy_u8Direction);
ES_t DCM_enuToggleDirection(void);
ES_t DCM_enuGetDirection(u8 *Copy_pu8Direction);
ES_t DCM_enuStop(void);                          /* speed 0, both inputs low */

#endif /* DCM_INTERFACE_H_ */
