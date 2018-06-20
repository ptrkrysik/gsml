/****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: logtask.cpp													*
 *																						*
 *	Description			: Logging task  												*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |11/11/00 | DSN    | File created												*
 *		 |		   |		|															*
 ****************************************************************************************
 */


#include <stdio.h>
#include <string.h>
#include <hostlib.h>

/* common for all */
#include "Os/JCModule.h"
#include "Os/JCTask.h"
#include "Os/JCMsgQueue.h"

/* common for GS */
#include "GP10OsTune.h"
#include "MnetModuleId.h"

#include "logging/vclogging.h"



/*
 * instance of sm task
 */
JCTask		gLogTask("Logging");


int	
SysCommand_Log(T_SYS_CMD	action)
{
	int tid;
	JC_STATUS	stat;

	switch(action){
		case SYS_SHUTDOWN:
			printf("[LogTask] Received system shutdown notification\n");
			break;
		case SYS_START:
			printf("[LogTask] Received task start notification\n");
			init_log_task();
			stat = gLogTask.JCTaskSpawn(LOG_TASK_PRIORITY,
									   LOG_TASK_OPTION,
									   LOG_TASK_STACK_SIZE,
							           (FUNCPTR)viper_log,
									   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
									   MODULE_LOG,				
									   JC_NON_CRITICAL_TASK);		
			if(stat == JC_ERROR){
				printf("[LogTask] JCTaskSpawn returned error.\n");
			}
			else{
				printf("[LogTask] task spawned (tid=0x%x) \n", gLogTask.GetTaskId());
                ViperLogSetModuleFilters(1000,0);
			}
			break;
		case SYS_REBOOT:
			printf("[LogTask] Reboot ready.\n");
			break;

		default:
			printf("[LogTask] Unknown system command received\n");
	}
	return 0;
}


/*
 * GetViperCellIpAddress
 * NOTE: This is left over from the old logger in GP10.  This should not be
 * here!!!!!!
 */
char *GetViperCellIpAddress()
{
	static char DirtyBuffer[MAX_BUFF_LENGTH],InterfaceName[MAX_BUFF_LENGTH];
	int Status;

	memset(DirtyBuffer,0,MAX_BUFF_LENGTH);
	memset(InterfaceName,0,MAX_BUFF_LENGTH);

	strcpy(InterfaceName,VIPERCELL_ETHERNET_INTERFACE);
	Status = ifAddrGet(InterfaceName,DirtyBuffer);
	if (Status == ERROR)
	{
		printf("Error retrieving interface %s's IP address!\n",InterfaceName);
		return "";
	}
	return(DirtyBuffer);
	
}
