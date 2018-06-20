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
// File        : JCTask.cpp
// Author(s)   : Tim Olson
// Create Date : 10/18/2000
// Description :  
//
// *******************************************************************

#include <stdio.h>
#include <errNoLib.h>
#include <string.h>
#include "Os/JCTask.h"
#include "Os/JCModule.h"

SEM_ID *pMnetSyncSem;

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCTask constructor
**
**    PURPOSE: Initialize data members for JCTask object.  
**
**    INPUT PARAMETERS: name - name of the task 
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
JCTask::JCTask
(    
    char *name        /* name of the task */
) : taskStatus(JC_TASK_IDLE), taskAlarmed(FALSE)
{
    if (name)
    {
        taskName = new char[strlen(name) + 1];
        if (taskName)
        {
            strcpy(taskName, name);
        }
        else
        {
            printf("JCTask::JCTask ERROR: Unable to allocate memory\n");
        }
    }
    else
    {
        taskName = 0;
    }
}   



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCTask destructor
**
**    PURPOSE: Delete data members for JCTask object.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
JCTask::~JCTask ()
{
    delete [] taskName;
}   




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCTask::JCTaskSpawn
**
**    PURPOSE:  Spawn a task. 
**
**    INPUT PARAMETERS: priority - (0-255)
**                      options - option word
**                      stackSize - size in bytes of stack
**                      entryPt - entry point of task
**                      arg1 - argument 1 passed to entryPt 
**                      arg2 - argument 1 passed to entryPt 
**                      arg3 - argument 1 passed to entryPt 
**                      arg4 - argument 1 passed to entryPt 
**                      arg5 - argument 1 passed to entryPt 
**                      arg6 - argument 1 passed to entryPt 
**                      arg7 - argument 1 passed to entryPt 
**                      arg8 - argument 1 passed to entryPt 
**                      arg9 - argument 1 passed to entryPt 
**                      arg10 - NOT AN ARGUMENT!!!!  This parameter can be used
**                                  to pass a function pointer to a function that
**                                  will be called if the task is suspended. 
**                      modId - module id number
**                      importance - critical or non-critical 
**
**    RETURN VALUE(S): taskId or ERROR
**
**----------------------------------------------------------------------------*/
int JCTask::JCTaskSpawn
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
)
{    
    
    JCModule::InsertTaskInfo(modId, this);    
    taskPri = priority;
    taskOpt = options;
    taskStackSize = stackSize;
    taskEntryPt = entryPt;
    taskArg1 = arg1;
    taskArg2 = arg2;
    taskArg3 = arg3;
    taskArg4 = arg4;
    taskArg5 = arg5;
    taskArg6 = arg6;
    taskArg7 = arg7;
    taskArg8 = arg8;
    taskArg9 = arg9;
    taskArg10 = arg10;
    taskImportance = importance;
    moduleId = modId;
    taskStatus = JC_TASK_INIT;
    coreDumpFunc = (FUNCPTR)arg10;
    
    taskId = taskSpawn(taskName, priority, options, stackSize, entryPt,
                    arg1, arg2, arg3, arg4, arg5, 
                    arg6, arg7, arg8, arg9, 0);

    return(taskId);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCTask::JCTaskEnterLoop()
**
**    PURPOSE:   
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void JCTask::JCTaskEnterLoop()
{
    taskStatus = JC_TASK_LOOP;
    
    // Spin here waiting for all other tasks to call enter loop.
    if (pMnetSyncSem)
        semBTake(*pMnetSyncSem, WAIT_FOREVER);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCTask::JCTaskNormExit()
**
**    PURPOSE:   
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void JCTask::JCTaskNormExit()
{
    taskStatus = JC_TASK_EXIT;
}
