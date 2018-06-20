/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: diagmsg.c
*
* Description:
*   This file contains functions that handle diag messages from Host to DSP.
*
* Public Functions:
*   diagMsgProc
*
* Private Functions:
*   none
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "dsp/dsphmsg.h"
#include "comdata.h"  
#include "bbdata.h"          
#include "diagdata.h" 
#include "dsp6201.h"                 
#include "intr.h"           
#include "agcdata.h"          
#include "dsprotyp.h"

extern Int8 g_dlDmaAdj;  /* from burstisr.c */

/*****************************************************************************
*
* Function: alignFrameProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/            
void alignFrameProc(Int16 frameOffset)
{       
    Uint8 rfCh; 
	t_TNInfo *ts;
	Uint8 timeSlot;    
    
   for(rfCh=0; rfCh < NUM_RFS_PER_DSP; rfCh++)                        
   {    
      for(timeSlot=0; timeSlot<NUM_TN_PER_RF; timeSlot++) 
      {
	     /*
	     * find right time slot
	     */
         ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
         ts->ulTblIndex += (frameOffset+ts->tblLength);
         if(ts->ulTblIndex > ts->tblLength)
            ts->ulTblIndex -= ts->tblLength;     
         if(ts->ulTblIndex > ts->tblLength)
            ts->ulTblIndex -= ts->tblLength;            
      }
   }       
}
        
/*****************************************************************************
*
* Function: diagMsgProc
*
* Description:
*   Handles diagnostic messages from Host to DSP.
*
* Inputs:  
*   diagCmd           ptr to diagnostic message structure
*   ->whatever          fill in     
*
* Outputs:
*   dspId             sets the DSP's identifier value  
*
******************************************************************************/            

