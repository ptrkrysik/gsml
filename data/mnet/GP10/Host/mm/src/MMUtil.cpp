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
// File        : MMUtil.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"
#include "JCC/JCCUtil.h"
#include "RIL3/ril3irt_db.h"
#include "JCC/LUDBInstance.h"
#include "JCC/LUDBConfig.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"

// MM Message Constants
#include "MMSendMsg.h"
#include "MMTypes.h"
#include "MM/MMConfig.h"
#include "MMUtil.h"

#include "stdio.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3_common.h"
#include "ril3/ril3md.h"

// included LAPDm header for downlink messages
#include "lapdm/lapdm_l3intf.h"

// OAM Config Data
extern MobilityConfigData	mmCfg;

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];

// Used by RRM to create IRT entries.
extern T_CNI_L3_ID mm_id;

// IRT table database
extern CNI_RIL3IRT_DB			CNI_IRT_database;

// ****************************************************************************
// Stage 2.5 This cleanup functionality actually belongs to an audit process.
void mmConnDataInit(void)
{
	short i;
	for(i=0; i< mmCfg.mm_max_mm_connections;i++) {
		// Stage 2.5 - We should have a way of cleaning signaling channels from RRM!! 
        if (!(mmEntries[i].free)) {
		    sendIntRRMsg (mmEntries[i].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		}

		entryInit(mmEntries[i]);
	}
}

// ****************************************************************************
void irtMMDataInit(void)
{
	short i;
	for(i=0; i< 25 ; i++) {
		CNI_IRT_database.m_entry[i].mm_id = mm_id;
	}
}

// ****************************************************************************
short MM_FindIrtMMId(T_CNI_IRT_ID entry_id)
{
	DBG_FUNC("MM_FindIrtMMId", MM_LAYER);
	DBG_ENTER();
	T_CNI_L3_ID irtMMId;
	short mmId;

	if((irtMMId = CNI_RIL3_IRT_Get_MM_Id(entry_id)) == CNI_NULL_L3_ID){
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", entry_id);
		DBG_LEAVE();
		return -1;
	}
	else{
		mmId = irtMMId.sub_id;
		
		// Check validity of the MM Id.
		if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections)){
			DBG_WARNING("invalid mmId in irtTable for entryId: %d\n", entry_id);
			DBG_LEAVE();
			return -2;
		}
	}
	DBG_LEAVE();
	return mmId;
}

// ****************************************************************************
void mmDataInit(void)
{
	mmConnDataInit();
	irtMMDataInit();
}

// ****************************************************************************
void entryInit(MMEntry_t &mmEntry)
{
	DBG_FUNC("entryInit", MM_LAYER);
	DBG_ENTER();
	mmEntry.oid = CNI_LAPDM_NULL_OID;

	// Always initialize the MM Id back to an unused value
	if (mmEntry.entryId != (T_CNI_IRT_ID) JCC_UNDEFINED)
	{
		if  (!(CNI_RIL3_IRT_Set_MM_Id(mmEntry.entryId, mm_id)))
		{
		//	DBG_ERROR("IRT Table problem\n");
		}
	}

	mmEntry.entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
	mmEntry.procData.procType = MM_PROC_NULL;
	mmEntry.procData.mmState = MM_START;
	mmEntry.free = true;
	mmEntry.pageTimer->cancelTimer();
	mmEntry.authTimer->cancelTimer();
    mmEntry.identTimer->cancelTimer();
    mmEntry.pageRetries = 0;
    mmEntry.ludbId = JCC_ERROR;
	mmEntry.mmId2 = JCC_ERROR;
	mmEntry.pageReq.imsi.mobileIdType = CNI_RIL3_NO_MOBILE_ID;
	mmEntry.lup_cksn = 0x07;
	mmEntry.emerg_ludb = 0;
	mmEntry.pagingTick = 0;
	mmEntry.locupTick = 0;
	mmEntry.cause = CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED; 

	//SMS <xxu:06-01-00> Begin
	mmEntry.hoConn = false;
    //SMS <xxu:06-01-00> End
        mmEntry.gsInitiated = false;

	DBG_LEAVE();
}

// ****************************************************************************
// Stage 2 changes needed for Orig and Term Calls - Begin
short checkExistingEntry(const T_CNI_IRT_ID newEntryId)
{
	short i;
	for(i=0; i< mmCfg.mm_max_mm_connections;i++) {
		if (mmEntries[i].free == false) {
			if ( mmEntries[i].entryId == newEntryId ) {
				return i;
			}
		}
	}
	return JCC_ERROR;
}
// Stage 2 changes needed for Orig and Term Calls - End
// ****************************************************************************

