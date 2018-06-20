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
// File        : MMRmMsgProc.cpp
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
#include "tickLib.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"

#include "JCC/JCCUtil.h"
#include "JCC/ludbApi.h"
#include "ril3/ril3_common.h"

// MM Message Constants
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMRmMsgProc.h"
#include "MMSendMsg.h"
#include "MMMdMsgProc.h"
#include "MMLudbMsgProc.h"
#include "MMPmProc.h"
#include "MMCcMsgProc.h"

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];

// OAM Config Data
extern MobilityConfigData	mmCfg;
extern short				ludb_auth_required;
extern int					mmCfgCipherMT;

extern T_CNI_RIL3MM_MSG	      mmToMSOutMsg; // MM output message buffer
extern IntraL3Msg_t              mmToCCOutMsg; // msg buffer - internal modules 


// ****************************************************************************
// important! mmId should be valid at this point
void MMRRPageRspCleanUp(short mmId)
{
	DBG_FUNC("MMRRPageRspCleanUp", MM_LAYER);
	DBG_ENTER();
	sendIntCCMsg(mmId, INTRA_L3_MM_EST_REJ, INTRA_L3_RR_PAGE_REQ);
	entryDealloc(mmId);
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedRRPageRspAuth(short mmId)
{
	DBG_FUNC("MMProceedRRPageRspAuth", MM_LAYER);
	DBG_ENTER();

	if(ludb_auth_required){
		// authentication
		DBG_TRACE("Authentication mmEntry[%d]\n", mmId);
		mmEntries[mmId].procData.mmState = MM_AUTH_TERM;
		short ludb_id = mmEntries[mmId].ludbId;
		short ret = ludbCheckAuthCount(ludb_id);
		if(ret == 0){ 			// okay to use the cureent security key 
			DBG_TRACE("Reuse Security Key for ludb %d\n", ludb_id);

			if(mmEntries[mmId].pageRsp.cipherKey.keySeq == ludbCKSN(ludb_id)){
				DBG_TRACE("Skip Auth for mmId: %d\n", mmId);
				MMProceedRRPageRspCipher(mmId);
			}
			else{
				DBG_TRACE("mobile returned cksn %d, differ from ludbcksn %d\n",
							mmEntries[mmId].pageRsp.cipherKey.keySeq,
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

	MMProceedRRPageRspCipher(mmId);

	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedRRPageRspCipher(short mmId)
{
	DBG_FUNC("MMProceedRRPageRspCipher", MM_LAYER);
	DBG_ENTER();

	sendCipherCmdRecordToLudb(mmId, 0);		// reset CipherCmdRecord in LUDB 

	if(mmCfg.mm_cipheringRequired){
		// ciphering
		DBG_TRACE("Ciphering mmEntry[%d]\n", mmId);
		if(!sendRRCipherModeCmd(mmId, MM_PROC_MOB_TERM)){

			// error, clear the entry here.

		}	
		DBG_LEAVE();
		return;
	}

//	MMProceedRRPageRspToCC(mmId);  
	MMProceedRRPageRspToVBLINK(mmId);
	
	DBG_LEAVE();
}

// ****************************************************************************

void MMProceedRRPageRspToVBLINK(short mmId)
{
	DBG_FUNC("MMProceedRRPageRspToVBLINK", MM_LAYER);
	DBG_ENTER();

	short	existInLudb = 0;
	NonStdRasMessagePageResponse_t 	mmToVBLINKOutMsg;

	if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections)){
		DBG_ERROR("Invalid mmId %d, sendIntCCMsg aborted\n", mmId);
		DBG_LEAVE();
		return;
	}

	mmToVBLINKOutMsg.req_id 	= mmEntries[mmId].vblinkReqId;
	mmToVBLINKOutMsg.need_profile 	= 0;
	
    	mmEntries[mmId].pageTimer->setTimer(mmCfg.mm_paging_t3113);
	mmEntries[mmId].pagingTick = tickGet();
	
	DBG_TRACE("PageResponse to VBLINK reqId(%x) need_p(%x)\n",
		 mmToVBLINKOutMsg.req_id,mmToVBLINKOutMsg.need_profile);
	
	VBLinkPageResponse(&mmToVBLINKOutMsg);

//	mmEntries[mmId].procData.mmState = MM_CONN_EST;
	DBG_LEAVE();
}

// ****************************************************************************
void MMProceedRRPageRspToCC(short mmId)
{
	DBG_FUNC("MMProceedRRPageRspToCC", MM_LAYER);
	DBG_ENTER();
	DBG_TRACE("PageRspToCC mm_id = %d, ludb_id = %d",mmId, mmEntries[mmId].ludbId);
	mmToCCOutMsg.l3_data.pageRsp = mmEntries[mmId].pageRsp; 
	sendIntCCMsg(mmId, INTRA_L3_MM_EST_CNF, INTRA_L3_RR_PAGE_RSP);
	mmEntries[mmId].procData.mmState = MM_CONN_EST;
	DBG_LEAVE();
}


// ****************************************************************************
// For mobile termination call,
// Find MM entry that requested RR connection.
// Pass RR establish confirm to CC for the terminating mobile.
// ****************************************************************************
void MM_RM_RRPageRspProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_RRPageRspProc", MM_LAYER);
	DBG_ENTER();

	// The connection entry for MM
	short	mmId;
	short	existInLudb = 0;
	
	short 	ludbId = ludbGetEntryByMobileId(rrInMsg->l3_data.pageRsp.mobileId);

	if( ludbId != JCC_ERROR){ // found in LUDB
		existInLudb = 1;
	}

	if ((mmId = getEntry(rrInMsg->l3_data.pageRsp.mobileId)) == JCC_ERROR)
	{
		// Cleanup action  - May need manual
		// Problem
		// send the clear request to the RRM - 
		sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

		DBG_WARNING("Unexpected/Late RREstabCnf, entry_id: %d\n", rrInMsg->entry_id);
		DBG_LEAVE();
		return;
	}

	DBG_TRACE("PageRsp for mmId: %d\n", mmId);

	mmEntries[mmId].pageCCTimer->cancelTimer();
	mmEntries[mmId].entryId = rrInMsg->entry_id;
    	mmEntries[mmId].pageRetries = 0;

	unsigned long tick = tickGet();
//	printf("Paging start tick %d\n", mmEntries[mmId].pagingTick);
//	printf("Paging end tick %d\n", tick);
	tick = tick - mmEntries[mmId].pagingTick;
	mmUpdMeanSuccPaging(tick);
 	
	T_CNI_RIL3_IRT_ENTRY	irtEntry;
	irtEntry.entry_id = rrInMsg->entry_id;

	if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry)) == false )
	{
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", rrInMsg->entry_id);

		// Cleanup action  - May need manual
		// Problem
		// attempt to send the clear request to the RRM - 
		sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

		// send est rej.
		getMobileId(mmId, mmToCCOutMsg.l3_data.pageReq.imsi);

		MMRRPageRspCleanUp(mmId);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].oid = irtEntry.lapdm_oid;
	mmEntries[mmId].sapi = SAPI_MAIN;

	T_CNI_L3_ID newId;
	newId.msgq_id =  mmMsgQId;
	newId.sub_id = (unsigned short) mmId;
	if  (!(CNI_RIL3_IRT_Set_MM_Id(rrInMsg->entry_id, newId)))
	{
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", rrInMsg->entry_id);

		// Problem
		// attempt to send the clear request to the RRM - 
		sendIntRRMsg (rrInMsg->entry_id, 
					INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		// send est rej.
		getMobileId(mmId, mmToCCOutMsg.l3_data.pageReq.imsi);

		MMRRPageRspCleanUp(mmId);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].pageRsp = rrInMsg->l3_data.pageRsp;

	if(!existInLudb){
		sendLocUpInPageReq(rrInMsg->l3_data.pageRsp.mobileId, mmId);
		DBG_ERROR("ludb not found on Page request\n ");
		DBG_LEAVE();
		return;
	}
	else
		mmEntries[mmId].ludbId = ludbId;

	MMProceedRRPageRspAuth(mmId);

	DBG_LEAVE();
}

// ****************************************************************************
// For handover target
// Find MM entry that requested RR connection.
// Pass RR handover ack to CC for the target mobile.
// ****************************************************************************
void MM_RM_RRHOAckProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_RRHOAckProc", MM_LAYER);
	DBG_ENTER();

	// The connection entry for MM
	short	mmId = rrInMsg->l3_data.handReqAck.mmId;

	if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections))
	{
		// send the clear request to the RRM - 
		sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

		DBG_ERROR("RR->MM Error : Bad mmId in HOAck %d\n", mmId);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].entryId = rrInMsg->entry_id;

	T_CNI_RIL3_IRT_ENTRY	irtEntry;
	irtEntry.entry_id = rrInMsg->entry_id;

	if((CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry)) == false)
	{
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", rrInMsg->entry_id);

		// attempt to send the clear request to the RRM - 
		sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

		// send handover rej.
		sendIntCCMsg(mmId, INTRA_L3_MM_EST_REJ, INTRA_L3_RR_HANDOVER_REQ);
		entryDealloc(mmId);

		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].oid = irtEntry.lapdm_oid;
	mmEntries[mmId].sapi = SAPI_MAIN;

	T_CNI_L3_ID newId;
	newId.msgq_id =  mmMsgQId;
	newId.sub_id = (unsigned short) mmId;
	if(!(CNI_RIL3_IRT_Set_MM_Id(rrInMsg->entry_id, newId)))
	{
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", rrInMsg->entry_id);

		// attempt to send the clear request to the RRM - 
		sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

		// send handover rej.
		sendIntCCMsg(mmId, INTRA_L3_MM_EST_REJ, INTRA_L3_RR_HANDOVER_REQ);
		entryDealloc(mmId);

		DBG_LEAVE();
		return;
	}

	mmToCCOutMsg.l3_data = rrInMsg->l3_data;
