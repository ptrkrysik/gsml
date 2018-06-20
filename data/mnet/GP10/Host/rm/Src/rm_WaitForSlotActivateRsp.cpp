/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_WAITFORSLOTACTIVATERSP_CPP__
#define __RM_WAITFORSLOTACTIVATERSP_CPP__

#include "rm\rm_head.h"

void rm_WaitForSlotActivateRsp(u8 trx, u8 slot)
{
   u8	gotRsp;                /* If the rsp is for the expected slot */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_WaitForSlotActivateRsp: Entering...\n" ));

   gotRsp = RM_FALSE;

   while ( !gotRsp )
   {
     /* Read until get a TRX-Management group msg from trx */
      rm_WaitForL1Rsp(trx, RM_L1MG_TRXMGMT,RM_L1MT_NULL_MSB,RM_L1MT_NULL_LSB);

     /*
     ** Make sure if the rsp is for SlotActivate sent earlier
     ** Received response is stored in rm_RxMsgBuf. Caller is
     ** to make sure bit 8-5 (5 MSB bits) of slot is 0
     */
     
     if ( (rm_pItcRxL1Msg->l3_data.buffer[4]&RM_U8_0X07) == slot)
     {
      
         /* Now further check if rsp is for SlotActivate sent earlier */
         if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_SLOTACTIVACK_MSB &&
              rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_SLOTACTIVACK_LSB )
         {
              /* Yes, it's rsp wanted and also positive one */
	      gotRsp = RM_TRUE;  
              TDEBUG__(("TRACE-rm_WaitForSlotActivateRsp: got an ACK for %d\n",slot));
         }
         else if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_SLOTACTIVNACK_MSB &&
                   rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_SLOTACTIVNACK_LSB )
         {
	      /* Got a negative ack, dump message out, then hanging up */
              EDEBUG__(("ERROR-rm_WaitForSlotActivateRsp: got a NAK for TRX:%d,SLOT:%d\n",
		        trx,slot));
              EDUMP__((MAREA__,(u8 *)&rm_ItcRxMsg,sizeof(rm_ItcRxMsg_t) ));
	      /*
	      ** TBD: Hangup here. This may only work for PreAlpha release 
	      */
              while(1); 
	 } else 
              /* Got an unexpected message */
	      UDEBUG__(("UNKNOWN-rm_WaitForSlotActivateRsp:got non-related message!\n"));  

     } 
     else 
     {
         UDEBUG__(("UNKNOWN-rm_WaitForSlotActivateRsp:got got a message from wrong slot:%d\n",
	           slot ));
     }   /* End of if-statement */

   } /* End of while (!gotRsp) */

} /* End of rm_WaitForSlotActivateRsp() */

#endif /* __RM_WAITFORSLOTACTIVATERSP_CPP__ */
