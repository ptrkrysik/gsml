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
// File        : MMLudbMsgProc.cpp
// Author(s)   : Kevin Lim 
// Create Date : 08-09-99 
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "taskLib.h"
#include "tickLib.h"

#include "LOGGING/vclogging.h"
#include "LOGGING/vcmodules.h"

#include "ril3/ril3_common.h"

#include "JCC/LUDBConfig.h"
#include "JCC/LUDBMmMsgProc.h"

#include "MMTypes.h"
#include "MMUtil.h"
#include "MMMdMsgProc.h"
#include "MMCCMsgProc.h"
#include "MMSendMsg.h"
#include "MMLudbMsgProc.h"
#include "MMPmProc.h"
#include "MMRmMsgProc.h"
#include "MMBssgpMsgProc.h"


// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];
extern MSG_Q_ID	ludbMsgQId;
extern short ludb_auth_required;

// ****************************************************************************
bool sendMMRequestToLudb(LudbMsgStruct *request)
{
	DBG_FUNC("mmSendMessageToLudb", MM_LAYER);
	DBG_ENTER();

	request->module_id = MODULE_MM;
	STATUS stat = msgQSend(ludbMsgQId, (char *) request,
							sizeof(LudbMsgStruct),
							NO_WAIT, MSG_PRI_NORMAL);
	if(stat == ERROR){
		DBG_ERROR("msgQSend Error\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendCipherCmdRecordToLudb(short mmId, short algo)
{
	DBG_FUNC("sendCipherCmdRecordToLudb", MM_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbRequest;
	LudbRequest.msg_type = LUDB_MM_CIPHER_SET;
	LudbRequest.mm_id = mmId;
	LudbRequest.ludb_id = mmEntries[mmId].ludbId;
	LudbRequest.algo = algo;
	
	bool status = sendMMRequestToLudb(&LudbRequest);
	if(!status) {
		DBG_ERROR("Security Info Request send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}

	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileSecInfoReqToLudb(short mmId, short ludbId)
{
	DBG_FUNC("sendMobileSecInfoReqToLudb", MM_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbRequest;
	LudbRequest.msg_type = LUDB_MM_SEC_REQ;
	LudbRequest.mm_id = mmId;
	LudbRequest.ludb_id = ludbId;
	
	bool status = sendMMRequestToLudb(&LudbRequest);
	if(!status) {
		DBG_ERROR("Security Info Request send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileRegReqToLudb( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId)
{
	DBG_FUNC("sendMobileRegReqToLudb", MM_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbRequest;
	LudbRequest.msg_type = LUDB_MM_REG_REQ;
	LudbRequest.mobile_id = mobileId;
	LudbRequest.mm_id = mmId;
	
	bool status = sendMMRequestToLudb(&LudbRequest);
	if(!status) {
		DBG_ERROR("Registration send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileUnRegReqToLudb( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId)
{
	DBG_FUNC("sendMobileUnRegReqToLudb", MM_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbRequest;
	LudbRequest.msg_type = LUDB_MM_UNREG_REQ;
	LudbRequest.mobile_id = mobileId;
	LudbRequest.mm_id = mmId;

	bool status = sendMMRequestToLudb(&LudbRequest);
	if(!status) {
		DBG_ERROR("UnRegistration send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendInsertEmergencyToLudb(short mmId)
{
	DBG_FUNC("sendInsertEmergencyToLudb", MM_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbRequest;
	LudbRequest.msg_type = LUDB_MM_EMERG_REQ;
	LudbRequest.mobile_id = mmEntries[mmId].mobileId;
	LudbRequest.mm_id = mmId;

	bool status = sendMMRequestToLudb(&LudbRequest);
	if(!status) {
		DBG_ERROR("UnRegistration send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendRemoveEmergencyToLudb(short mmId)
{
	DBG_FUNC("sendRemoveEmergencyToLudb", MM_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbRequest;
	LudbRequest.msg_type = LUDB_MM_REM_EMERG_REQ;
	LudbRequest.mm_id = mmId;
	LudbRequest.ludb_id = mmEntries[mmId].ludbId;

	bool status = sendMMRequestToLudb(&LudbRequest);
	if(!status) {
		DBG_ERROR("UnRegistration send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
void MM_Ludb_EmergRspProc(short mmId, short ludbId)
{
	DBG_FUNC("MM_Ludb_EmergRspProc", MM_LAYER);
	DBG_ENTER();
	if(mmEntries[mmId].procData.procType == MM_PROC_MOB_ORIG){
		if(mmEntries[mmId].procData.mmState == MM_EMERGENCY){
			mmEntries[mmId].ludbId = ludbId;
			mmEntries[mmId].emerg_ludb = 1;
			MMProceedCMServiceReqAccept(mmId);
			DBG_LEAVE();
			return;
		}
	}
			
	DBG_ERROR("Invalid procData.procType(%d) on LUDB EmergRsp\n",
						mmEntries[mmId].procData.procType);
	DBG_ERROR("Or Invalid procData.mmState(%d) on LUDB EmergRsp\n",
						mmEntries[mmId].procData.mmState);
	
	DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_RelReqProc(short mmId)
{
	DBG_FUNC("MM_Ludb_RelReqProc", MM_LAYER);
	DBG_ENTER();

	sendIntRRMsg (mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);	
	entryDealloc(mmId);

	DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_CipherSetProc(short mmId, short ludbId, short algo)
{
	DBG_FUNC("MM_Ludb_CipherSetProc", MM_LAYER);
	DBG_ENTER();

	ludbSetCipherAlgo(ludbId, algo);

	DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_SecRspProc(short mmId, short ludbId)
{
	DBG_FUNC("MM_Ludb_SecRspProc", MM_LAYER);
	DBG_ENTER();
	switch(mmEntries[mmId].procData.procType){
		case MM_PROC_MOB_ORIG:
			mmEntries[mmId].procData.mmState = MM_AUTH_ORIG;
			sendAuthReqToMobile(mmId, ludbId);
			break;

		case MM_PROC_MOB_TERM:
			mmEntries[mmId].procData.mmState = MM_AUTH_TERM;
			sendAuthReqToMobile(mmId, ludbId);
			break;

		default:
			DBG_ERROR("Invalid procData.procType(%d) on LUDB SecRsp\n",
						mmEntries[mmId].procData.procType);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_RegCnfProc(short mmId, short ludbId, short forceAuth)
{
	DBG_FUNC("MM_Ludb_RegCnfProc", MM_LAYER);
	DBG_ENTER();
	if(mmEntries[mmId].procData.mmState == MM_WAIT4REG)
	{	
		switch(mmEntries[mmId].procData.procType){
			case MM_PROC_LUDB:
	                // if it is initiated from the gs, send location update accept
        	        if (mmEntries[mmId].gsInitiated)
                	{
                    		MM_Bssgp_SendLuAccept(mmId);
                	} 
                	else
                	{
				MMProceedLocUpdateAuth(mmId, ludbId, forceAuth);
                	}
			break;

			case MM_PROC_MOB_ORIG:
				mmEntries[mmId].ludbId = ludbId;
				MMProceedCMServiceReqAuth(mmId);
				break;

			case MM_PROC_MOB_TERM:
				mmEntries[mmId].ludbId = ludbId;
				MMProceedRRPageRspAuth(mmId);  // shmin 08/02/01
				// sendPageReqToRRM(mmId);
				break;

			default:
				DBG_ERROR("Invalid procData.procType(%d) on LUDB RegCnf\n",
							mmEntries[mmId].procData.procType);
		}
	}
	else
	{
		DBG_TRACE("Mobile is not waiting for RCF, MM ID: %d state %d\n", 
				mmId, mmEntries[mmId].procData.mmState);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedLocUpdateAuth(short mmId, short ludbId, short forceAuth)
{
	DBG_FUNC("MMProceedLocUpdateAuth", MM_LAYER);
	DBG_ENTER();

	if(ludb_auth_required){
		mmEntries[mmId].procData.mmState = MM_AUTH_LOCUP;
		if(forceAuth){
			DBG_TRACE("mobile returned cksn %d, ludbcksn %d still use new key\n",
						mmEntries[mmId].lup_cksn,
						ludbCKSN(ludbId));
			sendAuthReqToMobile(mmId, ludbId);
		}
		else{
			if(mmEntries[mmId].lup_cksn == ludbCKSN(ludbId)){
				DBG_TRACE("Skip Auth for mmId: %d\n", mmId);
				MMProceedLocUpdateCipher(mmId, ludbId);
			}
			else{
				DBG_TRACE("mobile returned cksn %d, differ from ludbcksn %d\n",
						mmEntries[mmId].lup_cksn,
						ludbCKSN(ludbId));
				sendAuthReqToMobile(mmId, ludbId);
			}
		}
	}
	else{
		MMProceedLocUpdateCipher(mmId, ludbId);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedLocUpdateCipher(short mmId, short ludbId)
{
	MMProceedLocUpdateRegCnf(mmId, ludbId);
}

// ****************************************************************************
void MMProceedLocUpdateRegCnf(short mmId, short ludbId)
{
    DBG_FUNC("MMProceedLocUpdateRegCnf", MM_LAYER);
	DBG_ENTER();
	unsigned long tick = tickGet();
//	printf("LocationUpdate start tick %d\n", mmEntries[mmId].locupTick);
//	printf("LocationUpdate end tick %d\n", tick);
	tick = tick - mmEntries[mmId].locupTick;
	mmUpdMeanLocUpTime(tick);
 	
	sendLocationUpdateAccept(mmEntries[mmId].oid, 
						mmEntries[mmId].sapi, 
						mmEntries[mmId].entryId,
						getMobileId(mmId));
	entryDealloc(mmId);
    DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_RegRejProc(short mmId, T_CNI_RIL3_REJECT_CAUSE_VALUE cause)
{
	DBG_FUNC("MM_Ludb_RegRejProc", MM_LAYER);
	DBG_ENTER();
	switch(mmEntries[mmId].procData.procType){
		case MM_PROC_LUDB:
			if(mmEntries[mmId].procData.mmState == MM_WAIT4REG){
				sendLocationUpdateReject(mmEntries[mmId].oid, 
							mmEntries[mmId].sapi, 
							mmEntries[mmId].entryId, 
							cause);
				entryDealloc(mmId);
				DBG_LEAVE();
				return;
			}
			break;

		case MM_PROC_MOB_ORIG:
			if(mmEntries[mmId].procData.mmState == MM_AUTH_ORIG){
				MMCMServiceCleanUp(mmId);
				DBG_LEAVE();
				return;
			}
			break;

		case MM_PROC_MOB_TERM:
			if(mmEntries[mmId].procData.mmState == MM_AUTH_TERM){
				MMPageReqCleanUp(mmId);
				DBG_LEAVE();
				return;
			}
			break;
			
		default:
			DBG_ERROR("Invalid procData.procType(%d) on LUDB RegRej\n",
						mmEntries[mmId].procData.procType);
	}
	
	DBG_TRACE("Mobile is not waiting for RRJ, MM ID: %d proc %s state %s \n", mmId, 
				getMMProcStateString(mmEntries[mmId].procData.procType),
				getMMStateString(mmEntries[mmId].procData.mmState));
	DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_UnRegCnfProc(short mmId)
{
	DBG_FUNC("MM_Ludb_UnRegCnfProc", MM_LAYER);
	DBG_ENTER();
	if(mmEntries[mmId].procData.mmState == MM_WAIT4UNREG)
	{
		if(mmEntries[mmId].mmId2 != JCC_ERROR){ 
			short mmId2 = mmEntries[mmId].mmId2;
			mmEntries[mmId2].cause = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;  
			sendIntCCMsg (mmId2, INTRA_L3_MM_REL_IND, INTRA_L3_NULL_MSG);
			entryDealloc(mmId2);
		}

		sendIntRRMsg (mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		entryDealloc(mmId);
	}
	else
	{
		DBG_TRACE("Mobile is not waiting for UCF, MM ID: %d state %d\n", 
				mmId, mmEntries[mmId].procData.mmState);
	}
	DBG_LEAVE();
}
	
// ****************************************************************************
void MM_Ludb_UnRegRejProc(short mmId)
{
	DBG_FUNC("MM_Ludb_UnRegRejProc", MM_LAYER);
	DBG_ENTER();
	if(mmEntries[mmId].procData.mmState == MM_WAIT4UNREG)
	{
		if(mmEntries[mmId].mmId2 != JCC_ERROR){ 
			sendIntCCMsg (mmEntries[mmId].mmId2, INTRA_L3_MM_REL_IND, INTRA_L3_NULL_MSG);
			entryDealloc(mmEntries[mmId].mmId2);
		}

		sendIntRRMsg (mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		entryDealloc(mmId);
	}
	else
	{
		DBG_TRACE("Mobile is not waiting for URJ, MM ID: %d state %d\n",
				mmId, mmEntries[mmId].procData.mmState);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_Ludb_MsgProc(LudbMsgStruct *LudbInMsg)
{
	DBG_FUNC("MM_Ludb_MsgProc", MM_LAYER);
	DBG_ENTER();
	// The connection entry for MM
	short	mmId;
	
	mmId = LudbInMsg->mm_id;

	switch(LudbInMsg->msg_type)
	{
		case LUDB_MM_REL_REQ:
			DBG_TRACE("LUDB_MM_REL_REQ Received mmId %d\n", mmId);
			MM_Ludb_RelReqProc(mmId);
			break;

		case LUDB_MM_REG_CNF:
			DBG_TRACE("LUDB_MM_REG_CNF Received mmId %d\n", mmId);
			MM_Ludb_RegCnfProc(mmId, LudbInMsg->ludb_id, LudbInMsg->forceAuth);
			break;

		case LUDB_MM_REG_REJ:
			DBG_TRACE("LUDB_MM_REG_REJ Received mmId %d\n", mmId);
			MM_Ludb_RegRejProc(mmId, LudbInMsg->cause);
			break;

		case LUDB_MM_UNREG_CNF:
			DBG_TRACE("LUDB_MM_UNREG_CNF Received mmId %d\n", mmId);
			MM_Ludb_UnRegCnfProc(mmId);
			break;
						
		case LUDB_MM_UNREG_REJ:
			DBG_TRACE("LUDB_MM_UNREG_REJ Received mmId %d\n", mmId);
			MM_Ludb_UnRegRejProc(mmId);
			break;

		case LUDB_MM_SEC_RSP:
			DBG_TRACE("LUDB_MM_SEC_RSP Received mmId %d\n", mmId);
			MM_Ludb_SecRspProc(mmId, LudbInMsg->ludb_id);
			break;

		case LUDB_MM_EMERG_RSP:
			DBG_TRACE("LUDB_MM_EMERG_RSP Received mmId %d\n", mmId);
			MM_Ludb_EmergRspProc(mmId, LudbInMsg->ludb_id);
			break;

		case LUDB_MM_CIPHER_SET:
			DBG_TRACE("LUDB_MM_SEC_RSP Received mmId %d\n", mmId);
			MM_Ludb_CipherSetProc(mmId, LudbInMsg->ludb_id, LudbInMsg->algo);
			break;


		default:
			DBG_ERROR("Unexpected message type %d for Ludb\n", LudbInMsg->msg_type);
	}
	DBG_LEAVE();
}	 

