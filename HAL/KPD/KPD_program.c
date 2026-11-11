/******************************************************************************
 * File        : KPD_program.c
 * Layer       : HAL
 * Description : 4x4 keypad - implementation.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "DELAY.h"

#include "DIO_interface.h"

#include "KPD_interface.h"
#include "KPD_config.h"

static const u8 KPD_au8RowPort[KPD_ROWS]    = { KPD_R0_PORT, KPD_R1_PORT, KPD_R2_PORT, KPD_R3_PORT };
static const u8 KPD_au8RowPin[KPD_ROWS]     = { KPD_R0_PIN,  KPD_R1_PIN,  KPD_R2_PIN,  KPD_R3_PIN  };
static const u8 KPD_au8ColPort[KPD_COLUMNS] = { KPD_C0_PORT, KPD_C1_PORT, KPD_C2_PORT, KPD_C3_PORT };
static const u8 KPD_au8ColPin[KPD_COLUMNS]  = { KPD_C0_PIN,  KPD_C1_PIN,  KPD_C2_PIN,  KPD_C3_PIN  };

static const u8 KPD_au8KeyMap[KPD_ROWS][KPD_COLUMNS] = KPD_KEY_MAP;

static u8 KPD_u8LastRawKey = KPD_NO_KEY;

/* One full matrix scan; returns the first pressed key or KPD_NO_KEY */
static u8 KPD_u8ScanRaw(void)
{
    u8 Local_u8Key = KPD_NO_KEY;
    u8 Local_u8Row;
    u8 Local_u8Col;
    u8 Local_u8Level;

    for (Local_u8Row = 0U; (Local_u8Row < KPD_ROWS) && (Local_u8Key == KPD_NO_KEY); Local_u8Row++)
    {
        (void)DIO_enuSetPinValue(KPD_au8RowPort[Local_u8Row], KPD_au8RowPin[Local_u8Row], DIO_LOW);
        DELAY_US(5);    /* let the line settle */

        for (Local_u8Col = 0U; (Local_u8Col < KPD_COLUMNS) && (Local_u8Key == KPD_NO_KEY); Local_u8Col++)
        {
            Local_u8Level = DIO_HIGH;
            (void)DIO_enuGetPinValue(KPD_au8ColPort[Local_u8Col], KPD_au8ColPin[Local_u8Col], &Local_u8Level);
            if (Local_u8Level == DIO_LOW)
            {
                Local_u8Key = KPD_au8KeyMap[Local_u8Row][Local_u8Col];
            }
        }

        (void)DIO_enuSetPinValue(KPD_au8RowPort[Local_u8Row], KPD_au8RowPin[Local_u8Row], DIO_HIGH);
    }

    return Local_u8Key;
}

ES_t KPD_enuInit(void)
{
    u8 Local_u8Index;

    for (Local_u8Index = 0U; Local_u8Index < KPD_ROWS; Local_u8Index++)
    {
        (void)DIO_enuSetPinValue(KPD_au8RowPort[Local_u8Index], KPD_au8RowPin[Local_u8Index], DIO_HIGH);
        (void)DIO_enuSetPinDirection(KPD_au8RowPort[Local_u8Index], KPD_au8RowPin[Local_u8Index], DIO_OUTPUT);
    }

    for (Local_u8Index = 0U; Local_u8Index < KPD_COLUMNS; Local_u8Index++)
    {
        (void)DIO_enuSetPinDirection(KPD_au8ColPort[Local_u8Index], KPD_au8ColPin[Local_u8Index], DIO_INPUT);
        (void)DIO_enuSetPinValue(KPD_au8ColPort[Local_u8Index], KPD_au8ColPin[Local_u8Index], DIO_PULLUP);
    }

    KPD_u8LastRawKey = KPD_NO_KEY;

    return ES_OK;
}

ES_t KPD_enuGetPressedKey(u8 *Copy_pu8Key)
{
    ES_t Local_enuErrorState = ES_OK;
    u8   Local_u8RawKey;

    if (Copy_pu8Key == NULL)
    {
        Local_enuErrorState = ES_NULL_POINTER;
    }
    else
    {
        *Copy_pu8Key   = KPD_NO_KEY;
        Local_u8RawKey = KPD_u8ScanRaw();

        if ((Local_u8RawKey != KPD_NO_KEY) && (Local_u8RawKey != KPD_u8LastRawKey))
        {
            /* New press: confirm it after the bounce time */
            DELAY_MS(KPD_DEBOUNCE_MS);
            if (KPD_u8ScanRaw() == Local_u8RawKey)
            {
                *Copy_pu8Key = Local_u8RawKey;
            }
            else
            {
                Local_u8RawKey = KPD_NO_KEY;    /* bounce, ignore */
            }
        }

        KPD_u8LastRawKey = Local_u8RawKey;
    }

    return Local_enuErrorState;
}
