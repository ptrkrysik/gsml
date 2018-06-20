/******************************************************************************/
/*  REGS.H - TMS320C6x Peripheral Support Library CPU Register Support        */
/*                                                                            */
/*     This file provides the header for the DSP's register support.          */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*     REG_READ          - Read register at specified address                 */
/*     REG_WRITE         - Write to register at specified address             */
/*     RESET_BIT         - Clears bit in register.                            */
/*     GET_BIT           - Returns bit value in register.                     */
/*     SET_BIT           - Sets bit in register.                              */
/*     MASK_BIT          - Create (1's) mask for specified bit.               */
/*     ASSIGN_BIT_VAL    - Assign bit to specified value                      */
/*     RESET_FIELD       - Clears field in register                           */
/*     GET_FIELD         - Returns value of bit field in a register           */
/*     MASK_FIELD        - Create (1's) mask for specified field              */
/*     LOAD_FIELD        - Assigns bit field in register                      */
/*     GET_REG           - Returns value of non memory mapped register        */
/*     SET_REG           - Sets value of a non memory mapped register         */
/*     GET_REG_BIT       - Return bit value in non memory mapped register     */
/*     SET_REG_BIT       - Sets bit in non memory mapped register             */
/*     RESET_REG_BIT     - Resets given bit in non memory mapped register     */
/*     GET_REG_FIELD     - Return value of specified register field           */
/*     LOAD_REG_FIELD    - Set value of specified register fiedl              */
/*                                                                            */
/******************************************************************************/
#ifndef _REGS_H_
#define _REGS_H_

/*----------------------------------------------------------------------------*/
/* DEFINES                                                                    */
/*----------------------------------------------------------------------------*/

/**************************** CONTROL REGISTERS *******************************/

extern cregister volatile unsigned int AMR;     /* Address Mode Register      */
extern cregister volatile unsigned int CSR;     /* Control Status Register    */
extern cregister volatile unsigned int IFR;     /* Interrupt Flag Register    */
extern cregister volatile unsigned int ISR;     /* Interrupt Set Register     */
extern cregister volatile unsigned int ICR;     /* Interrupt Clear Register   */
extern cregister volatile unsigned int IER;     /* Interrupt Enable Register  */
extern cregister volatile unsigned int ISTP;    /* Interrupt Service Tbl Ptr  */
extern cregister volatile unsigned int IRP;     /* Interrupt Return Pointer   */
extern cregister volatile unsigned int NRP;     /* Non-maskable Int Return Ptr*/
extern cregister volatile unsigned int IN;      /* General Purpose Input Reg  */
extern cregister volatile unsigned int OUT;     /* General Purpose Output Reg */

/* Control Register Bitfields */

/* AMR */
#define A4_MODE               0
#define A4_MODE_SZ            2
#define A5_MODE               2
#define A5_MODE_SZ            2
#define A6_MODE               4
#define A6_MODE_SZ            2
#define A7_MODE               6
#define A7_MODE_SZ            2
#define B4_MODE               8
#define B4_MODE_SZ            2
#define B5_MODE              10
#define B5_MODE_SZ            2
#define B6_MODE              12
#define B6_MODE_SZ            2
#define B7_MODE              14
#define B7_MODE_SZ            2
#define BK0                  16
#define BK0_SZ                5
#define BK1                  21
#define BK1_SZ                5

/* CSR */
#define GIE                   0
#define PGIE                  1
#define DCC                   2
#define DCC_SZ                3
#define PCC                   5
#define PCC_SZ                3
#define EN                    8
#define SAT                   9
#define PWRD                 10
#define PWRD_SZ               6
#define REVISION_ID          16
#define REVISION_ID_SZ        8
#define CPU_ID               24
#define CPU_ID_SZ             8

/* Interrupt Enable Register (IER) */
#define NMIE                  1
#define IE4                   4
#define IE5                   5
#define IE6                   6
#define IE7                   7
#define IE8                   8
#define IE9                   9
#define IE10                 10
#define IE11                 11
#define IE12                 12
#define IE13                 13
#define IE14                 14
#define IE15                 15

