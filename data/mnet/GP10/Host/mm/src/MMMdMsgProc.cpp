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
// File        : MMMdMsgProc.cpp
// Author(s)   : Kevin Lim 
// Create Date : 07-26-99 
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "taskLib.h"
#include "stdLib.h"
#include "string.h"
#include "tickLib.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"
#include "JCC/JCCUtil.h"
#include "ril3/ril3_common.h"
#include "JCC/LUDBapi.h"
// included MD and IRT headers for messages from MS
#if defined(_NO_ED) || defined(_SOFT_MOB)
	#include "JCC/JCC_ED.h"
#else
	#define JCC_RIL3MM_Decode CNI_RIL3MM_Decode
#endif
#include "grr/grr_intf.h"

// MM Message Constants
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMMdMsgProc.h"
#include "MMSendMsg.h"
#include "MMPmProc.h"
#include "MMLudbMsgProc.h"
#include "MMRmMsgProc.h"

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];

// OAM Config Data
extern MobilityConfigData	mmCfg;
int mmCfgT3260 = 5 * 60;
extern int mmCfgIMSIrequired;

extern T_CNI_RIL3_IE_LOCATION_AREA_ID	mmCfgLocId;
extern Boolean							mmOpStatus;

extern T_CNI_RIL3MM_MSG	      mmToMSOutMsg; // MM output message buffer
extern IntraL3Msg_t           mmToCCOutMsg; // msg buffer - internal modules 

int	ta_HoldSdcch = 0;

int cipher_testmode = 0;
T_CNI_RIL3_IE_CIPHER_MODE_SETTING	cipher_testsetting;
extern T_AUTH_TRIPLET				cipher_testtriplet;
extern bool							ludb_auth_testmode;
extern char							ludb_auth_testcksn;
extern short						ludb_auth_required;

extern short rm_DecideCipherAlgo( 
             T_CNI_RIL3_IE_CIPHER_MODE_SETTING *setting,
             T_CNI_RIL3_IE_MS_CLASSMARK_2 *classmark2);


// ****************************************************************************
int taHoldSdcchOn()
{
	if(ta_HoldSdcch){
		printf("[MM] HoldSdcch Mode already turned on.\n");
		return -1;
	}
	else{
		ta_HoldSdcch = 1;
		printf("[MM] HoldSdcch Mode is now turned on.\n");
		printf("    (Mobile Location Update will not be responded.)\n");
		return 0;
	}
}

// ****************************************************************************
int taHoldSdcchOff()
{
	if(!ta_HoldSdcch){
		printf("[MM] HoldSdcch Mode already turned off.\n");
		return -1;
	}
	else{
		ta_HoldSdcch = 0;
		printf("[MM] HoldSdcch Mode is now turned off.\n");
		printf("    (Mobile Location Update will be responded normally.)\n");
		return 0;
	}
}

// ****************************************************************************
// StartCipherMode sets test mode for DSP testing.  It triggers Ciphering on MO
// ****************************************************************************
int StartCipherMode(int start, int algo, unsigned char lsb)
{
	int i;
	if(cipher_testmode){
		printf("[MM] Test Cipher Mode already turned on.\n");
		return -1;
	}
	else{
		mmCfg.mm_cipheringRequired = (Boolean_t)1;

		cipher_testmode = 1;
		cipher_testsetting.ie_present = 1;
		switch(start){
			case 0:
				cipher_testsetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
				break;
			case 1:
				cipher_testsetting.ciphering = CNI_RIl3_CIPHER_START_CIPHERING;
				break;
			default:
				cipher_testsetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
				printf("Unsupported start indicator: %d\n", start);
		}
		switch(algo){
			case 0: 
				cipher_testsetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A51;
				break;
			case 1:
				cipher_testsetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A52;
				break;
			default:
				cipher_testsetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A51;
				printf("Unsupported algorithm: %d\n", algo);
		}
		for(i=0; i<SEC_KC_LEN; i++) cipher_testtriplet.Kc[i] = 0xFF;
		cipher_testtriplet.Kc[SEC_KC_LEN-1] = lsb;
	
		printf("[MM] Test Cipher Mode is now turned on.\n");
		printf("    (Mobile Origination will initiate ciphering.)\n\n");
		printf("StartCipherMode(start=%d, algo=%d, LSB=0x%x)\n", start, algo, lsb);
		printf("                start - 0:no cipher, 1:cipher\n");
		printf("                algo  - 0:A5/1, 1:A5/2\n");
		printf("                ciphering key(Kc) is set to : 0x");
		for(i=0; i<SEC_KC_LEN; i++) printf("%02x", cipher_testtriplet.Kc[i]);
		printf("\n");
		return 0;
	}
}

// ****************************************************************************
int StopCipherMode()
{
	if(!cipher_testmode){
		printf("[MM] Test Cipher Mode already turned off.\n");
		return -1;
	}
	else{
		mmCfg.mm_cipheringRequired = (Boolean_t)0;

		cipher_testmode = 0;
		printf("[MM] Test Cipher Mode is now turned off.\n");
		printf("    (Mobile Origination will work without ciphering.)\n");
		return 0;
	}
}

// ****************************************************************************
short initNewConn(	T_CNI_IRT_ID               entryId,
					T_CNI_LAPDM_OID            oid,
					T_CNI_LAPDM_SAPI           sapi,
					MMProcedureType_t          procType)
{
	DBG_FUNC("initNewConn", MM_LAYER);
	DBG_ENTER();
	short newEntry;

	// Connection Initialization - 
	// New MS RR Connection that needs an MM/CC level service
	if ( (newEntry = entryAlloc()) == JCC_ERROR )
	{
		// No Connections left. 
		DBG_LEAVE();
		return JCC_ERROR;
	}

	mmEntries[newEntry].entryId = entryId;
	mmEntries[newEntry].oid = oid;
	mmEntries[newEntry].sapi = sapi;

	mmEntries[newEntry].procData.procType = procType;

	T_CNI_L3_ID newId;
	newId.msgq_id =  mmMsgQId;
	newId.sub_id = (unsigned short) newEntry;
	if  (!(CNI_RIL3_IRT_Set_MM_Id(entryId, newId)))
	{
		DBG_ERROR("IRT Table problem \n ");
		// Cleanup action ??  - Release all resources
		// Release MM Entry
		entryDealloc(newEntry);
		DBG_LEAVE();
		return JCC_ERROR;
	}

	DBG_LEAVE();
	return newEntry;
}


