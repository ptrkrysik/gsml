/******************************************************************************/
/*  MCBSP.H - TMS320C6x Peripheral Support Library McBSP Support              */
/*                                                                            */
/*     This file provides the header for the DSP's McBSP support.             */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*     MCBSP_BYTES_PER_WORD() - Return bytes required for word length         */
/*     MCBSP_ENABLE()         - Enables McBSP transit, receive or both        */
/*     MCBSP_TX_RESET()       - Reset McBSP transmitter                       */
/*     MCBSP_RX_RESET()       - Reset McBSP receiver                          */
/*     MCBSP_READ()           - Read data value from McBSP receive register   */
/*     MCBSP_WRITE()          - Write data value to McBSP transmit register   */
/*     MCBSP_IO_ENABLE()      - Place McBSP in general-purpose I/O mode       */
/*     MCBSP_IO_DISABLE()     - Remove McBSP from general-purpose I/O mode    */
/*     MCBSP_FRAME_SYNC_ENABLE()- Enables McBSP frame sync generation logic   */
/*     MCBSP_FRAME_SYNC_RESET() - Resets McBSP frame sync generation logic    */
/*     MCBSP_SAMPLE_RATE_ENABLE()-Enables McBSP sample rate generator         */
/*     MCBSP_SAMPLE_RATE_RESET()- Resets McBSP sample rate generator          */
/*     MCBSP_RRDY()           - Returns McBSP receiver ready status           */
/*     MCBSP_XRDY()           - Returns McBSP transmitter ready status        */
/*     MCBSP_LOOPBACK_ENABLE()- Configures McBSP in digital loopback mode     */
/*     MCBSP_LOOPBACK_DISABLE()-Disables McBSP digital loopback mode          */
/*                                                                            */
/*  FUNCTIONS:                                                                */
/*     mcbsp_init()           - Initializes McBSP registers                   */
/*                                                                            */
/*  GLOBAL VARIABLES                                                          */
/*                                                                            */ 
/*  REVISION HISTORY:                                                         */
/*                                                                            */
/*    DATE                              DESCRIPTION                           */
/*   -------  --------------------------------------------------------------  */
/*   13JUL98  Changed CLKSTP definition from 10 to 11.                        */
/*            Changed CLKSTP_SZ definition from 3 to 2.                       */
/*             -----> Fixed problem associated with typo in previous          */
/*                    Peripheral Reference Guide                              */
/*                                                                            */
/*   15JUN98  Changed McBSP address macros to use "port_no" instead of "port" */
/*              parameter for consistency with documentation and other code.  */
/*             -----> No functional change.                                   */
/*                                                                            */
/*   11MAY98  Added #define CLK_MODE_CLKS 0x00                                */
/*              and #define CLK_MODE_CPU  0x01                                */
/*             -----> Used in programming SRGR                                */
/*                                                                            */
/*   11MAY98  Redefined CLKS_POL_FALLING to 0x01                              */
/*                  and CLKS_POL_RISING  to 0x00                              */
/*             -----> Assignments were swapped                                */
/*                                                                            */
/******************************************************************************/

#ifndef _MCBSP_H_
#define _MCBSP_H_

#ifdef _INLINE
#define __INLINE static inline
#else
#define __INLINE
#endif

/*----------------------------------------------------------------------------*/
/* INCLUDES                                                                   */
/*----------------------------------------------------------------------------*/
#include "regs.h"

/*----------------------------------------------------------------------------*/
/* DEFINES AND MACROS                                                         */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/****************************** MCSP REGISTERS ********************************/

/* Multi-Channel Buffered Serial Port Control Registers & Bits                */
#define MCBSP_ADDR(port_no)         (0x018C0000 + ((port_no) * 0x40000))
#define MCBSP_DRR_ADDR(port_no)     (MCBSP_ADDR(port_no))
#define MCBSP_DXR_ADDR(port_no)     ((MCBSP_ADDR(port_no)) + 0x04)
#define MCBSP_SPCR_ADDR(port_no)    ((MCBSP_ADDR(port_no)) + 0x08)

