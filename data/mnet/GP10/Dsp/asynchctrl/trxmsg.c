/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: trxmsg.c
*
* Description:
*   This file contains functions that process TRX messages from the host.
*
* Public Functions:
*   trxMsgProc, frequencyHoppingAlg
*
* Private Functions:
*   phConnectInd
*
******************************************************************************/
#include "string.h"
#include "math.h"        /* is this needed? */
#include "c6xsupport.h"
#include "bbdata.h"
#include "gsmdata.h"
#include "oamdata.h"
#include "dsp/dsphmsg.h"        
#include "l1config.h"
#include "rrmsg.h"
#include "regs.h"
#include "dsp6201.h"
#include "dsprotyp.h"
#include "agcdata.h"
#include "codec.h"
              
#include <common.h>
#include <mcbsp.h>      /* mcbsp devlib                                       */
#include <mcbspdrv.h>   /* mcbsp driver                                       */
#include <dma.h>
#include <intr.h>
#include <c6x.h>
    
extern Uint8 g_maxCpuLoading;

int      HSN_LookUpTbl[114] = 
{ 48,  98,  63,   1,  36,  95,  78, 102,  94,  73,    
   0,  64,  25,  81,  76,  59, 124,  23, 104, 100, 
 101,  47, 118,  85,  18,  56,  96,  86,  54,   2,    
  80,  34, 127,  13,   6,  89,  57, 103,  12,  74,    
  55, 111,  75,  38, 109,  71, 112,  29,  11,  88,      
  87,  19,   3,  68, 110,  26,  33,  31,   8,  45,      
  82,  58,  40, 107,  32,   5, 106,  92,  62,  67,      
  77, 108, 122,  37,  60,  66, 121,  42,  51, 126,  
 117, 114,   4,  90,  43,  52,  53, 113, 120,  72,      
  16,  49,   7,  79, 119,  61,  22,  84,   9,  97,      
  91,  15,  21,  24,  46,  39,  93, 105,  65,  70,      
 125,  99,  17, 123 };

Uint8    g_NetIntConfig;         /* Network Interface Configuration */
Uint16   DLsynthA;
Uint16   DLsynthB;
Uint8    FPGADelay; 

