/******************************************************************************
 * File        : test_main.c
 * Description : Host unit tests. The drivers are compiled with -DHOST_TEST,
 *               which maps every register onto HOST_au8RegSpace[], so each
 *               test can preload "hardware" state, call the driver, and check
 *               the exact register values it produced. ISRs are plain
 *               functions here and are called directly to simulate interrupts.
 *
 *               Run with:  make test
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "ATMEGA32_REG.h"
#include "UTILS.h"

#include "DIO_interface.h"
#include "GIE_interface.h"
#include "EXTI_interface.h"
#include "ADC_interface.h"
#include "TIMER_interface.h"
#include "UART_interface.h"
#include "TWI_interface.h"

#include "LCD_interface.h"
#include "KPD_interface.h"
#include "SSD_interface.h"
#include "DCM_interface.h"
#include "STP_interface.h"
#include "LM35_interface.h"
#include "EEPROM_interface.h"
#include "RTC_interface.h"

#include "SCHED_interface.h"
#include "LOG_interface.h"

/* ISRs defined inside the drivers */
void VECT_INT1(void);
void VECT_TIMER2_COMP(void);
void VECT_USART_RXC(void);
void VECT_ADC(void);

/*------------------------------ Mini framework ----------------------------*/
static unsigned int TEST_uiChecks   = 0U;
static unsigned int TEST_uiFailures = 0U;

