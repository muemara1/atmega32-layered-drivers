/******************************************************************************
 * File        : EEPROM_program.c
 * Layer       : HAL
 * Description : 24C16 external EEPROM - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "DELAY.h"

#include "TWI_interface.h"

#include "EEPROM_interface.h"
#include "EEPROM_config.h"

#define EEPROM_BASE_ADDRESS     ((u8)0x50U)   /* 1010 xxx */
#define EEPROM_BLOCK_MASK       ((u8)0x07U)

/* Device address = 1010 + block bits (address bits 10..8) */
static u8 EEPROM_u8DeviceAddress(u16 Copy_u16Address)
{
    return (u8)(EEPROM_BASE_ADDRESS | (u8)((u8)(Copy_u16Address >> 8) & EEPROM_BLOCK_MASK));
}

ES_t EEPROM_enuWriteByte(u16 Copy_u16Address, u8 Copy_u8Data)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u16Address >= EEPROM_SIZE_BYTES)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        Local_enuErrorState = TWI_enuStart();
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuSendSlaveAddressWrite(EEPROM_u8DeviceAddress(Copy_u16Address));
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuWriteData((u8)(Copy_u16Address & 0xFFU));
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuWriteData(Copy_u8Data);
        }
        TWI_voidStop();

        /* Internal write cycle (t_WR max 5 ms) */
        DELAY_MS(EEPROM_WRITE_CYCLE_MS);
    }

    return Local_enuErrorState;
}

ES_t EEPROM_enuReadByte(u16 Copy_u16Address, u8 *Copy_pu8Data)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_u8Device;

    if (Copy_pu8Data == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u16Address >= EEPROM_SIZE_BYTES)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        Local_u8Device = EEPROM_u8DeviceAddress(Copy_u16Address);

        /* Dummy write to set the address, then random read */
        Local_enuErrorState = TWI_enuStart();
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuSendSlaveAddressWrite(Local_u8Device);
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuWriteData((u8)(Copy_u16Address & 0xFFU));
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuRepeatedStart();
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuSendSlaveAddressRead(Local_u8Device);
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = TWI_enuReadDataNack(Copy_pu8Data);
        }
        TWI_voidStop();
    }

    return Local_enuErrorState;
}

ES_t EEPROM_enuWriteBlock(u16 Copy_u16Address, const u8 *Copy_pu8Data, u16 Copy_u16Length)
{
    ES_t Local_enuErrorState = ES_OK;
    u16  Local_u16Index;

    if (Copy_pu8Data == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (((u32)Copy_u16Address + (u32)Copy_u16Length) > (u32)EEPROM_SIZE_BYTES)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        /* Byte writes avoid the 16-byte page roll-over trap of page writes */
        for (Local_u16Index = 0U; (Local_u16Index < Copy_u16Length) && (Local_enuErrorState == ES_OK); Local_u16Index++)
        {
            Local_enuErrorState = EEPROM_enuWriteByte((u16)(Copy_u16Address + Local_u16Index),
                                                      Copy_pu8Data[Local_u16Index]);
        }
    }

    return Local_enuErrorState;
}

ES_t EEPROM_enuReadBlock(u16 Copy_u16Address, u8 *Copy_pu8Data, u16 Copy_u16Length)
{
    ES_t Local_enuErrorState = ES_OK;
    u16  Local_u16Index;

    if (Copy_pu8Data == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (((u32)Copy_u16Address + (u32)Copy_u16Length) > (u32)EEPROM_SIZE_BYTES)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        for (Local_u16Index = 0U; (Local_u16Index < Copy_u16Length) && (Local_enuErrorState == ES_OK); Local_u16Index++)
        {
            Local_enuErrorState = EEPROM_enuReadByte((u16)(Copy_u16Address + Local_u16Index),
                                                     &Copy_pu8Data[Local_u16Index]);
        }
    }

    return Local_enuErrorState;
}
