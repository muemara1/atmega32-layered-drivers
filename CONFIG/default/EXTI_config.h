/******************************************************************************
 * File        : EXTI_config.h  (default)
 * Layer       : MCAL configuration
 * Options     : EXTI_LOW_LEVEL, EXTI_ANY_CHANGE (INT0/INT1 only),
 *               EXTI_FALLING_EDGE, EXTI_RISING_EDGE
 *               EXTI_ENABLED / EXTI_DISABLED
 *****************************************************************************/
#ifndef EXTI_CONFIG_H_
#define EXTI_CONFIG_H_

#define EXTI_INT0_SENSE     EXTI_FALLING_EDGE
#define EXTI_INT1_SENSE     EXTI_FALLING_EDGE
#define EXTI_INT2_SENSE     EXTI_FALLING_EDGE

#define EXTI_INT0_STATE     EXTI_DISABLED
#define EXTI_INT1_STATE     EXTI_DISABLED
#define EXTI_INT2_STATE     EXTI_DISABLED

#endif /* EXTI_CONFIG_H_ */
