/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_WAITFORL1RSP_CPP__
#define __RM_WAITFORL1RSP_CPP__

#include "rm\rm_head.h"

void rm_WaitForL1Rsp(u8 trx, u8 msgGroup, u8 msgTypeMSB, u8 msgTypeLSB)
{
   s16	ret;		/* Return code from calling a underlying fun */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_WaitForL1Rsp\n"));

   /* Expected response to the message */
   TDEBUG__(("TRACE-rm_WaitForL1Rsp:Expected msg:TRX=%x,MG=%x,MT_Hi=%x,MT_Lo=%x\n",
             trx, msgGroup, msgTypeMSB, msgTypeLSB));
  
   /* TBD: Wait forever unless got the expected rsp */
 
   while (1) 
   {
      ret = rm_msgQReceive(rm_MsgQId,(s8 *)&rm_ItcRxMsg, 
     		                      RM_MAX_RXQMSG_LEN, WAIT_FOREVER );
      if ( ret==ERROR )
      {
         /* Error in reading VxWorks message queue */
         EDEBUG__(("ERROR-rm_WaitForL1Rsp: VxWorks Queue read error:%d\n",
	           errno ));
      }
      else if ( rm_ItcRxMsg.module_id == MODULE_L1 )
      {   
	 rm_pItcRxL1Msg = (rm_ItcRxL1Msg_t *) &rm_ItcRxMsg;

	 /* Got a message from L1. Check if it's expected response */
         if ( rm_pItcRxL1Msg->l3_data.msgLength >= RM_L1MIN_MSG_LEN &&
	      rm_pItcRxL1Msg->l3_data.buffer[0] == msgGroup         )
         {
              if (  (rm_pItcRxL1Msg->l3_data.buffer[1]==msgTypeMSB &&
	             rm_pItcRxL1Msg->l3_data.buffer[2]==msgTypeLSB )   ||
		    (msgTypeMSB==RM_L1MT_NULL_MSB && 
                     msgTypeLSB==RM_L1MT_NULL_LSB )		       )
	      {	 
                    if ( trx==RM_L1TRX_NULL ||
                         rm_pItcRxL1Msg->l3_data.buffer[3]==trx )
		
	                 /* Got the expected response! */
	                 return;
	      } 	
         }
	 else 
         {
              /*=TBD=*/
	      /* Received unexpected stuff from L1. Try next read */	 
	      UDEBUG__(("UNKNOWN-rm_WaitForL1Rsp: got unexpected L1-msg while awaiting rsp\n" ));
              UDUMP__((MAREA__,(u8*)&rm_ItcRxMsg, sizeof(rm_ItcRxMsg_t) ));
	 }
      } 
      else
      {
          /* Received messages from non-L1 origin. Try next read */	 
	  UDEBUG__(("UNKNOWN-rm_WaitForL1Rsp: got a non-L1 msg while waiting for a rsp\n" ));
	  UDUMP__((MAREA__, (u8*)&rm_ItcRxMsg, sizeof(rm_ItcRxMsg_t) ));
      }

   } /* End of while() */

} /* End of rm_WaitForL1Rsp() */

#endif /* __RM_WAITFORL1RSP_CPP__ */
