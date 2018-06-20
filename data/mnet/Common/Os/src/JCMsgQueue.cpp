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
// File        : JCMsgQueue.cpp
// Author(s)   : Tim Olson
// Create Date : 10/18/2000
// Description :
//
// *******************************************************************
//
//  Revision history:
// ===================================================================
// Igal       | 11/30/00 | Fixed incorrect max message size acceptable by the queue
// -------------------------------------------------------------------
//
//

#include <stdio.h>
#include <errNoLib.h>
#include <string.h>
#include "Os/JCMsgQueue.h"
#include "JCMsgQDefs.h"

#define PADDING         4

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCMsgQueue constructor
**
**    PURPOSE: Initialize data members for JCMsgQueue object.  A message
**      queue for the native operating system will be created.
**
**    INPUT PARAMETERS: maxMsgs - max msgs that can be queued
**                      maxMsgLength - max bytes in a msg
**                      options - message queue options (MSG_Q_FIFO or
**                          MSG_Q_PRIORITY)
**
**    RETURN VALUE(S): none
**      Note: If the message queue cannot be created msgQId will be set to NULL
**
**----------------------------------------------------------------------------*/
JCMsgQueue::JCMsgQueue
(
    int  maxMsgs,       /* max msgs that can be queued */
    int  maxMsgLength,  /* max bytes in a msg   */
    int  options        /* message queue options */
) : msgQMaxLen(maxMsgLength+PADDING)
{
    if (!(msgQId = msgQCreate(maxMsgs, msgQMaxLen + JC_MSG_HDR_SIZE, options)))
    {
        printf("Error creating message queue %s\n", errnoGet());
    }

    if (!(msgQSndBuf = new char[msgQMaxLen + JC_MSG_HDR_SIZE]))
    {
        printf("Error: unable to allocate msg buffer memory\n");
    }

    if (!(msgQRcvBuf = new char[msgQMaxLen + JC_MSG_HDR_SIZE]))
    {
        printf("Error: unable to allocate msg buffer memory\n");
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCMsgQueue destructor
**
**    PURPOSE: Delete data members for JCMsgQueue object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
JCMsgQueue::~JCMsgQueue ()
{
    if (msgQDelete(msgQId) != OK)
    {
        printf("Error deleting message queue %s\n", errnoGet());
    }

    delete [] msgQSndBuf;
    delete [] msgQRcvBuf;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCMsgQueue::JCMsgQSend
**
**    PURPOSE: JCMsgQSend sends the specified message to the specified message
**      queue.  The buffer given will be copied into a local send buffer prior
**      to sending the message.
**
**    INPUT PARAMETERS: replyMsgQ - message queue to use for sending a reply
**                      msgType - type of message being sent
**                      modId - module ID of sender
**                      buffer - message to send
**                      nBytes - number of bytes in the message
**                      timeout - ticks to wait
**                      priority - priority of the message (normal or urgent)
**
**    RETURN VALUE(S): JC_STATUS
**
**----------------------------------------------------------------------------*/
JC_STATUS JCMsgQueue::JCMsgQSend
(
    JCMsgQueue *replyMsgQ,
    unsigned int msgType,
    MNET_MODULE_ID modId,
    char * buffer,
    unsigned int nBytes,
    int timeout,
    int priority
)
{
    // Verify a few parameters first.
    if (nBytes && !buffer)
        return JC_PARAM_INVALID;

    if (nBytes > msgQMaxLen-PADDING)
        return JC_MSG_LENGTH_ERROR;

    // Build message in the receive message buffer for this queue.
    JC_MSG_HDR *pMsg = (JC_MSG_HDR *)msgQSndBuf;

    pMsg->rplyQ   = replyMsgQ;
    pMsg->msgType = msgType  ;
    pMsg->modId   = modId    ;
    pMsg->bytes   = nBytes   ;
    if (buffer)
        memcpy (msgQSndBuf + JC_MSG_HDR_SIZE, buffer, nBytes);

    return (msgQSend(msgQId, msgQSndBuf, nBytes + JC_MSG_HDR_SIZE, timeout, priority));
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCMsgQueue::JCMsgQReceive
**
**    PURPOSE: JCMsgQReceive.
**
**    INPUT PARAMETERS: replyMsgQ - message queue to use for sending a reply
**                      msgType - type of message being sent
**                      modId - module ID of sender
**                      buffer - message to send
**                      nBytes - number of bytes in the message
**                      timeout - ticks to wait
**
**    RETURN VALUE(S): number of bytes received
**
**----------------------------------------------------------------------------*/
int JCMsgQueue::JCMsgQReceive
(
    JCMsgQueue **replyMsgQ,
    unsigned int *msgType,
    MNET_MODULE_ID *modId,
    char * buffer,
    unsigned int *nBytes,
    int timeout
)
{
    int bytesRcvd;

    // Verify a few parameters first.
    if ((!buffer) || (!replyMsgQ) || (!msgType) || (!nBytes))
        return JC_PARAM_INVALID;


    // Wait here for a message to arrive or timeout number of ticks to expire.
    if ((bytesRcvd = msgQReceive(msgQId, msgQRcvBuf, msgQMaxLen + JC_MSG_HDR_SIZE,
        timeout)) != ERROR)
    {
        // Access the message header at the start of the buffer.
        JC_MSG_HDR *pMsg = (JC_MSG_HDR *)msgQRcvBuf;

        *replyMsgQ = pMsg->rplyQ;
        *msgType = pMsg->msgType;
        *modId = pMsg->modId;
        *nBytes =  pMsg->bytes;

        // Message contents should follow the message header.
        memcpy(buffer, msgQRcvBuf + JC_MSG_HDR_SIZE, bytesRcvd - JC_MSG_HDR_SIZE);
    }

    return (bytesRcvd);
}
