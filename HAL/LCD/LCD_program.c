/******************************************************************************
 * File        : LCD_program.c
 * Layer       : HAL
 * Description : 16x2 character LCD (4-bit) - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "DELAY.h"
#include "UTILS.h"

#include "DIO_interface.h"

#include "LCD_interface.h"
#include "LCD_private.h"
#include "LCD_config.h"

static const u8 LCD_au8DataPort[4] = { LCD_D4_PORT, LCD_D5_PORT, LCD_D6_PORT, LCD_D7_PORT };
static const u8 LCD_au8DataPin[4]  = { LCD_D4_PIN,  LCD_D5_PIN,  LCD_D6_PIN,  LCD_D7_PIN  };

static u8 LCD_u8Initialized = STD_FALSE;

/* Puts the low nibble of Copy_u8Nibble on D4..D7 and latches it with EN */
static void LCD_voidWriteNibble(u8 Copy_u8Nibble)
{
    u8 Local_u8Bit;

    for (Local_u8Bit = 0U; Local_u8Bit < 4U; Local_u8Bit++)
    {
        (void)DIO_enuSetPinValue(LCD_au8DataPort[Local_u8Bit], LCD_au8DataPin[Local_u8Bit],
                                 GET_BIT(Copy_u8Nibble, Local_u8Bit));
    }

    (void)DIO_enuSetPinValue(LCD_EN_PORT, LCD_EN_PIN, DIO_HIGH);
    DELAY_US(1);                                    /* EN pulse >= 450 ns */
    (void)DIO_enuSetPinValue(LCD_EN_PORT, LCD_EN_PIN, DIO_LOW);
    DELAY_US(1);
}

static void LCD_voidWriteByte(u8 Copy_u8RegisterSelect, u8 Copy_u8Byte)
{
    (void)DIO_enuSetPinValue(LCD_RS_PORT, LCD_RS_PIN, Copy_u8RegisterSelect);
    LCD_voidWriteNibble((u8)(Copy_u8Byte >> 4));    /* high nibble first */
    LCD_voidWriteNibble((u8)(Copy_u8Byte & 0x0FU));
    DELAY_US(50);                                   /* most commands: 37 us */
}

ES_t LCD_enuInit(void)
{
    u8 Local_u8Bit;

    (void)DIO_enuSetPinDirection(LCD_RS_PORT, LCD_RS_PIN, DIO_OUTPUT);
    (void)DIO_enuSetPinDirection(LCD_EN_PORT, LCD_EN_PIN, DIO_OUTPUT);
    (void)DIO_enuSetPinValue(LCD_EN_PORT, LCD_EN_PIN, DIO_LOW);
    for (Local_u8Bit = 0U; Local_u8Bit < 4U; Local_u8Bit++)
    {
        (void)DIO_enuSetPinDirection(LCD_au8DataPort[Local_u8Bit], LCD_au8DataPin[Local_u8Bit],
                                     DIO_OUTPUT);
    }

    /* Power-on wait, then the "initialization by instruction" sequence
     * from the HD44780 datasheet (figure 24) to enter 4-bit mode. */
    DELAY_MS(50);
    (void)DIO_enuSetPinValue(LCD_RS_PORT, LCD_RS_PIN, LCD_REGISTER_COMMAND);
    LCD_voidWriteNibble(0x03U);
    DELAY_MS(5);
    LCD_voidWriteNibble(0x03U);
    DELAY_US(150);
    LCD_voidWriteNibble(0x03U);
    DELAY_US(150);
    LCD_voidWriteNibble(0x02U);
    DELAY_US(150);

    LCD_voidWriteByte(LCD_REGISTER_COMMAND, LCD_CMD_FUNCTION_4BIT);
    LCD_voidWriteByte(LCD_REGISTER_COMMAND, LCD_CMD_DISPLAY_OFF);
    LCD_voidWriteByte(LCD_REGISTER_COMMAND, LCD_CMD_CLEAR);
    DELAY_MS(2);
    LCD_voidWriteByte(LCD_REGISTER_COMMAND, LCD_CMD_ENTRY_MODE);
    LCD_voidWriteByte(LCD_REGISTER_COMMAND, LCD_CMD_DISPLAY_ON);

    LCD_u8Initialized = STD_TRUE;

    return ES_OK;
}

