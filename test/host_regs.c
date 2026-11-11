/******************************************************************************
 * File        : host_regs.c
 * Description : RAM that stands in for the ATmega32 I/O space when the
 *               drivers are compiled with -DHOST_TEST.
 *****************************************************************************/
#include "STD_TYPES.h"
#include "ATMEGA32_REG.h"

volatile u8 HOST_au8RegSpace[REG_SPACE_SIZE];
