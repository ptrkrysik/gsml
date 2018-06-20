/******************************************************************************/
/*  EMIF.H - TMS320C6x Peripheral Support Library EMIF Support                */
/*                                                                            */
/*     This file provides the header for the DSP's EMIF support.              */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*    SDRAM_REFRESH_ENABLE()  - Enable SDRAM refresh cycles                   */
/*    SDRAM_REFRESH_DISABLE() - Disable SDRAM refresh cycles                  */
/*    SDRAM_REFRESH_PERIOD()  - Assigns refresh period for SDRAM              */
/*    SDRAM_INIT()            - Perform initialization sequence for SDRAM     */
/*    EMIF_GET_MAP_MODE()     - Return value of MAP bit in EMIF global ctrl   */
/*                                                                            */
/*  FUNCTIONS:                                                                */
/*    emif_init()           - Sets all EMIF registers to parameter values     */
/*                                                                            */
/*    DATE                              DESCRIPTION                           */
/*   -------   -------------------------------------------------------------  */
/*   22May00  Changed #if _INLINE to #iddef _INLINE                           */
/*             -----> Test whether _INLINE is defined, not its value.         */
/*                                                                            */
/*   11MAY98  Removed #define TA and #define TA_SZ                            */
/*             -----> TA is no longer supported.                              */
/*                                                                            */
/*   11MAY98  Removed #define CLK2INV and #define SDCINV                      */
/*             -----> These bitfields are no longer supported.                */
/*                                                                            */
/*   11MAY98  Changed "LOAD_FIELD(EMIF_SDRAM_REF_ADDR, val)"                  */
/*                 to "LOAD_FIELD(EMIF_SDRAM_REF_ADDR, val, PERIOD,PERIOD_SZ)"*/ 
/*             -----> Fields were left out of the LOAD_FIELD macro call       */
/*                                                                            */
/*   11MAY98  Changed "#define WRTIE_STROBE_SZ"                               */
/*                 to "#define WRITE_STROBE_SZ"                               */
/*             -----> Misspelling                                             */
/*                                                                            */
/******************************************************************************/
#ifndef _EMIF_H_
#define _EMIF_H_

#include "regs.h"         /* EMIF Register Addresses and bitfield definitions */

#ifdef _INLINE
#define __INLINE static inline
#else
#define __INLINE
#endif

/******************************************************************************/
/****************************** EMIF REGISTERS ********************************/

#define EMIF_GCTRL_ADDR         0x01800000
#define EMIF_CE0_CTRL_ADDR      0x01800008
#define EMIF_CE1_CTRL_ADDR      0x01800004
#define EMIF_CE2_CTRL_ADDR      0x01800010
#define EMIF_CE3_CTRL_ADDR      0x01800014
#define EMIF_SDRAM_CTRL_ADDR    0x01800018
#define EMIF_SDRAM_REF_ADDR     0x0180001C

#define EMIF_GCTRL              (*(volatile unsigned int *)EMIF_GCTRL_ADDR)
#define EMIF_CE0_CTRL           (*(volatile unsigned int *)EMIF_CE0_CTRL_ADDR)
#define EMIF_CE1_CTRL           (*(volatile unsigned int *)EMIF_CE1_CTRL_ADDR)
#define EMIF_CE2_CTRL           (*(volatile unsigned int *)EMIF_CE2_CTRL_ADDR)
#define EMIF_CE3_CTRL           (*(volatile unsigned int *)EMIF_CE3_CTRL_ADDR)
#define EMIF_SDRAM_CTRL         (*(volatile unsigned int *)EMIF_SDRAM_CTRL_ADDR)
#define EMIF_SDRAM_REF          (*(volatile unsigned int *)EMIF_SDRAM_REF_ADDR)

/* EMIF Global Control Register Bits                                         */
#define MAP                0
#define RBTR8              1
#define SSCRT              2
#define CLK2EN             3
#define CLK1EN             4
#define SSCEN              5
#define SDCEN              6
#define NOHOLD             7
#define HOLDA              8
#define HOLD               9
#define ARDY              10

