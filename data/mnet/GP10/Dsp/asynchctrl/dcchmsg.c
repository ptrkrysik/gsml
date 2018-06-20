/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dcchmsg.c
*
* Description:
*   This file contains functions that handle DCCH messages from Host to DSP.
*
* Public Functions:
*   dcchMsgProc, sacchMsgProc
*
* Private Functions:
*   initPower, channelModeIE, encryptionIE
*
******************************************************************************/
#include "stdlib.h" 
#include "string.h" 
#include "gsmdata.h"
#include "oamdata.h"
#include "dsp/dsphmsg.h"
#include "bbdata.h" 
#include "l1config.h"
#include "diagdata.h" 
#include "agcdata.h"
#include "dsprotyp.h"
  
/*****************************************************************************
*
* Function: initPower
*
* Description:
*   Initializes TA, MS power, BTS power and dynamic power control
*
* Input:  
*   hostChActInfo         ptr to channel activation structure from Host
*   ->timeAdv               command parameters from Host...           
*   ->msPowerInit                 
*   ->msPower  
*   dspSacchInfo          ptr to SACCH Info structure in DSP
*
* Output:
*   dspSacchInfo          ptr to SACCH Info structure in DSP
*   ->timingAdvance         operating parameters in DSP...        
*   ->powerLevelInit        
*   ->powerLevel           
*   ->ulsigstren
*     .rxPwr[]
*     .powerSum
*     .rxPwrAve
*     .powerSumAve
*   ->toaAccumSub
*   ->MSPowerAccumSub        
*
******************************************************************************/
ProcResult initPower(chActMsgStruc *hostChActInfo, t_SACCHInfo *dspSacchInfo)
{
   Uint8    i;
   Int16    currentRxLev;
   Uint8    tempBuf[8];              

   if(g_loopBackMode == LOCAL_LOOPBACK)
   { 
     hostChActInfo->msPower = -47;  /* dBr */   
     g_USGC = 0x290;
  }
            
   /*-----------------------------------------------------------
   * Set initial TOA and MS and BTS power
   *----------------------------------------------------------*/
   dspSacchInfo->timingAdvance  = hostChActInfo->timeAdv;
   dspSacchInfo->powerLevelInit = hostChActInfo->msPowerInit; 
   dspSacchInfo->dynPwrRprtCntr = 0;
    
   /*-----------------------------------------------------------
   * Initialize UL dynamic power control
   *----------------------------------------------------------*/
   dspSacchInfo->powerLevel                   = hostChActInfo->msPowerInit;
   dspSacchInfo->dlsigstrenSCell.MSPwrSetting = hostChActInfo->msPowerInit;
   
   currentRxLev = g_MeasAgcDpcDbm + DBM_2_RXLEV_CONV_FACTOR
      + (hostChActInfo->msPower - g_MeasAgcDpcDbr) 
      + (((g_USGC - g_MeasAgcDpcRxgain)
         * g_gainStepsPerDb.inverseSlope) >> DIVISION_FACTOR); 
   
   /*-----------------------------------------------------------
   * For co-channel interference processing
   *----------------------------------------------------------*/
   g_IntRprtCntr = 0;


   /*-----------------------------------------------------------
   * Initialize UL and DL RXLEV and RXQUAL histories. Since we
   * have no information on DL RXLEV, initialize it to the same
   * value as UL RXLEV. Initialize all RXQUALs to high quality.
   *----------------------------------------------------------*/
   for (i=0; i<NUM_RX_SIGNAL_SAMPLES_MAX; i++)
   {
      dspSacchInfo->ulsigstren.rxLev[i] = currentRxLev;
      dspSacchInfo->ulsigqual.rxQual[i] = 0;
      dspSacchInfo->dlsigstrenSCell.rxLev[i] = currentRxLev;
      dspSacchInfo->dlsigqualSCell.rxQual[i] = 0;
   }

   dspSacchInfo->ulsigstren.count = 0;
   dspSacchInfo->ulsigstren.rxLevAve = currentRxLev; 
   dspSacchInfo->ulsigstren.rxPwrNormSmooth = 0; /* assume we're at target */

   dspSacchInfo->ulsigqual.count = 0;
   dspSacchInfo->ulsigqual.ber = 0;
   dspSacchInfo->ulsigqual.berAve = 0;
   dspSacchInfo->ulsigqual.rxQualAve = 0;

   dspSacchInfo->dlsigstrenSCell.count = 0;
   dspSacchInfo->dlsigstrenSCell.rxLevAve = currentRxLev; 

   dspSacchInfo->dlsigqualSCell.count = 0;
   dspSacchInfo->dlsigqualSCell.berAve = 0;
   dspSacchInfo->dlsigqualSCell.rxQualAve = 0;

   dspSacchInfo->toaAccumSub = 0; 			
   dspSacchInfo->MSPowerAccumSub = 0;
   dspSacchInfo->MSPowerAccumFull = 0;
   dspSacchInfo->MSPowerAccum = 0;

   dspSacchInfo->bitErrorAccum = 0;
   dspSacchInfo->bitCountAccum = 0;   
   dspSacchInfo->frameErrorAccum = 0;
   dspSacchInfo->frameCountAccum = 0;

   /*-----------------------------------------------------------
   * Initialize UL automatic gain control
   *----------------------------------------------------------*/
   if ( g_AGC_ON )
   {
      /*
      * If access burst (prior to this activation) saturated the ADC,
      * use a lower target -- one that still works if the signal is
      * exactly 0 dBr, but also gives headroom in case it is stronger
      */
      if ( hostChActInfo->msPower >= 0 )
      {
         dspSacchInfo->ulsigstren.rxgain = g_USGC +
            (hostChActInfo->msPower - SAT_RXPWR_TARGET) * g_gainStepsPerDb.slope;
      }
      else
      {       
         dspSacchInfo->ulsigstren.rxgain = g_USGC +
            (hostChActInfo->msPower - g_AgcRxPwrTarget) * g_gainStepsPerDb.slope;
      }  
                                                  
      /*-- limit gain to an unsigned 10-bit value --*/  
      if(dspSacchInfo->ulsigstren.rxgain > g_rxGainMinAct)
      {  
         dspSacchInfo->ulsigstren.rxgain = g_rxGainMinAct;            
      }
      else if(dspSacchInfo->ulsigstren.rxgain < RXGAIN_MAX)
      {  
         dspSacchInfo->ulsigstren.rxgain = RXGAIN_MAX;
      } 
      dspSacchInfo->agcHoldFlag = TRUE;
   }
   else
   {
      dspSacchInfo->agcHoldFlag = FALSE;  /* AGC is off */
   }  
   /*-----------------------------------------------------------
   * Initialize UL dynamic power control
   *----------------------------------------------------------*/
   if ( g_DPC_ON )
   {
      dspSacchInfo->dpcHoldFlag = TRUE;
   }
   else
   {
      dspSacchInfo->dpcHoldFlag = FALSE;  /* DPC is off */
   }  
   /*-----------------------------------------------------------
   * Send diagnostic msg with computed initial gain setting
   *----------------------------------------------------------*/
   tempBuf[0] = dspSacchInfo->powerLevel;
   tempBuf[1] = dspSacchInfo->dlsigstrenSCell.MSPwrSetting;
   tempBuf[2] = dspSacchInfo->ulsigstren.rxgain>>8;	           
   tempBuf[3] = dspSacchInfo->ulsigstren.rxgain;		           
   tempBuf[4] = dspSacchInfo->agcHoldFlag;    
   //   sendDiagMsg(0x97,0,0,5,tempBuf);

   return SUCCESS;
}

