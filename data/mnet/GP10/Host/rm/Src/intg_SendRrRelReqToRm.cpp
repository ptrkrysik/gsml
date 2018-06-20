/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDRRRELREQ_CPP__
#define __INTG_SENDTCHRELREQTORM_CPP__

#include "rm\rm_head.h"
//#include "stdio.h"
//#include "vxWorks.h"
//#include "msgQlib.h"
//#include "jcc\sys_const.h"
//#include "JCCJCCL3Msg.h"
//#include "rm\rm_data.h"
//#include "debug.h"

extern   MSG_Q_ID rm_MsgQId;

void intg_SendTchRelReqToRm(void);

void intg_SendTchRelReqToRm(void)
{
    u8 		i;
    STATUS 		result;
    IntraL3Msg_t	msg;
  
    msg.module_id      = MODULE_MM;
    msg.primitive_type = INTRA_L3_RR_REL_REQ;

    for (i=0;i<15;i++)
    {
    	   msg.entry_id = (i<<1)+1;
         result = msgQSend( 
	                  	 rm_MsgQId, 
					 (char *) &msg, 
					 sizeof(IntraL3Msg_t),
					 NO_WAIT,
					 MSG_PRI_NORMAL 
	            	  );
	   if ( result==ERROR )
		  EDEBUG__(("ERROR-intg_SentRrRelReqToRm: VxWorks sending error:%d\n",
		 		 errno));
    }
}
     
#endif /* __INTG_SENDTCHRELREQTORM_CPP__ */