/******************************************************************************/
/* sendPhConnectInd - send PH_CONNECT_INDication messages to the host         */
/******************************************************************************/
void phConnectInd (Uint8 tsNum, t_groupNum groupNum)
{
  msgStruc phConnectIndBuf;
  Uint8 k;
  
  


  phConnectIndBuf.msgSize = 7;
  phConnectIndBuf.function = RR_MANAGE_MSG;
  phConnectIndBuf.typeHiByte = PH_CONNECT_IND_HI;
  phConnectIndBuf.typeLoByte = PH_CONNECT_IND_LO;
  phConnectIndBuf.trx = 0;  /* for now */ 

  
  switch (groupNum)
  {
    case GSM_GROUP_1:
      /* FACCH */
      phConnectIndBuf.params[0] = 0x08 | (tsNum & 0x07);   /* Chnnel No. */
      phConnectIndBuf.params[1] = 0;    /* Channel No. */
      phConnectIndBuf.params[2] = 0x00;  /* link identifier */
      SendMsgHost_Que(&phConnectIndBuf);
      /* SACCH */
      phConnectIndBuf.params[0] = 0x08 | (tsNum & 0x07);   /* Chnnel No. */
      phConnectIndBuf.params[1] = 0;    /* Channel No. */
      phConnectIndBuf.params[2] = 0x40;  /* link identifier */
      SendMsgHost_Que(&phConnectIndBuf);
      break;
      
    case GSM_GROUP_5:
      /* SDCCH */

      for (k = 0; k < 4; k++){
        phConnectIndBuf.params[0] = 0x20| (k <<3 )| tsNum;  /* Channel No. */
        phConnectIndBuf.params[1] = 0;  /* Channel No. */
        phConnectIndBuf.params[2] = 0;  /* link identifier */
        SendMsgHost_Que(&phConnectIndBuf);
      }
    
      
      /* SACCH */
      for (k = 0; k <4; k++){
         phConnectIndBuf.params[0] = 0x20 |(k<<3)| tsNum;  /* Channel No. */
         phConnectIndBuf.params[1] = 0;     /* Channel No. */
         phConnectIndBuf.params[2] = 0x40;  /* link identifier */
         SendMsgHost_Que(&phConnectIndBuf);
      }
    
      /* BCCH */
      phConnectIndBuf.params[0] = 0x80 | tsNum;  /* Channel No. */
      phConnectIndBuf.params[1] = 0;     /* Channel No. */
      phConnectIndBuf.params[2] = 0x20;  /* link identifier */
      SendMsgHost_Que(&phConnectIndBuf);

      /* RACH */
      phConnectIndBuf.params[0] = 0x88 | tsNum;  /* Channel No. */
      phConnectIndBuf.params[1] = 0;     /* Channel No. */
      phConnectIndBuf.params[2] = 0x20;  /* link identifier */
      SendMsgHost_Que(&phConnectIndBuf);

      /* PCH */
      phConnectIndBuf.params[0] = 0x90 | tsNum;  /* Channel No. */
      phConnectIndBuf.params[1] = 0;     /* Channel No. */
      phConnectIndBuf.params[2] = 0x20;  /* link identifier */
      SendMsgHost_Que(&phConnectIndBuf);

      /* AGCH */
      phConnectIndBuf.params[0] = 0x90 | tsNum;  /* Channel No. */
      phConnectIndBuf.params[1] = 0;     /* Channel No. */
      phConnectIndBuf.params[2] = 0x20;  /* link identifier */
      SendMsgHost_Que(&phConnectIndBuf);

      break;

      case GSM_GROUP_7:

	
    /* SDCCH */
	for ( k = 0 ; k < NUM_SDCCH8_SUBCHANS; k++){
         phConnectIndBuf.params[0] = 0x40| (k <<3) | tsNum;  /* Channel No. */
         phConnectIndBuf.params[1] = 0;  /* Channel No. */
         phConnectIndBuf.params[2] = 0;  /* link identifier */
         SendMsgHost_Que(&phConnectIndBuf);
	}
      
      /* SACCH */
	for (k = 0; k < NUM_SDCCH8_SUBCHANS; k++){
          phConnectIndBuf.params[0] = 0x40 | (k<<3) |tsNum;  /* Channel No. */
          phConnectIndBuf.params[1] = 0;     /* Channel No. */
          phConnectIndBuf.params[2] = 0x40;  /* link identifier */
          SendMsgHost_Que(&phConnectIndBuf);
        }



      break;
      
    default:
      break;  
  
  }  /* switch */    
}  /* phConnectInd */

                 
/*****************************************************************************
*
* Function: trxMsgProc
*
* Description:
*   Processes TRX Messeges received from the host
*
* Inputs:  
*   msgStruc          ptr to message structure
*
* Outputs:
*
******************************************************************************/
ProcResult  trxMsgProc (msgStruc *p_msg)
{
  ProcResult  result;
  Int32       nxtIndx;
  Uint16      msgType;
  msgStruc    respMsg;
  Uint8       rfCh;
  t_BBInfo    *bbInfo;
  Int32       i;
  Uint8       len, fcn, j, k;
  Uint8       ref;
  Uint16      arfcn;
  Uint16      deltaArfcn;
  Uint16      hiFactorTimes16;
  Uint16      *TxOffsetArfcns;
  Uint16      index, gainStepsPerDbTimes256;
  Uint8       tempBuf[20];
  t_groupNum  groupNum;
  
  Uint32  msgBuf[10];
  

   result = SUCCESS;       /* set default result */
   rfCh = p_msg->trx & 0x01; 
   bbInfo = & g_BBInfo[rfCh];
   msgType = (p_msg->typeHiByte << 8) | (p_msg->typeLoByte);
   switch (msgType)
   {
   case TRX_CONFIG:
    
      bbInfo->ais = p_msg->params[0];     /* Air Interface Standerd */
      bbInfo->bsic = p_msg->params[1];    /* Base Station ID */
      bbInfo->tseqNum = bbInfo->bsic & 7;       
      g_NetIntConfig = p_msg->params[2];  /* Network Interface Configuration */
      
      /*
      *  update BSIC to SCH channel coding buffer
      */      
      g_dlSigBufsSCH[rfCh][0].data[0][0] = g_BBInfo[rfCh].bsic<<2;
      g_dlSigBufsSCH[rfCh][0].data[1][0] = g_BBInfo[rfCh].bsic<<2;
      
      /*
      *   initialize ramping part of modualtion buffer according to Air standand
      *    via the IQSwap flag
      */
      bbInfo->IQSwap = p_msg->params[17] & 0x01;   /* flag value */

      /* 
      * Set measured values for agc target and radio band dependent characteristics.
      * Measured values are taken using a particular radio with a received signal
      * of g_MeasAgcDpcDbm.   The static gain value (g_UlSG) is then adjusted
      * to yield g_MeasAgcDpcDbr dBr.   This is also the way to get the 
      * amplifier's gain characteristic (slope).
      */
      g_MeasAgcDpcDbm = MEAS_AGCDPC_DBM;
      g_MeasAgcDpcDbr = RXPWR_TARGET;
      
      /*
      * Load the g_codecBsic variable based on the packed g_bsic variable
      */
      for(i=0; i<BSIC_LENGTH; i++)
      {
         g_codecBsic[BSIC_LENGTH-1-i] = (bbInfo->bsic >> i) & 0x0001;
      }
   
      /* NetIntMap [3] */

      /* HopSeqNUM [11] */
      bbInfo->HSN = p_msg->params[11];
      
      /* MobAllocIndxOffset [12] */   
      bbInfo->MAIO = p_msg->params[12];

      /* Set peripheral register for agc source to dsp control  */
      AGC_SRC_SELECT_REG |= (1 << DSP_FIXED_GAIN_CTRL_FLAG);

      /* Send Acknowledgement */
      respMsg.msgSize = 4;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  TRX_CONFIG_ACK >> 8;  
      respMsg.typeLoByte =  TRX_CONFIG_ACK;  
      respMsg.trx = 0;   /* for now */
      sendMsgHost(&respMsg);
      
      break;
      
   case SLOT_ACTIV:

      groupNum = (t_groupNum)(p_msg->params[1] & 0xF); 
      tsConfigure((Int16)p_msg->trx, (Int16)p_msg->params[0] & 0x7, groupNum);
      phConnectInd(p_msg->params[0] & 0x7, groupNum);
      
      /* If any TS on this carrier contains BCCH, then send dummy bursts on all 
      *  inactive TS. The Idle Frame Mode is initialized FALSE in dspinit() */
      if ( groupNum == GSM_GROUP_4 ||
           groupNum == GSM_GROUP_5 ||
           groupNum == GSM_GROUP_6 )
      {
         g_IdleFrameMode = TRUE;
      }
      respMsg.msgSize = 6;
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte = SLOT_ACTIV_ACK >> 8;
      respMsg.typeLoByte = SLOT_ACTIV_ACK; 
      respMsg.trx = p_msg->trx;
      respMsg.params[0] = p_msg->params[0];
      respMsg.params[1] = p_msg->params[1];
      
      g_frmSync.state = TS_RE_SYNC;
      sendMsgHost(&respMsg);
      break;
      
   case TRX_CONFIG_MA:
   
      len = ( p_msg->params[0] & 0x3F ) / 2; 
                        /* bits 6-1: byte Length of ARFCN msgs. */
      fcn = p_msg->params[0] & 0x80; 
                        /* bit  8: Function (add/del ARFCNs) */
      j=1;     // index into MA array, always start at MA[1], MA[0] is unused.
      k=0;     // index into message sent from L3 host as TRX_CONFIG_MA.
      // Since the Mobile Allocation[N] range is 1<=N<=64 we can
      // get away with a (brute force) linear search.  !!!! However,
      // as N approaches numbers greater than 512 (large), this
      // search will consume too much time.  For now, using this search
      // is faster and less complex than a non-linear search method.
      // The chance of N being larger than 64 is not great in the
      // product's near future.

      switch (fcn)
      {        
      case 0:        // Delete ARFCN's
            while (k != len){
                 if ( bbInfo->MA[j] == 
                        (p_msg->params[2*k+1] << 8) |
                        (p_msg->params[2*k+2]) ) {
                  bbInfo->MA[j] = -1; 
                  k+=1;
                  bbInfo->N = bbInfo->N - 1;   // N = num of allocated freqs.
                 }   
                 j+=1;     
            }
            break;

      case 1:        // Add ARFCN's
            while (k != len){
                 if (bbInfo->MA[j] == -1) {
                  bbInfo->MA[j] = (p_msg->params[2*k+1] << 8) |
                                    (p_msg->params[2*k+2]); 
                  k+=1;
                  bbInfo->N = bbInfo->N + 1;   // N = num of allocated freqs.
                 }   
                 j+=1;     
            }
            break;
      }
      bbInfo->NBIN = (int) LOG2(bbInfo->N) + 1;

      /* Send Acknowledgement */
      respMsg.msgSize = 4;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  TRX_CONFIG_MA_ACK >> 8;  
      respMsg.typeLoByte =  TRX_CONFIG_MA_ACK;  
      respMsg.trx = 0;   /* for now */
      sendMsgHost(&respMsg);

      break;
      
   case SLOT_REL:
      break;
      
   case SACCH_FILL:
      sacchMsgProc( &(((msgUnionStruc *)p_msg)->u.sacchInfo), 
                    &g_dlSigBufsSYSINFO[p_msg->trx & 1][p_msg->params[0] - SYS_INFO_5 + TYPE_5],
                    p_msg->params[0] );
      break;
      
   case ERROR_REPORT:
      break;
    
   case SET_UL_GAIN:
    
      /* Setup Hardware */ 
      g_USGC = ((p_msg->params[0] & 0x03) << 8) | p_msg->params[1];
      UL_DSP_AGC_CONTROL_REG = g_USGC;

      /* Send Acknowledgement for gain debug */
        sendDiagMsg(0x81, p_msg->trx, 0, 2, &p_msg->params[0]);

      break;

   case SET_DL_POWER:
    
      /* Setup Hardware (params[0] is don't care) */
      g_DSPC = p_msg->params[1];

      break;
    
   case SET_ARFCN:          
      /*
      *  Set current Tx power offset for ARFCN, rounding from table 
      */
      if ( g_BBInfo[rfCh].ais == DCS_1800 )
      {
         TxOffsetArfcns = g_TxOffsetArfcns1800; /* fixed ARFCNs for 1800 */
      }
      else
      {
         TxOffsetArfcns = g_TxOffsetArfcns1900; /* fixed ARFCNs for 1900 */
      }  
      k = 1;
      arfcn = ( p_msg->params[0] << 8 | p_msg->params[1]) & 0xfff;
      while ( k < 15  &&  arfcn > TxOffsetArfcns[k] )  
      {
         k++; 
      }
      deltaArfcn = TxOffsetArfcns[k] - TxOffsetArfcns[k-1]; 
      hiFactorTimes16 = ((arfcn - TxOffsetArfcns[k-1]) * 16) / deltaArfcn; 
      g_CurrentTxPwrOffsetForArfcn = (
            hiFactorTimes16 * g_TxPwrOffsetVsArfcnTbl[k] +
            (16 - hiFactorTimes16) * g_TxPwrOffsetVsArfcnTbl[k-1] + 8 ) >> 4;
      /* 
      * Convert Tx Max Power Reduction and ARFCN adjustment to DAC input
      * range 0-255 and set register value. Tables all in half dB steps.
      */
      index =  4 * g_TxStaticGainLevel + g_CurrentTxPwrOffsetForArfcn;
      if ( index >= NUM_TX_VS_PWR_LEVEL )
      {
         index = NUM_TX_VS_PWR_LEVEL - 1;
      }  
      g_DSPC = g_TxMaxPwrRdct2DLPwrTbl[ index ];      
      break;

   case TUNE_SYNTH:          
      /*
      * Enable FPGA to set ARFCN value. Only DSP A is allowed to set
      * synthesizer, since FPGA conflicts on FPGA driving PIC input pin
      */
      AGC_SRC_SELECT_REG |= (1<<DSP_FPGA_ARFCH_CTRL);
      
      /* Setup Hardware */
      switch (p_msg->trx & 0x1)
      {
      // DL_ARFCN_SELECT_REG bit format:
      //    bit      15: SYNTH A/B side (0:A; 1:B) 
      //    bit      14: TRX#  (0=TRX1, 1=TRX2)
      //    bit      13: Hop   (0=non hopped, 1=hopped)
      //    bits     12  POWER AMP Switch
      //    bits  11-10: RESERVED
      //    bits    9-0: dlARFCN value      
      case 0:
                       
        bbInfo->hopFlag = 0;
        
        DLsynthA = p_msg->params[0] << 8 | p_msg->params[1]; 

        if (!(DLsynthA & (1<<TRX_SELECT)))
        { 
            /* TRX#1 : hopping disabled always; tune DL Synth A */  
            DL_ARFCN_SELECT_REG_1 = DLsynthA;
            msgBuf[0] = 0x01060000; 
            msgBuf[1] = DL_ARFCN_SELECT_REG_1;
        }
        else if(DLsynthA & SYNTH_AB_SELECT)
        {  
            /* TRX#2 : hopping disabled always for NOW; tune DL Synth A */
            DL_ARFCN_SELECT_REG_2B = DLsynthA;  
            msgBuf[0] = 0x01050000; 
            msgBuf[1] = DL_ARFCN_SELECT_REG_2B;                                 
        }
        else
        {
            /* TRX#2 : hopping disabled always for NOW; tune DL Synth A */
            DL_ARFCN_SELECT_REG_2A = DLsynthA;
            msgBuf[0] = 0x01040000; 
            msgBuf[1] = DL_ARFCN_SELECT_REG_2A;                               
        }
        break;

      case 1:
        /* TRX#2 : hopping sequence must be generated; tune DL Synth B  */
        if ((bbInfo->MAIO) == 0 && bbInfo->N == 1) {
            // characterizes a non-hopping rf channel sequence; disable hopping
            bbInfo->hopFlag = 0;
            DLsynthB = 0x4000 | 
                ( ( (short int)p_msg->params[0] & 0x3 ) << 8 ) |
                   (short int)p_msg->params[1] ;   
            DL_ARFCN_SELECT_REG_2A = DLsynthB; 
        }      
        // NOTE!! The radio frequency sequence hopping algorithm 
        // is called from the interrupt service routine.  The
        // goal here is only to set the hopFlag to allow which
        // type of hopping will be used. 
        // No peripheral register (DSP to FGPA) shall be set here,
        // but instead by the ISR.  This register needs to be 
        // updated every TDMA frame (~4.615ms) at the start of
        // the frame, i.e. timeslot 0.
        break;
      }

   case CONFIG_FPGA_DELAY:
   
      FPGADelay = (short int)p_msg->params[0];
      break;
      
   case SYNCH:
      /*
      *  synchronize GSM frame number with Logical channel combinations
      */          
      INTR_GLOBAL_DISABLE();
      /*
      * initialize frame number counters
      */
      setFrameNum.FN = 0;
      sysFrameNum.FN = 0;
      sysFrameNum.t1 = 0;
      sysFrameNum.t2 = 0;
      sysFrameNum.t3 = 0;
      g_frmSync.state = SET_SYNC;            
      /*
      *  initialize HW related function blocks
      */             
      init_hw();
      break; 
            
   case KEEP_ALIVE_PING:

      /* Send Acknowledgement */
      respMsg.msgSize = 9;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  KEEP_ALIVE_PONG >> 8;  
      respMsg.typeLoByte =  KEEP_ALIVE_PONG;  
      respMsg.trx = p_msg->trx;   /* for now */
      respMsg.params[0] = sysFrameNum.FN>>24;
      respMsg.params[1] = sysFrameNum.FN>>16;
      respMsg.params[2] = sysFrameNum.FN>>8;
      respMsg.params[3] = sysFrameNum.FN;  	  
      respMsg.params[4] = g_maxCpuLoading;       /* appended CPU loading */
      sendMsgHost(&respMsg);

	  g_maxCpuLoading = 0;
      break;

   case REQ_CONFIG_RADIO_TX_PWR:

      /* send message to host to request tables for downlink TX power characteristic curve */
      respMsg.msgSize = 4;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  REQ_CONFIG_RADIO_TX_PWR >> 8;  
      respMsg.typeLoByte =  REQ_CONFIG_RADIO_TX_PWR;  
      respMsg.trx = 0;   
      sendMsgHost(&respMsg);
      break;

   case RADIO_TX_PWR_CHAR_CONFIG:

      /*-- First two values are absolute --*/       
      g_TxMaxPwrRdct2DLPwrTbl[0] = p_msg->params[0];
      g_TxMaxPwrRdct2DLPwrTbl[1] = p_msg->params[1];

      /*-- Remaining values are differentially encoded --*/       
      ref = g_TxMaxPwrRdct2DLPwrTbl[1];
      for(j=0; j<(NUM_TX_VS_PWR_LEVEL-2)/2; j++)
      {
         ref -= ((p_msg->params[j+2] & 0xF0) >> 4);
         g_TxMaxPwrRdct2DLPwrTbl[2*j+2] = ref;
          
         ref -= (p_msg->params[j+2] & 0x0F);
         g_TxMaxPwrRdct2DLPwrTbl[2*j+3] = ref; 
      }
      /* Send Acknowledgement */
      respMsg.msgSize = 4;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  RADIO_TX_PWR_CHAR_CONFIG_ACK >> 8;  
      respMsg.typeLoByte =  RADIO_TX_PWR_CHAR_CONFIG_ACK;  
      respMsg.trx = p_msg->trx;   /* for now */
      sendMsgHost(&respMsg);
      break;  

   case TX_PWR_OFFSETS_CONFIG:

      for(j=0; j<15; j++)
      {
         g_TxPwrOffsetVsArfcnTbl[j] = p_msg->params[j];
      }
      /* Send Acknowledgement */
      respMsg.msgSize = 4;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  TX_PWR_OFFSETS_CONFIG_ACK >> 8;  
      respMsg.typeLoByte =  TX_PWR_OFFSETS_CONFIG_ACK;  
      respMsg.trx = p_msg->trx;   /* for now */
      sendMsgHost(&respMsg);
      break;

   case TEMPERATURE_COMPENSATION:

      g_TxPwrTemperatureDelta = p_msg->params[0];
      break;

   case RADIO_GAIN_CHAR_CONFIG:
      /*
      * Use 1800 and 1900 parameters embedded in RF EEPROM instead of I-Q flag
      */
      gainStepsPerDbTimes256   = ((p_msg->params[0] << 8) | p_msg->params[1]); 
      g_MeasAgcDpcRxgain       = ((p_msg->params[2] << 8) | p_msg->params[3]); 
      g_rxGainMinIdle          = ((p_msg->params[4] << 8) | p_msg->params[5]);
  
      g_gainStepsPerDb.slope = gainStepsPerDbTimes256 >> 8;
      g_gainStepsPerDb.inverseSlope = ( 65536L+(gainStepsPerDbTimes256 >> 1) ) / gainStepsPerDbTimes256;

      /* Send Acknowledgement */
      respMsg.msgSize = 4;      
      respMsg.function = TRX_MANAGE_MSG;
      respMsg.typeHiByte =  RADIO_GAIN_CHAR_CONFIG_ACK >> 8;  
      respMsg.typeLoByte =  RADIO_GAIN_CHAR_CONFIG_ACK;  
      respMsg.trx = p_msg->trx;   /* for now */
      sendMsgHost(&respMsg);
      break;

   case  SET_RACH_SNR_THRESHOLD:
    
      g_RachSnrThresh = p_msg->params[0]; 
      break;
        
   default:
      break;
  
  } /* switch p_msg->type */
  
  return(result);
}  

