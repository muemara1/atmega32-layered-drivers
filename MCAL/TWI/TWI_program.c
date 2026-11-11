/******************************************************************************
 * File        : TWI_program.c
 * Layer       : MCAL
 * Description : TWI (I2C) master driver - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "TWI_interface.h"
#include "TWI_config.h"
#include "TWI_private.h"

/* Starts the action encoded in Copy_u8Control, waits for TWINT and checks
 * the resulting status against Copy_u8Expected. */
static ES_t TWI_enuExecute(u8 Copy_u8Control, u8 Copy_u8Expected)
{
    ES_t Local_enuErrorState = ES_OK;
    u32  Local_u32Counter    = 0UL;

    TWCR = (u8)(Copy_u8Control | (u8)(1U << TWCR_TWINT) | (u8)(1U << TWCR_TWEN));

    while ((GET_BIT(TWCR, TWCR_TWINT) == 0U) && (Local_u32Counter < TWI_TIMEOUT_COUNT))
    {
        Local_u32Counter++;
    }

    if (Local_u32Counter >= TWI_TIMEOUT_COUNT)
    {
        Local_enuErrorState = ES_TIMEOUT;
    }
    else if ((TWSR & TWI_STATUS_MASK) != Copy_u8Expected)
    {
        Local_enuErrorState = ES_NOK;
    }
    else
    {
        /* Expected status */
    }

    return Local_enuErrorState;
}

ES_t TWI_enuInitMaster(void)
{
    TWBR = TWI_TWBR_VALUE;
    TWSR = 0U;                       /* prescaler bits TWPS1:0 = 0 */
    TWCR = (u8)(1U << TWCR_TWEN);

    return ES_OK;
}

ES_t TWI_enuStart(void)
{
    return TWI_enuExecute((u8)(1U << TWCR_TWSTA), TWI_SC_START);
}

ES_t TWI_enuRepeatedStart(void)
{
    return TWI_enuExecute((u8)(1U << TWCR_TWSTA), TWI_SC_REP_START);
}

ES_t TWI_enuSendSlaveAddressWrite(u8 Copy_u8Address7)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Address7 > 0x7FU)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        TWDR = (u8)(Copy_u8Address7 << 1);      /* R/W bit = 0 */
        Local_enuErrorState = TWI_enuExecute(0U, TWI_SC_MT_SLA_W_ACK);
    }

    return Local_enuErrorState;
}

ES_t TWI_enuSendSlaveAddressRead(u8 Copy_u8Address7)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Address7 > 0x7FU)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        TWDR = (u8)((u8)(Copy_u8Address7 << 1) | 1U);   /* R/W bit = 1 */
        Local_enuErrorState = TWI_enuExecute(0U, TWI_SC_MR_SLA_R_ACK);
    }

    return Local_enuErrorState;
}

ES_t TWI_enuWriteData(u8 Copy_u8Data)
{
    TWDR = Copy_u8Data;
    return TWI_enuExecute(0U, TWI_SC_MT_DATA_ACK);
}

ES_t TWI_enuReadDataAck(u8 *Copy_pu8Data)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pu8Data == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        Local_enuErrorState = TWI_enuExecute((u8)(1U << TWCR_TWEA), TWI_SC_MR_DATA_ACK);
        if (Local_enuErrorState == ES_OK)
        {
            *Copy_pu8Data = TWDR;
        }
    }

    return Local_enuErrorState;
}

ES_t TWI_enuReadDataNack(u8 *Copy_pu8Data)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pu8Data == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        Local_enuErrorState = TWI_enuExecute(0U, TWI_SC_MR_DATA_NACK);
        if (Local_enuErrorState == ES_OK)
        {
            *Copy_pu8Data = TWDR;
        }
    }

    return Local_enuErrorState;
}

void TWI_voidStop(void)
{
    u32 Local_u32Counter = 0UL;

    TWCR = (u8)((1U << TWCR_TWINT) | (1U << TWCR_TWSTO) | (1U << TWCR_TWEN));

    /* TWSTO is cleared by hardware once the STOP condition is on the bus */
    while ((GET_BIT(TWCR, TWCR_TWSTO) == 1U) && (Local_u32Counter < TWI_TIMEOUT_COUNT))
    {
        Local_u32Counter++;
    }
}