#define MCBSP_RCR_ADDR(port_no)     ((MCBSP_ADDR(port_no)) + 0x0c)
#define MCBSP_XCR_ADDR(port_no)     ((MCBSP_ADDR(port_no)) + 0x10)
#define MCBSP_SRGR_ADDR(port_no)    ((MCBSP_ADDR(port_no)) + 0x14)
#define MCBSP_MCR_ADDR(port_no)     ((MCBSP_ADDR(port_no)) + 0x18)
#define MCBSP_RCER_ADDR(port_no)    ((MCBSP_ADDR(port_no)) + 0x1c)
#define MCBSP_XCER_ADDR(port_no)    ((MCBSP_ADDR(port_no)) + 0x20)
#define MCBSP_PCR_ADDR(port_no)     ((MCBSP_ADDR(port_no)) + 0x24)

#define MCBSP0_DRR               *(volatile unsigned int *)(MCBSP_DRR_ADDR(0))
#define MCBSP0_DXR               *(volatile unsigned int *)(MCBSP_DXR_ADDR(0))
#define MCBSP0_SPCR              *(volatile unsigned int *)(MCBSP_SPCR_ADDR(0))
#define MCBSP0_RCR               *(volatile unsigned int *)(MCBSP_RCR_ADDR(0))
#define MCBSP0_XCR               *(volatile unsigned int *)(MCBSP_XCR_ADDR(0))
#define MCBSP0_SRGR              *(volatile unsigned int *)(MCBSP_SRGR_ADDR(0))
#define MCBSP0_MCR               *(volatile unsigned int *)(MCBSP_MCR_ADDR(0))
#define MCBSP0_RCER              *(volatile unsigned int *)(MCBSP_RCER_ADDR(0))
#define MCBSP0_XCER              *(volatile unsigned int *)(MCBSP_XCER_ADDR(0))
#define MCBSP0_PCR               *(volatile unsigned int *)(MCBSP_PCR_ADDR(0))

#define MCBSP1_DRR               *(volatile unsigned int *)(MCBSP_DRR_ADDR(1))
#define MCBSP1_DXR               *(volatile unsigned int *)(MCBSP_DXR_ADDR(1))
#define MCBSP1_SPCR              *(volatile unsigned int *)(MCBSP_SPCR_ADDR(1))
#define MCBSP1_RCR               *(volatile unsigned int *)(MCBSP_RCR_ADDR(1))
#define MCBSP1_XCR               *(volatile unsigned int *)(MCBSP_XCR_ADDR(1))
#define MCBSP1_SRGR              *(volatile unsigned int *)(MCBSP_SRGR_ADDR(1))
#define MCBSP1_MCR               *(volatile unsigned int *)(MCBSP_MCR_ADDR(1))
#define MCBSP1_RCER              *(volatile unsigned int *)(MCBSP_RCER_ADDR(1))
#define MCBSP1_XCER              *(volatile unsigned int *)(MCBSP_XCER_ADDR(1))
#define MCBSP1_PCR               *(volatile unsigned int *)(MCBSP_PCR_ADDR(1))

/* Multi-channel Serial Port Control Register Bits                            */
#define RRST           0
#define RRDY           1
#define RFULL          2
#define RSYNC_ERR      3
#define RINTM          4
#define RINTM_SZ       2
#define CLKSTP        11
#define CLKSTP_SZ      2
#define RJUST         13
#define RJUST_SZ       2
#define DLB           15
#define XRST          16
#define XRDY          17
#define XEMPTY        18
#define XSYNC_ERR     19
#define XINTM         20
#define XINTM_SZ       2
#define GRST          22
#define FRST          23

/* Multi-channel Serial Port Pin Control Reg Bits                             */
#define CLKRP          0
#define CLKXP          1
#define FSRP           2
#define FSXP           3
#define DR_STAT        4
#define DX_STAT        5
#define CLKS_STAT      6
#define CLKRM          8
#define CLKXM          9
#define FSRM          10
#define FSXM          11
#define RIOEN         12
#define XIOEN         13


