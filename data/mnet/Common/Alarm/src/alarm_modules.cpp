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
/* Description : This file contains alarm task module handling       */
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

#ifndef _ALARM_MODULE_CPP_
#define _ALARM_MODULE_CPP_

#include "AlarmTask.h"
#include <time.h>

Alarm_severity_t getSeverity(AlarmCode_t acode)
{
    Alarm_severity_t severity = Alarm_info;
    int tmp = (acode & 0xFFFF);


    if (tmp < ALARM_CRITICAL_START_POINT)
    {
        severity = Alarm_info;
    } else if (tmp < ALARM_MAJOR_START_POINT) {
        severity =  Alarm_critical;
    } else if (tmp < ALARM_MINOR_START_POINT) {
        severity =  Alarm_major;
    } else if (tmp < ALARM_INFO_START_POINT) {
        severity = Alarm_minor;
    } else {
        severity = Alarm_info;
    }
    return severity;
}


int AlarmTask::generateNextId()
{
    alarm_fenter("AlarmTask::generateNextId");
    static unsigned long id = 0;
    
    if(id == 0)
    {
        getLastId( &id);
        alarm_debug(("Last Module ID = %d\n", id ));
    }
    alarm_fexit();
    return id++;
}


AlarmCode_t AlarmTask::getMdlStatus(MNET_MODULE_ID mid)
{
    if (mid >=0 && mid < MNET_MAX_MODULE_IDS)
    {
        if (_ms[mid])
            return (_ap[mid].code);
    }
    return EC_COMMON_NOERROR;
}

AlarmEntry_t *AlarmTask::allocEntry(MNET_MODULE_ID mid)
{
    int i = 0;
    
    while (  _ap[i].code != EC_COMMON_NOERROR)
    {
        if ( i == ALARM_max_outstanding)
            return NULL;
        i++;
    }

    
    _ap[i].next = NULL;
    if (mid  >=0 && mid < MNET_MAX_MODULE_IDS)
    {
        if (_ms[mid] == NULL)
        {
            _ms[mid] = &_ap[i];
        }
        else
        {
            AlarmEntry_t * tmpentry = _ms[mid];
            
            while ( tmpentry->next  != NULL)
                tmpentry = tmpentry->next;

            tmpentry->next = &_ap[i];
        }
        return &_ap[i];
    }
    return NULL;
}

EnableDisableState_t AlarmTask::updateOperationalState()
{
    int i = 0;
    EnableDisableState_t state = stateEnabled;

    while ((state == stateEnabled) && (i < MNET_MAX_MODULE_IDS))
    {
        AlarmEntry_t * tmpentry = _ms[i];
        while(tmpentry)
        {
            if (tmpentry->severity == Alarm_critical)
            {
                state = stateDisabled;
                break;
            }
            tmpentry = tmpentry->next;
        }
        i++;
    }
    if ( (state == stateEnabled) && (i == MNET_MAX_MODULE_IDS))
    {
        // enable operational state
        setOperationState(stateEnabled);
       
        // set mib control state enable (in case this is first time)
        if (_mibAccessState == false)
        {
            setMibAcessControlState(stateEnabled);
        }

    } else {
        // disable operation state
        setOperationState(stateDisabled);
    }
    return state;
}


void AlarmTask::updateModuleLastError(MNET_MODULE_ID mid, MibTag moduleErrorTag)
{
    
    AlarmEntry_t * curnode = _ms[mid];
    AlarmCode_t errorcode = EC_COMMON_NOERROR;
    alarm_fenter("AlarmTask::updateModuleLastError");
    
    while(curnode)
    {
        errorcode = curnode->code;
        curnode = curnode->next;
    }
    
    setModuleLastError(moduleErrorTag, errorcode);
    alarm_fexit();
}



