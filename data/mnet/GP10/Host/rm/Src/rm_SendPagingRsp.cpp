/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef  __RM_SENDPAGINGRSP_CPP__
#define  __RM_SENDPAGINGRSP_CPP__

#include "rm\rm_head.h"

void rm_SendPagingRsp(void)
{
   rm_EdRet_t       rrEdRet;	       //Return code from RR decode
   rm_ItcTxL3Msg_t  msgToMm;	       //Send inter MM-Rm siganling
   rm_MmPagingRsp_t *pMmPagingRsp;   //Paging message got from MM
   rm_UmPagingRsp_t *pUmPagingRsp;   //Data for encoding PageReq1

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_SendPagingRsp\n"));

   //Decode the DL_EST_IND-carried data i.e.Um Paging Rsp message
   //PR1104
   //rrEdRet = CNI_RIL3RRM_Decode(&(rm_pItcRxMdMsg->l3_data),&rm_UmMsg);
   rrEdRet = rm_MsgAnalyze();

   if ( rrEdRet != RM_RRED_SUCCESS )
   {
	  EDEBUG__(("ERROR@rm_SendPagingRsp:LAPDm l3_data decoded failed\n"));
        return;
   }

   //Decoding successful. Further check if it's expected message
   if ( rm_UmMsg.header.protocol_descriminator != RM_PD              ||
        rm_UmMsg.header.message_type	     != RM_PAGING_RESPONSE )
   {
        IDEBUG__(("WARNING@rm_SendPagingRsp: RR msg PD:%d;MT:%d",
		       rm_UmMsg.header.protocol_descriminator, 
		       rm_UmMsg.header.message_type ));
        //PR1104
        rm_SendRrStatus(CNI_RIL3_RR_MESSAGE_TYPE_NOT_COMPATIBLE);
        return;
   }

   pUmPagingRsp = (rm_UmPagingRsp_t *) &(rm_UmMsg.pagingResponse);

   //Encode MM Paging Response message
   msgToMm.module_id      = MODULE_RM;
   msgToMm.entry_id	  = rm_ItcRxEntryId;
   msgToMm.primitive_type = RR_EST_CNF;
   msgToMm.message_type   = MM_PAGE_RSP;

   pMmPagingRsp = (rm_MmPagingRsp_t *) &(msgToMm.l3_data.pageRsp);

   //CC needs the whole paging rsp from LAPDm
   RM_MEMCPY( pMmPagingRsp, pUmPagingRsp, sizeof(rm_UmPagingRsp_t) );

   //Furnish the Paging Rsp message with required IEs data
   pMmPagingRsp->mobileId.ie_present   = pUmPagingRsp->mobileId.ie_present;
   pMmPagingRsp->mobileId.mobileIdType = pUmPagingRsp->mobileId.mobileIdType;
   pMmPagingRsp->mobileId.numDigits    = pUmPagingRsp->mobileId.numDigits;

   switch(pUmPagingRsp->mobileId.mobileIdType)
   {
   case RM_IMSI: 
 	  RM_MEMCPY( pMmPagingRsp->mobileId.digits,pUmPagingRsp->mobileId.digits,
                   pUmPagingRsp->mobileId.numDigits );
        break;
                    
   default:
        //Right now only handle IMSI, later may handle TMSI
        IDEBUG__(( "WARNING@rm_SendPagingRsp: unexpected mobile Id:%d\n", 
		        pUmPagingRsp->mobileId.mobileIdType ));
        return;
        break;

   } //End of switch()

   //Send the complete MM Paging Response message out
   api_SendMsgFromRmToMm( sizeof(rm_ItcTxL3Msg_t), (u8*) &msgToMm);

} //End of rm_SendPagingRsp()

#endif //__RM_SENDPAGINGRSP_CPP__
