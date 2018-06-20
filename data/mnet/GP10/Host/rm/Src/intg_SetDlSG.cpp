/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SETDLSG_CPP__
#define __INTG_SETDLSG_CPP__

#include "rm\rm_head.h"

void intg_SetDlSG(u8 trx, u16 gain)
{
   rm_ItcTxMsg_t       msgToL1;    /* Store an encoded message sent to L1 */
   u16		     length;     /* Length of the message to send to L1 */

   /* Encode SET Uplink GAIN message for TRX */
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_SETDGAIN_MSB;	//0x11
   msgToL1.buffer[length++] = RM_L1MT_SETDGAIN_LSB;	//0x00
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = (u8)((gain>>8)&0x00FF);
   msgToL1.buffer[length++] = (u8)(gain&0x00FF);


   msgToL1.msgLength = length;

   /* Send TRX CONFIGure to L1 */ 
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);   /* TBD: Provided by L1 */

} /* intg_SetDlSG() */

#endif /* __INTG_SETDLSG_CPP__ */
