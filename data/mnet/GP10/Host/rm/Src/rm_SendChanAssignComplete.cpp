/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_SENDCHANASSIGNCOMPLETE_CPP__
#define __RM_SENDCHANASSIGNCOMPLETE_CPP__

#include "rm\rm_head.h"

void rm_SendChanAssignComplete(void)
{
   rm_ItcTxL3Msg_t  msgToCc;	     /* Send inter MM-Rm siganling */
   T_CNI_L3_ID      ccId;
     
 
   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendChanAssignComplete\n"));
 
   /* TBD: may contain SAPI later if requested */
   msgToCc.module_id      = MODULE_RM;
   msgToCc.entry_id	  = rm_ItcRxEntryId;
   msgToCc.primitive_type = (IntraL3PrimitiveType_t)rm_pSmCtxt->callIdx; //INTRA_L3_DATA;
   msgToCc.message_type   = INTRA_L3_RR_CHANN_ASSIGN_COM;

   /* Send the complete MM Paging Response message out */
   ccId = CNI_RIL3_IRT_Get_CC_Id(rm_ItcRxEntryId);
   if ( ccId.msgq_id == CNI_NULL_L3_ID.msgq_id &&
        ccId.sub_id  == CNI_NULL_L3_ID.sub_id  )
       EDEBUG__(("ERROR-rm_SendChanAssignComplete: null CC QId\n"));
   else
       api_SendMsgFromRmToCc(ccId.msgq_id, sizeof(rm_ItcTxL3Msg_t), (u8*)&msgToCc );
 
} /* End of rm_SendChanAssignComplete() */

#endif /* __RM_SENDCHANASSIGNCOMPLETE_CPP__ */

