/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_SENDCHANMODEMODIFY_CPP__
#define __RM_SENDCHANMODEMODIFY_CPP__

#include "rm\rm_head.h"


void rm_SendChanModeModify(rm_SmCtxt_t *pSmCtxt )
{
   rm_EdRet_t       rrEdRet;	     /* Return code from RR decode */
   rm_L3Data_t      l3_data; 	     /* Encoded Paging Req Type 1  */
   rm_UmChanModeModify_t *pUmChanModeModify;   /* Data for encoding PageReq1 */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendChanModeModify\n"));

   /* Check if make sense to send this message to MS */
   if ( pSmCtxt->smType   != RM_MAIN_SM        ||
        pSmCtxt->sapi     != RM_SAPI0          ||
        pSmCtxt->chanType != RM_CTFACCH_F      )
   {
	EDEBUG__(( "ERROR-rm_SendChanModeModify:smType:%d, sapi:%d, chan:%d\n",
                   pSmCtxt->smType, pSmCtxt->sapi,pSmCtxt->chanType ));
	return;
   }

   /* Choose Imm Assign message structure */
   pUmChanModeModify = (rm_UmChanModeModify_t *) &(rm_UmMsg.channelModeModify);

   /*
   ** Populate the contents so that encoder knows what to encode but
   ** before we start populating the message content, zap it clean
   ** first.
   */
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   /* Header--PD,MT,SI */
   pUmChanModeModify->header.protocol_descriminator = RM_PD;
   pUmChanModeModify->header.si_ti		    = RM_SI;
   pUmChanModeModify->header.message_type	    = RM_CHANNEL_MODE_MODIFY;

   /* IE--Channel Description  */
   pUmChanModeModify->channelDescription.ie_present  = RM_TRUE; 
   pUmChanModeModify->channelDescription.hopping     = RM_FALSE; 
   pUmChanModeModify->channelDescription.channelType = pSmCtxt->pPhyChan->chanType;
   pUmChanModeModify->channelDescription.subChannelNumber = 
						     pSmCtxt->pPhyChan->subChanNumber;
   pUmChanModeModify->channelDescription.TN    = (pSmCtxt->pPhyChan->chanNumberMSB)&RM_U8_0X07;
   pUmChanModeModify->channelDescription.TSC   = OAMrm_BTS_TSC; //rm_pSmCtxt->pPhyChan->TSC;
   pUmChanModeModify->channelDescription.arfcn = OAMrm_ARFCN(pSmCtxt->pPhyChan->trxNumber);


   /* IE--Channel Mode */
   pUmChanModeModify->channelMode.ie_present = RM_TRUE;
// pUmChanModeModify->channelMode.value      = RM_CHANMODESP_VER1;
// pUmChanModeModify->channelMode.value      = (T_CNI_RIL3_CHANNEL_MODE_VALUE)OAMrm_GSMCODEC;

   //EFRAdded
   pUmChanModeModify->channelMode.value      = (T_CNI_RIL3_CHANNEL_MODE_VALUE)pSmCtxt->codAlgo;
			
   /* TBD : Add other IE here if needed */
   
   /* 
   ** Call RR message encoding functionality
   */
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   /* check the encoding result before sending the message */
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	/* Wrap the encoded Paging message in DL_DATA_REQ */
//C_CHN        rm_SendDlDataReq ( pSmCtxt->mOId,pSmCtxt->sapi,RM_CTFACCH_F,&l3_data );
        PostL3SendMsLog( pSmCtxt->mOId,pSmCtxt->sapi,&l3_data );
        rm_SendDlDataReq ( pSmCtxt->mOId,pSmCtxt->sapi,&l3_data );

	break;

   default:
	/* Um message encoding failed */
        EDEBUG__(("ERROR-rm_SendChanModeModify: Um encode error:%d\n", rrEdRet ));
   	break;

   } /* End of switch(rrEdRet) */

} /* End of rm_SendChanModeModify() */

#endif /* __RM_SENDCHANMODEMODIFY_CPP__ */

