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
/* Description : This file contains alarm task basic defination      */
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


#if !defined(_ALARM_H_)
#define _ALARM_H_

#include "AlarmCode.h"
#include "Os/JCMsgQueue.h"
#include "Os/JCModule.h"
#include "Os/JCCTimer.h"
#include "Os/JCTask.h"

#include "alarm_prodSpec.h"

typedef unsigned short              ubyte2;
typedef unsigned long               ubyte4;
typedef unsigned char               ubyte1;

typedef short                       sbyte2;
typedef long                        sbyte4;


#define ALARM_q_maxMsgs             32
#define ALARM_q_maxMsgLength        100
#define ALARM_q_option              MSG_Q_PRIORITY
#define ALARM_q_timeout             18000      /* 5 minutes */

#define ALARM_task_name             "tAlarm"
#define ALARM_mib_lock_timeout      7200    /* 2 minutes */

#define ALARM_record_length         70
#define ALARM_max_records           58
#define ALARM_max_outstanding       50


/* It is assumeed that current working directory is ViperCall */

#define ALARM_logFile               "/Alarm/alarm.log"
#define ALARM_logFileMaxSize        128
#define ALARM_dirChar               '/'

#define ALARM_maxMsgSize            1024
#define ALARM_defaultRecvBuffer     128
#define ALARM_optionalArgSize       40


#define ALARM_monStation      "QSEvent"
#define ALARM_mchResponse     "mchResponse"

typedef enum Alarm_severity 
{
   Alarm_critical=0,
   Alarm_major, 
   Alarm_minor,
   Alarm_info,
   Alarm_max_severity
} Alarm_severity_t;


typedef enum
{
    InterModuleAlarmMsgTypeClearAlarm = 0,
    InterModuleAlarmMsgTypeRaiseAlarm,
    InterModuleAlarmMsgTypeShutdown,
    InterModuleAlarmMsgTypeReboot
}InterModuleAlarmMsgType_t;


typedef struct
{
    ubyte1                    senderId;      /* message originator      */
    InterModuleAlarmMsgType_t msgType;       /* message type (MT_TRAP)  */
    MNET_MODULE_ID            srcModuleId;   /* Alarm source module ID  */
    MibTag                    errorTag;      /* This module's ErrorTag  */
    AlarmCode_t               code;          /* Alarm code              */
    char                      arg1[ALARM_optionalArgSize];          /* argument 1              */
    char                      arg2[ALARM_optionalArgSize];          /* argument 2              */
}InterModuleAlarmMsg_t;


typedef enum
{ 
    stateDisabled=0,
   stateEnabled=1
} EnableDisableState_t;


/* Printing and sending to logging module */
void alarm_print(char *format, ...);

#ifdef _UT
void HexDumper(ubyte1 *buffer, int length);
#define alarm_debug(XX)   printf("[ALARM TRACE] %s ->", fname); alarm_print XX
#define alarm_error(XX)   printf("[Alarm Error] %s ->", fname); alarm_print XX
#define alarm_warning(XX) printf("[Alarm Warning] %s ->", fname); alarm_print XX
#define alarm_dump(x,y);  HexDumper(x, y);
#define alarm_fenter(XX); static char fname[] = XX;
#define alarm_fexit();
#else
#define  alarm_fenter(XXX); \
   DBG_FUNC(XXX, MY_LOGID);  \
   DBG_ENTER(); \

#define alarm_fexit();  DBG_LEAVE();

#define alarm_debug(XX) DBG_TRACE XX 
#define alarm_error(XX) DBG_ERROR XX 
#define alarm_warning(XX) DBG_WARNING XX
#define alarm_dump(x,y);  DBG_HEXDUMP(x,y);
#endif 


/* defined at alarm_main module */
int alarmTask();
int alarm_taskCreate();
void alarm_taskQuit ();
void alarm_msgPrint(char *format, ...);
void alarm_rebootHandler ( int startType );


/* In file alarm_modules.cpp */
void alarm_rptMdlStatus(int id, int mid, int ecode, int severity=Alarm_critical, 
               char* arg1= (char *) NULL, char* arg2= (char *) NULL);
void alarm_printStatus();


/* In file alarm_message.cpp */
void alarm_processMsg(char *inComingMsg);

/* It will go to alarm_event.cpp */
void  alarm_clientMsg (char *, short, unsigned long);
void alarm_broadcast(const char *msg);
int alarm_generateNextId();


/* alarm_diskFile.cpp */
STATUS alarm_renameOldLogFile();

/* TimeSync api from Time Sync module */
extern void time_syncTime();

#endif