/* Multi-channel Serial Port RX & TX Ctrl Reg Bits                            */
#define RWDLEN1        5
#define RWDLEN1_SZ     3

#define RFRLEN1        8
#define RFRLEN1_SZ     7

#define RDATDLY       16
#define RDATDLY_SZ     2

#define RFIG          18

#define RCOMPAND      19
#define RCOMPAND_SZ    2

#define RWDLEN2       21
#define RWDLEN2_SZ     3

#define RFRLEN2       24
#define RFRLEN2_SZ     7

#define RPHASE        31

#define XWDLEN1        5
#define XWDLEN1_SZ     3

#define XFRLEN1        8
#define XFRLEN1_SZ     7

#define XDATDLY       16
#define XDATDLY_SZ     2

#define XFIG          18

#define XCOMPAND      19
#define XCOMPAND_SZ    2

#define XWDLEN2       21
#define XWDLEN2_SZ     3

#define XFRLEN2       24
#define XFRLEN2_SZ     7

#define XPHASE        31

/* Multi-channel Serial Port Sample Rate Gen Reg Bits                         */
#define CLKGDV         0
#define CLKGDV_SZ      8

#define FWID           8
#define FWID_SZ        8

#define FPER          16
#define FPER_SZ       12

#define FSGM          28

#define CLKSM         29

#define CLKSP         30

#define GSYNC         31

/* Multi-channel Serial Port Multi-Chan Ctrl Reg Bits                         */
#define RMCM           0

#define RCBLK          2
#define RCBLK_SZ       3

#define RPABLK         5
#define RPABLK_SZ      2

#define RPBBLK         7
#define RPBBLK_SZ      2

#define XMCM          16
#define XMCM_SZ        2

#define XCBLK         18
#define XCBLK_SZ       3

#define XPABLK        21
#define XPABLK_SZ      2

#define XPBBLK        23
#define XPBBLK_SZ      2

/* Multi-channel Serial Port Rec Enable Register Bits                         */

#define RCEA0          0
#define RCEA1          1
#define RCEA2          2
#define RCEA3          3
#define RCEA4          4
#define RCEA5          5
#define RCEA6          6
#define RCEA7          7
#define RCEA8          8
#define RCEA9          9
#define RCEA10        10
#define RCEA11        11
#define RCEA12        12
#define RCEA13        13
#define RCEA14        14
#define RCEA15        15

#define RCEB0         16
#define RCEB1         17
#define RCEB2         18
#define RCEB3         19
#define RCEB4         20
#define RCEB5         21
#define RCEB6         22
#define RCEB7         23
#define RCEB8         24
#define RCEB9         25
#define RCEB10        26
#define RCEB11        27
#define RCEB12        28
#define RCEB13        29
#define RCEB14        30
#define RCEB15        31

/* Multi-channel Serial Port TX Enable Register Bits                          */
#define XCEA0          0
#define XCEA1          1
#define XCEA2          2
#define XCEA3          3
#define XCEA4          4
#define XCEA5          5
#define XCEA6          6
#define XCEA7          7
#define XCEA8          8
#define XCEA9          9
#define XCEA10        10
#define XCEA11        11
#define XCEA12        12
#define XCEA13        13
#define XCEA14        14
#define XCEA15        15

#define XCEB0         16
#define XCEB1         17
#define XCEB2         18
#define XCEB3         19
#define XCEB4         20
#define XCEB5         21
#define XCEB6         22
#define XCEB7         23
#define XCEB8         24
#define XCEB9         25
#define XCEB10        26
#define XCEB11        27
#define XCEB12        28
#define XCEB13        29
#define XCEB14        30
#define XCEB15        31

#define MCBSP_RX      1
#define MCBSP_TX      2
#define MCBSP_BOTH    3

