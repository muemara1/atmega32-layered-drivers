/******************************************************************************
 * File        : DELAY.h
 * Layer       : LIB
 * Description : Busy-wait delays. Uses avr-libc on the target and compiles
 *               to nothing on the host so unit tests run instantly.
 *               Always pass compile-time constants (avr-libc requirement).
 *****************************************************************************/
#ifndef DELAY_H_
#define DELAY_H_

#ifdef HOST_TEST
    #define DELAY_MS(MS)    ((void)(MS))
    #define DELAY_US(US)    ((void)(US))
#else
    #ifndef F_CPU
    #define F_CPU 8000000UL
    #endif
    #include <util/delay.h>
    #define DELAY_MS(MS)    _delay_ms(MS)
    #define DELAY_US(US)    _delay_us(US)
#endif

#endif /* DELAY_H_ */
