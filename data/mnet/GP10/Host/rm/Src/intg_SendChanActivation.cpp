/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDCHANACTIVATION_CPP__
#define __INTG_SENDCHANACTIVATION_CPP__

#include "rm/rm_head.h"

void intg_SendChanActivation( u8 trx,u8 chan);

void intg_SendChanActivation( u8 trx, u8 chan)
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;
   u8				i;

   RDEBUG__(("ENTER-rm_SendChanActivation\n"));
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_MSB;
   msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_LSB;
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = (0x08|chan);
   msgToL1.buffer[length++] = 0;
   msgToL1.buffer[length++] = RM_L1ACTTYPE_IMMASS;

   //PR1248 Begin
   msgToL1.buffer[length] = 0;  // DTX ul&dl OFF

   //PR1381 BEGIN
   if ( ((int)OAMrm_MS_UPLINK_DTX_STATE==1) ||
        ((int)OAMrm_MS_UPLINK_DTX_STATE==0)  ) 
        msgToL1.buffer[length] |= 1; //DTX ul ON
   //PR1381 END

// PR1319
// if (!trx && OAMrm_MS_DNLINK_DTX_STATE)
   if (trx && OAMrm_MS_DNLINK_DTX_STATE)
       msgToL1.buffer[length] |= 2; //DTX dl ON
   length++;
   //PR1248 End

   msgToL1.buffer[length++] = RM_L1DATATTR_SPEECH;
   msgToL1.buffer[length++] = RM_L1RATTYPE_TCHF;
   msgToL1.buffer[length++] = 1; //RM_L1CODALGO_NORES;
   msgToL1.buffer[length++] = RM_L1CIPHTYPE_NCIPH;
   for (i=0;i<RM_L1CIPHKEY_LEN;i++)
           msgToL1.buffer[length++] = 0;

   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

} 
	
#endif /*__INTG_SENDCHANACTIVATION_CPP__*/
