// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CallConfig.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included VxWorks headers
#include "vxWorks.h"
#include "JCC/JCCLog.h"
#include "MnetModuleId.h"
#include "taskLib.h"
#include "stdio.h"

#include "CC/CCconfig.h"
#include "Os/JCTask.h"
#include "GP10OsTune.h"

// declare the CC task entry function
extern int callMain(const short callIndex);
extern JCTask *CallTaskObjs[CC_MAX_CC_CALLS_DEF_VAL];


int callInit(const short callIndex)
{
  // This call configuration function initializes one call session task.
  //
  // A typical invocation is usually be done from the 
  // CC subsystem

  int taskId;
  char callTaskName[10];

  sprintf(&callTaskName[0], "Call-%d\0", callIndex); 

  // create the call task
  CallTaskObjs[callIndex] = new JCTask(&callTaskName[0]);
  if(( taskId = CallTaskObjs[callIndex]->JCTaskSpawn(
                               CC_SUB_TASK_PRIORITY,	// priority
                               CC_SUB_TASK_OPTION,      // option word
// Calltask (Call-0) was overflowing at 10,352 bytes with
// external handover turned on and doing 3 way call hold/retrieve toggling
// The stack size is increased to 20,000 for now.
// Need to find a better number later!
// ext-ho <chenj:08-16-01>
                               CC_SUB_TASK_STACK_SIZE * 2,	// stack size
                               (FUNCPTR) callMain,      // entry function pointer
                               callIndex, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               MODULE_CC,
                               JC_NON_CRITICAL_TASK) )
     == ERROR )
    {
      // error creating task. log the error and return.
      printf("CC CALL FATAL ERROR: failed to create CC Call (task = %d)\n",
			 callIndex);
    };
  return taskId;
}