/*****************************************************************************
*
* Function: channelModeIE
*
* Description:
*   Translates Host Channel Mode IE into DSP enumerated values
*
* Input:
*   msgType               host command message that invoked D
*   ->dtxSelect             DTX state for UL and DL
*   ->chSelect              selects type of channel
*   ->dataRate              vocoder (Speech) or data rate (Data)
*
* Output:
*   tch                   ptr to TCH Info structure in DSP (later ChMode substruct?)
*   ->ULDtxMode              uplink DTX state
*   ->DLDtxMode              downlink DTX state
*   ->speechDataInd          speech/data indicator
*   ->vocAlgo                vocoder algorithm            
*
******************************************************************************/
ProcResult channelModeIE(msgType msgType, t_chModes *chModes, t_TNInfo *ts)
{
   ProcResult  result=SUCCESS;
   t_TCHInfo   *tch;   
   
   tch = & ts->u.group1.tch;
   /*----------------------------------------------------------------------------        
   * Set DTX state on uplink and downlink. If UL DTX is ON, set ulDtxUsed TRUE
   * so we average RXLEV SUB and RXQUAL SUB while MS remains in signalling mode.
   *---------------------------------------------------------------------------*/
   tch->ULDtxMode = (chModes->dtxSelect & UL_DTX)==UL_DTX;
   tch->DLDtxMode = (chModes->dtxSelect & DL_DTX)==DL_DTX;
   if ( tch->ULDtxMode )
   {
      tch->ulDtxUsed = TRUE;
      tch->ulDtxUsed104 = TRUE;
   }
   /* Set dlDtxUsed FALSE and don't use DL DTX until speech mode */
   tch->dlDtxUsed = FALSE;
   /*
   *  Make sure DL DTX mode is OFF on BCCH  
   */   
   if(g_BBInfo[0].TNInfo[0].groupNum == GSM_GROUP_5) 
   {                   
      tch->DLDtxMode = FALSE;



   }      
    
   /*-------------------------------------------------------        
   * Translate Host msg chSelect to DSP chSelect 
   *-------------------------------------------------------*/
   switch (chModes->chSelect)
   {
   case SPEECH_CHANNEL:
      tch->speechDataInd = IND_SPEECH;
      break;
   case DATA_CHANNEL:
      tch->speechDataInd = IND_DATA;
      break;
   case SIGNALING_CHANNEL:
      tch->speechDataInd = IND_SIG;
      break;
   default:
      result = FAILURE;
      break;
   }
   /*-------------------------------------------------------        
   * If speech, translate Host vocoder algorithm to DSP
   *-------------------------------------------------------*/
   if ( tch->speechDataInd == IND_SPEECH )
   { 
      switch (chModes->dataRate)
      {
      case VOCODER_ALGORITHM_V1:
         tch->vocAlgo = VOC_GSMF;
         break;
      case VOCODER_ALGORITHM_V2:
         tch->vocAlgo = VOC_EFR;
         break;
      default:
         result = FAILURE;
         break;
      }
   }
   else
   {
      tch->vocAlgo = VOC_NULL;
   }

   return (result);
}

