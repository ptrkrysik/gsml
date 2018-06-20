/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDHANDOVERCOMMAND_CPP__
#define __RM_SENDHANDOVERCOMMAND_CPP__

//HOADDED
#include "rm\rm_head.h"

void rm_SendHandoverCommand(void)
{
   RDEBUG__(("ENTER-rm_SendHandoverCommand\n"));

   /* Check if make sense to send this message to MS */
   if ( rm_pSmCtxt->smType   != RM_MAIN_SM        ||
        rm_pSmCtxt->sapi     != RM_SAPI0          ||
        rm_pSmCtxt->chanType != RM_CTFACCH_F      )
   {
	EDEBUG__(( "ERROR-rm_SendHandoverCommand:smType:%d, sapi:%d, chan:%d\n",
                   rm_pSmCtxt->smType, rm_pSmCtxt->sapi,rm_pSmCtxt->chanType ));
	return;
   }

   //Send Handover Command through DL_DATA_REQ 
TDUMP__((MAREA__,(unsigned char *)&(rm_pItcRxCcMsg->l3_data.handCmd.handCmd),40));

   PostL3SendMsLog( rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,&(rm_pItcRxCcMsg->l3_data.handCmd.handCmd) );
   rm_SendDlDataReq(rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,&(rm_pItcRxCcMsg->l3_data.handCmd.handCmd));
}

#endif /*__RM_SENDHANDOVERCOMMAND_CPP__*/

