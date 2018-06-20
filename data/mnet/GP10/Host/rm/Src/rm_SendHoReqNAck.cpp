/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef  __RM_SENDHOREQNACK_CPP__
#define  __RM_SENDHOREQNACK_CPP__

//HOADDED
#include "rm\rm_head.h"

void rm_SendHoReqNAck(u8 ref)
{
   rm_ItcTxL3Msg_t  msgToMm;	//Send RM->MM message

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_SendHoReqNAck\n"));

   //Prepare HoReqNAck MM message to send
   msgToMm.module_id      = MODULE_RM;
   msgToMm.entry_id       = CNI_IRT_NULL_ID;
   msgToMm.primitive_type = INTRA_L3_RR_EST_REJ;
   msgToMm.message_type   = INTRA_L3_RR_HANDOVER_REQ_NACK;
   msgToMm.l3_data.handReqNack.cause = JC_FAILURE_RADIO_RESOURCE_UNAVAILABLE;
   msgToMm.l3_data.handReqNack.mmId  = ref; //rm_pSmCtxt->pPhyChan->hoRef;

   //Send out the HoReqNAck messag to MM
   api_SendMsgFromRmToMm(sizeof(rm_ItcTxL3Msg_t), (u8*)&msgToMm);

} /* End of rm_SendHoReqNAck() */

#endif /* __RM_SENDHOREQNACK_CPP__ */