/*****************************************************************************
*
* Function: frequencyHoppingAlg
*
* Description:
*   This function will implement the hopping sequence generation algorithm
*   defined by GSM 05.02 section 6.2.3.
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
Uint8 frequencyHoppingAlg (Uint8 *ndx, Uint8 *seqnum)
{
      int S, Mp, T, M, T1R, xorFCN, a;
      int lut, mai;
      t_BBInfo    *bbInfo; 
      
      if (*seqnum = 0){
            // algorithm chooses a cyclic hopping path.
            bbInfo->hopFlag = 1;
            mai = ((sysFrameNum.t1 *
                  sysFrameNum.t2 *
                  sysFrameNum.t3)  + *ndx) % (bbInfo->N);
      }

      else {
            // otherwise, the freq. hopping is random.
            bbInfo->hopFlag = 2;
            T1R = sysFrameNum.t1 % (bbInfo->N);

            xorFCN = (T1R | *seqnum) & ( ~(T1R & *seqnum));
            a = sysFrameNum.t3 + xorFCN;  
            lut = HSN_LookUpTbl[a];
            M = lut + sysFrameNum.t2;

            Mp = M % (int) (pow(2,bbInfo->NBIN));
      
            if (Mp < (bbInfo->N))
                  S = Mp;
            else {
                  T = sysFrameNum.t3 % (int) (pow(2,bbInfo->NBIN));
                  S = (Mp + T) % (bbInfo->N);
            }
      
            mai = (S + *ndx) % (bbInfo->N);  
      }
      // !!REMINDER, ISR needs to (for both cyclic and random):
      //    1.  Use mai to find next hop, bbInfo->MA[mai]
      //    2.  Set bits in peripheral register
      //          DLsynthB = 0xC000 | 
      //        ( ( (short int)p_msg->params[0] & 0x3 ) << 8 ) |
      //           (short int)p_msg->params[1] ;   
      //    3. Write to FPGA, 
      //          REG_WRITE(DL_ARFCN_SELECT_REG, DLsynthB); 
      
      return (mai);
}      