/*****************************************************************************
*
* Function: encryptionIE
*
* Description:
*   Translates Host Encryption Information IE into DSP enumerated values
*
* Input:
*   msgType               host command message that invoked DCH Msg Proc  
*   hostEncryptInfo       encryption state structure
*   .algorithmId            encryption algorithm
*   .key[8]                 encryption key, 8 bytes, [0] is LSByte 
*   encryptState          ptr to encryption structure in DSP
*
* Output:
*   dspEncryptState       ptr to encryption structure in DSP
*   ->algorithm             current encryption algorithm (None,A5/1,A5/2)
*   ->newAlgorithm          newly commanded encryption algorithm
*   ->encryptMode           encryption mode (Normal, Change, Pending)
*   ->encryptKey[2]         encryption key, 2 longwords, [0] is LSWord            
*
******************************************************************************/
ProcResult encryptionIE(msgType msgType, t_encryptInfo  *hostEncryptInfo,
                      t_encryptState *dspEncryptState)
{
   t_encryptAlgorithm   *setAlgorithm;
   ProcResult           result=SUCCESS;
   Uint8                i;
   
   /*-------------------------------------------------------        
   * If Channel Activation, just continue using encryption
   * already set up. If Encr Cmd, change algorithms.  
   *-------------------------------------------------------*/
   if ( msgType == DCH_CH_ACTIVATION_REQ )
   {
      setAlgorithm = & dspEncryptState->algorithm;
      dspEncryptState->encryptMode = ENCR_NORMAL;
   }
   else
   {
      setAlgorithm = & dspEncryptState->newAlgorithm;
      dspEncryptState->encryptMode = ENCR_PEND_CHANGE;
   } 
   
   /*-------------------------------------------------------        
   * Translate Host encryption algorithm to DSP equivalent
   *-------------------------------------------------------*/
   switch (hostEncryptInfo->algorithmId)
   {
   case ENCRYPTION_ALGORITHM_NONE:
      *setAlgorithm = ENCR_NONE;
      break;
   case ENCRYPTION_ALGORITHM_A51:
      *setAlgorithm = ENCR_A51;
      break;
   case ENCRYPTION_ALGORITHM_A52:
      *setAlgorithm = ENCR_A52;
      break;
   default:
      result = FAILURE;
      break;
   }
   /*-------------------------------------------------------        
   * Pack encryption key into longwords
   *-------------------------------------------------------*/
   if ( result == SUCCESS )
   {
      for ( i=0; i<ENCRYPT_KEY_LEN; i++ )
      {
         dspEncryptState->encryptKey[i] =
            (hostEncryptInfo->key[4*i] << 24) | 
            (hostEncryptInfo->key[4*i+1] << 16) | 
            (hostEncryptInfo->key[4*i+2] <<  8) | 
            hostEncryptInfo->key[4*i+3];  
      }
   }
   return (result);
}

