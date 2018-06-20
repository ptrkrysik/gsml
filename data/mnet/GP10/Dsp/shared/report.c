/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: report.c
*
* Description:
*   This file contains functions that report diagnostic messages and error
*   messages to the Host. Each enabled message is sent once per superframe.
*
* Public Functions:
*   ReportDiagToHost, ReportErrorToHost
*
* Private Functions:
*   ReportCapturedBurst
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"                
#include "oamdata.h"                
#include "dsp/dsphmsg.h"
#include "bbdata.h"            
#include "diagdata.h" 
#include "intr.h"
#include "dsprotyp.h"

#define MAX_DIAGS_PER_CALL 8  // Max diag report msgs to send per function call

/*****************************************************************************
*
* Function: ReportCapturedBurst
*
* Description:
*   Sends captured burst (raw I/Q samples) host
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void SendCapturedBurst(Uint8 rfCh, Uint8 timeSlot, Uint32 *data)
{
   msgStruc diagMsg;
   Int16    i, j;
   Int16    blockStartIndex;
   Uint32   *outBuf;

   diagMsg.msgSize   = 4*SAMPLES_PER_CAPTURE_MSG + 8;
   diagMsg.function  = DIAGNOSTIC_MSG;
   diagMsg.trx       = rfCh;
   diagMsg.typeLoByte= MPH_DSP_DIAG_BURST_CAPTURE_RSP;
   diagMsg.typeHiByte= MPH_DSP_DIAG_BURST_CAPTURE_RSP>>8;    
   diagMsg.params[0] = timeSlot;
   diagMsg.params[1] = 0;
   diagMsg.params[2] = 0;
   
   /*
   * Ready pointer to copy longwords
   */
   outBuf = (Uint32*)(&diagMsg.params[4]); 
   
   for(j=0; j<NUM_CAPTURE_MSGS; j++)
   {
      diagMsg.params[3] = j;      /* block number */
      blockStartIndex = j * SAMPLES_PER_CAPTURE_MSG; 
      for(i=0; i<SAMPLES_PER_CAPTURE_MSG; i++)
         outBuf[i] = data[i+blockStartIndex];     		            
      SendMsgHost_Que( & diagMsg);
   }
}

/*****************************************************************************
*
* Function: initMsReport
*
* Description:
*   Initializes MS report variables for one logical channel
*
* Input:  
*   rfCh            RF channel index
*   reportIndex     GSM time slot (Group1) or subchannel table index (Group5)
*
* Output:
*   g_diagData.msReport[][]  initialized structure variables        
*
******************************************************************************/
ProcResult initMsReport(Uint8 rfCh, Uint8 reportIndex)
{
   t_diagMsReport *msReport = & g_diagData.msReport[rfCh][reportIndex];

   msReport->cumErrs       = 0;
   msReport->cumBits       = 0;
   msReport->cumFrameErrs  = 0;
   msReport->cumFrames     = 0;
   msReport->sacchCount    = 0;
   msReport->minToa        = 99;
   msReport->maxToa        = -99;
   msReport->cumToa        = 0;
}

/*****************************************************************************
*
* Function: initGprsMsReport
*
* Description:
*   Initializes MS report variables for one logical channel
*
* Input:  
*   rfCh            RF channel index
*   reportIndex     GSM time slot (Group1) or subchannel table index (Group5)
*
* Output:
*   g_diagData.msReport[][]  initialized structure variables        
*
******************************************************************************/
ProcResult initGprsMsReport(Uint8 rfCh, Uint8 tfi)
{
   t_diagGprsMsReport *gprsMsReport;
   
   gprsMsReport = & g_diagData.gprsMsReport[rfCh][tfi];

   gprsMsReport->cumErrs       = 0;
   gprsMsReport->cumBits       = 0;
   gprsMsReport->cumFrameErrs  = 0;
   gprsMsReport->cumFrames     = 0;
   gprsMsReport->rlcBlkCount   = 0;
   gprsMsReport->minToa        = 99;
   gprsMsReport->maxToa        = -99;
   gprsMsReport->cumToa        = 0;
}


