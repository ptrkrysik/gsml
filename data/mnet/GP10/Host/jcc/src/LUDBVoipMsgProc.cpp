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
// File        : LUDBVoipMsgProc.cpp
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
#include "sysLib.h"
#include "usrLib.h"
#include "errnoLib.h"
#include "string.h"

#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"

// included L3 Message header for messages from other Layer 3 modules
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/JCCTimer.h"

#include "JCC/LUDB.h"
#include "JCC/LUDBConfig.h"
#include "JCC/LUDBApi.h"
#include "JCC/LUDBMMMsgProc.h"

#include "LUDBVoipMsgProc.h"

#include "voip/vblink.h"
#include "cc/cc_util_ext.h"

extern LUDB		gLUDB;
extern short	ludb_auth_required;

extern void ludbMakeDefaultSubscMsg(char *msg, short *size);

bool ludbRunRegTest = false;
T_CNI_RIL3_IE_MOBILE_ID		ludbRunRegTestMob;
WDOG_ID ludbRunRegTestWID;
int ludbRunRegTestTick;

bool ludbAutoLog = false;

bool ludb_auth_testmode = false;
char ludb_auth_testcksn = 0;
T_AUTH_TRIPLET				cipher_testtriplet;

// ****************************************************************************
// StartAuthMode sets test mode for Authentication testing.
// ****************************************************************************
int StartAuthMode(int cksn, unsigned char lsb)
{
	int i;
	if(ludb_auth_testmode){
		printf("[LUDB] Test Authentication Mode already turned on.\n");
		return -1;
	}
	else{
		ludb_auth_testmode = 1;
		ludb_auth_required = 1;
		if(cksn >= 7)
			printf(" CKSN value should be smaller than 7\n");
		else
			ludb_auth_testcksn = cksn;

		for(i=0; i<SEC_RAND_LEN; i++) cipher_testtriplet.RAND[i] = 0xFF;
		cipher_testtriplet.RAND[SEC_RAND_LEN-1] = lsb;
	
		printf("\n[LUDB] Test Authentication Mode is now turned on.\n");
		printf("(Mobile Location Update will initiate authentication without VB auth.)\n\n");
		printf("StartAuthMode(cksn=%d, Random LSB=0x%x)\n", cksn, lsb);
		printf("Random is set to: 0x");
		for(i=0; i<SEC_RAND_LEN; i++) printf("%02x", cipher_testtriplet.RAND[i]);
		printf("\n");
		return 0;
	}
}

// ****************************************************************************
int StopAuthMode()
{
	if(!ludb_auth_testmode){
		printf("[LUDB] Test Authentication Mode already turned off.\n");
		return -1;
	}
	else{
		ludb_auth_testmode = 0;
		ludb_auth_required = 0;
		printf("[LUDB] Test Authentication Mode is now turned off.\n");
		printf("    (Mobile Location Update will work normal.)\n");
		return 0;
	}
}

// ****************************************************************************
void ludbSendRegTest()
{
	if(!ludbRunRegTest) return;

	short ludbId = ludbGetEntryByMobileId(ludbRunRegTestMob);

	bool status;
	if(ludbId > -1){
		printf("\n[LUDB] Sending UnReg Request.\n");
		status = VBLinkUnregisterMobile(ludbRunRegTestMob);
		if(!status) {
			printf("UnRegistration API call unsuccessful\n");
		}
		else{
			printf("ludb %d ->VBLink MobileUnRegRequest\n", ludbId);
			ludbStartVBReqTimer(ludbId);
			gLUDB.update(ludbId, LUDBEntry::UNREGREQ);
		}
	}	
	else{
		ludbId = gLUDB.registerUser(ludbRunRegTestMob);
		printf("\n[LUDB] Sending Reg Request.\n");
		status = VBLinkRegisterMobile(ludbRunRegTestMob);		
		if(!status) {
			printf("Registration API call unsuccessful\n");
		}
		else{
			printf("ludb %d ->VBLink MobileRegRequest\n", ludbId);
			ludbStartVBReqTimer(ludbId);
			gLUDB.update(ludbId, LUDBEntry::REGREQ);
		}
	}

	if(ludbRunRegTest){
		if(wdStart(ludbRunRegTestWID, ludbRunRegTestTick, 
			(FUNCPTR)ludbSendRegTest, ludbId)){
			printf("[LUDB] wdStart Error!\n");
		}
	}

}