/* EMIF CE0/1/2/3 Control Register Bits                                      */
#define READ_HOLD          0
#define READ_HOLD_SZ       2

#define MTYPE              4
#define MTYPE_SZ           3

#define READ_STROBE        8
#define READ_STROBE_SZ     6

#define READ_SETUP        16
#define READ_SETUP_SZ      4

#define WRITE_HOLD        20
#define WRITE_HOLD_SZ      2

#define WRITE_STROBE      22
#define WRITE_STROBE_SZ    6

#define WRITE_SETUP       28
#define WRITE_SETUP_SZ     4


/* EMIF SDRAM Control Register Bits                                          */
#define TRC               12
#define TRC_SZ             4

#define TRP               16
#define TRP_SZ             4

#define TRCD              20
#define TRCD_SZ            4

#define INIT              24

#define RFEN              25

#define SDWID             26


/* EMIF SDRAM Timing Register Bits                                           */

#define PERIOD             0
#define PERIOD_SZ         12

#define COUNTER           12
#define COUNTER_SZ        12


/* EMIF Global Control Register Bitfield Values                              */

/* EMIF CE Space Control Register Bitfield Values                            */
#define MTYPE_8ROM          0x00     /* 8 bit wide ROM                       */
#define MTYPE_16ROM         0x01     /* 16 bit wide ROM                      */
#define MTYPE_32ASYNC       0x02     /* 32 bit asynchronous interface        */
#define MTYPE_32SDRAM       0x03     /* 32 bit SDRAM                         */
#define MTYPE_32SBSRAM      0x04     /* 32 bit SBSRAM                        */

/*---------------------------------------------------------------------------*/
/* MACRO FUNCTIONS                                                           */
/*---------------------------------------------------------------------------*/

#define SDRAM_REFRESH_ENABLE() \
        SET_BIT(EMIF_SDRAM_CTRL_ADDR,RFEN)

#define SDRAM_REFRESH_DISABLE() \
        RESET_BIT(EMIF_SDRAM_CTRL_ADDR,RFEN)

#define SDRAM_REFRESH_PERIOD(val) \
        LOAD_FIELD(EMIF_SDRAM_REF_ADDR,val,PERIOD,PERIOD_SZ)

#define SDRAM_INIT() \
        SET_BIT(EMIF_SDRAM_CTRL_ADDR,INIT)

#define EMIF_GET_MAP_MODE() \
        GET_BIT(EMIF_GCTRL_ADDR,MAP)

__INLINE
void emif_init(unsigned int g_ctrl,
               unsigned int ce0_ctrl,
               unsigned int ce1_ctrl,
               unsigned int ce2_ctrl,
               unsigned int ce3_ctrl,
               unsigned int sdram_ctrl,
               unsigned int sdram_refresh
              );

#ifdef _INLINE
__INLINE
void emif_init(unsigned int g_ctrl,
               unsigned int ce0_ctrl,
               unsigned int ce1_ctrl,
               unsigned int ce2_ctrl,
               unsigned int ce3_ctrl,
               unsigned int sdram_ctrl,
               unsigned int sdram_refresh
              )
{
  REG_WRITE(EMIF_GCTRL_ADDR, g_ctrl);
  REG_WRITE(EMIF_CE0_CTRL_ADDR, ce0_ctrl);
  REG_WRITE(EMIF_CE1_CTRL_ADDR, ce1_ctrl);
  REG_WRITE(EMIF_CE2_CTRL_ADDR, ce2_ctrl);
  REG_WRITE(EMIF_CE3_CTRL_ADDR, ce3_ctrl);
  REG_WRITE(EMIF_SDRAM_CTRL_ADDR, sdram_ctrl);
  REG_WRITE(EMIF_SDRAM_REF_ADDR, sdram_refresh);
}
#endif  /* _INLINE  */

#ifdef __INLINE
#undef __INLINE
#endif

#endif  /* _EMIF_H_ */
