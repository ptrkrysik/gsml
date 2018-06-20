/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDSLOTACTIVATETOTRX_CPP__
#define __RM_SENDSLOTACTIVATETOTRX_CPP__

#include "rm\rm_head.h"

void rm_SendSlotActivateToTrx(u8 trx, u8 slot)
{
   u16		      length;	   /* Length of the L1 Slot Activate msg  */
   rm_ItcTxMsg_t      msgToL1;     /* Store an encoded message sent to L1 */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendSlotActivateToTrx: Entering...\n" ));

   /* ATTN:
   ** ====
   ** slot's bit 8-5 i.e. 5 MSB bits must be masked by caller of this
   ** function. Caller is also responsibile for validity of SLOT COMB
   ** value associated with this slot. This function just passes COMB
   ** it gets to L1
   */
 
   /* Encode Slot Activate message  */

   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_SLOTACTIV_MSB;	//0x17
   msgToL1.buffer[length++] = RM_L1MT_SLOTACTIV_LSB;	//0x00
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = slot;
   msgToL1.buffer[length++] = OAMrm_TRX_SLOT_COMB(trx,slot);

   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);   /* TBD: Provided by L1 */

} /* End of rm_SendSlotActivateToSlot() */

void intg_SendSlotActivateToTrx(u8 trx, u8 slot, u8 comb)
{
   u16		      length;	   /* Length of the L1 Slot Activate msg  */
   rm_ItcTxMsg_t      msgToL1;     /* Store an encoded message sent to L1 */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-intg_SendSlotActivateToTrx: Entering...\n" ));

   /* ATTN:
   ** ====
   ** slot's bit 8-5 i.e. 5 MSB bits must be masked by caller of this
   ** function. Caller is also responsibile for validity of SLOT COMB
   ** value associated with this slot. This function just passes COMB
   ** it gets to L1
   */
 
   /* Encode Slot Activate message  */

   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_SLOTACTIV_MSB;	//0x17
   msgToL1.buffer[length++] = RM_L1MT_SLOTACTIV_LSB;	//0x00
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = slot;
   msgToL1.buffer[length++] = comb; //OAMrm_TRX_SLOT_COMB(trx,slot);

   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);   /* TBD: Provided by L1 */
}
#endif /* __RM_SENDSLOTACTIVATETOTRX_CPP__ */
