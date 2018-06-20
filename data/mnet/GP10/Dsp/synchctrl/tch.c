/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: tch.c
*
* Description:
*   This file contains functions that process TCH frames and route SACCH
*   frames to the SACCH functions when appropriate.
*
* Public Functions:
*   ulTchProcBurst_0, ulTchProcBurst_3, ulIdleProc, dlIdleProc,
*   dlTchProcBurst_0, dlTchProcBurst_3, ulSachIdleProc, dlSachIdleProc
*
* Private Functions:
*   none
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h" 
#include "dsp/dsphmsg.h"
#include "bbdata.h"
#include "dsprotyp.h"
#include "diagdata.h" 
#include "agcdata.h"
#include "l1config.h"

#pragma DATA_ALIGN(frSilentFrameToast, 4);                                                        
Uint8 frSilentFrameToast[33] = {0xDA, 0xA7, 0xAA, 0xA5, 0x1A, 0x00, 0x20, 0x38,
                                0xE4, 0x6D, 0xB9, 0x1B, 0x00, 0x20, 0x38, 0xE4,
                                0x6D, 0xB9, 0x1B, 0x00, 0x20, 0x38, 0xE4, 0x6D,
                                0xB9, 0x1B, 0x00, 0x20, 0x38, 0xE4, 0x6D, 0xB9,
                                0x1B};

/*
*  EFR idle frame needs to be refilled !!!!!!!!!!!!!!!!!!!!!
*/
#pragma DATA_ALIGN(efrSilentFrameToast, 4);  
Uint8 efrSilentFrameToast[33] = {0xDA, 0xA7, 0xAA, 0xA5, 0x1A, 0x00, 0x20, 0x38,
                                0xE4, 0x6D, 0xB9, 0x1B, 0x00, 0x20, 0x38, 0xE4,
                                0x6D, 0xB9, 0x1B, 0x00, 0x20, 0x38, 0xE4, 0x6D,
                                0xB9, 0x1B, 0x00, 0x20, 0x38, 0xE4, 0x6D, 0xB9,
                                0x1B};
            
Uint32 tchReceived  = 0;
Uint32 facchSent[8] = {0, 0 , 0, 0, 0, 0, 0, 0};

/*****************************************************************************
*
* Function: ulTchProcBurst_0
*
* Description:
*   Processes uplink TCH bursts for frames 0,1,2 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulTchProcBurst_0(Uint8 rfCh, Uint8 timeSlot)
{                
   Uint16      i, k;
   t_TNInfo    *ts;
   Uint32      *rcvData;
   t_SACCHInfo *sacchInfo;
          
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   rcvData = (Uint32*)g_BBInfo[rfCh].ulBBSmpl;
   sacchInfo = & ts->u.group1.sacch;

   switch(ts->groupNum)
   {
   /*-------------------------------------------------------
    *  Channel combination I (TCH/F + SACCH)
    *------------------------------------------------------*/
   case GSM_GROUP_1:
      if(ts->u.group1.tch.state == CHAN_ACTIVE)
      {                        
         /*-------------------------------------------------------
          * If enabled, save raw samples for I/Q Loopback
          *------------------------------------------------------*/
         if ( g_loopBackMode == DSP_IQ_LOOPBACK )
         {                     
            if ( rcvRawIQBufIndex[timeSlot] < 156 )
               k = 0;
            else if ( rcvRawIQBufIndex[timeSlot] < 2*156 )
               k = 156;
            else if ( rcvRawIQBufIndex[timeSlot] < 3*156 )
               k = 2*156;
            else
               k = 3*156;

            rcvRawIQBufIndex[timeSlot] = k + 156;

            for(i=0; i<156; i++)
            {
               rcvdRawIQFrames[timeSlot][k++] =  rcvData[i];
            }   
         }
         /*-------------------------------------------------------
          * If enabled, capture samples in one burst 
          *------------------------------------------------------*/
         if ( g_diagData.burstCapture.state == CAPTURE_ARMED )
         {
            for(i=0; i<BURST_CAPTURE_SIZE; i++)
            {
               g_diagData.burstCapture.data[i] =  rcvData[i];
            }
            g_diagData.burstCapture.state = CAPTURE_DONE;           
         }
         /*-------------------------------------------------------
          * If HO, try to detect AB, else demodulate NB
          *------------------------------------------------------*/
         if(ts->u.group1.tch.HOFlag)
         {         
            ulRachProc(rfCh, timeSlot);
         }
         else                 
         {         
            DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, timeSlot );
            demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                        TCHF, 
                        g_BBInfo[rfCh].tseqNum, 
                        &g_ULBurstData,
                        ts->groupNum,
                        rfCh,
                        timeSlot,
                        0 );              
            decrypt(rfCh, timeSlot);      
            chanDecoder(TCHF, 0, timeSlot);
            accumPower(rfCh, timeSlot, ts->groupNum, TCHF, sacchInfo, NULL);
          }
      }
     else
      {
       /*
       *   perform Fire decode, if there is no uplink burst process is inactive
       */
         ulCCHFireDecoder();
     }
      break;

   /*-------------------------------------------------------
    * Default case should never occur
    *------------------------------------------------------*/
   default:
      break;
   }
}

