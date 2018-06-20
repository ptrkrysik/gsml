// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : JCMsgQueue.h
// Author(s)   : Tim Olson
// Create Date : 9/18/2000
// Description : 
//
// *******************************************************************
#ifndef _JCMSGQUEUE_H_
#define _JCMSGQUEUE_H_  /* include once only */

#include "JCErr.h"
#include "msgQLib.h"
#include "MnetModuleId.h"

/* OS independent message queue id */
#define  JC_MSG_Q_ID                MSG_Q_ID

/* Message queue sending options */
#define  JC_MSG_PRI_NORMAL          MSG_PRI_NORMAL
#define  JC_MSG_PRI_URGENT          MSG_PRI_URGENT

/* Message queue creation options */
#define  JC_MSG_Q_FIFO              MSG_Q_FIFO
#define  JC_MSG_Q_PRIORITY          MSG_Q_PRIORITY

/* Message queue reception timeout options */
#define  JC_NO_WAIT                 NO_WAIT
#define  JC_WAIT_FOREVER            WAIT_FOREVER

class JCMsgQueue {
public:
    JCMsgQueue
    (    
        int  maxMsgs,       /* max msgs that can be queued */
        int  maxMsgLength,  /* max bytes in a msg   */
        int  options        /* message queue options */
    );

    ~JCMsgQueue();

    JC_STATUS JCMsgQSend
    (
        JCMsgQueue *replyMsgQ,
        unsigned int msgType,
        MNET_MODULE_ID modId,
        char * buffer,
        unsigned int nBytes,
        int timeout,
        int priority
    );

    int JCMsgQReceive
    (
        JCMsgQueue **replyMsgQ,
        unsigned int *msgType,
        MNET_MODULE_ID *modId, 
        char * buffer,
        unsigned int *nBytes,
        int timeout
    );

    JC_MSG_Q_ID JCGetMsgQId()
    {
        return(msgQId);
    }

private:
    JC_MSG_Q_ID     msgQId;
    int             msgQMaxLen;
    char            *msgQSndBuf;
    char            *msgQRcvBuf;

    // Disallow the following functions
    JCMsgQueue();
    JCMsgQueue(const JCMsgQueue&);
    JCMsgQueue& operator=(const JCMsgQueue&);
    int operator== (const JCMsgQueue&) const;
};


#endif