ProcResult diagMsgProc(cmdStruc *diagCmd)
{          
  ProcResult Result;
  rspStruc CommRspMsg;             
  Int8 *Ptr;
  Uint8 *rdPtr;
  Uint8 *wrPtr;
  Int16 i;
  Uint16 msgType;
  Uint16 rspMsgType;
  Uint16 oldLpbkMode;
  Int16 frameOffset;
  Uint8 reqLength;
  t_TNInfo *ts;

  Result = SUCCESS;
  msgType = diagCmd->typeLoByte |  (diagCmd->typeHiByte << 8);
       
  switch(msgType)
  {                        
   case MPH_DSP_DIAG_GET_DSP_QUE_STATUS_REQ:
      getRspQueStatus(diagCmd);
      break;
                         
   case MPH_DSP_DIAG_PING_REQ:
      CommRspMsg.function = DIAGNOSTIC_MSG; 
      rspMsgType = MPH_DSP_DIAG_PONG_RSP;
      CommRspMsg.typeLoByte =  rspMsgType;
      CommRspMsg.typeHiByte =  rspMsgType >> 8; 
      
      CommRspMsg.msgSize = MSG_HEAD_SIZE;
      sendMsgHost(& CommRspMsg);
      break;
      
   case MPH_DSP_DIAG_ECHO_MSG_REQ:              /* send back payload as response message */
      if ( diagCmd->params[0] < MSG_HEAD_SIZE )
      {
         CommRspMsg.msgSize = MSG_HEAD_SIZE;
      }
      else if ( diagCmd->params[0] > HPI_MSG_SIZE-MSG_HEAD_SIZE )
      {
         CommRspMsg.msgSize = HPI_MSG_SIZE-MSG_HEAD_SIZE;
      }
      else
      {
         CommRspMsg.msgSize = diagCmd->params[0];
      }             
      rdPtr = diagCmd->params + 4; /* this +4 skips requested count in Echo Request */
      wrPtr = & CommRspMsg.function;
      for(i=0; i<CommRspMsg.msgSize+4; i++) /* this +4 copies function, type and trx */
      { 
         *wrPtr++ = *rdPtr++;
      }
      sendMsgHost(& CommRspMsg);
      break;                  
      
   case MPH_DSP_SET_ID_REQ:       /* set DSP ID is the 1st command from host */
      DspId = diagCmd->params[0];
      break; 
      
   case MPH_DSP_DIAG_REPORT_REQ:
      switch ( diagCmd->params[0] )
      {
      case MS_REPORT:
         g_diagData.msReportTsMask = diagCmd->params[1];
         /*--- dcchReport will have one additional parameter ---*/
         if ( diagCmd->msgSize > 6 && diagCmd->params[2]==TRUE )
         {
            g_diagData.dcchReportOnly = TRUE;
         }
         else
         {
            g_diagData.dcchReportOnly = FALSE;
         }
         break;
         
      case HO_REPORT:
         g_diagData.hoReportTsMask = diagCmd->params[1];
         break;
         
      case DL_MEAS_REPORT:
         g_diagData.dlMeasReportTsMask = diagCmd->params[1];
         break;    
      
      case GPRS_MS_REPORT:
         g_diagData.gprsMsReportTfiMask = (diagCmd->params[1]<<24) |
                                          (diagCmd->params[2]<<16) |
                                          (diagCmd->params[3]<< 8) |
                                          (diagCmd->params[4]) ;
         break;

      case GPRS_DL_MEAS_REPORT:
         g_diagData.gprsDlMeasReportTfiMask = (diagCmd->params[1]<<24) |
                                              (diagCmd->params[2]<<16) |
                                              (diagCmd->params[3]<< 8) |
                                              (diagCmd->params[4]) ;
         break;    
      
      case DC_OFFSET_REPORT:
         if(diagCmd->params[1] == 0)
            g_diagData.dcOffsetReport = FALSE;
         else
            g_diagData.dcOffsetReport = TRUE;
         g_diagData.dcOffsetTsMask = diagCmd->params[2];
         break;    
      }  
      break;

   case MPH_DSP_DIAG_SET_AGC_DBR_TARGET:  
      g_AgcRxPwrTarget = (Short) ((diagCmd->params[0] << 8) | (diagCmd->params[1]));
      break;   

   case MPH_DSP_DIAG_BURST_CAPTURE_REQ:
      g_diagData.burstCapture.state = CAPTURE_ARMED;
      g_diagData.burstCapture.rfCh = diagCmd->trx; 
      g_diagData.burstCapture.timeSlot = diagCmd->params[0]; 
      break;     

   case MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ:
      if(diagCmd->params[0] == 0)
         g_diagData.sendIdleFrameCapture = FALSE;
      else
         g_diagData.sendIdleFrameCapture = TRUE;
      break;

   case MPH_DSP_DIAG_SET_MASKING_THRESHOLDS:
      g_TchBerThresh = diagCmd->params[0];
      g_TchMetricThresh = (Short) ((diagCmd->params[1] << 8) | (diagCmd->params[2]));;
      break;     

    case MPH_DSP_SET_LOOPBACK_MODE_REQ:

       /*g_frmSync.state = SET_SYNC;	                */
 
       switch(g_loopBackMode)
       {
         case NO_LOOPBACK:
         case DSP_TCH_LOOPBACK:
         case DSP_IQ_LOOPBACK:  
         case DSP_TCH_TO_HOST_LOOPBACK:         
           if (diagCmd->params[0] == BASEBAND_LOOPBACK)
	       {
             /* g_frameCounts.TNNumInit = 2;*/
             /*g_BBInfo[diagCmd->trx].ulBurst += 3;*/
             frameOffset = 3;

             INTR_GLOBAL_DISABLE();
             /*g_BBInfo[diagCmd->trx].ulBurst &= 3;        */
             alignFrameProc(frameOffset);  
             INTR_GLOBAL_ENABLE();       
           }
           g_loopBackMode = diagCmd->params[0];           
           break; 
                         
                         
         case BASEBAND_LOOPBACK:
           if(diagCmd->params[0] != BASEBAND_LOOPBACK)
           {
             frameOffset = -3;

             INTR_GLOBAL_DISABLE();
             /*g_BBInfo[diagCmd->trx].ulBurst &= 3;        */
             alignFrameProc(frameOffset);  
             INTR_GLOBAL_ENABLE();                 
           } 
           g_loopBackMode = diagCmd->params[0];           
           break;                              
           
         default:
           break;
       }          
       CommRspMsg.msgSize = MSG_HEAD_SIZE+1;              
       CommRspMsg.function = DIAGNOSTIC_MSG; 
       CommRspMsg.trx =  diagCmd->trx;        
       CommRspMsg.typeLoByte =  MPH_DSP_SET_LOOPBACK_MODE_RSP;
       CommRspMsg.typeHiByte =  MPH_DSP_SET_LOOPBACK_MODE_RSP >> 8; 
       CommRspMsg.params[0] = g_loopBackMode;
       sendMsgHost(& CommRspMsg);                   

       /*
       * initialize frame number counters
       
         INTR_DISABLE(CPU_INT7);                  
         INTR_DISABLE(CPU_INT8);
         setFrameNum.FN = 0;
         sysFrameNum.FN = 0;
         sysFrameNum.t1 = 0;
         sysFrameNum.t2 = 0;
         sysFrameNum.t3 = 0;
         g_frmSync.state == SET_SYNC;
         chanCodecInit(); 	   
         init_hw();
       */
                  
       break;  

    case MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST:
       if(diagCmd->params[0] == 0)
          g_diagData.sendRadioLinkLost = FALSE;
       else
          g_diagData.sendRadioLinkLost = TRUE;
       break;

    case MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN:
       if(diagCmd->params[0] == 0)
          g_RATE_TRAN_ON = FALSE;
       else
          g_RATE_TRAN_ON = TRUE;

       g_ULCCHData.chCodec = GPRS_CS_1;
       break;

    case MPH_DSP_DIAG_SWITCH_AGC:
       if(diagCmd->params[0] == 0)
          g_AGC_ON = FALSE;
       else
          g_AGC_ON = TRUE;
       break;
    
    case MPH_DSP_DIAG_SWITCH_POWER_CONTROL:   
       if(diagCmd->params[0] == 0)
          g_DPC_ON = FALSE;
       else
          g_DPC_ON = TRUE;
       break;

    case MPH_DSP_DIAG_DL_DMA_ADJ:
       g_dlDmaAdj = diagCmd->params[0];
       break;

    case MPH_DSP_DIAG_CHK_QUEUE_REQ:
       CommRspMsg.msgSize = MSG_HEAD_SIZE+12;              
       CommRspMsg.function = DIAGNOSTIC_MSG; 
       CommRspMsg.trx =  diagCmd->trx;        
       CommRspMsg.typeLoByte =  MPH_DSP_DIAG_CHK_QUEUE_RSP;
       CommRspMsg.typeHiByte =  MPH_DSP_DIAG_CHK_QUEUE_RSP >> 8; 
       * (Int32 *) & ( CommRspMsg.params[0]) = RspQue->msgOverflow;
       * (Int32 *) & ( CommRspMsg.params[4]) = HmsgQue->msgOverflow;   
       * (Int32 *) & ( CommRspMsg.params[8]) = FastRspQue->msgOverflow;
       * (Int32 *) & ( CommRspMsg.params[12]) = FastHmsgQue->msgOverflow;   
       sendMsgHost(& CommRspMsg);                   
       break;

   case MPH_DSP_DIAG_SET_DUMMY_BURST_MODE:
      if(diagCmd->params[0] == 0)
          g_IdleFrameMode = FALSE;
      else
          g_IdleFrameMode = TRUE;  /* send dummy bursts when inactive */
      break;
       
   case MPH_DSP_DIAG_TBF_ACT:


      /*---------------------------------------------
      * Initialize diagnostic GPRS MS Report parameters
      *--------------------------------------------*/
      initGprsMsReport(diagCmd->trx, g_tbfInfo[0][0].ulTbf.tfi);


      /*
      *  init a TBF pair g_tbfInfo[trx][Serial Number].
      */                           
      g_tbfInfo[0][0].dlTbf.state = CHAN_ACTIVE;      
      g_tbfInfo[0][0].dlTbf.tfi = 0;
      g_tbfInfo[0][0].dlTbf.timeSlotAllocation = 1 << diagCmd->params[0];
      g_tbfInfo[0][0].dlTbf.channelCodec = diagCmd->params[1];
            
      g_tbfInfo[0][0].ulTbf.state = CHAN_ACTIVE; 
      g_tbfInfo[0][0].ulTbf.tfi = 0;
      g_tbfInfo[0][0].ulTbf.tbfMode = TBF_DYNAMIC;
      g_tbfInfo[0][0].ulTbf.timeSlotAllocation = 1<<diagCmd->params[0];   /* USF diagCmd->params[1] */
      g_tbfInfo[0][0].ulTbf.opMode.dynamic.usfTsMap =  0; //diagCmd->params[1]  << (diagCmd->params[0]<<2); /*    0x00050000;      USF 5 on Time Slot 4 */
      g_tbfInfo[0][0].ulTbf.channelCodec = diagCmd->params[1];
      g_tbfInfo[0][0].ulTbf.tai = 0;
      g_tbfInfo[0][0].ulTbf.taiTimeSlot = diagCmd->params[0];
      g_tbfInfo[0][0].ulTbf.timingAdvance = 0;
      g_tbfInfo[0][0].ulTbf.sigqual.berAve = 0;
      g_tbfInfo[0][0].ulTbf.sigqual.blerAve = 0;
      g_tbfInfo[0][0].ulTbf.sigqual.berAve = 0;
      g_tbfInfo[0][0].ulTbf.sigqual.blerAve = 0;

      if ((diagCmd->params[2] & 0x01) == 1) 
      {
         ts = & g_BBInfo[diagCmd->trx].TNInfo[diagCmd->params[0]];

         g_tbfInfo[0][0].ulTbf.opMode.dynamic.usfTsMap = (diagCmd->params[2] << 24) | 
                                                         (diagCmd->params[3] << 16) |
                                                         (diagCmd->params[4] <<  8) | 
                                                         (diagCmd->params[5]);  /* ex: Usf=3 on TS3 */
         ts->u.group13.nextUsf = (g_tbfInfo[0][0].ulTbf.opMode.dynamic.usfTsMap >> 
                                       (2 << diagCmd->params[0])) & 0x07;
         ts->u.group13.currentUsf = ts->u.group13.nextUsf;
      }

      sendDiagMsg(0x99, diagCmd->trx, diagCmd->params[0], 1, & diagCmd->params[1] );

	  break;


    default:
      ReportError(UNKNOWN_CMD, diagCmd->trx, 0, 2, & diagCmd->typeHiByte);
      Result = FAILURE;
      break;
  }
  return(Result);
} 

