/******************************************************************************
 * File        : EXTI_program.c
 * Layer       : MCAL
 * Description : External interrupts - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "EXTI_interface.h"
#include "EXTI_private.h"
#include "EXTI_config.h"

static void (*EXTI_apfCallback[EXTI_COUNT])(void) = { NULL, NULL, NULL };

ES_t EXTI_enuInit(void)
{
    ES_t Local_enuErrorState;

    Local_enuErrorState = EXTI_enuSetSenseControl(EXTI_INT0, EXTI_INT0_SENSE);
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = EXTI_enuSetSenseControl(EXTI_INT1, EXTI_INT1_SENSE);
    }
    if (Local_enuErrorState == ES_OK)
    {
        Local_enuErrorState = EXTI_enuSetSenseControl(EXTI_INT2, EXTI_INT2_SENSE);
    }

    /* Clear stale flags (write 1 to clear), then apply enable state */
    GIFR = (u8)((1U << GIFR_INTF0) | (1U << GIFR_INTF1) | (1U << GIFR_INTF2));
    WRT_BIT(GICR, GICR_INT0, EXTI_INT0_STATE);
    WRT_BIT(GICR, GICR_INT1, EXTI_INT1_STATE);
    WRT_BIT(GICR, GICR_INT2, EXTI_INT2_STATE);

    return Local_enuErrorState;
}

ES_t EXTI_enuSetSenseControl(u8 Copy_u8IntId, u8 Copy_u8Sense)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Sense > EXTI_RISING_EDGE)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        switch (Copy_u8IntId)
        {
            case EXTI_INT0:
                MCUCR = (u8)((MCUCR & EXTI_INT0_ISC_MASK) | Copy_u8Sense);
                break;

            case EXTI_INT1:
                MCUCR = (u8)((MCUCR & EXTI_INT1_ISC_MASK) | (u8)(Copy_u8Sense << EXTI_INT1_ISC_SHIFT));
                break;

            case EXTI_INT2:
                /* INT2 is edge-only. Disable it while changing ISC2, as the
                 * datasheet recommends, then clear the flag that may be set. */
                if (Copy_u8Sense == EXTI_FALLING_EDGE)
                {
                    u8 Local_u8WasEnabled = GET_BIT(GICR, GICR_INT2);
                    CLR_BIT(GICR, GICR_INT2);
                    CLR_BIT(MCUCSR, MCUCSR_ISC2);
                    GIFR = (u8)(1U << GIFR_INTF2);
                    WRT_BIT(GICR, GICR_INT2, Local_u8WasEnabled);
                }
                else if (Copy_u8Sense == EXTI_RISING_EDGE)
                {
                    u8 Local_u8WasEnabled = GET_BIT(GICR, GICR_INT2);
                    CLR_BIT(GICR, GICR_INT2);
                    SET_BIT(MCUCSR, MCUCSR_ISC2);
                    GIFR = (u8)(1U << GIFR_INTF2);
                    WRT_BIT(GICR, GICR_INT2, Local_u8WasEnabled);
                }
                else
                {
                    Local_enuErrorState = ES_OUT_OF_RANGE;
                }
                break;

            default:
                Local_enuErrorState = ES_OUT_OF_RANGE;
                break;
        }
    }

    return Local_enuErrorState;
}

ES_t EXTI_enuEnable(u8 Copy_u8IntId)
{
    ES_t Local_enuErrorState = ES_OK;

    switch (Copy_u8IntId)
    {
        case EXTI_INT0: SET_BIT(GICR, GICR_INT0); break;
        case EXTI_INT1: SET_BIT(GICR, GICR_INT1); break;
        case EXTI_INT2: SET_BIT(GICR, GICR_INT2); break;
        default:        Local_enuErrorState = ES_OUT_OF_RANGE; break;
    }

    return Local_enuErrorState;
}

ES_t EXTI_enuDisable(u8 Copy_u8IntId)
{
    ES_t Local_enuErrorState = ES_OK;

    switch (Copy_u8IntId)
    {
        case EXTI_INT0: CLR_BIT(GICR, GICR_INT0); break;
        case EXTI_INT1: CLR_BIT(GICR, GICR_INT1); break;
        case EXTI_INT2: CLR_BIT(GICR, GICR_INT2); break;
        default:        Local_enuErrorState = ES_OUT_OF_RANGE; break;
    }

    return Local_enuErrorState;
}

ES_t EXTI_enuSetCallback(u8 Copy_u8IntId, void (*Copy_pfCallback)(void))
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pfCallback == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u8IntId < EXTI_COUNT)
    {
        EXTI_apfCallback[Copy_u8IntId] = Copy_pfCallback;
    }
    else
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    return Local_enuErrorState;
}

/*----------------------------- ISRs ---------------------------------------*/
ISR_DEFINE(VECT_INT0)
{
    if (EXTI_apfCallback[EXTI_INT0] != NULL)
    {
        EXTI_apfCallback[EXTI_INT0]();
    }
}

ISR_DEFINE(VECT_INT1)
{
    if (EXTI_apfCallback[EXTI_INT1] != NULL)
    {
        EXTI_apfCallback[EXTI_INT1]();
    }
}

ISR_DEFINE(VECT_INT2)
{
    if (EXTI_apfCallback[EXTI_INT2] != NULL)
    {
        EXTI_apfCallback[EXTI_INT2]();
    }
}
