/******************************************************************************
 * File        : STP_program.c
 * Layer       : HAL
 * Description : Unipolar stepper motor - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "DELAY.h"

#include "DIO_interface.h"

#include "STP_interface.h"
#include "STP_config.h"

#define STP_COILS           ((u8)4U)
#define STP_SEQUENCE_LENGTH ((u8)8U)

/* Half-step sequence, bit0 = coil A ... bit3 = coil D */
static const u8 STP_au8Sequence[STP_SEQUENCE_LENGTH] =
    { 0x01U, 0x03U, 0x02U, 0x06U, 0x04U, 0x0CU, 0x08U, 0x09U };

static const u8 STP_au8CoilPort[STP_COILS] = { STP_A_PORT, STP_B_PORT, STP_C_PORT, STP_D_PORT };
static const u8 STP_au8CoilPin[STP_COILS]  = { STP_A_PIN,  STP_B_PIN,  STP_C_PIN,  STP_D_PIN  };

static u8 STP_u8Index = 0U;

static void STP_voidWriteCoils(u8 Copy_u8Pattern)
{
    u8 Local_u8Coil;

    for (Local_u8Coil = 0U; Local_u8Coil < STP_COILS; Local_u8Coil++)
    {
        (void)DIO_enuSetPinValue(STP_au8CoilPort[Local_u8Coil], STP_au8CoilPin[Local_u8Coil],
                                 GET_BIT(Copy_u8Pattern, Local_u8Coil));
    }
}

ES_t STP_enuInit(void)
{
    u8 Local_u8Coil;

    for (Local_u8Coil = 0U; Local_u8Coil < STP_COILS; Local_u8Coil++)
    {
        (void)DIO_enuSetPinDirection(STP_au8CoilPort[Local_u8Coil], STP_au8CoilPin[Local_u8Coil],
                                     DIO_OUTPUT);
    }

    STP_u8Index = 0U;
    STP_voidWriteCoils(0U);

    return ES_OK;
}

ES_t STP_enuStep(u8 Copy_u8Direction)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Direction == STP_DIR_CW)
    {
        STP_u8Index = (u8)((STP_u8Index + 1U) % STP_SEQUENCE_LENGTH);
    }
    else if (Copy_u8Direction == STP_DIR_CCW)
    {
        STP_u8Index = (u8)((STP_u8Index + STP_SEQUENCE_LENGTH - 1U) % STP_SEQUENCE_LENGTH);
    }
    else
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    if (Local_enuErrorState == ES_OK)
    {
        STP_voidWriteCoils(STP_au8Sequence[STP_u8Index]);
    }

    return Local_enuErrorState;
}

ES_t STP_enuRotateSteps(u16 Copy_u16Steps, u8 Copy_u8Direction)
{
    ES_t Local_enuErrorState = ES_OK;
    u16  Local_u16Step;

    for (Local_u16Step = 0U; (Local_u16Step < Copy_u16Steps) && (Local_enuErrorState == ES_OK); Local_u16Step++)
    {
        Local_enuErrorState = STP_enuStep(Copy_u8Direction);
        DELAY_MS(STP_STEP_DELAY_MS);
    }

    return Local_enuErrorState;
}

ES_t STP_enuRelease(void)
{
    STP_voidWriteCoils(0U);
    return ES_OK;
}
