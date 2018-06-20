/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
  */

#ifndef __API_SENDMSGFROMRMTOCC_CPP__
#define __API_SENDMSGFROMRMTOCC_CPP__

#include "rm\rm_head.h"

void api_SendMsgFromRmToCc (
            MSG_Q_ID		QId,
		unsigned short 	length,
		unsigned char	*msg
		   )
{
     STATUS	result;

//     TDUMP__(( TXMSG__, (unsigned char *)msg,length ));

     result = msgQSend( 
			 QId, 
			 (char *) msg,
			 length,
			 NO_WAIT,
			 MSG_PRI_NORMAL 
		      );

     if ( result==ERROR )
	  EDEBUG__(("ERROR-api_SendMsgFromRmToMm:VxWorks sending error:%d\n",
		   errno));

rm_ItcTxL3Msg_t  *pCcMsg;
pCcMsg = (rm_ItcTxL3Msg_t *)msg;

PDEBUG__(("RM sends MSG to CC: ccQId:%d, moduleID:%d, entryId:%d, prim:%d,msgType:%d\n",
        QId,
        pCcMsg->module_id,
        pCcMsg->entry_id,
        pCcMsg->primitive_type,
        pCcMsg->message_type)); 
}

#endif /* __API_SENDMSGFROMRMTOCC_CPP__ */
