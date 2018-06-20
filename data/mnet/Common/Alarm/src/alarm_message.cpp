
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
/* Description : This file contains alarm task message processing    */
/*               routines                                            */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |07/14/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/



#ifndef _ALARM_MESSAGE_CPP_
#define _ALARM_MESSAGE_CPP_

#include "AlarmTask.h"


void alarm_processTrap(TrapMsg *trapMsg)
{
	/* The Oam Message */
	MibTag tag = trapMsg->mibTag;
	int table = MIB_TBL(tag);
	int mid = MIB_ITM(tag) - 1; // need to do this

	alarm_fenter("alarm_processOamMsg");
	
    alarm_fexit();
    return;
}

void alarm_processAlarmMsg(InterModuleAlarmMsg_t * alarmMsg)
{
	alarm_fenter("alarm_processAlarmMsg");
    switch(alarmMsg->msgType)
    {

    case InterModuleAlarmMsgTypeClearAlarm:
        AlarmTask::theAlarmTask->delAlarmEntry(alarmMsg->srcModuleId, alarmMsg->errorTag, alarmMsg->code);
        break;

    case InterModuleAlarmMsgTypeRaiseAlarm:
        AlarmTask::theAlarmTask->addAlarmEntry(alarmMsg->srcModuleId, alarmMsg->errorTag, alarmMsg->code, alarmMsg->arg1, alarmMsg->arg2);
        break;

    case InterModuleAlarmMsgTypeShutdown:
        AlarmTask::theAlarmTask->quit();
        break;
    
    case InterModuleAlarmMsgTypeReboot:
        AlarmTask::theAlarmTask->quit();
        break;

    default:
        alarm_warning(("Not recognized alarm message type (message type = %d)", alarmMsg->msgType));
        break;
    }
	alarm_fexit();
    return;
}

void alarm_processMsg(char *inComingMsg)
{	
	/* process the trap message  here*/
		alarm_fenter("alarm_processMsg");
	alarm_debug(("A new message has just arrived.\n"));

	switch(inComingMsg[0])
	{
	case MNET_MODULE_SNMP:
		alarm_processTrap((TrapMsg *) inComingMsg);
		break;
    case MY_MODULE_ID:
        alarm_processAlarmMsg((InterModuleAlarmMsg_t *) inComingMsg);
        break;
	default:
		alarm_error(("Message from unsupported module (ID = %d)\n", inComingMsg[0]));
	}
	alarm_fexit();
}

#endif /* _ALARM_MESSAGE_CPP_ */