/*****************************************************************************
*
* Function: initSdcch8Report
*
* Description:
*   Initializes Sdcch8 report variables for one logical channel
*
* Input:  
*   rfCh            RF channel index
*   reportIndex     subchannel table index (Group7)
*
* Output:
*   g_diagData.sdcch8Report[][]  initialized structure variables        
*
******************************************************************************/
ProcResult initSdcch8Report(Uint8 rfCh, Uint8 reportIndex)
{
   t_diagMsReport *msReport = & g_diagData.sdcch8Report[rfCh][reportIndex];

   msReport->cumErrs       = 0;
   msReport->cumBits       = 0;
   msReport->cumFrameErrs  = 0;
   msReport->cumFrames     = 0;
   msReport->sacchCount    = 0;
   msReport->minToa        = 99;
   msReport->maxToa        = -99;
   msReport->cumToa        = 0;
}
   	      
/*****************************************************************************
* Function: ReportDiagToHost
*
* Description:
*   Sends diagnostic reports to host every superframe. Disables burst
*   interrupt, then re-enables when finished. 
*
******************************************************************************/
void ReportDiagToHost(void)
{
   Uint8          rfCh;									 
   Uint8          timeSlot;									 
   Uint8          tfi;									 
   Uint8          channelNumber;
   Uint8          reportIndex;									 
   Uint8          index;									 
   Uint16         count;
   Uint16         limit;
   t_SACCHInfo    *sacchInfo;
   t_diagMsReport *msReport;
   t_diagGprsMsReport *gprsMsReport;
   Uint8          params[48];
   Int16          shortValue;
   Uint8          diagSent;
   tbfStruct      *tbf;

   /*-------------------------------------------------------------
    * Send diagnostic message to host every superframe 
    *------------------------------------------------------------*/ 
   if ( sysFrameNum.t2==0 && sysFrameNum.t3==0 && g_diagData.enable )    
   {  
      /*-----------------------------------------------------------------
       * Disable further diagnostic reports for this superframe (in case
       * this function is called from inside and outside of an ISR)
       *----------------------------------------------------------------*/
      g_diagData.enable = FALSE;
      
      /*-----------------------------------------------------------------
       * Check for MS_REPORT diagnostic (TCH or SDCCH) from any TS.
       * If reports are not activated on any slot, skip this altogether. 
       *----------------------------------------------------------------*/ 
      if ( g_diagData.msReportTsMask )
      {         
         for ( rfCh=0; rfCh<NUM_RFS_PER_DSP; rfCh++ )
         {
            for ( timeSlot=0; timeSlot<NUM_TN_PER_RF; timeSlot++ )
            {
               /*-----------------------------------------------------------
               * Check whether report has been requested for this time slot 
               *-----------------------------------------------------------*/ 
               if ( g_diagData.msReportTsMask & (1<<timeSlot) )
               {
                  /*--------------------------------------------------------
                  * If this time slot is Group V, check NUM_SDCCH_SUBCHANS 
                  * subchannels of SDCCH. Else, check the one TCH channel. 
                  *--------------------------------------------------------*/ 
                  if ( g_BBInfo[rfCh].TNInfo[timeSlot].groupNum==GSM_GROUP_5 )
                  {
                     limit = NUM_SDCCH_SUBCHANS;
                  }     
                  else if ( g_BBInfo[rfCh].TNInfo[timeSlot].groupNum==GSM_GROUP_7)
                  {
                     limit = NUM_SDCCH8_SUBCHANS;
                  }
                  else
                  {
                     limit = 1;
                  }
                  /*--------------------------------------------------------
                  * Check the channel or each subchannel as determied above 
                  *--------------------------------------------------------*/ 
                  for ( count=0; count<limit; count++ )
                  { 
		               switch( g_BBInfo[rfCh].TNInfo[timeSlot].groupNum)
                     {
                     /*--------------------------------------------------------
                     * If Group 1 TCH, set its SACCH Info and MS Report pointers 
                     *--------------------------------------------------------*/ 
                     case GSM_GROUP_1:
                        sacchInfo = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.sacch;
                        reportIndex = timeSlot;
                        msReport = & g_diagData.msReport[rfCh][reportIndex];
                        channelNumber = timeSlot;
			               break;
	       
                     /*---------------------------------------------------------------
                     * If Group 5 SDCCH, use count as the subchannel in setting SACCH
                     * Info and MS Report pointers, eventually hitting all of them  
                     *---------------------------------------------------------------*/ 
                     case GSM_GROUP_5:
                        sacchInfo = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group5.sacch[count];
                        reportIndex = NUM_TN_PER_RF + count;
                        msReport = & g_diagData.msReport[rfCh][reportIndex];
                        channelNumber = (1<<5) | (count<<3) | timeSlot;
                        break;

                     /*---------------------------------------------------------------
                     * If Group 7 SDCCH, use count as the subchannel in setting SACCH 
                     * Info and SDCCH8 Report pointers, eventually hitting all of them  
                     *---------------------------------------------------------------*/ 
                     case GSM_GROUP_7:	                              
                        sacchInfo = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group7.sacch[count];
                        reportIndex =  count;
                        msReport = & g_diagData.sdcch8Report[rfCh][reportIndex];
                        channelNumber = (1<<6) | (count<<3) | timeSlot;
                        break;
                     

                     /*--------------------------------------------------------
                     * If Group 13 PDCH4, set appropriate pointers for type approval report
                     *--------------------------------------------------------*/ 
                     case GSM_GROUP_13:
                        sacchInfo = NULL;
                        reportIndex = timeSlot;
                        msReport = & g_diagData.msReport[rfCh][reportIndex];
                        channelNumber = timeSlot;
                        msReport->sacchCount = 1;  // only done to trigger report below.
			                     break;
                 }

                     /*-----------------------------------------------------------
                     * Finally, if the MS Report has accumulated values, send them 
                     *-----------------------------------------------------------*/ 
                   
                     if ( msReport->sacchCount != 0 )
                     {
                        INTR_DISABLE(CPU_INT8);     
                        params[0] = (msReport->cumErrs >> 24) & 0xff;
                        params[1] = (msReport->cumErrs >> 16) & 0xff;
                        params[2] = (msReport->cumErrs >>  8) & 0xff;
                        params[3] = msReport->cumErrs & 0xff;
                        params[4] = (msReport->cumBits >> 24) & 0xff;
                        params[5] = (msReport->cumBits >> 16) & 0xff;
                        params[6] = (msReport->cumBits >>  8) & 0xff;
                        params[7] = msReport->cumBits & 0xff;
                        params[8] = msReport->sacchCount;
                        if (sacchInfo != NULL)
                        {
                          params[9] = (sacchInfo->ulsigstren.rxLevAve >>  8) & 0xff;
                          params[10] = sacchInfo->ulsigstren.rxLevAve & 0xff;
                        }

                        params[11] = msReport->minToa;
                        params[12] = msReport->maxToa;
                        params[13] = msReport->cumToa;

                        if (sacchInfo != NULL)
                        {
                          params[14] = sacchInfo->timingAdvance;
                          params[15] = sacchInfo->powerLevel;
                        }

                        else  /* used by l1proxy to determine if TCH or PDCH */
                        {
                          params[14] = 0xff;
                          params[15] = 0xff;
                        }

                        params[16] = (msReport->cumFrameErrs >>  8) & 0xff;
                        params[17] = msReport->cumFrameErrs & 0xff;
                        params[18] = (msReport->cumFrames >>  8) & 0xff;
                        params[19] = msReport->cumFrames & 0xff;

                        initMsReport(rfCh, reportIndex);
                        INTR_ENABLE(CPU_INT8);
                        sendDiagMsg(MS_REPORT, rfCh, channelNumber, 20, params);
                     }

                  }
               }
            }
         }
      }
      /*-----------------------------------------------------------------
       * Check for GPRS_MS_REPORT diagnostic (PDCH) from any TFI. If
       * reports are not activated on any TFI, skip this altogether. 
       *----------------------------------------------------------------*/ 
      if ( g_diagData.gprsMsReportTfiMask )
      {
         diagSent = 0;         
         for ( rfCh=0; rfCh<NUM_RFS_PER_DSP; rfCh++ )
         {
            for ( tfi=0; tfi<MAX_TFI; tfi++ )
            {
               /*--------------------------------------------------------------
               * Check whether UL TBF is active and whether a report has been  
               * requested for its TFI. Also limit the number of diags sent. 
               *--------------------------------------------------------------*/ 
               tbf = & g_tbfInfo[rfCh][tfi];
               if ( (tbf->ulTbf.state == CHAN_ACTIVE) && 
                    (g_diagData.gprsMsReportTfiMask & (1L<<tfi)) &&
                    (diagSent < MAX_DIAGS_PER_CALL) )
               {
                  sacchInfo = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.sacch;
                  gprsMsReport = & g_diagData.gprsMsReport[rfCh][tbf->ulTbf.tfi];
                  /*--------------------------------------------------------------
                  * If RLC blocks have been received this interval, send a report 
                  * and clear the counters for the next interval
                  *--------------------------------------------------------------*/ 
                  if ( gprsMsReport->rlcBlkCount != 0 )
                  { 
                     INTR_DISABLE(CPU_INT8);     
                     params[0] = (gprsMsReport->cumErrs >> 24) & 0xff;
                     params[1] = (gprsMsReport->cumErrs >> 16) & 0xff;
                     params[2] = (gprsMsReport->cumErrs >>  8) & 0xff;
                     params[3] = gprsMsReport->cumErrs & 0xff;

                     params[4] = (gprsMsReport->cumBits >> 24) & 0xff;
                     params[5] = (gprsMsReport->cumBits >> 16) & 0xff;
                     params[6] = (gprsMsReport->cumBits >>  8) & 0xff;
                     params[7] = gprsMsReport->cumBits & 0xff;

                     params[8] = tbf->ulTbf.tfi;

                     params[9] = (tbf->ulTbf.accum.MSPower >>  8) & 0xff;
                     params[10] = tbf->ulTbf.accum.MSPower & 0xff;
                 
                     params[11] = gprsMsReport->minToa;
                     params[12] = gprsMsReport->maxToa;
                     params[13] = gprsMsReport->cumToa;

                     params[14] = tbf->ulTbf.timingAdvance;
                     params[15] = 0;
                     params[16] = (gprsMsReport->cumFrameErrs >>  8) & 0xff;
                     params[17] = gprsMsReport->cumFrameErrs & 0xff;
                     params[18] = (gprsMsReport->cumFrames >>  8) & 0xff;
                     params[19] = gprsMsReport->cumFrames & 0xff;

                     params[20] = gprsMsReport->rlcBlkCount;
                     params[21] = tbf->ulTbf.channelCodec;
                     params[22] = tbf->dlTbf.channelCodec;
                     params[23] = tbf->ulTbf.sigqual.berAve >> 8;
                     params[24] = tbf->ulTbf.sigqual.berAve;

                     params[25] = tbf->ulTbf.sigqual.blerAve >> 8;
                     params[26] = tbf->ulTbf.sigqual.blerAve;

                     initGprsMsReport(rfCh, tbf->ulTbf.tfi);
                     INTR_ENABLE(CPU_INT8);      
                     sendDiagMsg(GPRS_MS_REPORT, rfCh, 0, 27, params);
                     diagSent += 1;                                    
                  }
               }
            }
         }
      }    
      /*-----------------------------------------------------------------
       * Check for Burst Capture ready. After sending, turn it off. 
       *----------------------------------------------------------------*/ 
      if ( g_diagData.burstCapture.state == CAPTURE_DONE )
      { 
         SendCapturedBurst(g_diagData.burstCapture.rfCh,
                           g_diagData.burstCapture.timeSlot,
                           g_diagData.burstCapture.data);
                           
         g_diagData.burstCapture.state = CAPTURE_OFF;
      }
      /*-----------------------------------------------------------------
       * Check for DC Offset diagnostic. Remember it's in 1/256 units. 
       *----------------------------------------------------------------*/ 
      if ( g_diagData.dcOffsetReport )
      { 
         count = 0;     
         INTR_DISABLE(CPU_INT8);
         for ( rfCh=0; rfCh<NUM_RFS_PER_DSP; rfCh++ )
         {
            for ( index=0; index<NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS+1; index++ )
            {
               shortValue = g_DcOffset[rfCh][index].I >> 8; 
               params[count++] = (shortValue >>  8) & 0xff;
               params[count++] = shortValue & 0xff;
               shortValue = g_DcOffset[rfCh][index].Q >> 8; 
               params[count++] = (shortValue >>  8) & 0xff;
               params[count++] = shortValue & 0xff;
            }

	    for ( index=0; index<NUM_SDCCH8_SUBCHANS; index++ )
            {
               shortValue = g_Dc_7_Offset[rfCh][index].I >> 8; 
               params[count++] = (shortValue >>  8) & 0xff;
               params[count++] = shortValue & 0xff;
               shortValue = g_Dc_7_Offset[rfCh][index].Q >> 8; 
               params[count++] = (shortValue >>  8) & 0xff;
               params[count++] = shortValue & 0xff;
            }

	    
         }         
         INTR_ENABLE(CPU_INT8);
                  
         sendDiagMsg(DC_OFFSET_REPORT, 0, 0, count, params);
      }
      /*-----------------------------------------------------------------
       * Check for DL underflow, overflow and out-of-seq. First load FN.
       *----------------------------------------------------------------*/
      params[0] = sysFrameNum.t1 >> 8;
      params[1] = sysFrameNum.t1 & 0xff;
      params[2] = sysFrameNum.t2;
      params[3] = sysFrameNum.t3;
      
      for ( rfCh=0; rfCh<NUM_RFS_PER_DSP; rfCh++ )
      {
         for ( timeSlot=0; timeSlot<8; timeSlot++ )
         {
            /*---------------------------------------------
             * Downlink speech frame underflow
             *--------------------------------------------*/
            if ( g_DlUnderflowCount[rfCh][timeSlot] > 0 )
            {
               INTR_DISABLE(CPU_INT8);     
               params[4] = g_DlUnderflowCount[rfCh][timeSlot] >> 8;
               params[5] = g_DlUnderflowCount[rfCh][timeSlot] & 0xff; 
               g_DlUnderflowCount[rfCh][timeSlot] = 0;
               INTR_ENABLE(CPU_INT8);
               sendDiagMsg(SPEECH_UNDERFLOW, rfCh, timeSlot, 6, params);
            }
            /*---------------------------------------------
             * Downlink speech frame overflow
             *--------------------------------------------*/
            if ( g_DlOverflowCount[rfCh][timeSlot] > 0 )
            {
               INTR_DISABLE(CPU_INT8);     
               params[4] = g_DlOverflowCount[rfCh][timeSlot] >> 8;
               params[5] = g_DlOverflowCount[rfCh][timeSlot] & 0xff; 
               g_DlOverflowCount[rfCh][timeSlot] = 0;
               INTR_ENABLE(CPU_INT8);
               sendDiagMsg(SPEECH_OVERFLOW, rfCh, timeSlot, 6, params);      
            }
            /*---------------------------------------------
             * Downlink speech frame out of sequence
             *--------------------------------------------*/
            if ( g_DlOutOfSequenceCount[rfCh][timeSlot] > 0 )
            {
               INTR_DISABLE(CPU_INT8);     
               params[4] = g_DlOutOfSequenceCount[rfCh][timeSlot] >> 8;
               params[5] = g_DlOutOfSequenceCount[rfCh][timeSlot] & 0xff; 
               g_DlOutOfSequenceCount[rfCh][timeSlot] = 0;
               INTR_ENABLE(CPU_INT8);
               sendDiagMsg(SPEECH_OUT_OF_SEQ, rfCh, timeSlot, 6, params);      
            }
         }
      } 
   }
   /*-------------------------------------------------------------
    * Enable diagnostic report for next superframe 
    *------------------------------------------------------------*/ 
   if ( sysFrameNum.t2!=0 || sysFrameNum.t3!=0 )
   {  
      g_diagData.enable = TRUE;
   }
}

/*****************************************************************************
* Function: ReportErrorToHost
*
* Description:
*   Sends error reports to host once every superframe. Disables burst
*   interrupt, then re-enables when finished. 
*
******************************************************************************/
void ReportErrorToHost(void)
{
   Uint16 rfCh, timeSlot;
   Uint8  params[40];

   /*-------------------------------------------------------------
    * Send diagnostic message to host every superframe 
    *------------------------------------------------------------*/ 
   if ( sysFrameNum.t2==0 && sysFrameNum.t3==0 && g_ErrorEnable )
   {  
      /*-----------------------------------------------------------------
       * Disable further diagnostic report for this superframe
       *----------------------------------------------------------------*/
      g_ErrorEnable = FALSE;

      /*-----------------------------------------------------------------
       * Check for error conditions here... 
       *----------------------------------------------------------------*/ 
   }
   /*-------------------------------------------------------------
    * Enable diagnostic report for next superframe 
    *------------------------------------------------------------*/ 
   if ( sysFrameNum.t2!=0 || sysFrameNum.t3!=0 )
   {  
      g_ErrorEnable = TRUE;
   }
}