ES_t LCD_enuSendCommand(u8 Copy_u8Command)
{
    ES_t Local_enuErrorState = ES_OK;

    if (LCD_u8Initialized == STD_FALSE)
    {
        Local_enuErrorState = ES_NOT_INIT;
    }
    else
    {
        LCD_voidWriteByte(LCD_REGISTER_COMMAND, Copy_u8Command);
        if ((Copy_u8Command == LCD_CMD_CLEAR) || (Copy_u8Command == LCD_CMD_HOME))
        {
            DELAY_MS(2);                            /* 1.52 ms commands */
        }
    }

    return Local_enuErrorState;
}

ES_t LCD_enuClear(void)
{
    return LCD_enuSendCommand(LCD_CMD_CLEAR);
}

ES_t LCD_enuGoTo(u8 Copy_u8Row, u8 Copy_u8Column)
{
    ES_t Local_enuErrorState = ES_OK;

    if ((Copy_u8Row >= LCD_ROWS) || (Copy_u8Column >= LCD_COLUMNS))
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        u8 Local_u8Address = (Copy_u8Row == LCD_ROW0) ? Copy_u8Column
                                                       : (u8)(LCD_ROW1_ADDRESS_OFFSET + Copy_u8Column);
        Local_enuErrorState = LCD_enuSendCommand((u8)(LCD_CMD_SET_DDRAM | Local_u8Address));
    }

    return Local_enuErrorState;
}

ES_t LCD_enuWriteChar(char Copy_cChar)
{
    ES_t Local_enuErrorState = ES_OK;

    if (LCD_u8Initialized == STD_FALSE)
    {
        Local_enuErrorState = ES_NOT_INIT;
    }
    else
    {
        LCD_voidWriteByte(LCD_REGISTER_DATA, (u8)Copy_cChar);
    }

    return Local_enuErrorState;
}

ES_t LCD_enuWriteString(const char *Copy_pcString)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_u8Index       = 0U;

    if (Copy_pcString == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        while ((Copy_pcString[Local_u8Index] != '\0') && (Local_enuErrorState == ES_OK))
        {
            Local_enuErrorState = LCD_enuWriteChar(Copy_pcString[Local_u8Index]);
            Local_u8Index++;
        }
    }

    return Local_enuErrorState;
}

ES_t LCD_enuWriteNumber(s32 Copy_s32Number)
{
    char Local_acBuffer[LCD_NUMBER_BUFFER_SIZE];

    (void)UTILS_u8S32ToString(Copy_s32Number, Local_acBuffer, LCD_NUMBER_BUFFER_SIZE);

    return LCD_enuWriteString(Local_acBuffer);
}

ES_t LCD_enuWriteLine(u8 Copy_u8Row, const char *Copy_pcString)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_u8Column      = 0U;

    if (Copy_pcString == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        Local_enuErrorState = LCD_enuGoTo(Copy_u8Row, 0U);

        /* Write the text, then pad with spaces to overwrite old content */
        while ((Local_u8Column < LCD_COLUMNS) && (Local_enuErrorState == ES_OK))
        {
            if (Copy_pcString[Local_u8Column] == '\0')
            {
                break;
            }
            Local_enuErrorState = LCD_enuWriteChar(Copy_pcString[Local_u8Column]);
            Local_u8Column++;
        }
        while ((Local_u8Column < LCD_COLUMNS) && (Local_enuErrorState == ES_OK))
        {
            Local_enuErrorState = LCD_enuWriteChar(' ');
            Local_u8Column++;
        }
    }

    return Local_enuErrorState;
}

ES_t LCD_enuCreateCustomChar(u8 Copy_u8Slot, const u8 Copy_au8Pattern[8])
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_u8Row;

    if (Copy_au8Pattern == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else if (Copy_u8Slot >= LCD_CGRAM_SLOTS)
    {
        Local_enuErrorState = ES_OUT_OF_RANGE;
    }
    else
    {
        Local_enuErrorState = LCD_enuSendCommand((u8)(LCD_CMD_SET_CGRAM | (u8)(Copy_u8Slot << 3)));
        for (Local_u8Row = 0U; (Local_u8Row < LCD_GLYPH_ROWS) && (Local_enuErrorState == ES_OK); Local_u8Row++)
        {
            Local_enuErrorState = LCD_enuWriteChar((char)Copy_au8Pattern[Local_u8Row]);
        }
        /* Return to DDRAM so the next write goes to the display */
        if (Local_enuErrorState == ES_OK)
        {
            Local_enuErrorState = LCD_enuGoTo(LCD_ROW0, 0U);
        }
    }

    return Local_enuErrorState;
}
