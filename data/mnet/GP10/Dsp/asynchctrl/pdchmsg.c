#include "string.h"
#include "dsprotyp.h"
#include "gsmdata.h"
#include "agcdata.h"
#include "dsprotyp.h"
#include "intr.h"

static DataBlkSizes[] = {23, 34, 39, 53, 0, 0, 0, 0};

ProcResult pdchMsgProc(msgStruc *pdchMsg);
static inline pacchReqMsg(msgStruc pdchMsg, 
                          t_TNInfo* ts,  
                          t_DLSigBufs* p_pacchDlBuf)
{

}

/* static msg2DlSigBuf */
/*******************************************************************************
pdchMsgProc - PDCH Message Processing 

Process PDCH management messages received from the host.
*/
ProcResult pdchMsgProc
  (
    msgStruc *pdchMsg       /* IN: message */
  )
{
  dlTbfStruct* p_dlTbf;
  Uint16 msgType; 
  Uint8 trxNum;
  Uint8 psinfo;
  Uint8 tsNum;
  t_TNInfo* ts;
  int i;
  tbfStruct* p_tbfInfo;
  ulTbfStruct* p_ulTbf;
  Uint32 tlli;
  msgStruc ackMsg;

  msgType = (pdchMsg->typeHiByte << 8) | pdchMsg->typeLoByte;
  trxNum = pdchMsg->trx & 0x01;

/*
  if ((ts->groupNum) == GSM_GROUP_12)
    group = 
  else if ((ts->groupNum) == GSM_GROUP_13)
    group = 
  else 
    return ERROR;
*/

  switch (msgType)  
  {
     
  
  

    case PH_PTCCH_REQ:
    {                      /*
 ** rfch
 *
 *  FILENAME: Y:\GP10\Dsp\asynchctrl\pdchmsg.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

      /* index constants */
      const int iChanNum = 0;
      const int iData = 2;

      t_DLSigBufs* p_ptcchDlBuf;   /* PTCCH down link signalling buffer */


      tsNum = pdchMsg->params[iChanNum] & 0x07;
      ts = &g_BBInfo[trxNum].TNInfo[tsNum];

      INTR_DISABLE(CPU_INT8);
      
      /* find the right buffer A PTCCH SubChannel number is required */

      switch (ts->groupNum){
      case GSM_GROUP_11:
              p_ptcchDlBuf = ts->u.group11.ptcch.dlSigBuf;       
              break;
      case GSM_GROUP_12:
               p_ptcchDlBuf = ts->u.group12.ptcch.dlSigBuf;
              break;
      default:
               p_ptcchDlBuf = ts->u.group13.ptcch.dlSigBuf;
      break;
      }
      
    
      if (p_ptcchDlBuf->numMsgsReady == DL_SIG_Q_DEPTH)
      {
        ; /* UN_PROCESSED */
      }
      else
      { 
        memcpy (p_ptcchDlBuf->data[p_ptcchDlBuf->writeIndex], 
                &pdchMsg->params[iData], 23);

        if (++p_ptcchDlBuf->writeIndex == DL_SIG_Q_DEPTH) 
          p_ptcchDlBuf->writeIndex = 0;

        p_ptcchDlBuf->numMsgsReady++;
      } 

      INTR_ENABLE(CPU_INT8);
      
      break;
    }

    case PH_PACCH_REQ:
    {   
      /* index constants */
      const int iChanNum = 0;
      const int iTag = 2;
      const int iBurstType = 3;
      const int iData = 4;

      t_DLSigBufs* p_pacchDlBuf;   /* PACCH down link signalling buffer */


      tsNum = pdchMsg->params[iChanNum] & 0x07;
      ts = &g_BBInfo[trxNum].TNInfo[tsNum];

      INTR_DISABLE(CPU_INT8);

      /* find the right buffer */
      switch (ts->groupNum){
      case GSM_GROUP_11:
           p_pacchDlBuf = ts->u.group11.pacch.dlSigBuf;
              break;
      case GSM_GROUP_12:
           p_pacchDlBuf = ts->u.group12.pacch.dlSigBuf;
              break;
      default:
        p_pacchDlBuf = ts->u.group13.pacch.dlSigBuf;
        break;
      }
      
              
      
       

      if (p_pacchDlBuf->numMsgsReady == DL_SIG_Q_DEPTH)
      {
        ; /* UN_PROCESSED */
      }
      else
      { 
        p_pacchDlBuf->tag[p_pacchDlBuf->writeIndex]       = pdchMsg->params[iTag];
        p_pacchDlBuf->burstType[p_pacchDlBuf->writeIndex] = pdchMsg->params[iBurstType];
        memcpy (p_pacchDlBuf->data[p_pacchDlBuf->writeIndex], &pdchMsg->params[iData], 23);

        if (++p_pacchDlBuf->writeIndex == DL_SIG_Q_DEPTH) 
          p_pacchDlBuf->writeIndex = 0;

        p_pacchDlBuf->numMsgsReady++;
      } 
     
      INTR_ENABLE(CPU_INT8);

      break;
    }
  
    case PH_PDTCH_REQ:
    {
      /* index constants */
      const int iChanNum = 0;
      const int iTag = 2;
      const int iBurstType = 3;
      const int iCodecRate = 4;
      const int iData = 5;           

      t_dlPdtchBufs* p_pdtchDlBuf;
      Uint16 dataBlkSize;
      Uint8* rdPtr;
      Uint8* wrPtr;

      tsNum = pdchMsg->params[iChanNum] & 0x07;
      ts = &g_BBInfo[trxNum].TNInfo[tsNum];

      INTR_DISABLE(CPU_INT8);

      /* find the right buffer and data block size */

      switch (ts->groupNum){
      case GSM_GROUP_11:
        p_pdtchDlBuf = ts->u.group11.pdtch.dlSigBuf;
        dataBlkSize = DataBlkSizes[pdchMsg->params[iCodecRate]];
        break;
      case GSM_GROUP_12:
        p_pdtchDlBuf = ts->u.group12.pdtch.dlSigBuf;
        dataBlkSize = DataBlkSizes[pdchMsg->params[iCodecRate]];
        break;
      default:
        p_pdtchDlBuf = ts->u.group13.pdtch.dlSigBuf;
        dataBlkSize = DataBlkSizes[pdchMsg->params[iCodecRate]];
        break;
      }
      

      /* these were added to support access burst acknowledgements */
      p_pdtchDlBuf->tag[p_pdtchDlBuf->writeIndex]       = pdchMsg->params[iTag];
      p_pdtchDlBuf->burstType[p_pdtchDlBuf->writeIndex] = pdchMsg->params[iBurstType];
      p_pdtchDlBuf->chCodec[p_pdtchDlBuf->writeIndex]   = pdchMsg->params[iCodecRate];

      if (p_pdtchDlBuf->frameCount < NUM_TCH_JBUF)
      {
        /* fill buffer */
        rdPtr = &pdchMsg->params[iData];
        wrPtr = (Uint8 *) p_pdtchDlBuf->buffer[p_pdtchDlBuf->writeIndex];
        for (i = 0; i <dataBlkSize; i++)
          *wrPtr++ = *rdPtr++;        

        /* update write index */
        if (++p_pdtchDlBuf->writeIndex == NUM_TCH_JBUF)
           p_pdtchDlBuf->writeIndex = 0;
        
        /* update count */
        p_pdtchDlBuf->frameCount++;
      }

      INTR_ENABLE(CPU_INT8);

      break;
    }

    case PDCH_UL_TBF_ACT_REQ:
    {
      /* index constants */
      const int iTlli = 0;
      const int iTbfMode = 4; 
      const int iPdchGrpIndxTfi = 5;
      const int iChannelCodec = 6;
      const int iTsAlloc = 7;
      const int iUsfTsMap = 8;
      const int iTai = 12; 

      tlli = (pdchMsg->params[iTlli + 0] << 24) |
             (pdchMsg->params[iTlli + 1] << 16) |
             (pdchMsg->params[iTlli + 2] << 8) |
              pdchMsg->params[iTlli + 3];
      /*
      * Look for TBF with matching TLLI
      */
      p_tbfInfo = NULL;
      for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
      {
          if ( g_tbfInfo[trxNum][i].TLLI == tlli )
             p_tbfInfo = &g_tbfInfo[trxNum][i];
      }
      /* 
      * If no TLLI match found, look for any available TBF
      */
      if ( p_tbfInfo == NULL )
      {
          for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
          {
              if ( g_tbfInfo[trxNum][i].dlTbf.state != CHAN_ACTIVE &&
                   g_tbfInfo[trxNum][i].ulTbf.state != CHAN_ACTIVE  )
              {
                 p_tbfInfo = &g_tbfInfo[trxNum][i];
              }        
          }
      }
      /*
      * If TLLI was found or new TBF assigned, set state parameters and ACK
      */
      if (p_tbfInfo)
      {  
        p_tbfInfo->TLLI = tlli;
        p_ulTbf = &p_tbfInfo->ulTbf;
        switch (pdchMsg->params[iTbfMode]) 
        {
          case 0:
            p_ulTbf->tbfMode = TBF_STATIC;
            break;
          case 1:
            p_ulTbf->tbfMode = TBF_DYNAMIC;
            break;
          case 2:
            p_ulTbf->tbfMode = TBF_DYNAMIC_EXTENDED;
            break;
        }
        p_ulTbf->tfi = pdchMsg->params[iPdchGrpIndxTfi] & 0x1F;
        p_ulTbf->channelCodec = pdchMsg->params[iChannelCodec];
        p_ulTbf->timeSlotAllocation = pdchMsg->params[iTsAlloc];
        if ((pdchMsg->params[iTbfMode] == 0) || 
            (pdchMsg->params[iTbfMode] == 1))
        {
            p_ulTbf->opMode.dynamic.usfTsMap = (pdchMsg->params[iUsfTsMap + 0] << 24) |
                                               (pdchMsg->params[iUsfTsMap + 1] << 16) |
                                               (pdchMsg->params[iUsfTsMap + 2] << 8)  |
                                               (pdchMsg->params[iUsfTsMap + 3]);
        }
        p_ulTbf->taiTimeSlot = pdchMsg->params[iTai + 0] & 0x07;
        p_ulTbf->tai = pdchMsg->params[iTai + 1] & 0x0F;
        p_ulTbf->state = CHAN_ACTIVE;

        /*
        *  Initialize for Coding Rate Change functionality.
        */
        p_ulTbf->sigqual.berAve = 0;
        p_ulTbf->sigqual.blerAve = 0;

        /* Send Acknowledgement */
        ackMsg.msgSize = 20;
        ackMsg.function = PDCH_MANAGE_MSG;
        ackMsg.typeHiByte = PDCH_UL_TBF_ACT_ACK >> 8;
        ackMsg.typeLoByte = PDCH_UL_TBF_ACT_ACK;
        ackMsg.params[0] = (p_tbfInfo->TLLI >> 24) & 0xff;
        ackMsg.params[1] = (p_tbfInfo->TLLI >> 16) & 0xff;
        ackMsg.params[2] = (p_tbfInfo->TLLI >>  8) & 0xff;
        ackMsg.params[3] = (p_tbfInfo->TLLI      ) & 0xff;
        ackMsg.params[4] = ((Uint16)p_tbfInfo >> 8 ) & 0xff;
        ackMsg.params[5] = ((Uint16)p_tbfInfo      ) & 0xff;
        ackMsg.params[6] = p_ulTbf->tbfMode;
        ackMsg.params[7] = p_ulTbf->tfi;
        ackMsg.params[8] = p_ulTbf->channelCodec;
        ackMsg.params[9] = p_ulTbf->timeSlotAllocation;
        ackMsg.params[10] = (p_ulTbf->opMode.dynamic.usfTsMap >> 24) & 0xff;
        ackMsg.params[11] = (p_ulTbf->opMode.dynamic.usfTsMap >> 16) & 0xff;
        ackMsg.params[12] = (p_ulTbf->opMode.dynamic.usfTsMap >>  8) & 0xff;
        ackMsg.params[13] = (p_ulTbf->opMode.dynamic.usfTsMap      ) & 0xff;
        ackMsg.params[14] = p_ulTbf->taiTimeSlot;
        ackMsg.params[15] = p_ulTbf->tai;
        SendMsgHost_Que(&ackMsg);

        /*---------------------------------------------
        * Initialize diagnostic GPRS MS Report parameters
        *--------------------------------------------*/
        initGprsMsReport(trxNum, p_ulTbf->tfi);

  /*    for(i=0; i<8; i++)
      {
         if( (p_ulTbf->timeSlotAllocation >> i ) & 0x01)
         {
            
            if(g_BBInfo[trxNum].TNInfo[i].groupNum == GSM_GROUP_13)
            {
            rtsCheck(trxNum, i, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group13.pdtch.dlSigBuf->frameCount));
            rtsCheck(trxNum, i, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group13.pacch.dlSigBuf->numMsgsReady));
            rtsCheck(trxNum, i, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group13.ptcch.dlSigBuf->numMsgsReady));

            }
            else if (g_BBInfo[trxNum].TNInfo[i].groupNum == GSM_GROUP_11)
            {

            rtsCheck(trxNum, i, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group11.pdtch.dlSigBuf->frameCount));
            rtsCheck(trxNum, i, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group11.pacch.dlSigBuf->numMsgsReady));
            rtsCheck(trxNum, i, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group11.ptcch.dlSigBuf->numMsgsReady));

            }
            else if (g_BBInfo[trxNum].TNInfo[i].groupNum == GSM_GROUP_12)
            {

            rtsCheck(trxNum, i, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group12.pdtch.dlSigBuf->frameCount));
            rtsCheck(trxNum, i, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group12.pacch.dlSigBuf->numMsgsReady));
            rtsCheck(trxNum, i, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group12.ptcch.dlSigBuf->numMsgsReady));

            }
         }
      }
*/

      }
      break;
    }

    case PDCH_UL_TBF_DEACT_REQ:
    {
      /* index constants */
      const iTlli = 0;

      tlli = (pdchMsg->params[iTlli + 0] << 24) |
             (pdchMsg->params[iTlli + 1] << 16) |
             (pdchMsg->params[iTlli + 2] << 8) |
              pdchMsg->params[iTlli + 3];
      /*
      * Look for TBF with matching TLLI
      */
      p_tbfInfo = NULL;
      for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
      {
          if ( g_tbfInfo[trxNum][i].TLLI == tlli )
             p_tbfInfo = &g_tbfInfo[trxNum][i];
      }
      /*
      * If TLLI was found, set state parameters and ACK
      */
      if (p_tbfInfo)
      {  
        /* Set UL state inactive */   
        p_ulTbf = &p_tbfInfo->ulTbf;
        p_ulTbf->state = CHAN_INACTIVE;
        /* If DL is also inactive, zero out TLLI for cleaner diags */
        p_dlTbf = &p_tbfInfo->dlTbf;
        if ( p_dlTbf->state == CHAN_INACTIVE )
        {
           p_tbfInfo->TLLI = 0;
        }
        ackMsg.msgSize = 8;
        ackMsg.function = PDCH_MANAGE_MSG;
        ackMsg.typeHiByte = PDCH_UL_TBF_DEACT_ACK >> 8;
        ackMsg.typeLoByte = PDCH_UL_TBF_DEACT_ACK;
        ackMsg.params[0] = pdchMsg->params[0];
        ackMsg.params[1] = pdchMsg->params[1];
        ackMsg.params[2] = pdchMsg->params[2];
        ackMsg.params[3] = pdchMsg->params[3];
        SendMsgHost_Que(&ackMsg);
      }       
      break;
    }

    case PDCH_DL_TBF_ACT_REQ:
    {
      /* index constants */
      const int iTlli = 0;
      const int iPdchGrpTfi = 4;
      const int iChannelCodec = 5;
      const int iTsAlloc = 6;
      const int iMsPower = 7;
      const int iTimingAdvance = 8;
      
      tlli = (pdchMsg->params[iTlli + 0] << 24) |
             (pdchMsg->params[iTlli + 1] << 16) |
             (pdchMsg->params[iTlli + 2] << 8) |
              pdchMsg->params[iTlli + 3];
      /*
      * Look for TBF with matching TLLI
      */
      p_tbfInfo = NULL;
      for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
      {
          if ( g_tbfInfo[trxNum][i].TLLI == tlli )
             p_tbfInfo = &g_tbfInfo[trxNum][i];
      }
      /*
      * If no TLLI match found, look for any available TBF
      */
      if ( p_tbfInfo == NULL )
      {
          for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
          {
              if ( g_tbfInfo[trxNum][i].dlTbf.state != CHAN_ACTIVE &&
                   g_tbfInfo[trxNum][i].ulTbf.state != CHAN_ACTIVE  )
              {
                 /* Found available TBF. Initialize UL gain. */
                 p_tbfInfo = &g_tbfInfo[trxNum][i];
                 p_tbfInfo->ulTbf.sigstren.rxgain = g_USGC;        
              }        
          }
      }
      /*
      * If TLLI was found or new TBF assigned, set state parameters and ACK
      */
      if (p_tbfInfo)
      {   
        p_tbfInfo->TLLI = tlli;
        p_dlTbf = &p_tbfInfo->dlTbf;
        p_ulTbf = &p_tbfInfo->ulTbf;
        p_dlTbf->tfi = pdchMsg->params[iPdchGrpTfi] & 0x1F;
        p_dlTbf->channelCodec = pdchMsg->params[iChannelCodec];
        p_dlTbf->timeSlotAllocation = pdchMsg->params[iTsAlloc];
        p_ulTbf->timingAdvance = pdchMsg->params[iTimingAdvance];
        p_dlTbf->state = CHAN_ACTIVE;

        /* Send Acknowledgement */
        ackMsg.msgSize = 14;
        ackMsg.function = PDCH_MANAGE_MSG;
        ackMsg.typeHiByte = PDCH_DL_TBF_ACT_ACK >> 8;
        ackMsg.typeLoByte = PDCH_DL_TBF_ACT_ACK;
        ackMsg.params[0] = (p_tbfInfo->TLLI >> 24) & 0xff;
        ackMsg.params[1] = (p_tbfInfo->TLLI >> 16) & 0xff;
        ackMsg.params[2] = (p_tbfInfo->TLLI >>  8) & 0xff;
        ackMsg.params[3] = (p_tbfInfo->TLLI      ) & 0xff;
        ackMsg.params[4] = ((Uint16)p_tbfInfo >> 8 ) & 0xff;
        ackMsg.params[5] = ((Uint16)p_tbfInfo      ) & 0xff;
        ackMsg.params[6] = p_dlTbf->tfi;
        ackMsg.params[7] = p_dlTbf->channelCodec;
        ackMsg.params[8] = p_dlTbf->timeSlotAllocation;
        ackMsg.params[9] = p_ulTbf->timingAdvance;
        SendMsgHost_Que(&ackMsg);

        /*---------------------------------------------
        * Initialize diagnostic GPRS MS Report parameters
        *--------------------------------------------*/
        initGprsMsReport(trxNum, p_dlTbf->tfi);
       /*---------------------------------------------
        * Initialize parameters for DL code rate monitoring
        *--------------------------------------------*/
        p_dlTbf->pktDlAckNack.ber = 0;
        p_dlTbf->pktDlAckNack.berAve = 0;
        p_dlTbf->pktDlAckNack.C_VALUE = 0;
        p_dlTbf->pktDlAckNack.RXQUAL  = 0;

        for(i=0; i<NUM_TS_PER_RF; i++)      
          p_dlTbf->pktDlAckNack.iLevelTN[i] = 0xFF; /* values not valid */
       }

 /*     for(i=0; i<8; i++)
      {
         if( (p_dlTbf->timeSlotAllocation >> i ) & 0x01)
         {
            
            if(g_BBInfo[trxNum].TNInfo[i].groupNum == GSM_GROUP_13)
            {
            rtsCheck(trxNum, i, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group13.pdtch.dlSigBuf->frameCount));
            rtsCheck(trxNum, i, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group13.pacch.dlSigBuf->numMsgsReady));
            rtsCheck(trxNum, i, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group13.pacch.dlSigBuf->numMsgsReady));

            }
            else if (g_BBInfo[trxNum].TNInfo[i].groupNum == GSM_GROUP_11)
            {

            rtsCheck(trxNum, i, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group11.pdtch.dlSigBuf->frameCount));
            rtsCheck(trxNum, i, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group11.pacch.dlSigBuf->numMsgsReady));
            rtsCheck(trxNum, i, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group11.pacch.dlSigBuf->numMsgsReady));

            }
            else if (g_BBInfo[trxNum].TNInfo[i].groupNum == GSM_GROUP_12)
            {

            rtsCheck(trxNum, i, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group12.pdtch.dlSigBuf->frameCount));
            rtsCheck(trxNum, i, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group12.pacch.dlSigBuf->numMsgsReady));
            rtsCheck(trxNum, i, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group12.pacch.dlSigBuf->numMsgsReady));

            }
         }
      }

  */
      break;
    }
    
    case PDCH_DL_TBF_DEACT_REQ:
    {
      /* index constants */
      const iTlli = 0;

      tlli = (pdchMsg->params[iTlli + 0] << 24) |
             (pdchMsg->params[iTlli + 1] << 16) |
             (pdchMsg->params[iTlli + 2] << 8) |
              pdchMsg->params[iTlli + 3];
      /*
      * Look for TBF with matching TLLI
      */
      p_tbfInfo = NULL;
      for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
      {
          if ( g_tbfInfo[trxNum][i].TLLI == tlli )
             p_tbfInfo = &g_tbfInfo[trxNum][i];
      }
      /*
      * If TLLI was found, set state parameters and ACK
      */
      if (p_tbfInfo)
      {
        /* Set DL state inactive */   
        p_dlTbf = &p_tbfInfo->dlTbf;
        p_dlTbf->state = CHAN_INACTIVE;
        /* If UL is also inactive, zero out TLLI for cleaner diags */
        p_ulTbf = &p_tbfInfo->ulTbf;
        if ( p_ulTbf->state == CHAN_INACTIVE )
        {
           p_tbfInfo->TLLI = 0;
        }
        ackMsg.msgSize = 8;
        ackMsg.function = PDCH_MANAGE_MSG;
        ackMsg.typeHiByte = PDCH_DL_TBF_DEACT_ACK >> 8;
        ackMsg.typeLoByte = PDCH_DL_TBF_DEACT_ACK;
        ackMsg.params[0] = pdchMsg->params[0];
        ackMsg.params[1] = pdchMsg->params[1];
        ackMsg.params[2] = pdchMsg->params[2];
        ackMsg.params[3] = pdchMsg->params[3];
        SendMsgHost_Que(&ackMsg);
      }       
      break;
    }

    case PDCH_UL_TBF_RECONFIG_REQ:
    {
      /* index constants */
      const int iTlli = 0;
      const int iTbfMode = 6; 
      const int iPdchGrpIndxTfi = 4;
      const int iChannelCodec = 5;
      const int iTsAlloc = 7;
      const int iUsfTsMap = 8;
      const int iTai = 12;
     
      tlli = (pdchMsg->params[iTlli + 0] << 24) |
             (pdchMsg->params[iTlli + 1] << 16) |
             (pdchMsg->params[iTlli + 2] << 8) |
              pdchMsg->params[iTlli + 3];
      /*
      * Look for TBF with matching TLLI
      */
      p_tbfInfo = NULL;
      for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
      {
          if ( g_tbfInfo[trxNum][i].TLLI == tlli )
             p_tbfInfo = &g_tbfInfo[trxNum][i];
      }
      /*
      * If TLLI was found, set state parameters and ACK
      */
      if (p_tbfInfo)
      {   
        p_tbfInfo->TLLI = tlli;
        p_ulTbf = &p_tbfInfo->ulTbf;
        switch (pdchMsg->params[iTbfMode]) 
        {
          case 0:
            p_ulTbf->tbfMode = TBF_STATIC;
            break;
          case 1:
            p_ulTbf->tbfMode = TBF_DYNAMIC;
            break;
          case 2:
            p_ulTbf->tbfMode = TBF_DYNAMIC_EXTENDED;
            break;
        }
        p_ulTbf->tfi = pdchMsg->params[iPdchGrpIndxTfi] & 0x1F;
        p_ulTbf->channelCodec = pdchMsg->params[iChannelCodec];
        p_ulTbf->timeSlotAllocation = pdchMsg->params[iTsAlloc];
        if ((pdchMsg->params[iTbfMode] == 0) || 
            (pdchMsg->params[iTbfMode] == 1))
        {
          p_ulTbf->opMode.dynamic.usfTsMap = (pdchMsg->params[iUsfTsMap + 0] << 24) |
                                             (pdchMsg->params[iUsfTsMap + 1] << 16) |
                                             (pdchMsg->params[iUsfTsMap + 2] << 8)  |
                                             (pdchMsg->params[iUsfTsMap + 3]);
        }       
        p_ulTbf->taiTimeSlot = pdchMsg->params[iTai + 0] & 0x07;
        p_ulTbf->tai = pdchMsg->params[iTai + 1] & 0x0F;

        /* Send Acknowledgement */
        ackMsg.msgSize = 20;
        ackMsg.function = PDCH_MANAGE_MSG;
        ackMsg.typeHiByte = PDCH_UL_TBF_RECONFIG_ACK >> 8;
        ackMsg.typeLoByte = PDCH_UL_TBF_RECONFIG_ACK;
        ackMsg.params[0] = (p_tbfInfo->TLLI >> 24) & 0xff;
        ackMsg.params[1] = (p_tbfInfo->TLLI >> 16) & 0xff;
        ackMsg.params[2] = (p_tbfInfo->TLLI >>  8) & 0xff;
        ackMsg.params[3] = (p_tbfInfo->TLLI      ) & 0xff;
        ackMsg.params[4] = ((Uint16)p_tbfInfo >> 8 ) & 0xff;
        ackMsg.params[5] = ((Uint16)p_tbfInfo      ) & 0xff;
        ackMsg.params[6] = p_ulTbf->tbfMode;
        ackMsg.params[7] = p_ulTbf->tfi;
        ackMsg.params[8] = p_ulTbf->channelCodec;
        ackMsg.params[9] = p_ulTbf->timeSlotAllocation;
        ackMsg.params[10] = (p_ulTbf->opMode.dynamic.usfTsMap >> 24) & 0xff;
        ackMsg.params[11] = (p_ulTbf->opMode.dynamic.usfTsMap >> 16) & 0xff;
        ackMsg.params[12] = (p_ulTbf->opMode.dynamic.usfTsMap >>  8) & 0xff;
        ackMsg.params[13] = (p_ulTbf->opMode.dynamic.usfTsMap      ) & 0xff;
        ackMsg.params[14] = p_ulTbf->taiTimeSlot;
        ackMsg.params[15] = p_ulTbf->tai;
        SendMsgHost_Que(&ackMsg);

        /*---------------------------------------------
        * Initialize diagnostic GPRS MS Report parameters
        *--------------------------------------------*/
        initGprsMsReport(trxNum, p_ulTbf->tfi);

      }
      break;
   }

   case PDCH_DL_TBF_RECONFIG_REQ:
   {
      /* index constants */
      const int iTlli = 0;
      const int iPdchGrpTfi = 4;
      const int iChannelCodec = 5;
      const int iTsAlloc = 6;
      const int iTimingAdvance = 8;
      
      tlli = (pdchMsg->params[iTlli + 0] << 24) |
             (pdchMsg->params[iTlli + 1] << 16) |
             (pdchMsg->params[iTlli + 2] << 8) |
              pdchMsg->params[iTlli + 3];

      /* Look for TBF with matching TLLI */
      p_tbfInfo = NULL;
      for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
      {
          if ( g_tbfInfo[trxNum][i].TLLI == tlli )
             p_tbfInfo = &g_tbfInfo[trxNum][i];
      }
      /*
      * If TLLI was found, set state parameters and ACK
      */
      if (p_tbfInfo)
      {   
        p_tbfInfo->TLLI = tlli;
        p_dlTbf = &p_tbfInfo->dlTbf;
        p_ulTbf = &p_tbfInfo->ulTbf;
        p_dlTbf->tfi = pdchMsg->params[iPdchGrpTfi] & 0x1F;
        p_dlTbf->channelCodec = pdchMsg->params[iChannelCodec];
        p_dlTbf->timeSlotAllocation = pdchMsg->params[iTsAlloc];
        p_ulTbf->timingAdvance = pdchMsg->params[iTimingAdvance];

        /* Send Acknowledgement */
        ackMsg.msgSize = 14;
        ackMsg.function = PDCH_MANAGE_MSG;
        ackMsg.typeHiByte = PDCH_DL_TBF_RECONFIG_ACK >> 8;
        ackMsg.typeLoByte = PDCH_DL_TBF_RECONFIG_ACK;
        ackMsg.params[0] = (p_tbfInfo->TLLI >> 24) & 0xff;
        ackMsg.params[1] = (p_tbfInfo->TLLI >> 16) & 0xff;
        ackMsg.params[2] = (p_tbfInfo->TLLI >>  8) & 0xff;
        ackMsg.params[3] = (p_tbfInfo->TLLI      ) & 0xff;
        ackMsg.params[4] = ((Uint16)p_tbfInfo >> 8 ) & 0xff;
        ackMsg.params[5] = ((Uint16)p_tbfInfo      ) & 0xff;
        ackMsg.params[6] = p_dlTbf->tfi;
        ackMsg.params[7] = p_dlTbf->channelCodec;
        ackMsg.params[8] = p_dlTbf->timeSlotAllocation;
        ackMsg.params[9] = p_ulTbf->timingAdvance;
        SendMsgHost_Que(&ackMsg);

        /*---------------------------------------------
        * Initialize diagnostic GPRS MS Report parameters
        *--------------------------------------------*/
        initGprsMsReport(trxNum, p_dlTbf->tfi);

        /*---------------------------------------------
        * Initialize parameters for DL code rate monitoring
        *--------------------------------------------*/
        p_dlTbf->pktDlAckNack.ber = 0;
        p_dlTbf->pktDlAckNack.berAve = 0;
        p_dlTbf->pktDlAckNack.C_VALUE = 0;
        p_dlTbf->pktDlAckNack.RXQUAL  = 0;

        for(i=0; i<NUM_TS_PER_RF; i++)      
          p_dlTbf->pktDlAckNack.iLevelTN[i] = 0xFF; /* values not valid */

      }
      break;
   }

   case PDCH_SINGLE_BLOCK_ASSIGN_ALERT_ACK:
   {   
      /* Message from L1Proxy telling us that the other DSP detected 
      *  the SBA/Immediate Assign; each SBA entry in a watch table is set.
      */
      /* index constants */
      const int iChanNum = 0;
      const int iStartTime = 2;
      Uint32 FN1;
      Uint8  blockNum, tempbuf[6];

      tsNum = pdchMsg->params[iChanNum] & 0x07;
      ts = &g_BBInfo[trxNum].TNInfo[tsNum];

      /* extract starting frame of UL block */
      FN1 = (pdchMsg->params[iStartTime]   << 24) | 
            (pdchMsg->params[iStartTime+1] << 16) |
            (pdchMsg->params[iStartTime+2] << 8)  |
             pdchMsg->params[iStartTime+3];
      /*
      *  Convert absolute frame number to block number (0..11)
      */
      blockNum = ((FN1%GPRS_MULTI_FRAMES)/(GPRS_MULTI_FRAMES/4)) * 3 +
                  (FN1%(GPRS_MULTI_FRAMES/4)) / 4;
      if ( blockNum >= BLOCKS_PER_GPRS_MULTI_FRAMES ) blockNum = 0;

      if( ts->u.group13.blkNumWatch[blockNum].tagFillCause != NONE) {
        /*
        *  Diagnostic for debugging conflicts
        */
        tempbuf[0] = ts->u.group13.blkNumWatch[blockNum].tagFillCause;
        tempbuf[1] = ts->u.group13.blkNumWatch[blockNum].tag; 
        tempbuf[2] = blockNum; 
        tempbuf[3] = (Uint8) ((Uint16)ts->u.group13.blkNumWatch[blockNum].tbf >> 8);
        tempbuf[4] = (Uint8) ((Uint16)ts->u.group13.blkNumWatch[blockNum].tbf & 0xff);
        sendDiagMsg(0x52,0,ts->tsNumber,5,tempbuf);   
      }    

      /*
      *  Add entry to watch table !! Overwrite for now, if a conflict exists
      */
      ts->u.group13.blkNumWatch[blockNum].tagFillCause = SBA;
      ts->u.group13.blkNumWatch[blockNum].tag = SBA_TAG;
      ts->u.group13.blkNumWatch[blockNum].tbf = NULL;

      /*
      *  Diagnostic for debugging conflicts
      */
      tempbuf[0] = ts->u.group13.blkNumWatch[blockNum].tagFillCause;
      tempbuf[1] = ts->u.group13.blkNumWatch[blockNum].tag; 
      tempbuf[2] = blockNum; 
      tempbuf[3] = (Uint8) ((Uint16)ts->u.group13.blkNumWatch[blockNum].tbf >> 8);
      tempbuf[4] = (Uint8) ((Uint16)ts->u.group13.blkNumWatch[blockNum].tbf & 0xff);
      sendDiagMsg(0x53,0,ts->tsNumber,5,tempbuf); 
 
      break;
   }

     case PDCH_DL_ACK_NACK_INFO:
   {
     /* Message from L1Proxy via RLC/MAC giving us inputs to the rate selection algorithm
     *  taken from the PACKET DOWNLINK ACK NACK RLC uplink control message.
     */

     Uint8 timeSlot, tempbuf[15];
     Uint32 tlli;

     /* index constants */
     const int iTLLI = 0;
     const int iTfi  = 4;
     const int iCVal = 5;
     const int iRxQual = 6;
     const int iILevelTNAvailMask = 7;

     tlli = ((pdchMsg->params[iTLLI+0] << 24) | 
             (pdchMsg->params[iTLLI+1] << 16) |
             (pdchMsg->params[iTLLI+2] << 8) | 
              pdchMsg->params[iTLLI+3] );

     /*
     * Look for TBF with matching TLLI
     */
     p_tbfInfo = NULL;
     for ( i=0; (i<TBF_NUMBER_MAX) && p_tbfInfo==NULL; i++ )
     {
       if ( g_tbfInfo[trxNum][i].TLLI == tlli )
         p_tbfInfo = &g_tbfInfo[trxNum][i];
     }
      
     /*
     * If no TLLI match found, look for any available TBF
     */
     if ( p_tbfInfo == NULL )
     {
        /* Error */ 
     }
     
     /*
     * If TLLI was found or new TBF assigned, set state parameters and ACK
     */
     if (p_tbfInfo)
     {   
       p_dlTbf = &p_tbfInfo->dlTbf;     

       if (p_dlTbf->tfi == (pdchMsg->params[iTfi] & 0x1F))
       {
         p_dlTbf->pktDlAckNack.C_VALUE = pdchMsg->params[iCVal];
         p_dlTbf->pktDlAckNack.RXQUAL  = pdchMsg->params[iRxQual];

         for(timeSlot=0; timeSlot<NUM_TS_PER_RF; timeSlot++)      
         {
           if (((pdchMsg->params[iILevelTNAvailMask] >> timeSlot) & 0x01) == 1)
             p_dlTbf->pktDlAckNack.iLevelTN[timeSlot] = pdchMsg->params[iILevelTNAvailMask+1+timeSlot];
           else
             p_dlTbf->pktDlAckNack.iLevelTN[timeSlot] = (p_dlTbf->pktDlAckNack.C_VALUE)/2;   /* non-existent */
         }

         p_dlTbf->pktDlAckNack.ber = g_rxQual2ber_LookUpTbl[p_dlTbf->pktDlAckNack.RXQUAL];
         
         /*
         *  Diagnostic
         */
         tempbuf[0] = tlli >> 24;
         tempbuf[1] = tlli >> 16;
         tempbuf[2] = tlli >> 8; 
         tempbuf[3] = tlli;
         tempbuf[4] = p_dlTbf->tfi;
         tempbuf[5] = p_dlTbf->pktDlAckNack.C_VALUE;
         tempbuf[6] = p_dlTbf->pktDlAckNack.RXQUAL;
         tempbuf[7] = pdchMsg->params[iILevelTNAvailMask];
         sendDiagMsg(0x01,0,ts->tsNumber,8,tempbuf); 
       }
     }

     break;
   }

   case PDCH_DL_ASSIGN_ALERT_ACK:
   {   

      /* Message from L1Proxy telling us that the other DSP detected 
      *  the DL Assignment; thus, a DL Assign entry in a watch table is set.
      */

      /* index constants */
      const int iChanNum = 0;
      const int iStartTime = 2;
      const int iTag = 6;
      Uint32 FN1;
      Uint8  blockNum, tempbuf[24];
      Uint8 i;

      tsNum = pdchMsg->params[iChanNum] & 0x07;
      ts = &g_BBInfo[trxNum].TNInfo[tsNum];

      /* extract starting frame of UL block */
      FN1 = (pdchMsg->params[iStartTime]   << 24) | 
            (pdchMsg->params[iStartTime+1] << 16) |
            (pdchMsg->params[iStartTime+2] << 8)  |
             pdchMsg->params[iStartTime+3];
      /*
      *  Convert absolute frame number to block number (0..11)
      */
      blockNum = ((FN1%GPRS_MULTI_FRAMES)/(GPRS_MULTI_FRAMES/4)) * 3 +
                  (FN1%(GPRS_MULTI_FRAMES/4)) / 4;
      if ( blockNum >= BLOCKS_PER_GPRS_MULTI_FRAMES ) blockNum = 0;

      if( ts->u.group13.blkNumWatch[blockNum].tagFillCause != NONE) {
        /*
        *  Diagnostic for debugging conflicts
        */
        tempbuf[0] = ts->u.group13.blkNumWatch[blockNum].tagFillCause;
        tempbuf[1] = ts->u.group13.blkNumWatch[blockNum].tag; 
        tempbuf[2] = blockNum; 
        tempbuf[3] = (Uint8) ((Uint16)ts->u.group13.blkNumWatch[blockNum].tbf >> 8);
        tempbuf[4] = (Uint8) ((Uint16)ts->u.group13.blkNumWatch[blockNum].tbf & 0xff);
        sendDiagMsg(0x53,0,ts->tsNumber,5,tempbuf); 
      }

      /*
      *  Add entry to watch table  !! Overwrite for now, if a conflict exists
      */
      ts->u.group13.blkNumWatch[blockNum].tagFillCause = DLA;
      ts->u.group13.blkNumWatch[blockNum].tag = pdchMsg->params[iTag];
      ts->u.group13.blkNumWatch[blockNum].tbf = NULL;

      /*
      *  Diagnostic for debugging conflicts
      */
      for(i=0; i<12; i++) 
      {
        tempbuf[i*2] = ts->u.group13.blkNumWatch[i].tagFillCause;
        tempbuf[(i*2)+1] = ts->u.group13.blkNumWatch[i].tag; 
      }  
      sendDiagMsg(0x54,0,ts->tsNumber,24,tempbuf); 
 
      break;
   }
  }  
}




















