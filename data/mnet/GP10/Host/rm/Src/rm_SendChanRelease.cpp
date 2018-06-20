/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDCHANRELEASE_CPP__
#define __RM_SENDCHANRELEASE_CPP__

#include "rm\rm_head.h"

void rm_SendChanRelease(rm_IrtEntryId_t entryId)
{
   rm_EdRet_t       rrEdRet;	     /* Return code from RR decode */
   rm_L3Data_t      l3_data; 	     /* Encoded Paging Req Type 1  */
   rm_UmChanRel_t   *pUmChanRel;     /* Data for encoding ChanRel  */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendChanRelease:entryId=%d\n",entryId));

   if (rm_SmCtxt[entryId].smType != RM_MAIN_SM)
   {
       EDEBUG__(("ERROR-rm_SendChanRelease: entryId not for main SM\n"));
       return;
   }

   /* Choose Chan Release message structure */
   pUmChanRel = (rm_UmChanRel_t *) &(rm_UmMsg.channelRelease);

   /*
   ** Populate the contents so that encoder knows what to encode but
   ** before we start populating the message content, zap it clean
   ** first.
   */
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   /* Header--PD,MT,SI */
   pUmChanRel->header.protocol_descriminator = RM_PD;
   pUmChanRel->header.si_ti		     = RM_SI;
   pUmChanRel->header.message_type	     =  CNI_RIL3RRM_MSGID_CHANNEL_RELEASE;
//RM_CHANNEL_RELEASE;

   /* IE--RR Cause */        
   pUmChanRel->rrCause.ie_present = RM_TRUE;
   pUmChanRel->rrCause.causeValue = RM_NORMAL_EVENT;

   /* TBD : Add other IEs later like BA, Group Key */   

   /* 
   ** Call RR message encoding functionality
   */
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   /* check the encoding result before sending the message */
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	/* Send out the message through DL_DATA_REQ */
PDEBUG__(("rm_SendChanRelease: mOId=%d,entryId=%d,sapi=%d\n",
        rm_SmCtxt[entryId].mOId, entryId, rm_SmCtxt[entryId].sapi));
TDUMP__((MAREA__,(u8*)&(l3_data), 20));

//CT_CHN        rm_SendDlDataReq ( rm_SmCtxt[entryId].mOId, rm_SmCtxt[entryId].sapi,
//			   rm_SmCtxt[entryId].chanType,&l3_data );
        PostL3SendMsLog( rm_SmCtxt[entryId].mOId, rm_SmCtxt[entryId].sapi,&l3_data );
        rm_SendDlDataReq ( rm_SmCtxt[entryId].mOId, rm_SmCtxt[entryId].sapi,&l3_data );

	break;

   default:
	/* Um message encoding failed */
        EDEBUG__(("ERROR-rm_SendChanRelease: Um encode error:%d\n", rrEdRet ));
   	break;

   } /* End of switch(rrEdRet) */

} /* End of rm_SendChanRelease() */

#endif /* __RM_SENDCHANRELEASE_CPP__ */


