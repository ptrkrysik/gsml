/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef  __RM_SENDPAGINGREQUESTTYPE1_CPP__
#define  __RM_SENDPAGINGREQUESTTYPE1_CPP__

#include "rm\rm_head.h"

void rm_SendPagingRequestType1(void)
{
   u8		         pagingGroup;   /* Paging group got from IMSI */
   u16		   length;	      /* Length of a msg sent to L1 */
   rm_EdRet_t        rrEdRet;	      /* Return code from RR decode */
   rm_L3Data_t       l3_data;       /* Encoded Paging Req Type 1  */
   rm_ItcTxMsg_t     msgToL1;	      /* Send PH_DATA_REQ msg to L1 */
   rm_MmPaging_t    *pMmPaging;     /* Paging message got from MM */
   rm_UmPageType1_t *pUmPaging;     /* Data for encoding PageReq1 */
   u16	         len_save;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendPagingRequestType1\n"));

   /* TBD:MM paging format */
   pMmPaging = (rm_MmPaging_t    *) &(rm_pItcRxMmMsg->l3_data.pageReq);
   pUmPaging = (rm_UmPageType1_t *) &(rm_UmMsg.pagingRequestType1);

   /*
   ** Populate the contents so that encoder knows what to encode but
   ** before we start populating the message content, zap it clean
   ** first.
   */
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   /* Header--PD,MT,SI */
   pUmPaging->header.protocol_descriminator = RM_PD;
   pUmPaging->header.si_ti		    = RM_SI;
   pUmPaging->header.message_type	    = RM_PAGING_REQUEST_TYPE_1;

   /* IE--Page Mode */        
   pUmPaging->pageMode.ie_present	    = RM_TRUE;
   pUmPaging->pageMode.pm	 	    = RM_NORMAL_PAGING;

   /* IE--Page Channel Needed */
   pUmPaging->channelNeeded.ie_present       = RM_TRUE; 
   pUmPaging->channelNeeded.numChannels      = 1;
   pUmPaging->channelNeeded.channelNeeded[0] = CNI_RIL3_CHANNEL_NEEDED_ANY_CHANNEL;
								//RM_CHANNEEDED_TCHF;

   /* IE--Mobile Identity */
   pUmPaging->mobileId1.ie_present   = RM_TRUE;
   pUmPaging->mobileId1.mobileIdType = pMmPaging->imsi.mobileIdType;
   pUmPaging->mobileId1.numDigits    = pMmPaging->imsi.numDigits;
   RM_MEMCPY( pUmPaging->mobileId1.digits, pMmPaging->imsi.digits,
	        pMmPaging->imsi.numDigits );

   //G2
   pUmPaging->mobileId2.ie_present   = RM_FALSE;
	      
   // Bhava Nelakanti - Mandatory IE missing Fix
   /* IE--P1 Rest Octets */
   pUmPaging->p1RestOctets.ie_present = RM_TRUE;
   pUmPaging->p1RestOctets.nchi	  = CNI_RIL3_REDUCED_NCH_MONITERING_NOT_USED;

   //G2
   //NLN(PCH) field
   pUmPaging->p1RestOctets.flag_nln = RM_FALSE;
   pUmPaging->p1RestOctets.nln = 0;
   pUmPaging->p1RestOctets.nln_status = 0;

   //Priority_1 field
   pUmPaging->p1RestOctets.flag_p1 = RM_FALSE;
   pUmPaging->p1RestOctets.priority_1 = 0;

   //Priority_2 field
   pUmPaging->p1RestOctets.flag_p2 = RM_FALSE;
   pUmPaging->p1RestOctets.priority_2 = 0;

   //group call not supported, encoding will be marked as false 

   //packet_page_indication (1 or 2) should be set to true is it's packet paging procedure.
   //It should be set to false, if it is paging procedure for RR connection establishment.
   pUmPaging->p1RestOctets.packet_page_indication_1 = (rm_ItcRxOrigin==MODULE_BSSGP)?RM_TRUE:RM_FALSE;
   pUmPaging->p1RestOctets.packet_page_indication_2 = RM_FALSE;	

   /* 
   ** Call RR message encoding functionality
   */
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   /* check the encoding result before sending the message */
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	
	/* Wrap the encoded Paging message in PH_DATA_REQ */
        pagingGroup = rm_SolvePagingGroup(&(pMmPaging->imsi));
        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_LNKMGMT;
        msgToL1.buffer[length++] = RM_L1MT_PHDATAREQ_MSB;
        msgToL1.buffer[length++] = RM_L1MT_PHDATAREQ_LSB;
	  msgToL1.buffer[length++] = OAMrm_BCCH_TRX;
        msgToL1.buffer[length++] = RM_L1PCH_CHANNUMBER_MSB;
//len_save = length;
        msgToL1.buffer[length++] = pagingGroup;
	  msgToL1.buffer[length++] = RM_L1LINK_VOID;
	  RM_MEMCPY(&msgToL1.buffer[length], l3_data.buffer, l3_data.msgLength);
        msgToL1.msgLength   = length + l3_data.msgLength;

	/* Send PH_DATA_REQ to L1 */
//msgToL1.buffer[len_save] = 0;
//        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
//msgToL1.buffer[len_save] = 1;
//        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
//msgToL1.buffer[len_save] = 2;
        PostL3SendMsLog ( &l3_data );
        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
       
	break;

   default:
	/* Encoding failed */
        EDEBUG__(("ERROR-rm_SendPagingRequestType1: RIL3-RR encode error:%d\n",
		  rrEdRet ));
   	break;

   } /* End of switch(rrEdRet) */

} /* End of rm_SendPagingRequestType1() */

#endif /* __RM_SENDPAGINGREQUESTTYPE1_CPP__ */
