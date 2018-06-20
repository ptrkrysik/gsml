/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_TIMEREXPIRED_CPP__
#define __RM_TIMEREXPIRED_CPP__

#include "rm\rm_head.h"

int rm_TimerExpired( int param)
{   
    STATUS ret;

    /*
    ** "param" is of four bytes which stores the entryId of a SM. When
    ** a timer expires, the "param" is used to distinguish from ons SM
    ** another.
    */

    rm_ItcRxRmMsg_t timerMsg;

    timerMsg.module_id      = MODULE_RM;
    timerMsg.primitive_type = RM_TIMER_IND;
    timerMsg.entry_id       = (u8)( (param>>8) & 0x000000FF );
    timerMsg.timerId	    = (u8)( param & 0x000000FF );

    /* Send a Timer Expiry Indication to RM inbound message queue */	
    ret = msgQSend( rm_MsgQId, (s8 *) &timerMsg, RM_MAX_TXQMSG_LENGTH,
		    NO_WAIT, MSG_PRI_NORMAL );

    if (ret == ERROR)
        EDEBUG__(("ERROR-rm_TimerExpired: msgQSend errno:%d\n", errno));

    return 0;
}

#endif /* __RM_TIMEREXPIRED_CPP__ */
