/******************************************************************************
 * File        : LCD_interface.h
 * Layer       : HAL
 * Description : HD44780-compatible 16x2 character LCD in 4-bit mode.
 *               RW is tied to GND, so the driver uses datasheet delays
 *               instead of polling the busy flag.
 *****************************************************************************/
#ifndef LCD_INTERFACE_H_
#define LCD_INTERFACE_H_

#include "STD_TYPES.h"

#define LCD_ROWS                ((u8)2U)
#define LCD_COLUMNS             ((u8)16U)

#define LCD_ROW0                ((u8)0U)
#define LCD_ROW1                ((u8)1U)

/* Commands */
#define LCD_CMD_CLEAR           ((u8)0x01U)
#define LCD_CMD_HOME            ((u8)0x02U)
#define LCD_CMD_ENTRY_MODE      ((u8)0x06U)   /* increment, no shift   */
#define LCD_CMD_DISPLAY_OFF     ((u8)0x08U)
#define LCD_CMD_DISPLAY_ON      ((u8)0x0CU)   /* cursor off, blink off */
#define LCD_CMD_CURSOR_BLINK    ((u8)0x0FU)
#define LCD_CMD_FUNCTION_4BIT   ((u8)0x28U)   /* 4-bit, 2 lines, 5x8   */
#define LCD_CMD_SET_CGRAM       ((u8)0x40U)
#define LCD_CMD_SET_DDRAM       ((u8)0x80U)

ES_t LCD_enuInit(void);
ES_t LCD_enuSendCommand(u8 Copy_u8Command);
ES_t LCD_enuClear(void);
ES_t LCD_enuGoTo(u8 Copy_u8Row, u8 Copy_u8Column);

ES_t LCD_enuWriteChar(char Copy_cChar);
ES_t LCD_enuWriteString(const char *Copy_pcString);
ES_t LCD_enuWriteNumber(s32 Copy_s32Number);

/* Clears one row and writes a string there (convenient for status lines) */
ES_t LCD_enuWriteLine(u8 Copy_u8Row, const char *Copy_pcString);

/* Stores a custom 5x8 glyph in CGRAM slot 0..7; print it with WriteChar(slot) */
ES_t LCD_enuCreateCustomChar(u8 Copy_u8Slot, const u8 Copy_au8Pattern[8]);

#endif /* LCD_INTERFACE_H_ */