/*****************************************************************************
*
* Function: ulTchProcBurst_3
*
* Description:
*   Processes uplink TCH bursts for frame 3 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulTchProcBurst_3(Uint8 rfCh, Uint8 timeSlot)
{      
   Uint16         i, k;
   t_TNInfo       *ts;          
   Uint32         *rcvData;
   msgUnionStruc  tchMsg;
   t_SACCHInfo    *sacchInfo;
   Bool           dtxSid;
   Uint8          sidFlag;
   Int8           writeIndex;
   t_ULSignalStrength *sigS;
	
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   rcvData = (Uint32*)g_BBInfo[rfCh].ulBBSmpl;
   sacchInfo = & ts->u.group1.sacch; 

   switch(ts->groupNum)
   {
   /*------------------------------------------------------
   * Channel combination I (TCH/F + SACCH)
   *------------------------------------------------------*/
   case GSM_GROUP_1:
      if ( ts->u.group1.tch.state == CHAN_ACTIVE &&
           g_loopBackMode != DSP_TCH_TO_HOST_LOOPBACK )
      {
         /*
         * Note: if TCH Loopback to Host, downlink TCH frame is sent
         * back to host on tchMsgProc(). Don't process UL TCH here.
         */   
           
         /*------------------------------------------------------
         * If enabled, save raw samples for I/Q loopback
         *------------------------------------------------------*/
         if(g_loopBackMode == DSP_IQ_LOOPBACK)
         {                     
            k = 3*156;
            rcvRawIQBufIndex[timeSlot] = 0;

            for(i=0; i<156; i++)
            {
               rcvdRawIQFrames[timeSlot][k++] = rcvData[i];
            }
         }
         /*------------------------------------------------------
         * If HO, try to detect AB, else demodulate NB
         *------------------------------------------------------*/
         if(ts->u.group1.tch.HOFlag)
         {
            ulRachProc(rfCh, timeSlot);        
         }
         else
         {         
            DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, timeSlot );
            demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                        TCHF, 
                        g_BBInfo[rfCh].tseqNum, 
                        &g_ULBurstData,
                        ts->groupNum,
                        rfCh,
                        timeSlot,
                        0 );              
            decrypt(rfCh, timeSlot);      
            chanDecoder(TCHF, 1, timeSlot);  
            accumPower(rfCh, timeSlot, ts->groupNum, TCHF, sacchInfo, NULL);  

            /*-----------------------------------------------------------
            * Accumlate bit errors and frame errors into SACCH structure
            * Note1: This must come before FACCH processing
            * Note2: We also use this Boolean, dtxSid, later in function
            *-----------------------------------------------------------*/

            dtxSid = (ts->u.group1.ulIndex == ((12-timeSlot) & 0x7) && ts->ulTblIndex == 7);
            accumBer(rfCh, timeSlot, ts->groupNum, TCHF, 0, sacchInfo,
                     ts->u.group1.tch.ulDtxUsed, dtxSid); 
                              
            /*----------------------------------------------------------
            * Update AGC on final burst of every TCH frame, no update if UL DTX is used
            *----------------------------------------------------------*/                              
            if(g_AGC_ON && (!ts->u.group1.tch.ulDtxUsed || dtxSid))
            {
               sigS = & sacchInfo->ulsigstren;
               sigS->rxPwrNormSmooth = ul_agc(sigS);
            }
            /*----------------------------------------------------------
            * If stolen frame, process FACCH message
            *----------------------------------------------------------*/
            ulSyncMsgProcess(FACCH, 0, timeSlot, rfCh);
                                               
            /*----------------------------------------------------------
            * Process speech frames only if channel is in speech mode
            *----------------------------------------------------------*/
            if( ts->u.group1.tch.speechDataInd == IND_SPEECH &&
                g_ULTCHData.ultchword0.bitval.bufferValid )
            {                   

               g_ULTCHData.ultchword0.bitval.bufferValid = FALSE;

               tchMsg.msgSize = 46;      
               tchMsg.function = TRAFFIC_MSG;
               tchMsg.typeHiByte =  UP_LINK_TCH >> 8;  
               tchMsg.typeLoByte =  UP_LINK_TCH;  
               tchMsg.trx = rfCh;
               tchMsg.u.params[0] = timeSlot | 0x08;
               tchMsg.u.params[1] = 0;

               /*----------------------------------------------------------
               * If frame contains good speech (passed parity and did not
               * trigger perceptual masking), then copy to tchFrame, and
               * send to host. If not good speech, send nothing to host.
               *----------------------------------------------------------*/
               if( ! g_ULTCHData.ultchword0.bitval.bfi &&
                   ! g_ULTCHData.ultchword0.bitval.mask )
               {   
                  tchReceived++;

                             /*
                  *  Check if an SID Frame is detected. This detection should happen
                  *  before it is detected on SACCH frame. Note: Need to use sidFlag
                  *  even if we're not in UL DTX mode to properly handle disabling it.
                  */
                  sidFlag = sidDetection( (Uint8*)&g_ULTCHData.data[4], ts->u.group1.tch.vocAlgo);
                  if( ts->u.group1.tch.ULDtxMode ) 
                  {
                     if( sidFlag == 2 ) 

                     {
                        if(!ts->u.group1.tch.ulDtxUsed)
                        {
                           /*
                           *  A SID frame is detected
                           */
                           ts->u.group1.tch.ulDtxUsed = TRUE;
                           sendDiagMsg(5,rfCh, timeSlot,1, &sidFlag);
                        }
                        /*
                        *   Erase SID frame Temporary for Incompleted CISCO Gateway to handle
                        *   SID frame
                        */
                        if( ts->u.group1.tch.vocAlgo == VOC_GSMF )
                           return;
                     }
                     else 
                     {
                        if(ts->u.group1.tch.ulDtxUsed)
                        {
                           ts->u.group1.tch.ulDtxUsed = FALSE;
                           sendDiagMsg(10,rfCh, timeSlot,1, &sidFlag);
                        }
                     }
                  }
                  /*
                  *  If the operator disabled UL DTX while the call was active, wait for
                  *  the TCH frame that would normally carry SID. If the MS has not yet
                  *  realized UL DTX has been disabled, then this frame may contain SID,
                  *  indicating the talker is silent and the MS is not sending up all UL
                  *  frames. In this case, don't set ulDtxUsed=FALSE yet because our RXLEV 
                  *  and BER accumlation would go bad. Look for this TCH frame to contain
                  *  non-SID before switching ulDtxUsed=FALSE. We still run the risk that
                  *  the talker goes silent just after we do this while the MS has not yet
                  *  realized UL DTX is off, but we hope it's unlikely and brief. All of
                  *  this is to avoid false handover when UL DTX switches from On to Off.   
                  */  
                  else if ( dtxSid && (sidFlag==0) )
                  {
                     ts->u.group1.tch.ulDtxUsed = FALSE;
                  }

#ifdef ETSI_FRAME_PACKING                  
                  packToByteBuffer( &g_ULTCHData.data[0], 
                                    tchMsg.u.tchInfo.tchFrame,
                                    NUM_BITS_TCH_FRAME );
#else              
                  /*--------------------------------------------------------
                  * Copy input speech to tchFrame (whether loopback or not)
                  *--------------------------------------------------------*/
                  packToByteMSB( (UChar*)&g_ULTCHData.data[0], 
                                 tchMsg.u.tchInfo.tchFrame,
                                 NUM_BITS_TCH_FRAME+4 );    
#endif     
                  /*--------------------------------------------------------
                  * Send the speech frame to the host
                  *--------------------------------------------------------*/
                  SendMsgHost_Que((msgStruc*)(&tchMsg));

                  /*                  
                  *  debug code to send back a SID frame
                  if(ts->u.group1.tch.ulDtxUsed &&
                     ts->u.group1.ulIndex == ((12-timeSlot) & 0x7) &&
                     ts->ulTblIndex == 7)
                  {                 
                     tchMsg.u.params[2] =  ts->u.group1.ulIndex;
                     tchMsg.u.params[3] =  ts->ulTblIndex;
                     sendDiagMsg(0x11, rfCh, timeSlot, 40, & tchMsg.u.params[2]);
                  }
                  */      
               }
               /*--------------------------------------------------------
               * If TCH loopback, pack UL TCH frame to DL TCH buffer
               *--------------------------------------------------------*/
               if ( g_loopBackMode == DSP_TCH_LOOPBACK &&
                    !g_ULTCHData.ultchword0.bitval.bfi )
               {      
                  rcvData = (Uint32 *) tchMsg.u.tchInfo.tchFrame;     
                  /*
                  *  copy uplink GSM TCH frame to downlink TCH buffer
                  */
                  writeIndex = g_dlTchFrame[timeSlot].writeIndex;                    
                  for(i=0; i<10; i++)
                  {
                     g_dlTchFrame[timeSlot].buffer[writeIndex][i] = rcvData[i];
                  }     
      
                  /*
                  *  update buffer index (circular)
                  */                              
                  if(++g_dlTchFrame[timeSlot].writeIndex >= NUM_TCH_JBUF)
                     g_dlTchFrame[timeSlot].writeIndex = 0;
  
                  g_dlTchFrame[timeSlot].frameCount += 1;

                  /*----------------------------------------------------
                  * Check overflow
                  *---------------------------------------------------*/
                  if(g_dlTchFrame[timeSlot].frameCount > NUM_TCH_JBUF)
                  {
                     g_dlTchFrame[timeSlot].frameCount = NUM_TCH_JBUF;
                     ReportError(0x91, rfCh, timeSlot, 0, &rfCh);
                  }
               }                             
            }
         } 
      }
      else
      {
         /*-----------------------------------------------------
         * Slot is inactive, run Fire decoder
         *-----------------------------------------------------*/
         ulCCHFireDecoder();
      }
                                
      break;
        
   /*-------------------------------------------------------
    * Default case should never occur
    *------------------------------------------------------*/
   default:
      break;
   }
}
 

