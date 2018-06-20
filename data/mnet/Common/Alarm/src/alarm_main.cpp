/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : AlarmTask.h                                         */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 9/18/2000                                           */
/* Description : This file contains main routines for the alarm task */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |09/28/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#include "AlarmTask.h"
#include "ctype.h"

#if defined( _TCP_SERVER_EXIST)
#include "TcpServer/TcpSrvApi.h"
#endif

AlarmTask * AlarmTask::theAlarmTask = NULL;

bool AlarmTask::isOneCreated = false;

AlarmTask::~AlarmTask()
{
    alarm_fenter("AlarmTask::~AlarmTask");
    alarm_debug(("Alarm Task instance is being deleted"));
    delete jcMsgQ;
    delete initTimer;
    alarm_fexit();
}

int AlarmTask::entryPoint()
{

    return theAlarmTask->startup();

}

int  AlarmTask::startup()
{
    int status;
    unsigned int msgType;
    MNET_MODULE_ID senderId;
    unsigned int recvMsgsize;

    
    alarm_fenter("alarmTask");

    // Rename the old alarm file
    // alarm_renameOldLogFile();

#if defined( _TCP_SERVER_EXIST)

    // Register TCP server call back function
     TcpSrvRegHand(kAlarmGpName,       // Client Group Name (<= kClientGpNameMax)
                   alarm_clientMsg);     // Function pointer to be called
                  
#endif

    /* now go into the message loop */
    JCTaskEnterLoop();
    _quit = false;

    while(_quit == false)
    {
        
        /* get message from queue; if necessary wait for a maximum of ALARM_q_timeout ticks */
        status = jcMsgQ->JCMsgQReceive(&jcMsgQ,  &msgType, &senderId, _msgRecvBuff, &recvMsgsize, ALARM_q_timeout);
        if (status == ERROR)
        {
            if (errno == S_objLib_OBJ_TIMEOUT)
            {

                /* Send heartbeat message to AMS in case it was down and now it has been up */
                sendSummaryUpdateToAms();
                continue;
            }
            
        }
        else
        {

            alarm_processMsg(_msgRecvBuff);
            sendSummaryUpdateToAms();
        }
    }
    
    JCTaskNormExit();

    closeSocket();

    isOneCreated = false;
    //delete theAlarmTask;
    theAlarmTask = NULL;    

    /* The application need to unsunscribe trap before exiting. */
    alarm_debug(("Alarm Task is exiting ... \n"));
    alarm_fexit();
    return OK;
}


/* print alarm messages */
void alarm_print(char *format, ...) 
{
    va_list marker;
    va_start( marker, format );     /* Initialize variable arguments. */
    vprintf(format, marker);
    fflush(stdout);
    va_end( marker );              /* Reset variable arguments. */
    return;
    
}

#ifdef _UT
void HexDumper(ubyte1 *buffer, int length )
/*++  Purpose:
Put the inbound data in a hex dump format  
Arguments:          
buffer - points to the extension control block     
length - specifies the number of bytes to dump --*/ 
{     
    int size;     
    int i;     
    int dwPos = 0;      
    while (length > 0) { 
        //         
        // Take min of 16 or length         
        //                  
        size = min(16, length );          
        //         
        // Build text line        
        //                  
        printf("  %04X ", dwPos );          
        for (i = 0; i < size; i++) 
        {             
            printf("%02X ", buffer[i] );             
        }          
        //         
        // Add spaces for short lines         
        //                  
        while (i < 16) 
        {             
            printf("   " );             
            i++;         
        }          
        //         
        // Add ASCII chars         
        //                  
        for (i = 0; i < size; i++) 
        {             
            if (isprint(buffer[i])) 
            {                 
                printf("%c", buffer[i] );                 
            } else {                 
                printf("." );             
            }         
        }          
        
        printf("\n");
        //         
        // Advance positions         
        //                  
        length -= size;         
        dwPos += size;         
        buffer += size;     
    }
}
#endif  /* _UT */

