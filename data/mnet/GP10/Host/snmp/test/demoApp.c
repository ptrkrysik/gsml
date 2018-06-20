#include <oam_api.h>
#include <stdio.h>
#define INCLUDE_MIB_CONTROL_TABLES
#include <mibtags.h>
#include "vipermib.h"		/* Get the ViperCell MIB definitions.	*/
#include "mibleaf.h"

#include "msgQLib.h"
#include "taskLib.h"
#include "inetLib.h"

/* defines */

#define MY_TABLE_NAME MIB_resourceConfigData
#define H323_TABLE_NAME MIB_h323Data

#define MY_MODULE_ID 7
#define MAX_MSGS 1000

MSG_Q_ID myMsgQId;
static  int oamApiDemoTaskQuit = 1;

oamAppDemo (void)
{
	int i, status;
	INT_32_T tag;
	TrapMsg myTrapMsg;
	ResourceConfigData rrmStruct;
	
	/* create a message queue to receive message from agent. If you already have one
	 * you can reuse the existing one.
	 */
	
	if ((myMsgQId = msgQCreate (MAX_MSGS, sizeof(TrapMsg), MSG_Q_PRIORITY)) 
		== NULL)
		return (ERROR);
	
		/* Your application may need to get the current value during
		 * initialization. If your application were rrm, you would do something like
		 * this
	     */
	
	status = oam_getMibStruct(MY_TABLE_NAME,(char *) &rrmStruct, sizeof(rrmStruct));
	
	if(status != STATUS_OK)
	{
		/* put here your error handling code */
	}
	
    /* Now you can consume the value*/
	
	printf("value of rm_viperCellTsc = %d\n", rrmStruct.rm_viperCellTsc);
	
	/* You can process other members of the sturct here */
	
	/* You may also need to set the trap. The following for loop
	 *  sets the trap for all the member of rrm structure
	 */
	
	for ( i = 0; i < LEAF_rm_synchChanInfo; i++)
	{
		tag = MIB_resourceConfigData+i;
		oam_setTrap(MY_MODULE_ID, myMsgQId, tag);
	}
	
	/* This is your typical message loop where you wait for incoming messages */
	
	while(oamApiDemoTaskQuit)
	{
		int status;
		
		/* get message from queue; if necessary wait for a maximum 1000 ticks */
		status = msgQReceive(myMsgQId, (char *) &myTrapMsg, sizeof(TrapMsg), 1000);
		if (status == ERROR)
		{
			if (errno == S_objLib_OBJ_TIMEOUT)
				
				continue;
		} else {
			
			/* display message */
			printf ("Trap message received\n");
			
			if (myTrapMsg.valType == VT_VALUE)
			{
				printf ("The new value of Mib Variable for tag %d is %d\n", 
					myTrapMsg.mibTag, myTrapMsg.val.varVal);
			} else {
				
				printf ("The new address of Mib Variable for tag %d is %x\n", 
					myTrapMsg.mibTag, myTrapMsg.val.varAdr);
			}
		}
	}

	/*
	* The application need to unsunscribe trap before exiting.
	*/
	oam_unsetTrapByModule(MY_MODULE_ID);

	/* you can also selectively unsubscribe a trap  by unsetTrap() method */
	
return OK;
}

/* to allow demo task exit type "quitOamApiDemoTask(0)" */

quitOamApiDemoTask (int val)
{
	oamApiDemoTaskQuit = val;
}

/* spawns the demo task */
oamDemoTask(void)
{
	taskSpawn("oamAppDemo", 150, VX_PRIVATE_ENV,10000, oamAppDemo, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
}


printH323Mib()
{
	H323Data h323Struct;
	struct in_addr iaddr;
	int status = oam_getMibStruct(H323_TABLE_NAME, (char *) &h323Struct, sizeof(h323Struct));
	
	if(status != STATUS_OK)
	{
		/* put here your error handling code */
		printf("getMibStruct() failed\n");
	}

	iaddr.s_addr = h323Struct.GKIPAddress;
	printf("Gate Keeper's port = %d\n", h323Struct.GKPort);
	printf("Gate Keeper's IP Address = %s\n", inet_ntoa (iaddr));
	printf("Q931 Call Signaling Port = %d\n", h323Struct.Q931CallSignalingPort);
	printf("Q931 Connect Time Out = %d\n", h323Struct.Q931ConnectTimeOut);
	printf("Q931 Response time Out = %d\n", h323Struct.Q931ResponseTimeOut);
	printf("t35 Country Code = %d\n", h323Struct.t35CountryCode);
}


STATUS getRRM(int idx)
{
	
	INT_32_T mibValue;
	int status;
	INT_32_T tag = MIB_ITEM(MTBL_resourceConfigData, idx);
	
	status = oam_getMibIntVar(tag, &mibValue);
	if (status != STATUS_OK)
	{
		printf("getMibIntVar failed; status = %d\n", status);
	} else {
		printf ("The value of the variable is %d\n", mibValue);
	}
	return status;
	
}

STATUS setRRM(int idx, INT_32_T varval)
{
	
	int status;
	INT_32_T tag = MIB_ITEM(MTBL_resourceConfigData, idx);
	
	status = oam_setMibIntVar(SNMP_AGENT_MODULE_ID, tag, varval);
	if (status != STATUS_OK)
	{
		printf("setMibIntVar failed; status = %d\n", status);
	} else {
		printf ("The value of the variable set to %d\n", varval);
	}
	return status;
	
}