short getEntry(const T_CNI_RIL3_IE_MOBILE_ID &mobId)
{
	for(short i=0; i< mmCfg.mm_max_mm_connections;i++) {
		if (mmEntries[i].free == false &&
	//		mmEntries[i].procData.procType == MM_PROC_MOB_TERM &&
	//		mmEntries[i].procData.mmState == MM_PAGE_REQ &&
			// Stage 2.5 changes needed for Orig and Term Calls - Begin
			mmEntries[i].pageReq.imsi == mobId)
		{
			return i;
		}
		// Stage 2.5 changes needed for Orig and Term Calls - End
	}

	return JCC_ERROR;
}

// ****************************************************************************
// add overload function for getEntry. -gz
short getEntry(const short ludbId)
{
	for(short i=0; i< mmCfg.mm_max_mm_connections;i++) {
		if (mmEntries[i].free == false && mmEntries[i].ludbId == ludbId) {
			return i;
		}
	}

	return JCC_ERROR;
}

// ****************************************************************************
short getMobileId(short mmId, T_CNI_RIL3_IE_MOBILE_ID &mobId)
{

	mobId = mmEntries[mmId].pageReq.imsi;
	return JCC_SUCCESS;
}


// ****************************************************************************
// add overload function for getMobileId. -gz
T_CNI_RIL3_IE_MOBILE_ID getMobileId(short mmId)
{
	if ((mmId >= 0) && (mmId < mmCfg.mm_max_mm_connections)) {
		return mmEntries[mmId].mobileId;
	}
	else {
		T_CNI_RIL3_IE_MOBILE_ID mobileId;
		mobileId.mobileIdType = CNI_RIL3_NO_MOBILE_ID;
		return mobileId;
	}
}


// ****************************************************************************
// get MM Id from LUDB Id. -gz
short getMMId(const short ludbId)
{
    for(int i=0; i<mmCfg.mm_max_mm_connections; i++) {
		if (mmEntries[i].ludbId == ludbId)
			return i;
	}
	
	return JCC_ERROR;
}


// ****************************************************************************
short entryAlloc()
{
	DBG_FUNC("entryAlloc", MM_LAYER);
	DBG_ENTER();
	short i;
	for(i=0; i< mmCfg.mm_max_mm_connections;i++) {
		if (mmEntries[i].free == true) {
			mmEntries[i].free = false;

			//SMS <xxu:06-01-00> Begin
			mmEntries[i].hoConn = false;
            //SMS <xxu:06-01-00> End

			DBG_TRACE("Entry Alloc %d\n", i);
			DBG_LEAVE();
			return i;
		}
	}

	DBG_WARNING("Entry Alloc Failed \n");
	DBG_LEAVE();
	return JCC_ERROR;
}

// ****************************************************************************
void entryDealloc(short i)
{
	DBG_FUNC("entryDealloc", MM_LAYER);
	DBG_ENTER();
	DBG_TRACE("Entry Dealloc %d\n", i);
	entryInit(mmEntries[i]);
	DBG_LEAVE();
}

// Print Data.
// ****************************************************************************
void entryPrint(MMEntry_t &mmEntry)
{
	printf(" %02d   %02d   %s  %s %2d   %02d\n",
	 mmEntry.oid,
	 mmEntry.entryId,
	 getMMProcStateString(mmEntry.procData.procType),
	 getMMStateString(mmEntry.procData.mmState),
	 mmEntry.free,
	 mmEntry.ludbId);
}

// ****************************************************************************

void mmDataPrint(int number)
{
	short i;
	if(number == 0) number = 20;
	printf("*******************************************************************\n");
	printf(" [ID]  oid  eid  ProcType    State      Free LUDB\n"); 
	for(i=0; i< number;i++) {
		printf(" [%02d] ", i); 
		entryPrint(mmEntries[i]);
    }
}

// ****************************************************************************
char *getMMEntryState(short mmId)
{
	if((mmId >= 0)&&(mmId < mmCfg.mm_max_mm_connections)){
		return(getMMStateString(mmEntries[mmId].procData.mmState));
	}
	else{
		return("BAD mmId");
	}
}

// ****************************************************************************
char *getMMProcStateString(MMProcedureType_t proc_type)
{
	switch(proc_type){
		case MM_PROC_NULL:		return("NULL      ");
		case MM_PROC_LUDB:		return("LOC_UPDATE");
		case MM_PROC_MOB_ORIG:	return("MOB_ORIG  ");
		case MM_PROC_MOB_TERM:	return("MOB_TERM  ");
		default:				return("UNKNOWN   ");
	}
}

