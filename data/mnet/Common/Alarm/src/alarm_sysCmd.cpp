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
/* Description : This file contains alarm task syscommand handling   */
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
/* Bhawani  |03/09/01| Changed alarm log directory to match others   */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#ifndef __ALARM_SYSCMD_CPP__
#define __ALARM_SYSCMD_CPP__

#include "AlarmTask.h"

/*
*********************************************************************
**  Global variables 
********************************************************************
*/

/* Initialize the alarm task */

AlarmTask::AlarmTask(char *name): JCTask(name)
{
    alarm_fenter("AlarmTask::AlarmTask");
    if (isOneCreated == false)
    {
        int size, i,j;

        // Initialize the message queue first
        jcMsgQ = new JCMsgQueue(ALARM_q_maxMsgs, ALARM_q_maxMsgLength,  MSG_Q_FIFO);

#ifdef MNET_PRODUCT_SPECIFIC_GP10
        // Initialize the timer
        initTimer = new JCCTimer( (FUNCPTR ) &AlarmTask::bootupTimeoutHandler);
        initTimer->setTimer(ALARM_mib_lock_timeout);
#else
        _mibAccessState = stateEnabled;
#endif
       
        // message receive buffer
        _msgRecvBuff = new char[ALARM_q_maxMsgLength];
        
        _mnetbase = getenv( "MNET_BASE" );

        // compute the log file size
        size = strlen(_mnetbase) + strlen(ALARM_logFile) + 1;
        _logfilename = new char[size];
        sprintf(_logfilename, "%s%s", _mnetbase, ALARM_logFile);

        // initialize the module status table
        for (i = 0; i < MNET_MAX_MODULE_IDS; i++)
            _ms[i] = NULL;

    
        // initialize the alarm pool
        _ap = new AlarmEntry_t[ALARM_max_outstanding];
        for (i =0; i <ALARM_max_outstanding; i++)
        {
            _ap[i].code = EC_COMMON_NOERROR;
            _ap[i].next = NULL;
        }

        for (i=0; i <2; i++)
            for (j=0; j< Alarm_max_severity; j++)
                _statistics[i][j] = 0;

        theAlarmTask =  this;
        populateSourceName();
        isOneCreated = true;
    }
    alarm_fexit();
}

int AlarmTask::bootupTimeoutHandler()
{
    alarm_fenter("alarm_bootupTimeoutHandler");

    /* The timer has expired and mib access needs to be enabled. */
    theAlarmTask->setMibAcessControlState(stateEnabled);
    alarm_warning(("The system did not complete reboot. Mib access has been re-enabled\n"));
    alarm_fexit();
    return 0;
}


int  SysCommand_Alarm (T_SYS_CMD action) 
{
    char *home;
    JC_STATUS	status = OK;
    
    alarm_fenter("SysCommand_Alarm");
    switch(action)
    {
    case SYS_SHUTDOWN:
        break;
        
    case SYS_REBOOT:
        alarm_warning(("Reboot notification has been received\n"));
        AlarmTask::theAlarmTask->alarmSend(InterModuleAlarmMsgTypeReboot, MY_MODULE_ID);
        break;
        
    case SYS_START:
        alarm_debug(("Alarm initialization has been invoked\n"));
        
        /* Get the value of the MNET_BASE environment variable. */
        home = getenv( "MNET_BASE" );
        
        if( home == NULL )
        {
            alarm_error(("Environment variable \"MNET_BASE\" must be defined!!\n"));
            alarm_fexit();
            return ERROR;
        }
        {
            AlarmTask *mytask = new AlarmTask(ALARM_task_name);

#ifdef MNET_PRODUCT_SPECIFIC_GP10            
            /* block the mib access */
            mytask->setMibAcessControlState(stateDisabled);
#endif

            status = mytask->JCTaskSpawn(
                ALARM_TASK_PRIORITY,
                ALARM_TASK_OPTION,
                ALARM_TASK_STACK_SIZE,
                (FUNCPTR) AlarmTask::entryPoint,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                MY_MODULE_ID, JC_CRITICAL_TASK);
            
            if(status == JC_ERROR)
            {
                alarm_error(("Alarm task spawn failure\n"));
                status = ERROR;
            }
            
        }        
        break;
        
    default:
        alarm_warning(("Unknown SysCommand: %d\n", action));
        status = ERROR;
        break;
    }
    alarm_fexit();
    return status;
    
}

#endif /* __ALARM_SYSCMD_CPP__ */


