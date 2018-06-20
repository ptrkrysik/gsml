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
// File        : LudbTimerProc.cpp
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

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/JCCTimer.h"

// MM Message Constants
#include "JCC/LUDB.h"
#include "JCC/LUDBApi.h"
#include "JCC/LUDBMMMsgProc.h"
#include "LUDBVoipMsgProc.h"

extern LUDB				gLUDB;

// ****************************************************************************
void LudbTimerMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_Timer_MsgProc", LUDB_LAYER);
	DBG_ENTER();
	
	short ludb_id = inMsg->ludb_id;
	short mmId = ludbGetMMId(ludb_id);	
	bool status;
	switch(inMsg->msg_type)
	{
		case LUDB_VBREQ_TIMEOUT:
				DBG_TRACE("ludb %d vb request timeout\n", ludb_id);
				status = sendMobileRegRejToMM(mmId, CNI_RIL3_REJECT_NETWORK_FAILURE);
				if(!status) {
					DBG_ERROR("Registration Reject send to MM failed\n");
				}
				if(gLUDB.getState(ludb_id) == LUDBEntry::SECREQ)
					gLUDB.update(ludb_id, LUDBEntry::ACTIVE);
				else
					ludbEntryInit(ludb_id);	
				break;
		case LUDB_PURGE_TIMEOUT:
				DBG_TRACE("ludb %d purge timeout\n", ludb_id);

				T_CNI_RIL3_IE_MOBILE_ID mobile_id = ludbGetMobileId(ludb_id);
				status = sendMobileUnRegReq(mobile_id, mmId, ludb_id);
				if(!status){
					DBG_ERROR("UnRegistration Request send to VB failed\n");
				}
			//	ludbEntryInit(ludb_id);	
				break;
		default:
			DBG_TRACE("Unknown message received from Timer, Msg Type: %d\n", inMsg->msg_type);
	}
	DBG_LEAVE();
}

