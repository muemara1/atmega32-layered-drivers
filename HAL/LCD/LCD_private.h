/******************************************************************************
 * File        : LCD_private.h
 * Layer       : HAL
 *****************************************************************************/
#ifndef LCD_PRIVATE_H_
#define LCD_PRIVATE_H_

#define LCD_ROW1_ADDRESS_OFFSET ((u8)0x40U)
#define LCD_CGRAM_SLOTS         ((u8)8U)
#define LCD_GLYPH_ROWS          ((u8)8U)
#define LCD_NUMBER_BUFFER_SIZE  ((u8)12U)

#define LCD_REGISTER_COMMAND    DIO_LOW
#define LCD_REGISTER_DATA       DIO_HIGH

#endif /* LCD_PRIVATE_H_ */
