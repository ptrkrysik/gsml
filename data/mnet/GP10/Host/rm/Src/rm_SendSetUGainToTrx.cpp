/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDSETUGAINTOTRX_CPP__
#define __RM_SENDSETUGAINTOTRX_CPP__

#include "rm\rm_head.h"

void rm_SendSetUGainToTrx(u8 trx)
{
   rm_ItcTxMsg_t       msgToL1;    /* Store an encoded message sent to L1 */
   u16		       length;	   /* Length of the message to send to L1 */

   /* It's caller's responsiblity to make sure trx valid */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendSetUGainToTrx: Entering...\n" ));

   /* Encode SET Uplink GAIN message for TRX */
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_SETUGAIN_MSB;	//0x10
   msgToL1.buffer[length++] = RM_L1MT_SETUGAIN_LSB;	//0x00
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = (u8)((OAMrm_UPLINK_SGAINCTRL_SETTING(trx))>>8); //0x00
   msgToL1.buffer[length++] = (u8)(OAMrm_UPLINK_SGAINCTRL_SETTING(trx)); //0x00
   msgToL1.msgLength = length;

   /* Send TRX CONFIGure to L1 */ 
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);   /* TBD: Provided by L1 */

} /* rm_SendSetUGainToTrx() */

#endif /* __RM_SENDSETUGAINTOTRX_CPP__ */
