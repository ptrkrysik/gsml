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
// File        : LudbOamMsgProc.cpp
// Author(s)   : Kevin Lim 
// Create Date : 09-09-99
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "stdio.h"
#include "string.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"

// Ludb Message Constants
#include "LudbOamMsgProc.h"
#include "JCC/LudbApi.h"

// OAM Config Data

// Global variable for Message Queue
extern MSG_Q_ID ludbMsgQId;


// ****************************************************************************
void ludbHandleOpStChanged(INT32 value)
{
	DBG_FUNC("ludbHandleOpStChange", LUDB_LAYER);
	DBG_ENTER();
	if(value){
	}
	else{
		ludbDataInit();
	}	
	DBG_LEAVE();
}

// ****************************************************************************
void ludbHandleAuthFreqChanged(INT32 value)
{
	DBG_FUNC("ludbHandleAuthFreqChange", LUDB_LAYER);
	DBG_ENTER();
	ludbSetAuthFreq((int)value);
	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_OAM_MsgProc(TrapMsg *trapMsg)
{
	DBG_FUNC("LUDB_OAM_MsgProc", LUDB_LAYER);
	DBG_ENTER();
	
	if(trapMsg->valType != VT_VALUE){
		DBG_ERROR("Unexpected valType from OAM trap message: %d\n", trapMsg->valType);
		DBG_LEAVE();
		return;
	}

	switch(trapMsg->mibTag)
	{
		case MIB_bts_operationalState:
			DBG_TRACE("ludbOpStatus MIB Trap received : %d\n", trapMsg->val.varVal);
			ludbHandleOpStChanged(trapMsg->val.varVal);
			break;
		case MIB_rm_nim_0_7:
			DBG_TRACE("ludbOpStatus MIB Trap received : %d\n", trapMsg->val.varVal);
			ludbHandleAuthFreqChanged(trapMsg->val.varVal);
			break;
  
		default:
			DBG_ERROR("Unexpected mib tag %d\n", trapMsg->mibTag); 
	}
	DBG_LEAVE();
}	 