// ****************************************************************************
int ludbRegTestStart(char *imsi, int interval)
{
	if(imsi == 0){
		printf("usage: ludbRegTestStart(char *imsi, int interval) - interval in seconds\n");
		return -1;
	}
	if(interval == 0){
		printf("usage: ludbRegTestStart(char *imsi, int interval) - interval in seconds\n");
		return -1;
	}
	if(ludbRunRegTest){
		printf("[LUDB] RegTest already started.\n");
		return -1;
	}
	else{
		printf("[LUDB] RegTest is now started.\n");
		printf("[LUDB] IMSI: %s\n", imsi);
		printf("[LUDB] every %d seconds\n", interval);
		int len = strlen(imsi);

		ludbRunRegTestMob.mobileIdType = CNI_RIL3_IMSI;
		ludbRunRegTestMob.ie_present = true;
		ludbRunRegTestMob.numDigits = len;
		for(int i=0; i<len; i++){
			ludbRunRegTestMob.digits[i] = imsi[i] - '0';
		}
		ludbRunRegTest = true;
		ludbRunRegTestWID = wdCreate();
		ludbRunRegTestTick = interval * sysClkRateGet();
		ludbSendRegTest();
		return 0;
	}
}

// ****************************************************************************
int ludbRegTestStop()
{
	if(!ludbRunRegTest){
		printf("[LUDB] RegTest already stopped.\n");
		return -1;
	}
	else{
		printf("[LUDB] RegTest is now stopped.\n");
		ludbRunRegTest = false;

		return 0;
	}
}

