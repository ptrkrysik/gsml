/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: frameisr.c
*
* Description:
*   This file contains the function that enables the BSP and synchronizes DMA
*   tranfers to the GSM frame clock. Also contains the function that increments
*   GSM frame numbers.
*
* Public Functions:
*   frameProc
*
* Private Functions:
*   gsmFrameIsr
*
******************************************************************************/
#include "stdlib.h" 
#include "gsmdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"
#include "dsp6201.h" 
#include "mcbsp.h"                        
#include "intr.h"
        
Int32 dlFrameCounter = 0;
Int32 dlFrameBurstError = 0;
extern Int32 dlBurstCounter;
extern Int32 ulBurstCounter;
/* *Int32 dlFrameBurstBuf[0x80];
 Int32 dlFrameBurstBufIndex = 0; */
 
/*****************************************************************************
*
* Function: gsmFrameIsr
*
* Description:
*   Enables the BSP and synchronizes DMA tranfers to the GSM frame clock
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void interrupt gsmFrameIsr(void)
{
    Uint8 timeSlot;
	frameProc();
	
	if(sysFrameNum.t2 == 0 && sysFrameNum.t3 == 0)
	{
		/*
		* if it the time to set frame number, system needs to be synchronized
		* according to frame number given.
		*/
		if(g_frmSync.state == SET_SYNC)
		{              
	  		/* Enable MCBSP Rx */
	  		MCBSP_ENABLE(0,MCBSP_RX);
	  		/* Enable MCBSP Tx */
	  		MCBSP_ENABLE(0,MCBSP_TX);   
	                                  
			sysSyncProc();
             
			g_frmSync.state = CHECK_SYNC;
			dlBurstCounter = 0;
			ulBurstCounter = 0;		
        	dlFrameCounter =0;
        	dlFrameBurstError = 0;
			/*
			dlFrameBurstBufIndex = 0;
			dlFrameBurstBuf[dlFrameBurstBufIndex] = 0;
			*/
		
			/* CALIBRATE_CONTROL_REG = 0;*/  
			
    		g_burstProc.dlTN = 0;
    		
    		/*
    		*  process uplink Time Slot - 1 to avoid DMA Banking ???? Special Case
    		*/
    		g_burstProc.ulTN = (g_burstProc.dlTN + 8-g_frameCounts.TNNumInit) & 0x07 ; 

    		g_BBInfo[0].dlBurst = 0;    	
    		g_BBInfo[0].ulBurst = g_BBInfo[0].dlBurst + 1;
    		g_BBInfo[0].ulBBSmpl = ulBurstBufPtr[g_BBInfo[0].ulBurst];
    	        
			/*
			* disable frame ISR
			*/    	        
			INTR_DISABLE(CPU_INT7);
			
            /*
            *  Release Reset AD7002 Receiver for A/D Callibration;
            */
            AGC_SRC_SELECT_REG |= (1<<AD7002_RESET);
		}
		else if(g_frmSync.state == TS_RE_SYNC)
		{
  		   for(timeSlot=0; timeSlot<NUM_TS_PER_RF; timeSlot++)
		   {           
              tsFrameSync(0, timeSlot);
		   }		
		   g_frmSync.state = CHECK_SYNC;		   
		}
		
	}
		    	
}
                            
/*****************************************************************************
*
* Function: frameProc
*
* Description:
*   Increments GSM frame numbers
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void  frameProc(void)
{
	Uint8    timeSlot;
	Uint8    rfCh;
   t_BBInfo *bbInfo; 
	Int32 Temp;
    
    bbInfo = & g_BBInfo[0];

	/*
	* update frame number
	*/
	if(++sysFrameNum.FN >= T1_FRAMES*T2_FRAMES*T3_FRAMES)
	{
	   sysFrameNum.FN = 0;
	  
	}

	if(++sysFrameNum.t2 >= T2_FRAMES) sysFrameNum.t2 = 0;

	if(++sysFrameNum.t3 >= T3_FRAMES)
 {
       
       sysFrameNum.t3 = 0;
       /*
       *  check if paging configuration change is required
       */
       if(g_frmSync.newPageState == TRUE)
       {
          /*
          *  calculate PCH index
          */
          if( g_BBInfo[0].TNInfo[0].groupNum == GSM_GROUP_5)
          {
             Temp = (sysFrameNum.FN + bbInfo->frame.offset) / T3_FRAMES;

             Temp = Temp % (g_BBInfo[0].BSPaMfrms);		       
             bbInfo->frame.dlG5PagchSubCh = Temp * PAGE_BLOCKS_G5;
			 
             g_BBInfo[0].TNInfo[0].u.group5.dlPpchSubCh = bbInfo->frame.dlG5PagchSubCh;

             sendDiagMsg(100, 0, 0, 1, (Uint8*) & Temp) ;
          }

          g_frmSync.newPageState = FALSE;
       }
	}

	if(sysFrameNum.t2 == 0 && sysFrameNum.t3 == 0)
	{
		 sysFrameNum.t1++;
		 if(sysFrameNum.t1 >= T1_FRAMES)
		 {
		   sysFrameNum.t1 = 0;                                 
		    
        if(g_BBInfo[0].TNInfo[0].groupNum == GSM_GROUP_5)
        {
 	         g_BBInfo[0].TNInfo[0].u.group5.dlPpchSubCh = 0;
 	         sendDiagMsg(HYPERFRAME_ROLLOVER, 0, 0, 0, &rfCh); /* 5th arg to satisfy proto */  
 	      }
 	      if(sysFrameNum.FN != 0)
 	      {
	          ReportError(HYPERFRAME_ROLLOVER, 0, 0, 0, &rfCh); /* 5th arg to satisfy proto */   	        
 	      }
   }
		
   /*
   *  Blink LED
   */                        
   AGC_SRC_SELECT_REG ^= (1<<DSP_LED1); 
              
   /*
   *  check is a time slot has been reconfigued
   */                                
   if(g_frmSync.state == TS_RE_SYNC)
   {  
  		 for(timeSlot=0; timeSlot<NUM_TS_PER_RF; timeSlot++)
     {           
       tsFrameSync(0, timeSlot);
     }
        
		   g_frmSync.state = CHECK_SYNC;
		   sendDiagMsg(0x00, 0, timeSlot, 0, &rfCh); /* 5th arg to satisfy prototype */
    }
 }
    
    /*
    *  for DEBUG only
    dlFrameCounter++;
    if(g_burstProc.dlTN	!= 0)
    {
       dlFrameBurstError++; 
    }
    */    
}






















