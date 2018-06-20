/******************************************************************************/
/*  HPI.H - TMS320C6x Peripheral Support Library EMIF Support                 */
/*                                                                            */
/*     This file provides the header for the DSP's HPI support.               */
/*                                                                            */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*    HPI_SET_HINT()                                                          */
/*    HPI_RESET_DSPINT()                                                      */
/*    HPI_GET_HINT()                                                          */
/*    HPI_GET_DSPINT()                                                        */
/*                                                                            */
/*  FUNCTIONS:                                                                */
/*    None.                                                                   */
/*                                                                            */
/******************************************************************************/
#ifndef _HPI_H_
#define _HPI_H_

/*----------------------------------------------------------------------------*/
/* INCLUDES                                                                   */
/*----------------------------------------------------------------------------*/
#include "regs.h" 

/*----------------------------------------------------------------------------*/
/* DEFINES AND MACROS                                                         */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/****************************** HPI REGISTERS *********************************/

#define HPIC_ADDR          0x01880000                  /* HPI Ctrl Reg Addr   */
#define HPIC               *(volatile unsigned int *)HPIC_ADDR  /* HPI Ctrl   */

/* HPIC Register bits */
#define HWOB               0
#define DSPINT             1
#define HINT               2
#define HRDY               3
#define FETCH              4

#define HPI_SET_HINT() \
        (REG_WRITE(HPIC_ADDR,(REG_READ(HPIC_ADDR) & 0xFFFFFFFD) | 4))

#define HPI_RESET_DSPINT() \
        (REG_WRITE(HPIC_ADDR,(REG_READ(HPIC_ADDR) & 0xFFFFFFFB) | 2))

#define HPI_GET_HINT() \
        (GET_BIT(HPIC_ADDR,HINT))

#define HPI_GET_DSPINT() \
        (GET_BIT(HPIC_ADDR,DSPINT))

/*----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                  */
/*----------------------------------------------------------------------------*/

#endif