// ****************************************************************************
// message sent to vblink
// RRQ
// URQ
// SRQ
// ****************************************************************************
bool sendMobileRegReq( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId, short ludbId, short fSec)
{
	DBG_FUNC("sendMobileRegReq", LUDB_LAYER);
    DBG_ENTER();

	ludbStopPurgeTimer(ludbId);	// stop purge timer if it was running

	ludbSetMMId(ludbId, mmId);

	bool status = VBLinkRegisterMobile(mobileId);
	if(!status) {
		DBG_ERROR("Registration API call unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	else{
		DBG_TRACE("ludb %d ->VBLink MobileRegRequest\n", ludbId);
		ludbStartVBReqTimer(ludbId);
		if(fSec)
			gLUDB.update(ludbId, LUDBEntry::REGTHENSEC);
		else
			gLUDB.update(ludbId, LUDBEntry::REGREQ);
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileUnRegReq( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId, short ludbId)
{
	DBG_FUNC("sendMobileUnRegReq", LUDB_LAYER);
    DBG_ENTER();

	bool status = VBLinkUnregisterMobile(mobileId);
	if(!status) {
		DBG_ERROR("UnRegistration API call unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	else{
		DBG_TRACE("ludb %d ->VBLink MobileUnRegRequest\n", ludbId);
		ludbStartVBReqTimer(ludbId);
		gLUDB.update(ludbId, LUDBEntry::UNREGREQ);
	}
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
bool sendMobileSecInfoReq( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId, short ludbId, short fReg)
{
	DBG_FUNC("sendMobileSecInfoReq", LUDB_LAYER);
    DBG_ENTER();

	ludbSetMMId(ludbId, mmId);

	bool status = VBLinkSecurityRequest(mobileId);
	if(!status) {
		DBG_ERROR("Security Info Req API call unsuccessful\n");
		DBG_LEAVE();
		return false;
	}
	else{
		DBG_TRACE("ludb %d ->VBLink SecurityInfoRequest\n", ludbId);
		ludbStartVBReqTimer(ludbId);
		if(fReg)
			gLUDB.update(ludbId, LUDBEntry::SECINREG);
		else
			gLUDB.update(ludbId, LUDBEntry::SECREQ);
	}

	DBG_LEAVE();
	return true;
}

// *******************************************************************
// message received from vblink
// RCF
// RRJ
// UCF
// URJ
// SPI
// SRS
// URQ
// *******************************************************************
void LUDB_VBLINK_RCFMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_RCFMsgProc", LUDB_LAYER);
	DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received RCF on entry that can't be found by mobile_id\n");
		DBG_LEAVE();
		return;
	}

	DBG_TRACE("ludb %d registration confirmed\n", ludb_id);
	ludbStopVBReqTimer(ludb_id);

	if(ludbRunRegTest){
		printf("[LUDB] Registration Confirm Received.\n");
		gLUDB.update(ludb_id, LUDBEntry::ACTIVE);
		return;
	}	

/*  shmin 08/20 changed mcpla: this funtion to Profile function

	short mmId = ludbGetMMId(ludb_id);	
	if(mmId == -1){
		DBG_ERROR("Received RCF on entry that has no mm_id set\n");
		DBG_LEAVE();
		return;
	}	

	LUDBEntry::LUDBState state = gLUDB.getState(ludb_id);
	bool status;
	if(state == LUDBEntry::REGREQ){
		status = sendMobileRegCnfToMM(mmId, ludb_id, 0);
		if(!status) {
			DBG_ERROR("Registration Confirm send to MM failed\n");
		}
	}
	else if(state == LUDBEntry::REGTHENSEC){
		if(ludb_auth_testmode){
			DBG_TRACE("auth_testmode encountered. bypass sec req to VB.\n");
			status = sendMobileRegCnfToMM(mmId, ludb_id, 0);
			DBG_LEAVE();
			return;
		}

		short ret = ludbCheckAuthCount(ludb_id);
		if(ret == 0){ 			// okay to use the cureent security key 
			DBG_TRACE("Reuse Security Key for ludb %d\n", ludb_id);
			status = sendMobileRegCnfToMM(mmId, ludb_id, 0);
			if(!status) {
				DBG_ERROR("Registration Confirm send to MM failed\n");
			}
		}		
		else if(ret == 1){		// get a new set 
			DBG_TRACE("Get a new Security Key for ludb %d\n", ludb_id);
			status = sendMobileSecInfoReq(inMsg->mobile_id, mmId, ludb_id, 1);
			if(!status) {
				DBG_ERROR("SecReq after RegCnf to VB failed\n");
			}		
		}
		else if(ret == -1){
			DBG_ERROR("possible error: ludb_id out of bound\n");
		}
	}
	else{
		DBG_ERROR("unknown state in RegCnf: %d\n", state);
	}
*/
	DBG_LEAVE();	
}

// *******************************************************************
void LUDB_VBLINK_RRJMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_RRJMsgProc", LUDB_LAYER);
	DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received RRJ on entry that can't be found by mobile_id\n");
		DBG_LEAVE();
		return;
	}

	DBG_TRACE("ludb %d registration rejected by VBLINK\n", ludb_id);
	ludbStopVBReqTimer(ludb_id);

	if(ludbRunRegTest){
		printf("[LUDB] Registration Reject Received. ludb_id %d\n", ludb_id);
		ludbEntryInit(ludb_id);
		return;
	}	

	short mmId = ludbGetMMId(ludb_id);	
	if(mmId == -1){
		DBG_ERROR("Received RRJ on entry that has no mm_id set\n");
		DBG_LEAVE();
		return;
	}
	bool status = sendMobileRegRejToMM(mmId, CNI_RIL3_REJECT_NATIONAL_ROAMING_NOT_ALLOWED);
	if(!status) {
		DBG_ERROR("Registration Reject send to MM failed\n");
	}

	ludbEntryInit(ludb_id);
	DBG_LEAVE();	
}

// *******************************************************************
void LUDB_VBLINK_UCFMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_UCFMsgProc", LUDB_LAYER);
    DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received UCF on entry that can't be found by mobile_id\n");
		DBG_LEAVE();
		return;
	}

	DBG_TRACE("ludb %d unregistration confirmed\n", ludb_id);
	ludbStopVBReqTimer(ludb_id);

//	VOIP_MOBILE_HANDLE mobile_handle = VOIP_NULL_MOBILE_HANDLE;
//	gLUDB.update(ludb_id, mobile_handle);
//	ludbStartPurgeTimer(ludb_id);
//	gLUDB.update(ludb_id, LUDBEntry::INACTIVE);

//	short mmId = ludbGetMMId(ludb_id);	
//	bool status = sendMobileUnRegCnfToMM(mmId);
//	if(!status) {
//		DBG_ERROR("UnRegistration Confirm send to MM failed\n");
//	}

	ludbEntryInit(ludb_id);	
	DBG_LEAVE();	
}

