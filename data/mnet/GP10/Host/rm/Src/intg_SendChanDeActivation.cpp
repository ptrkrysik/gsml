/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDCHANDEACTIVATION_CPP__
#define __INTG_SENDCHANDEACTIVATION_CPP__

#include "rm\rm_head.h"

void intg_SendChanDeActivation( u8 trx, u8 chan)
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;

   RDEBUG__(("ENTER-rm_SendChanDeActivation\n"));
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_RFCHANREL_MSB;
   msgToL1.buffer[length++] = RM_L1MT_RFCHANREL_LSB;
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = (0x08|chan);
   msgToL1.buffer[length++] = 0;
   msgToL1.msgLength = length;
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

} 
	
#endif /*__INTG_SENDCHANDEACTIVATION_CPP__*/
