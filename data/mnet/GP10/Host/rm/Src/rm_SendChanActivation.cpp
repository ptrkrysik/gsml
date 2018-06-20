/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_SENDCHANACTIVATION_CPP__
#define __RM_SENDCHANACTIVATION_CPP__


#include "rm\rm_head.h"

void rm_SendChanActivation( rm_PhyChanDesc_t    *pPhyChan,
			    rm_PhyChanActDesc_t *pActDesc )
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendChanActivation\n"));
	
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_MSB;
   msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_LSB;
   msgToL1.buffer[length++] = pPhyChan->trxNumber;
   msgToL1.buffer[length++] = pPhyChan->chanNumberMSB;
   msgToL1.buffer[length++] = pPhyChan->chanNumberLSB;
   msgToL1.buffer[length++] = pActDesc->actType;
   msgToL1.buffer[length++] = pActDesc->dtxCtrl;
   msgToL1.buffer[length++] = pActDesc->datAttr;
   msgToL1.buffer[length++] = pActDesc->ratType;
   msgToL1.buffer[length++] = pActDesc->codAlgo;
   msgToL1.buffer[length++] = pActDesc->ciphSet;
   RM_MEMCPY(&msgToL1.buffer[length], pActDesc->ciphKey, RM_L1CIPHKEY_LEN);
   length                   = length + RM_L1CIPHKEY_LEN;    
   msgToL1.buffer[length++] = pActDesc->bsPower;


// NewlyAdded
// msgToL1.buffer[length++] = pActDesc->msPower;
   msgToL1.buffer[length++] = rm_pItcRxMdMsg->l3_data.buffer[4]; //MS pwr on Rach

   msgToL1.buffer[length++] = pActDesc->taValue;

//NewlyAdded
   msgToL1.buffer[length++] = OAMrm_MS_TX_PWR_MAX_CCH;           //MS init TX pwr

   msgToL1.buffer[length++] = pActDesc->UIC;     /* TBD: may use memcpy later */
   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

} /* End of rm_SendChanActivation() */

#endif /* __RM_SENDCHANACTIVATION_CPP__ */
