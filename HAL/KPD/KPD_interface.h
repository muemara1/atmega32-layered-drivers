/******************************************************************************
 * File        : KPD_interface.h
 * Layer       : HAL
 * Description : 4x4 matrix keypad. Rows are outputs, columns are inputs with
 *               internal pull-ups. Non-blocking: a key is reported once per
 *               press (on the debounced press edge), never while held.
 *****************************************************************************/
#ifndef KPD_INTERFACE_H_
#define KPD_INTERFACE_H_

#include "STD_TYPES.h"

#define KPD_ROWS        ((u8)4U)
#define KPD_COLUMNS     ((u8)4U)
#define KPD_NO_KEY      ((u8)0xFFU)

ES_t KPD_enuInit(void);

/* Writes the key character (from KPD_config.h) or KPD_NO_KEY */
ES_t KPD_enuGetPressedKey(u8 *Copy_pu8Key);

#endif /* KPD_INTERFACE_H_ */
