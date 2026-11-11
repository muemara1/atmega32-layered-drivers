/******************************************************************************
 * File        : SSD_program.c
 * Layer       : HAL
 * Description : Seven-segment display - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "DIO_interface.h"

#include "SSD_interface.h"
#include "SSD_config.h"

#define SSD_SEGMENTS    ((u8)7U)
#define SSD_DIGITS      ((u8)10U)

/*                                         0     1     2     3     4     5     6     7     8     9  */
static const u8 SSD_au8Digits[SSD_DIGITS] = { 0x3FU, 0x06U, 0x5BU, 0x4FU, 0x66U, 0x6DU, 0x7DU, 0x07U, 0x7FU, 0x6FU };

static const u8 SSD_au8SegPort[SSD_SEGMENTS] = { SSD_A_PORT, SSD_B_PORT, SSD_C_PORT, SSD_D_PORT,
                                                 SSD_E_PORT, SSD_F_PORT, SSD_G_PORT };
static const u8 SSD_au8SegPin[SSD_SEGMENTS]  = { SSD_A_PIN,  SSD_B_PIN,  SSD_C_PIN,  SSD_D_PIN,
                                                 SSD_E_PIN,  SSD_F_PIN,  SSD_G_PIN  };

ES_t SSD_enuInit(void)
{
    u8 Local_u8Segment;

    for (Local_u8Segment = 0U; Local_u8Segment < SSD_SEGMENTS; Local_u8Segment++)
    {
        (void)DIO_enuSetPinDirection(SSD_au8SegPort[Local_u8Segment], SSD_au8SegPin[Local_u8Segment],
                                     DIO_OUTPUT);
    }

    return SSD_enuTurnOff();
}

ES_t SSD_enuDisplayPattern(u8 Copy_u8Pattern)
{
    u8 Local_u8Segment;
    u8 Local_u8Level;

    for (Local_u8Segment = 0U; Local_u8Segment < SSD_SEGMENTS; Local_u8Segment++)
    {
        Local_u8Level = GET_BIT(Copy_u8Pattern, Local_u8Segment);
#if (SSD_TYPE == SSD_COMMON_ANODE)
        Local_u8Level ^= 1U;            /* segment ON = pin LOW */
#endif
        (void)DIO_enuSetPinValue(SSD_au8SegPort[Local_u8Segment], SSD_au8SegPin[Local_u8Segment],
                                 Local_u8Level);
    }

    return ES_OK;
}

ES_t SSD_enuDisplayDigit(u8 Copy_u8Digit)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Digit >= SSD_DIGITS)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        Local_enuErrorState = SSD_enuDisplayPattern(SSD_au8Digits[Copy_u8Digit]);
    }

    return Local_enuErrorState;
}

ES_t SSD_enuTurnOff(void)
{
    return SSD_enuDisplayPattern(SSD_SYMBOL_BLANK);
}
