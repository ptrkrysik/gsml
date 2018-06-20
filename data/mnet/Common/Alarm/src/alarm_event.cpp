
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
/* Create Date : 07/11/99                                            */
/* Description : contains alarm module message that interact with    */
/*               other module               .                        */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |07/11/99| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/*----------+--------+-----------------------------------------------*/

#ifndef _ALARM_EVENT_CPP_
#define _ALARM_EVENT_CPP_

#include "AlarmTask.h"

void  alarm_clientMsg ( char * message,   // the message from the client
                        short size,         // size of the message
                        unsigned long clientId // unique client ID of the requester client 
                       )
{
    
    alarm_fenter("alarm_clientMsg");
    alarm_debug(("Message receive from client (ID = %d)", clientId));
    alarm_debug(("%s\n", message));
    alarm_fexit();
}


void alarm_broadcast(const char *msg)
{
    int len;
    alarm_fenter("alarm_broadcast");
    fflush(stdout);

    len = strlen(msg);
    //TcpSrvSendResponse(msg, len, kAlarmGpName);
    alarm_debug(("Alarm message was sent\n")); 
    alarm_fexit();
}

JC_STATUS AlarmTask::msgqSend(unsigned int msgType, char * buffer, unsigned int nBytes)
{
    return jcMsgQ->JCMsgQSend(this->jcMsgQ, msgType, MY_MODULE_ID, buffer, nBytes, JC_NO_WAIT, JC_MSG_Q_FIFO);
}


JC_STATUS AlarmTask::alarmSend
      (
         InterModuleAlarmMsgType_t mtype,
         MNET_MODULE_ID             mid, 
         MibTag                     moduleErrorTag, 
         AlarmCode_t                alarm_code,
         char                       *arg1,
         char                       *arg2
      )
{
    InterModuleAlarmMsg_t alarmMsg;
    
    memset(&alarmMsg, 0, sizeof(InterModuleAlarmMsg_t));

    alarmMsg.errorTag = moduleErrorTag;
    alarmMsg.code = alarm_code;
    alarmMsg.senderId = (ubyte1) MY_MODULE_ID;
    alarmMsg.msgType = mtype;
    alarmMsg.srcModuleId = mid;
    
    if (NULL != arg1)
    {
       strncpy(alarmMsg.arg1, arg1, ALARM_optionalArgSize-1);
    }

    if (NULL != arg2)
    {
       strncpy(alarmMsg.arg2, arg2, ALARM_optionalArgSize-1);
    }
    
    return AlarmTask::theAlarmTask->msgqSend
        (InterModuleAlarmMsgTypeRaiseAlarm,
        (char *) &alarmMsg,
        sizeof(InterModuleAlarmMsg_t));
    
}

extern "C" JC_STATUS alarm_raise(MNET_MODULE_ID mid, MibTag moduleErrorTag, AlarmCode_t alarm_code)
{
    return AlarmTask::theAlarmTask->alarmSend(InterModuleAlarmMsgTypeRaiseAlarm, mid, moduleErrorTag, alarm_code);
    
}

extern "C" JC_STATUS alarm_raise_args(MNET_MODULE_ID mid, MibTag moduleErrorTag, AlarmCode_t alarm_code, int arg1, int arg2)
{
   char pszArg1[ALARM_optionalArgSize];
   char pszArg2[ALARM_optionalArgSize];
   sprintf (pszArg1, "%d", arg1);
   sprintf (pszArg2, "%d", arg2);

    return AlarmTask::theAlarmTask->alarmSend(InterModuleAlarmMsgTypeRaiseAlarm, mid, moduleErrorTag, alarm_code, pszArg1, pszArg2);
    
}

extern "C" JC_STATUS alarm_clear(MNET_MODULE_ID mid, MibTag moduleErrorTag, AlarmCode_t alarm_code)
{
    return AlarmTask::theAlarmTask->alarmSend(InterModuleAlarmMsgTypeClearAlarm, mid, moduleErrorTag, alarm_code);
}

EnableDisableState_t AlarmTask::getOperationState()
{
    long opState;
    oam_getMibIntVar(MY_MIB_OPERATION_STATE, &opState);
    return (EnableDisableState_t) opState;
}



void AlarmTask::setOperationState(EnableDisableState_t opstate )
{
    EnableDisableState_t state = getOperationState();
    
    if(state != opstate)
    {
        oam_setMibIntVar(MY_MODULE_ID, MY_MIB_OPERATION_STATE, opstate);
    }
}

    
EnableDisableState_t AlarmTask::getMibAcessControlState()
{
    long opState;
    oam_getMibIntVar(MY_MIB_ACCESS_CONTROL_TAG, &opState);
    return (EnableDisableState_t) opState;
}


void AlarmTask::setMibAcessControlState(EnableDisableState_t opstate )
{
    EnableDisableState_t state = getMibAcessControlState();
    alarm_fenter("AlarmTask::setMibAcessControlState");
    
    if(state != opstate)
    {
        oam_setMibIntVar(MY_MODULE_ID, MY_MIB_ACCESS_CONTROL_TAG, opstate);
    }
    
    // local mib status 
    _mibAccessState = (opstate == stateEnabled);
    
    if (_mibAccessState)
    {
        // This is the first time
        initTimer->cancelTimer();
        alarm_debug (("Mib access is now enabled\n"));
    }
    alarm_fexit();   
}

void AlarmTask::setModuleLastError(MibTag moduleErrorTag, AlarmCode_t alarm_code)
{
        oam_setMibIntVar(MY_MODULE_ID, moduleErrorTag, alarm_code);
}

void AlarmTask::populateSourceName()
{
    const int kDisplayStringSize = 256;
    char name[kDisplayStringSize];

    memset(name, 0, kDisplayStringSize);
    oam_getMibByteAry(MY_MIB_SOURCE_NAME, (ubyte1 *) name, kDisplayStringSize-1);
    _sourceName = new char[strlen(name+1)];
    strcpy(_sourceName, name);
}

char * AlarmTask::srcName()
{
    return theAlarmTask->_sourceName;
}


#endif /* _ALARM_EVENT_CPP_ */