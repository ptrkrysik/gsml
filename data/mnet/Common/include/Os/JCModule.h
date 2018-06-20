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
// File        : JCModule.h
// Author(s)   : Tim Olson
// Create Date : 9/18/2000
// Description : 
//
// *******************************************************************
#ifndef _JCMODULE_H_
#define _JCMODULE_H_  /* include once only */

#include <sysSymTbl.h>
#include "MnetModuleId.h"
#include "Os/JCTask.h"

#define MAX_NUM_SUB_TASKS   50


typedef enum {
	SYS_SHUTDOWN		=	0,
    SYS_START			=	1,
	SYS_REBOOT			=	2,
} T_SYS_CMD;


typedef int (*JC_SYSCMD_FUNC)(T_SYS_CMD); 

typedef enum
{
    TASK_SUSPENDED,
    TASK_MISSING,
    MODULE_OK
} SYSTEM_MODULE_STATUS;

class JCModule {
public:
    JCModule
    (
        MNET_MODULE_ID  id,
        JC_SYSCMD_FUNC  cmdFunc,
        char            *cmdFuncStr
    );

    ~JCModule();
    
    bool StartModule();
    void ShutdownModule();
    void RebootModule();
    SYSTEM_MODULE_STATUS CheckModule(JCTask **pTask);
    MNET_MODULE_ID GetModuleId() { return (modId); }
    
    static JCModule *systemModules[];
    static void InsertTaskInfo(MNET_MODULE_ID id, JCTask *pTask);
    static bool AllTasksInMainLoop();
    static void ShowModuleStat();
    
private:
    JC_SYSCMD_FUNC          syscmd;
    char                    *modName;
    MNET_MODULE_ID          modId;
    int                     numRun;
    int                     numSub;
    JCTask                  *tasks[MAX_NUM_SUB_TASKS];
    bool                    isModuleLoaded;

    // Private functions
    static int CheckSym(char *name, int val, SYM_TYPE type, int arg, UINT16 group);
    int TestSym(char *str);
    

    // Disallow the following functions
    JCModule();
    JCModule(const JCModule&);
    JCModule& operator=(const JCModule&);
    int operator== (const JCModule&) const;
};


#endif
