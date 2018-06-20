/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/******************************************************************************/
/*  init_hw2.c                                                                */
/*                                                                            */
/*  Description                                                               */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*                                                                            */
/*  FUNCTIONS:                                                                */
/*                                                                            */
/*  STATIC FUNCTIONS:                                                         */
/*                                                                            */
/*  GLOBAL VARIABLES DEFINED:                                                 */
/*                                                                            */
/******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h" 
#include "bbdata.h"        
#include "dsp6201.h"
#include "diagdata.h" 
#include "dsprotyp.h"
                  
#include <common.h>
#include <mcbsp.h>      /* mcbsp devlib                                       */
#include <mcbspdrv.h>   /* mcbsp driver                                       */
#include <dma.h>
#include <intr.h>
    
#define BURST_TO_FRAME_ADJ (-26)       /* adjust alignment for agc */
/*#define INTERNAL_BSP_CLK*/

static int numBuffsSent = 0;
static int numBuffsRcv = 0;
static int buffNumXmt = 0;
static int buffNumRcv = 2;   /* first buff ISR sets up is 2 */

extern int dlBurstCounter;
extern int ulBurstCounter;
     
/* Don't know where to find this prototype. It's for a function in dev6x.lib */
void mcsp_init (unsigned short port_no,
                unsigned int spcr_ctrl,
                unsigned int rcr_ctrl,
                unsigned int xcr_ctrl, 
                unsigned int srgr_ctrl,
                unsigned int mcr_ctrl, 
                unsigned int rcer_ctrl,
                unsigned int xcer_ctrl, 
                unsigned int pcr_ctrl);

/******************************************************************************/
/* init_dma - initialize DMA                                                  */
/******************************************************************************/  
void init_dma(void)
{
  dma_reset();

  /* Set up Global Configuration Registers for the DMA */
  dma_global_init(0,          /* gcr - no used */
                  0x00010138, /* gcra */
                  0x00010271, /* gcrb */
                  0xF6400004, /* gndxa  Frame Count: -625, element count 4*/
                  0, /* gndxb */                  
                  0,          /* gaddra */
                  (unsigned int)dlBurstBufPtr[1],  /* gaddrb */
                  (unsigned int)ulBurstBufPtr[0],   /* gaddrc */
                  0);         /* gaddrd */
  
  /*----------------------------*/
  /* Set up DMA Ch0 to Transmit */
  /*----------------------------*/
  dma_init(DMA_CH0, 
           0x13600010,  /* pri_ctrl - */
           0x00050080,  /* output Block event to DMAC Pin, sec_ctrl */
           (unsigned int)dlBurstBufPtr[0],   /* src_addr */
           0x018C0004,  /* dst_addr */
           316 + BURST_TO_FRAME_ADJ); /* first DAM trans_ctr */

  intr_map(CPU_INT8, ISN_DMA_INT0);
  intr_hook(dma0Isr, CPU_INT8);
  INTR_ENABLE(CPU_INT8);  
  
  /* Begin DMA operation */
  DMA_AUTO_START(DMA_CH0);

  /*---------------------------*/
  /* Set up DMA Ch1 to Receive */
  /*---------------------------*/
  dma_init(DMA_CH1, 
           0x8B035040,  /* pri_ctrl - */
           0x00050080,  /* output Block event to DMAC Pin, sec_ctrl */
           0x018C0000,   /* src_addr */
           (unsigned int)ulBurstBufPtr[0],  /* dst_addr */
           333 + BURST_TO_FRAME_ADJ/2); /* first DMA trans_ctr */

  intr_map(CPU_INT9, ISN_DMA_INT1);
  intr_hook(dma1Isr, CPU_INT9);
  /*INTR_ENABLE(CPU_INT9);*/

  /* Begin DMA operation */
  DMA_AUTO_START(DMA_CH1); 
 
	  /* Enable MCBSP Rx
	  MCBSP_ENABLE(0,MCBSP_RX);      */
	  /* Enable MCBSP Tx
	  MCBSP_ENABLE(0,MCBSP_TX);      */
}

