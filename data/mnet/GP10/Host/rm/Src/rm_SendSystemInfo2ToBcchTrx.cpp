/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef  __RM_SENDSYSTEMINFO2TOBCCHTRX_CPP__
#define  __RM_SENDSYSTEMINFO2TOBCCHTRX_CPP__

#include "rm\rm_head.h"

void rm_SendSystemInfo2ToBcchTrx(u8 trx, u8 SI, T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   u16		    length;	     /* Length of a msg sent to L1 */
   rm_EdRet_t       rrEdRet;	     /* Return code from RR decode */
   rm_L3Data_t      l3_data; 	     /* Encoded SI Type 2  */
   rm_ItcTxMsg_t    msgToL1;	     /* Send BCCH INFORMATION msg to L1 */
  
   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendSystemInfo2ToBcchTrx\n"));
 
   /* Zap rm_UmMsg 0-clean first */
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );
        
   /* Populate SI2 for use by RRM encoder */
   rm_PopulateSI2(barState);
   RM_MEMCPY( &rm_UmMsg, &rm_UmSI2, sizeof(rm_UmSI2_t));

   /* Call RR message encoding functionality */
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   /* check the encoding result before sending the message */
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	
	  /* Wrap the encoded SI in PH_DATA_REQ */
        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_COMCHAN;
        msgToL1.buffer[length++] = RM_L1MT_BCCHINFO_MSB;
        msgToL1.buffer[length++] = RM_L1MT_BCCHINFO_LSB;
	  msgToL1.buffer[length++] = trx;
        msgToL1.buffer[length++] = RM_L1BCCH_CHANNUMBER_MSB;
	  msgToL1.buffer[length++] = RM_L1BCCH_CHANNUMBER_LSB;
	  msgToL1.buffer[length++] = SI;
        msgToL1.buffer[length++] = l3_data.msgLength;
	  RM_MEMCPY(&msgToL1.buffer[length], l3_data.buffer, l3_data.msgLength);
        length = length + l3_data.msgLength;
        msgToL1.buffer[length++] = 1; //Start time==immediate
        msgToL1.buffer[length++] = 0;
        msgToL1.buffer[length++] = 0;
        msgToL1.msgLength        = length;

	  /* Send BCCH INFORMATION TO L1 */
        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
       
	break;

   default:
	/* Encoding failed */
        EDEBUG__(("ERROR-rm_SendSystemInfo2ToBcchTrx: RIL3-RR encode error:%d\n",
		  rrEdRet ));
   	break;

   } /* End of switch(rrEdRet) */

} /* End of rm_SendSystemInfo2ToBcchTrx() */

#endif /* __RM_SENDSYSTEMINFO2TOBCCHTRX_CPP__ */
