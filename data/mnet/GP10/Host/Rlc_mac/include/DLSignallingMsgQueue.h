// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLSignallingMsgQueue.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __DLSIGNALLINGMSGQUEUE_H__
#define __DLSIGNALLINGMSGQUEUE_H__

#include "GP10Err.h"
#include "msgL1.h"

#define MAX_DL_SIGNALLING_QUEUE_ENTRIES     5

// *******************************************************************
// class DLSignallingMsgQueue
//
// Description
//    
// *******************************************************************
class DLSignallingMsgQueue {
public:
    DLSignallingMsgQueue() : qHead(0), qTail(0), numEntries(0) {}
    ~DLSignallingMsgQueue() {}
    
    JC_STATUS QueueDLSignallingMsg
    (
        MsgPacchReq *l1Msg
    );
    
    MsgPacchReq *DequeueDLSignallingMsg();
    
    
private:
    MsgPacchReq     pacchMsg[MAX_DL_SIGNALLING_QUEUE_ENTRIES];
    int             qHead, qTail;
    int             numEntries;
};


#endif