#ifdef DEBUG_MM_HANDOVER 
	printf("Recv Handover Command(in handReqAck) from RM: size %d\n",
			  rrInMsg->l3_data.handReqAck.handCmd.msgLength);
	for(int i=0; i<rrInMsg->l3_data.handReqAck.handCmd.msgLength; i++){
		printf(" %x", rrInMsg->l3_data.handReqAck.handCmd.buffer[i]);
	}
	printf("\n");
	printf("Pass Handover Command(in handReqAck) to CC: size %d\n",
			mmToCCOutMsg.l3_data.handReqAck.handCmd.msgLength);
	for(int j=0; j<mmToCCOutMsg.l3_data.handReqAck.handCmd.msgLength; j++){
		printf(" %x", mmToCCOutMsg.l3_data.handReqAck.handCmd.buffer[j]);
	}
	printf("\n");
#endif

	sendIntCCMsg(mmId, INTRA_L3_MM_EST_CNF, INTRA_L3_RR_HANDOVER_REQ_ACK);

	mmEntries[mmId].procData.mmState = MM_CONN_EST;

	//SMS <xxu:06-01-00> Begin
	mmEntries[mmId].hoConn = true;
	//SMS <xxu:06-01-00> End

	DBG_LEAVE();
}

// ****************************************************************************
// For mobile termination call, or handover target,
// ****************************************************************************
void MM_RM_RREstabCnfProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_RREstabCnfProc", MM_LAYER);
	DBG_ENTER();
	switch(rrInMsg->message_type){
		case INTRA_L3_RR_PAGE_RSP:
			DBG_TRACE("Page Response Received.\n");
			MM_RM_RRPageRspProc(rrInMsg);
			break;
		case INTRA_L3_RR_HANDOVER_REQ_ACK:
			DBG_TRACE("HandOver Ack Received.\n");
			MM_RM_RRHOAckProc(rrInMsg);
			break;
		default:
			// Cleanup action  - May need manual
			// Problem
			// send the clear request to the RRM - 
			sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
			DBG_ERROR("Unexpected Msg. from RR, Msg Type: %d\n", 
				rrInMsg->message_type);
	}
	DBG_LEAVE();
}

