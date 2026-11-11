/******************************************************************************
 * File        : EEPROM_interface.h
 * Layer       : HAL
 * Description : External I2C EEPROM 24C16 (2 KB = 8 blocks x 256 bytes).
 *               The block number is carried in the device address bits, so
 *               A0..A2 pins of the chip are not used. Call TWI_enuInitMaster()
 *               before using this driver.
 *****************************************************************************/
#ifndef EEPROM_INTERFACE_H_
#define EEPROM_INTERFACE_H_

#include "STD_TYPES.h"

#define EEPROM_SIZE_BYTES   ((u16)2048U)

ES_t EEPROM_enuWriteByte(u16 Copy_u16Address, u8 Copy_u8Data);
ES_t EEPROM_enuReadByte(u16 Copy_u16Address, u8 *Copy_pu8Data);

ES_t EEPROM_enuWriteBlock(u16 Copy_u16Address, const u8 *Copy_pu8Data, u16 Copy_u16Length);
ES_t EEPROM_enuReadBlock(u16 Copy_u16Address, u8 *Copy_pu8Data, u16 Copy_u16Length);

#endif /* EEPROM_INTERFACE_H_ */