#define CHECK(COND)                                                         \
    do {                                                                    \
        TEST_uiChecks++;                                                    \
        if (!(COND)) {                                                      \
            TEST_uiFailures++;                                              \
            printf("\n    FAIL line %d: %s", __LINE__, #COND);              \
        }                                                                   \
    } while (0)

#define RUN(TEST_FN)                                                        \
    do {                                                                    \
        unsigned int Local_uiBefore = TEST_uiFailures;                      \
        TEST_voidResetRegisters();                                          \
        printf("  %-32s", #TEST_FN);                                        \
        TEST_FN();                                                          \
        printf("%s\n", (TEST_uiFailures == Local_uiBefore) ? "ok" : "\n  -> FAILED"); \
    } while (0)

static void TEST_voidResetRegisters(void)
{
    u16 Local_u16Index;

    for (Local_u16Index = 0U; Local_u16Index < REG_SPACE_SIZE; Local_u16Index++)
    {
        HOST_au8RegSpace[Local_u16Index] = 0U;
    }
}

/*=============================== LIB ======================================*/
static void test_bit_math(void)
{
    u8 Local_u8Reg = 0U;

    SET_BIT(Local_u8Reg, 7U);
    CHECK(Local_u8Reg == 0x80U);
    SET_BIT(Local_u8Reg, 0U);
    CHECK(Local_u8Reg == 0x81U);
    CLR_BIT(Local_u8Reg, 7U);
    CHECK(Local_u8Reg == 0x01U);
    TOG_BIT(Local_u8Reg, 1U);
    CHECK(Local_u8Reg == 0x03U);
    CHECK(GET_BIT(Local_u8Reg, 1U) == 1U);
    CHECK(GET_BIT(Local_u8Reg, 2U) == 0U);
    WRT_BIT(Local_u8Reg, 0U, 0U);
    CHECK(Local_u8Reg == 0x02U);
    WRT_BIT(Local_u8Reg, 4U, 1U);
    CHECK(Local_u8Reg == 0x12U);
    CHECK(CONC_BIT(1, 0, 1, 0, 0, 1, 0, 1) == 0xA5U);
}

static void test_utils(void)
{
    char Local_acBuf[12];
    u8   Local_au8A[4] = { 1U, 2U, 3U, 4U };
    u8   Local_au8B[4] = { 1U, 2U, 3U, 4U };

    CHECK(UTILS_u8U32ToString(0UL, Local_acBuf, 12U) == 1U);
    CHECK(strcmp(Local_acBuf, "0") == 0);
    CHECK(UTILS_u8U32ToString(12345UL, Local_acBuf, 12U) == 5U);
    CHECK(strcmp(Local_acBuf, "12345") == 0);
    CHECK(UTILS_u8U32ToString(4294967295UL, Local_acBuf, 12U) == 10U);
    CHECK(strcmp(Local_acBuf, "4294967295") == 0);
    CHECK(UTILS_u8U32ToString(12345UL, Local_acBuf, 5U) == 0U);      /* no room for NUL */
    CHECK(Local_acBuf[0] == '\0');
    CHECK(UTILS_u8U32ToString(1UL, NULL, 5U) == 0U);

    CHECK(UTILS_u8S32ToString(-42L, Local_acBuf, 12U) == 3U);
    CHECK(strcmp(Local_acBuf, "-42") == 0);
    CHECK(UTILS_u8S32ToString(-2147483647L - 1L, Local_acBuf, 12U) == 11U);
    CHECK(strcmp(Local_acBuf, "-2147483648") == 0);

    UTILS_voidTwoDigits(7U, Local_acBuf);
    CHECK((Local_acBuf[0] == '0') && (Local_acBuf[1] == '7'));
    UTILS_voidTwoDigits(150U, Local_acBuf);
    CHECK((Local_acBuf[0] == '9') && (Local_acBuf[1] == '9'));

    CHECK(UTILS_u8BytesEqual(Local_au8A, Local_au8B, 4U) == STD_TRUE);
    Local_au8B[3] = 5U;
    CHECK(UTILS_u8BytesEqual(Local_au8A, Local_au8B, 4U) == STD_FALSE);
    CHECK(UTILS_u8BytesEqual(NULL, Local_au8B, 4U) == STD_FALSE);

    CHECK(UTILS_u8BcdToBin(0x59U) == 59U);
    CHECK(UTILS_u8BinToBcd(47U) == 0x47U);
}

/*=============================== MCAL =====================================*/
static void test_dio(void)
{
    u8 Local_u8Value = 0xAAU;

    PORTA = 0xFFU;
    DDRA  = 0xFFU;
    CHECK(DIO_enuInit() == ES_OK);
    CHECK((PORTA == 0U) && (DDRA == 0U));       /* default config: inputs */
    CHECK(GET_BIT(MCUCSR, MCUCSR_JTD) == 1U);   /* JTAG off, PC2..PC5 usable */

    CHECK(DIO_enuSetPinDirection(DIO_PORTB, DIO_PIN3, DIO_OUTPUT) == ES_OK);
    CHECK(DDRB == 0x08U);
    CHECK(DIO_enuSetPinValue(DIO_PORTB, DIO_PIN3, DIO_HIGH) == ES_OK);
    CHECK(PORTB == 0x08U);
    CHECK(DIO_enuTogglePinValue(DIO_PORTB, DIO_PIN3) == ES_OK);
    CHECK(PORTB == 0x00U);

    PINC = 0x20U;
    CHECK(DIO_enuGetPinValue(DIO_PORTC, DIO_PIN5, &Local_u8Value) == ES_OK);
    CHECK(Local_u8Value == DIO_HIGH);
    CHECK(DIO_enuGetPinValue(DIO_PORTC, DIO_PIN4, &Local_u8Value) == ES_OK);
    CHECK(Local_u8Value == DIO_LOW);

    CHECK(DIO_enuSetPortDirection(DIO_PORTD, 0xF0U) == ES_OK);
    CHECK(DDRD == 0xF0U);
    CHECK(DIO_enuSetPortValue(DIO_PORTD, 0x5AU) == ES_OK);
    CHECK(PORTD == 0x5AU);
    PIND = 0xC3U;
    CHECK(DIO_enuGetPortValue(DIO_PORTD, &Local_u8Value) == ES_OK);
    CHECK(Local_u8Value == 0xC3U);

    /* Argument checking */
    CHECK(DIO_enuSetPinValue(4U, DIO_PIN0, DIO_HIGH) == ES_OUT_OF_RANGE);
    CHECK(DIO_enuSetPinValue(DIO_PORTA, 8U, DIO_HIGH) == ES_OUT_OF_RANGE);
    CHECK(DIO_enuSetPinValue(DIO_PORTA, DIO_PIN0, 2U) == ES_OUT_OF_RANGE);
    CHECK(DIO_enuSetPinDirection(DIO_PORTA, DIO_PIN0, 7U) == ES_OUT_OF_RANGE);
    CHECK(DIO_enuGetPinValue(DIO_PORTA, DIO_PIN0, NULL) == ES_NULL_POINTER);
    CHECK(DIO_enuGetPortValue(DIO_PORTA, NULL) == ES_NULL_POINTER);
    CHECK(DIO_enuSetPortValue(9U, 0U) == ES_OUT_OF_RANGE);
}

static void test_gie(void)
{
    u8 Local_u8Saved;

    GIE_voidEnable();
    CHECK(SREG == 0x80U);
    Local_u8Saved = GIE_u8EnterCritical();
    CHECK(Local_u8Saved == 0x80U);
    CHECK(SREG == 0x00U);
    GIE_voidRestore(Local_u8Saved);
    CHECK(SREG == 0x80U);
    GIE_voidDisable();
    CHECK(SREG == 0x00U);
}

static u8 TEST_u8Int1Count = 0U;
static void TEST_voidInt1Callback(void)
{
    TEST_u8Int1Count++;
}

static void test_exti(void)
{
    MCUCR = 0xF0U;  /* upper bits belong to sleep control: must be preserved */
    CHECK(EXTI_enuInit() == ES_OK);
    CHECK(MCUCR == 0xFAU);              /* INT0 and INT1 falling: 10 10 */
    CHECK((GICR & 0xE0U) == 0U);        /* default: all disabled */

    CHECK(EXTI_enuSetSenseControl(EXTI_INT0, EXTI_RISING_EDGE) == ES_OK);
    CHECK((MCUCR & 0x03U) == 0x03U);
    CHECK(EXTI_enuSetSenseControl(EXTI_INT1, EXTI_ANY_CHANGE) == ES_OK);
    CHECK((MCUCR & 0x0CU) == 0x04U);
    CHECK((MCUCR & 0xF0U) == 0xF0U);

    CHECK(EXTI_enuSetSenseControl(EXTI_INT2, EXTI_RISING_EDGE) == ES_OK);
    CHECK(GET_BIT(MCUCSR, MCUCSR_ISC2) == 1U);
    CHECK(EXTI_enuSetSenseControl(EXTI_INT2, EXTI_LOW_LEVEL) == ES_OUT_OF_RANGE);
    CHECK(EXTI_enuSetSenseControl(3U, EXTI_RISING_EDGE) == ES_OUT_OF_RANGE);

    CHECK(EXTI_enuEnable(EXTI_INT1) == ES_OK);
    CHECK(GET_BIT(GICR, GICR_INT1) == 1U);
    CHECK(EXTI_enuDisable(EXTI_INT1) == ES_OK);
    CHECK(GET_BIT(GICR, GICR_INT1) == 0U);

    CHECK(EXTI_enuSetCallback(EXTI_INT1, NULL) == ES_NULL_POINTER);
    CHECK(EXTI_enuSetCallback(EXTI_INT1, &TEST_voidInt1Callback) == ES_OK);
    VECT_INT1();
    VECT_INT1();
    CHECK(TEST_u8Int1Count == 2U);
}

static u16 TEST_u16AsyncResult = 0U;
static void TEST_voidAdcCallback(u16 Copy_u16Value)
{
    TEST_u16AsyncResult = Copy_u16Value;
}

static void test_adc(void)
{
    u16 Local_u16Value = 0U;

    CHECK(ADC_enuInit() == ES_OK);
    CHECK(ADMUX == 0x40U);              /* AVCC, right adjust, channel 0 */
    CHECK(ADCSRA == 0x86U);             /* ADEN, prescaler 64 */

    /* No conversion-complete flag: must time out, not hang */
    CHECK(ADC_enuReadChannelSync(ADC_CHANNEL5, &Local_u16Value) == ES_TIMEOUT);
    CHECK((ADMUX & 0x1FU) == 5U);
    CHECK((ADMUX & 0xE0U) == 0x40U);    /* reference bits preserved */

    /* "Hardware" finished a conversion with result 0x234 */
    SET_BIT(ADCSRA, ADCSRA_ADIF);
    ADCL = 0x34U;
    ADCH = 0x02U;
    CHECK(ADC_enuReadChannelSync(ADC_CHANNEL2, &Local_u16Value) == ES_OK);
    CHECK(Local_u16Value == 0x234U);

    CHECK(ADC_enuReadChannelSync(8U, &Local_u16Value) == ES_OUT_OF_RANGE);
    CHECK(ADC_enuReadChannelSync(ADC_CHANNEL0, NULL) == ES_NULL_POINTER);

    CHECK(ADC_u16RawToMilliVolt(0U) == 0U);
    CHECK(ADC_u16RawToMilliVolt(1023U) == 4995U);
    CHECK(ADC_u16RawToMilliVolt(512U) == 2500U);

    /* Asynchronous conversion completes in the ISR */
    CHECK(ADC_enuStartConversionAsync(ADC_CHANNEL1, &TEST_voidAdcCallback) == ES_OK);
    CHECK(GET_BIT(ADCSRA, ADCSRA_ADIE) == 1U);
    CHECK(ADC_enuStartConversionAsync(ADC_CHANNEL1, &TEST_voidAdcCallback) == ES_NOK);  /* busy */
    ADCL = 0xFFU;
    ADCH = 0x03U;
    VECT_ADC();
    CHECK(TEST_u16AsyncResult == 1023U);
    CHECK(GET_BIT(ADCSRA, ADCSRA_ADIE) == 0U);
    CHECK(ADC_enuStartConversionAsync(ADC_CHANNEL1, &TEST_voidAdcCallback) == ES_OK);  /* idle again */
    VECT_ADC();
}

static void test_timer0_pwm(void)
{
    CHECK(TIMER0_enuInit() == ES_OK);
    CHECK(TCCR0 == 0x48U);                          /* fast PWM, OC0 disconnected */
    CHECK(GET_BIT(DDRB, 3U) == 1U);                 /* OC0 = PB3 output */

    CHECK(TIMER0_enuStart() == ES_OK);
    CHECK((TCCR0 & 0x07U) == 0x02U);                /* clk/8 */

    CHECK(TIMER0_enuSetDutyCycle(50U) == ES_OK);
    CHECK(OCR0 == 127U);
    CHECK((TCCR0 & 0x30U) == 0x20U);                /* non-inverting */
    CHECK(TIMER0_enuSetDutyCycle(100U) == ES_OK);
    CHECK(OCR0 == 255U);
    CHECK(TIMER0_enuSetDutyCycle(0U) == ES_OK);
    CHECK((TCCR0 & 0x30U) == 0x00U);                /* disconnected: no glitch */
    CHECK(TIMER0_enuSetDutyCycle(101U) == ES_OUT_OF_RANGE);

    CHECK(TIMER0_enuStop() == ES_OK);
    CHECK((TCCR0 & 0x07U) == 0x00U);
}

static void test_timer2_tick(void)
{
    CHECK(TIMER2_enuInitTick() == ES_OK);
    CHECK(OCR2 == 124U);
    CHECK(TCCR2 == 0x08U);                          /* CTC, stopped */
    CHECK(GET_BIT(TIMSK, TIMSK_OCIE2) == 1U);
    CHECK(TIMER2_enuStart() == ES_OK);
    CHECK((TCCR2 & 0x07U) == 0x04U);                /* clk/64 -> 1 ms */
    CHECK(TIMER2_enuSetCallback(NULL) == ES_NULL_POINTER);
}

static u8 TEST_u8LastRx = 0U;
static void TEST_voidRxCallback(u8 Copy_u8Byte)
{
    TEST_u8LastRx = Copy_u8Byte;
}

static void test_uart(void)
{
    u8 Local_u8Byte = 0U;

    CHECK(UART_enuInit() == ES_OK);
    CHECK(UBRRL == 51U);                            /* 9600 baud at 8 MHz */
    CHECK(UCSRC == 0x86U);                          /* URSEL | 8N1 (written last) */
    CHECK(UCSRB == 0x18U);                          /* RXEN | TXEN */

    CHECK(UART_enuSendByte('A') == ES_TIMEOUT);     /* UDRE never set */
    SET_BIT(UCSRA, UCSRA_UDRE);
    CHECK(UART_enuSendByte('A') == ES_OK);
    CHECK(UDR == 'A');
    CHECK(UART_enuSendString("Hi") == ES_OK);
    CHECK(UDR == 'i');
    CHECK(UART_enuSendString(NULL) == ES_NULL_POINTER);

    CHECK(UART_enuReceiveByte(&Local_u8Byte) == ES_TIMEOUT);
    SET_BIT(UCSRA, UCSRA_RXC);
    UDR = 0x5AU;
    CHECK(UART_enuReceiveByte(&Local_u8Byte) == ES_OK);
    CHECK(Local_u8Byte == 0x5AU);

    CHECK(UART_enuSetRxCallback(&TEST_voidRxCallback) == ES_OK);
    CHECK(GET_BIT(UCSRB, UCSRB_RXCIE) == 1U);
    UDR = 'z';
    VECT_USART_RXC();
    CHECK(TEST_u8LastRx == 'z');
}

static void test_twi(void)
{
    CHECK(TWI_enuInitMaster() == ES_OK);
    CHECK(TWBR == 32U);                             /* 100 kHz at 8 MHz */

    TWSR = 0x00U;
    CHECK(TWI_enuStart() == ES_NOK);                /* unexpected status */
    TWSR = 0x08U;
    CHECK(TWI_enuStart() == ES_OK);
    CHECK((TWCR & 0xA4U) == 0xA4U);                 /* TWINT | TWSTA | TWEN */

    TWSR = 0x18U;
    CHECK(TWI_enuSendSlaveAddressWrite(0x68U) == ES_OK);
    CHECK(TWDR == 0xD0U);
    TWSR = 0x40U;
    CHECK(TWI_enuSendSlaveAddressRead(0x68U) == ES_OK);
    CHECK(TWDR == 0xD1U);
    CHECK(TWI_enuSendSlaveAddressWrite(0x80U) == ES_OUT_OF_RANGE);
    CHECK(TWI_enuReadDataNack(NULL) == ES_NULL_POINTER);

    TWI_voidStop();                                 /* bounded wait, returns */
    CHECK(GET_BIT(TWCR, TWCR_TWSTO) == 1U);
}

/*=============================== HAL ======================================*/
static void test_lcd(void)
{
    CHECK(LCD_enuInit() == ES_OK);
    CHECK((DDRB & 0xF3U) == 0xF3U);                 /* RS, EN, D4..D7 outputs */
    CHECK(GET_BIT(PORTB, 1U) == 0U);                /* EN idles low */

    CHECK(LCD_enuWriteChar('A') == ES_OK);          /* 0x41: last nibble 0x1 */
    CHECK(GET_BIT(PORTB, 0U) == 1U);                /* RS = data */
    CHECK((PORTB & 0xF0U) == 0x10U);

    CHECK(LCD_enuGoTo(1U, 15U) == ES_OK);           /* 0x80 | 0x4F: last nibble 0xF */
    CHECK(GET_BIT(PORTB, 0U) == 0U);                /* RS = command */
    CHECK((PORTB & 0xF0U) == 0xF0U);

    CHECK(LCD_enuGoTo(2U, 0U) == ES_OUT_OF_RANGE);
    CHECK(LCD_enuGoTo(0U, 16U) == ES_OUT_OF_RANGE);
    CHECK(LCD_enuWriteString(NULL) == ES_NULL_POINTER);
    CHECK(LCD_enuWriteLine(LCD_ROW0, "Hello") == ES_OK);
    CHECK(LCD_enuWriteNumber(-123L) == ES_OK);
    CHECK(LCD_enuCreateCustomChar(8U, (const u8 *)"12345678") == ES_OUT_OF_RANGE);
}

static void test_keypad(void)
{
    u8 Local_u8Key = 0U;

    CHECK(KPD_enuInit() == ES_OK);
    CHECK((DDRC & 0x3CU) == 0x3CU);                 /* rows PC2..PC5 outputs */
    CHECK((PORTC & 0x3CU) == 0x3CU);                /* rows idle high */
    CHECK((DDRD & 0xF0U) == 0x00U);                 /* cols PD4..PD7 inputs */
    CHECK((PORTD & 0xF0U) == 0xF0U);                /* with pull-ups */

    PIND = 0xFFU;                                   /* nothing pressed */
    CHECK(KPD_enuGetPressedKey(&Local_u8Key) == ES_OK);
    CHECK(Local_u8Key == KPD_NO_KEY);

    PIND = 0xEFU;                                   /* column 0 low */
    CHECK(KPD_enuGetPressedKey(&Local_u8Key) == ES_OK);
    CHECK(Local_u8Key == '7');                      /* row 0 is scanned first */
    CHECK(KPD_enuGetPressedKey(&Local_u8Key) == ES_OK);
    CHECK(Local_u8Key == KPD_NO_KEY);               /* still held: no repeat */

    PIND = 0xFFU;                                   /* released */
    CHECK(KPD_enuGetPressedKey(&Local_u8Key) == ES_OK);
    CHECK(Local_u8Key == KPD_NO_KEY);

    PIND = 0x7FU;                                   /* column 3 low */
    CHECK(KPD_enuGetPressedKey(&Local_u8Key) == ES_OK);
    CHECK(Local_u8Key == '/');
    CHECK(KPD_enuGetPressedKey(NULL) == ES_NULL_POINTER);
}

static void test_ssd(void)
{
    CHECK(SSD_enuInit() == ES_OK);
    CHECK((DDRA & 0xFEU) == 0xFEU);                 /* PA1..PA7 outputs */
    CHECK(GET_BIT(DDRA, 0U) == 0U);                 /* PA0 left for the ADC */
    CHECK((PORTA & 0xFEU) == 0x00U);                /* blank */

    CHECK(SSD_enuDisplayDigit(3U) == ES_OK);        /* 0x4F shifted to PA1 */
    CHECK(PORTA == 0x9EU);
    CHECK(SSD_enuDisplayDigit(8U) == ES_OK);
    CHECK(PORTA == 0xFEU);
    CHECK(SSD_enuDisplayDigit(10U) == ES_OUT_OF_RANGE);
    CHECK(SSD_enuDisplayPattern(SSD_SYMBOL_F) == ES_OK);
    CHECK(PORTA == 0xE2U);
}

static void test_dc_motor(void)
{
    u8 Local_u8Direction = 0xFFU;

    CHECK(DCM_enuInit() == ES_OK);
    CHECK((DDRD & 0x30U) == 0x30U);                 /* IN1, IN2 outputs */
    CHECK((PORTD & 0x30U) == 0x00U);

    CHECK(DCM_enuSetSpeed(60U) == ES_OK);
    CHECK(OCR0 == 153U);
    CHECK((PORTD & 0x30U) == 0x10U);                /* CW: IN1 high */

    CHECK(DCM_enuToggleDirection() == ES_OK);
    CHECK((PORTD & 0x30U) == 0x20U);                /* CCW: IN2 high */
    CHECK(DCM_enuGetDirection(&Local_u8Direction) == ES_OK);
    CHECK(Local_u8Direction == DCM_DIR_CCW);

    CHECK(DCM_enuStop() == ES_OK);
    CHECK((PORTD & 0x30U) == 0x00U);
    CHECK((TCCR0 & 0x30U) == 0x00U);
    CHECK(DCM_enuSetSpeed(120U) == ES_OUT_OF_RANGE);
    CHECK(DCM_enuSetDirection(5U) == ES_OUT_OF_RANGE);
}

static void test_stepper(void)
{
    CHECK(STP_enuInit() == ES_OK);
    CHECK((DDRC & 0x3CU) == 0x3CU);
    CHECK((PORTC & 0x3CU) == 0x00U);

    CHECK(STP_enuStep(STP_DIR_CW) == ES_OK);        /* index 1: A+B */
    CHECK((PORTC & 0x3CU) == (0x03U << 2));
    CHECK(STP_enuStep(STP_DIR_CCW) == ES_OK);       /* index 0: A */
    CHECK((PORTC & 0x3CU) == (0x01U << 2));
    CHECK(STP_enuStep(STP_DIR_CCW) == ES_OK);       /* index 7: A+D (wraps) */
    CHECK((PORTC & 0x3CU) == (0x09U << 2));
    CHECK(STP_enuRotateSteps(8U, STP_DIR_CW) == ES_OK);   /* full cycle */
    CHECK((PORTC & 0x3CU) == (0x09U << 2));
    CHECK(STP_enuStep(2U) == ES_OUT_OF_RANGE);
    CHECK(STP_enuRelease() == ES_OK);
    CHECK((PORTC & 0x3CU) == 0x00U);
}

static void test_lm35(void)
{
    u16 Local_u16Tenths = 0U;
    u8  Local_u8Celsius = 0U;

    CHECK(ADC_enuInit() == ES_OK);
    CHECK(LM35_enuReadTenths(&Local_u16Tenths) == ES_TIMEOUT);

    /* 102 counts * 5000 mV / 1024 = 498 mV = 49.8 degC */
    SET_BIT(ADCSRA, ADCSRA_ADIF);
    ADCL = 102U;
    ADCH = 0U;
    CHECK(LM35_enuReadTenths(&Local_u16Tenths) == ES_OK);
    CHECK(Local_u16Tenths == 498U);
    CHECK((ADMUX & 0x1FU) == 1U);                   /* channel from config */
    CHECK(LM35_enuReadCelsius(&Local_u8Celsius) == ES_OK);
    CHECK(Local_u8Celsius == 50U);
    CHECK(LM35_enuReadCelsius(NULL) == ES_NULL_POINTER);
}

static void test_eeprom_rtc_errors(void)
{
    u8         Local_u8Byte = 0U;
    u8         Local_au8Buf[4];
    RTC_Time_t Local_strTime = { 0U, 0U, 24U, 1U, 1U, 1U, 26U };

    CHECK(TWI_enuInitMaster() == ES_OK);

    /* Argument checks never touch the bus */
    CHECK(EEPROM_enuWriteByte(2048U, 0U) == ES_OUT_OF_RANGE);
    CHECK(EEPROM_enuReadByte(0U, NULL) == ES_NULL_POINTER);
    CHECK(EEPROM_enuReadBlock(2046U, Local_au8Buf, 4U) == ES_OUT_OF_RANGE);

    /* No device answering (status register never matches): error, no hang */
    CHECK(EEPROM_enuReadByte(0x123U, &Local_u8Byte) == ES_NOK);
    CHECK(RTC_enuGetTime(&Local_strTime) == ES_NOK);

    CHECK(RTC_enuSetTime(&Local_strTime) == ES_OUT_OF_RANGE);   /* hour 24 */
    Local_strTime.Hours = 23U;
    Local_strTime.Month = 13U;
    CHECK(RTC_enuSetTime(&Local_strTime) == ES_OUT_OF_RANGE);
    CHECK(RTC_enuSetTime(NULL) == ES_NULL_POINTER);
}

/*=============================== SERVICE ==================================*/
static u16 TEST_u16FastRuns = 0U;
static u16 TEST_u16SlowRuns = 0U;
static void TEST_voidFastTask(void) { TEST_u16FastRuns++; }
static void TEST_voidSlowTask(void) { TEST_u16SlowRuns++; }

static void test_scheduler(void)
{
    u16 Local_u16Tick;

    CHECK(SCHED_enuInit() == ES_OK);
    CHECK(SCHED_enuAddTask(NULL, 10U, 0U) == ES_NULL_POINTER);
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 0U, 0U) == ES_OUT_OF_RANGE);
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 10U, 0U) == ES_OK);
    CHECK(SCHED_enuAddTask(&TEST_voidSlowTask, 25U, 5U) == ES_OK);
    CHECK(SCHED_enuStart() == ES_OK);

    /* 100 ms, dispatching after every tick like the main loop would */
    for (Local_u16Tick = 0U; Local_u16Tick < 100U; Local_u16Tick++)
    {
        VECT_TIMER2_COMP();
        SCHED_voidDispatch();
    }
    CHECK(SCHED_u32GetTicks() == 100UL);
    CHECK(TEST_u16FastRuns == 10U);     /* ticks 1, 11, ..., 91 */
    CHECK(TEST_u16SlowRuns == 4U);      /* ticks 6, 31, 56, 81 */

    /* A 60 ms stall: missed periods are dropped, not replayed in a burst */
    TEST_u16FastRuns = 0U;
    for (Local_u16Tick = 0U; Local_u16Tick < 60U; Local_u16Tick++)
    {
        VECT_TIMER2_COMP();
    }
    SCHED_voidDispatch();
    CHECK(TEST_u16FastRuns == 1U);

    /* Capacity limit */
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 1U, 0U) == ES_OK);
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 1U, 0U) == ES_OK);
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 1U, 0U) == ES_OK);
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 1U, 0U) == ES_OK);
    CHECK(SCHED_enuAddTask(&TEST_voidFastTask, 1U, 0U) == ES_OUT_OF_RANGE);
}

