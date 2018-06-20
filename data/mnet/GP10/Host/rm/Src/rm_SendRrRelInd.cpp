/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDRRRELIND_CPP__
#define __RM_SENDRRRELIND_CPP__

#include "rm\rm_head.h"

void rm_SendRrRelInd(u8 cause, rm_IrtEntryId_t entryId)
{
   rm_ItcTxL3Msg_t  msgToMm;	 

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_SendRrRelInd\n"));

   //Encode RR_REL_IND and send to MM. TBD: Include cause or not
   msgToMm.module_id      = MODULE_RM;
   msgToMm.entry_id	  = entryId;
   msgToMm.primitive_type = RR_REL_IND;

   msgToMm.cause          = rm_pSmCtxt->relCause;

   //Send the complete MM Paging Response message out
   api_SendMsgFromRmToMm( sizeof(rm_ItcTxL3Msg_t), (u8*) &msgToMm);
}

#endif //__RM_SENDRRRELCNF_CPP__
