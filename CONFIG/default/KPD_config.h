/******************************************************************************
 * File        : KPD_config.h  (default)
 * Layer       : HAL configuration
 * Description : 4x4 keypad wiring and key map (row-major, as seen from front).
 *****************************************************************************/
#ifndef KPD_CONFIG_H_
#define KPD_CONFIG_H_

/* Rows (outputs) */
#define KPD_R0_PORT     DIO_PORTC
#define KPD_R0_PIN      DIO_PIN2
#define KPD_R1_PORT     DIO_PORTC
#define KPD_R1_PIN      DIO_PIN3
#define KPD_R2_PORT     DIO_PORTC
#define KPD_R2_PIN      DIO_PIN4
#define KPD_R3_PORT     DIO_PORTC
#define KPD_R3_PIN      DIO_PIN5

/* Columns (inputs with pull-up) */
#define KPD_C0_PORT     DIO_PORTD
#define KPD_C0_PIN      DIO_PIN4
#define KPD_C1_PORT     DIO_PORTD
#define KPD_C1_PIN      DIO_PIN5
#define KPD_C2_PORT     DIO_PORTD
#define KPD_C2_PIN      DIO_PIN6
#define KPD_C3_PORT     DIO_PORTD
#define KPD_C3_PIN      DIO_PIN7

#define KPD_KEY_MAP                     \
{                                       \
    { '7', '8', '9', '/' },             \
    { '4', '5', '6', '*' },             \
    { '1', '2', '3', '-' },             \
    { 'C', '0', '=', '+' }              \
}

#define KPD_DEBOUNCE_MS     20

#endif /* KPD_CONFIG_H_ */
