/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: burstisr.c
*
* Description:
*   This file contains functions that are invoked by the DMA interrupt at
*   each burst boundary.
*
* Public Functions:
*   dlBurstProcIsr, ulBurstProcIsr
*
* Private Functions:
*   dma0Isr, dma1Isr
*
******************************************************************************/
#include "stdlib.h" 
#include "gsmdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"                 
#include <mcbsp.h>      /* mcbsp devlib                                       */
#include <mcbspdrv.h>   /* mcbsp driver                                       */
#include <dma.h>
#include <intr.h>   
#include "dsp6201.h"
#include "agcdata.h"   
#include "diagdata.h"   
     
int   dlBurstCounter = 0;
int   ulBurstCounter = 0;
Int8  g_dlDmaAdj = 0;
Int16 g_dlDmaCount = 312;

#define IDLE_7  8

Uint8 Group5ChannelMapTbl[102] = {
3,3,3,3,R,R,2,2,2,2,3,3,3,3,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
0,0,0,0,1,1,1,1,R,R,2,2,2,2,
3,3,3,3,R,R,0,0,0,0,1,1,1,1,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
0,0,0,0,1,1,1,1,R,R,2,2,2,2 };


/* for Group7 Table 8 signifies idle frame  */

Uint8 Group7ChannelMapTbl[102] = {
5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,
5,5,6,6,6,6,7,7,7,7,0,0,0,0,
1,1,1,1,2,2,2,2,3,3,3,3,8,8,8,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,
5,5,6,6,6,6,7,7,7,7,4,4,4,4 };


/*****************************************************************************
*
* Function: dlBurstProcIsr
*
* Description:
*   Processes one downlink burst per RF channel
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/
void dlBurstProcIsr(void)
{   
   Void  (** frameProcPtr)();
   t_TNInfo    *tsInfo; 
   Int8        rfCh;	
   Uint32      *sample; 
   short       k;


    
   /*
   * SPECIAL CASE: only when frame ISR runs within Burst ISR
   */  
   static Uint8 firstFrmIsr = TRUE;
    
   if(firstFrmIsr != TRUE)
   {           
      if(g_burstProc.dlTN == 0) {
	frameProc();
      }
    	/*
    	*  allow resync of dma to burst clock
    	*/
   }
   else
   {
      /*
      *  At first frame ISR, gsmFrameIsr() sets GSM frame counters
      *  and runs frameProc()
      */    
      firstFrmIsr = FALSE;
   }    
    
   /*
   *  Process a burst from each RF Channel
   */
	for(rfCh=0; rfCh < NUM_RFS_PER_DSP; rfCh++)                        
	{

		/*
		*  Find right time slot and perform DL synchronous processing
		*/
		tsInfo = & g_BBInfo[rfCh].TNInfo[g_burstProc.dlTN];

		frameProcPtr = (Void *) & tsInfo->groupDLTable [tsInfo->dlTblIndex];
		(* frameProcPtr)(rfCh, g_burstProc.dlTN);
		        	
      /*
      *  If requested by Host, swap I and Q components to invert DL spectrum
      */
      
      if ( g_BBInfo[rfCh].IQSwap == TRUE )
      { 
         sample = (Uint32*)g_BBInfo[rfCh].dlBBSmpl;
               
         for ( k=RAMP_SAMPLES; k<312-RAMP_SAMPLES; k++ )
         {
            sample[k] = ((sample[k] & 0xfff) << 12) | ((sample[k] >> 12) & 0xfff);
         }
      }    
		/*
		*  Advance table index and burst buffer, and get new buffer size
		*/ 
		if(++tsInfo->dlTblIndex >= tsInfo->tblLength)
		{
			tsInfo->dlTblIndex = 0;
		}
		g_BBInfo[rfCh].dlBurst++;
		g_BBInfo[rfCh].dlBurst &= 1;		
		g_BBInfo[rfCh].dlBBSmpl = dlBurstBufPtr[g_BBInfo[rfCh].dlBurst];
	}
   g_burstProc.dlTN += 1;
   g_burstProc.dlTN &= 7; 
}

