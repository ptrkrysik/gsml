/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: rrmsg.c
*
* Description:   
*   This file contains defined constants, enumerations and type definitions
*   of shared items in support of the Layer 1 Baseband Processing routines.
*   An item is included in this header file if it is used by multiple
*   baseband processing functions.
*
*****************************************************************************/
#include "string.h"
#include "gsmdata.h"
#include "dsp/dsphmsg.h" 
#include "rrmsg.h"
#include "intr.h"
#include "dsprotyp.h"

/******************************************************************************/
/* rrMsgProc - Radio Resource Messages Processing                             */
/*             Message send to Mobiles
/******************************************************************************/
ProcResult rrMsgProc(msgStruc  *p_msg )
{
  ProcResult result;    
      
  switch (p_msg->typeHiByte<<8 | p_msg->typeLoByte)
  {
     case  PH_DATA_REQ:
        result = rcvPhDataReq(p_msg);
     break;
     
     default:
		result = SUCCESS;
        break;
  }
  return(result);  
}          

/******************************************************************************/
/* rcvPhDataReq - receive  PH_DATA_REQ                                        */
/******************************************************************************/
ProcResult rcvPhDataReq(msgStruc  *p_msg )
{
  Uint8        tsNum;
  Uint8        cBits;
  Uint8        ccch;
  Uint8        subchan;
  Uint8        linkId;
  Uint8        *wrPtr, *rdPtr, i;
  t_DLSigBufs  *p_dlSigBuf;
  t_CBCHInfo   cbchBuf;
  Uint8        cbchBlock;   
  Uint8        sacchMsgCheck = 0;
  Uint8        queueDepth = DL_SIG_Q_DEPTH;     /* default Q depth for most of the buffers */
  ProcResult   result;
  
  tsNum = p_msg->params[0] & 0x07;
  cBits = (p_msg->params[0]) >> 3;
  ccch = p_msg->params[1] & 0x3f;
 linkId = (p_msg->params[2]) >> 5;
  
  cbchBlock = p_msg->params[2] & 0x3;  
  /*
  *  SMSCB message, different from normal RR Messages, is consistant of 4 blocks
  */  
  if (cBits == IE_CBCH || cBits == IE_CBCH_EXT)
  {  
     if(cBits == IE_CBCH)
     {
        cbchBuf.dlSigBuf =  & g_dlSigBufsCBCH[0];
     }   
     else   
        cbchBuf.dlSigBuf =  & g_dlSigBufsCBCH[1];

     cbchBuf.dlSigBuf->bufValid = 0;   
             
     wrPtr = cbchBuf.dlSigBuf->data[cbchBlock];     
     
     rdPtr =  & p_msg->params[3];  
     
     for(i=0; i<DL_SIG_MSG_SIZE; i++) wrPtr[i] = rdPtr[i];
     
     if(cbchBlock == 3)  /* check the last block of SMSCB message */
     {                                   
        /*
        * one complete message is ready
        */
        cbchBuf.dlSigBuf->bufValid = 1;     
     }
     return;
  }  
                         
  if (cBits)
  {          
	   if (cBits == 0x01)
	   {
	     if (linkId == 0)
	       p_dlSigBuf =  &g_dlSigBufsFACCH[0][tsNum];   /* FACCH */
	     else if (linkId == 2)
	     {
	       p_dlSigBuf =  &g_dlSigBufsTchSACCH[0][tsNum];   /* SACCH */  
	       sacchMsgCheck = 1; 
	     }
	   else 
	     p_dlSigBuf = NULL;
	 }
	 else if (cBits < 0x04)
	   p_dlSigBuf = NULL;    /* FACCH */ /* SACCH */	
	 else if (cBits < 0x10)
	 {    
	   if (cBits < 0x08)
	     subchan = cBits & 0x03;   /* SDCCH/4 + SACCH */
	   else
	     subchan = cBits & 0x07;   /* SDCCH/8 + SACCH */

	   if (linkId == 0){
	     if (cBits < 0x8)
	       p_dlSigBuf =  &g_dlSigBufsSDCCH[0][tsNum][subchan];   /* SDCCH */
	     else
	       p_dlSigBuf  =  &g_dlSigBufsSDCCH8[0][0][subchan];     /* SDCCH8 */ 
	   }
    else if (linkId == 2)
	   {
	     if (cBits < 0x8)
	       p_dlSigBuf =  &g_dlSigBufsSACCH[0][tsNum][subchan];     /* SACCH */  
      else
	       p_dlSigBuf =  &g_dlSigBufsSACCH8[0][0][subchan];     /* SACCH8 */ 

	     sacchMsgCheck = 1; 
	   }
	   else 
	     p_dlSigBuf = NULL;
	 }
	 else if (cBits == 0x10)
	 {
	   p_dlSigBuf =  NULL;  /* BCCH */
	 }

  /* new PCH + AGCH scheme */
	 else if (cBits == 0x12) 
	 {
    /* new function */
    putPPCHBuff(0, tsNum, ccch, RR_MANAGE_MSG, p_msg);	 

    /* buffering of data done in putPCCHBuff() */
    p_dlSigBuf = NULL;
	 }

	 else 
    p_dlSigBuf = NULL;
  } 
    
  if (p_dlSigBuf != NULL)
  {                                                      
    /*
    *  Reserve 2 bytes L1 Header for SACCH
    */
    wrPtr =  & (p_dlSigBuf->data[p_dlSigBuf->writeIndex][sacchMsgCheck<<1]);
    rdPtr =  &p_msg->params[3];  
    
    INTR_DISABLE(CPU_INT8);     
    
	   /*
	   *  check if specific Message QUEUE is full
	   */
	   if (p_dlSigBuf->numMsgsReady == queueDepth)
    {
		    /*
		    *   if queue is full, mark this message as unprocessed, it will be stored in
		    *   a globla queue and processed later (applies to all messages except PCH,AGCH)
		    */
		    result = UN_PROCESSED;
	   }
	   else
	   {

	     /*
	     *   The TI C6x compiler generates:
	     *
	     *   DL_SIG_MSG_SIZE - (sacchMsgCheck<<1) != DL_SIG_MSG_SIZE - sacchMsgCheck<<1
	     *
	     */
      for(i=0; i<(DL_SIG_MSG_SIZE - (sacchMsgCheck<<1)); i++) wrPtr[i] = rdPtr[i];
    
	     if(++p_dlSigBuf->writeIndex >= queueDepth) p_dlSigBuf->writeIndex = 0; 

      p_dlSigBuf->numMsgsReady++;  
      if (p_dlSigBuf->numMsgsReady > queueDepth)
	     {
        p_dlSigBuf->overflow++;  /* 5th arg to satisfy prototype */
        ReportError(DCH_MSG_OVERFLOW, p_msg->trx, tsNum, 1, & ccch); 
	       p_dlSigBuf->numMsgsReady = queueDepth;
      }

      result = SUCCESS;
    }
    INTR_ENABLE(CPU_INT8);
  }     
  else
  {   
    if (cBits != 0x12)
    {
      /*
      *  invalid RR Message
      */
      ReportError(13, p_msg->trx, tsNum, 20, (Uint8*)p_msg);  
	     result = FAILURE;
    }
  }
  return(result);
}


                  
/******************************************************************************/
/* chXlate - channel type translate                                           */
/******************************************************************************/
void chXlate(t_groupNum  groupNum,
        t_chanSel   chanType,
        Uint8       subchan,
        Uint8       *p_chanNum,
        Uint8       *p_linkId
       )
{
  switch (chanType)
  {    
    case CBCH:
      *p_chanNum = (IE_CBCH<<3);
      *p_linkId = 0x0;    
      break;
      
    case CBCH_EXT:
      *p_chanNum = (IE_CBCH_EXT<<3);
      *p_linkId = 0x0;    
      break;
          
    case AGCH:
    case PCH:
      *p_chanNum = (0x12<<3);
      *p_linkId = 0x0;    
      break;
      
    case SACCH:
      switch (groupNum)
      {
        case GSM_GROUP_1:
          *p_chanNum = 0x08;
          break;

        case GSM_GROUP_2: 
        case GSM_GROUP_3:
          *p_chanNum = 0x10 | ((subchan & 0x01)<< 3);                  
          break;

        case GSM_GROUP_5:
          *p_chanNum = 0x20 | ((subchan & 0x03)<< 3);
          break;

        case GSM_GROUP_7:
          *p_chanNum = 0x40 | ((subchan & 0x07)<< 3);        
          break;
      }
      *p_linkId = 0x40;
      break;
    
    case FACCH:
      switch (groupNum)
      {
        case GSM_GROUP_1:
          *p_chanNum = 0x08;
          break;
          
        case GSM_GROUP_2: 
        case GSM_GROUP_3:
          *p_chanNum = 0x10 | (subchan << 3);                  
          break;
      }         
      *p_linkId = 0x00;
      break;

    case SDCCH4:
		switch(groupNum){
		case GSM_GROUP_5:
          *p_chanNum = 0x20 | (subchan << 3);
        break;

	   case GSM_GROUP_7:
          *p_chanNum = 0x40 | ((subchan & 0x07)<< 3);        
          break;
		}

		  *p_linkId = 0x00;
      break;

    case SDCCH8:
      *p_chanNum = 0x40 | (subchan << 3);        
      *p_linkId = 0x00;
      break;

    case RACH:
      switch (groupNum)
      {
        case GSM_GROUP_1:
          *p_chanNum = 0x08;
          *p_linkId = 0x00;
          break;
          
        default:
          *p_chanNum = 0x88;
          *p_linkId = 0x20;
          break;
      }
      break;

    case PRACH:
       *p_chanNum = CHAN_NUM_IE_PRACH<<3;
       *p_linkId = 0x00;
       break;

          
    case PACCH:
       *p_chanNum = CHAN_NUM_IE_PACCH<<3;
       *p_linkId = 0x00;
       break;
      
    case PTCCH:
       *p_chanNum = CHAN_NUM_IE_PTCCH<<3;
       *p_linkId = 0x00;
       break;

    case PDTCH:
       *p_chanNum = CHAN_NUM_IE_PDTCH<<3;
       *p_linkId = 0x00;
       break;
      
    default:
      *p_chanNum = 0;
      *p_linkId = 0x20;  
  }
}










