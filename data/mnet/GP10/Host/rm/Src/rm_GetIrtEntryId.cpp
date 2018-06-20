/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_GETIRTENTRYID_CPP__
#define __RM_GETIRTENTRYID_CPP__

#include "rm\rm_head.h"

rm_IrtEntryId_t rm_GetIrtEntryId( rm_ItcRxL1Msg_t * pL1Msg )
{
   u8			idx,trx,chan;
   rm_IrtEntryId_t	entry_id;


   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_GetIrtEntryId\n"));
  
   /* 
   ** Solve for entry id in terms of message origin 
   ** Please note CHANNE REQ, MM PAGE REQ have been
   ** processed before calling this function.
   */

   trx  = pL1Msg->l3_data.buffer[3];
   chan = pL1Msg->l3_data.buffer[4];

   entry_id = CNI_IRT_NULL_ID;

   if ( pL1Msg->l3_data.buffer[0] != RM_L1MG_DEDMGMT )
        EDEBUG__(("ERROR-rm_GetIrtEntryId: not DEDMGMT group\n"));

   else if ( RM_ISTCHF(chan) )
   {
	idx      = RM_TCHF_ENTRYIDX(trx, chan);
	entry_id = rm_TchfEntryId[idx];
   }
   else if ( RM_ISSDCCH4(chan) )
   {
	idx      = RM_SDCCH4_ENTRYIDX(trx,chan);
	entry_id = rm_Sdcch4EntryId[idx];
PDEBUG__(("TRACK-rm_GetIrtEntryId: Idx=%d, entryId=%d\n",idx,entry_id));

   }
   else
	PDEBUG__(("UNKNOWN-rm_GetIrtEntryId: chan type:%d",chan));

   return (entry_id);

} /* End of rm_GetIrtEntryId() */

#endif /* __RM_GETIRTENTRYID_CPP__ */
