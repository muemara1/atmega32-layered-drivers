/******************************************************************************
 * File        : EXTI_config.h  (smart_home)
 *****************************************************************************/
#ifndef EXTI_CONFIG_H_
#define EXTI_CONFIG_H_

#define EXTI_INT0_SENSE     EXTI_FALLING_EDGE   /* flame sensor goes LOW */
#define EXTI_INT1_SENSE     EXTI_FALLING_EDGE   /* reset button          */
#define EXTI_INT2_SENSE     EXTI_FALLING_EDGE

#define EXTI_INT0_STATE     EXTI_ENABLED
#define EXTI_INT1_STATE     EXTI_ENABLED
#define EXTI_INT2_STATE     EXTI_DISABLED

#endif /* EXTI_CONFIG_H_ */