/* CONFIGURATION REGISTER BIT and BITFIELD values                             */
/* Serial Port Control Register SPCR                                          */

#define INTM_RDY            0x00     /* R/X INT driven by R/X RDY             */
#define INTM_BLOCK          0x01     /* R/X INT driven by new multichannel blk*/
#define INTM_FRAME          0x02     /* R/X INT driven by new frame sync      */
#define INTM_SYNCERR        0x03     /* R/X INT generated by R/X SYNCERR      */

#define DLB_ENABLE          0x01     /* Enable Digital Loopback Mode          */
#define DLB_DISABLE         0x00     /* Disable Digital Loopback Mode         */

#define RXJUST_RJZF         0x00     /* Receive Right Justify Zero Fill       */
#define RXJUST_RJSE         0x01     /* Receive Right Justify Sign Extend     */
#define RXJUST_LJZF         0x02     /* Receive Left Justify Zero Fill        */

/* Pin Control Register PCR                                                   */

#define CLKR_POL_RISING     0x01     /* R Data Sampled on Rising Edge of CLKR */
#define CLKR_POL_FALLING    0x00     /* R Data Sampled on Falling Edge of CLKR*/
#define CLKX_POL_RISING     0x00     /* X Data Sent on Rising Edge of CLKX    */
#define CLKX_POL_FALLING    0x01     /* X Data Sent on Falling Edge of CLKX   */
#define FSYNC_POL_HIGH      0x00     /* Frame Sync Pulse Active High          */
#define FSYNC_POL_LOW       0x01     /* Frame Sync Pulse Active Low           */

#define CLK_MODE_EXT        0x00     /* Clock derived from external source    */
#define CLK_MODE_INT        0x01     /* Clock derived from internal source    */

#define FSYNC_MODE_EXT      0x00     /* Frame Sync derived from external src  */
#define FSYNC_MODE_INT      0x01     /* Frame Sync dervived from internal src */

/* Transmit Receive Control Register XCR/RCR                                  */

#define SINGLE_PHASE        0x00     /* Selects single phase frames           */
#define DUAL_PHASE          0x01     /* Selects dual phase frames             */

#define MAX_FRAME_LENGTH    0x7f     /* maximum number of words per frame     */

#define WORD_LENGTH_8       0x00     /* 8 bit word length (requires filling)  */
#define WORD_LENGTH_12      0x01     /* 12 bit word length       ""           */
#define WORD_LENGTH_16      0x02     /* 16 bit word length       ""           */
#define WORD_LENGTH_20      0x03     /* 20 bit word length       ""           */
#define WORD_LENGTH_24      0x04     /* 24 bit word length       ""           */
#define WORD_LENGTH_32      0x05     /* 32 bit word length (matches DRR DXR sz*/

#define MAX_WORD_LENGTH     WORD_LENGTH_32

#define NO_COMPAND_MSB_1ST  0x00     /* No Companding, Data XFER starts w/MSb */
#define NO_COMPAND_LSB_1ST  0x01     /* No Companding, Data XFER starts w/LSb */
#define COMPAND_ULAW        0x02     /* Compand ULAW, 8 bit word length only  */
#define COMPAND_ALAW        0x03     /* Compand ALAW, 8 bit word length only  */

#define FRAME_IGNORE        0x01     /* Ignore frame sync pulses after 1st    */
#define NO_FRAME_IGNORE     0x00     /* Utilize frame sync pulses             */

#define DATA_DELAY0         0x00     /* 1st bit in same clk period as fsync   */
#define DATA_DELAY1         0x01     /* 1st bit 1 clk period after fsync      */
#define DATA_DELAY2         0x02     /* 1st bit 2 clk periods after fsync     */
  
/* Sample Rate Generator Register SRGR */

#define MAX_SRG_CLK_DIV     0xFF     /* max value to divide Sample Rate Gen Cl*/
#define MAX_FRAME_WIDTH     0xFF     /* maximum FSG width in CLKG periods     */
#define MAX_FRAME_PERIOD    0x0FFF   /* FSG period in CLKG periods            */