// *******************************************************************
void LUDB_VBLINK_URJMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_URJMsgProc", LUDB_LAYER);
    DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received URJ on entry that can't be found by mobile_id\n");
		DBG_LEAVE();
		return;
	}

	DBG_TRACE("ludb %d unregistration rejected by VBLINK\n", ludb_id);
	ludbStopVBReqTimer(ludb_id);

	short mmId = ludbGetMMId(ludb_id);	

	bool status = sendMobileUnRegRejToMM(mmId);
	if(!status) {
		DBG_ERROR("UnRegistration Reject send to MM failed\n");
	}

	ludbEntryInit(ludb_id);
	DBG_LEAVE();	
}

// *******************************************************************
void LUDB_VBLINK_SPIMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_PRSMsgProc", LUDB_LAYER);
    DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received SPI on entry that can't be found by mobile_id\n");
		ludb_id = gLUDB.registerUser(inMsg->mobile_id);
		if(ludb_id < 0){
			DBG_LEAVE();
			return;
		}
		ludbStopVBReqTimer(ludb_id);
		gLUDB.update(ludb_id, LUDBEntry::ACTIVE);
	}

	DBG_TRACE("ludb %d user profile received\n", ludb_id);

	short size = (short)inMsg->vb_msg_size;
	char vb_msg[400];
	char *msg;

	if(size > VB_MAX_MSG_LENGTH){
		DBG_ERROR("Profile from ViperBase is too large\n");
		DBG_LEAVE();
		return;
	}
	else if(size == 0) {
		msg = vb_msg;
		ludbMakeDefaultSubscMsg(msg, &size);
	} 
	else{
		msg = inMsg->vb_msg;
	}
	
	short ret = ludbSetDecodeSubscInfo(ludb_id, (unsigned char *)msg, size);
	if(ret){
		DBG_ERROR("ludbSetDecodeSubscInfo returned Error\n");
		printSubscMsg((unsigned char *)msg, size);
	}
	else{
		ludbSetSubscInfoExists(ludb_id, true);
	}
	short mmId = ludbGetMMId(ludb_id);	
	if(mmId == -1){
		DBG_ERROR("Received RCF on entry that has no mm_id set\n");
		DBG_LEAVE();
		return;
	}	

// shmin 08/20 mcpla function change the RegCnfToMM message to here

	LUDBEntry::LUDBState state = gLUDB.getState(ludb_id);
	bool status;
	if(state == LUDBEntry::REGREQ){
		status = sendMobileRegCnfToMM(mmId, ludb_id, 0);
		if(!status) {
			DBG_ERROR("Registration Confirm send to MM failed\n");
		}
	}
	else if(state == LUDBEntry::REGTHENSEC){
		if(ludb_auth_testmode){
			DBG_TRACE("auth_testmode encountered. bypass sec req to VB.\n");
			status = sendMobileRegCnfToMM(mmId, ludb_id, 0);
			DBG_LEAVE();
			return;
		}

		short ret = ludbCheckAuthCount(ludb_id);
		if(ret == 0){ 			// okay to use the cureent security key 
			DBG_TRACE("Reuse Security Key for ludb %d\n", ludb_id);
			status = sendMobileRegCnfToMM(mmId, ludb_id, 0);
			if(!status) {
				DBG_ERROR("Registration Confirm send to MM failed\n");
			}
		}		
		else if(ret == 1){		// get a new set 
			DBG_TRACE("Get a new Security Key for ludb %d\n", ludb_id);
			status = sendMobileSecInfoReq(inMsg->mobile_id, mmId, ludb_id, 1);
			if(!status) {
				DBG_ERROR("SecReq after RegCnf to VB failed\n");
			}		
		}
		else if(ret == -1){
			DBG_ERROR("possible error: ludb_id out of bound\n");
		}
	}
	else{
		DBG_ERROR("unknown state in RegCnf: %d\n", state);
	}

	DBG_LEAVE();	
}

