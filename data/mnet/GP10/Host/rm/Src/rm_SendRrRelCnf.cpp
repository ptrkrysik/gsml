/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDRRRELCNF_CPP__
#define __RM_SENDRRRELCNF_CPP__

#include "rm\rm_head.h"

void rm_SendRrRelCnf( void )
{
   rm_ItcTxL3Msg_t  msgToMm;	     //Send inter MM-Rm siganling

   //Monitoring entrance to a function
   //RDEBUG__(("ENTER-rm_SendRrRelCnf\n"));
   PDEBUG__(("ENTER@rm_SendRrRelCnf:entryId=%d\n",rm_ItcRxEntryId));

   //Encode RR_REL_IND and send to MM. TBD: Include cause or not
   msgToMm.module_id      = MODULE_RM;
   msgToMm.entry_id	  = rm_ItcRxEntryId;
   msgToMm.primitive_type = RR_REL_CNF;

   //Send the complete MM Paging Response message out
   api_SendMsgFromRmToMm( sizeof(rm_ItcTxL3Msg_t), (u8*) &msgToMm);
} 

#endif //__RM_SENDRRRELCNF_CPP__
