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
// File        : LUDBTask.cpp
// Author(s)   : Kevin S. Lim
// Create Date : 04-27-99
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "stdLib.h"
#include "vxWorks.h"
#include "taskLib.h"
#include "usrLib.h"
#include "errnoLib.h"
#include "string.h"

#include "Os/JCTask.h"
#include "Os/JCModule.h"
#include "GP10OsTune.h"
#include "LOGGING//vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"

// included L3 Message header for messages from other Layer 3 modules
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/JCCTimer.h"

#include "JCC/Ludb.h"
#include "JCC/LUDBConfig.h"
#include "JCC/LUDBApi.h"
#include "JCC/LUDBMmMsgProc.h"

#include "LUDBVoipMsgProc.h"
#include "LUDBOamMsgProc.h"
#include "LUDBTimerProc.h"
#include "LudbHashTbl.h"

extern LudbHashTbl *ludbImsiHash;

extern MSG_Q_ID	ludbMsgQId;
JCTask ludbTaskObj("LUDBTask");

// function forward declaration
int ludbTaskMain();
void ludbTaskInit();


// ****************************************************************************
int	SysCommand_Ludb(T_SYS_CMD	action)
{
	int tid;

	switch(action){
		case SYS_SHUTDOWN:
			printf("[LUDB] Received system shutdown notification\n");
			ludbDataPrint();
			break;
		case SYS_START:
			printf("[LUDB] Received task start notification\n");
			ludbTaskInit();
			tid = ludbTaskObj.JCTaskSpawn(
							LUDB_TASK_PRIORITY,
							LUDB_TASK_OPTION,
							LUDB_TASK_STACK_SIZE,
							(FUNCPTR)ludbTaskMain,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							MODULE_LUDB, JC_CRITICAL_TASK);				
			break;
		case SYS_REBOOT:
			delete ludbImsiHash;
			printf("[LUDB] Reboot ready.\n");
			break;
		default:
			printf("[LUDB] Unknown system command received\n");
	}
	return 0;
}

// *******************************************************************
void ludbCfgRead()
{
	INT_32_T	oam_freq;	
	DBG_FUNC("ludbCfgRead", MM_LAYER);
	DBG_ENTER();

	STATUS ret;

	if(ret = oam_getMibIntVar(MIB_rm_nim_0_7, &oam_freq)){
		DBG_ERROR("Failed to initialize auth_freq with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
	}
	else{
		ludbHandleAuthFreqChanged(oam_freq);
	}
	DBG_LEAVE();
}
	
// *******************************************************************
void ludbSetOAMTraps()
{
	DBG_FUNC("ludbSetOAMTraps", LUDB_LAYER);
	DBG_ENTER();
	STATUS oamStatus;

	if((oamStatus = oam_setTrap(MODULE_LUDB, ludbMsgQId, MIB_bts_operationalState))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_bts_operationalState\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_LUDB, ludbMsgQId, MIB_rm_nim_0_7))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_rm_nim_0_7\n", oamStatus);
	}
	DBG_LEAVE();
}

// *******************************************************************
void ludbTaskInit()
{
	DBG_FUNC("ludbTaskInit", LUDB_LAYER);
	DBG_ENTER();

	ludbCfgRead();

	ludbDataInit();
	
	ludbImsiHash = new LudbHashTbl(LUDB_IMSI_HASH_NUM_BUCKET, LUDB_MAX_IMSI_KEY_LEN);

	// create message queue
	ludbMsgQId = msgQCreate(LUDB_MAX_INCOMING_MSG,
							LUDB_MAX_MSG_LENGTH,
							MSG_Q_FIFO);

	if(ludbMsgQId == NULL){
		DBG_ERROR("Fatal Error on msgQCreate : %d\n", errnoGet());
		return;
	}

	ludbSetOAMTraps();
	
	DBG_LEAVE();
}

// *******************************************************************
int ludbTaskMain()
{
	DBG_FUNC("ludbTaskMain", LUDB_LAYER);
	DBG_ENTER();

	char inMsg[LUDB_MAX_MSG_LENGTH];

	ludbTaskObj.JCTaskEnterLoop();

	while(true)
	{
		if(msgQReceive(	ludbMsgQId, inMsg,
						LUDB_MAX_MSG_LENGTH,
						WAIT_FOREVER) 
						== ERROR)
		{
			DBG_ERROR("Fatal Error on msgQReceive : %d\n", errnoGet());
			DBG_LEAVE();
			return ERROR;
		}

		switch(inMsg[0]){
			case MODULE_MM:
				LUDB_MM_MsgProc((LudbMsgStruct *)inMsg);
				break;

			case MODULE_H323:
				LUDB_VBLINK_MsgProc((LudbMsgStruct *)inMsg);
				break;

			case MODULE_OAM:
				LUDB_OAM_MsgProc((TrapMsg *)inMsg);
				break;

			case MODULE_LUDB:
				LudbTimerMsgProc((LudbMsgStruct *)inMsg);
				break;

			default:
				DBG_ERROR("Message from unexpected Module: %d\n", inMsg[0]);
				DBG_LEAVE();
				return ERROR;
		}
	}
    ludbTaskObj.JCTaskNormExit();
	DBG_LEAVE();
	return 0;
}

