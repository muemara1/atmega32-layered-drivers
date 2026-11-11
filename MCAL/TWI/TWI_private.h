/******************************************************************************
 * File        : TWI_private.h
 * Layer       : MCAL
 *****************************************************************************/
#ifndef TWI_PRIVATE_H_
#define TWI_PRIVATE_H_

#ifndef F_CPU
    #error "F_CPU must be defined (see Makefile)"
#endif

/* SCL = F_CPU / (16 + 2 * TWBR * 4^TWPS), with TWPS = 0 */
#define TWI_TWBR_VALUE  ((u8)((((u32)F_CPU / (u32)TWI_SCL_FREQ_HZ) - 16UL) / 2UL))

#define TWI_STATUS_MASK         ((u8)0xF8U)

/* Master status codes (datasheet tables 74 and 75) */
#define TWI_SC_START            ((u8)0x08U)
#define TWI_SC_REP_START        ((u8)0x10U)
#define TWI_SC_MT_SLA_W_ACK     ((u8)0x18U)
#define TWI_SC_MT_DATA_ACK      ((u8)0x28U)
#define TWI_SC_MR_SLA_R_ACK     ((u8)0x40U)
#define TWI_SC_MR_DATA_ACK      ((u8)0x50U)
#define TWI_SC_MR_DATA_NACK     ((u8)0x58U)

#define TWI_TIMEOUT_COUNT       ((u32)30000UL)

#endif /* TWI_PRIVATE_H_ */
