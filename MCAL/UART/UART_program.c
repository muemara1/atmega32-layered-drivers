/******************************************************************************
 * File        : UART_program.c
 * Layer       : MCAL
 * Description : USART driver - implementation (8 data bits).
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"

#include "UART_interface.h"
#include "UART_config.h"
#include "UART_private.h"

static void (*UART_pfRxCallback)(u8) = NULL;

ES_t UART_enuInit(void)
{
    u8  Local_u8Ucsrc = (u8)(1U << UCSRC_URSEL);    /* select UCSRC */
    u16 Local_u16Ubrr = UART_UBRR_VALUE;

    /* Asynchronous, 8 data bits */
    Local_u8Ucsrc |= (u8)((1U << UCSRC_UCSZ1) | (1U << UCSRC_UCSZ0));
    Local_u8Ucsrc |= (u8)((u8)UART_PARITY << UART_UPM_SHIFT);
    Local_u8Ucsrc |= (u8)((u8)UART_STOP_BITS << UCSRC_USBS);

    /* UBRRH shares the UCSRC address; URSEL = 0 selects UBRRH */
    UBRRH = (u8)((Local_u16Ubrr >> 8) & 0x0FU);
    UBRRL = (u8)(Local_u16Ubrr & 0xFFU);

    UCSRC = Local_u8Ucsrc;

    CLR_BIT(UCSRA, UCSRA_U2X);
    UCSRB = (u8)((1U << UCSRB_RXEN) | (1U << UCSRB_TXEN));

    return ES_OK;
}

ES_t UART_enuSendByte(u8 Copy_u8Data)
{
    ES_t Local_enuErrorState = ES_OK;
    u32  Local_u32Counter    = 0UL;

    while ((GET_BIT(UCSRA, UCSRA_UDRE) == 0U) && (Local_u32Counter < UART_TIMEOUT_COUNT))
    {
        Local_u32Counter++;
    }

    if (Local_u32Counter >= UART_TIMEOUT_COUNT)
    {
        Local_enuErrorState = ES_TIMEOUT;
    }
    else
    {
        UDR = Copy_u8Data;
    }

    return Local_enuErrorState;
}

ES_t UART_enuSendString(const char *Copy_pcString)
{
    ES_t Local_enuErrorState = ES_OK;
    u16  Local_u16Index      = 0U;

    if (Copy_pcString == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        while ((Copy_pcString[Local_u16Index] != '\0') && (Local_enuErrorState == ES_OK))
        {
            Local_enuErrorState = UART_enuSendByte((u8)Copy_pcString[Local_u16Index]);
            Local_u16Index++;
        }
    }

    return Local_enuErrorState;
}

ES_t UART_enuReceiveByte(u8 *Copy_pu8Data)
{
    ES_t Local_enuErrorState = ES_OK;
    u32  Local_u32Counter    = 0UL;

    if (Copy_pu8Data == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        while ((GET_BIT(UCSRA, UCSRA_RXC) == 0U) && (Local_u32Counter < UART_TIMEOUT_COUNT))
        {
            Local_u32Counter++;
        }

        if (Local_u32Counter >= UART_TIMEOUT_COUNT)
        {
            Local_enuErrorState = ES_TIMEOUT;
        }
        else
        {
            *Copy_pu8Data = UDR;
        }
    }

    return Local_enuErrorState;
}

ES_t UART_enuSetRxCallback(void (*Copy_pfCallback)(u8))
{
    ES_t Local_enuErrorState = ES_OK;

    if (Copy_pfCallback == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        UART_pfRxCallback = Copy_pfCallback;
        SET_BIT(UCSRB, UCSRB_RXCIE);
    }

    return Local_enuErrorState;
}

ISR_DEFINE(VECT_USART_RXC)
{
    u8 Local_u8Data = UDR;  /* reading UDR clears RXC */

    if (UART_pfRxCallback != NULL)
    {
        UART_pfRxCallback(Local_u8Data);
    }
}