#define FSX_DXR_TO_XSR      0x00     /* Transmit FSX due to DXR to XSR copy   */
#define FSX_FSG             0x01     /* Transmit FSX due to FSG               */

#define CLK_MODE_CLKS       0x00     /* Clock derived from CLKS source        */
#define CLK_MODE_CPU        0x01     /* Clock derived from CPU clock source   */

#define CLKS_POL_FALLING    0x01     /* falling edge generates CLKG and FSG   */
#define CLKS_POL_RISING     0x00     /* rising edge generates CLKG and FSG    */

#define GSYNC_OFF           0x00     /* CLKG always running                   */
#define GSYNC_ON            0x01     /* CLKG and FSG synched to FSR           */ 

/******************************************************************************/
/* MCBSP_BYTES_PER_WORD - return # of bytes required to hold #                */
/*                        of bits indicated by wdlen                          */
/******************************************************************************/
#define MCBSP_BYTES_PER_WORD(wdlen) \
        ( (wdlen) == WORD_LENGTH_32 ? 4 : (int)(((wdlen) + 2) / 2) )

/******************************************************************************/
/* MCBSP_ENABLE(unsigned short port_no, unsigned short type) -                */
/*            starts serial port receive and/or transmit                      */
/*            type= 1 rx, type= 2 tx, type= 3 both                            */
/******************************************************************************/
#define MCBSP_ENABLE(port_no,type)\
     (*(unsigned int *)MCBSP_SPCR_ADDR(port_no) |= \
     ((type % 2) * MASK_BIT(RRST)) | ((type/2) * MASK_BIT(XRST)))

/******************************************************************************/
/* MCBSP_TX_RESET() - reset transmit side of serial port                      */
/******************************************************************************/
#define MCBSP_TX_RESET(port_no)\
     (*(unsigned int *)MCBSP_SPCR_ADDR(port_no) &= ~MASK_BIT(XRST))
    

/******************************************************************************/
/* MCBSP_RX_RESET() - reset receive side of serial port                       */
/******************************************************************************/
#define MCBSP_RX_RESET(port_no)\
    (*(unsigned int *)MCBSP_SPCR_ADDR(port_no) &= ~MASK_BIT(RRST))

/******************************************************************************/
/* MCBSP_READ() - read data value from serial port                            */
/******************************************************************************/
#define MCBSP_READ(port_no)\
     (*(unsigned int *)(MCBSP_DRR_ADDR(port_no)))

/******************************************************************************/
/* MCBSP_WRITE() - write data value to serial port transmit reg               */
/******************************************************************************/
#define MCBSP_WRITE(port_no, data)\
     (*(unsigned int *)(MCBSP_DXR_ADDR(port_no)) = (unsigned int) data)

/******************************************************************************/
/* MCBSP_IO_ENABLE() - place port in general purpose I/O mode                 */
/******************************************************************************/
#define MCBSP_IO_ENABLE(port_no) \
        { MCBSP_TX_RESET(port_no); MCBSP_RX_RESET(port_no); \
          RESET_FIELD(MCBSP_PCR_ADDR(port_no),RIOEN,2); }

/******************************************************************************/
/* MCBSP_IO_DISABLE() - take port out of general purpose I/O mode             */
/******************************************************************************/
#define MCBSP_IO_DISABLE(port_no) \
        SET_FIELD(MCBSP_PCR_ADDR(port_no),RIOEN,2)

/******************************************************************************/
/* MCBSP_FRAME_SYNC_ENABLE - sets FRST bit in SPCR                            */
/******************************************************************************/
#define MCBSP_FRAME_SYNC_ENABLE(port_no) \
        (SET_BIT(MCBSP_SPCR_ADDR(port_no),FRST))

/******************************************************************************/
/* MCBSP_FRAME_SYNC_RESET - clrs FRST bit in SPCR                             */
/******************************************************************************/
#define MCBSP_FRAME_SYNC_RESET(port_no) \
        (RESET_BIT(MCBSP_SPCR_ADDR(port_no),FRST))