static void TEST_voidFeedConsole(const char *Copy_pcText)
{
    while (*Copy_pcText != '\0')
    {
        UDR = (u8)*Copy_pcText;
        VECT_USART_RXC();
        Copy_pcText++;
    }
}

static void test_log(void)
{
    CHECK(UART_enuInit() == ES_OK);
    CHECK(TWI_enuInitMaster() == ES_OK);
    SET_BIT(UCSRA, UCSRA_UDRE);

    CHECK(LOG_enuWrite(NULL) == ES_NULL_POINTER);
    CHECK(LOG_enuWrite("event") == ES_OK);          /* RTC absent: dashed stamp */
    CHECK(UDR == '\n');
    CHECK(LOG_enuWriteValue("count=", -5L) == ES_OK);

    /* Console: a full line is processed once, then the buffer is reusable.
     * The RTC write fails on the host, so the reply ends in "...hhmmss\r\n". */
    CHECK(LOG_enuInitConsole() == ES_OK);
    TEST_voidFeedConsole("T261110230600\r");
    LOG_voidProcessConsole();
    CHECK(UDR == '\n');
    UDR = 0U;
    LOG_voidProcessConsole();                       /* nothing pending */
    CHECK(UDR == 0U);
    TEST_voidFeedConsole("hello\n");
    LOG_voidProcessConsole();
    CHECK(UDR == '\n');
}

/*=============================== main =====================================*/
int main(void)
{
    printf("ATmega32 layered drivers - host unit tests\n\n");

    printf("LIB\n");
    RUN(test_bit_math);
    RUN(test_utils);

    printf("MCAL\n");
    RUN(test_dio);
    RUN(test_gie);
    RUN(test_exti);
    RUN(test_adc);
    RUN(test_timer0_pwm);
    RUN(test_timer2_tick);
    RUN(test_uart);
    RUN(test_twi);

    printf("HAL\n");
    RUN(test_lcd);
    RUN(test_keypad);
    RUN(test_ssd);
    RUN(test_dc_motor);
    RUN(test_stepper);
    RUN(test_lm35);
    RUN(test_eeprom_rtc_errors);

    printf("SERVICE\n");
    RUN(test_scheduler);
    RUN(test_log);

    printf("\n%u checks, %u failed\n", TEST_uiChecks, TEST_uiFailures);

    return (TEST_uiFailures == 0U) ? 0 : 1;
}
