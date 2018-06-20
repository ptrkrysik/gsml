/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/***************************************************************************
* file name: dsp62x.h               
*
****************************************************************************/
        
/*
#define DISABLE_INT asm(" ssbx intm")
#define ENABLE_INT asm(" rsbx intm")

#define SET_XF asm(" ssbx xf")
#define CLR_XF asm(" rsbx xf")
*/                         

#define UL_DSP_AGC_CONTROL_REG *(Int16 *) 0x01000000

#define HW_AGC_CONTROL_REG *(Int16 *) 0x01010000
                                   
/*
*  dsp fpga control reqister
*/                                   
#define AGC_SRC_SELECT_REG *(Int16 *) 0x01020000
/*
*  bits defined in "AGC_SRC_SELECT_REG"
*/
#define FPGA_GAIN_RAMPING_TEST    0  /* 0: Disabled; 1: Enabled */


#define DSP_FIXED_GAIN_CTRL_FLAG  1   /* 1: SW control; 0: HW control */
     
/*
Bit 4:	set flag for FPGA to take ARFCH value
	0=inactive
	1=active
*/
#define DSP_FPGA_ARFCH_CTRL       4

     
/*
Bit 5:	set flag for sync to other DSP
	This will identify that the FPGA on the DSP you are using will mark itself
	as READY TO RECEIVE interrupts.
	Both FPGAs will have to have this bit set in order to receive interrupts.

	0=inactive
	1=active
*/
#define DSP_BSP_INT_SYNC_READY    5  /* 1: ready to receive; 0: inactive */

/*
    Bit 6:	allows the corresponding DSP to receive interrupts no matter if 
    the other DSP has identified itself as READY 
	to RECEIVE interrupts.

	0=inactive
	1=active
*/
#define DSP_BSP_INT_SYNC_CONTROL  6  /* 1: sync disabled; 0: Sync required */ 


#define AD7002_RESET  3  /* */ 

#define AD7002_RESET_COMPLETE  3  /* */ 


#define DSP_LED0  7  /**/ 
#define DSP_LED1  8  /**/ 

#define DL_PWR_CONTROL_REG *(Int16 *) 0x01030000

#define DL_ARFCN_SELECT_REG_2A *(Int16 *) 0x01040000
#define DL_ARFCN_SELECT_REG_2B *(Int16 *) 0x01050000
#define DL_ARFCN_SELECT_REG_1 *(Int16 *) 0x01060000 

/* bit 9:0 ARFCH value */

/*   bit 12 ENABLE PA */
#define POWER_AMP_SWITCH    12
   
/*   bit 13 Freq HopPing enable */
#define FREQ_HOPPING_ENABLE 13
   
/*   bit 14 TRX(1/2) select register */
#define TRX_SELECT          14     

/*   bit 15 SYNCTH(A/B) select register */  
#define SYNTH_AB_SELECT     0x8000

  
/*  Reserved
  #define UL_ARFCN_SELECT_REG  *(Int16 *) 0x01050000
*/

#define SYNC_CONTROL_REG *(Int16 *) 0x01060000
/* bit 0 of SYNC_CONTROL_REG(Read): 0-> in reset; 1: done */
#define SYNTH_SYNC_DONE     0                                

/* bit 1 of SYNC_CONTROL_REG(Write): 0-> no effect; 1: start sync */
#define SYNTH_SYNC_ENBALE   1                                

                                                  
/* a count of the offset between GSM burst Clock and TX DMA completion event */                                                  
#define DMA_BURST_SYNC_COUNT *(Uint16 *) 0x01070000

/* LED used in burstisr.c */                                                  
#define LED0 *(volatile Uint32 *) 0x01380000