bool AlarmTask::addAlarmEntry(MNET_MODULE_ID mid,  MibTag moduleErrorTag, AlarmCode_t errorCode, char *arg1, char *arg2)
{
    bool status = true;
    Alarm_severity_t severity;
    int id;
    AlarmEntry_t *aentry; 
    
    alarm_fenter("addAlarmEntry");


    aentry = findAlarmEntry(mid, errorCode);

    if (aentry != NULL)
    {
        alarm_warning(("Alarm (code = 0x%06x) was raised %d times by %s, since UTC %s",  
            errorCode,  ++aentry->count, GetMnetModuleName(mid), ctime(&aentry->utc)));
        return true;
    }

    severity = getSeverity(errorCode);
    
    if (severity < Alarm_max_severity)
    {
        _statistics[0][severity]++;
        _statistics[1][severity]++;
    }

    switch (severity)
    {
    case Alarm_critical:
        /* disable Network Elelemt Operational state  if it is not already disabled */
        setOperationState(stateDisabled);
    case Alarm_major:
    case Alarm_minor:
    case Alarm_info:
        aentry = allocEntry(mid);
        id = generateNextId();

        if (aentry )
        {
            aentry->count = 1;
            aentry->code = errorCode;
            aentry->id = id;
            aentry->severity = severity;
            aentry->utc = time(NULL);
        } else {
            alarm_error(("Too many alarm entries.\n"));
        }
       
        updateModuleLastError(mid, moduleErrorTag); 
        reportNewAlarm(id, mid, errorCode, severity, arg1, arg2);
        break;

    default:
        alarm_error(("Illegle Severity code\n"));
        break;
    }   
    alarm_fexit();
    return status;
}


AlarmEntry_t *  AlarmTask::findAlarmEntry(MNET_MODULE_ID mid, AlarmCode_t errorCode)
{
    AlarmEntry_t * curnode = _ms[mid];
    
    while (curnode )
    {
        if (curnode->code == errorCode)
            return curnode;
        curnode= curnode->next;
    }
    return NULL;
}



bool AlarmTask::delAlarmEntry(MNET_MODULE_ID mid, MibTag moduleErrorTag, AlarmCode_t errorCode)
{
    bool status = true;
    char prevAlarm[20];
    char prevId[20];
    AlarmCode_t lastError = (AlarmCode_t) 0;
    Alarm_severity_t severity;
    int         lastId = 0;

    memset(prevAlarm, 0, 20);
    
    
    if ((mid  >=0) && (mid < MNET_MAX_MODULE_IDS))
    {
        AlarmEntry_t * curnode = _ms[mid];
        AlarmEntry_t * prevnode = curnode;
        
        if (errorCode == EC_COMMON_NOERROR)
        {
            // clear all the error for this module
            _ms[mid] = NULL;
            while (curnode )
            {
                prevnode = curnode;
                curnode = curnode->next;
                
                severity = getSeverity(prevnode->code);
                
                if (severity < Alarm_max_severity)
                {
                    _statistics[1][severity]--;
                }
                lastError = prevnode->code;
                lastId = prevnode->id;
                prevnode->code = EC_COMMON_NOERROR;
                prevnode->next = NULL;
            }
        } else {
            
            while (curnode && (curnode->code != errorCode))
            {
                prevnode = curnode;
                curnode = curnode->next;
            }
            
            if (curnode)
            {
                if (curnode == prevnode)
                {
                    // the first node
                    _ms[mid] = curnode->next;
                } else {
                    prevnode->next = curnode->next;
                }
                
                severity = getSeverity(curnode->code);
                
                if (severity < Alarm_max_severity)
                {
                    _statistics[1][severity]--;
                }
                lastError = prevnode->code;
                lastId = prevnode->id;
                curnode->code = EC_COMMON_NOERROR;
                curnode->next = NULL;
            }
        }
        int id = generateNextId();
        sprintf(prevAlarm,"%d", lastError);
        sprintf(prevId, "%d", lastId);
        reportNewAlarm(id, mid, EC_COMMON_NOERROR, Alarm_info, prevAlarm, prevId);
        updateOperationalState();
        updateModuleLastError(mid, moduleErrorTag);        
    } else {
        status = false;
    }
    return status;
}