/*****************************************************************************
*
* Function: ulIdleProc
*
* Description:
*   Processes uplink idle frames
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulIdleProc(Uint8 rfCh, Uint8 timeSlot)
{   
   t_TNInfo *ts;                              
   Int16    i;
   Uint32   *rdPtr;
   t_SACCHInfo *sacchInfo;
   Int16       currentRxLev; 

   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   
  
   sacchInfo = & ts->u.group1.sacch;
 

   rdPtr = (Uint32*)g_BBInfo[rfCh].ulBBSmpl;
                                         
   for(i=0; i<156; i++)
   {
      g_idleFrameInfo[rfCh][timeSlot].data[i]=  rdPtr[i];
   }

   /*
   *  Update the all-carrier DC offset
   */

   if (ts->groupNum != GSM_GROUP_7){
     DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS );
   }
   else{
     DCOffset( g_BBInfo[rfCh].ulBBSmpl, GSM_GROUP_5, rfCh, NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS );
   }
   /*
   *  Demodulate just to get received power
   */    
   demodulate( g_BBInfo[rfCh].ulBBSmpl, 
               TCHF, 
               g_BBInfo[rfCh].tseqNum, 
               &g_ULBurstData,
               ts->groupNum,
               rfCh,
               timeSlot,
               0 );              
                                                                
   g_idleFrameInfo[rfCh][timeSlot].rcvPower = g_ULBurstData.rcvPower;

   /* used for interference reporting */
   if ((sacchInfo->state != CHAN_ACTIVE) && (ts->groupNum != GSM_GROUP_7)){
    /*
    *  set up link gain for idle channel monitoring based on radio board versions.
    */
     sacchInfo->ulsigstren.rxgain = g_rxGainMinIdle;

    /*----------------------------------------------------------
     * Compute uplink RXLEV of burst using rcvPower (dBr) and
     * current rxgain (0-0x3ff) as input. All other terms are
     * measured calibration constants specific to ViperCell H/W.
     *----------------------------------------------------------*/
     currentRxLev = g_MeasAgcDpcDbm + DBM_2_RXLEV_CONV_FACTOR
      + (g_ULBurstData.rcvPower - g_MeasAgcDpcDbr) 
      + (((sacchInfo->ulsigstren.rxgain - g_MeasAgcDpcRxgain)
         * g_gainStepsPerDb.inverseSlope) >> DIVISION_FACTOR); 

     if(currentRxLev < 0) {   /* Limit to a positive range */
       currentRxLev = 0;
     }  

     sacchInfo->MSPowerAccumFull += currentRxLev;
   }
}

