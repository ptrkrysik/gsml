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
// File        : JCModule.cpp
// Author(s)   : Tim Olson
// Create Date : 10/18/2000
// Description :  
//
// *******************************************************************

#include <stdio.h>
#include <errNoLib.h>
#include <string.h>
#include <ioLib.h>
#include <dbgLib.h>
#include <usrLib.h>
#include "Os/JCModule.h"
#include "AlarmCode.h"

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule constructor
**
**    PURPOSE: Initialize data members for JCModule object.  
**
**    INPUT PARAMETERS: id - module id
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
JCModule::JCModule
(    
    MNET_MODULE_ID  id,
    JC_SYSCMD_FUNC  cmdFunc,
    char            *cmdFuncStr
) : modId(id), numRun(0), numSub(0)
{
    char *name = GetMnetModuleName(id);
    if (name)
    {
        modName = new char[strlen(name) + 1];
        if (modName)
        {
            strcpy(modName, name);
        }
        else
        {
            printf("JCModule::JCModule ERROR: Unable to allocate memory\n");
        }
    }
    else
    {
        modName = 0;
    }
    
    
    if (!TestSym(cmdFuncStr))
    {
        syscmd = cmdFunc;
        isModuleLoaded = TRUE;
    }
    else
    {
        isModuleLoaded = FALSE;
    }
    
    for (int i=0; i < MAX_NUM_SUB_TASKS; i++)
    {
        tasks[i] = 0;
    }
}   



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule destructor
**
**    PURPOSE: Delete data members for JCModule object.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
JCModule::~JCModule ()
{
    delete [] modName;
}   



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule InsertTaskInfo
**
**    PURPOSE: InsertTaskInfo adds the task object to the list of tasks
**      currently running under this module.  
**
**    INPUT PARAMETERS: id - module id
**                      pTask - pointer to the new task object
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void JCModule::InsertTaskInfo(MNET_MODULE_ID id, JCTask *pTask)
{
    for (int j=0; j < MNET_MAX_MODULE_IDS; j++)
    {
        if (systemModules[j])
        {
            if (systemModules[j]->modId == id)
            {
                for (int i=0; i < MAX_NUM_SUB_TASKS; i++)
                {
                    if (!systemModules[j]->tasks[i])
                    {
                        systemModules[j]->tasks[i] = pTask;
                        systemModules[j]->numSub++;
                        break;
                    }
                }
                break; 
            }
        }
    }
}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule StartModule
**
**    PURPOSE: StartModule sends a SYS_START system command to the module.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): TRUE - if module is loaded
**                     FALSE - if module is not loaded
**
**----------------------------------------------------------------------------*/
bool JCModule::StartModule()
{
    bool ret = FALSE;
    
    if (isModuleLoaded)
    {
        if (!numRun)
        {
            printf("SysCommand(SYS_START): %s\n", modName);

	        syscmd(SYS_START);
            numRun++;
        }
        else
        {
            printf("Module %d already started\n", modId);
        }
        ret = TRUE;
    }
    
    return(ret);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule ShutdownModule
**
**    PURPOSE: StartModule sends a SYS_SHUTDOWN system command to the module.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void JCModule::ShutdownModule()
{
    if (isModuleLoaded)
    {
        printf("SysCommand(SYS_SHUTDOWN): %s\n", modName);
        syscmd(SYS_SHUTDOWN);
    }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule RebootModule
**
**    PURPOSE: StartModule sends a SYS_REBOOT system command to the module.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void JCModule::RebootModule()
{
    if (isModuleLoaded)
    {
        printf("SysCommand(SYS_REBOOT): %s\n", modName);
        syscmd(SYS_REBOOT);
    }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule CheckModule
**
**    PURPOSE: CheckModule verifies that all tasks associated with this module
**      are currently running.  
**
**    INPUT PARAMETERS: pTask - if problems a pointer to the troubled task
**                                  with the highest criticality
**
**    RETURN VALUE(S): SYSTEM_MODULE_STATUS
**
**----------------------------------------------------------------------------*/
SYSTEM_MODULE_STATUS JCModule::CheckModule(JCTask **pTask)
{
    SYSTEM_MODULE_STATUS retCode = MODULE_OK;
    *pTask = 0;
    if (isModuleLoaded)
    {
	    char *tName;
        for(int i=0; i < numSub; i++){ 
            // check for exit abnormal
		    tName = taskName(tasks[i]->GetTaskId());
		    if(tName == NULL)
            {
                printf("Alert! found missing task(%s)\n", tasks[i]->GetTaskName());
                
                if (!*pTask)
                {
                    retCode = TASK_MISSING;
                    *pTask = tasks[i];
                }
                else if ((tasks[i]->GetTaskImportance() == JC_CRITICAL_TASK) &&
                        ((*pTask)->GetTaskImportance() == JC_NON_CRITICAL_TASK))
                {
                    retCode = TASK_MISSING;
                    *pTask = tasks[i];
                }
		    }
            // check for suspended
            else if(taskIsSuspended(tasks[i]->GetTaskId()))
            {
                char coreFileName[128];
                char *pMnetBase;
                int coreFd = ERROR;
                int stdoutFd, stderrFd;
                
                // Create a core file for this task.
                pMnetBase = getenv("MNET_BASE");
                
                if (pMnetBase && !tasks[i]->GetTaskAlarmStatus())
                {
                    strcpy(coreFileName, pMnetBase);
                    strcat(coreFileName, "\\");
                    strcat(coreFileName, tasks[i]->GetTaskName());
                    strcat(coreFileName, ".dmp");
                    
                    // If the file was created replace stdout and stderr with the
                    // file descriptor for the task core file.
                    if ((coreFd = creat(coreFileName, O_RDWR)) != ERROR)
                    {
                        stdoutFd = ioGlobalStdGet(1);
                        stderrFd = ioGlobalStdGet(2);
                        ioGlobalStdSet(1, coreFd);
                        ioGlobalStdSet(2, coreFd);
                    }
                }
                
                printf("Alert! found suspended task(%s)\n", tasks[i]->GetTaskName());
                tt(tasks[i]->GetTaskId());
                ti(tasks[i]->GetTaskId());
                tasks[i]->ExecuteCoreDumpFunc();
                
                // If a core file was created then return stdout and stderr.
                if (coreFd != ERROR)
                {
                    ioGlobalStdSet(1, stdoutFd);
                    ioGlobalStdSet(2, stderrFd);
                    close(coreFd);
                    printf("Alert! found suspended task(%s)\n", tasks[i]->GetTaskName());
                    tt(tasks[i]->GetTaskId());
                    ti(tasks[i]->GetTaskId());
                    tasks[i]->ExecuteCoreDumpFunc();
                }
                
                if (!*pTask)
                {
                    retCode = TASK_SUSPENDED;
                    *pTask = tasks[i];
                }
                else if ((tasks[i]->GetTaskImportance() == JC_CRITICAL_TASK) &&
                        ((*pTask)->GetTaskImportance() == JC_NON_CRITICAL_TASK))
                {
                    retCode = TASK_SUSPENDED;
                    *pTask = tasks[i];
                }
            }
        } 
    }
    
    return(retCode);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule AllTasksInMainLoop
**
**    PURPOSE: Check to see if all tasks are in the main loop.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): TRUE - all in main
**                     FALSE - not all tasks in main loop
**
**----------------------------------------------------------------------------*/
bool JCModule::AllTasksInMainLoop()
{
    for (int i=0; i < MNET_MAX_MODULE_IDS; i++)
    {
        if (systemModules[i])
        {
            for (int j=0; j < MAX_NUM_SUB_TASKS; j++)
            {
                if (systemModules[i]->tasks[j])
                {
                    if (systemModules[i]->tasks[j]->GetTaskStatus() != JC_TASK_LOOP)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    return (TRUE);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule ShowModuleStat
**
**    PURPOSE: Display module status.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
extern "C"
{
void ShellShowModuleStat()
{
    JCModule::ShowModuleStat();
}
}

void JCModule::ShowModuleStat()
{
    int totalTasks, totalInit, totalIdle, totalLoop, totalExit, totalUnknown;

    printf ("System Module Status\n");
    
    for (int i=0; i < MNET_MAX_MODULE_IDS; i++)
    {
        if (systemModules[i])
        {
            totalTasks = 0;
            totalInit = 0;
            totalIdle = 0;
            totalLoop = 0;
            totalExit = 0;
            totalUnknown = 0;
            
            for (int j=0; j < MAX_NUM_SUB_TASKS; j++)
            {
                if (systemModules[i]->tasks[j])
                {
                    totalTasks++;
                    switch (systemModules[i]->tasks[j]->GetTaskStatus())
                    {
                        case JC_TASK_IDLE :
                            totalIdle++;
                            break;
                        case JC_TASK_INIT :
                            totalInit++;
                            break;
                        case JC_TASK_LOOP :
                            totalLoop++;
                            break;
                        case JC_TASK_EXIT :
                            totalExit++;
                            break;
                        default :
                            totalUnknown++;
                    }
                }
            }
            printf("\tModule - %s\n", systemModules[i]->modName);
            printf("\t\tTotalTasks(%d) Idle(%d) Init(%d) Loop(%d) Exit(%d) Unknown(%d)\n",
                totalTasks, totalIdle, totalInit, totalLoop, totalExit, totalUnknown);  
        }
    }
    
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule CheckSym
**
**    PURPOSE: CheckSym does a string compare to see if against the symbol.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): 0 - string match
**                     1 - no string match
**
**----------------------------------------------------------------------------*/
int JCModule::CheckSym(char *name, int val, SYM_TYPE type, int arg, UINT16 group)
{
	if(strstr(name, (char *)arg))
    {
		return 0;   // exit symEach
	}
	return 1;	// continue
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: JCModule TestSym
**
**    PURPOSE: Loop through each symbol in the symbol table testing if the
**      string given matches a valid symbol.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): 0 - symbol found
**                     1 - symbol not found
**
**----------------------------------------------------------------------------*/
int JCModule::TestSym(char *str)
{
	SYMBOL *symbol = symEach(sysSymTbl, (FUNCPTR)JCModule::CheckSym, (int)str);

	if(symbol) return 0;
	else
    {
		printf("[WARNING] function not found from symbol table: %s\n", str);
		return 1;
	}
}
