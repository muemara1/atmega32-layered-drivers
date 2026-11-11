/******************************************************************************
 * File        : DIO_program.c
 * Layer       : MCAL
 * Description : Digital input/output driver - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "DIO_interface.h"
#include "DIO_private.h"
#include "DIO_config.h"

/* Register look-up tables, indexed by DIO_PORTx */
static volatile u8 * const DIO_apu8DdrReg[DIO_PORT_COUNT]  = { &DDRA,  &DDRB,  &DDRC,  &DDRD  };
static volatile u8 * const DIO_apu8PortReg[DIO_PORT_COUNT] = { &PORTA, &PORTB, &PORTC, &PORTD };
static volatile u8 * const DIO_apu8PinReg[DIO_PORT_COUNT]  = { &PINA,  &PINB,  &PINC,  &PIND  };

static ES_t DIO_enuCheckPortPin(u8 Copy_u8Port, u8 Copy_u8Pin)
{
    ES_t Local_enuErrorState = ES_OK;

    if ((Copy_u8Port >= DIO_PORT_COUNT) || (Copy_u8Pin >= DIO_PIN_COUNT))
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    return Local_enuErrorState;
}

ES_t DIO_enuInit(void)
{
    /* The JTAGEN fuse is programmed at the factory, which takes PC2..PC5
     * away from the port (keypad rows, stepper, smart-home outputs all live
     * there). JTD must be written twice within four cycles to take effect;
     * two back-to-back volatile stores compile to two consecutive OUTs. */
    u8 Local_u8Mcucsr = (u8)(MCUCSR | (u8)(1U << MCUCSR_JTD));
    MCUCSR = Local_u8Mcucsr;
    MCUCSR = Local_u8Mcucsr;

    /* Write PORT before DDR so outputs come up at their configured level */
    PORTA = DIO_PORTA_INIT_VALUE;
    DDRA  = DIO_PORTA_DIRECTION;

    PORTB = DIO_PORTB_INIT_VALUE;
    DDRB  = DIO_PORTB_DIRECTION;

    PORTC = DIO_PORTC_INIT_VALUE;
    DDRC  = DIO_PORTC_DIRECTION;

    PORTD = DIO_PORTD_INIT_VALUE;
    DDRD  = DIO_PORTD_DIRECTION;

    return ES_OK;
}

ES_t DIO_enuSetPinDirection(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Direction)
{
    ES_t Local_enuErrorState = DIO_enuCheckPortPin(Copy_u8Port, Copy_u8Pin);

    if (Local_enuErrorState == ES_OK)
    {
        if (Copy_u8Direction == DIO_OUTPUT)
        {
            SET_BIT(*DIO_apu8DdrReg[Copy_u8Port], Copy_u8Pin);
        }
        else if (Copy_u8Direction == DIO_INPUT)
        {
            CLR_BIT(*DIO_apu8DdrReg[Copy_u8Port], Copy_u8Pin);
        }
        else
        {
            Local_enuErrorState = ES_OUT_OF_RANGE;
        }
    }

    return Local_enuErrorState;
}

ES_t DIO_enuSetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Value)
{
    ES_t Local_enuErrorState = DIO_enuCheckPortPin(Copy_u8Port, Copy_u8Pin);

    if (Local_enuErrorState == ES_OK)
    {
        if (Copy_u8Value == DIO_HIGH)
        {
            SET_BIT(*DIO_apu8PortReg[Copy_u8Port], Copy_u8Pin);
        }
        else if (Copy_u8Value == DIO_LOW)
        {
            CLR_BIT(*DIO_apu8PortReg[Copy_u8Port], Copy_u8Pin);
        }
        else
        {
            Local_enuErrorState = ES_OUT_OF_RANGE;
        }
    }

    return Local_enuErrorState;
}

ES_t DIO_enuGetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin, u8 *Copy_pu8Value)
{
    ES_t Local_enuErrorState = DIO_enuCheckPortPin(Copy_u8Port, Copy_u8Pin);

    if (Copy_pu8Value == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Local_enuErrorState == ES_OK)
    {
        *Copy_pu8Value = GET_BIT(*DIO_apu8PinReg[Copy_u8Port], Copy_u8Pin);
    }
    else
    {
        /* Range error already recorded */
    }

    return Local_enuErrorState;
}

ES_t DIO_enuTogglePinValue(u8 Copy_u8Port, u8 Copy_u8Pin)
{
    ES_t Local_enuErrorState = DIO_enuCheckPortPin(Copy_u8Port, Copy_u8Pin);

    if (Local_enuErrorState == ES_OK)
    {
        TOG_BIT(*DIO_apu8PortReg[Copy_u8Port], Copy_u8Pin);
    }

    return Local_enuErrorState;
}

ES_t DIO_enuSetPortDirection(u8 Copy_u8Port, u8 Copy_u8Direction)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Port < DIO_PORT_COUNT)
    {
        *DIO_apu8DdrReg[Copy_u8Port] = Copy_u8Direction;
    }
    else
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    return Local_enuErrorState;
}

ES_t DIO_enuSetPortValue(u8 Copy_u8Port, u8 Copy_u8Value)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_u8Port < DIO_PORT_COUNT)
    {
        *DIO_apu8PortReg[Copy_u8Port] = Copy_u8Value;
    }
    else
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    return Local_enuErrorState;
}

ES_t DIO_enuGetPortValue(u8 Copy_u8Port, u8 *Copy_pu8Value)
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pu8Value == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u8Port < DIO_PORT_COUNT)
    {
        *Copy_pu8Value = *DIO_apu8PinReg[Copy_u8Port];
    }
    else
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }

    return Local_enuErrorState;
}
