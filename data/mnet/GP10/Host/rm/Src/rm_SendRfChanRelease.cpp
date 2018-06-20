/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDRFCHANRELEASE_CPP__
#define __RM_SENDRFCHANRELEASE_CPP__

#include "rm\rm_head.h"

void rm_SendRfChanRelease(rm_PhyChanDesc_t *pPhyChan)
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendRfChanRelease\n"));

   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_RFCHANREL_MSB;
   msgToL1.buffer[length++] = RM_L1MT_RFCHANREL_LSB;
   msgToL1.buffer[length++] = pPhyChan->trxNumber;
   msgToL1.buffer[length++] = pPhyChan->chanNumberMSB;
   msgToL1.buffer[length++] = pPhyChan->chanNumberLSB;
   msgToL1.msgLength        = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

} /* End of rm_SendRfChanRelease() */

#endif /* __RM_SENDRFCHANRELEASE_CPP__ */
