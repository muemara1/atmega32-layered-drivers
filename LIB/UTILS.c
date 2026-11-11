/******************************************************************************
 * File        : UTILS.c
 * Layer       : LIB
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#define UTILS_MAX_U32_DIGITS    ((u8)10U)

u8 UTILS_u8U32ToString(u32 Copy_u32Value, char *Copy_pcBuffer, u8 Copy_u8BufferSize)
{
    char Local_acReversed[UTILS_MAX_U32_DIGITS];
    u8   Local_u8Count = 0U;
    u8   Local_u8Index;
    u32  Local_u32Value = Copy_u32Value;

    if (Copy_pcBuffer == NULL)
    {
        Local_u8Count = 0U;
    }
    else
    {
        do
        {
            Local_acReversed[Local_u8Count] = (char)('0' + (char)(Local_u32Value % 10UL));
            Local_u32Value /= 10UL;
            Local_u8Count++;
        } while ((Local_u32Value != 0UL) && (Local_u8Count < UTILS_MAX_U32_DIGITS));

        if ((u8)(Local_u8Count + 1U) > Copy_u8BufferSize)
        {
            Local_u8Count = 0U;
            if (Copy_u8BufferSize > 0U)
            {
                Copy_pcBuffer[0] = '\0';
            }
        }
        else
        {
            for (Local_u8Index = 0U; Local_u8Index < Local_u8Count; Local_u8Index++)
            {
                Copy_pcBuffer[Local_u8Index] = Local_acReversed[Local_u8Count - 1U - Local_u8Index];
            }
            Copy_pcBuffer[Local_u8Count] = '\0';
        }
    }

    return Local_u8Count;
}

u8 UTILS_u8S32ToString(s32 Copy_s32Value, char *Copy_pcBuffer, u8 Copy_u8BufferSize)
{
    u8 Local_u8Length = 0U;

    if ((Copy_pcBuffer == NULL) || (Copy_u8BufferSize < 2U))
    {
        Local_u8Length = 0U;
    }
    else if (Copy_s32Value < 0L)
    {
        /* Negate in unsigned arithmetic so INT32_MIN is handled */
        u32 Local_u32Magnitude = (u32)0UL - (u32)Copy_s32Value;

        Copy_pcBuffer[0] = '-';
        Local_u8Length = UTILS_u8U32ToString(Local_u32Magnitude, &Copy_pcBuffer[1],
                                             (u8)(Copy_u8BufferSize - 1U));
        if (Local_u8Length == 0U)
        {
            Copy_pcBuffer[0] = '\0';
        }
        else
        {
            Local_u8Length++;
        }
    }
    else
    {
        Local_u8Length = UTILS_u8U32ToString((u32)Copy_s32Value, Copy_pcBuffer, Copy_u8BufferSize);
    }

    return Local_u8Length;
}

void UTILS_voidTwoDigits(u8 Copy_u8Value, char *Copy_pcOut)
{
    u8 Local_u8Value = (Copy_u8Value > 99U) ? 99U : Copy_u8Value;

    if (Copy_pcOut != NULL)
    {
        Copy_pcOut[0] = (char)('0' + (char)(Local_u8Value / 10U));
        Copy_pcOut[1] = (char)('0' + (char)(Local_u8Value % 10U));
    }
}

u8 UTILS_u8BytesEqual(const u8 *Copy_pu8A, const u8 *Copy_pu8B, u8 Copy_u8Length)
{
    u8 Local_u8Diff = 0U;
    u8 Local_u8Index;

    if ((Copy_pu8A == NULL) || (Copy_pu8B == NULL))
    {
        Local_u8Diff = 1U;
    }
    else
    {
        for (Local_u8Index = 0U; Local_u8Index < Copy_u8Length; Local_u8Index++)
        {
            Local_u8Diff |= (u8)(Copy_pu8A[Local_u8Index] ^ Copy_pu8B[Local_u8Index]);
        }
    }

    return (Local_u8Diff == 0U) ? STD_TRUE : STD_FALSE;
}

u8 UTILS_u8BcdToBin(u8 Copy_u8Bcd)
{
    return (u8)((u8)((Copy_u8Bcd >> 4) * 10U) + (u8)(Copy_u8Bcd & 0x0FU));
}

u8 UTILS_u8BinToBcd(u8 Copy_u8Bin)
{
    return (u8)((u8)((Copy_u8Bin / 10U) << 4) | (u8)(Copy_u8Bin % 10U));
}
