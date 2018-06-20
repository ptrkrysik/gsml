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
// File        : MMOamMsgProc.cpp
// Author(s)   : Kevin Lim 
// Create Date : 07-23-99
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "taskLib.h"
#include "stdLib.h"
#include "string.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"
#include "ril3/ie_location_area_id.h"

// MM Message Constants
#include "MM/MMConfig.h"
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMOamMsgProc.h"

// OAM Config Data
extern MobilityConfigData	mmCfg;
extern T_CNI_RIL3_IE_LOCATION_AREA_ID	mmCfgLocId;
extern int								mmOpStatus;

// Global variable for Message Queue
extern MSG_Q_ID mmMsgQId;


// ****************************************************************************
void mmHandleOpStChanged(INT32 value)
{
	DBG_FUNC("mmHandleOpStChange", MM_LAYER);
	DBG_ENTER();
	if(mmOpStatus == 1){
		if(value){
		}
		else{
			// deactivate MM
			mmOpStatus = 0;
			mmDataInit();
		}	
	}
	else{
		if(value){
			// activate MM
			mmOpStatus = 1;
		}
		else{
		}
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_OAM_MsgProc(TrapMsg *trapMsg)
{
	DBG_FUNC("MM_OAM_MsgProc", MM_LAYER);
	DBG_ENTER();
	
	if(trapMsg->valType != VT_VALUE){
		DBG_ERROR("Unexpected valType from OAM trap message: %d\n", trapMsg->valType);
		DBG_LEAVE();
		return;
	}

	switch(trapMsg->mibTag)
	{
/*
		case MIB_mm_max_mm_connections:
			DBG_TRACE("mm_max_mm_connections MIB Trap received\n");
			mmCfgSetMaxMMConnections(trapMsg->val.varVal);			
			break;
*/
			
		case MIB_t3113:
			DBG_TRACE("t3113 MIB Trap received\n");
			mmCfgSetPagingT3113(trapMsg->val.varVal);			
			break;

		case MIB_mm_authenticationRequired:
			DBG_TRACE("mm_authentication_required MIB Trap received\n");
			mmCfgSetAuthenticationRequired(trapMsg->val.varVal);			
			break;
						
		case MIB_mm_cipheringRequired:
			DBG_TRACE("mm_ciphering_required MIB Trap received\n");
			mmCfgSetCipheringRequired(trapMsg->val.varVal);			
			break;

		case MIB_mm_IMEICheckRequired:
			DBG_TRACE("mm_imei_check_required MIB Trap received\n");
			mmCfgSetIMEICheckRequired(trapMsg->val.varVal);			
			break;

		case MIB_rm_nim_1_0:
			DBG_TRACE("mm_imsi_requried MIB Trap received\n");
			mmCfgSetIMSIRequired(trapMsg->val.varVal);			
			break;

		case MIB_bts_mcc:
			DBG_TRACE("mm_bts_mcc MIB Trap received\n");
			mmConvertMCC(trapMsg->val.varVal);
			break;

		case MIB_bts_mnc:
			DBG_TRACE("mm_bts_mnc MIB Trap received\n");
			mmConvertMNC(trapMsg->val.varVal);
			break;

		case MIB_bts_lac:
			DBG_TRACE("mm_bts_lac MIB Trap received\n");
			mmConvertLAC(trapMsg->val.varVal);
			break;

		case MIB_bts_operationalState:
			DBG_TRACE("mmOpStatus MIB Trap received\n");
			mmHandleOpStChanged(trapMsg->val.varVal);
			break;
 
		default:
			DBG_ERROR("Unexpected mib tag %d\n", trapMsg->mibTag); 
	}
	DBG_LEAVE();
}	 
