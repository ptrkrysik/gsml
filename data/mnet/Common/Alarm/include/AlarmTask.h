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
/* Description : This file contains Alarm task routines              */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |09/28/00| Initial Draft                                 */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/

#ifndef _ALARMTASK_H_
#define _ALARMTASK_H_  /* include once only */

#include  "Alarm.h"


/* Include product specific code here */
#if defined (MNET_PRODUCT_SPECIFIC_GS)
#include  "GSOsTune.h"
#elif defined (MNET_PRODUCT_SPECIFIC_GMC)
#include  "vclogging.h"
#elif defined (MNET_PRODUCT_SPECIFIC_GP10)
#include  "Logging/vclogging.h"
#endif

typedef struct _AlarmEntry_t
{
    AlarmCode_t     code;      // last error code
    int             id;        // first alarm ID
    Alarm_severity_t severity; // severity
    ubyte4          utc;       // outstanding since this time
    int             count;
    struct _AlarmEntry_t *next;
}AlarmEntry_t;


class AlarmTask:public JCTask {
public:

    AlarmTask(char *);
    ~AlarmTask();

    int InitAlarmTask();
    
    static int bootupTimeoutHandler();
    static int entryPoint();

    static int AlarmMain(void);
    static char *srcName();
    
    static AlarmTask *theAlarmTask;
    
    // disk operation
    STATUS saveAlarm(const char* msg);
    STATUS renameOldLogFile();
    STATUS getLastId(unsigned long *id);
    int generateNextId();

    AlarmEntry_t *findAlarmEntry(MNET_MODULE_ID mid, AlarmCode_t errorCode);
    bool addAlarmEntry(MNET_MODULE_ID mid, MibTag moduleErrorTag, AlarmCode_t code, char *arg1=NULL, char *arg2=NULL);
    bool delAlarmEntry(MNET_MODULE_ID mid, MibTag moduleErrorTag, AlarmCode_t code);
    EnableDisableState_t updateOperationalState();


    // module handling
    AlarmCode_t getMdlStatus(MNET_MODULE_ID mid);
    AlarmEntry_t *allocEntry(MNET_MODULE_ID id);

    void printStatus();
    void getOutstandingAlarmCount(ubyte1 &, ubyte1 &, ubyte1 &);

    // message q operation
    JC_STATUS msgqSend( unsigned int msgType, char * buffer, unsigned int nBytes);
    JC_STATUS alarmSend(InterModuleAlarmMsgType_t mtype,
                        MNET_MODULE_ID             mid, 
                        MibTag                     moduleErrorTag=0, 
                        AlarmCode_t                alarm_code=EC_COMMON_NOERROR,
                        char                       *arg1=NULL,
                        char                       *arg2=NULL
                        );

    //AMS Operation
    bool sendSummaryUpdateToAms();
    void reportNewAlarm(int id, MNET_MODULE_ID mid, AlarmCode_t ecode,
        Alarm_severity_t severity, char * arg1=0, char * arg2=0);
   
    //Operational State  Management
    EnableDisableState_t getOperationState();
    void setOperationState(EnableDisableState_t opstate );

    // Mib access control status
    EnableDisableState_t getMibAcessControlState();
    void setMibAcessControlState(EnableDisableState_t opstate);
    
    void updateModuleLastError(MNET_MODULE_ID mid, MibTag moduleErrorTag);
    void setModuleLastError(MibTag moduleErrorTag, AlarmCode_t alarm_code);
    
    // startup and quit
    void quit();
    int startup();
    void closeSocket();
    

private:
    JCMsgQueue *jcMsgQ;
    JCCTimer *initTimer;
    char *_mnetbase;        /* holds MNET_BASE environment variable */
    char *_logfilename;     /* holds absolute path of the logfilename */

    char *_sourceName;      /* holds the source name */
    char *_sourceType;      /* type of the source (Gp, Gmc, Itp, */

    char *_msgRecvBuff;     /* holds received message */
    AlarmEntry_t *_ms[MNET_MAX_MODULE_IDS];  /* current module Alarm Status */
    AlarmEntry_t *_ap;                       /* Alarm pools */

    int _statistics[2][Alarm_max_severity];

    void populateSourceName();

    //status variables
    bool _quit;
    bool _mibAccessState;

    static bool isOneCreated;
    
    // Disallow the following functions
    AlarmTask();
    AlarmTask(const AlarmTask&);
    AlarmTask& operator=(const AlarmTask&);
    int operator== (const AlarmTask&) const;
};


#endif