/*****************************************************************************
*
* Function: zeroTxBurst
*
* Description:
*   Zeroes out buffer (for turning off Tx power)
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void zeroTxBurst(Uint32 *burstBuf)
{               
   Int16 i;
   /*
   *  turn off output power
   *  i.e., send zero energy on I/Q channel
   */
   for(i=0; i<312; i++)                
   {
       burstBuf[i] = 0;
   }
}        

/*****************************************************************************
*
* Function: dlIdleProc
*
* Description:
*   Processes downlink idle frames
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/           
void dlIdleProc(Uint8 rfCh, Uint8 timeSlot)
{   
   Int16 i;
      
   t_TNInfo *ts;           
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];    

   /*-------------------------------------------------------------
    * Idle Frame Mode flag determines whether to shut off
    * Tx during this burst or to send a dummy burst
    *------------------------------------------------------------*/     
   if( ! g_IdleFrameMode )
   {     
       zeroTxBurst((Uint32*)g_BBInfo[rfCh].dlBBSmpl);
   }
   else
   {   
      g_DLCodedData.tx = TRUE;
      buildGSMBurst(IDLE,  g_BBInfo[rfCh].tseqNum);
      GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, IDLE);
   }
}

/*****************************************************************************
*
* Function: dlTchProcBurst_0
*
* Description:
*   Processes downlink TCH bursts for frame 0 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlTchProcBurst_0(Uint8 rfCh, Uint8 timeSlot)
{
   t_TNInfo    *ts;          
   t_DLSigBufs *facchMsg;
   t_TCHInfo   *tch;
   Int16       i;
   Uint8       txOn;
   Uint8       sid;
   t_chanSel   payloadType;
   Uint8       doRtsCheck = FALSE;
    
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   dlTchNetProc();

   switch(ts->groupNum)
   {
   case GSM_GROUP_1:    
      tch = & ts->u.group1.tch;
      facchMsg = ts->u.group1.facch.dlSigBuf;      
                                 
      /*             
      *  set TX on as default
      */                           
      txOn = 1;                                 
       
      if(tch->state == CHAN_ACTIVE)
      {       
         /*------------------------------------------------------
         * If enabled, send raw samples for I/Q loopback
         *------------------------------------------------------*/
         if(g_loopBackMode == DSP_IQ_LOOPBACK)
         {                     
            compLpbkBuf(& rcvdRawIQFrames[timeSlot][0], (Uint32*)g_BBInfo[rfCh].dlBBSmpl);
            txRawIQBufIndex[timeSlot] = 156;
         }
         /*----------------------------------------------------------
         * Check for FACCH, speech or data, then build and modulate
         *----------------------------------------------------------*/
         else  
         {
            /*----------------------------------------------------------
            * If FACCH message is ready, let it override speech or data
            *----------------------------------------------------------*/
            if(facchMsg->numMsgsReady > 0)
            {    
               facchSent[timeSlot]++;              
               /*
               *  Process FACCH first. Set flags to properly encode,
               *  encrypt and check RTS below.
               */
               processCCHBuff(facchMsg);
               payloadType = FACCH;
               doRtsCheck = TRUE;
               /*
               *  skip a speech frame if a it is stolen by FACCH
               */
               if(g_dlTchFrame[timeSlot].frameCount > 0)
               {
                  g_dlTchFrame[timeSlot].frameCount--;
            
                  if(++g_dlTchFrame[timeSlot].readIndex >= NUM_TCH_JBUF)            
                     g_dlTchFrame[timeSlot].readIndex = 0;
               }
               /*
               *  Check if a SID frame is stolen by a FACCH message.
               */               
               if(g_dlSidFrame[timeSlot].frameCount > 0 &&
                  ts->u.group1.dlIndex == ((12-timeSlot) & 0x7)  && 
                  ts->dlTblIndex == 0) 
               {
                  ts->u.group1.tch.stolenSid++;
               }
            }
            /*---------------------------------------------------------------
            * Process as speech if channel is in speech mode and no FACCH msg
            *---------------------------------------------------------------*/
            else if ( ts->u.group1.tch.speechDataInd == IND_SPEECH )
            {                    

   #ifdef ETSI_FRAME_PACKING                  
         
               unpackFromByteBuffer(g_dlTchFrame[timeSlot].buffer,
                                    (UChar *)&g_DLTCHData.data[0], 
                                    NUM_BITS_TCH_FRAME);
               payloadType = TCHF;                             
   #else
               /*---------------------------------------------------------------
               * If speech is not available, send silence, SID or turn off Tx, but
               * do not send dummy tch data if LOCAL LOOPBACK.
                *---------------------------------------------------------------*/

               if((g_dlTchFrame[timeSlot].frameCount <=0) && (g_loopBackMode != LOCAL_LOOPBACK))
               
               {  
                  /*
                  *  check if a SID is required to send
                  */                                      
                  if( g_dlSidFrame[timeSlot].frameCount > 0  &&
                      ( (ts->u.group1.dlIndex == ((12-timeSlot) & 0x7) && ts->dlTblIndex == 0) || 
                         ts->u.group1.tch.stolenSid > 0) )
                  { 
                     if ( ts->u.group1.tch.stolenSid > 0 )
                     {                                 
                        /*
                        *  send a SID frame on scheduled frame number or been stolen
                        *  by a FACCH message
                        */                      
                        ts->u.group1.tch.stolenSid--;
                     }
                     else
                     {
                        /*
                        *  send a SID frame on schedule
                        */
                        g_dlSidFrame[timeSlot].frameCount--; 
                     }
                     
                     unpackByteMSB((UChar*)g_dlSidFrame[timeSlot].buffer,
                                   (UChar *)&g_DLTCHData.data[0], 
                                   NUM_BITS_TCH_FRAME+4);                                   
                     payloadType = TCHF;   
                
                     sendDiagMsg(0x8, rfCh, timeSlot, 1, &sid);
                  }
                  else
                  {     /*
                     *  If DL DTX mode and it's not time to send a mandatory frame, then turn off
                     *  transmission (zero I/Q output). This should not happen on BCCH carrier!
                     */
                     if ( ts->u.group1.tch.DLDtxMode &&
                          !(ts->u.group1.dlIndex == ((12-timeSlot) & 0x7) && ts->dlTblIndex == 0) )
                     {
                        txOn = 0;                      
                     }                  
                            
                     /*
                     *  Send DCCH filler when not in DTX mode, letting MS mask the gap
                     */                  
                     unpackFromByteBuffer(dcchFiller,
                                          (UChar*)g_DLCCHData.data, 
                                          NUM_BITS_CCH_FRAME);             
                     payloadType = FACCH;      

                  }
               }
               /*---------------------------------------------------------------
               * Else speech is available, send it. If DTX, copy SID frame.
               *---------------------------------------------------------------*/
               else
               {              
                  /*
                  *  unpack a speech frame
                  */
                  unpackByteMSB((UChar*)g_dlTchFrame[timeSlot].buffer[g_dlTchFrame[timeSlot].readIndex],
                                (UChar *)&g_DLTCHData.data[0], 
                                NUM_BITS_TCH_FRAME+4);

                  payloadType = TCHF;      
                              
                  if( ts->u.group1.tch.DLDtxMode &&
                      (sid = sidDetection((UChar*) & g_DLTCHData.data[4], ts->u.group1.tch.vocAlgo)) == 2)
                  {                          
                     if ( !ts->u.group1.tch.dlDtxUsed ||
                          (ts->u.group1.dlIndex == ((12-timeSlot) & 0x7) && ts->dlTblIndex == 0) )
                     {  
                        /*
                        *  it is the first SID frame or on it is on SID TX schedule,
                        * send it immediately
                        */
                        sendDiagMsg(0x7, rfCh, timeSlot, 1, &sid);
                     }
                     else
                     {   
                        /*
                        *  it is not the right frame to send SID frame
                        */                                        
                        txOn = 0;
                     }
                     /* a valid SID Code found, DL DTX mode is applied */
                     if ( !ts->u.group1.tch.dlDtxUsed )
                     {
                        sendDiagMsg(0x6, rfCh, timeSlot, 1, &sid);
                        ts->u.group1.tch.dlDtxUsed = TRUE;
                     }                                          
                     /*
                     *  copy SID frame to SID buffer
                     */
                     for(i=0; i<10; i++)
                     {
                         g_dlSidFrame[timeSlot].buffer[i] =
                           g_dlTchFrame[timeSlot].buffer[g_dlTchFrame[timeSlot].readIndex][i];
                     } 
                     g_dlSidFrame[timeSlot].frameCount = 1;

                  }                                
                  else
                  {
                     /*
                     *  NON DTX mode or a non-SID Frame found
                     */
                     if ( ts->u.group1.tch.dlDtxUsed )
                     {
                             //   sendDiagMsg(11, rfCh, timeSlot, 1, &sid);
                        ts->u.group1.tch.dlDtxUsed = FALSE;
                     }                                
                     g_dlSidFrame[timeSlot].frameCount = 0;
                  }
                  /*
                  *  update read index and frame count
                  */
                  if ( ++g_dlTchFrame[timeSlot].readIndex >= NUM_TCH_JBUF )
                  {            
                     g_dlTchFrame[timeSlot].readIndex = 0;
                  }
                  g_dlTchFrame[timeSlot].frameCount--;
               }
   #endif                        
               /*---------------------------------------------------------------
               * If speech frame was available, update read index and frame count.
               * If it is not available, increment frame error count and let
               * ReportDiagToHost send the diag msg at the next SF boundary.
               *--------------------------------------------------------------*/        
               if( g_dlTchFrame[timeSlot].frameCount < 0 )
               {   
                  if( g_dlTchFrame[timeSlot].frameStarted &&
                      !ts->u.group1.tch.DLDtxMode &&
                      !ts->u.group1.tch.ULDtxMode )
                  {  
                     g_DlUnderflowCount[rfCh][timeSlot] += 1;
                  }
                  g_dlTchFrame[timeSlot].frameCount = 0;
               }
            }                                                     
            else
            {    
               /*---------------------------------------------------------------
               * Non speech mode, send a filler, if no FACCH message
               *---------------------------------------------------------------*/
               unpackFromByteBuffer(dcchFiller,
                                    (UChar*)g_DLCCHData.data, 
                                    NUM_BITS_CCH_FRAME);                              
            }
            /*---------------------------------------------------------------
            * For TCH, channel encode and encrypt
            *---------------------------------------------------------------*/
            if ( payloadType == TCHF )
            {                                                            
               g_DLTCHData.dltchword0.bitval.bufferValid = 1;
               g_DLTCHData.dltchword0.bitval.sp = 1;
               chanEncoder(TCHF, 1, timeSlot, txOn);     
               encrypt(rfCh, timeSlot, 0);
            }
            /*---------------------------------------------------------------
            * For FACCH, channel encode and encrypt. Check RTS on non-filler. 
            *---------------------------------------------------------------*/
            else /* payloadType is FACCH */
            {
               g_DLCCHData.bufferValid = TRUE;                                    
               chanEncoder(FACCH, 1, timeSlot, txOn);
               encrypt(rfCh, timeSlot, 0);      
               if ( doRtsCheck )
               {
                  rtsCheck(rfCh, timeSlot, FACCH, 0, facchMsg->numMsgsReady);
               }    
            }                 
            buildGSMBurst(TCHF,  g_BBInfo[rfCh].tseqNum);
            GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, TCHF);             
         }
      }    
      /*----------------------------------------------------------
      * If TCH inactive, dlIdleProc() sends either dummy burst or 
      * nothing depending on whether this RF is the BCCH carrier
      *----------------------------------------------------------*/
      else
      { 
         dlIdleProc(rfCh, timeSlot);
      }
      break;
      
   default:
      /*----------------------------------------------------------
      * If not Group I, dlIdleProc() decides what to do
      *----------------------------------------------------------*/
      dlIdleProc(rfCh, timeSlot);
      break;
   }       
}


