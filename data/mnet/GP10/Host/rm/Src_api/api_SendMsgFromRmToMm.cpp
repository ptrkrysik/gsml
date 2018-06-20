/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
  */

#ifndef __API_SENDMSGFROMRMTOMM_CPP__
#define __API_SENDMSGFROMRMTOMM_CPP__

#include "rm\rm_head.h"

void api_SendMsgFromRmToMm (
		unsigned short 	length,
		unsigned char	*msg
		   )
{
     STATUS	result; 

//     TDUMP__(( TXMSG__, (unsigned char *)msg,length ));

     result = msgQSend( 
			 mmMsgQId, 
			 (char *) msg,
			 length,
			 NO_WAIT,
			 MSG_PRI_NORMAL 
		      );

     if ( result==ERROR )
	  EDEBUG__(("ERROR-api_SendMsgFromRmToMm:VxWorks sending error:%d\n",
		   errno));

rm_ItcTxL3Msg_t  *pMmMsg;
pMmMsg = (rm_ItcTxL3Msg_t *)msg;

PDEBUG__(("RM sends MSG to MM: moduleID:%d, entryId:%d, prim:%d,msgType:%d\n",
        pMmMsg->module_id,
        pMmMsg->entry_id,
        pMmMsg->primitive_type,
        pMmMsg->message_type)); 
}

#endif /* __API_SENDMSGFROMRMTOMM_CPP__ */
