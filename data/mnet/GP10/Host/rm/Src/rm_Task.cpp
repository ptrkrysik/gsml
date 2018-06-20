/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_TASK_CPP__
#define __RM_TASK_CPP__

#include "rm\rm_head.h"
#include "Os\JCTask.h"
#include "GP10OsTune.h"

extern JCTask rmTaskObj;

void rm_Task( void )
{
     RDEBUG__(("ENTER-rmTask starting...\n"));

     // Create rmTask
     if( ERROR == rmTaskObj.JCTaskSpawn(
			 RM_TASK_PRIORITY,	            // Priority
			 RM_TASK_OPTION,               // Option word
			 RM_TASK_STACK_SIZE,	      // Stack size
			 (FUNCPTR)rm_Main,             // Task entry
			 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        MODULE_RM, JC_CRITICAL_TASK  // Parameters
			    ) )
      {
	 // Error occuring in creating rmTask, report the error and return.
	 EDEBUG__(("ERROR-rm_Task: VxWorks TaskSpawn failed, err:%d\n", errno));
      }
   
}

#endif /* __RM_TASK_CPP__ */
