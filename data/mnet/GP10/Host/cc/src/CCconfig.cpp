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
// File        : CCconfig.cpp
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
#include "MnetModuleId.h"
#include "taskLib.h"

#include "CC/CCconfig.h"

#include "Os/JCTask.h"
#include "GP10OsTune.h"

#include "stdio.h"

// declare the CC task entry function
extern int ccMain();
extern JCTask ccTaskObj;


void ccInit()
{
  // This CC configuration function initializes the entire CC
  // module.
  //
  // A typical invocation is usually be done from the 
  // OA&M subsystem

  // create the CC task
  if( ERROR == ccTaskObj.JCTaskSpawn(
							  CC_TASK_PRIORITY,	    // priority
							  CC_TASK_OPTION,        // option word
							  CC_TASK_STACK_SIZE,	// stack size
							  (FUNCPTR) ccMain,      // entry function pointer
							  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							  MODULE_CC, 
							  JC_CRITICAL_TASK) )
    {
      // error creating task. log the error and return.
      printf("CC FATAL ERROR: failed to create CC Main task.\n" );
    };	
}