// ****************************************************************************
char *getMMStateString(MMState_t state)
{
	switch(state){
		case MM_START: 			return("IDLE      ");
		case MM_CLEAR_REQ:		return("CLEAR_REQ ");
		case MM_CONN_EST:		return("CONN_EST  ");
		case MM_EMERGENCY:		return("EMERGENCY ");
		case MM_CIPHER_CMD:		return("CIPHERING ");
		case MM_AUTH_LOCUP:		return("AUTH_LOCUP");
		case MM_AUTH_ORIG:		return("AUTH_ORIG ");
		case MM_AUTH_TERM:		return("AUTH_TERM ");
		case MM_WAIT4REG: 		return("WAIT4REG  ");
		case MM_WAIT4UNREG:		return("WAIT4UNREG");
		case MM_WAIT4SPI:		return("WAIT4SPI  ");
		case MM_PAGE_REQ:		return("PAGE_REQ  ");
		case MM_HAND_REQ:		return("HAND_REQ  ");
		default:				return("UNKNOWN   ");
	}
}

// ****************************************************************************
char *getMMPrimStr(IntraL3PrimitiveType_t prim)
{
	switch(prim){
		case INTRA_L3_DATA:			return("L3Data  ");
		case INTRA_L3_RR_EST_REQ:	return("RREstReq"); 
		case INTRA_L3_RR_REL_REQ:	return("RRRelReq");
		case INTRA_L3_RR_REL_CNF:	return("RRRelCnf");
		case INTRA_L3_RR_EST_CNF:	return("RREstCnf");
		case INTRA_L3_RR_REL_IND:	return("RRRelInd");
		case INTRA_L3_RR_EST_REJ:	return("RREstRej");	
		case INTRA_L3_MM_EST_IND:	return("MMEstInd");
		case INTRA_L3_MM_EST_REQ:	return("MMEstReq");
		case INTRA_L3_MM_EST_CNF:	return("MMEstCnf");
		case INTRA_L3_MM_EST_REJ:	return("MMEstRej");
		case INTRA_L3_MM_REL_REQ:	return("MMRelReq");
		case INTRA_L3_MM_REL_IND:	return("MMRelInd");
		default: return("Unknown");
	}
}

// ****************************************************************************
char *getMMMsgTypeStr(IntraL3MsgType_t msgType)
{
	switch(msgType){
		case INTRA_L3_RR_PAGE_REQ:			return("PageReq");
		case INTRA_L3_RR_PAGE_RSP:			return("PageRsp");
		case INTRA_L3_RR_HANDOVER_REQ:		return("HandReq");
		case INTRA_L3_RR_HANDOVER_REQ_ACK:	return("HandAck");
		case INTRA_L3_RR_HANDOVER_REQ_NACK:	return("HandNak");
		case INTRA_L3_RR_CIPHER_COMMAND:	return("CiphCmd");
		case INTRA_L3_RR_CIPHER_COMPLETE:	return("CiphCmp");
		case INTRA_L3_MM_LOCATE_MS:			return("PageReq");
		case INTRA_L3_NULL_MSG:				return("Null   ");
		default: return("Unknown");
	}
}  

// ****************************************************************************
char *getRIL3MMTypeStr(T_CNI_RIL3_MESSAGE_TYPE msgType)
{
	switch(msgType){
		case CNI_RIL3MM_MSGID_IMSI_DETACH_INDICATION   	: return("IMSI_DETACH");
		case CNI_RIL3MM_MSGID_LOCATION_UPDATING_ACCEPT 	: return("LOC_UP_ACPT");
		case CNI_RIL3MM_MSGID_LOCATION_UPDATING_REJECT 	: return("LOC_UP_RJT ");
		case CNI_RIL3MM_MSGID_LOCATION_UPDATING_REQUEST	: return("LOC_UP_REQ ");

		// RIL3MM Security messages:
		case CNI_RIL3MM_MSGID_AUTHENTICATION_REJECT    	: return("AUTH_REJ   "); 
		case CNI_RIL3MM_MSGID_AUTHENTICATION_REQUEST   	: return("AUTH_REQ   ");
		case CNI_RIL3MM_MSGID_AUTHENTICATION_RESPONSE  	: return("AUTH_RESP  ");
		case CNI_RIL3MM_MSGID_IDENTITY_REQUEST         	: return("IDENT_REQ  ");
 		case CNI_RIL3MM_MSGID_IDENTITY_RESPONSE        	: return("IDENT_RESP ");
 		case CNI_RIL3MM_MSGID_TMSI_REALLOCATION_COMMAND	: return("TMSI_RE_CMD");
 		case CNI_RIL3MM_MSGID_TMSI_REALLOCATION_COMPLETE: return("TMSI_RE_COM");
 
		// R IL3MM Connection management messages:
		case CNI_RIL3MM_MSGID_CM_SERVICE_ACCEPT        	: return("CM_SVC_ACPT");	
		case CNI_RIL3MM_MSGID_CM_SERVICE_REJECT        	: return("CM_SVC_REJ ");
		case CNI_RIL3MM_MSGID_CM_SERVICE_ABORT         	: return("CM_SVC_ABRT");
		case CNI_RIL3MM_MSGID_CM_SERVICE_REQUEST       	: return("CM_SVC_REQ ");
		case CNI_RIL3MM_MSGID_CM_SERVICE_PROMPT        	: return("CM_SVC_PROM");
		case CNI_RIL3MM_MSGID_CM_REESTABLISHMENT_REQUEST: return("CM_RE_ESTAB");
		case CNI_RIL3MM_MSGID_ABORT                     : return("ABORT      ");

		// R IL3MM Miscellaneous messages:
		case CNI_RIL3MM_MSGID_MM_NULL					: return("NULL       ");
		case CNI_RIL3MM_MSGID_MM_STATUS					: return("STATUS     ");
		case CNI_RIL3MM_MSGID_MM_INFORMATION			: return("INFORMATION");
		default: return("Unknown");
	}
}

