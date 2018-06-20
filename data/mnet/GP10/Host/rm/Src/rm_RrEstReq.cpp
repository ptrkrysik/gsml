/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_RRESTREQ_CPP__
#define __RM_RRESTREQ_CPP__

#include "rm\rm_head.h"

void rm_RrEstReq(void)
{
   //Monitoring entrance to a func
   RDEBUG__(("ENTER@rm_RrEstReq\n"));
   
   switch(rm_pItcRxMmMsg->message_type)
   {
   case MM_PAGE_REQ: 
        //Block any terminating call initiation if needed
        if (rm_PhyChanBCcch[RM_PCH_IDX].amState!=unlocked)
        {
		PDEBUG__(("INFO@rm_RrEstReq:PCH-resident TS blocked or shutting down now\n"));
		return;
        }
        //Only PagingReqType1 now!
        rm_SendPagingRequestType1();
        break;

   case INTRA_L3_RR_HANDOVER_REQ:
        //Block any terminating call initiation if needed
        if (rm_PhyChanBCcch[RM_PCH_IDX].amState!=unlocked)
        {
		PDEBUG__(("INFO@rm_RrEstReq:PCH-resident TS blocked or shutting down now\n"));
            //Send back Ho NAck to this Handover Req immediately!
            rm_SendHoReqNAck(rm_pItcRxMmMsg->l3_data.handReq.mmId);
		return;
        }
        rm_HoReq();
        break;

   default:
	  UDEBUG__(("UNKNOWN@rm_RrEstReq: message type:%d\n", rm_pItcRxMmMsg->message_type));
  	  break;

   } //End of switch()

} //End rm_RrEstReq()

#endif //__RM_RRESTREQ_CPP__