/*****************************************************************************
*
* Function: dcchMsgProc
*
* Description:
*   Handles DCCH messages from Host to DSP.
*
* Inputs:  
*   dcchMsg           ptr to CCCH message structure
*   ->trx               TRX      
*   ->typeLoByte        message type
*   ->typeHiByte        message type
*   ->params[]          logical channel, subchannel, timeslot, etc.     
*
* Outputs:
*   g_BBInfo[trx]
*    .TNInfo[timeSlot]  ptr to TN Info structure in DSP
*      .u.group1          GSM Group 1 structure
*         .tch              TCH substructure update params for one TCH/F
*         .sacch            SACCH substuctur update params for its SACCH
*      .u.group5          GSM Group 5 structure
*         .sdcch[]          SDCCH subchannel structures update params
*         .sacch[]          SACCH subchannel structures update params
*
******************************************************************************/
ProcResult dcchMsgProc(msgUnionStruc *dcchMsg)
{                 
   t_DLSigBufs *dcchBuf;
   t_TNInfo    *ts;	
   ProcResult  Result;          
   Uint8       chanNum, i;
   msgStruc    ackMsg;
   Uint8       rfCh;
   Uint8       timeSlot;
   Uint8       subCh;
   Uint8       logCh;	
   Uint16      msgType;
   Uint8       failCause = 0;
   Uint8       hoCause;
   Uint8       params[2];
   t_PNbufs    *PNbufs;
   Uint8       ackWarningFlag;

   chanNum = dcchMsg->u.params[0]; 
   if((chanNum & 0xE0) == 0x20)
   {
      logCh = SDCCH4;
      subCh = (chanNum & 0x18) >> 3;
	}
	else if((chanNum & 0xC0) == 0x40)
	{
      logCh = SDCCH8;
      subCh = (chanNum & 0x38) >> 3;
	}		
   /*-------------------------------------------------------        
   * Set RF channel, time slot and pointer to TS Info struct
   *-------------------------------------------------------*/
   rfCh = dcchMsg->trx & 0x01; 
   timeSlot = chanNum & 0x07;
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   Result = SUCCESS;
   msgType = dcchMsg->typeLoByte |	(dcchMsg->typeHiByte<<8);
	       
   switch(ts->groupNum)
	{
	case GSM_GROUP_1: 
		switch(msgType)
		{
      case DCH_CH_ACTIVATION_REQ:
         /*---------------------------------------------------------        
         * If channel is inactive, process the Chan Activation.
         * If it's already active, set a warning flag.  
         *---------------------------------------------------------*/
         if ( ts->u.group1.tch.state != CHAN_ACTIVE )
         {
            ackWarningFlag = 0;
         }
         else
         {
            ackWarningFlag = 1;
         }     
         /*-------------------------------------------------------        
         * Translate Host Channel Mode IE to DSP enum values
         *-------------------------------------------------------*/
         if ( channelModeIE(msgType, & dcchMsg->u.chActInfo.chModes, ts) != SUCCESS )
         {   
            failCause = 2; 
         }  
         /*-------------------------------------------------------        
         * Translate Host encryption command to DSP enum values
         *-------------------------------------------------------*/
         if ( encryptionIE(msgType, & dcchMsg->u.chActInfo.encryptInfo,
                        & ts->u.group1.tch.encryptState) != SUCCESS )
         {   
            failCause = 3; 
         }  
         /*-------------------------------------------------------        
         * If Channel is activated for Handover, set HO flag
         *-------------------------------------------------------*/
         if ( dcchMsg->u.chActInfo.chActType == ASYNC_HANDOVER_TYPE )
         {
            ts->u.group1.tch.HOFlag = TRUE;
            ts->u.group1.tch.HOReference = dcchMsg->u.chActInfo.hoRef;
         }                                
         else
         {
            ts->u.group1.tch.HOFlag = FALSE;
         }
                          
         ts->u.group1.sacch.lostCount = 0;
         
         /* special case for CHANNEL ACTIVATION for Handover */
         if ( ts->u.group1.tch.HOFlag == TRUE) 
         {
            ts->u.group1.sacch.timingAdvance = 0;
            ts->u.group1.sacch.powerLevelInit = dcchMsg->u.chActInfo.msPowerInit; 
            ts->u.group1.sacch.powerLevel = dcchMsg->u.chActInfo.msPowerInit; 
            ts->u.group1.sacch.ulsigstren.rxgain = g_USGC;

            ts->u.group1.sacch.agcHoldFlag = TRUE;
            ts->u.group1.sacch.dpcHoldFlag = TRUE;
         }
         else 
         {
            /*-------------------------------------------------------        
            * Initialize TA, MS and BTS power, dynamic pwr control
            *-------------------------------------------------------*/
            initPower(& dcchMsg->u.chActInfo, & ts->u.group1.sacch);       
         }

         /*-------------------------------------------------------        
         * Initialize P-N buffers for handover processing
         *-------------------------------------------------------*/
         PNbufs = & g_PNbufs[timeSlot];

         PNbufs->hoUL.sigstren.index = 0;
         PNbufs->hoUL.sigstren.flagsum = 0;
         PNbufs->hoDL.sigstrenSCell.index = 0;
         PNbufs->hoDL.sigstrenSCell.flagsum = 0;

         PNbufs->hoUL.sigqual.index = 0;
         PNbufs->hoUL.sigqual.flagsum = 0;
         PNbufs->hoDL.sigqualSCell.index = 0;
         PNbufs->hoDL.sigqualSCell.flagsum = 0;

         PNbufs->hoUL.intsigstren.index = 0;
         PNbufs->hoUL.intsigstren.flagsum = 0;
         PNbufs->hoDL.intsigstrenSCell.index = 0;
         PNbufs->hoDL.intsigstrenSCell.flagsum = 0;

         for(i=0; i < NXBUF_SIZ_MAX; i++) {  
            PNbufs->hoUL.sigstren.flag[i] = 0;
            PNbufs->hoUL.sigqual.flag[i] = 0;
            PNbufs->hoDL.sigstrenSCell.flag[i] = 0;
            PNbufs->hoDL.sigqualSCell.flag[i] = 0;
            PNbufs->hoUL.intsigstren.flag[i] = 0;
            PNbufs->hoDL.intsigstrenSCell.flag[i] = 0;
         }
         /* end P and N buffer init */

         /*
         * only perform 5.08 averaging when all samples in buffer are collected
         */
         ts->u.group1.sacch.averagingHoldCounter = 0;

         /*
         *  clear stolen SID count
         */
         ts->u.group1.tch.stolenSid = 0;
                               
         /*-------------------------------------------------------        
         * Clear Facch Buffer related parameters
         *-------------------------------------------------------*/
         ts->u.group1.facch.dlSigBuf->numMsgsReady = 0;
         ts->u.group1.facch.dlSigBuf->readIndex = 0;
         ts->u.group1.facch.dlSigBuf->writeIndex = 0;
         /*-------------------------------------------------------        
         * Clear Sacch Buffer related parameters
         *-------------------------------------------------------*/
         ts->u.group1.sacch.dlSigBuf->numMsgsReady = 0;
         ts->u.group1.sacch.dlSigBuf->readIndex = 0;
         ts->u.group1.sacch.dlSigBuf->writeIndex = 0;
         /*-------------------------------------------------------        
         * Initialize TCH interleave/de-interleave block index
         *-------------------------------------------------------*/
         g_intBuf8Deep.tchTXBurstIndex[(timeSlot)] = -1;             
         g_deintBuf8Deep.tchRXBurstIndex[(timeSlot)] = -1;
            
         rtsCheck(dcchMsg->trx, ts->tsNumber, FACCH, 0, 0);			                 

         /*---------------------------------------------------
         * Initialize TCH jitter buffer related parameters  			
         *---------------------------------------------------*/
         g_dlTchFrame[timeSlot].frameCount = 0;   
         g_dlTchFrame[timeSlot].readIndex = 0;      
         g_dlTchFrame[timeSlot].writeIndex = 0;			         
         g_dlTchFrame[timeSlot].frameStarted = 0;  

         /*---------------------------------------------------
         * Initialize SID frame count   			
         *---------------------------------------------------*/            
         g_dlSidFrame[timeSlot].frameCount = 0;

         /*---------------------------------------------------
         * Initialize time slot DC offset to all-carrier value   			
         *---------------------------------------------------*/            
         g_DcOffset[rfCh][timeSlot].I =
            g_DcOffset[rfCh][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS].I;
         g_DcOffset[rfCh][timeSlot].Q =
            g_DcOffset[rfCh][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS].Q;

         /*---------------------------------------------
         * Initialize diagnostic MS Report parameters
         *--------------------------------------------*/
         initMsReport(rfCh, timeSlot);
                   
         /*-------------------------------------------------------
         * Load common parameters for response msg to L3   			
         *-------------------------------------------------------*/
         ackMsg.msgSize = 9;
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.params[0] = dcchMsg->u.params[0];  /* channel number */
         ackMsg.params[1] = dcchMsg->u.params[1];  /* channel number */
         /*-------------------------------------------------------
         * If error, send NACK with failCause   			
         *-------------------------------------------------------*/
         if ( failCause )
         {
            ackMsg.typeLoByte = DCH_CH_ACTIVATION_REQ_NACK;
            ackMsg.typeHiByte = DCH_CH_ACTIVATION_REQ_NACK>>8;            
            ackMsg.params[2] = 0;
            ackMsg.params[3] = failCause;
         }
         /*-------------------------------------------------------
         * If no error, send ACK with FN  			
         *-------------------------------------------------------*/
         else
         {
            ackMsg.typeLoByte = DCH_CH_ACTIVATION_REQ_ACK;
            ackMsg.typeHiByte = DCH_CH_ACTIVATION_REQ_ACK>>8;            
            ackMsg.params[2] = (sysFrameNum.t1 & 0x001F) << 3 |
                                 (sysFrameNum.t3 & 0x38) >> 3;
            ackMsg.params[3] = (sysFrameNum.t3 & 0x03) << 5 |
            	                  (sysFrameNum.t2 & 0x1F);
            ackMsg.params[4] = ackWarningFlag;
         }
         sendMsgHost(& ackMsg);           	     
         
         /*---------------------------------------------------
         * If Host message is valid, activate channel   			
         *---------------------------------------------------*/
         if ( !failCause )
         {
            ts->u.group1.tch.state = CHAN_ACTIVE;
            ts->u.group1.sacch.state = CHAN_ACTIVE;
         }         
         break;
		      
      case DCH_CH_RELEASE_REQ:

         if(g_AGC_ON) {      
           ts->u.group1.sacch.ulsigstren.rxgain = g_rxGainMinIdle;
         }
         ts->u.group1.tch.state = CHAN_INACTIVE;
         ts->u.group1.sacch.state = CHAN_INACTIVE; 
           
         /*
          * RESET; only perform 5.08 averaging when all samples in buffer are collected
          */
         ts->u.group1.sacch.averagingHoldCounter = 0;

         /*---------------------------------------------
         * Clear diagnostic MS Report parameters
         *--------------------------------------------*/
         initMsReport(rfCh, timeSlot);
         
         /*---------------------------------------------
         * Send ack message to Layer 3
         *--------------------------------------------*/
         ackMsg.msgSize = 6;
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.typeLoByte = DCH_CH_RELEASE_REQ_ACK;
         ackMsg.typeHiByte = DCH_CH_RELEASE_REQ_ACK>>8;            
         ackMsg.params[0] = dcchMsg->u.params[0];
         ackMsg.params[1] = dcchMsg->u.params[1];
         
         sendMsgHost(& ackMsg);                                    
         break;
		               		      
      case DCH_SACCH_DEACTIVATE_REQ:
         ts->u.group1.sacch.state = CHAN_INACTIVE;
         break;
		
      case DCH_SACCH_INFO_MOD_REQ:
//         sacchMsgProc( (sacchMsgStruc *) &dcchMsg->u.dchSacchInfo.sysInfoType, 
//                       ts->u.group1.sacch.dlSigBuf, type); ***NOT USED*** 						
         break;
		
      case DCH_MODE_MOD_REQ:
         /*-------------------------------------------------------        
         * Check that channel is active, else reject Mode Modify
         *-------------------------------------------------------*/
         if ( ts->u.group1.tch.state == CHAN_ACTIVE )
         {
            /*-------------------------------------------------------        
            * Translate Host Channel Mode IE to DSP enum values
            *-------------------------------------------------------*/
            if ( channelModeIE(msgType, &dcchMsg->u.chActInfo.chModes, ts) != SUCCESS )
            {   
               failCause = 2; 
            }
         }
         else
         {
            failCause = 1;
         }  
         /*-------------------------------------------------------
         * Load common parameters for response msg to L3   			
         *-------------------------------------------------------*/
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.params[0] = dcchMsg->u.params[0];  /* channel number */
         ackMsg.params[1] = dcchMsg->u.params[1];  /* channel number */
         /*-------------------------------------------------------
         * If error, send NACK with failCause   			
         *-------------------------------------------------------*/
         if ( failCause )
         {
            ackMsg.msgSize = 8;
            ackMsg.typeLoByte = DCH_MODE_MOD_REQ_NACK;
            ackMsg.typeHiByte = DCH_MODE_MOD_REQ_NACK>>8;            
            ackMsg.params[2] = 0;
            ackMsg.params[3] = failCause;
         }
         /*-------------------------------------------------------
         * If no error, send ACK without last field  			
         *-------------------------------------------------------*/
         else
         {
            ackMsg.msgSize = 6;
            ackMsg.typeLoByte = DCH_MODE_MOD_REQ_ACK;
            ackMsg.typeHiByte = DCH_MODE_MOD_REQ_ACK>>8;            
         }
         sendMsgHost(& ackMsg);           	     
                 
         break;
              
      case DCH_OM_ASYNC_HO_REQ:
         hoCause = DL_RXLEV | UL_RXLEV | DL_RXQUAL | UL_RXQUAL;
         sendAsyncHoMsg(dcchMsg->trx, timeSlot, hoCause);           
         break;      
        
      case DCH_ENCRYPT_REQ:
         /*-------------------------------------------------------        
         * Translate Host encryption command to DSP enum values   
         *-------------------------------------------------------*/
         if ( encryptionIE(msgType, & dcchMsg->u.encryptCmdInfo.encryptInfo,
                    & ts->u.group1.tch.encryptState) != SUCCESS )
         {
            params[0] = DCH_ENCRYPT_REQ >> 8;
            params[1] = DCH_ENCRYPT_REQ & 0xff;      
            ReportError(COMMAND_FAILED,dcchMsg->trx,timeSlot,2,params);
         }
         break;      
        
      default:
         ReportError(UNKNOWN_MSG_TYPE, dcchMsg->trx, timeSlot,0,params);         
         break;
      }
      break;              
            			
   case GSM_GROUP_5:		                                 
      switch(msgType)
      {
      case DCH_CH_ACTIVATION_REQ:
         /*---------------------------------------------------------        
         * If channel is inactive, process the Chan Activation.
         * If it's already active, set a warning flag.  
         *---------------------------------------------------------*/
         if ( ts->u.group5.sdcch[subCh].state != CHAN_ACTIVE )
         {
	         ackWarningFlag = 0;
         }
         else
         {
            ackWarningFlag = 1;
         }     
         /*-------------------------------------------------------        
         * Initialize message buffer and lost SACCH cound
         *-------------------------------------------------------*/
         dcchBuf = ts->u.group5.sdcch[subCh].dlSigBuf;
         dcchBuf->valid = 1;
         dcchBuf->readIndex = 0;
         dcchBuf->writeIndex = 0;
         dcchBuf->numMsgsReady = 0;
         ts->u.group5.sacch[subCh].lostCount = 0;
		 
         /*-------------------------------------------------------        
         * Clear Sacch Buffer related parameters
         *-------------------------------------------------------*/
         ts->u.group5.sacch[subCh].dlSigBuf->numMsgsReady = 0;
         ts->u.group5.sacch[subCh].dlSigBuf->readIndex = 0;
         ts->u.group5.sacch[subCh].dlSigBuf->writeIndex = 0;

         /*
         *  Initialize  Cell Broadcasting Buffer
         */ 
         if(subCh == 2)
         {             
            if(dcchMsg->u.chActInfo.chActType == CBCH_TYPE)
               ts->u.group5.cbchUsed = TRUE;
            else                            
               ts->u.group5.cbchUsed = FALSE;               
         }

         /*-------------------------------------------------------        
         * Translate Host encryption command to DSP enum values
         *-------------------------------------------------------*/
         if ( encryptionIE(msgType, & dcchMsg->u.chActInfo.encryptInfo,
                        & ts->u.group5.sdcch[subCh].encryptState) != SUCCESS )
         {   
            failCause = 3; 
         }
         
         /*-------------------------------------------------------        
         * Initialize TA, MS and BTS power, dynamic pwr control
         *-------------------------------------------------------*/
         initPower(& dcchMsg->u.chActInfo, & ts->u.group5.sacch[subCh]);  	
  	              
         /*-------------------------------------------------------
         * Set local handover reference  			                              
         *-------------------------------------------------------*/
         ts->u.group5.sdcch[subCh].HOReference = dcchMsg->u.chActInfo.hoRef;
            
         /*---------------------------------------------------
         * Initialize subchan DC offset to all-carrier value   			
         *---------------------------------------------------*/            
         g_DcOffset[rfCh][NUM_TN_PER_RF+subCh].I =
            g_DcOffset[rfCh][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS].I;
         g_DcOffset[rfCh][NUM_TN_PER_RF+subCh].Q =
            g_DcOffset[rfCh][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS].Q;
               
         /*---------------------------------------------
         * Initialize diagnostic MS Report parameters
         *--------------------------------------------*/
         initMsReport(rfCh, NUM_TN_PER_RF+subCh);

         /*-------------------------------------------------------
         * Load common parameters for response msg to L3   			
         *-------------------------------------------------------*/
         ackMsg.msgSize = 9;
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.params[0] = dcchMsg->u.params[0];  /* channel number */
         ackMsg.params[1] = dcchMsg->u.params[1];  /* channel number */
         /*-------------------------------------------------------
         * If error, send NACK with failCause   			
         *-------------------------------------------------------*/
         if ( failCause )
         {
            ackMsg.typeLoByte = DCH_CH_ACTIVATION_REQ_NACK;
            ackMsg.typeHiByte = DCH_CH_ACTIVATION_REQ_NACK>>8;            
            ackMsg.params[2] = 0;
            ackMsg.params[3] = failCause;
         }
         /*-------------------------------------------------------
         * If no error, send ACK with FN  			
         *-------------------------------------------------------*/
         else
         {
            ackMsg.typeLoByte = DCH_CH_ACTIVATION_REQ_ACK;
            ackMsg.typeHiByte = DCH_CH_ACTIVATION_REQ_ACK>>8;            
            ackMsg.params[2] = (sysFrameNum.t1 & 0x001F) << 3 |
                                 (sysFrameNum.t3 & 0x38) >> 3;
            ackMsg.params[3] = (sysFrameNum.t3 & 0x03) << 5 |
            	                  (sysFrameNum.t2 & 0x1F);
            ackMsg.params[4] = ackWarningFlag;
         }
         sendMsgHost(& ackMsg);           	     
			           	               			
         /*---------------------------------------------------
         * If the Chan Act was valid, flag the channel active   			
         *---------------------------------------------------*/
         if ( !failCause )
         {
            ts->u.group5.sdcch[subCh].state = CHAN_ACTIVE;
            ts->u.group5.sacch[subCh].state = CHAN_ACTIVE; 
            rtsCheck(dcchMsg->trx, ts->tsNumber, SDCCH4, subCh, 0);
         }
         break;
                      
      case DCH_CH_RELEASE_REQ:
         ts->u.group5.sdcch[subCh].state = CHAN_INACTIVE;     
         ts->u.group5.sacch[subCh].state = CHAN_INACTIVE;
         
         if(subCh == 2)
         {
            ts->u.group5.cbchUsed = FALSE;
            ts->u.group5.cbch[0].dlSigBuf->bufValid = 0;
            ts->u.group5.cbch[1].dlSigBuf->bufValid = 0;            
         }
         /*---------------------------------------------
         * Clear diagnostic MS Report parameters
         *--------------------------------------------*/
         initMsReport(rfCh, NUM_TN_PER_RF+subCh);
         
         /*---------------------------------------------
         * Send ack message to Layer 3
         *--------------------------------------------*/
         ackMsg.msgSize = 6;
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.typeLoByte = DCH_CH_RELEASE_REQ_ACK;
         ackMsg.typeHiByte = DCH_CH_RELEASE_REQ_ACK>>8;
         ackMsg.params[0] = dcchMsg->u.params[0];
         ackMsg.params[1] = dcchMsg->u.params[1];
         sendMsgHost(& ackMsg);             
         break;
		                      
      case DCH_SACCH_DEACTIVATE_REQ:
         ts->u.group5.sacch[subCh].state = CHAN_INACTIVE;
         break;
      case DCH_SACCH_INFO_MOD_REQ:
         /*
         * update SACCH for selected sub channel
         */
//         sacchMsgProc( (sacchMsgStruc *) &dcchMsg->u.dchSacchInfo.sysInfoType, 
//                        ts->u.group5.sacch[subCh].dlSigBuf, type); ***NOT USED*
         break;
					      
      case DCH_ENCRYPT_REQ:
         /*-------------------------------------------------------        
         * Translate Host encryption command to DSP enum values   
         *-------------------------------------------------------*/
         if ( encryptionIE(msgType, & dcchMsg->u.encryptCmdInfo.encryptInfo,
                    & ts->u.group5.sdcch[subCh].encryptState) != SUCCESS )
         {
            params[0] = DCH_ENCRYPT_REQ >> 8;
            params[1] = DCH_ENCRYPT_REQ & 0xff;      
            ReportError(COMMAND_FAILED,dcchMsg->trx,timeSlot,2,params);
         }	
         break;      
        
      default: 
         ReportError(UNKNOWN_MSG_TYPE, dcchMsg->trx, timeSlot, 0, params);      
         break;
      }
      break;

            			
   case GSM_GROUP_7:

         

      switch(msgType)
      {
      case DCH_CH_ACTIVATION_REQ:
	
    
         /*---------------------------------------------------------        
         * If channel is inactive, process the Chan Activation.
         * If it's already active, set a warning flag.  
         *---------------------------------------------------------*/
         if ( ts->u.group7.sdcch[subCh].state != CHAN_ACTIVE )
         {
	         ackWarningFlag = 0;
         }
         else
         {
            ackWarningFlag = 1;
         }     
         /*-------------------------------------------------------        
         * Initialize message buffer and lost SACCH cound
         *-------------------------------------------------------*/
         dcchBuf = ts->u.group7.sdcch[subCh].dlSigBuf;
         dcchBuf->valid = 1;
         dcchBuf->readIndex = 0;
         dcchBuf->writeIndex = 0;
         dcchBuf->numMsgsReady = 0;
         ts->u.group7.sacch[subCh].lostCount = 0;
		 
         /*-------------------------------------------------------        
         * Clear Sacch Buffer related parameters
         *-------------------------------------------------------*/
         ts->u.group7.sacch[subCh].dlSigBuf->numMsgsReady = 0;
         ts->u.group7.sacch[subCh].dlSigBuf->readIndex = 0;
         ts->u.group7.sacch[subCh].dlSigBuf->writeIndex = 0;

    

         /*-------------------------------------------------------        
         * Translate Host encryption command to DSP enum values
         *-------------------------------------------------------*/
         if ( encryptionIE(msgType, & dcchMsg->u.chActInfo.encryptInfo,
                        & ts->u.group7.sdcch[subCh].encryptState) != SUCCESS )
         {   
            failCause = 3; 
         }
         
         /*-------------------------------------------------------        
         * Initialize TA, MS and BTS power, dynamic pwr control
         *-------------------------------------------------------*/
         initPower(& dcchMsg->u.chActInfo, & ts->u.group7.sacch[subCh]);  	
  	              
         /*-------------------------------------------------------
         * Set local handover reference  			                              
         *-------------------------------------------------------*/
         ts->u.group7.sdcch[subCh].HOReference = dcchMsg->u.chActInfo.hoRef;
            
         /*---------------------------------------------------
         * Initialize subchan DC offset to all-carrier value   			
         *---------------------------------------------------*/            
         g_Dc_7_Offset[rfCh][subCh].I =
            g_DcOffset[rfCh][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS].I;
         g_Dc_7_Offset[rfCh][subCh].Q =
            g_DcOffset[rfCh][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS].Q;
               
         /*---------------------------------------------
         * Initialize diagnostic MS Report parameters
         *--------------------------------------------*/
         initSdcch8Report(rfCh, subCh);

         /*-------------------------------------------------------
         * Load common parameters for response msg to L3   			
         *-------------------------------------------------------*/
         ackMsg.msgSize = 9;
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.params[0] = dcchMsg->u.params[0];  /* channel number */
         ackMsg.params[1] = dcchMsg->u.params[1];  /* channel number */
         /*-------------------------------------------------------
         * If error, send NACK with failCause   			
         *-------------------------------------------------------*/
         if ( failCause )
         {
            ackMsg.typeLoByte = DCH_CH_ACTIVATION_REQ_NACK;
            ackMsg.typeHiByte = DCH_CH_ACTIVATION_REQ_NACK>>8;            
            ackMsg.params[2] = 0;
            ackMsg.params[3] = failCause;
         }
         /*-------------------------------------------------------
         * If no error, send ACK with FN  			
         *-------------------------------------------------------*/
         else
         {
            ackMsg.typeLoByte = DCH_CH_ACTIVATION_REQ_ACK;
            ackMsg.typeHiByte = DCH_CH_ACTIVATION_REQ_ACK>>8;            
            ackMsg.params[2] = (sysFrameNum.t1 & 0x001F) << 3 |
                                 (sysFrameNum.t3 & 0x38) >> 3;
            ackMsg.params[3] = (sysFrameNum.t3 & 0x03) << 5 |
            	                  (sysFrameNum.t2 & 0x1F);
            ackMsg.params[4] = ackWarningFlag;
         }
         sendMsgHost(& ackMsg);           	     
			           	               			
         /*---------------------------------------------------
         * If the Chan Act was valid, flag the channel active   			
         *---------------------------------------------------*/
         if ( !failCause )
         {
            ts->u.group7.sdcch[subCh].state = CHAN_ACTIVE;
            ts->u.group7.sacch[subCh].state = CHAN_ACTIVE; 
            rtsCheck(dcchMsg->trx, ts->tsNumber, SDCCH4, subCh, 0);
         }


	
         break;
                      
      case DCH_CH_RELEASE_REQ:
         ts->u.group7.sdcch[subCh].state = CHAN_INACTIVE;     
         ts->u.group7.sacch[subCh].state = CHAN_INACTIVE;
        
         /*---------------------------------------------
         * Clear diagnostic MS Report parameters
         *--------------------------------------------*/
         initSdcch8Report(rfCh, subCh);
         
         /*---------------------------------------------
         * Send ack message to Layer 3
         *--------------------------------------------*/
         ackMsg.msgSize = 6;
         ackMsg.function =  dcchMsg->function;
         ackMsg.trx =  dcchMsg->trx;
         ackMsg.typeLoByte = DCH_CH_RELEASE_REQ_ACK;
         ackMsg.typeHiByte = DCH_CH_RELEASE_REQ_ACK>>8;
         ackMsg.params[0] = dcchMsg->u.params[0];
         ackMsg.params[1] = dcchMsg->u.params[1];
         sendMsgHost(& ackMsg);             
         break;
		                      
      case DCH_SACCH_DEACTIVATE_REQ:
         ts->u.group7.sacch[subCh].state = CHAN_INACTIVE;
         break;
      case DCH_SACCH_INFO_MOD_REQ:
         /*
         * update SACCH for selected sub channel
         */
//         sacchMsgProc( (sacchMsgStruc *) &dcchMsg->u.dchSacchInfo.sysInfoType, 
//                        ts->u.group7.sacch[subCh].dlSigBuf, type); ***NOT USED*
         break;
					      
      case DCH_ENCRYPT_REQ:
         /*-------------------------------------------------------        
         * Translate Host encryption command to DSP enum values   
         *-------------------------------------------------------*/
         if ( encryptionIE(msgType, & dcchMsg->u.encryptCmdInfo.encryptInfo,
                    & ts->u.group7.sdcch[subCh].encryptState) != SUCCESS )
         {
            params[0] = DCH_ENCRYPT_REQ >> 8;
            params[1] = DCH_ENCRYPT_REQ & 0xff;      
            ReportError(COMMAND_FAILED,dcchMsg->trx,timeSlot,2,params);
         }	
         break;      
        
      default: 
         ReportError(UNKNOWN_MSG_TYPE, dcchMsg->trx, timeSlot, 0, params);      
         break;
      }
      break;		
   default:
      ReportError(TIME_SLOT_NOT_CONFIGURED, dcchMsg->trx, timeSlot, 0, params);
      break;
   }
	
   return(Result);		
}

