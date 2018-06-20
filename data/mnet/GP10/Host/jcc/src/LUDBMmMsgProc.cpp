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
// File        : LUDBMmMsgProc.cpp
// Author(s)   : Kevin S. Lim
// Create Date : 07-31-99
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

// from common vob
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"

// included L3 Message header for messages from other Layer 3 modules
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/JCCTimer.h"
#include "JCC/LUDB.h"
#include "JCC/LUDBapi.h"
#include "JCC/LUDBConfig.h"

#include "JCC/LUDBMmMsgProc.h"
#include "LUDBVoipMsgProc.h"

// Global variable for Message Queue
extern MSG_Q_ID mmMsgQId;
extern LUDB		gLUDB;
extern short	ludb_auth_required;

// ****************************************************************************
bool sendLudbRspToMM(LudbMsgStruct *rsp)
{
	DBG_FUNC("sendLudbRspToMM", LUDB_LAYER);
	DBG_ENTER();

	rsp->module_id = MODULE_LUDB;
	STATUS stat = msgQSend(mmMsgQId, (char *) rsp,
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
bool sendMobileRelReqToMM(short mmId)
{
	DBG_FUNC("sendMobileRelReqToMM", LUDB_LAYER);
	DBG_ENTER();

	LudbMsgStruct LudbResponse;
	LudbResponse.msg_type = LUDB_MM_REL_REQ;
	LudbResponse.mm_id = mmId;

	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}
	
// ****************************************************************************
bool sendMobileRegCnfToMM(short mmId, short ludbId, short forceAuth)
{
	DBG_FUNC("sendMobileRegCnfToMM", LUDB_LAYER);
    DBG_ENTER();

	gLUDB.update(ludbId, LUDBEntry::ACTIVE);

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_REG_CNF;
	LudbResponse.mm_id = mmId;
	LudbResponse.ludb_id = ludbId;
	LudbResponse.forceAuth = forceAuth;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendEmergRspToMM(short mmId, short ludbId)
{
	DBG_FUNC("sendEmergRspToMM", LUDB_LAYER);
    DBG_ENTER();

	gLUDB.update(ludbId, LUDBEntry::EMERGENCY);

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_EMERG_RSP;
	LudbResponse.mm_id = mmId;
	LudbResponse.ludb_id = ludbId;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendRemEmergRspToMM(short mmId)
{
	DBG_FUNC("sendRemEmergRspToMM", LUDB_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_REM_EMERG_CNF;
	LudbResponse.mm_id = mmId;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileSecRspToMM(short mmId, short ludbId)
{
	DBG_FUNC("sendMobileSecRspToMM", LUDB_LAYER);
    DBG_ENTER();

	gLUDB.update(ludbId, LUDBEntry::ACTIVE);

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_SEC_RSP;
	LudbResponse.mm_id = mmId;
	LudbResponse.ludb_id = ludbId;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileRegRejToMM(short mmId, T_CNI_RIL3_REJECT_CAUSE_VALUE cause)
{
	DBG_FUNC("sendMobileRegRejToMM", LUDB_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_REG_REJ;
	LudbResponse.mm_id = mmId;
	LudbResponse.cause = cause;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileUnRegCnfToMM(short mmId)
{
	DBG_FUNC("sendMobileUnRegCnfToMM", LUDB_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_UNREG_CNF;
	LudbResponse.mm_id = mmId;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileUnRegRejToMM(short mmId)
{
	DBG_FUNC("sendMobileUnRegRejToMM", LUDB_LAYER);
    DBG_ENTER();

	LudbMsgStruct	LudbResponse;
	LudbResponse.msg_type = LUDB_MM_UNREG_REJ;
	LudbResponse.mm_id = mmId;
	
	bool status = sendLudbRspToMM(&LudbResponse);
	if(!status) {
		DBG_ERROR("LudbRespsonse send unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
void LUDB_MM_EmergReqProc(T_CNI_RIL3_IE_MOBILE_ID& mobile_id, short mmId)
{
	DBG_FUNC("LUDB_MM_RegReqProc", LUDB_LAYER);
	DBG_ENTER();

	short ludb_id = gLUDB.registerUser(mobile_id);
	if(ludb_id != -1){
		T_SUBSC_INFO subsc;	
		char msisdn[MAX_NUM_SUBSC_ISDN];
		char imsi[MAX_NUM_SUBSC_IMSI];
		int i, len;

		subscInfoInitialize(&subsc);
		switch(mobile_id.mobileIdType){
			case CNI_RIL3_IMSI:
				len = (mobile_id.numDigits > MAX_NUM_SUBSC_IMSI)?
							MAX_NUM_SUBSC_IMSI:mobile_id.numDigits;
				for(i=0; i<len; i++)
					imsi[i] = mobile_id.digits[i] + '0';
				imsi[i] = '\0';
				subscSetIMSI(&subsc, imsi);
			case CNI_RIL3_IMEI:
			case CNI_RIL3_IMEISV:
				len = (mobile_id.numDigits > MAX_NUM_SUBSC_ISDN)?
							MAX_NUM_SUBSC_ISDN:mobile_id.numDigits;
				for(i=0; i<len; i++)
					msisdn[i] = mobile_id.digits[i] + '0';
				msisdn[i] = '\0';
				break;
			default:
				DBG_ERROR("unsupported mobile id for EmergencyReq %d\n", 
							mobile_id.mobileIdType);
				msisdn[0] = '\0';
		}
		subscSetMSISDN(&subsc, msisdn);
		subscSetUserInfo(&subsc, "ViperCell");
		ludbSetSubscInfoExists(ludb_id, true);
		gLUDB.update(ludb_id, subsc);

		bool status = sendEmergRspToMM(mmId, ludb_id);
		if(!status){
			ludbEntryInit(ludb_id);	
		}
	}
	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_MM_RemEmergReqProc(short mmId, short ludb_id)
{
	DBG_FUNC("LUDB_MM_RemEmergReqProc", LUDB_LAYER);
	DBG_ENTER();

	if(ludb_id != -1){
		LUDBEntry::LUDBState state = gLUDB.getState(ludb_id);
		if(state == LUDBEntry::EMERGENCY){
			ludbEntryInit(ludb_id);			
		}
		else{
			DBG_ERROR("ludb entry not expecting REM_EMERG_REQ %d\n", ludb_id);	
		}
	}
	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_MM_RegReqProc(T_CNI_RIL3_IE_MOBILE_ID& mobile_id, short mmId)
{
	DBG_FUNC("LUDB_MM_RegReqProc", LUDB_LAYER);
	DBG_ENTER();
	short ludb_id = gLUDB.getEntry(mobile_id);

	if(ludb_id == JCC_ERROR){	// mobile doesn't exist in LUDB
		ludb_id = gLUDB.registerUser(mobile_id);
		if(ludb_id != JCC_ERROR){
			bool status = sendMobileRegReq(mobile_id, mmId, ludb_id, ludb_auth_required);
			if(!status){
				ludbEntryInit(ludb_id);	
				sendMobileRegRejToMM(mmId, CNI_RIL3_REJECT_NETWORK_FAILURE);	
			}
		}
		else{
			DBG_ERROR("registerUser failed\n");
			sendMobileRegRejToMM(mmId, CNI_RIL3_REJECT_CONGESTION);	
		}
	}
	else{						// mobile exists in LUDB
		LUDBEntry::LUDBState state = gLUDB.getState(ludb_id);
//		if(state == LUDBEntry::ACTIVE){
//			sendMobileRegCnfToMM(mmId, ludb_id);
//		}
//		else if((state == LUDBEntry::INACTIVE)||
//			    (state == LUDBEntry::LUUNREGISTRJ)){
		if((state == LUDBEntry::ACTIVE)||
			(state == LUDBEntry::UNREGREQ) ||
			(state == LUDBEntry::INACTIVE)){
			bool status = sendMobileRegReq(mobile_id, mmId, ludb_id, ludb_auth_required);	
			if(!status){
				ludbEntryInit(ludb_id);	
				sendMobileRegRejToMM(mmId, CNI_RIL3_REJECT_NETWORK_FAILURE);	
			}
		}
		else{
			DBG_ERROR("Ignored: LUDB entry %d was waiting for Network response.\n", ludb_id);
			sendMobileRelReqToMM(mmId);
		}
	}
	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_MM_UnRegReqProc(T_CNI_RIL3_IE_MOBILE_ID& mobile_id, short mmId)
{
	DBG_FUNC("LUDB_MM_UnRegReqProc", LUDB_LAYER);
	DBG_ENTER();
	short	ludb_id = gLUDB.getEntry(mobile_id);
	
	if(ludb_id != JCC_ERROR){	// mobile exists in LUDB
		LUDBEntry::LUDBState state = gLUDB.getState(ludb_id);

		if((state == LUDBEntry::UNREGREQ) ||
		   (state == LUDBEntry::INACTIVE)) {
			sendMobileUnRegRejToMM(mmId);
		}
		else{
			ludbStartPurgeTimer(ludb_id);
			gLUDB.update(ludb_id, LUDBEntry::INACTIVE);
			bool status = sendMobileUnRegCnfToMM(mmId);
			if(!status) {
				DBG_ERROR("UnRegistration Confirm send to MM failed\n");
			}

		//	VOIP_MOBILE_HANDLE mobile_handle = gLUDB.getMobileHandle(ludb_id);
		//	DBG_TRACE("Unregister mobile handle %d\n", (int)mobile_handle);
		//	bool status = sendMobileUnRegReq(mobile_handle, mmId, ludb_id);
		//	if(!status){
		//		sendMobileUnRegRejToMM(mmId);	
		//	}
		}
	}
	else{						// mobile doesn't exist in LUDB
		DBG_ERROR("Received IMSI Detach but Mobile not in LUDB\n");		
		sendMobileUnRegRejToMM(mmId);	
	}
	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_MM_SecReqProc(short mm_id, short ludb_id)
{
	DBG_FUNC("LUDB_MM_SecReqProc", LUDB_LAYER);
	DBG_ENTER();
	
	T_CNI_RIL3_IE_MOBILE_ID mobile_id = ludbGetMobileId(ludb_id);
	ludbSetMMId(ludb_id, mm_id);
			
	bool status = sendMobileSecInfoReq(mobile_id, mm_id, ludb_id, 0);
	if(!status) {
		DBG_ERROR("Security Request send to VB failed\n");
	}

	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_MM_CipherSetProc(short mm_id, short ludb_id, short algo)
{
	DBG_FUNC("LUDB_MM_CipherSetProc", LUDB_LAYER);
	DBG_ENTER();
	
	ludbSetCipherAlgo(ludb_id, algo);

	DBG_LEAVE();
}

// ****************************************************************************
void LUDB_MM_MsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_MM_MsgProc", LUDB_LAYER);
	DBG_ENTER();
	short mmId = inMsg->mm_id;
	T_CNI_RIL3_IE_MOBILE_ID mobile_id;

	switch(inMsg->msg_type)
	{
		case LUDB_MM_REG_REQ:
			DBG_TRACE("LUDB_MM_REG_REQ Received mmId %d\n", mmId);
			mobile_id = inMsg->mobile_id;
			LUDB_MM_RegReqProc(mobile_id, mmId);
			break;

		case LUDB_MM_UNREG_REQ:
			DBG_TRACE("LUDB_MM_UNREG_REQ Received mmId %d\n", mmId);
			mobile_id = inMsg->mobile_id;
			LUDB_MM_UnRegReqProc(mobile_id, mmId);
			break;

		case LUDB_MM_SEC_REQ:
			DBG_TRACE("LUDB_MM_SEC_REQ Received mmId %d\n", mmId);
			LUDB_MM_SecReqProc(mmId, inMsg->ludb_id);
			break;
						
		case LUDB_MM_EMERG_REQ:
			DBG_TRACE("LUDB_MM_EMERG_REQ Received mmId %d\n", mmId);
			mobile_id = inMsg->mobile_id;
			LUDB_MM_EmergReqProc(mobile_id, mmId);
			break;

		case LUDB_MM_REM_EMERG_REQ:
			DBG_TRACE("LUDB_MM_REM_EMERG_REQ Received mmId %d\n", mmId);
			LUDB_MM_RemEmergReqProc(mmId, inMsg->ludb_id);
			break;

		case LUDB_MM_CIPHER_SET:
			DBG_TRACE("LUDB_MM_CIPHER_SET Received ludbId %d\n", inMsg->ludb_id);
			LUDB_MM_CipherSetProc(mmId, inMsg->ludb_id, inMsg->algo);
			break;

			
						
		default:
			DBG_ERROR("Ludb Error:Unexpected message type %d\n", inMsg->msg_type);
	}
	DBG_LEAVE();
}

