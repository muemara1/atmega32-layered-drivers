/******************************************************************************
 * File        : LOG_program.c
 * Layer       : SERVICE
 * Description : UART event logger with RTC time stamps - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "UTILS.h"

#include "UART_interface.h"
#include "RTC_interface.h"

#include "LOG_interface.h"

/* "[2026-11-10 23:06:59] " = 22 characters + NUL */
#define LOG_STAMP_LENGTH    ((u8)23U)
#define LOG_VALUE_LENGTH    ((u8)12U)

/* Console: "T" + 12 digits */
#define LOG_LINE_SIZE       ((u8)16U)
#define LOG_TIME_CMD_LENGTH ((u8)13U)

static volatile char LOG_acLine[LOG_LINE_SIZE];
static volatile u8   LOG_u8LineLength = 0U;
static volatile u8   LOG_u8LineReady  = STD_FALSE;

/* UART RX callback (ISR context): collects one line at a time */
static void LOG_voidRxHandler(u8 Copy_u8Byte)
{
    if (LOG_u8LineReady == STD_FALSE)
    {
        if ((Copy_u8Byte == (u8)'\r') || (Copy_u8Byte == (u8)'\n'))
        {
            if (LOG_u8LineLength > 0U)
            {
                LOG_u8LineReady = STD_TRUE;
            }
        }
        else if (LOG_u8LineLength < (u8)(LOG_LINE_SIZE - 1U))
        {
            LOG_acLine[LOG_u8LineLength] = (char)Copy_u8Byte;
            LOG_u8LineLength++;
        }
        else
        {
            LOG_u8LineLength = 0U;  /* overflow: drop the line */
        }
    }
}

/* Reads two ASCII digits at Copy_u8Pos; returns 0xFF if not digits */
static u8 LOG_u8ParseTwoDigits(u8 Copy_u8Pos)
{
    char Local_cTens  = LOG_acLine[Copy_u8Pos];
    char Local_cUnits = LOG_acLine[Copy_u8Pos + 1U];
    u8   Local_u8Value = 0xFFU;

    if ((Local_cTens >= '0') && (Local_cTens <= '9') && (Local_cUnits >= '0') && (Local_cUnits <= '9'))
    {
        Local_u8Value = (u8)((u8)((u8)(Local_cTens - '0') * 10U) + (u8)(Local_cUnits - '0'));
    }

    return Local_u8Value;
}

ES_t LOG_enuInitConsole(void)
{
    LOG_u8LineLength = 0U;
    LOG_u8LineReady  = STD_FALSE;
    return UART_enuSetRxCallback(&LOG_voidRxHandler);
}

void LOG_voidProcessConsole(void)
{
    RTC_Time_t Local_strTime;

    if (LOG_u8LineReady == STD_TRUE)
    {
        if ((LOG_u8LineLength == LOG_TIME_CMD_LENGTH) && ((LOG_acLine[0] == 'T') || (LOG_acLine[0] == 't')))
        {
            Local_strTime.Year    = LOG_u8ParseTwoDigits(1U);
            Local_strTime.Month   = LOG_u8ParseTwoDigits(3U);
            Local_strTime.Date    = LOG_u8ParseTwoDigits(5U);
            Local_strTime.Hours   = LOG_u8ParseTwoDigits(7U);
            Local_strTime.Minutes = LOG_u8ParseTwoDigits(9U);
            Local_strTime.Seconds = LOG_u8ParseTwoDigits(11U);
            Local_strTime.Day     = 1U;

            /* RTC_enuSetTime range-checks every field (0xFF fails) */
            if (RTC_enuSetTime(&Local_strTime) == ES_OK)
            {
                (void)LOG_enuWrite("Time set");
            }
            else
            {
                (void)LOG_enuWrite("Invalid time, use TYYMMDDhhmmss");
            }
        }
        else
        {
            (void)LOG_enuWrite("Unknown command, use TYYMMDDhhmmss");
        }

        /* Release the buffer for the next line */
        LOG_u8LineLength = 0U;
        LOG_u8LineReady  = STD_FALSE;
    }
}

static void LOG_voidBuildStamp(char Copy_acStamp[LOG_STAMP_LENGTH])
{
    RTC_Time_t Local_strTime = { 0U, 0U, 0U, 1U, 1U, 1U, 0U };
    static const char Local_acTemplate[LOG_STAMP_LENGTH] = "[20xx-xx-xx xx:xx:xx] ";
    u8 Local_u8Index;

    for (Local_u8Index = 0U; Local_u8Index < LOG_STAMP_LENGTH; Local_u8Index++)
    {
        Copy_acStamp[Local_u8Index] = Local_acTemplate[Local_u8Index];
    }

    if (RTC_enuGetTime(&Local_strTime) == ES_OK)
    {
        UTILS_voidTwoDigits(Local_strTime.Year,    &Copy_acStamp[3]);
        UTILS_voidTwoDigits(Local_strTime.Month,   &Copy_acStamp[6]);
        UTILS_voidTwoDigits(Local_strTime.Date,    &Copy_acStamp[9]);
        UTILS_voidTwoDigits(Local_strTime.Hours,   &Copy_acStamp[12]);
        UTILS_voidTwoDigits(Local_strTime.Minutes, &Copy_acStamp[15]);
        UTILS_voidTwoDigits(Local_strTime.Seconds, &Copy_acStamp[18]);
    }
    /* else: keep the dashed template so a missing RTC is visible in the log */
}

ES_t LOG_enuWrite(const char *Copy_pcMessage)
{
    ES_t Local_enuErrorState = ES_OK;
    char Local_acStamp[LOG_STAMP_LENGTH];

    if (Copy_pcMessage == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        LOG_voidBuildStamp(Local_acStamp);
        Local_enuErrorState = UART_enuSendString(Local_acStamp);
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = UART_enuSendString(Copy_pcMessage);
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = UART_enuSendString("\r\n");
        }
    }

    return Local_enuErrorState;
}

ES_t LOG_enuWriteValue(const char *Copy_pcMessage, s32 Copy_s32Value)
{
    ES_t Local_enuErrorState = ES_OK;
    char Local_acStamp[LOG_STAMP_LENGTH];
    char Local_acValue[LOG_VALUE_LENGTH];

    if (Copy_pcMessage == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        LOG_voidBuildStamp(Local_acStamp);
        (void)UTILS_u8S32ToString(Copy_s32Value, Local_acValue, LOG_VALUE_LENGTH);

        Local_enuErrorState = UART_enuSendString(Local_acStamp);
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = UART_enuSendString(Copy_pcMessage);
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = UART_enuSendString(Local_acValue);
        }
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = UART_enuSendString("\r\n");
        }
    }

    return Local_enuErrorState;
}
