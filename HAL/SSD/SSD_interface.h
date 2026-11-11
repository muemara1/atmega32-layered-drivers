/******************************************************************************
 * File        : SSD_interface.h
 * Layer       : HAL
 * Description : Single-digit seven-segment display, common cathode or
 *               common anode, segments on any pins.
 *****************************************************************************/
#ifndef SSD_INTERFACE_H_
#define SSD_INTERFACE_H_

#include "STD_TYPES.h"

#define SSD_COMMON_CATHODE  0U
#define SSD_COMMON_ANODE    1U

/* Segment patterns, bit order g f e d c b a */
#define SSD_SYMBOL_DASH     ((u8)0x40U)
#define SSD_SYMBOL_F        ((u8)0x71U)    /* "F" = full */
#define SSD_SYMBOL_E        ((u8)0x79U)    /* "E" = error */
#define SSD_SYMBOL_BLANK    ((u8)0x00U)

ES_t SSD_enuInit(void);
ES_t SSD_enuDisplayDigit(u8 Copy_u8Digit);          /* 0..9 */
ES_t SSD_enuDisplayPattern(u8 Copy_u8Pattern);      /* raw gfedcba */
ES_t SSD_enuTurnOff(void);

#endif /* SSD_INTERFACE_H_ */