/*****************************************************************************
*
* Function: ulBurstProcIsr
*
* Description:
*   Processes one uplink burst per RF channel
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/
void ulBurstProcIsr(void)
{   
   Void  (** frameProcPtr)();
   t_TNInfo *tsInfo;
   Int32    i;
   Int8     rfCh;

   /*
   *  table to correct Uneven Ul burst buffer size to affect TOA on different TS
   */ 
   const Int8 taTimeSlotOffset[8] = {1+1, 1, 1, 1, 1+1, 1, 1, 1};   
   
   
#ifdef FIND_BURST_BOUNDARY
   static Int32 ulBurstDataIndex = 0;
   t_ULComplexSamp *rcvData;
#endif

   /*
   *  Process a burst from each RF Channel
   */
   for(rfCh=0; rfCh < NUM_RFS_PER_DSP; rfCh++)                        
   {     

#ifdef FIND_BURST_BOUNDARY
        /*
        * test code to find out burst boundary
        */
      rcvData = g_BBInfo[rfCh].ulBBSmpl;
      for(i=0; i<156; i++)              
      {
         if( (rcvData->I * rcvData->I + rcvData->Q * (rcvData++)->Q) > 0x4000*0x4000)
         {       
            LED0 = 3;
            if (ulBurstDataIndex < 156)
            {
               g_UlBurstLog[ulBurstDataIndex++] = (Int32 ) (rcvData-1);
            }
         }  
         }
      
      LED0 = 0;
#endif
        
		/*
		*  Find right time slot and perform DL synchronous processing
		*/

     
      tsInfo = & g_BBInfo[rfCh].TNInfo[g_burstProc.ulTN];
      frameProcPtr = (Void *) & tsInfo->groupULTable [tsInfo->ulTblIndex];
		(* frameProcPtr)(rfCh, g_burstProc.ulTN);
                                                      		
		/*
		*  Advance table index and burst buffer, and get new buffer size
		*/ 
		if(++tsInfo->ulTblIndex >= tsInfo->tblLength)
		{
			tsInfo->ulTblIndex = 0;
		}       
		g_BBInfo[rfCh].ulBurst++;
		g_BBInfo[rfCh].ulBurst &= 0x3;		
		g_BBInfo[rfCh].ulBBSmpl = ulBurstBufPtr[g_BBInfo[rfCh].ulBurst] + taTimeSlotOffset[g_burstProc.ulTN];
    }

   g_burstProc.ulTN++; 
   g_burstProc.ulTN &= 7;    	
}

