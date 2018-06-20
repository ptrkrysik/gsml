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
// File        : MsgQTest.cpp
// Author(s)   : Tim Olson
// Create Date : 10/18/2000
// Description :  
//
// *******************************************************************


#include <stdio.h>
#include "Os/JCMsgQueue.h"

JCMsgQueue *pMsgQ;
bool isTestComplete = FALSE;

extern "C"
{
void InitializeQueue()
{
    pMsgQ = new JCMsgQueue(10, 256, JC_MSG_Q_FIFO);
    
    if (pMsgQ->JCGetMsgQId() == 0)
    {
        printf("Error creating test message queue\n");
        return;
    }
    
    JC_MSG_Q_ID replyMsgQ;
    unsigned int msgType;
    char buffer[256];
    unsigned int nBytes;
    MNET_MODULE_ID modId;
    
    while (!isTestComplete)
    {
        JC_STATUS status;
        if ((status = pMsgQ->JCMsgQReceive(&replyMsgQ, &msgType, &modId, buffer, &nBytes, 
            JC_WAIT_FOREVER)) == JC_OK)
        {
            printf("Received a test message!\n");
            printf("\treplyMsgQ = %x\n", replyMsgQ);
            printf("\tmsgType = %x\n", msgType);
            printf("\tmodId = %x\n", modId);
            printf("\tnBytes = %x\n", nBytes);
            printf("\tbuffer = ");
            for (int i=0; i < nBytes; i++)
                printf("%02x ", buffer[i]);
            printf("\n");    
        }
        else
        {
            printf("Error receiving message - errcode %x\n", status);
        }   
    }
}



void SendToQueue()
{
    char msg[256];
    JC_STATUS status;
    
    // build a test message
    for (int i=0; i < 16; i++)
    {
        msg[i] = (char)i;
    }
    
    isTestComplete = TRUE;
    unsigned int msgType = 13;
    unsigned int msgLen = 16;
    
    if ((status = pMsgQ->JCMsgQSend((JC_MSG_Q_ID)0xabcd0123, msgType, LOGGER, msg, msgLen, JC_WAIT_FOREVER, 
        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        printf("Error sending test message - errcode %x\n", status);
    }
}
    
}        