// ****************************************************************************
// For handover target
// Find MM entry that requested RR connection.
// Pass RR handover nack to CC for the target mobile.
// ****************************************************************************
void MM_RM_RRHONackProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_RRHONackProc", MM_LAYER);
	DBG_ENTER();

	short	mmId = rrInMsg->l3_data.handReqNack.mmId;

	if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections))
	{
		DBG_ERROR("RR->MM Error : Bad mmId in HONack %d\n", mmId);
		DBG_LEAVE();
		return;
	}

	mmToCCOutMsg.l3_data = rrInMsg->l3_data;

	sendIntCCMsg(mmId, INTRA_L3_MM_EST_REJ, INTRA_L3_RR_HANDOVER_REQ_NACK);
	entryDealloc(mmId);

	DBG_LEAVE();
}

// ****************************************************************************
void MM_RM_RREstabRejProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_RREstabRejProc", MM_LAYER);
	DBG_ENTER();
	switch(rrInMsg->message_type){
		case INTRA_L3_RR_HANDOVER_REQ_NACK:
			DBG_TRACE("HandOver Nack Received.\n");
			MM_RM_RRHONackProc(rrInMsg);
			break;
		default:
			DBG_ERROR("Unexpected Msg. from RR, Msg Type: %d\n", 
				rrInMsg->message_type);
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_RM_RRRelIndProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_RRRelIndProc", MM_LAYER);
	DBG_ENTER();
	T_CNI_L3_ID irtMMId;
	// The connection entry for MM
	short mmId;

	if ( (irtMMId = CNI_RIL3_IRT_Get_MM_Id(rrInMsg->entry_id) )
					== CNI_NULL_L3_ID )
	{
		// During Termination setup, entry id is null
		// What can be done? - major error
		// Only manual intervention
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", rrInMsg->entry_id);
		DBG_LEAVE();
		return;
	}
	else 
	{
		mmId = irtMMId.sub_id;
			
		// Check validity of the MM Id.
		if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections))
		{
			DBG_TRACE("Unexpected/Late RRRelInd, entry_id: %d\n", rrInMsg->entry_id);
			DBG_LEAVE();
			return;
		}
	}

	// Now send the Release Indication to the CC - 
	if(mmEntries[mmId].procData.procType != MM_PROC_LUDB){
		mmEntries[mmId].cause = rrInMsg->cause;
		sendIntCCMsg (mmId, INTRA_L3_MM_REL_IND, INTRA_L3_NULL_MSG);
	}

	if(mmEntries[mmId].procData.procType == MM_PROC_MOB_ORIG){
		if(mmEntries[mmId].emerg_ludb){
			sendRemoveEmergencyToLudb(mmId);		
		}
	}

	// Deallocate the MM entry 
	entryDealloc(mmId);
	DBG_LEAVE();
}

