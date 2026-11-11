/******************************************************************************
 * File        : ATMEGA32_REG.h
 * Layer       : LIB
 * Description : Memory-mapped register map of the ATmega32 (data-space
 *               addresses = I/O address + 0x20) and the ISR definition macro.
 *
 *               When compiled with -DHOST_TEST the registers are mapped onto
 *               a RAM array so the drivers can be unit-tested on a PC.
 *****************************************************************************/
#ifndef ATMEGA32_REG_H_
#define ATMEGA32_REG_H_

#include "STD_TYPES.h"

#ifdef HOST_TEST
    #define REG_SPACE_SIZE  (0x60U)
    extern volatile u8 HOST_au8RegSpace[REG_SPACE_SIZE];
    #define REG8(ADDR)      (HOST_au8RegSpace[(ADDR)])
#else
    #define REG8(ADDR)      (*(volatile u8 *)(ADDR))
#endif

/*----------------------------- DIO ----------------------------------------*/
#define PORTA   REG8(0x3BU)
#define DDRA    REG8(0x3AU)
#define PINA    REG8(0x39U)

#define PORTB   REG8(0x38U)
#define DDRB    REG8(0x37U)
#define PINB    REG8(0x36U)

#define PORTC   REG8(0x35U)
#define DDRC    REG8(0x34U)
#define PINC    REG8(0x33U)

#define PORTD   REG8(0x32U)
#define DDRD    REG8(0x31U)
#define PIND    REG8(0x30U)

/*----------------------------- Status / interrupts ------------------------*/
#define SREG    REG8(0x5FU)
#define SREG_I      7U

#define GICR    REG8(0x5BU)
#define GICR_INT1   7U
#define GICR_INT0   6U
#define GICR_INT2   5U

#define GIFR    REG8(0x5AU)
#define GIFR_INTF1  7U
#define GIFR_INTF0  6U
#define GIFR_INTF2  5U

#define MCUCR   REG8(0x55U)
#define MCUCR_ISC11 3U
#define MCUCR_ISC10 2U
#define MCUCR_ISC01 1U
#define MCUCR_ISC00 0U

#define MCUCSR  REG8(0x54U)
#define MCUCSR_JTD  7U
#define MCUCSR_ISC2 6U

#define SFIOR   REG8(0x50U)

/*----------------------------- ADC ----------------------------------------*/
#define ADMUX   REG8(0x27U)
#define ADMUX_REFS1 7U
#define ADMUX_REFS0 6U
#define ADMUX_ADLAR 5U

#define ADCSRA  REG8(0x26U)
#define ADCSRA_ADEN  7U
#define ADCSRA_ADSC  6U
#define ADCSRA_ADATE 5U
#define ADCSRA_ADIF  4U
#define ADCSRA_ADIE  3U

#define ADCH    REG8(0x25U)
#define ADCL    REG8(0x24U)

/*----------------------------- Timers -------------------------------------*/
#define TCCR0   REG8(0x53U)
#define TCCR0_FOC0  7U
#define TCCR0_WGM00 6U
#define TCCR0_COM01 5U
#define TCCR0_COM00 4U
#define TCCR0_WGM01 3U

#define TCNT0   REG8(0x52U)
#define OCR0    REG8(0x5CU)

#define TCCR2   REG8(0x45U)
#define TCCR2_FOC2  7U
#define TCCR2_WGM20 6U
#define TCCR2_COM21 5U
#define TCCR2_COM20 4U
#define TCCR2_WGM21 3U

#define TCNT2   REG8(0x44U)
#define OCR2    REG8(0x43U)

#define TIMSK   REG8(0x59U)
#define TIMSK_OCIE2 7U
#define TIMSK_TOIE2 6U
#define TIMSK_OCIE0 1U
#define TIMSK_TOIE0 0U

#define TIFR    REG8(0x58U)

/* Clock-select field occupies bits 2..0 of TCCR0 and TCCR2 */
#define TIMER_CS_MASK   ((u8)0xF8U)

/*----------------------------- USART --------------------------------------*/
#define UDR     REG8(0x2CU)

#define UCSRA   REG8(0x2BU)
#define UCSRA_RXC   7U
#define UCSRA_TXC   6U
#define UCSRA_UDRE  5U
#define UCSRA_U2X   1U

#define UCSRB   REG8(0x2AU)
#define UCSRB_RXCIE 7U
#define UCSRB_RXEN  4U
#define UCSRB_TXEN  3U
#define UCSRB_UCSZ2 2U

#define UBRRL   REG8(0x29U)
#define UCSRC   REG8(0x40U)     /* shares its address with UBRRH */
#define UBRRH   REG8(0x40U)
#define UCSRC_URSEL 7U
#define UCSRC_UMSEL 6U
#define UCSRC_UPM1  5U
#define UCSRC_UPM0  4U
#define UCSRC_USBS  3U
#define UCSRC_UCSZ1 2U
#define UCSRC_UCSZ0 1U

/*----------------------------- TWI (I2C) ----------------------------------*/
#define TWBR    REG8(0x20U)
#define TWSR    REG8(0x21U)
#define TWAR    REG8(0x22U)
#define TWDR    REG8(0x23U)

#define TWCR    REG8(0x56U)
#define TWCR_TWINT  7U
#define TWCR_TWEA   6U
#define TWCR_TWSTA  5U
#define TWCR_TWSTO  4U
#define TWCR_TWEN   2U
#define TWCR_TWIE   0U

/*----------------------------- Interrupt vectors --------------------------*/
#define VECT_INT0           __vector_1
#define VECT_INT1           __vector_2
#define VECT_INT2           __vector_3
#define VECT_TIMER2_COMP    __vector_4
#define VECT_TIMER2_OVF     __vector_5
#define VECT_TIMER0_COMP    __vector_10
#define VECT_TIMER0_OVF     __vector_11
#define VECT_USART_RXC      __vector_13
#define VECT_ADC            __vector_16

/* Defines an interrupt service routine for the given vector name.
 * On the target, the 'signal' attribute makes avr-gcc save the context
 * and return with RETI. On the host it is a plain function so tests can
 * call it to simulate the interrupt. */
#ifdef HOST_TEST
    #define ISR_DEFINE(VECT)    void VECT(void); void VECT(void)
#else
    #define ISR_DEFINE(VECT)    void VECT(void) __attribute__((signal, used, externally_visible)); \
                                void VECT(void)
#endif

#endif /* ATMEGA32_REG_H_ */
