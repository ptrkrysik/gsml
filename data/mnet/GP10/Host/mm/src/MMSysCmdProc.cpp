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
// File        : MMSysCmdProc.cpp
// Author(s)   : Kevin Lim 
// Create Date : 07-26-99 
// Description :  
//
// *******************************************************************

// #define DEBUG_MM_HANDOVER
// #define DEBUG_MM_HANDOVER_EFR

// *******************************************************************
// Include Files.
// *******************************************************************

#include "taskLib.h"
#include "stdLib.h"
#include "string.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"

#include "oam_api.h"
#include "Os/JCTask.h"
#include "Os/JCModule.h"
#include "GP10OsTune.h"

// MM Message Constants
#include "MM/MMConfig.h"
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMSysCmdProc.h"
#include "MMTask.h"




extern int mmMain();
extern JCTask mmTaskObj;

// forward declarations
MSG_Q_ID createMMMsgQ();

// ****************************************************************************
int	SysCommand_MM(T_SYS_CMD	action)
{
	int tid;
	int number = 20;
	switch(action){
		case SYS_SHUTDOWN:
			printf("[MM] Received system shutdown notification\n");
			mmDataPrint(number);
			break;
		case SYS_START:
			printf("[MM] Received task start notification\n");
			createMMMsgQ();
			mmInitialize();
			tid = mmTaskObj.JCTaskSpawn(
							MM_TASK_PRIORITY,
							MM_TASK_OPTION,
							MM_TASK_STACK_SIZE,
							(FUNCPTR)mmMain,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							MODULE_MM, JC_CRITICAL_TASK);				
			break;
		case SYS_REBOOT:
			printf("[MM] Reboot ready.\n");
			break;

		default:
			printf("[MM] Unknown system command received\n");
	}
	return 0;
}

// ****************************************************************************
MSG_Q_ID createMMMsgQ()
{
	// create message queue and put the message queue ID in a global
	// variable so that all the Layer-3 Interface API functions can send
	// message to this queue

	DBG_FUNC("createMMMsgQ", MM_LAYER);
    DBG_ENTER();

	mmMsgQId = msgQCreate(	MM_MAX_L3_MSGS,			// max number of messages
							MM_MAX_MSG_LENGTH,	    // size of each message
							MSG_Q_FIFO				// option for msg queue
							);
	if(mmMsgQId == NULL){
		DBG_ERROR("msgQCreate error\n");
	}
	DBG_LEAVE();

	return(mmMsgQId);
}

