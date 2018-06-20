/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef  __RM_SENDHOREQACK_CPP__
#define  __RM_SENDHOREQACK_CPP__

//HOADDED

#include "rm\rm_head.h"

u8 rm_SendHoReqAck(void)
{
   rm_EdRet_t       rrEdRet;	     		
   rm_ItcTxL3Msg_t  msgToMm;   //Message to send to MM
   rm_PhyChanDesc_t *pPhyChan; //Description of physical channel     		
   IntraL3HandoverRequestAck_t *pMmHoCmd;  //Message carried in msgToMm
   T_CNI_RIL3RRM_MSG_HANDOVER_COMMAND *pUmHoCmd; //air Handover Command 
   rm_L3Data_t      l3_data; 	 //Store encoded Handover Command message

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_SendHoReqAck\n"));

   //Initialize pointers and memeory used properly first
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );
   pMmHoCmd = (IntraL3HandoverRequestAck_t *)&(msgToMm.l3_data.handReqAck);
   pUmHoCmd = (T_CNI_RIL3RRM_MSG_HANDOVER_COMMAND *)&(rm_UmMsg.handoverCommand);
   pPhyChan = rm_pSmCtxt->pPhyChan; //Pointing to corresponding physical chan.

   //Encode MM Handover Req Ack message header
   msgToMm.module_id      = MODULE_RM;
   msgToMm.entry_id	  = rm_ItcRxEntryId;
   msgToMm.primitive_type = RR_EST_CNF;
   msgToMm.message_type   = INTRA_L3_RR_HANDOVER_REQ_ACK;
   
   //Encode MM Handover Req Ack message body: hoRef
   pMmHoCmd->mmId = rm_pSmCtxt->pPhyChan->hoRef;

   //Encode MM Handover Req Ack message body: Handover Command
   //Install IEs: header part
   pUmHoCmd->header.protocol_descriminator = RM_PD;
   pUmHoCmd->header.si_ti		   = RM_SI;
   pUmHoCmd->header.message_type	   = CNI_RIL3RRM_MSGID_HANDOVER_COMMAND;

   //Install IE: Cell Description
   pUmHoCmd->cellDescription.ie_present = RM_TRUE;
   pUmHoCmd->cellDescription.arfcn	    = OAMrm_BCCH_ARFCN;
   pUmHoCmd->cellDescription.ncc        = OAMrm_NCC;      
   pUmHoCmd->cellDescription.bcc        = OAMrm_BCC;

   //Install IE: First Channel Description, after time
   pUmHoCmd->firstChannelDescription.ie_present       = RM_TRUE; 
   pUmHoCmd->firstChannelDescription.hopping          = RM_FALSE; 
   pUmHoCmd->firstChannelDescription.channelType      = pPhyChan->chanType;
   pUmHoCmd->firstChannelDescription.subChannelNumber = pPhyChan->subChanNumber;
   pUmHoCmd->firstChannelDescription.TN               = (pPhyChan->chanNumberMSB)&RM_U8_0X07;
   pUmHoCmd->firstChannelDescription.TSC              = OAMrm_BTS_TSC; //pPhyChan->TSC;
   pUmHoCmd->firstChannelDescription.arfcn            = OAMrm_ARFCN(pPhyChan->trxNumber);

   //Install IE: Handover Reference
   pUmHoCmd->handoverReference.ie_present = RM_TRUE;
   pUmHoCmd->handoverReference.handoverReference = pPhyChan->hoRef; 
	      
   //Install IE: Power Command and Access Type
   //HOADDED
   //acs and power level may not be fixed in the future
   pUmHoCmd->powerCommandAndAccess.ie_present = RM_TRUE;
   pUmHoCmd->powerCommandAndAccess.acs = CNI_RIL3_SENDING_HANDOVER_ACCESS_MANDATORY;
   pUmHoCmd->powerCommandAndAccess.powerLevel = OAMrm_MS_TX_PWR_MAX_CCH; 

   //Optional IE: Synchronization Indication
   pUmHoCmd->syncIndication.ie_present = RM_TRUE;
   pUmHoCmd->syncIndication.nci        = CNI_RIL3_OUT_OF_RANGE_TIMING_ADVANCE_IGNORED;
   pUmHoCmd->syncIndication.rot        = CNI_RIL3_MOBILE_TIME_DIFFERENCE_SHALL_NOT_INCLUDED;
   pUmHoCmd->syncIndication.si         = CNI_RIL3_SI_NON_SYNCHRONIZED;

   //All optional and conditional IEs ignored at this time. Note that the rm_UmMsg
   //already reset to 0 at the beginning, thus no such IEs should be encoded even
   //no RM_FALSE assigned to their ie_present field.

   //Call RR message encoding functionality
   rrEdRet = CNI_RIL3RRM_Encode(&rm_UmMsg, &l3_data);

   //check if encoding of the message successful or not
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	  //Send out the message to MM 
        RM_MEMCPY((u8*)&(msgToMm.l3_data.handReqAck.handCmd),(u8*)&l3_data,
                  sizeof(rm_L3Data_t)); 
        //TDUMP__((MAREA__,(unsigned char *)&(msgToMm.l3_data.handCmd.handCmd),40));
        api_SendMsgFromRmToMm(sizeof(rm_ItcTxL3Msg_t), (u8*)&msgToMm);
        break;

   default:
	  //Encoding failed
        EDEBUG__(("ERROR-rm_SendHoReqAck: RIL3-RR encode error:%d\n",
		      rrEdRet ));
   	  return RM_FALSE;

   } //End of switch(rrEdRet)
  
   return RM_TRUE;

} //End of rm_SendHoReqAck()

#endif //__RM_SENDHOREQACK_CPP__