/* Interrupt Flag Register (IFR) */
#define NMIF                  1
#define IF4                   4
#define IF5                   5
#define IF6                   6
#define IF7                   7
#define IF8                   8
#define IF9                   9
#define IF10                 10
#define IF11                 11
#define IF12                 12
#define IF13                 13
#define IF14                 14
#define IF15                 15

/* Interrupt Set register (ISR) */
#define IS4                   4
#define IS5                   5
#define IS6                   6
#define IS7                   7
#define IS8                   8
#define IS9                   9
#define IS10                 10
#define IS11                 11
#define IS12                 12
#define IS13                 13
#define IS14                 14
#define IS15                 15

/* Interrupt Clear Register (ICR) */
#define IC4                   4
#define IC5                   5
#define IC6                   6
#define IC7                   7
#define IC8                   8
#define IC9                   9
#define IC10                 10
#define IC11                 11
#define IC12                 12
#define IC13                 13
#define IC14                 14
#define IC15                 15

/* Interrupt Service Table Pointer (ISTP) */
#define ISTB                 10
#define ISTB_SZ              22
#define HPEINT                5
#define HPEINT_SZ             5

/*----------------------------------------------------------------------------*/
/* MACRO FUNCTIONS                                                            */
/*----------------------------------------------------------------------------*/

#define CONTENTS_OF(addr) \
        (*((volatile unsigned int *)(addr)))

#define LENGTH_TO_BITS(length) \
        (~(0xffffffff << (length)))

/* MACROS to SET, CLEAR and RETURN bits and bitfields in Memory Mapped        */
/* locations using the address of the specified register.                     */

#define REG_READ(addr) \
        (CONTENTS_OF(addr))

#define REG_WRITE(addr,val) \
        (CONTENTS_OF(addr) = (val))


#define MASK_BIT(bit) \
        (1 << (bit))

#define RESET_BIT(addr,bit) \
        (CONTENTS_OF(addr) &= (~MASK_BIT(bit)))

#define GET_BIT(addr,bit) \
        ((CONTENTS_OF(addr) & MASK_BIT(bit)) ? 1 : 0)

#define SET_BIT(addr,bit) \
        (CONTENTS_OF(addr) = (CONTENTS_OF(addr)) | (MASK_BIT(bit)))

#define ASSIGN_BIT_VAL(addr,bit,val) \
        ( (val) ? SET_BIT(addr,bit) : RESET_BIT(addr,bit) )



#define MASK_FIELD(bit,length) \
        (LENGTH_TO_BITS(length) << (bit))

#define RESET_FIELD(addr,bit,length) \
        ( CONTENTS_OF(addr) &= (~MASK_FIELD(bit,length)))

#define GET_FIELD(addr,bit,length) \
       ((CONTENTS_OF(addr) & MASK_FIELD(bit,length)) >> bit)

#define LOAD_FIELD(addr,val,bit,length) \
        (CONTENTS_OF(addr) = (CONTENTS_OF(addr) & (~MASK_FIELD(bit,length))) | (val<<bit))

/* MACROS to SET, CLEAR and RETURN bits and bitfields in Memory Mapped        */
/* and Non-Memory Mapped using register names.                                */

#define GET_REG(reg) \
        (reg)

#define SET_REG(reg,val) \
        ((reg)= (val))

#define GET_REG_BIT(reg,bit) \
        ((reg) & MASK_BIT(bit) ? 1 : 0)

#define SET_REG_BIT(reg,bit) \
        ((reg) |= MASK_BIT(bit))

#define RESET_REG_BIT(reg,bit) \
        ((reg) &= (~MASK_BIT(bit)))

#define GET_REG_FIELD(reg,bit,length) \
        ((reg & MASK_FIELD(bit,length)) >> bit)

#define LOAD_REG_FIELD(reg,val,bit,length) \
        (reg &= (~MASK_FIELD(bit,length)) | (val<<bit))

#endif /* ifndef _REGS_H_ */
