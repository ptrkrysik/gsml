/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
*
* File: oammsg.c
* 
* Description:
*   This file contains functionality needed for parsing OA&M 
*   messages from the layer 3 (RRM).  Inside these messages, exists
*   the (re) parameterization of GSM functionality for dynamic power 
*   control and handover.  
*
******************************************************************************/           
#include "string.h"
#include "math.h"
#include "gsmdata.h"
#include "oamdata.h"
#include "dsp/dsphmsg.h"        
#include "l1config.h"
#include "regs.h"
#include "dsp6201.h"
#include "agcdata.h"
#include "dsprotyp.h"
#include <intr.h>

/*****************************************************************************
*
* Function: dBmToReductionXlate
*
* Description:
*   Translates power in dBm to Power Reduction Value 0-15.
*   Used only by oamMsgProc in same file.
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dBmToReductionXlate(Bool serveCell, Uint16 index, Uint8 dBmValue) 
{
   Uint8 pwrReductionValue;
   
   /*
   *  0-1 dBm translates to Reduction Value 15, 2-3 dBm translates to 14, etc.
   */
   if ( dBmValue <= 30 )
   {
      pwrReductionValue = 15 - dBmValue/2;
   }
   else
   {
      pwrReductionValue = 0;
   } 
   
   /*
   *  Copy result to serving cell or neighbor as appropriate
   */
   if (serveCell)
   {
      g_HoOAMInfo.serveParms.msTxPwrMax = pwrReductionValue;
   }
   else
   {
      g_HoOAMInfo.uniqueParms[index].msTxPwrMax = pwrReductionValue;   
   }
    
   return;
}