/******************************************************************************/
/* init_bsp - initialize BSP 0                                                */
/******************************************************************************/
static init_bsp(void)
{
    /* Reset sample rate generator before configuring */
    MCBSP_SAMPLE_RATE_RESET(0);

    /* sample rate generator configuration requires tx and rx reset as well */
    MCBSP_TX_RESET(0);
    MCBSP_RX_RESET(0);

    /* reset frame sync generation counters */
    MCBSP_FRAME_SYNC_RESET(0);
    
#ifdef INTERNAL_BSP_CLK
    mcsp_init(0,     /* port no. */
               0x00008000, /* spcr_ctrl */ /* for test */
               0x002000A0, /* rcr_ctrl */  
               0x00200080, /* xcr_ctrl */
               0x20000120, /* srgr_ctrl */  /* for test */
               0x00000000, /* mcr_ctrl */
               0x00000000, /* rcer_ctrl */
               0x00000000, /* xcer_ctrl */ 
               0x00000B00 /* pcr_ctrl */ /* for test */
               );
                    
#else                    
    mcsp_init(0,     /* port no. */
               0x00000000, /* spcr_ctrl */
               0x002100A0, /* rcr_ctrl */  
               0x00210080, /* xcr_ctrl */
               0x000001FF, /* srgr_ctrl */
               0x00000000, /* mcr_ctrl */
               0x00000000, /* rcer_ctrl */
               0x00000000, /* xcer_ctrl */ 
               0x00000000 /* pcr_ctrl */
               );
               
#endif                                   
    /* Enable sample rate generator */
    MCBSP_SAMPLE_RATE_ENABLE(0);
}                     

/******************************************************************************/
/* init_hw - Initialize Hardware                                              */
/******************************************************************************/
void init_hw(void)
{
   int   i = 0x7ffffff; 
   Uint8 dummyArg;  
  /*
   Calibration Control Register 
   This register contains control bits related to the AD7002 calibration process.   
   For the bit "CALSet" a "1" written to this indicates to the hardware that it 
   should begin the Calibration process using the CAL line on the AD7002.  
   Writing a "0" will have not effect on the hardware.
   
   For the bit "MZEROset" a  "1" written to this indicates that the MZERO line 
   should be driven to high during the calibration (RF Analog Section disconnected).  
   Writing a "0" will leave the MZERO line low during the calibration process 
   (RF Analog Section connected).

   For the bit "Calibration Complete" a  "1" in this register indicates that the
   calibration process has completed.  This bit is only valid after setting of the 
   "CALset" bit..   A "0" read from this indicates that the hardware is waiting for 
   the calibration to complete if the "CALset" bit was previously written.  Upon 
   completion the "MZEROset" and "CALset" bits are set to "0".
  */
  AGC_SRC_SELECT_REG |= (1<<AD7002_RESET);
  
  if(g_loopBackMode != LOCAL_LOOPBACK)
  
  while (i-- > 0) {
    if (AGC_SRC_SELECT_REG & (1<<AD7002_RESET_COMPLETE)) 
      break;
  }
  
  if( i<= 0)
  {
     /*
     *  AD 7002 failed on Reset
     */     
     ReportError(0xFF, 0,  0,  0, &dummyArg);
  }
  
  init_bsp();  
     
 /* 
  sysSyncProc();
 
  g_frmSync.state = CHECK_SYNC;
*/		               
  /* start at Time Slot '0' */
/*
  g_burstProc.dlTN = 0;
  g_burstProc.ulTN = (g_burstProc.dlTN + 8-g_frameCounts.TNNumInit) & 0x07 ; 
    	
  g_BBInfo[0].ulBurst = (g_burstProc.ulTN+3) & 0x3;
  g_BBInfo[0].ulBBSmpl = ulBurstBufPtr[g_BBInfo[0].ulBurst];  
 
*/  
  
  init_dma();   
  
  intr_map(CPU_INT7, ISN_EXT_INT7);   
  intr_hook(gsmFrameIsr, CPU_INT7);
  
  INTR_ENABLE(CPU_INT7);
  INTR_GLOBAL_ENABLE();
                        
  /*
  *  DSP is ready to set sync flag
  */                        
  AGC_SRC_SELECT_REG |= 1<<DSP_BSP_INT_SYNC_READY;
}
               


