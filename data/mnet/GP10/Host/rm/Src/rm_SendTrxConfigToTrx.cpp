/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDTRXCONFIGTOTRX_CPP__
#define __RM_SENDTRXCONFIGTOTRX_CPP__

#include "rm\rm_head.h"


void rm_SendTrxConfigToTrx(u8 trx)
{
   u16		   length;	 /* Length of the message to send to L1 */
   rm_ItcTxMsg_t   msgToL1;      /* Store an encoded message sent to L1 */
   
   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendTrxConfigToTrx: Entering...\n" ));

   /* Encode TRX CONFIGure message */
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_TRXCONFIG_MSB;	//0x15
   msgToL1.buffer[length++] = RM_L1MT_TRXCONFIG_LSB;  //0x00
   msgToL1.buffer[length++] = trx;                    //OAMrm_PREALPHA_TRX;
   msgToL1.buffer[length++] = OAMrm_AIRINTERFACE;	//0x02
   msgToL1.buffer[length++] = OAMrm_BTS_BSIC;	//0x5D
   msgToL1.buffer[length++] = OAMrm_NETWORKIFCONFIG; 	//0x00

   //Skip those not in-use fields
   length += 14;
   msgToL1.buffer[length++] = (u8) OAMrm_I_Q_SWAP;    //0:non-swap, 1:swap
   
   msgToL1.msgLength = length + 14;
    
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);   /* TBD: Provided by L1 */
   
} /* rm_SendTrxConfigToTrx() */

#endif /* __RM_SENDTRXCONFIGTOTRX_CPP__ */