/*****************************************************************************
*
* Function: setUplinkGain
*
* Description:
*   Sets uplink gain (load DAC value into FPGA register) for next time slot
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/
void setUplinkGain(void)
{   
   t_TNInfo *tsInfo;
   Int32    timeSlot;
   Uint8    subChan;
   Int8     t3index;
   Int8     t3frame;
   Int8     rfCh;
   tbfStruct   *TBF;   /* GPRS */

   /*
   *  Set uplink gain for one time slot on each RF channel
   */
   for(rfCh=0; rfCh < NUM_RFS_PER_DSP; rfCh++)                        
   {     
      /*--------------------------------------
      *  Automatic Gain Control
      *--------------------------------------*/
      if ( g_AGC_ON )
      {
         timeSlot = (g_burstProc.ulTN + AGC_TS_OFFSET) % NUM_TS_PER_RF;  
         tsInfo = & g_BBInfo[rfCh].TNInfo[timeSlot];

         switch(tsInfo->groupNum)
         {
         case GSM_GROUP_1: 
                             
            UL_DSP_AGC_CONTROL_REG = tsInfo->u.group1.sacch.ulsigstren.rxgain;   
            break;

         case GSM_GROUP_5:
            
            /*------------------------------------------------------------
            *  Find T3 frame number within superframe, compute frame
            *  index within block of 102, then look up RACH or subChan
            *------------------------------------------------------------*/
            t3frame = (sysFrameNum.t3 - sysFrameNum.t2 + T2_FRAMES) % T2_FRAMES;
            t3index = sysFrameNum.t3 + 51*(t3frame & 1);
            subChan = Group5ChannelMapTbl[t3index]; 

            /*------------------------------------------------------------
            *  First check for RACH
            *------------------------------------------------------------*/
            if ( subChan == R )
            {
               UL_DSP_AGC_CONTROL_REG = g_USGC;
            }
            /*------------------------------------------------------------
            *  Else use table value as SDCCH subchannel
            *------------------------------------------------------------*/
            else
            {
               UL_DSP_AGC_CONTROL_REG = tsInfo->u.group5.sacch[subChan].ulsigstren.rxgain; 
            }
            break;

         case GSM_GROUP_7:
            /*------------------------------------------------------------
            *  Find T3 frame number within superframe, compute frame
            *  index within block of 102, then look up subChan
            *------------------------------------------------------------*/
            t3frame = (sysFrameNum.t3 - sysFrameNum.t2 + T2_FRAMES) % T2_FRAMES;
            t3index = sysFrameNum.t3 + 51*(t3frame & 1);
            subChan = Group7ChannelMapTbl[t3index]; 

			
            if (subChan !=  IDLE_7){
               UL_DSP_AGC_CONTROL_REG = tsInfo->u.group7.sacch[subChan].ulsigstren.rxgain; 
	    }

         case GSM_GROUP_13:
         
			/* GPRS */
           if( !(TBF = ulChan2TbfMap(rfCh, timeSlot)) ) {
                   UL_DSP_AGC_CONTROL_REG = g_rxGainMinIdle;
           }

            else {
              UL_DSP_AGC_CONTROL_REG = TBF->ulTbf.sigstren.rxgain; 
            }

           break;
         }
        }

      /*--------------------------------------
      *  Manual Gain Control
      *--------------------------------------*/
      else 
      {  
         UL_DSP_AGC_CONTROL_REG = g_USGC;
      }
 }   
}

               
/******************************************************************************/
/* dma0Isr static variables 
/******************************************************************************/   
Uint32   dmaBurstOffsetCount = 0x7FFFFFFF;
Uint32   dmaBurstSlipCount = 0; 
Int16    dynamicDmaAdj = 0;      
Uint8    evenOdd;      
 
Uint8    g_maxCpuLoading = 0;

/*****************************************************************************
*
* Function: dma0Isr
*
* Description:
*   DMA0 Interrupt Service Routine
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/
interrupt void dma0Isr(void)
{ 
   /*		     LED0 = 3;   for EVM board only */  

   static Int8 dmaBurstResync = 0;
   static Int8 lockDmaAdj = 0;

   Int16  maxCpuLoading;
   Int16 adjustedDlPwrControlValue;
   
   /*
   *  get DMA transfer conter at beginning of the ISR, The counter should be
   *  reloade upon completion of a block DMA.
   */
   maxCpuLoading = DMA0_XFER_COUNTER;
   
   /* clear applicable condition bits in DMA secondary cntl                    */
   RESET_BIT(DMA0_SECONDARY_CTRL_ADDR, BLOCK_COND); 
  
   if(dmaBurstResync == 0)
   {
      if(dmaBurstOffsetCount - DMA_BURST_SYNC_COUNT == 12)
      {              
         /*
         *  the different is 12 13MHz clock period, take higher count as reference
         *  The reference will keep the same as the System is up
         */
         dmaBurstResync = 1;
         evenOdd = 0;
      }
      else
      {
         dmaBurstOffsetCount = DMA_BURST_SYNC_COUNT; 
      } 
      dynamicDmaAdj = 0;
   }
   else
   {  
      /*
      *   dynamicDmaAdj should be always be '0", if DMA and Burst Clock are in sync
      */
      dynamicDmaAdj = dmaBurstOffsetCount - DMA_BURST_SYNC_COUNT - evenOdd*12;

      /*
      *  flip evenOdd
      */      
      evenOdd ^= 0x01;
            
      if((dynamicDmaAdj > -12) && (dynamicDmaAdj < 12))
      {  
         /*
         *  no adjustment is required, if the different is whin one GSM Bit
         *  i.e, 2 TX samples
         */
          dynamicDmaAdj = 0;
      }
      else if(lockDmaAdj == 0)
      {   
          /*
          *  Transmit Data Sample rate is 541.6667 KSPS, the count is a 13 MHz clock
          *
          *  13M/14 = 541.6667
          *
          *   so adj in sample is count/12, or count*(32768/12)/32768
          *   and 32768/12 = 2730
          */
          dmaBurstSlipCount += 1;
          /* send both bytes of short integer */
          sendDiagMsg(0xEE, 0, 0, 2, (Uint8*)(&dynamicDmaAdj));
                    
          dynamicDmaAdj =  (dynamicDmaAdj * (2730/2) + 4) >> 14;
          lockDmaAdj = 8;
      }
      else if(lockDmaAdj > 0)
      {
         lockDmaAdj--;
         dynamicDmaAdj = 0;
      }
   }
   /*
   *  turn-off LED when enter Interrupt
   */ 
   AGC_SRC_SELECT_REG &=  ~(1<<DSP_LED0);

   DMA_GADDR_B =  (Uint32) g_BBInfo[0].dlBBSmpl;
