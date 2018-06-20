/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_SENDMODEMODIFY_CPP__
#define __RM_SENDMODEMODIFY_CPP__

#include "rm\rm_head.h"

void rm_SendModeModify( rm_PhyChanDesc_t    *pPhyChan,
		        rm_PhyChanActDesc_t *pActDesc )
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendModeModify\n"));
	
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_MODEMODIFY_MSB;
   msgToL1.buffer[length++] = RM_L1MT_MODEMODIFY_LSB;
   msgToL1.buffer[length++] = pPhyChan->trxNumber;
   msgToL1.buffer[length++] = pPhyChan->chanNumberMSB;
   msgToL1.buffer[length++] = pPhyChan->chanNumberLSB;
   msgToL1.buffer[length++] = 0; //This is a dummy byte for avoiding change in DSP s/w
   msgToL1.buffer[length++] = pActDesc->dtxCtrl;
   msgToL1.buffer[length++] = pActDesc->datAttr;
   msgToL1.buffer[length++] = pActDesc->ratType;
   msgToL1.buffer[length++] = pActDesc->codAlgo;
   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
}

#endif /* End of rm_SendModeModify() */

