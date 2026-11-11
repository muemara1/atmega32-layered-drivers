/******************************************************************************
 * File        : RTC_interface.h
 * Layer       : HAL
 * Description : DS1307 real-time clock on I2C (address 0x68), 24-hour mode.
 *               Call TWI_enuInitMaster() before using this driver.
 *****************************************************************************/
#ifndef RTC_INTERFACE_H_
#define RTC_INTERFACE_H_

#include "STD_TYPES.h"

typedef struct
{
    u8 Seconds;     /* 0..59 */
    u8 Minutes;     /* 0..59 */
    u8 Hours;       /* 0..23 */
    u8 Day;         /* 1..7, day of week */
    u8 Date;        /* 1..31 */
    u8 Month;       /* 1..12 */
    u8 Year;        /* 0..99 -> 2000..2099 */
} RTC_Time_t;

/* Starts the oscillator if it is halted (keeps the stored time) */
ES_t RTC_enuInit(void);

ES_t RTC_enuSetTime(const RTC_Time_t *Copy_pstrTime);
ES_t RTC_enuGetTime(RTC_Time_t *Copy_pstrTime);

#endif /* RTC_INTERFACE_H_ */