// ****************************************************************************
char *getMMRejectCauseStr(T_CNI_RIL3_REJECT_CAUSE_VALUE cause)
{
	switch(cause){
		case CNI_RIL3_REJECT_IMSI_UNKNOWN_IN_HLR						:
			return("IMSI_UNKNOWN_IN_HLR");
		case CNI_RIL3_REJECT_ILLEGAL_MS									:
			return("ILLEGAL_MS");
		case CNI_RIL3_REJECT_IMSI_UNKNOWN_IN_VLR						:
			return("IMSI_UNKNOWN_IN_VLR");
		case CNI_RIL3_REJECT_IMEI_NOT_ACCEPTED							:
			return("IMEI_NOT_ACCEPTED");
		case CNI_RIL3_REJECT_ILLEGAL_ME									:
			return("ILLEGAL_ME");
		case CNI_RIL3_REJECT_PLMN_NOT_ALLOWED							:
			return("PLMN_NOT_ALLOWED");
		case CNI_RIL3_REJECT_LOCATION_AREA_NOT_ALLOWED					:
			return("LOCATION_AREA_NOT_ALLOWED");
		case CNI_RIL3_REJECT_NATIONAL_ROAMING_NOT_ALLOWED				:
			return("NATIONAL_ROAMING_NOT_ALLOWED"); 
		case CNI_RIL3_REJECT_NETWORK_FAILURE							:
			return("NETWORK_FAILURE");
		case CNI_RIL3_REJECT_CONGESTION									: 
			return("CONGESTION");
		case CNI_RIL3_REJECT_SERVICE_OPTION_NOT_SUPPORTED				:
			return("SERVICE_OPTION_NOT_SUPPORTED");
		case CNI_RIL3_REJECT_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED	:
			return("REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED"); 
		case CNI_RIL3_REJECT_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER 	:
			return("SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER");
		case CNI_RIL3_REJECT_CALL_CANNOT_BE_IDENTIFIED 					:
			return("CALL_CANNOT_BE_IDENTIFIED");
		case CNI_RIL3_REJECT_SEMANTICALLY_INCORRECT_MESSAGE				:
			return("SEMANTICALLY_INCORRECT_MESSAGE");
		case CNI_RIL3_REJECT_INVALID_MANDATORY_INFORMATION				:
			return("INVALID_MANDATORY_INFORMATION");
		case CNI_RIL3_REJECT_MESSAGE_TYPE_NON_EXISTENT					:
			return("MESSAGE_TYPE_NON_EXISTENT");
		case CNI_RIL3_REJECT_MESSAGE_TYPE_NOT_COMPATIBLE				:
			return("MESSAGE_TYPE_NOT_COMPATIBLE"); 
		case CNI_RIL3_REJECT_INFORMATION_ELEMENT_NON_EXISTENT			:
			return("INFORMATION_ELEMENT_NON_EXISTENT");
		case CNI_RIL3_REJECT_CONDITIONAL_IE_ERROR						:
			return("CONDITIONAL_IE_ERROR");
		case CNI_RIL3_REJECT_MESSAGE_NOT_COMPATIBLE 					:
			return("MESSAGE_NOT_COMPATIBLE");
		case CNI_RIL3_REJECT_PROTOCOL_ERROR_UNSPECIFIED					:
			return("PROTOCOL_ERROR_UNSPECIFIED");
		default: return("Unknown"); 
	} 
}

// ****************************************************************************
// print separator on log
void mmBlank()
{

	DBG_FUNC("mmBlank", MM_LAYER);
	DBG_ENTER();
	DBG_TRACE("--------------------------------------------------------");
	printf("--------------------------------------------------------\n");
	DBG_LEAVE();
}



