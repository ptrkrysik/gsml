/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDSYNCHINFOTOBCCHTRX_CPP__
#define __RM_SENDSYNCHINFOTOBCCHTRX_CPP__

#include "rm\rm_head.h"

void rm_SendSynchInfoToBcchTrx( u8 trx)
{
   u16		      length;	   /* Length of the L1 Slot Activate msg  */
   rm_ItcTxMsg_t      msgToL1;     /* Store an encoded message sent to L1 */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendSynchInfoToBcchTrx\n"));

   /* Encode SYNCH INFO message */

   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_SYNCH_MSB;	//0x00
   msgToL1.buffer[length++] = RM_L1MT_SYNCH_LSB;	//0x1D
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = 0;  //TBD: OAMrm_SYNCHINFO(0), MSBbyte
   msgToL1.buffer[length++] = 0;  //TBD: OAMrm_SYNCHINFO(1)
   msgToL1.buffer[length++] = 0;  //TBD: OAMrm_SYNCHINFO(2)
   msgToL1.buffer[length++] = 0;  //TBD: OAMrm_SYNCHINFO(3), LSBbyte

   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

} /* End of rm_SendSynchInfoToBcchTrx() */

#endif /* __RM_SENDSYNCHINFOTOBCCHTRX_CPP__ */