// ****************************************************************************
// TO MOBILE
// ****************************************************************************
void sendLocationUpdateAccept(		T_CNI_LAPDM_OID   oid,  
									T_CNI_LAPDM_SAPI sapi, 
									T_CNI_IRT_ID entryId,
							const	T_CNI_RIL3_IE_MOBILE_ID& mobileId)
{
	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_LOCATION_UPDATE_ACCEPT	
	*locUpdAccept = &mmToMSOutMsg.locationUpdateAccept ;   // Loc Upd portion
	locUpdAccept->header.message_type = CNI_RIL3MM_MSGID_LOCATION_UPDATING_ACCEPT;

	// Build IE portion ---------------------------------------------
	// Only Mandatory IE needed is LAI

	locUpdAccept->locationId.ie_present = true;

	locUpdAccept->locationId.mcc[0] = mmCfgLocId.mcc[0];
	locUpdAccept->locationId.mcc[1] = mmCfgLocId.mcc[1];  
	locUpdAccept->locationId.mcc[2] = mmCfgLocId.mcc[2];
	locUpdAccept->locationId.mnc[0] = mmCfgLocId.mnc[0];
	locUpdAccept->locationId.mnc[1] = mmCfgLocId.mnc[1];
	locUpdAccept->locationId.mnc[2] = mmCfgLocId.mnc[2]; //formerly 0xf
	
	//printf("MNC go from 2-3 digits MNC[%x%x%x]\n", 
	//		locUpdAccept->locationId.mnc[0],
	//		locUpdAccept->locationId.mnc[1],
	//		locUpdAccept->locationId.mnc[2]);

	locUpdAccept->locationId.lac = mmCfgLocId.lac;

	// Return IMSI to force the stored TMSI in the SIM is deleted.
	// The MS should then register with its IMSI.
	// Do this only if the MS sent a TMSI. 
	// The following is needed only until we complete real registrations
	// with the Gatekeeper - in a couple of weeks!!
	locUpdAccept->mobileId.ie_present = true;
	locUpdAccept->mobileId = mobileId;

	mmIncTrnSubIdIMSI();		// PM report

	if(ta_HoldSdcch) return;

	// Even if the result is not success,
	// we still send release req. below. no check needed.
	sendMSMsg(oid, sapi);

	// Now send the clear request to the RRM - 
	// How do we make sure that LAPDm sends the Loc Upd Accept first?

	sendIntRRMsg (entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
	

}

// ****************************************************************************
void sendLocationUpdateReject(		T_CNI_LAPDM_OID   oid,  
									T_CNI_LAPDM_SAPI sapi ,
									T_CNI_IRT_ID entryId,
									T_CNI_RIL3_REJECT_CAUSE_VALUE cause)
{
	DBG_FUNC("sendLocationUpdateReject", MM_LAYER);
	DBG_ENTER();

	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REJECT	
	*locationUpdateReject = &mmToMSOutMsg.locationUpdateReject ;   // Loc Upd portion
	locationUpdateReject->header.message_type = CNI_RIL3MM_MSGID_LOCATION_UPDATING_REJECT;
	locationUpdateReject->rejectCause.ie_present = true;
	locationUpdateReject->rejectCause.causeValue = cause;
	sendMSMsg(oid,sapi);
	DBG_TRACE("Reject Cause: %s\n", getMMRejectCauseStr(cause));
	// moved from INTRA_L3_MM_RAS_RRJ for consistency - klim 4/22/99 
    // Stage 2.5 fix - Clear message sent with the wrong entry id.
	sendIntRRMsg (entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

	DBG_LEAVE();
}

// ****************************************************************************
void sendIdentityRequest(short mmId)
{
	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_IDENTITY_REQUEST	
	*identityRequest = &mmToMSOutMsg.identityRequest ; 
	identityRequest->header.message_type = CNI_RIL3MM_MSGID_IDENTITY_REQUEST;
	// Build IE portion
	identityRequest->identityType.ie_present = true;
	identityRequest->identityType.identityType = CNI_RIL3_IDENTITY_TYPE_IMSI;
	mmEntries[mmId].identTimer->setTimer(mmCfgT3260);
	sendMSMsg(mmEntries[mmId].oid, mmEntries[mmId].sapi);
}

// ****************************************************************************
void sendMMStatusToMobile( T_CNI_LAPDM_OID   oid,  
						   T_CNI_LAPDM_SAPI  sapi,
						   T_CNI_RIL3_REJECT_CAUSE_VALUE cause)
{
	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_MM_STATUS *mmStatus = &mmToMSOutMsg.mmStatus; 
	mmStatus->header.message_type = CNI_RIL3MM_MSGID_MM_STATUS;
	// Build IE portion
	mmStatus->rejectCause.ie_present = true;
	mmStatus->rejectCause.causeValue = cause;
	sendMSMsg(oid, sapi);
}


// ****************************************************************************
void sendAuthReqToMobile(short mmId, short ludbId)
{
	DBG_FUNC("sendAuthReqToMobile", MM_LAYER);
	DBG_ENTER();

	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_AUTHENTICATION_REQUEST	
	*authReq = &mmToMSOutMsg.authRequest ; 
	authReq->header.message_type = CNI_RIL3MM_MSGID_AUTHENTICATION_REQUEST;

	T_AUTH_TRIPLET *triplet = ludbCurTriplet(ludbId);
	if(triplet == 0){
		DBG_ERROR("zero triplet returned\n");
		// clean up here.
		DBG_LEAVE();
		return;
	}

	if(ludb_auth_testmode)
		authReq->cipherKey.keySeq = ludb_auth_testcksn;
	else{
		authReq->cipherKey.keySeq = ludbCKSN(ludbId);
	}

	if(ludb_auth_testmode){
		for(int i=0; i<SEC_RAND_LEN; i++)
			authReq->authRand.value[i] = cipher_testtriplet.RAND[i];
	}
	else{
		for(int i=0; i<SEC_RAND_LEN; i++)
			authReq->authRand.value[i] = triplet->RAND[i];
	}

	authReq->cipherKey.ie_present = true;
	authReq->authRand.ie_present = true;

	DBG_TRACE("KeySeq: 0x%02x\n", authReq->cipherKey.keySeq);

	mmEntries[mmId].ludbId = ludbId;	// response reference
	mmEntries[mmId].authTimer->setTimer(mmCfgT3260);

	sendMSMsg(mmEntries[mmId].oid, mmEntries[mmId].sapi);

	DBG_LEAVE();
}

// ****************************************************************************
void sendAuthRejToMobile(short mmId)
{
	DBG_FUNC("sendAuthRejToMobile", MM_LAYER);
	DBG_ENTER();

	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_AUTHENTICATION_REJECT	
	*authReq = &mmToMSOutMsg.authReject ; 
	authReq->header.message_type = CNI_RIL3MM_MSGID_AUTHENTICATION_REJECT;

	sendMSMsg(mmEntries[mmId].oid, mmEntries[mmId].sapi);

	DBG_LEAVE();
}

// ****************************************************************************
void sendCMServRej(		T_CNI_LAPDM_OID   oid,  
						T_CNI_LAPDM_SAPI sapi ,
						T_CNI_IRT_ID entryId,
						T_CNI_RIL3_REJECT_CAUSE_VALUE cause)
{
	DBG_FUNC("sendCMServRej", MM_LAYER);
	DBG_ENTER();

	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_CM_SERVICE_REJECT	
	*cmServiceReject = &mmToMSOutMsg.cmServiceReject;
	cmServiceReject->header.message_type = CNI_RIL3MM_MSGID_CM_SERVICE_REJECT;
	cmServiceReject->rejectCause.ie_present = true;
	cmServiceReject->rejectCause.causeValue = cause;
	sendMSMsg(oid,sapi);
	DBG_TRACE("Reject Cause: %s\n", getMMRejectCauseStr(cause));
	sendIntRRMsg (entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

	DBG_LEAVE();
}
		
// ****************************************************************************
// TO VBLINK
// ****************************************************************************
// Note: For sendMobileRegister, mmId will be referenced for CNF/REJ

short sendMobileRegister(short mmId)
{
	DBG_FUNC("sendMobileRegister", MM_LAYER);
	DBG_ENTER();

	DBG_TRACE("LOCUP keySeq: 0x%X, for mmId %d\n", mmEntries[mmId].lup_cksn, mmId);

	mmEntries[mmId].mmId2 = -1;
	mmEntries[mmId].procData.mmState = MM_WAIT4REG;
	mmEntries[mmId].locupTick = tickGet();
	if(sendMobileRegReqToLudb(mmEntries[mmId].mobileId, mmId)){
		DBG_TRACE("MobileRegReq sent to Ludb successful for MM Id %d\n", mmId); 
		DBG_LEAVE();
		return 0;
	}
	else{
		DBG_ERROR("MobileRegReq send failed for MM Id %d\n", mmId); 
		sendIntRRMsg ( mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		entryDealloc(mmId);		
		DBG_LEAVE();
		return -1;
	}
}

// ****************************************************************************
// Note: For sendLocUpInCMServReq, mmId will be referenced for CMServiceRequestAccept 

short sendLocUpInCMServReq(	T_CNI_RIL3_IE_MOBILE_ID mobileId,
						 	short mmId)	
{
	DBG_FUNC("sendLocUpInCMServReq", MM_LAYER);
	DBG_ENTER();
	mmEntries[mmId].mmId2 = -1; 
	mmEntries[mmId].procData.procType = MM_PROC_MOB_ORIG;
	mmEntries[mmId].procData.mmState = MM_WAIT4REG;
	mmEntries[mmId].mobileId = mobileId;
	mmEntries[mmId].lup_cksn = 0x07;
	if(sendMobileRegReqToLudb(mobileId, mmId)){
		DBG_TRACE("LocUpInCMServReq sent to Ludb successful for MM Id %d\n", mmId); 
		DBG_LEAVE();
		return 0;
	}
	else{
		DBG_ERROR("LocUpInCMServReq send failed for MM Id %d\n", mmId); 
		sendIntRRMsg ( mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		entryDealloc(mmId);		
		DBG_LEAVE();
		return -1;
	}
}
			
// ****************************************************************************
// Note: For sendMobileUnRegister, mmId will also be referenced for UCF/REJ

short sendMobileUnRegister(	T_CNI_RIL3_IE_MOBILE_ID &mobileId,
							const T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("sendMobileUnRegister", MM_LAYER);
	DBG_ENTER();

	short mmId = entryAlloc();
	if(mmId != JCC_ERROR) {
		mmEntries[mmId].mobileId = mobileId;
		mmEntries[mmId].oid = msInMsg->lapdm_oid;
		mmEntries[mmId].sapi =  msInMsg->sapi;
		mmEntries[mmId].mmId2 = checkExistingEntry(msInMsg->entry_id);
		mmEntries[mmId].entryId = msInMsg->entry_id;
		mmEntries[mmId].procData.procType = MM_PROC_LUDB;
		mmEntries[mmId].procData.mmState = MM_WAIT4UNREG;
		if(sendMobileUnRegReqToLudb(mobileId, mmId)) {
			DBG_TRACE("MobileUnRegReq sent to Ludb successful for MM Id %d\n", mmId); 
			DBG_LEAVE();
			return 0;
		}
		else {
			DBG_TRACE("MobileUnRegReq send failed for MM Id %d\n", mmId);
			sendIntRRMsg ( msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
			entryDealloc(mmId);		
			DBG_LEAVE();
			return -1;
		}
	}
	else {
		DBG_ERROR("entryAlloc failed. sendMobileUnRegister aborted.\n");
		sendIntRRMsg ( msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
		return -1;
	}
}

// ****************************************************************************
// Location Update Request from Mobile
//
void MM_MD_LocUpdateReqProc(
			T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REQUEST *locUpdReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_LocUpdateReqProc", MM_LAYER);
	DBG_ENTER();
/*
	printf("[CLASSMARK1] A5/1 Availability (0-avail, 1-not) :%d\n", 
			locUpdReq->classmark1.a51);
*/
	if(!mmOpStatus){
		printf("[MM] LU req. ignored due to BTS operational status\n");
		DBG_WARNING("[MM] LU req. ignored due to BTS operational status\n");
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
		return;
	}	

	short mmId = initNewConn( msInMsg->entry_id,  msInMsg->lapdm_oid,  
							msInMsg->sapi, MM_PROC_LUDB);
	if(mmId != JCC_ERROR){
		mmEntries[mmId].mobileId = locUpdReq->mobileId;
		mmEntries[mmId].lup_cksn = locUpdReq->cipherKey.keySeq;
		DBG_TRACE("Mobile sent keySeq: 0x%X\n", locUpdReq->cipherKey.keySeq);

		switch(locUpdReq->mobileId.mobileIdType){	// check mobile id type	
			case CNI_RIL3_TMSI: // ask for IMSI
				sendIdentityRequest(mmId);
				return;
			case CNI_RIL3_IMSI:
				mmIncTrnSubIdIMSI();		// PM report
				sendMobileRegister(mmId);
				return;
			case CNI_RIL3_IMEI:
			case CNI_RIL3_IMEISV:
	
			default:
				DBG_ERROR("unsupported type of mobile id encountered %d\n",
						locUpdReq->mobileId.mobileIdType);
		}
	}
	else{
		DBG_ERROR("entryAlloc failed. LocUpdateReq aborted.\n");
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
	}
}
	
// ****************************************************************************
void MM_MD_IdentityRespProc(
			T_CNI_RIL3MM_MSG_IDENTITY_RESPONSE *identityResponse,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_IdentityRespProc", MM_LAYER);
	DBG_ENTER();
	short mmId = MM_FindIrtMMId(msInMsg->entry_id);
	if(mmId < 0){
		DBG_ERROR("unexpected identity response from entry %d\n", msInMsg->entry_id);
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
	}	
	else{
		mmEntries[mmId].mobileId = identityResponse->mobileId;
		mmEntries[mmId].identTimer->cancelTimer();
		switch(mmEntries[mmId].procData.procType){
			case MM_PROC_LUDB:
				if(identityResponse->mobileId.mobileIdType == CNI_RIL3_IMSI){
					mmIncTrnSubIdIMSI();		// PM report
					sendMobileRegister(mmId);
				}
				else{
					DBG_ERROR("unsupported type of mobile id encountered %d\n",
						identityResponse->mobileId.mobileIdType);
					sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
					entryDealloc(mmId);		
				}
				break;
			case MM_PROC_MOB_ORIG:
				if(mmEntries[mmId].procData.mmState == MM_EMERGENCY){
					mmEntries[mmId].cmServReq.mobileId = identityResponse->mobileId;
					MM_ProceedEmergencyIMSI(mmId);
				}	
				else{
					DBG_ERROR("IdentityResp when mm state %d\n", 
								mmEntries[mmId].procData.mmState);
					sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
					entryDealloc(mmId);		
				}
				break;
			default:
				DBG_ERROR("unexpected identity response in proc state %d\n", 
							mmEntries[mmId].procData.procType);
		}
	}
	DBG_LEAVE();
}

// ****************************************************************************
// TO RRM
// ****************************************************************************
bool sendRRCipherModeCmd(short mmId, MMProcedureType_t proc)
{
	DBG_FUNC("sendRRCipherModeCmd", MM_LAYER);
	DBG_ENTER();
	IntraL3MsgData_t	l3_data;
	T_CNI_RIL3_IE_MS_CLASSMARK_2 classmark2;
 
	switch(proc){
		case MM_PROC_MOB_ORIG:
			classmark2 = mmEntries[mmId].cmServReq.classmark2;
			break; 
		case MM_PROC_MOB_TERM:
			classmark2 = mmEntries[mmId].pageRsp.classmark2;
			break; 
		default:
			DBG_ERROR("Unknown procType(%d) on sendRRCipherModeCmd\n", proc);
	}
	if(cipher_testmode){
		switch(cipher_testsetting.algorithm){
			case CNI_RIL3_CIPHER_ALGORITHM_A51:
				if(classmark2.a51){
					printf("[MM] ERROR! Setting ciphering mode: Mobile does not support A5/1\n");
				}
				break;
			case CNI_RIL3_CIPHER_ALGORITHM_A52:
				if(!classmark2.a52){
					printf("[MM] ERROR! Setting ciphering mode: Mobile does not support A5/2\n");
				}
				break;
			case CNI_RIL3_CIPHER_ALGORITHM_A53:
				if(!classmark2.a53){
					printf("[MM] ERROR! Setting ciphering mode: Mobile does not support A5/3\n");
				}
				break;
			default:
				printf("[MM] ERROR! Test ciphering algorithm hasn't been set yet!\n");
		}
		printf("[MM] CLASSMARK2 A5/1 Availability (0-avail, 1-not) :%d\n", classmark2.a51);
		printf("[MM] CLASSMARK2 A5/2 Availability (0-not, 1-avail) :%d\n", classmark2.a52);
		printf("[MM] CLASSMARK2 A5/3 Availability (0-not, 1-avail) :%d\n", classmark2.a53);

		l3_data.cipherCmd.cmd.cipherModeSetting = cipher_testsetting;
		for(int i=0; i<SEC_KC_LEN; i++) 
			l3_data.cipherCmd.Kc[i] = cipher_testtriplet.Kc[i];
	}	
	else{
		// set real ciphermode setting
		DBG_TRACE("[MM] CLASSMARK2 (0-avail, 1-not): CallType(%d) A51(%d) A52(%d) A53(%d)\n",
                           proc, classmark2.a51, classmark2.a52, classmark2.a53);

		short algo_choice = 0; 
	//	algo_choice = rm_DecideCipherAlgo(&l3_data.cipherCmd.cmd.cipherModeSetting, &classmark2);
		algo_choice = grr_DecideCipherAlgo(&l3_data.cipherCmd.cmd.cipherModeSetting, &classmark2);
		DBG_TRACE("MM set ciphermode: algo_choice(%d) ciphering(%d) algo(%d)\n",
			       algo_choice,
				   l3_data.cipherCmd.cmd.cipherModeSetting.ciphering,
				   l3_data.cipherCmd.cmd.cipherModeSetting.algorithm
				 );

		//l3_data.cipherCmd.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_START_CIPHERING;
   		//if(!classmark2.a52){
		//	if(classmark2.a51){
		//		DBG_ERROR("A5/2, A5/1 both not available on mobile\n");
		//		l3_data.cipherCmd.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
		//		algo_choice = 0;
		//	}
		//	else{
		//		DBG_TRACE("set ciphermode: A5/1\n");
		//		l3_data.cipherCmd.cmd.cipherModeSetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A51;
		//		algo_choice = 1;
		//	}
		//}
		//else{
		//	DBG_TRACE("set ciphermode: A5/2\n");
		//	l3_data.cipherCmd.cmd.cipherModeSetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A52;
		//	algo_choice = 2;
		//}
		short ludbId = mmEntries[mmId].ludbId; 

		// keep cipher modes in ludb record
		sendCipherCmdRecordToLudb(mmId, algo_choice);

		T_AUTH_TRIPLET *triplet = ludbCurTriplet(ludbId);
		if(triplet == 0){
			DBG_ERROR("triplet returned 0 in CipherModeCmd for ludb %d\n", ludbId);
			// clean up here
			DBG_LEAVE();
			return false;
		}
		else{
			for(int i=0; i<SEC_KC_LEN; i++) 
				l3_data.cipherCmd.Kc[i] = triplet->Kc[i];
		}
	}
	l3_data.cipherCmd.cmd.cipherModeSetting.ie_present = 1;

	l3_data.cipherCmd.cmd.cipherResponse.ie_present = 1;
	l3_data.cipherCmd.cmd.cipherResponse.cipherResponse = CNI_RIL3_IMEISV_NOT_INCLUDED;

	// Change state to  MM_CIPHER_CMD
	mmEntries[mmId].procData.procType = proc;
	mmEntries[mmId].procData.mmState = MM_CIPHER_CMD;
	mmEntries[mmId].authTimer->setTimer(mmCfgT3260);
	sendIntRRDataMsg (mmEntries[mmId].entryId, INTRA_L3_DATA, INTRA_L3_RR_CIPHER_COMMAND, l3_data);
	mmIncAttCipher();	// PM report
	DBG_LEAVE();
	return true;
}

// ****************************************************************************
// TO MOBILE, again
// ****************************************************************************
bool sendCMServiceAccept(short mmId)
{
	DBG_FUNC("sendCMServiceAccept", MM_LAYER);
	DBG_ENTER();

	if(mmId == JCC_ERROR){
		DBG_ERROR("mmId Error in sendCMServiceAccept\n");
		DBG_LEAVE();
		return false;
	}
	T_CNI_LAPDM_OID   	oid		= mmEntries[mmId].oid;  
	T_CNI_LAPDM_SAPI 	sapi	= mmEntries[mmId].sapi; 
	T_CNI_IRT_ID 		entryId	= mmEntries[mmId].entryId;

	// encode RIL3 message to send to the MS
	buildMSMsgCommon ();

	T_CNI_RIL3MM_MSG_CM_SERVICE_ACCEPT	*cmServAccept
						= &mmToMSOutMsg.cmServiceAccept;

	cmServAccept->header.message_type = CNI_RIL3MM_MSGID_CM_SERVICE_ACCEPT;

	// Build IE portion begin 
	// No Mandatory IEs needed
	// Build IE portion done 

	T_CNI_RIL3_RESULT result;
	if ( (result = sendMSMsg(oid, sapi)) != CNI_RIL3_RESULT_SUCCESS )
	{
		// Problem
		// send the clear request to the RRM - 
		DBG_ERROR("Problem sending MS msg for Entry Id %d\n", entryId);
		sendIntRRMsg (entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		entryDealloc(mmId);
		DBG_LEAVE();
		return false;
	}
	DBG_TRACE("CMServiceAccept sent to mobile for MM Id %d\n", mmId); 
	DBG_LEAVE();	
	return true;
}

// ****************************************************************************
// TO CC
// ****************************************************************************
bool sendCMServiceReqToCC(short mmId)
{
	DBG_FUNC("sendCMServiceReqToCC", MM_LAYER);
	DBG_ENTER();

	// Change state from MM_START state.
	mmEntries[mmId].procData.procType = MM_PROC_MOB_ORIG;
	mmEntries[mmId].procData.mmState = MM_CONN_EST;
						
	// Send the whole service request message to CC
	mmToCCOutMsg.l3_data.servReq = mmEntries[mmId].cmServReq;

	// Send the establishment indication to the CC
	sendIntCCMsg(mmId, INTRA_L3_MM_EST_IND, INTRA_L3_NULL_MSG);

	DBG_LEAVE();
	return true;
}

// ****************************************************************************
void MM_MD_CMServiceAbortProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_ABORT *cmServAbort,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_CMServiceAbortProc", MM_LAYER);
	DBG_ENTER();
	short mmId = checkExistingEntry(msInMsg->entry_id);
	if(mmId == JCC_ERROR){
		DBG_WARNING("No existing mmEntry associated with CMServiceAbort entryId %d\n",
					msInMsg->entry_id);
		DBG_LEAVE();
		return;
	}
	DBG_TRACE("CMServiceAbort for mmId %d\n", mmId);
	//entryPrint(mmEntries[mmId]);	

	if(mmEntries[mmId].procData.procType != MM_PROC_MOB_ORIG){
		DBG_WARNING("CM Abort NOT from Originating Mobile!  Msg Ignored.\n");
		DBG_LEAVE();
		return;
	}
	else{ // check for emergency call, in case we need to remove temporary emergency profile from ludb	
		if(mmEntries[mmId].emerg_ludb){
			sendRemoveEmergencyToLudb(mmId);		
		}
	}

	if(mmEntries[mmId].procData.mmState == MM_CONN_EST){
		DBG_TRACE("send abort message to CC\n");
		sendIntCCMsg(mmId, INTRA_L3_MM_REL_IND, INTRA_L3_NULL_MSG);
	}
	DBG_TRACE("send abort message to RR\n");
	sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
	entryDealloc(mmId);		

	DBG_LEAVE();
}

// ****************************************************************************
// For mobile origniating call,
void MM_MD_CMServiceReqProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST *cmServReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_CMServiceReqProc", MM_LAYER);
	DBG_ENTER();
	if(!mmOpStatus){
		printf("[MM] originating call ignored due to BTS operational status\n");
		DBG_WARNING("[MM] originating call ignored due to BTS operational status\n");
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
		return;
	}

	if(cmServReq->cmServiceType.serviceType == CNI_RIL3_CM_SERVICE_TYPE_EMERGENCY_CALL){
		MM_MD_CMServiceReqEmergencyProc(cmServReq, msInMsg);
	}
	else{
		MM_MD_CMServiceReqNormalProc(cmServReq, msInMsg);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_MD_CMServiceReqEmergencyProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST *cmServReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_CMServiceReqEmergencyProc", MM_LAYER);
	DBG_ENTER();
	DBG_TRACE("Emergency Call received\n");

    mmIncAttMOEmergencyCalls();
	short	mmId;
	if((mmId = checkExistingEntry(msInMsg->entry_id)) != JCC_ERROR) {
		DBG_ERROR("EMERGENCY CMServiceRequest came with existing entry_id %d\n", 
				msInMsg->entry_id);
		DBG_LEAVE();
		return;
	}

	if ((mmId = initNewConn( msInMsg->entry_id,  msInMsg->lapdm_oid,  
							msInMsg->sapi, MM_PROC_MOB_ORIG))
							== JCC_ERROR)
	{
		DBG_ERROR("Problem creating NewConn for Entry Id %d\n", msInMsg->entry_id);
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].cmServReq = *cmServReq;	// copy cmServiceRequest
	mmEntries[mmId].procData.mmState = MM_EMERGENCY;
	mmEntries[mmId].mobileId = cmServReq->mobileId;

	switch(cmServReq->mobileId.mobileIdType){	// check mobile id type	
		case CNI_RIL3_TMSI: // ask for IMSI
			sendIdentityRequest(mmId);
			return;
		case CNI_RIL3_IMSI:
			MM_ProceedEmergencyIMSI(mmId);
			return;
		case CNI_RIL3_IMEI:
		case CNI_RIL3_IMEISV:
			if(mmCfgIMSIrequired)
				MMCMServiceCleanUp(mmId);
			else
				sendInsertEmergencyToLudb(mmId);
			return;	
		default:
			DBG_ERROR("unsupported type of mobile id encountered %d\n",
					cmServReq->mobileId.mobileIdType);
	}
}

// ****************************************************************************
void MM_ProceedEmergencyIMSI(short mmId)
{
	DBG_FUNC("MM_ProceedEmergencyIMSI", MM_LAYER);
	DBG_ENTER();

	short existInLudb = 0;

	short ludbId = ludbGetEntryByMobileId(mmEntries[mmId].mobileId);
	// check if mobile is registered with LUDB. 
	if( ludbId == JCC_ERROR){ // not found in LUDB
		sendInsertEmergencyToLudb(mmId);
		DBG_LEAVE();
		return;
	}
	else
		mmEntries[mmId].ludbId = ludbId;

	MMProceedCMServiceReqAccept(mmId);
	
	DBG_LEAVE();
}

// ****************************************************************************
void MM_MD_CMServiceReqNormalProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST *cmServReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_CMServiceReqNormalProc", MM_LAYER);
	DBG_ENTER();

	// The connection entry for MM
	short	mmId;
	short existInLudb = 0;
	short ludbId = ludbGetEntryByMobileId(cmServReq->mobileId);

	if((mmId = checkExistingEntry(msInMsg->entry_id)) != JCC_ERROR)
	{
		//SMS <Igal:06-01-00> Begin
		if (cmServReq->cmServiceType.serviceType == CNI_RIL3_CM_SERVICE_TYPE_SHORT_MESSAGE_SERVICE)
        {
			if(mmEntries[mmId].hoConn == false && ludbIsProvSMS_MO(ludbId) == false)
	    //SMS <Igal:06-01-00> End
            {
				sendCMServRej(msInMsg->lapdm_oid, msInMsg->sapi, msInMsg->entry_id, 
				  				CNI_RIL3_REJECT_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED);
				DBG_LEAVE();
				return;
			}

			mmEntries[mmId].cmServReq = *cmServReq;	// copy cmServiceRequest
			MMProceedCMServiceReqAccept(mmId);
			DBG_LEAVE();
			return;
		}		
		if(msInMsg->primitive_type == L23PT_DL_EST_IND){
			// Drop the new service request message.
			// This is probably a late SABM coming through
			DBG_WARNING("Late SABM on CMSrvReq, entry_id %d\n", msInMsg->entry_id);
			DBG_LEAVE();
			return;
		}
		else if(msInMsg->primitive_type == L23PT_DL_DATA_IND){
			DBG_TRACE("CMServiceReq in DL_DATA_IND, entry_id: %d\n", msInMsg->entry_id);
			sendCMServiceAccept(mmId);
			DBG_LEAVE();
			return;
		}
	}

	if ((mmId = initNewConn( msInMsg->entry_id,  msInMsg->lapdm_oid,  
							msInMsg->sapi, MM_PROC_MOB_ORIG))
							== JCC_ERROR)
	{
		DBG_ERROR("Problem creating NewConn for Entry Id %d\n", msInMsg->entry_id);
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].cmServReq = *cmServReq;	// copy cmServiceRequest

	if(ludbId == JCC_ERROR){ // not found in LUDB
		sendLocUpInCMServReq(cmServReq->mobileId, mmId);
		DBG_LEAVE();
		return;
	}
	else
		mmEntries[mmId].ludbId = ludbId;

	MMProceedCMServiceReqAuth(mmId);

	DBG_LEAVE();
}

// ****************************************************************************
// important! mmId should be valid at this point
void MMCMServiceCleanUp(short mmId)
{
	DBG_FUNC("MMCMServiceCleanUp", MM_LAYER);
	DBG_ENTER();
	sendIntRRMsg(mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
	entryDealloc(mmId);
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedCMServiceReqAuth(short mmId)
{
	DBG_FUNC("MMProceedCMServiceReqAuth", MM_LAYER);
	DBG_ENTER();

	short ludb_id = mmEntries[mmId].ludbId;
	if(mmEntries[mmId].cmServReq.cmServiceType.serviceType 
	   == CNI_RIL3_CM_SERVICE_TYPE_SHORT_MESSAGE_SERVICE){
		// check profile
		if(!ludbIsProvSMS_MO(ludb_id)){
			sendCMServRej(mmEntries[mmId].oid, mmEntries[mmId].sapi, mmEntries[mmId].entryId, 
					  CNI_RIL3_REJECT_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED);
			DBG_LEAVE();
			return;
		}
	}
	if(ludb_auth_required){
		// authentication
		DBG_TRACE("Authentication mmEntry[%d]\n", mmId);
		mmEntries[mmId].procData.mmState = MM_AUTH_ORIG;
		short ret = ludbCheckAuthCount(ludb_id);
		if(ret == 0){ 			// okay to use the cureent security key 
			DBG_TRACE("Reuse Security Key for ludb %d\n", ludb_id);

			if(mmEntries[mmId].cmServReq.cipherKey.keySeq == ludbCKSN(ludb_id)){
				DBG_TRACE("Skip Auth for mmId: %d\n", mmId);
				MMProceedCMServiceReqCipher(mmId);
			}
			else{
				DBG_TRACE("mobile returned cksn %d, differ from ludbcksn %d\n",
							mmEntries[mmId].cmServReq.cipherKey.keySeq,
							ludbCKSN(ludb_id));
				sendAuthReqToMobile(mmId, ludb_id);
			}
		}		
		else if(ret == 1){		// get a new set	
			DBG_TRACE("Get a new Security Key for mmId %d\n", mmId);
			bool status = sendMobileSecInfoReqToLudb(mmId, ludb_id);
			if(!status) {
				DBG_ERROR("SecInfoReq to LUDB failed\n");
			}		
		}
		else if(ret == -1){
			DBG_ERROR("possible error: ludb_id out of bound\n");
		}
		DBG_LEAVE();
		return;
	}

	MMProceedCMServiceReqCipher(mmId);
	
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedCMServiceReqCipher(short mmId)
{
	DBG_FUNC("MMProceedCMServiceReqCipher", MM_LAYER);
	DBG_ENTER();

	sendCipherCmdRecordToLudb(mmId, 0);		// reset CipherCmdRecord in LUDB 

	if(mmCfg.mm_cipheringRequired){
		// ciphering
		DBG_TRACE("Ciphering mmEntry[%d]\n", mmId);
		if(!sendRRCipherModeCmd(mmId, MM_PROC_MOB_ORIG)){

			// error, clear the entry here.
		}	
		DBG_LEAVE();
		return;
	}

	MMProceedCMServiceReqAccept(mmId);
	
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedCMServiceReqAccept(short mmId)
{
	DBG_FUNC("MMProceedCMServiceReqAccept", MM_LAYER);
	DBG_ENTER();

	if(sendCMServiceAccept(mmId)){
		sendCMServiceReqToCC(mmId);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_MD_IMSIDetachIndProc(
			T_CNI_RIL3MM_MSG_IMSI_DETACH_INDICATION *imsiDetachInd,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_IMSIDetachIndProc", MM_LAYER);
	DBG_ENTER();

	if(!mmOpStatus){
		printf("[MM] IMSI Detach ignored due to BTS operational status\n");
		DBG_WARNING("[MM] IMSI Detach ignored due to BTS operational status\n");
		sendIntRRMsg (msInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		DBG_LEAVE();
		return;
	}	

	if (imsiDetachInd->mobileId.ie_present)
	{
		if (imsiDetachInd->mobileId.mobileIdType == CNI_RIL3_IMSI)
		{
			T_CNI_RIL3_IE_MOBILE_ID  mobileId =  imsiDetachInd->mobileId;
			sendMobileUnRegister(mobileId, msInMsg); 
		}
		else
		{
			DBG_ERROR("A non-IMSI Mobile Id. is received for IMSI detach.\n");
		}
	}
	else
	{
		// mobile Id doesn't exist.
		DBG_ERROR("Mobile ID is not present for IMSI detach\n");
	}		
	DBG_LEAVE();
}

// ****************************************************************************
bool MM_IsSRESCorrect(short ludbId, T_CNI_RIL3_IE_AUTHENTICATION_PARAM_SRES *authSres)
{
	DBG_FUNC("MM_IsSRESCorrect", MM_LAYER);
	DBG_ENTER();

	if(ludbId == -1){
		DBG_ERROR("IsSRESCorrect: invalid ludbId: %d\n", ludbId);
		DBG_LEAVE();
		return false;
	}
	T_AUTH_TRIPLET *triplet = ludbCurTriplet(ludbId);
	if(triplet == 0){
		DBG_ERROR("IsSRESCorrect: no triplet returned for ludbId: %d\n", ludbId);
		DBG_LEAVE();
		return false;
	}
	else{
		bool same = true;
		for(int i=0; i<SEC_SRES_LEN; i++){
			if(((char *)&authSres->value)[i] != triplet->SRES[i]){
				DBG_TRACE("IsSRESECorrect: SRES Mismatch!\n");
				DBG_TRACE("LUDB  MS\n");
				for(int j=0; j<SEC_SRES_LEN; j++)
					DBG_TRACE(" %02x   %02x", ((char *)&authSres->value)[j], 
											  triplet->SRES[j]);
				same = false;
				break;
			}
		}
		DBG_LEAVE();
		return same;
	}
}

// ****************************************************************************
void MM_MD_AuthRespProc(
			T_CNI_RIL3MM_MSG_AUTHENTICATION_RESPONSE *authResponse,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_AuthRespProc", MM_LAYER);
	DBG_ENTER();
	short mmId, ludbId;
	T_CNI_L3_ID	irtMMId;

	DBG_TRACE("received response: 0x%x\n", authResponse->authSres.value); 
	DBG_TRACE("\n");

	if((irtMMId = CNI_RIL3_IRT_Get_MM_Id(msInMsg->entry_id)) == CNI_NULL_L3_ID){
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", msInMsg->entry_id);
		DBG_LEAVE();
		return;
	}
	else{
		mmId = irtMMId.sub_id;
		
		// Check validity of the MM Id.
		if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections)){
			DBG_WARNING("invalid mmId in irtTable for entryId: %d\n", msInMsg->entry_id);
			DBG_LEAVE();
			return;
		}
	}
	mmEntries[mmId].authTimer->cancelTimer();
	ludbId = mmEntries[mmId].ludbId;
	// comparison with actual SRES value here.
	bool same = MM_IsSRESCorrect(ludbId, &authResponse->authSres);
	if(!same){
		T_CNI_RIL3_IE_MOBILE_ID mobileId = mmEntries[mmId].mobileId;
        sendAuthRejToMobile(mmId); // send AuthRej 

        if(mmEntries[mmId].procData.mmState == MM_AUTH_LOCUP){ // need to cleanup registration
		    mmEntries[mmId].procData.mmState = MM_WAIT4UNREG;
		    if(sendMobileUnRegReqToLudb(mobileId, mmId)) {
			    DBG_TRACE("MobileUnRegReq sent to Ludb successful for MM Id %d\n", mmId); 
            }
		}

		DBG_LEAVE();
		return;
	}			
	
	switch(mmEntries[mmId].procData.mmState){
		case MM_AUTH_LOCUP:
			MMProceedLocUpdateCipher(mmId, ludbId);
			break;
		case MM_AUTH_ORIG:
			MMProceedCMServiceReqCipher(mmId);
			break;
		case MM_AUTH_TERM:
			MMProceedRRPageRspCipher(mmId);
			break;
		default:
			DBG_ERROR("unknown mm state in AuthRespProc %d\n", 
						mmEntries[mmId].procData.mmState);
	}	

	DBG_LEAVE();
}

// ****************************************************************************
void MM_MD_MMStatusProc( 
			T_CNI_RIL3MM_MSG_MM_STATUS *mmStatus,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_MMStatusProc", MM_LAYER);
	DBG_ENTER();
	short mmId = checkExistingEntry(msInMsg->entry_id);
	if(mmId == JCC_ERROR){
		DBG_WARNING("No existing mmEntry associated with MMStatus entryId %d\n",
					msInMsg->entry_id);
		DBG_LEAVE();
		return;
	}
	DBG_TRACE("MMStatus for mmId %d\n", mmId);
	if(mmStatus->rejectCause.ie_present)
		DBG_TRACE("Received rejectCause 0x%x\n", mmStatus->rejectCause.causeValue);

	DBG_LEAVE();
}

// ****************************************************************************
void MM_MD_MsgProc(T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
	DBG_FUNC("MM_MD_MsgProc", MM_LAYER);
	DBG_ENTER();
	if ((msInMsg->primitive_type != L23PT_DL_EST_IND)
		&&(msInMsg->primitive_type != L23PT_DL_DATA_IND))
	{
		DBG_ERROR("Only EstInd or DataInd expected. Bad Primitive: %d\n",
			msInMsg->primitive_type);
		DBG_LEAVE();
		return;
	}

   PostL3RecvMsLog(msInMsg->lapdm_oid, msInMsg->sapi, &msInMsg->l3_data);
   
	// Decode the MS message.
	T_CNI_RIL3MM_MSG	msDecodedMsg;		// MM message buffer


	// call the Decoder function to decode the message
	T_CNI_RIL3_RESULT result =
				JCC_RIL3MM_Decode
				  (
#ifdef _NO_ED
				   TRUE,
#else
  #ifdef _SOFT_MOB
				   !((msInMsg->lapdm_oid) < softMobLapdmOid[0]),
  #endif
#endif
					&msInMsg->l3_data,  // input encoded layer-3 msg
					&msDecodedMsg	   // output decoded message 
				);

	// check the encoding result before sending the message
	if( result != CNI_RIL3_RESULT_SUCCESS)
	{
		// decoding error. Something wrong with the message. Switch on
		// the return code to handle different types of error
		switch(result) 
		{
			case CNI_RIL3_MANDATORY_IE_MISSING:
				// missing mandatory IE(s). This is a protocol violation from 
				// the peer layer-3 entity 
				sendMMStatusToMobile(msInMsg->lapdm_oid, msInMsg->sapi,
							CNI_RIL3_REJECT_INVALID_MANDATORY_INFORMATION);		
				break;

			case CNI_RIL3_INCORRECT_LENGTH:
				// message length error.
				break;

			case CNI_RIL3_UNKNOWN_MESSAGE_TYPE:
				sendMMStatusToMobile(msInMsg->lapdm_oid, msInMsg->sapi,
							CNI_RIL3_REJECT_MESSAGE_TYPE_NON_EXISTENT);		
				break;

			default:
				break;
		}

		// the received message can not be decoded properly. handle the 
		// situation according to the state of the layer-3 entity.
		DBG_ERROR("Decoding MS Message Problem. \n ");
		// Protocol Error Action??
		DBG_LEAVE();
		return;
	}

	// decoded layer-3 message correctly.
	// switch on the message type

	switch(msDecodedMsg.header.message_type) 
	{
		case CNI_RIL3MM_MSGID_LOCATION_UPDATING_REQUEST:
			DBG_TRACE("Received MD msg - Location Update Request\n");
			mmIncIMSIAttach();			// PM report
			MM_MD_LocUpdateReqProc(&msDecodedMsg.locationUpdateRequest, msInMsg);
 			break;

		case  CNI_RIL3MM_MSGID_CM_SERVICE_REQUEST:
			DBG_TRACE("Received MD msg - CM Service Request\n");
			mmIncAttMOCalls();			// PM report
			mmIncTrnSubIdIMSI();		// PM report
			MM_MD_CMServiceReqProc(&msDecodedMsg.cmServiceRequest, msInMsg);
			break;

		// add IMSI detach. -gz
		case CNI_RIL3MM_MSGID_IMSI_DETACH_INDICATION:
			DBG_TRACE("Received MD msg - IMSI Detach Indication\n");
			mmIncTrnSubIdIMSI();		// PM report
			mmIncIMSIDetach();			// PM report
			MM_MD_IMSIDetachIndProc(&msDecodedMsg.imsiDetachInd, msInMsg);
			break;

		// add indentity response. -gz
		case CNI_RIL3MM_MSGID_IDENTITY_RESPONSE:
			DBG_TRACE("Received MD msg - Identity Response\n");
			MM_MD_IdentityRespProc(&msDecodedMsg.identityResponse, msInMsg);
			break;	

		case CNI_RIL3MM_MSGID_AUTHENTICATION_RESPONSE:
			DBG_TRACE("Received MD msg - Authentication Response\n");
			MM_MD_AuthRespProc(&msDecodedMsg.authResponse, msInMsg);
			break;

		case CNI_RIL3MM_MSGID_CM_SERVICE_ABORT:
			DBG_TRACE("Received MD msg - CM Service Abort\n");
			MM_MD_CMServiceAbortProc(&msDecodedMsg.cmServiceAbort, msInMsg);
			break;

		case CNI_RIL3MM_MSGID_MM_STATUS:
			DBG_TRACE("Received MD msg - MM Status\n");
			MM_MD_MMStatusProc(&msDecodedMsg.mmStatus, msInMsg);
			break;

		default:
			// log error
			DBG_ERROR("Unexpected Message from MD, Msg Type: %d\n",  
				msDecodedMsg.header.message_type);
			// Protocol Error Action ??
	}
	DBG_LEAVE();
}



