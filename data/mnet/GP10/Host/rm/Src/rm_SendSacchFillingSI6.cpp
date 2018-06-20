/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef  __RM_SENDSACCHFILLINGSI6_CPP__
#define  __RM_SENDSACCHFILLINGSI6_CPP__

#include "rm\rm_head.h"

void rm_SendSacchFillingSI6(u8 trx, u8 SI)
{
   u16		  length;	     /* Length of a msg sent to L1 */
   rm_EdRet_t       rrEdRet;	     /* Return code from RR decode */
   rm_L3Data_t      l3_data; 	     /* Encoded SI Type 5  */
   rm_ItcTxMsg_t    msgToL1;	     /* Send BCCH INFORMATION msg to L1 */
  
   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendSacchFillingToTrx\n"));

   /* Zap rm_UmMsg 0-clean first */
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );
        
   /* Populate SI2 for use by RRM encoder */
   rm_PopulateSI6();
   RM_MEMCPY( &rm_UmMsg, &rm_UmSI6, sizeof(rm_UmSI6_t));

   /* Call RR message encoding functionality */
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   /* check the encoding result before sending the message */
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:

//printf("rm_SendSacchFillingSI6: SYSINFO6 length=%d\n", l3_data.msgLength);
//for (int i=0;i<l3_data.msgLength;i++)
//printf("%02x ", l3_data.buffer[i]);
//printf("\n"); 
	/* Wrap the encoded SI in SACCH FILLING sent to L1 */
        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;
        msgToL1.buffer[length++] = RM_L1MT_SACCHFILL_MSB;
        msgToL1.buffer[length++] = RM_L1MT_SACCHFILL_LSB;
	  msgToL1.buffer[length++] = trx;
	  msgToL1.buffer[length++] = SI;
        msgToL1.buffer[length++] = l3_data.msgLength;
	  RM_MEMCPY(&msgToL1.buffer[length], l3_data.buffer, l3_data.msgLength);
        length = length + l3_data.msgLength;
        msgToL1.buffer[length++] = 1; //Start time==immediate
        msgToL1.buffer[length++] = 0;
        msgToL1.buffer[length++] = 0;
        msgToL1.msgLength        = length;

	  /* Send SACCH FILLING TO L1 */
        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
       
	break;

   default:
	/* Encoding failed */
        EDEBUG__(("ERROR-rm_SendSacchFillingSI6: RIL3-RR encode error:%d\n",
		  rrEdRet ));
   	break;

   } /* End of switch(rrEdRet) */

} /* End of rm_SendSacchFillingSI6() */

#endif /* __RM_SENDSACCHFILLINGSI6_CPP__ */
