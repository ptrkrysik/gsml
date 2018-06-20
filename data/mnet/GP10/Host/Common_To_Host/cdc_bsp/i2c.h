/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
JetCell modification history
----------------------------
99/06/01  ck   Initial Version
*/

/*
 * File name     : i2c.h
 * Description   : Constants and Definitions for I2C
 * History:
 *
 * 06-19-97	stu	Initial Version
 *
 */

#ifndef _I2C_H
#define _I2C_H

#ifdef __cplusplus 
extern "C" { 
#endif

/* ==================== GENERAL CONSTANTS AND DEFINITIONS =============== */

#define TRUE 1
#define FALSE 0

#define ON	1
#define OFF 0

/* ================== APPLICATION CONSTANTS AND DEFINITIONS  =============== */
#define I2C_BASE 0x3c80
#define RPBASE I2C_BASE

#define I2C_RBASE(base)	 (CAST(VUINT16 *) (base + RPBASE + 0x00))
#define I2C_TBASE(base)	 (CAST(VUINT16 *) (base + RPBASE + 0x02))
#define I2C_RFCR(base)	 (CAST(VUINT8  *) (base + RPBASE + 0x04))
#define I2C_TFCR(base)	 (CAST(VUINT8  *) (base + RPBASE + 0x05))
#define I2C_MRBLR(base)	 (CAST(VUINT16 *) (base + RPBASE + 0x06))
#define I2C_RSTATE(base) (CAST(VUINT32 *) (base + RPBASE + 0x08))
#define I2C_RBPTR(base)	 (CAST(VUINT16 *) (base + RPBASE + 0x10))
#define I2C_TSTATE(base) (CAST(VUINT32 *) (base + RPBASE + 0x18))
#define I2C_TBPTR(base)	 (CAST(VUINT16 *) (base + RPBASE + 0x20))

/*
 * Definitions for ATMEL AT24C01 EEPROM
 */

/* Hard-wired E2PROM Address -- Determined by Board Designer */
#define E2PROM_ADDR (0x50)

/* MPC860 I2C address */
#define MPC860_I2C_ADDR (0x66)

/*
 * AT24C01 EEPROM Data Sheet specifies a maximum serial clock frequency
 *     of 400 KHz
 * 
 * ==> Provide an overal clock divider of >=60 for 24MHz System Clock
 */
#define BAUD_PREDIV_AT24C01 (0x0)
#define BAUD_DIV_AT24C01 (0x10)
#define LONGTIME 0x00100000

/* ----------------------------------------------------------------- */

/* Transmit Buffer Descriptor Indices */
#define TXBD_0 0
#define TXBD_1 1

/* I2C Read/Write Bit Definitions */
#define WRITE_BIT (0)   /* write from mpc821 to eeprom */
#define READ_BIT  (1)   /* read from eeprom to mpc821 */

/* Transmit Buffer Descriptor Control and Status Bit Definitions */
#define TXBD_R  (0x8000)
#define TXBD_W  (0x2000)
#define TXBD_I  (0x1000)
#define TXBD_L  (0x0800)
#define TXBD_S  (0x0400)
#define TXBD_N  (0x0004)

/* Receive Buffer Descriptor Control and Status Bit Definitions */
#define RXBD_E  (0x8000)

/* Interrupt Level */
#define INTERRUPT_LEVEL  (4)

/* SIU Vector Interrupt Code: Level 4 */
#define IC_LEVEL_4  (0x24)

/* I2C Bit in CPM In-Service Interrupt Register */
#define I2C_INTERRUPT (1<<16)

/* I2C Interrupt Vector Code in CPM Vector Register (CIVR) */
#define I2C_VECTOR (0x10)

/* Address of I2C Condition Flag */
#define I2C_FLAG_LOC (0x50000)

#define I2C_RX_LEN    (512)  /* Rx Maximum characters */
#define I2C_TX_LEN    (512)  /* Tx Maximum characters */

/*
 * Buffer Descriptor Format
 */

typedef struct I2C_BufferDescriptor {
    unsigned short bd_cstatus;     /* control and status */
    unsigned short bd_length;      /* transfer length */
    unsigned char *bd_addr;        /* Pointer to Buffer */
} I2C_BD;

typedef struct rx_tx_bd {
    I2C_BD rxbd;
    I2C_BD txbd[2];
} RTXBD;

extern STATUS I2Cwrite(unsigned char*, unsigned char, unsigned char, int);
extern STATUS I2Cread(unsigned char*, unsigned char, unsigned char, int);
extern STATUS I2Cwrite2(unsigned char*, unsigned char, int);
extern STATUS I2Cread2(unsigned char*, unsigned char, int);
extern void I2Coperation(int);
extern void RxBD(int);
extern int readRxBD(unsigned char *, int);

#ifdef __cplusplus 
} 
#endif

#endif /* _I2C_H */