/******************************************************************************/
/* MCBSP_SAMPLE_RATE_ENABLE - sets GRST bit in SPCR                           */
/******************************************************************************/
#define MCBSP_SAMPLE_RATE_ENABLE(port_no) \
        (SET_BIT(MCBSP_SPCR_ADDR(port_no),GRST))

/******************************************************************************/
/* MCBSP_SAMPLE_RATE_RESET - clrs GRST bit in SPCR                            */
/******************************************************************************/
#define MCBSP_SAMPLE_RATE_RESET(port_no) \
        (RESET_BIT(MCBSP_SPCR_ADDR(port_no),GRST))

/******************************************************************************/
/* MCBSP_RRDY - returns selected ports RRDY                                   */
/******************************************************************************/
#define MCBSP_RRDY(port_no) \
        (GET_BIT(MCBSP_SPCR_ADDR(port_no),RRDY))

/******************************************************************************/
/* MCBSP_XRDY - returns selected ports XRDY                                   */
/******************************************************************************/
#define MCBSP_XRDY(port_no) \
        (GET_BIT(MCBSP_SPCR_ADDR(port_no),XRDY))

/******************************************************************************/
/* MCBSP_LOOPBACK_ENABLE - places selected port in loopback                   */
/******************************************************************************/
#define MCBSP_LOOPBACK_ENABLE(port_no) \
        (SET_BIT(MCBSP_SPCR_ADDR(port_no),DLB))

/******************************************************************************/
/* MCBSP_LOOPBACK_DISABLE - takes port out of DLB                             */
/******************************************************************************/
#define MCBSP_LOOPBACK_DISABLE(port_no) \
        (RESET_BIT(MCBSP_SPCR_ADDR(port_no),DLB))

/*----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                  */
/*----------------------------------------------------------------------------*/

__INLINE void mcbsp_init(unsigned short port_no,
                         unsigned int spcr_ctrl,
                         unsigned int rcr_ctrl,
                         unsigned int xcr_ctrl, 
                         unsigned int srgr_ctrl,
                         unsigned int mcr_ctrl, 
                         unsigned int rcer_ctrl,
                         unsigned int xcer_ctrl, 
                         unsigned int pcr_ctrl);


#ifdef _INLINE
/******************************************************************/
/* mcbsp_init - initialize and start serial port operation        */
/*                                                                */
/******************************************************************/
static inline void mcbsp_init(unsigned short port_no,
                              unsigned int spcr_ctrl,
                              unsigned int rcr_ctrl,
                              unsigned int xcr_ctrl, 
                              unsigned int srgr_ctrl,
                              unsigned int mcr_ctrl, 
                              unsigned int rcer_ctrl,
                              unsigned int xcer_ctrl, 
                              unsigned int pcr_ctrl)
{
   unsigned int *port = (unsigned int *)(MCBSP_ADDR(port_no));

   /****************************************************************/
   /* Place port in reset - setting XRST & RRST to 0               */
   /****************************************************************/
   *(port + 2) 	&= ~(MASK_BIT(RRST) | MASK_BIT(XRST));

   /****************************************************************/
   /* Set values of all control reigsters                          */
   /****************************************************************/
   *(port + 3) = rcr_ctrl;
   *(port + 4) = xcr_ctrl;
   *(port + 5) = srgr_ctrl;
   *(port + 6) = mcr_ctrl;
   *(port + 7) = rcer_ctrl;
   *(port + 8) = xcer_ctrl;
   *(port + 9) = pcr_ctrl;

   *(port + 2) = ~(MASK_BIT(RRST) | MASK_BIT(XRST)) & (spcr_ctrl);
   *(port + 2) |= (MASK_BIT(RRST) | MASK_BIT(XRST)) & (spcr_ctrl);
}
    
#endif

#ifdef __INLINE
#undef __INLINE
#endif

#endif /* _MCBSP_H_ */
