/******************************************************************************
 * File        : RTC_program.c
 * Layer       : HAL
 * Description : DS1307 real-time clock - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#include "TWI_interface.h"

#include "RTC_interface.h"

#define RTC_I2C_ADDRESS     ((u8)0x68U)
#define RTC_REG_SECONDS     ((u8)0x00U)
#define RTC_REG_COUNT       ((u8)7U)
#define RTC_CH_BIT_MASK     ((u8)0x80U)   /* clock halt, in seconds register */
#define RTC_HOURS_24H_MASK  ((u8)0x3FU)

static ES_t RTC_enuWriteRegisters(u8 Copy_u8StartReg, const u8 *Copy_pu8Data, u8 Copy_u8Count)
{
    ES_t Local_enuErrorState = TWI_enuStart();
    u8   Local_u8Index;

    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TWI_enuSendSlaveAddressWrite(RTC_I2C_ADDRESS);
    }
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TWI_enuWriteData(Copy_u8StartReg);
    }
    for (Local_u8Index = 0U; (Local_u8Index < Copy_u8Count) && (Local_enuErrorState == ES_OK); Local_u8Index++)
    {
        Local_enuErrorState = TWI_enuWriteData(Copy_pu8Data[Local_u8Index]);
    }
    TWI_voidStop();

    return Local_enuErrorState;
}

static ES_t RTC_enuReadRegisters(u8 Copy_u8StartReg, u8 *Copy_pu8Data, u8 Copy_u8Count)
{
    ES_t Local_enuErrorState = TWI_enuStart();
    u8   Local_u8Index;

    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TWI_enuSendSlaveAddressWrite(RTC_I2C_ADDRESS);
    }
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TWI_enuWriteData(Copy_u8StartReg);
    }
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TWI_enuRepeatedStart();
    }
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = TWI_enuSendSlaveAddressRead(RTC_I2C_ADDRESS);
    }
    for (Local_u8Index = 0U; (Local_u8Index < Copy_u8Count) && (Local_enuErrorState == ES_OK); Local_u8Index++)
    {
        if (Local_u8Index == (u8)(Copy_u8Count - 1U))
        {
            Local_enuErrorState = TWI_enuReadDataNack(&Copy_pu8Data[Local_u8Index]);
        }
        else
        {
            Local_enuErrorState = TWI_enuReadDataAck(&Copy_pu8Data[Local_u8Index]);
        }
    }
    TWI_voidStop();

    return Local_enuErrorState;
}

ES_t RTC_enuInit(void)
{
    u8   Local_u8Seconds     = 0U;
    ES_t Local_enuErrorState = RTC_enuReadRegisters(RTC_REG_SECONDS, &Local_u8Seconds, 1U);

    if ((Local_enuErrorState == ES_OK) && ((Local_u8Seconds & RTC_CH_BIT_MASK) != 0U))
    {
        Local_u8Seconds &= (u8)(~RTC_CH_BIT_MASK);
        Local_enuErrorState = RTC_enuWriteRegisters(RTC_REG_SECONDS, &Local_u8Seconds, 1U);
    }

    return Local_enuErrorState;
}

ES_t RTC_enuSetTime(const RTC_Time_t *Copy_pstrTime)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_au8Regs[RTC_REG_COUNT];

    if (Copy_pstrTime == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if ((Copy_pstrTime->Seconds > 59U) || (Copy_pstrTime->Minutes > 59U) ||
             (Copy_pstrTime->Hours > 23U)   || (Copy_pstrTime->Day < 1U) || (Copy_pstrTime->Day > 7U) ||
             (Copy_pstrTime->Date < 1U)     || (Copy_pstrTime->Date > 31U) ||
             (Copy_pstrTime->Month < 1U)    || (Copy_pstrTime->Month > 12U) || (Copy_pstrTime->Year > 99U))
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        Local_au8Regs[0] = UTILS_u8BinToBcd(Copy_pstrTime->Seconds);   /* CH = 0: run */
        Local_au8Regs[1] = UTILS_u8BinToBcd(Copy_pstrTime->Minutes);
        Local_au8Regs[2] = UTILS_u8BinToBcd(Copy_pstrTime->Hours);     /* bit 6 = 0: 24 h */
        Local_au8Regs[3] = Copy_pstrTime->Day;
        Local_au8Regs[4] = UTILS_u8BinToBcd(Copy_pstrTime->Date);
        Local_au8Regs[5] = UTILS_u8BinToBcd(Copy_pstrTime->Month);
        Local_au8Regs[6] = UTILS_u8BinToBcd(Copy_pstrTime->Year);

        Local_enuErrorState = RTC_enuWriteRegisters(RTC_REG_SECONDS, Local_au8Regs, RTC_REG_COUNT);
    }

    return Local_enuErrorState;
}

ES_t RTC_enuGetTime(RTC_Time_t *Copy_pstrTime)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_au8Regs[RTC_REG_COUNT] = { 0U };

    if (Copy_pstrTime == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        Local_enuErrorState = RTC_enuReadRegisters(RTC_REG_SECONDS, Local_au8Regs, RTC_REG_COUNT);
        if (Local_enuErrorState == ES_OK)
        {
            Copy_pstrTime->Seconds = UTILS_u8BcdToBin((u8)(Local_au8Regs[0] & (u8)(~RTC_CH_BIT_MASK)));
            Copy_pstrTime->Minutes = UTILS_u8BcdToBin(Local_au8Regs[1]);
            Copy_pstrTime->Hours   = UTILS_u8BcdToBin((u8)(Local_au8Regs[2] & RTC_HOURS_24H_MASK));
            Copy_pstrTime->Day     = Local_au8Regs[3];
            Copy_pstrTime->Date    = UTILS_u8BcdToBin(Local_au8Regs[4]);
            Copy_pstrTime->Month   = UTILS_u8BcdToBin(Local_au8Regs[5]);
            Copy_pstrTime->Year    = UTILS_u8BcdToBin(Local_au8Regs[6]);
        }
    }

    return Local_enuErrorState;
}
