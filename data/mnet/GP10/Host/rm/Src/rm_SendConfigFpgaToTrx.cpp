/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDCONFIGFPGADELAYTOTRX_CPP__
#define __RM_SENDCONFIGFPGADELAYTOTRX_CPP__

#include "rm\rm_head.h"

void rm_SendConfigFpgaDelayToTrx(u8 trx)
{
   rm_ItcTxMsg_t   msgToL1;      /* Store an encoded message sent to L1 */
   u16		   length;	 /* Length of the message to send to L1 */

   /* It's caller's responsiblity to make sure trx valid */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendConfigFpgaDelayToTrx: Entering...\n" ));

   /* Encode CONFIGure FPGA DELAY message to TRX */
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;			//0x10
   msgToL1.buffer[length++] = RM_L1MT_CONFIGFPGADELAY_MSB;	//0x14
   msgToL1.buffer[length++] = RM_L1MT_CONFIGFPGADELAY_LSB;	//0x00
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = OAMrm_TRX_ULFPGADELAY_MSB(trx);  
   msgToL1.buffer[length++] = OAMrm_TRX_ULFPGADELAY_LSB(trx);
   msgToL1.buffer[length++] = OAMrm_TRX_DLFPGADELAY_MSB(trx);
   msgToL1.buffer[length++] = OAMrm_TRX_DLFPGADELAY_LSB(trx);

//PDEBUG__(("%d-th FPGA:%d,%d,%d,%d\n",
//    OAMrm_TRX_ULFPGADELAY_MSB(trx),
//    OAMrm_TRX_ULFPGADELAY_LSB(trx),
//    OAMrm_TRX_DLFPGADELAY_MSB(trx),
//    OAMrm_TRX_DLFPGADELAY_LSB(trx) ));

   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);   /* TBD: Provided by L1 */

} /* rm_SendConfigFpgaDelayToTrx() */

#endif /* __RM_SENDCONFIGFPGADELAYTOTRX_CPP__ */