/*****************************************************************************
*
* Function: oamMsgProc
*
* Description:
*   Processes OAM Messeges received from the host and populates the global
*   data structures for dynamic power control and handover OA&M parameters
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
ProcResult  oamMsgProc(msgStruc *p_msg)
{
  ProcResult   result;
  Uint16       index;
  Uint8        indxcnt, i, j;
  static Uint8 flushBfrFlag = 0;
  static Uint8 indxcntr;
  Uint16       msgType;
  Uint8        rfCh;
  Uint8        timeSlot;
  msgStruc     respMsg;
  t_BBInfo     *bbInfo;
  Int16        txPwrOffsetForArfcn=0;
  Uint8        temp[36];
  t_PNbufs     *PNbufs;
  t_TNInfo     *ts;
  Bool         dlDtxMode;
  Bool         ulDtxMode;  

  result = SUCCESS;       /* set default result */
  rfCh = p_msg->trx & 0x1; 
  bbInfo = & g_BBInfo[rfCh];
  msgType = (p_msg->typeHiByte << 8) | (p_msg->typeLoByte);

  switch (msgType)
  {
    case OAM_DPC_CONFIG:
      /* sample buffer size for averaging.  To get an average from a sacch frame takes 480 ms */
      g_DynPwrOAMInfo.ulsigstren.numRxSigSamples = p_msg->params[0];     
      g_DynPwrOAMInfo.ulsigqual.numRxSigSamples = p_msg->params[1]; 
      
      /* set threshold for activating uplink dynamic power control (RXLEV_UL_P) */
      g_DpcRxLevThresh = p_msg->params[2];
       
      temp[0] = g_DynPwrOAMInfo.ulsigstren.numRxSigSamples;  
      temp[1] = g_DynPwrOAMInfo.ulsigqual.numRxSigSamples;       
      temp[2] = g_DpcRxLevThresh;

      sendDiagMsg(0x99,0,0,3,temp); 

      /* Send Acknowledgement */
      respMsg.msgSize = 4;      
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte =  OAM_DPC_CONFIG_ACK >> 8;  
      respMsg.typeLoByte =  OAM_DPC_CONFIG_ACK;  
      respMsg.trx = 0;   /* for now */
      SendMsgHost_Que(&respMsg);
      
      break;

    case OAM_HO_SCELL_CONFIG:
      /* these parameters are used by serving cell only */

      /* uplink threshold in RXLEV units */
      g_DynPwrOAMInfo.ulsigstren.LRxPwrULH = p_msg->params[0];
              
      /* downlink threshold in RXLEV units */ 
      g_DynPwrOAMInfo.dlsigstrenSCell.LRxLevDLH = p_msg->params[1];

      /* P-N downlink or uplink as defined in 05.08 */
      g_DynPwrOAMInfo.sigstren.pxH = p_msg->params[2];
      g_DynPwrOAMInfo.sigstren.nxH = p_msg->params[3];

      /* uplink threshold */
      g_DynPwrOAMInfo.ulsigqual.LRxQualULH = p_msg->params[4];

      /* downlink threshold */ 
      g_DynPwrOAMInfo.dlsigqualSCell.LRxQualDLH = p_msg->params[5];

      /* P-N downlink or uplink as defined in 05.08 */
      g_DynPwrOAMInfo.sigqual.pxH = p_msg->params[6];
      g_DynPwrOAMInfo.sigqual.nxH = p_msg->params[7];

      /* defaults for serving cell or undefined adjacent cells */
      g_HoOAMInfo.serveParms.hoMargin = p_msg->params[8];

      /* need to translate GSM 12.20 range in dBm [+30..0] to power level range [0..15] */
      dBmToReductionXlate(TRUE, 0, p_msg->params[9]);

      g_HoOAMInfo.serveParms.rxLevMin = p_msg->params[10];

      /*-------------------------------------------------------        
      * Initialize P-N buffers for handover processing
      *-------------------------------------------------------*/
      INTR_GLOBAL_DISABLE();

      for (i=0; i<NUM_TS_PER_RF; i++) {
        PNbufs = & g_PNbufs[i];

        PNbufs->hoUL.sigstren.index = 0;
        PNbufs->hoUL.sigstren.flagsum = 0;
        PNbufs->hoDL.sigstrenSCell.index = 0;
        PNbufs->hoDL.sigstrenSCell.flagsum = 0;

        PNbufs->hoUL.sigqual.index = 0;
        PNbufs->hoUL.sigqual.flagsum = 0;
        PNbufs->hoDL.sigqualSCell.index = 0;
        PNbufs->hoDL.sigqualSCell.flagsum = 0;

        for(j=0; j < NXBUF_SIZ_MAX; j++) {  
          PNbufs->hoUL.sigstren.flag[j] = 0;
          PNbufs->hoUL.sigqual.flag[j] = 0;
          PNbufs->hoDL.sigstrenSCell.flag[j] = 0;
          PNbufs->hoDL.sigqualSCell.flag[j] = 0;
        }
      }

      INTR_GLOBAL_ENABLE();
      /* end P and N buffer init */

      /* !! DEBUG */
      temp[0] = g_DynPwrOAMInfo.ulsigstren.LRxPwrULH;   
      temp[1] = g_DynPwrOAMInfo.dlsigstrenSCell.LRxLevDLH; 
      temp[2] = g_DynPwrOAMInfo.sigstren.pxH;
      temp[3] = g_DynPwrOAMInfo.sigstren.nxH;
      temp[4] = g_DynPwrOAMInfo.ulsigqual.LRxQualULH;   
      temp[5] = g_DynPwrOAMInfo.dlsigqualSCell.LRxQualDLH; 
      temp[6] = g_DynPwrOAMInfo.sigqual.pxH;
      temp[7] = g_DynPwrOAMInfo.sigqual.nxH;
      temp[8] = g_HoOAMInfo.serveParms.hoMargin;
      temp[9] = g_HoOAMInfo.serveParms.msTxPwrMax;
      temp[10] = g_HoOAMInfo.serveParms.rxLevMin;  

        sendDiagMsg(0x99,0,0,11,temp); 

      /* Send Acknowledgement */
      respMsg.msgSize = 4;
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte = OAM_HO_SCELL_CONFIG_ACK >> 8;
      respMsg.typeLoByte = OAM_HO_SCELL_CONFIG_ACK; 
      respMsg.trx = p_msg->trx;  
      SendMsgHost_Que(&respMsg);
 
      break; 

    case OAM_HO_NCELL_CONFIG:
      
      /* parameters unique to a (adjacent) neighbouring cell only */  
    
      /* AdjCell_HandoverEntry.adjCell_handoverIndex (0..32) from vipermib.h is for t_HoOAMInfo */ 
      index = p_msg->params[0];

      /* number available indices to expect. no dsp report if this value is never met. */
      indxcnt = p_msg->params[1];

      /* flush all previous entries in global and local memory for neighbour cells */;
      if(flushBfrFlag == 0) {      

        flushBfrFlag = 1;    /* set */
        indxcntr = indxcnt;
        for(j=0; j < NUM_NCELLS; j++) {  
          g_HoOAMInfo.uniqueParms[j].rfCh = 0; 
          g_HoOAMInfo.uniqueParms[j].bsic = 0;
          g_HoOAMInfo.uniqueParms[j].hoMargin = 0;
          g_HoOAMInfo.uniqueParms[j].msTxPwrMax = 0;
          g_HoOAMInfo.uniqueParms[j].rxLevMin = 0;
        }           
      }

      /* AdjCell_HandoverEntry.adjCell_bCCHFrequency from vipermib.h = ARFCN value */
      g_HoOAMInfo.uniqueParms[index].rfCh = (p_msg->params[2] << 8) | p_msg->params[3];

      /* AdjCell_HandoverEntry.adjCell_ncc (3 bit sub field in bsic) from vipermib.h */   
      g_HoOAMInfo.uniqueParms[index].bsic = p_msg->params[4];

      /* AdjCell_HandoverEntry.adjCell_hoMargin from vipermib.h */ 
      g_HoOAMInfo.uniqueParms[index].hoMargin = p_msg->params[5];
      
      /* AdjCell_HandoverEntry.adjCell_msTxPwrMaxCell from vipermib.h */
      
      /* need to translate GSM 12.20 range in dBm [+30..0] to power level range [0..15] */
      dBmToReductionXlate(FALSE, index, p_msg->params[6]);
     
      /* AdjCell_HandoverEntry.adjCell_rxLevMinCell from vipermib.h */
      g_HoOAMInfo.uniqueParms[index].rxLevMin = p_msg->params[7];

      /*  reset the flag to flush previous entries for next time ViperWatch is modified */
      indxcntr -= 1;

      if(indxcntr == 0) {
        flushBfrFlag = 0;             
      }
  
      /* !! DEBUG */  
      temp[0] = index;   
      temp[1] = indxcnt; 
      temp[2] = g_HoOAMInfo.uniqueParms[index].rfCh;
      temp[3] = g_HoOAMInfo.uniqueParms[index].bsic;
      temp[4] = g_HoOAMInfo.uniqueParms[index].hoMargin;
      temp[5] = g_HoOAMInfo.uniqueParms[index].msTxPwrMax;
      temp[6] = g_HoOAMInfo.uniqueParms[index].rxLevMin;
      temp[7] = flushBfrFlag;
      temp[8] = indxcntr;
      sendDiagMsg(0x99,0,0,9,temp);

       /* Send Acknowledgement */
      respMsg.msgSize = 5;
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte = OAM_HO_NCELL_CONFIG_ACK >> 8;
      respMsg.typeLoByte = OAM_HO_NCELL_CONFIG_ACK; 
      respMsg.trx = p_msg->trx;  
      respMsg.params[0] = index;
      SendMsgHost_Que(&respMsg);

      break;

    case OAM_TXPWR_MAX_RDCT_CONFIG:

      /* 
      * Convert Tx Max Power Reduction to DAC input range 0-255. Successive
      * table entries corresponds with half-decibel decrements in Tx power.
      * Frequency-dependent offset is expressed in half-decibel units, too.
      */
      g_TxStaticGainLevel = p_msg->params[0];
      index =  4 * g_TxStaticGainLevel + g_CurrentTxPwrOffsetForArfcn;
      if ( index >= NUM_TX_VS_PWR_LEVEL )
      {
         index = NUM_TX_VS_PWR_LEVEL - 1;
      }  
      g_DSPC = g_TxMaxPwrRdct2DLPwrTbl[ index ];
      
      /* !! DEBUG */ 
      temp[0] = g_DSPC;
      sendDiagMsg(0x99,0,0,1,temp);

       /* Send Acknowledgement */
      respMsg.msgSize = 5;
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte = OAM_TXPWR_MAX_RDCT_CONFIG_ACK >> 8;
      respMsg.typeLoByte = OAM_TXPWR_MAX_RDCT_CONFIG_ACK; 
      respMsg.trx = p_msg->trx;  
      respMsg.params[0] = g_DSPC;
      SendMsgHost_Que(&respMsg);
      break;

      
    case OAM_PRACH_CONFIG:

            if (p_msg->params[0] == 11){
                    g_PrachBits  = 11;
            } else {
                    g_PrachBits  = 8;
            }
        /* Send Acknowledgement */
      respMsg.msgSize = 4;
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte = OAM_PRACH_CONFIG_ACK >> 8;
      respMsg.typeLoByte = OAM_PRACH_CONFIG_ACK; 
      respMsg.trx = p_msg->trx;  
 
      SendMsgHost_Que(&respMsg);


    break;
            
          
    case OAM_G11_CONFIG:
            /* This function is used to configure the group 11 parameters  */
            /*  param 0 number of PBCCH blocks (1..4)
            /*  param 1 number of non PPCH and PBCCH blocks */
            /*  param 2 number of PRACH blocks (0..12)
            /*  param 3 PSI1_REPEAT_CYCLE (1..16)  */

            
            

            if ((p_msg->params[0]>0) &&  (p_msg->params[0] <5)){
                    
                    g11Params.bsPBcchBlks  = p_msg->params[0];
            }


            if (p_msg->params[1]< 12){    
                        g11Params.bsPagBlksRes = p_msg->params[1];
            }
                
 
            if (p_msg->params[2]<= 12){
                g11Params.bsPrachBlks  = p_msg->params[2];
            }
            
            if ((p_msg->params[3] >0) && (p_msg->params[4] <=16)){
                    g11Params.psi1_repeat  = p_msg->params[4];
            }
            
           g11Params.psi1_cycle   = 52 * p_msg->params[4];

           /* Setup for new assignment */
           
           g11Params.newAssignment    = TRUE;
        

        /* Send Acknowledgement */
      respMsg.msgSize = 4;
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte = OAM_G11_CONFIG_ACK >> 8;
      respMsg.typeLoByte = OAM_G11_CONFIG_ACK; 
      respMsg.trx = p_msg->trx;  
 
      SendMsgHost_Que(&respMsg);
        
     break;
            
    case OAM_BTS_BASIC_CONFIG:



      /* 
      *  Used for generic BTS paramters.
      */
      g_HoOAMInfo.serveParms.radioLinkTimeout = p_msg->params[0];
  
      /*
      *  The BSPaMfrms = BSPaMfrms(MIB) + 2
      */
      if( p_msg->params[1] < 8)
      { 
         /*
         *  check if new paging spread is set
         */
         if(g_BBInfo[rfCh].BSPaMfrms != p_msg->params[1]+2)
         {
            g_BBInfo[rfCh].BSPaMfrms = p_msg->params[1]+2;
            g_frmSync.newPageState = TRUE;
         }
      }
      else
      {
         ReportError(125, rfCh, 0, 1, & p_msg->params[1]);
      }
      
      /*
      *  Set correct UL and DL DTX modes for all slots (only necessary
      *  for active slots, but it doesn't hurt to set them all). Note:
      *  if DTX was previously on and is going off here, we rely on
      *  TCH processing functions to turn off ulDtxUsed.
      */
      ulDtxMode = (p_msg->params[2] & UL_DTX)==UL_DTX;
      dlDtxMode = (p_msg->params[2] & DL_DTX)==DL_DTX;
      /*--- Prohibit DL DTX on BCCH carrier ---*/     
      if(g_BBInfo[rfCh].TNInfo[0].groupNum == GSM_GROUP_5) 
      {                   
         dlDtxMode = FALSE;
      }      
      for( i=0; i<NUM_RFS_PER_DSP; i++ )
      {
         for ( timeSlot=0; timeSlot<NUM_TS_PER_RF; timeSlot++ )
         {   
            if ( g_BBInfo[i].TNInfo[timeSlot].groupNum == GSM_GROUP_1 )
            {
               g_BBInfo[i].TNInfo[timeSlot].u.group1.tch.ULDtxMode = ulDtxMode;
               g_BBInfo[i].TNInfo[timeSlot].u.group1.tch.DLDtxMode = dlDtxMode;
               g_BBInfo[i].TNInfo[timeSlot].u.group1.tch.ulDtxUsed = TRUE;
               g_BBInfo[i].TNInfo[timeSlot].u.group1.tch.dlDtxUsed = FALSE;
            }
         }
      }
      
      /* !! DEBUG */ 
      sendDiagMsg(0x99,rfCh,0,3, p_msg->params);

      /* Send Acknowledgement */
      respMsg.msgSize = 5;
      respMsg.function = OAM_MANAGE_MSG;
      respMsg.typeHiByte = OAM_BTS_BASIC_CONFIG_ACK >> 8;
      respMsg.typeLoByte = OAM_BTS_BASIC_CONFIG_ACK; 
      respMsg.trx = p_msg->trx;  
      respMsg.params[0] = g_HoOAMInfo.serveParms.radioLinkTimeout;
      SendMsgHost_Que(&respMsg);
      break;

  } 
  
  return(result);
}  




