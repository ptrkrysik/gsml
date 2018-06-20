/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDASYNCHOREQUESTTODSP_CPP__
#define __INTG_SENDASYNCHOREQUESTTODSP_CPP__

//HOADDED
#include "rm/rm_head.h"

void intg_SendAsyncHoRequestToDsp( u8 trx, u8 chan);

void intg_SendAsyncHoRequestToDsp( u8 trx, u8 chan)
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16		    length;
   u8			    i;

   RDEBUG__(("ENTER-rm_SendHoRequestToDsp\n"));
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_AYSNCHO_REQ_MSB;
   msgToL1.buffer[length++] = RM_L1MT_AYSNCHO_REQ_LSB;
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = (0x08|chan);  // chan is TCH/F assumed only
   msgToL1.buffer[length++] = 0;
   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

} 

#endif /*__INTG_SENDASYNCHOREQUESTTODSP_CPP__*/