/*****************************************************************************
*
* Function: dlTchProcBurst_3
*
* Description:
*   Processes downlink TCH bursts for frames 1,2,3 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlTchProcBurst_3(Uint8 rfCh, Uint8 timeSlot)
{      
   t_TNInfo *ts; 
   t_TCHInfo *tch;
   t_DLSigBufs *facchMsg;
   Uint16 k;
   
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   
   switch(ts->groupNum)
   {
   case GSM_GROUP_1: 
      tch = & ts->u.group1.tch;  
      facchMsg = ts->u.group1.facch.dlSigBuf;            
   
      if(tch->state == CHAN_ACTIVE)
      {      
         if(g_loopBackMode == DSP_IQ_LOOPBACK)
         {                     
            if ( txRawIQBufIndex[timeSlot] < 156 )
               k = 0;
            else if ( txRawIQBufIndex[timeSlot] < 2*156 )
               k = 156;
            else if ( txRawIQBufIndex[timeSlot] < 3*156 )
               k = 2*156;
            else
               k = 3*156;

            txRawIQBufIndex[timeSlot] = k + 156;        

            compLpbkBuf(& rcvdRawIQFrames[timeSlot][k], (Uint32*)g_BBInfo[rfCh].dlBBSmpl);
         }
         else  
         {                  
            chanEncoder(TCHF, 0, timeSlot, 0);
            encrypt(rfCh, timeSlot, 3);      
            buildGSMBurst(TCHF,  g_BBInfo[rfCh].tseqNum);
            GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, TCHF);       
         }
      }    
      /*----------------------------------------------------------
      * If TCH inactive, dlIdleProc() sends either dummy burst or 
      * nothing depending on whether this RF is the BCCH carrier
      *----------------------------------------------------------*/
      else
      { 
         dlIdleProc(rfCh, timeSlot);
      }
      break;

   default:
      /*----------------------------------------------------------
      * If not Group I, dlIdleProc() decides what to do
      *----------------------------------------------------------*/
      dlIdleProc(rfCh, timeSlot);
      break;
   }        
}

/*****************************************************************************
*
* Function: ulSachIdleProc
*
* Description:
*   Translates to uplink SACCH or Idle function depending on postion in MF
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulSachIdleProc(Uint8 rfCh, Uint8 timeSlot)
{                          
   Void (** funcTbl ) ();
   t_TNInfo *ts;          
                   
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   
   funcTbl = (void *) & ulSacchIdleTbl[ts->u.group1.ulIndex++];
   (* funcTbl)(rfCh, timeSlot);
    
   ts->u.group1.ulIndex &= 0x7;
}

/*****************************************************************************
*
* Function: dlSachIdleProc
*
* Description:
*   Translates to downlink SACCH or Idle function depending on postion in MF
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlSachIdleProc(Uint8 rfCh, Uint8 timeSlot)
{                     
   
   void (** funcTbl ) ();
   t_TNInfo *ts;
   
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   funcTbl = (void *) & dlSacchIdleTbl[ts->u.group1.dlIndex++];
   (* funcTbl)(rfCh, timeSlot);

   ts->u.group1.dlIndex &= 0x7;
}

