/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDHOFAILURE_CPP__
#define __RM_SENDHOFAILURE_CPP__

//HOADDED
#include "rm\rm_head.h"

//ext-HO <xxu:06-25-01>added cause parameter
void rm_SendHoFailure(int cause, int rrCause)
{
   rm_ItcTxL3Msg_t  msgToCc;	  //Hold msg to CC
   T_CNI_L3_ID      ccId;	  //Refer to CCQID
     
   //Monitoring entrance to a function 
   RDEBUG__(("ENTER-rm_SendHoFailure\n"));
 
   //TBD: may contain SAPI later if requested
   msgToCc.module_id      = MODULE_RM;
   msgToCc.entry_id	  = rm_ItcRxEntryId;
   msgToCc.primitive_type = INTRA_L3_DATA;
   msgToCc.message_type   = INTRA_L3_RR_HANDOVER_FAILURE;
   msgToCc.l3_data.handFail.cause = cause;
   msgToCc.l3_data.handFail.rrCause = rrCause;

   //Refer to relevant CC task Queue ID by entryID
   ccId = CNI_RIL3_IRT_Get_CC_Id(rm_ItcRxEntryId);
   if ( ccId.msgq_id == CNI_NULL_L3_ID.msgq_id &&
        ccId.sub_id  == CNI_NULL_L3_ID.sub_id  )
       EDEBUG__(("ERROR-rm_SendHoFailure: null CC Task QId\n"));
   else
       api_SendMsgFromRmToCc(ccId.msgq_id, sizeof(rm_ItcTxL3Msg_t), (u8*)&msgToCc );
 
} /* End of rm_SendHoFailure() */

#endif /* __RM_SENDHOFAILURE_CPP__ */
