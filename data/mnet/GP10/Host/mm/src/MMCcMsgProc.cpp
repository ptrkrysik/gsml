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
// File        : MMCcMsgProc.cpp
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

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/LUDBApi.h"

// MM Message Constants
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMSendMsg.h"
#include "MMCcMsgProc.h"
#include "MMPmProc.h"
#include "MMLudbMsgProc.h"
#include "MMTimerProc.h"
#include "MMRmMsgProc.h"

// OAM Config Data
extern MobilityConfigData				mmCfg;
extern Boolean							mmOpStatus;

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];

extern IntraL3Msg_t              mmToRROutMsg; // msg buffer - internal modules 


// ****************************************************************************
void sendPageReqToRRM(short mmId)
{
	DBG_FUNC("sendPageReqToRRM", MM_LAYER);
	DBG_ENTER();

	DBG_TRACE("sending PageReq for mmId: %d(new)\n", mmId);
	mmEntries[mmId].procData.procType = MM_PROC_MOB_TERM;
	mmEntries[mmId].procData.mmState = MM_PAGE_REQ;

    // Setup page timer 
//    	mmEntries[mmId].pageTimer->setTimer(mmCfg.mm_paging_t3113);
//	mmEntries[mmId].pagingTick = tickGet();

	// No entry Id in the IRT table assigned yet.
	mmToRROutMsg.l3_data.pageReq = mmEntries[mmId].pageReq;
	sendIntRRMsg(CNI_IRT_NULL_ID, INTRA_L3_RR_EST_REQ, INTRA_L3_RR_PAGE_REQ);

	mmIncAttTrnPaging();			// PM report
	DBG_LEAVE();
}

// ****************************************************************************
// important! mmId should be valid at this point
void MMPageReqCleanUp(short mmId)
{
	DBG_FUNC("MMPageReqCleanUp", MM_LAYER);
	DBG_ENTER();
	bounceIntCCMsg(mmEntries[mmId].ccQId, 
					INTRA_L3_MM_EST_REJ, INTRA_L3_RR_PAGE_REQ);
	entryDealloc(mmId);		
	DBG_LEAVE();
}

// ****************************************************************************
// Note: For sendLocUpInPageReq, mmId will be referenced for PageReq

short sendLocUpInPageReq(T_CNI_RIL3_IE_MOBILE_ID mobileId, short mmId)	
{
	DBG_FUNC("sendLocUpInPageReq", MM_LAYER);
	DBG_ENTER();
	mmEntries[mmId].mmId2 = -1; 
	mmEntries[mmId].procData.procType = MM_PROC_MOB_TERM;
	mmEntries[mmId].procData.mmState = MM_WAIT4REG;
	mmEntries[mmId].mobileId = mobileId;
	mmEntries[mmId].lup_cksn = 0x07;
	if(sendMobileRegReqToLudb(mobileId, mmId)){
		DBG_TRACE("LocUpInPageReq sent to Ludb successful for MM Id %d\n", mmId); 
		DBG_LEAVE();
		return 0;
	}
	else{
		DBG_ERROR("LocUpInPageReq send failed for MM Id %d\n", mmId); 
		MMPageReqCleanUp(mmId);
		DBG_LEAVE();
		return -1;
	}
}

// ****************************************************************************
// Terminating Call
// Create a new MM entry.
// Request to RR to Page the terminating mobile.
// ****************************************************************************
/*
void MM_CC_PageReqProc(IntraL3Msg_t *ccInMsg)
{
	DBG_FUNC("MM_CC_PageReqProc", MM_LAYER);
	DBG_ENTER();
	// The connection entry for MM
	short	mmId;
	short	existInLudb = 0;

	// check if mobile is registered with LUDB. 
	short 	ludbId = ludbGetEntryByMobileId(ccInMsg->l3_data.pageReq.imsi);
	if( ludbId != JCC_ERROR){ // found in LUDB
		existInLudb = 1;
	}

	if(!mmOpStatus){
		DBG_WARNING("[MM] terminating call ignored due to BTS operational status\n");
		bounceIntCCMsg(ccInMsg->l3_data.pageReq.qid, 
						INTRA_L3_MM_EST_REJ, INTRA_L3_RR_PAGE_REQ);
		DBG_LEAVE();
		return;
	}

	if ( (mmId = entryAlloc()) == JCC_ERROR )
	{
		DBG_ERROR("entryAlloc Failed\n ");
		// Internal no resources action - send est rej.
		// No entry Id in the IRT table assigned yet. Echo back the Page Req.
		bounceIntCCMsg(ccInMsg->l3_data.pageReq.qid, 
						INTRA_L3_MM_EST_REJ, INTRA_L3_RR_PAGE_REQ);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].pageReq = ccInMsg ->l3_data.pageReq;
	mmEntries[mmId].ccQId = ccInMsg->l3_data.pageReq.qid;

	if(!existInLudb){
		sendLocUpInPageReq(ccInMsg->l3_data.pageReq.imsi, mmId);
		DBG_LEAVE();
		return;
	}
	else
		mmEntries[mmId].ludbId = ludbId;

	sendPageReqToRRM(mmId);

	DBG_LEAVE();
}
*/
void MM_CC_PageReqProc(IntraL3Msg_t *ccInMsg)
{
	DBG_FUNC("MM_CC_PageReqProc", MM_LAYER);
	DBG_ENTER();

	short	mmId;

	if ((mmId = getEntry(ccInMsg->l3_data.pageReq.imsi)) == JCC_ERROR)
	{
		// Cleanup action  - May need manual
		// send the clear request to the RRM - 
		// PROBLEM
		//sendIntRRMsg (rrInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
		//

		bounceIntCCMsg(ccInMsg->l3_data.pageReq.qid, 
						INTRA_L3_MM_EST_REJ, INTRA_L3_RR_PAGE_REQ);

		DBG_WARNING("Unexpected/Late CC Page Req!!!n")
		DBG_HEXDUMP(ccInMsg->l3_data.pageReq.imsi.digits, 8);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].pageTimer->cancelTimer();

	mmEntries[mmId].pageReq = ccInMsg->l3_data.pageReq;
	mmEntries[mmId].ccQId 	= ccInMsg->l3_data.pageReq.qid;

	MMProceedRRPageRspToCC(mmId);

	DBG_LEAVE();
}

