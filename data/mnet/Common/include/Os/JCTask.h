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
// File        : JCTask.h
// Author(s)   : Tim Olson
// Create Date : 9/18/2000
// Description : 
//
// *******************************************************************
#ifndef _JCTASK_H_
#define _JCTASK_H_  /* include once only */

#include <taskLib.h>
#include <semLib.h>
#include "JCErr.h"
#include "MnetModuleId.h"

/* OS independent task id */
#define  JC_TASK_ID                 int

/* Task creation options */
#define  JC_VX_FP_TASK          VX_FP_TASK          /* execute with floating-point coprocessor support */
#define  JC_VX_PRIVATE_ENV      VX_PRIVATE_ENV      /* include private environment support */
#define  JC_VX_NO_STACK_FILL    VX_NO_STACK_FILL    /* do not fill the stack for use by checkStack */
#define  JC_VX_UNBREAKABLE      VX_UNBREAKABLE      /* do not allow breakpoint debugging */

extern SEM_ID *pMnetSyncSem;

/* System importance for task */
typedef enum {
    JC_CRITICAL_TASK,
    JC_NON_CRITICAL_TASK
} JC_TASK_IMPORTANCE;

/* Task status */
typedef enum {
    JC_TASK_IDLE,
	JC_TASK_INIT,
	JC_TASK_LOOP,
	JC_TASK_EXIT,
} JC_TASK_STATUS;


class JCTask {
public:
    JCTask
    (
        char *name
    );

    ~JCTask();

    int JCTaskSpawn
    (
        int                     priority,   /* priority of new task                      */
        int                     options,    /* task option word                          */
        int                     stackSize,  /* size (bytes) of stack needed plus name    */
        FUNCPTR                 entryPt,    /* entry point of new task                   */
        int                     arg1,       /* 1st of 10 req'd task args to pass to func */
        int                     arg2,
        int                     arg3,
        int                     arg4,
        int                     arg5,
        int                     arg6,
        int                     arg7,
        int                     arg8,
        int                     arg9,
        int                     arg10,
        MNET_MODULE_ID          modId,
        JC_TASK_IMPORTANCE      importance
    );

    void JCTaskEnterLoop();
    void JCTaskNormExit();
    
    const char *GetTaskName() { return (taskName); }
    int GetTaskId() { return (taskId); }
    JC_TASK_IMPORTANCE GetTaskImportance() { return (taskImportance); }
    JC_TASK_STATUS GetTaskStatus() { return (taskStatus); }

    JC_TASK_ID JCTaskId()
    {
        return(taskId);
    }
    
    void SetTaskAlarmStatus(bool stat) { taskAlarmed = stat; }
    bool GetTaskAlarmStatus() { return(taskAlarmed); }
    
    void ExecuteCoreDumpFunc() { if (coreDumpFunc) coreDumpFunc(); }

private:
    int             taskId;
    char            *taskName;
    int             taskPri;        /* priority of new task                      */
    int             taskOpt;        /* task option word                          */
    int             taskStackSize;  /* size (bytes) of stack needed plus name    */
    FUNCPTR         taskEntryPt;    /* entry point of new task                   */
    int             taskArg1;       /* 1st of 10 req'd task args to pass to func */
    int             taskArg2;
    int             taskArg3;
    int             taskArg4;
    int             taskArg5;
    int             taskArg6;
    int             taskArg7;
    int             taskArg8;
    int             taskArg9;
    int             taskArg10;
    MNET_MODULE_ID  moduleId; 
    JC_TASK_IMPORTANCE taskImportance;
    FUNCPTR         coreDumpFunc;
    JC_TASK_STATUS  taskStatus;
    bool            taskAlarmed;


    // Disallow the following functions
    JCTask();
    JCTask(const JCTask&);
    JCTask& operator=(const JCTask&);
    int operator== (const JCTask&) const;
};


#endif
