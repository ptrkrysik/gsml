/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDHOCOMPLETE_CPP__
#define __RM_SENDHOCOMPLETE_CPP__

//HOADDED

#include "rm\rm_head.h"

void rm_SendHoComplete(void)
{
   rm_ItcTxL3Msg_t  msgToCc;	//Send message to CC
   T_CNI_L3_ID      ccId;
     
 
   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_SendHoComplete\n"));
 
   //Prepare Handover Complete message to CC
   //TBD:may contain SAPI later if requested
   msgToCc.module_id      = MODULE_RM;
   msgToCc.entry_id	  = rm_ItcRxEntryId;
   msgToCc.primitive_type = INTRA_L3_DATA;
   msgToCc.message_type   = INTRA_L3_RR_HANDOVER_COMPLETE;

   //Send Handover Complete message to CC
   ccId = CNI_RIL3_IRT_Get_CC_Id(rm_ItcRxEntryId);
   if ( ccId.msgq_id == CNI_NULL_L3_ID.msgq_id &&
        ccId.sub_id  == CNI_NULL_L3_ID.sub_id  )
        EDEBUG__(("ERROR-rm_SendHoComplete: null CC QId\n"));
   else
        api_SendMsgFromRmToCc(ccId.msgq_id, sizeof(rm_ItcTxL3Msg_t), (u8*)&msgToCc );
 
} /* End of rm_SendHoComplete() */

#endif /* __RM_SENDHOCOMPLETE_CPP__ */