void AlarmTask::reportNewAlarm(int id, MNET_MODULE_ID mid, AlarmCode_t ecode, Alarm_severity_t severity, 
                   char * arg1, char * arg2)
{
	extern bool alarm_sendAlarmEventToAms(ubyte4, ubyte4, ubyte4, ubyte4, char*, char*);

    char strMsg[ALARM_maxMsgSize];
    time_t ltime;
    int posCount = 0;
    alarm_fenter("AlarmTask::reportNewAlarm");


    alarm_debug(("Formating alarm message to the GUI\n"));
    memset((void *) strMsg,0,ALARM_maxMsgSize);
    posCount += sprintf(strMsg + posCount, "%d|", id);

    /* Get current local time */
    time( &ltime );
    
    posCount += sprintf(strMsg + posCount, "%ld|", ltime);
    posCount += sprintf(strMsg + posCount, "%d|", severity);
    posCount += sprintf(strMsg + posCount, "%d|", mid);
    posCount += sprintf(strMsg + posCount, "%s|", GetMnetModuleName(mid));
    posCount += sprintf(strMsg + posCount, "%d", ecode);
    
    if (arg1 != NULL)
        posCount += sprintf(strMsg + posCount, "|%s", arg1);
    if(arg2 != NULL)
        posCount += sprintf(strMsg + posCount, "|%s", arg2);
    
    for (int i = posCount; i < ALARM_record_length-1; i++)
        strMsg[i] = ' ';

    strMsg[ALARM_record_length-1] = '\n';
    strMsg[ALARM_record_length] = '\0';


    // Broadcast message to all the Java Client Applets
    alarm_broadcast(strMsg);

	// we write to flash for all events
	saveAlarm(strMsg);

    // Send message to First AMS server
	alarm_sendAlarmEventToAms(id, ecode, ltime, mid, arg1, arg2);
    
    if (ecode == 0)
    {
        alarm_warning(("Alarm (Id = %d) in module \"%s\" has been cleared\n", id, GetMnetModuleName( mid)));
    } else  {
        alarm_warning(("The module \"%s\" has raised an alarm (Id= %d, Code = %d)\n", GetMnetModuleName(mid), id, ecode));
    }
    alarm_fexit();

}

extern "C" void AlarmPrintStatus()
{

    AlarmTask::theAlarmTask->printStatus();

}

void AlarmTask::printStatus()
{
    int i;
    AlarmEntry_t * tmpentry;

    printf("\nNetwork Element Operational State: %d\n", getOperationState());
    printf("Mib Access Status: %d\n", _mibAccessState);

    printf("\nAlarm Statistics:\n");
    printf("Total Alarms in this session:\n");
    printf("\tCritical: %d\n", _statistics[0][Alarm_critical]);
    printf("\tMajor: %d\n", _statistics[0][Alarm_major]);
    printf("\tMinor: %d\n", _statistics[0][Alarm_minor]);
    printf("\tInformational: %d\n", _statistics[0][Alarm_info]);
    
    printf("Total Outstanding Alarms:\n");
    printf("\tCritical: %d\n", _statistics[1][Alarm_critical]);
    printf("\tMajor: %d\n", _statistics[1][Alarm_major]);
    printf("\tMinor: %d\n", _statistics[1][Alarm_minor]);
    
    if (_statistics[1][Alarm_critical]
        + _statistics[1][Alarm_major]
        + _statistics[1][Alarm_minor])
    {
        printf("%20s%20s\n", "MODULE NAME",  "Outstanding Alarms");
        printf("%20s%20s\n", "===========",  "===================");
        for(i=0; i< MNET_MAX_MODULE_IDS; i++)
        {
            if (_ms[i])
            {
                tmpentry = _ms[i];
                printf("%20s  ", GetMnetModuleName((MNET_MODULE_ID)i));
                while(tmpentry)
                {
                    printf(" %06x", tmpentry->code);
                    tmpentry = tmpentry->next;
                }
                printf("\n");
            }
        }
    }
}

void AlarmTask::getOutstandingAlarmCount(ubyte1 &critical, ubyte1 &major, ubyte1 &minor)
{
    critical =  _statistics[1][Alarm_critical];
    major= _statistics[1][Alarm_major];
    minor=  _statistics[1][Alarm_minor];
}

#endif /* _ALARM_MODULE_CPP_ */
