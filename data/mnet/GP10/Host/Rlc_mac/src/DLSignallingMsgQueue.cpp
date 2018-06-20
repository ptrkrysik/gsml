// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLSignallingMsgQueue.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "DLSignallingMsgQueue.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"


JC_STATUS DLSignallingMsgQueue::QueueDLSignallingMsg
(
    MsgPacchReq *l1Msg
)
{
    DBG_FUNC("DLSignallingMsgQueue::QueueDLSignallingMsg", RLC_MAC);
    DBG_ENTER();

    JC_STATUS status = JC_OK;
    
    if (numEntries < MAX_DL_SIGNALLING_QUEUE_ENTRIES)
    {
        qTail = ((qHead+1) % MAX_DL_SIGNALLING_QUEUE_ENTRIES);
        bcopy((char *)&pacchMsg[qTail], (char *)l1Msg, sizeof(MsgPacchReq));
        numEntries++;
    }
    else
    {
        status == JC_ERROR;
    }
    
    DBG_LEAVE();
    return (status);
}

MsgPacchReq *DLSignallingMsgQueue::DequeueDLSignallingMsg()
{
    DBG_FUNC("DLSignallingMsgQueue::DequeueDLSignallingMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgPacchReq *pL1Msg;
        
    if (qHead == qTail)
    {
        pL1Msg = 0;
    }
    else
    {
        pL1Msg = &pacchMsg[qTail];
        qHead = (++qHead % MAX_DL_SIGNALLING_QUEUE_ENTRIES);
        numEntries--;
    }
    
    DBG_LEAVE();
    return (pL1Msg);
}
