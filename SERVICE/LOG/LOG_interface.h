/******************************************************************************
 * File        : LOG_interface.h
 * Layer       : SERVICE
 * Description : Event logger: sends "[20yy-mm-dd hh:mm:ss] message" lines
 *               over UART, time-stamped by the DS1307 RTC.
 *               Requires UART_enuInit(), TWI_enuInitMaster(), RTC_enuInit().
 *****************************************************************************/
#ifndef LOG_INTERFACE_H_
#define LOG_INTERFACE_H_

#include "STD_TYPES.h"

/* Enables the serial console that lets you set the RTC from a terminal.
 * Send "TYYMMDDhhmmss" + Enter, e.g. "T261110230600" = 2026-11-10 23:06:00 */
ES_t LOG_enuInitConsole(void);

/* Handles a complete console line if one arrived; call it periodically */
void LOG_voidProcessConsole(void);

ES_t LOG_enuWrite(const char *Copy_pcMessage);

/* "message<value>", e.g. LOG_enuWriteValue("Free slots: ", 3) */
ES_t LOG_enuWriteValue(const char *Copy_pcMessage, s32 Copy_s32Value);

#endif /* LOG_INTERFACE_H_ */
