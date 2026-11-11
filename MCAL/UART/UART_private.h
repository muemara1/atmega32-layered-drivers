/******************************************************************************
 * File        : UART_private.h
 * Layer       : MCAL
 *****************************************************************************/
#ifndef UART_PRIVATE_H_
#define UART_PRIVATE_H_

#ifndef F_CPU
    #error "F_CPU must be defined (see Makefile)"
#endif

/* Rounded UBRR for normal speed (U2X = 0) */
#define UART_UBRR_VALUE \
    ((u16)((((u32)F_CPU + (8UL * (u32)UART_BAUD_RATE)) / (16UL * (u32)UART_BAUD_RATE)) - 1UL))

#define UART_TIMEOUT_COUNT      ((u32)60000UL)
#define UART_UPM_SHIFT          (4U)

#endif /* UART_PRIVATE_H_ */
