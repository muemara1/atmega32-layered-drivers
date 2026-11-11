/******************************************************************************
 * File        : STD_TYPES.h
 * Layer       : LIB
 * Description : Fixed-width standard types and the common error-status type
 *               returned by every driver API in this project.
 *****************************************************************************/
#ifndef STD_TYPES_H_
#define STD_TYPES_H_

typedef unsigned char       u8;
typedef unsigned short int  u16;
typedef unsigned long int   u32;

typedef signed char         s8;
typedef signed short int    s16;
typedef signed long int     s32;

typedef float               f32;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define STD_LOW     ((u8)0U)
#define STD_HIGH    ((u8)1U)

#define STD_FALSE   ((u8)0U)
#define STD_TRUE    ((u8)1U)

/* Error status returned by every public API */
typedef enum
{
    ES_OK = 0,          /* Request done successfully                    */
    ES_NOK,             /* Generic failure                              */
    ES_NULL_POINTER,    /* A pointer argument was NULL                  */
    ES_OUT_OF_RANGE,    /* An argument was outside its valid range      */
    ES_TIMEOUT,         /* Hardware did not respond within the timeout  */
    ES_NOT_INIT         /* Module used before its init function         */
} ES_t;

#endif /* STD_TYPES_H_ */