/* 
#ifndef OVERSAMPLING_RATE_4X
   if(DMA_GCR_A == 312)
   
      DMA_GCR_A = 313 + g_dlDmaAdj; 
   else                                                             
 	  DMA_GCR_A = 312 + g_dlDmaAdj; 
#endif              
*/             
           
   if(g_dlDmaCount == 312)
   {              
      g_dlDmaCount = 313;
   }                                       
   else
   {
      g_dlDmaCount = 312;
   }
   DMA_GCR_A = g_dlDmaCount + g_dlDmaAdj + dynamicDmaAdj;
   
   /*
   *  DMA adjustment can be used once only, it should be cleared once it is used
   */
   g_dlDmaAdj = 0;
   
   /*
   * Set uplink gain control value in FPGA
   */
   setUplinkGain();
   
   /*
   * Set downlink gain control value in FPGA
   */
   adjustedDlPwrControlValue = g_DSPC + g_TxPwrTemperatureDelta;    
   if ( adjustedDlPwrControlValue > DL_PWR_CONTROL_MAX )
   {
      DL_PWR_CONTROL_REG = DL_PWR_CONTROL_MAX;
   }
   else if ( adjustedDlPwrControlValue < DL_PWR_CONTROL_MIN )
   {
      DL_PWR_CONTROL_REG = DL_PWR_CONTROL_MIN;
   }
   else
   {
      DL_PWR_CONTROL_REG = adjustedDlPwrControlValue;
   } 
            
   /*
   * Process the burst. This is where we do all of the work.
   */
   dlBurstProcIsr();
   ulBurstProcIsr();
   dlBurstCounter++; 
    
/* LED0 = 0;   for EVM board only */
   
   /*
   * turn-on LED when exit Interrupt
   */
   AGC_SRC_SELECT_REG |=  1<<DSP_LED0;

   /*
   *  measure the difference of the DMA counter readings and convert it to
   *  percentage value
   */
   maxCpuLoading = ((maxCpuLoading - DMA0_XFER_COUNTER) * 100) / 312;

   /*
   *  store the max reading
   */
   if( maxCpuLoading > g_maxCpuLoading && maxCpuLoading < 101)
   {
        g_maxCpuLoading = maxCpuLoading;
   }
}   

/*****************************************************************************
*
* Function: dma1Isr
*
* Description:
*   DMA1 Interrupt Service Routine - currently unused?
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/
interrupt void dma1Isr(void)
{
   Int32 temp;
    	            
  /* clear applicable condition bits in DMA secondary cntl                    */
  RESET_BIT(DMA1_SECONDARY_CTRL_ADDR, BLOCK_COND);  

  /*temp = (g_BBInfo[0].ulBurst+2) & 0x3;
  DMA_GADDR_C = (Uint32) ulBurstBufPtr[temp];
  
  if (temp == 0)
    DMA_GCR_B = 157;
  else
    DMA_GCR_B = 156;  
  */ 
   ulBurstCounter++; 
    		 
}




















