/******************************************************************************
 * File        : TWI_interface.h
 * Layer       : MCAL
 * Description : Two-wire interface (I2C) in master mode, SCL = PC0, SDA = PC1.
 *               Slave addresses are 7-bit (e.g. 0x68 for the DS1307).
 *               Use external 4.7k pull-ups on SCL and SDA.
 *****************************************************************************/
#ifndef TWI_INTERFACE_H_
#define TWI_INTERFACE_H_

#include "STD_TYPES.h"

ES_t TWI_enuInitMaster(void);

ES_t TWI_enuStart(void);
ES_t TWI_enuRepeatedStart(void);
ES_t TWI_enuSendSlaveAddressWrite(u8 Copy_u8Address7);
ES_t TWI_enuSendSlaveAddressRead(u8 Copy_u8Address7);
ES_t TWI_enuWriteData(u8 Copy_u8Data);

/* Read one byte and answer ACK (more bytes follow) or NACK (last byte) */
ES_t TWI_enuReadDataAck(u8 *Copy_pu8Data);
ES_t TWI_enuReadDataNack(u8 *Copy_pu8Data);

void TWI_voidStop(void);

#endif /* TWI_INTERFACE_H_ */
