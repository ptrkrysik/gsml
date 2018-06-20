/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dspinit.c
*
* Description:
*   This file contains DSP initialization functions
*
* Public Functions:
*   dsp_init    
*
* Private Functions:
*   modInit
*
******************************************************************************/
#include "stdlib.h" 
#include "string.h" 
#include "gsmdata.h" 
#include "oamdata.h" 
#include "dsp6201.h"               
#include "dsp/dsphmsg.h"
#include "comdata.h"
#include "bbdata.h"
#include "diagdata.h" 
#include "l1config.h"
#include "agcdata.h"
#include "dsprotyp.h"

/*****************************************************************************
*
* Function: modInit
*
* Description:
*   One-time initialization of modulation buffers at startup
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/ 
void modInit(void)
{  
   memset((UChar*)&g_ULBBData,   0, sizeof(t_ULBBData));
   memset((UChar*)&g_DLBBData,   0, sizeof(t_DLBBData));
   memset((UChar*)g_UlBurstLog, 0, 156*sizeof(Int32));
}        

/*****************************************************************************
*
* Function: dsp_init
*
* Description:
*   One-time initialization of DSP parameters at startup
*
* Inputs: None  
*
* Outputs: None
*
******************************************************************************/
void dsp_init(void)
{
   short i, k;
                        
   /*---------------------------------------------
    * Assign Host port buffer pointers
    *--------------------------------------------*/
   hpiRxMsgPtr = (msgStruc *)  & rxMsgBuf;  
   HostMsgNumWord = 0;
   hpiTxMsgPtr = (msgStruc *)  & txMsgBuf; 
   DspMsgNumWord = 0;
            
   /*---------------------------------------------
    * Create message queues (since we do not yet
    * use Fast Queues, set their lengths to 1)
    *--------------------------------------------*/
   HmsgQue = (qCmdStruc *) qCreate(20, 0, HmsgQue);
   FastHmsgQue = (qCmdStruc *) qCreate(1, 1, FastHmsgQue);

   RspQue = (qRspStruc *) qCreate(40, 0, RspQue);    
   FastRspQue = (qRspStruc *) qCreate(1, 1, FastRspQue);      
  
   /*---------------------------------------------
    * Initialize modulator buffers, group and
    * frame structures, codec, OAM
    *--------------------------------------------*/
   modInit(); 
   tsGroupInit();	                            
   tchFrameInit();
   codecInit();
   oamInit();
   
   /*---------------------------------------------
    * Initialize BER and Viterbi metricthresholds
    * for perceptual voice masking
    *--------------------------------------------*/
   g_TchBerThresh          = TCH_BER_THRESH;
   g_TchMetricThresh       = TCH_METRIC_THRESH; 
   g_RachMetricThresh      = RACH_METRIC_THRESHOLD;
   g_RachSnrThresh         = RACH_SNR_THRESHOLD;
   
   /*---------------------------------------------
    * Initialize UL dynamic power control, 
    * UL AGC, DL Tx temperature compensation  
    *--------------------------------------------*/
   g_DPC_ON = TRUE;
   g_AGC_ON = TRUE;                          
   g_USGC   = RXGAIN_INIT;
   g_TxPwrTemperatureDelta = 0;
   g_AgcRxPwrTarget = RXPWR_TARGET;     /* current, dBr */
   g_DpcRxLevThresh = DPC_DBM_THRESH + DBM_2_RXLEV_CONV_FACTOR;
                        
   /*---------------------------------------------
    * Init Tx power adjustment for current ARFCN
    *--------------------------------------------*/
   g_CurrentTxPwrOffsetForArfcn = 0;
   
   /*---------------------------------------------
    * Send no Tx signal on inactive TS. Override 
    * TrxMsgProc if SLOT_ACTIVE configures BCCH.
    *--------------------------------------------*/
   g_IdleFrameMode = FALSE;

   /*---------------------------------------------
    * Initialize UL DC offset for each TS and
    * also the all-carrier DC offset (last one)  
    *--------------------------------------------*/
   for ( i=0; i<NUM_RFS_PER_DSP; i++ )
   {
      for ( k=0; k<NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS+1; k++ )
      {
         g_DcOffset[i][k].I = 0;
         g_DcOffset[i][k].Q = 0;
      }

      for (k = 0; k< NUM_SDCCH8_SUBCHANS; k++) {
         g_Dc_7_Offset[i][k].I = 0;
	 g_Dc_7_Offset[i][k].Q = 0;
      }
   }
   /*---------------------------------------------
    * Initialize diagnostics and error events
    *--------------------------------------------*/
   g_diagData.msReportTsMask          = 0;
   g_diagData.dcchReportOnly          = FALSE;
   g_diagData.hoReportTsMask          = 0;
   g_diagData.dlMeasReportTsMask      = 0;
   g_diagData.gprsMsReportTfiMask     = 0;
   g_diagData.gprsDlMeasReportTfiMask = 0;
   g_diagData.dcOffsetTsMask          = 0xff;
   g_diagData.dcOffsetReport          = FALSE;
   g_diagData.burstCapture.state      = CAPTURE_OFF;
   g_diagData.sendRadioLinkLost       = TRUE;
   g_diagData.sendIdleFrameCapture    = FALSE;
   for ( i=0; i<NUM_RFS_PER_DSP; i++ )
   {
      for ( k=0; k<NUM_TN_PER_RF; k++ )
      {
         g_DlUnderflowCount[i][k] = 0;
         g_DlOverflowCount[i][k] = 0;
         g_DlOutOfSequenceCount[i][k] = 0;
      }
      for ( k=0; k<NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS; k++ )
      {
         g_diagData.msReport[i][k].cumErrs = 0;
         g_diagData.msReport[i][k].cumBits = 0;
         g_diagData.msReport[i][k].cumFrameErrs = 0;
         g_diagData.msReport[i][k].cumFrames = 0;
         g_diagData.msReport[i][k].sacchCount = 0;
         g_diagData.msReport[i][k].minToa = 99;
         g_diagData.msReport[i][k].maxToa = -99;
         g_diagData.msReport[i][k].cumToa = 0;
       }

      /*  initialize reports for config 7 msReport */
   
      for ( k=0; k< NUM_SDCCH8_SUBCHANS; k++ )
      {
         g_diagData.sdcch8Report[i][k].cumErrs = 0;
         g_diagData.sdcch8Report[i][k].cumBits = 0;
         g_diagData.sdcch8Report[i][k].cumFrameErrs = 0;
         g_diagData.sdcch8Report[i][k].cumFrames = 0;
         g_diagData.sdcch8Report[i][k].sacchCount = 0;
         g_diagData.sdcch8Report[i][k].minToa = 99;
         g_diagData.sdcch8Report[i][k].maxToa = -99;
         g_diagData.sdcch8Report[i][k].cumToa = 0;
       }
   }

   for ( i=0; i<8; i++ )
   {
      rcvRawIQBufIndex[i] = 0;
      txRawIQBufIndex[i] = 0;
   }            
}  
   