// ****************************************************************************
void MM_RM_CipherModeCmpProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_CipherModeCmpProc", MM_LAYER);
	DBG_ENTER();
	T_CNI_L3_ID irtMMId;
	// The connection entry for MM
	short mmId;

	if ( (irtMMId = CNI_RIL3_IRT_Get_MM_Id(rrInMsg->entry_id) )
					== CNI_NULL_L3_ID )
	{
		// During Termination setup, entry id is null
		// What can be done? - major error
		// Only manual intervention
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", rrInMsg->entry_id);
		DBG_LEAVE();
		return;
	}
	else 
	{
		mmId = irtMMId.sub_id;
			
		// Check validity of the MM Id.
		if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections))
		{
			DBG_WARNING("wrong entry_id for CipherModeCmp: %d\n", rrInMsg->entry_id);
			DBG_LEAVE();
			return;
		}
	}

	// check if expecting this
	if(mmEntries[mmId].procData.mmState == MM_CIPHER_CMD){
		mmEntries[mmId].authTimer->cancelTimer();
		mmIncSuccCipher();	// PM report
		switch(mmEntries[mmId].procData.procType){
			case MM_PROC_MOB_ORIG:
				sendCMServiceReqToCC(mmId);
				break;
			case MM_PROC_MOB_TERM:
				MMProceedRRPageRspToVBLINK(mmId);  
				// MMProceedRRPageRspToCC(mmId);
				break;
			default:
				DBG_ERROR("unknown mm proc state in CipherModeComplete %d\n",
							mmEntries[mmId].procData.procType);
		}
	}
	else{
		DBG_ERROR("MM entry %d not expecting CipherModeComplete message\n", mmId);
	}
	
	DBG_LEAVE();
}

// ****************************************************************************
void MM_RM_L3DataProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_L3DataProc", MM_LAYER);
	DBG_ENTER();
	switch(rrInMsg->message_type){
		case INTRA_L3_RR_CIPHER_COMPLETE:
			DBG_TRACE("Received RR->MM - CIPHER_COMPLETE, entry_id: %d\n",
						rrInMsg->entry_id);
			MM_RM_CipherModeCmpProc(rrInMsg);
			break;
		default:
			DBG_ERROR("Unexpected Msg Type from RR: %d\n",  
				rrInMsg->message_type);
			
	}
	DBG_LEAVE();
}

// ****************************************************************************
void MM_RM_MsgProc(IntraL3Msg_t *rrInMsg)
{
	DBG_FUNC("MM_RM_MsgProc", MM_LAYER);
	DBG_ENTER();

	switch (rrInMsg->primitive_type) 
	{
		case INTRA_L3_RR_EST_CNF:
			DBG_TRACE("Received RR->MM - Estab Confirm, entry_id: %d\n",
						rrInMsg->entry_id);
			MM_RM_RREstabCnfProc(rrInMsg);
			break;

		case INTRA_L3_RR_EST_REJ:
			DBG_TRACE("Received RR->MM - Estab Reject, entry_id: %d\n",
						rrInMsg->entry_id);
			MM_RM_RREstabRejProc(rrInMsg);
			break;

		case INTRA_L3_RR_REL_CNF:
			DBG_TRACE("Received RR->MM - Release Confirm, entry_id: %d\n",
						rrInMsg->entry_id);
			break;

		case INTRA_L3_RR_REL_IND:
			DBG_TRACE("Received RR->MM - Release Indication, entry_id: %d\n",
						rrInMsg->entry_id);
			MM_RM_RRRelIndProc(rrInMsg);
			break;

		case INTRA_L3_DATA:
			MM_RM_L3DataProc(rrInMsg);
			break;

		default:
			DBG_ERROR("Unexpected Msg Primitive from RR: %d\n",  
				rrInMsg->primitive_type);
	}
	DBG_LEAVE();
}
