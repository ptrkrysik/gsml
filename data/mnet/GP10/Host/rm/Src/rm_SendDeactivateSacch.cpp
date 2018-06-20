/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDDEACTIVATESACCH_CPP__
#define __RM_SENDDEACTIVATESACCH_CPP__

#include "rm\rm_head.h"

void  rm_SendDeactivateSacch(rm_PhyChanDesc_t *pPhyChan)
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendDeactivateSacch\n"));
	
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_DEACTIVSACCH_MSB;
   msgToL1.buffer[length++] = RM_L1MT_DEACTIVSACCH_LSB;
   msgToL1.buffer[length++] = pPhyChan->trxNumber;
   msgToL1.buffer[length++] = pPhyChan->chanNumberMSB;
   msgToL1.buffer[length++] = pPhyChan->chanNumberLSB;
   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
}

#endif /* __RM_SENDDEACTIVATESACCH_CPP__ */