// *******************************************************************
void LUDB_VBLINK_SRSMsgProc(LudbMsgStruct *inMsg)
{
    short AuC_failure = 0;

	DBG_FUNC("LUDB_VBLINK_SRSMsgProc", LUDB_LAYER);
	DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received SRS on entry that can't be found by mobile_id\n");
		DBG_LEAVE();
		return;
	}
	
	DBG_TRACE("ludb %d security info received\n", ludb_id);
	ludbStopVBReqTimer(ludb_id);

	if(inMsg->vb_msg_size == 1){
		T_AUTH_TRIPLET triplet;
		memcpy((char *)&triplet, inMsg->vb_msg, sizeof(T_AUTH_TRIPLET));
		gLUDB.update(ludb_id, triplet);
	}
	else{
        AuC_failure = 1;
		DBG_TRACE("!!!!zero SRS!!!!\n");
    }

	short mmId = ludbGetMMId(ludb_id);	
	if(mmId == -1){
		DBG_ERROR("No mmId in ludb entry for SRS\n");
		DBG_LEAVE();
		return;
	}	

	LUDBEntry::LUDBState state = gLUDB.getState(ludb_id);
	bool status;
	if(state == LUDBEntry::SECINREG){
        if(AuC_failure){
		    status = sendMobileRegRejToMM(mmId, CNI_RIL3_REJECT_NETWORK_FAILURE);
		    if(!status) {
			    DBG_ERROR("Registration Reject send to MM failed\n");
		    }
			status = sendMobileUnRegReq(inMsg->mobile_id, mmId, ludb_id);
			if(!status){
				DBG_ERROR("UnRegistration Request send to VB failed\n");
			}
        }
        else{
		    status = sendMobileRegCnfToMM(mmId, ludb_id, 1);
		    if(!status) {
			    DBG_ERROR("Registration Confirm send to MM failed\n");
		    }
        }
	}
	else if(state == LUDBEntry::SECREQ){
		status = sendMobileSecRspToMM(mmId, ludb_id);
		if(!status) {
			DBG_ERROR("Security Info Response send to MM failed\n");
		}
	}
	else{
		DBG_ERROR("unknown state in SRS\n");
	}	
	DBG_LEAVE();	
}

// *******************************************************************
void LUDB_VBLINK_URQMsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_URQMsgProc", LUDB_LAYER);
    DBG_ENTER();

	short ludb_id = ludbGetEntryByMobileId(inMsg->mobile_id);
	if(ludb_id == -1){
		DBG_ERROR("Received URQ on entry that can't be found by mobile_id\n");
		DBG_LEAVE();
		return;
	}

    // <Igal: 04-10-01>
    // Notify affected CC Sessions
    CcNotifyPurge (ludb_id);
	DBG_TRACE("ludb %d unregistered by VBLINK request\n", ludb_id);
	ludbEntryInit(ludb_id);
	DBG_LEAVE();	
}

// ****************************************************************************
// ****************************************************************************
int ludbAutoLogStart()
{
	if(ludbAutoLog){
		printf("[LUDB] AutoLog already started.\n");
		return -1;
	}
	else{
		printf("[LUDB] AutoLog is now started.\n");
		ludbAutoLog = true;
		return 0;
	}
}

// ****************************************************************************
int ludbAutoLogStop()
{
	if(!ludbAutoLog){
		printf("[LUDB] AutoLog already stopped.\n");
		return -1;
	}
	else{
		printf("[LUDB] AutoLog is now stopped.\n");
		ludbAutoLog = false;
		return 0;
	}
}

// *******************************************************************
void LUDB_VBLINK_MsgProc(LudbMsgStruct *inMsg)
{
	DBG_FUNC("LUDB_VBLINK_MsgProc", LUDB_LAYER);
	DBG_ENTER();
	// force mobile_id type fields
	inMsg->mobile_id.ie_present = 1;  
	inMsg->mobile_id.mobileIdType = CNI_RIL3_IMSI;  
	switch(inMsg->msg_type){
		case VOIP_LUDB_REG_CNF:
			LUDB_VBLINK_RCFMsgProc(inMsg);
			break;
		case VOIP_LUDB_REG_REJ:
			LUDB_VBLINK_RRJMsgProc(inMsg);
			break;
		case VOIP_LUDB_UNREG_CNF:
			LUDB_VBLINK_UCFMsgProc(inMsg);
			break;
		case VOIP_LUDB_UNREG_REJ:
			LUDB_VBLINK_URJMsgProc(inMsg);
			break;
		case VOIP_LUDB_SPI_IND:
			LUDB_VBLINK_SPIMsgProc(inMsg);
			break;
		case VOIP_LUDB_SEC_RSP:
			LUDB_VBLINK_SRSMsgProc(inMsg);
			break;
		case VOIP_LUDB_UNREG_REQ:
			LUDB_VBLINK_URQMsgProc(inMsg);
			break;
		default:
			DBG_ERROR("Received unknown message type from VOIP: %d\n",
				inMsg->msg_type);
	}
	if((ludbRunRegTest)||(ludbAutoLog)) ludbDataPrint();
	DBG_LEAVE();
}

