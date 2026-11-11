/******************************************************************************
 * File        : UART_interface.h
 * Layer       : MCAL
 * Description : USART in asynchronous mode, RXD = PD0, TXD = PD1.
 *****************************************************************************/
#ifndef UART_INTERFACE_H_
#define UART_INTERFACE_H_

#include "STD_TYPES.h"

/* Parity options (UART_config.h) */
#define UART_PARITY_NONE    0U
#define UART_PARITY_EVEN    2U
#define UART_PARITY_ODD     3U

/* Stop bit options (UART_config.h) */
#define UART_STOP_1BIT      0U
#define UART_STOP_2BIT      1U

ES_t UART_enuInit(void);

ES_t UART_enuSendByte(u8 Copy_u8Data);
ES_t UART_enuSendString(const char *Copy_pcString);

/* Blocking receive with timeout (ES_TIMEOUT if nothing arrives) */
ES_t UART_enuReceiveByte(u8 *Copy_pu8Data);

/* Interrupt-driven receive: callback runs in ISR context for every byte */
ES_t UART_enuSetRxCallback(void (*Copy_pfCallback)(u8));

#endif /* UART_INTERFACE_H_ */