// ****************************************************************************
// Handover Target Call
// Create a new MM entry.
// Request to RR for Handover request.
// ****************************************************************************
void MM_CC_HOReqProc(IntraL3Msg_t *ccInMsg)
{
	DBG_FUNC("MM_CC_HOReqProc", MM_LAYER);
	DBG_ENTER();
	// The connection entry for MM
	short	mmId;

	if(!mmOpStatus){
		DBG_WARNING("[MM] handover call ignored due to BTS operational status\n");
		bounceIntCCMsg(ccInMsg->l3_data.handReq.qid, 
						INTRA_L3_MM_EST_REJ, INTRA_L3_RR_HANDOVER_REQ);
		DBG_LEAVE();
		return;
	}

	if ( (mmId = entryAlloc()) == JCC_ERROR )
	{
		DBG_ERROR("entryAlloc Failed\n ");
		// Internal no resources action - send est rej.
		// No entry Id in the IRT table assigned yet. Echo back the Page Req.
		bounceIntCCMsg(ccInMsg->l3_data.handReq.qid, 
						INTRA_L3_MM_EST_REJ, INTRA_L3_RR_HANDOVER_REQ);
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].ccQId = ccInMsg ->l3_data.handReq.qid;
	mmEntries[mmId].procData.procType = MM_PROC_MOB_TERM;
	mmEntries[mmId].procData.mmState = MM_HAND_REQ;

	mmToRROutMsg.l3_data = ccInMsg->l3_data;
	// replace union qid with mmId which we need when response comes back.
	mmToRROutMsg.l3_data.handReq.mmId = mmId;

#ifdef DEBUG_MM_HANDOVER_EFR 
	printf("[MM] Recv Handover Req from CC - speech version: %d\n",
			mmToRROutMsg.l3_data.handReq.channelType.speechChann.version);
#endif

	// No entry Id in the IRT table assigned yet.
	sendIntRRMsg(CNI_IRT_NULL_ID, INTRA_L3_RR_EST_REQ, INTRA_L3_RR_HANDOVER_REQ);
	DBG_LEAVE();
}

// ****************************************************************************
// For mobile termination call, or handover target
// ****************************************************************************
void MM_CC_MMEstabReqProc(IntraL3Msg_t *ccInMsg)
{
	DBG_FUNC("MM_CC_MMEstabReqProc", MM_LAYER);
	DBG_ENTER();
	switch(ccInMsg->message_type){
		case INTRA_L3_MM_LOCATE_MS:
			DBG_TRACE("Page Request Received.\n");
			MM_CC_PageReqProc(ccInMsg);
			break;
		case INTRA_L3_RR_HANDOVER_REQ:
			DBG_TRACE("HandOver Request Received.\n");
			MM_CC_HOReqProc(ccInMsg);
			break;
		default:
			DBG_ERROR("Unexpected Msg. from CC, Msg Type: %d\n", 
				ccInMsg->message_type);
	}
	DBG_LEAVE();
}


// ****************************************************************************
void MM_CC_MMRelReqProc(IntraL3Msg_t *ccInMsg)
{
	DBG_FUNC("MM_CC_MMRelReqProc", MM_LAYER);
	DBG_ENTER();
	T_CNI_L3_ID irtMMId;
	// The connection entry for MM
	short	mmId;

	if ((irtMMId = CNI_RIL3_IRT_Get_MM_Id(ccInMsg->entry_id)) == CNI_NULL_L3_ID ) {
		DBG_ERROR("IRT Table Problem, entry_id: %d\n", ccInMsg->entry_id);
		DBG_WARNING("Release Request from CC aborted\n");
		DBG_LEAVE();
		return;
	}
	else 
    {
		mmId = irtMMId.sub_id;

		// Check validity of the MM Id.
		if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections)) 
        {
			DBG_WARNING("Unexpected/Late CCRelReq, entry_id: %d\n", ccInMsg->entry_id);
			DBG_LEAVE();
			return;
		}

		// Now send the clear request to the RRM - 
		sendIntRRMsg (ccInMsg->entry_id, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
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
void MM_CC_MsgProc(IntraL3Msg_t *ccInMsg)
{
	DBG_FUNC("MM_CC_MsgProc", MM_LAYER);
	DBG_ENTER();

	switch (ccInMsg->primitive_type) 
	{
		case INTRA_L3_MM_EST_REQ:
			DBG_TRACE("Received CC->MM - Estab Request\n");
			MM_CC_MMEstabReqProc(ccInMsg);
			break;

		case INTRA_L3_MM_REL_REQ:
			DBG_TRACE("Received CC->MM - Release Request, entry_id: %d\n",
						ccInMsg->entry_id);
			MM_CC_MMRelReqProc(ccInMsg);
			break;

		default:
			DBG_ERROR("Unexpected Msg Primitive from CC: %d\n",  
				ccInMsg->primitive_type);
	}
	DBG_LEAVE();
}