/*****************************************************************************
*
* Function: sacchMsgProc
*
* Description:
*   Handles SACCH messages from Host to DSP.
*
* Inputs:  
*   sacchMsg          ptr to CCCH message structure
*   ->startTime         fill in      
*   ->sysInfo           fill in
*
* Outputs:
*   sacchBuf          fill in
*   sysInfoType       fill in
*
******************************************************************************/
void sacchMsgProc(sacchMsgStruc *sacchMsg, t_sysInfoQueue *sacchBuf, Uint8 sysInfoType)
{                            
   Uint8 bufferIndex;   
   bufferIndex = sacchBuf->state ^ 0x01;
   
   /*
   * if message size is 0, stop sending this SYS INFO on the SACCH
   */
   if(sacchMsg->msgSize == 0)
   {
      sacchBuf->valid = 0;
   }
   else
   {    
      memcpy((Uint8 *) (& sacchBuf->data[bufferIndex][5]), & sacchMsg->sysInfo[0], 19);
      /*
      *  added L2 Headers
      */		
      sacchBuf->data[bufferIndex][2] = 0x03;
      sacchBuf->data[bufferIndex][3] = 0x03;                     
      /*
      * for  SYS_INFO_6, pseudo length ix 0x31, rest is 0x49
      */
      if( sysInfoType ==  SYS_INFO_6)
         sacchBuf->data[bufferIndex][4] = 0x2d;
      else			          
         sacchBuf->data[bufferIndex][4] = 0x49;		

      /*
      * if change is immediate, change active buffer now
      */
      if(sacchMsg->startTime.IMM)
      {
         sacchBuf->state = bufferIndex;
         /*                                                            
         * start sending this sysInfo type
         */	
         sacchBuf->valid = 1; 			
		}
		else
		{
			/*
			* set sysinfo update frame number
			*/		                                          
			sacchBuf->frameNumUpdate = 
					(sacchMsg->startTime.timeByte0 & 0xf8)>>3 *1326 +   /*t1*/
					
					((sacchMsg->startTime.timeByte0 & 0x07)<<3 |   /*t3*/
					(sacchMsg->startTime.timeByte1 & 0xE0)>>5) *
									              
					(sacchMsg->startTime.timeByte1 & 0x1F)>>3;    /*t2*/
		}
	}
}
