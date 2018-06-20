// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.1
// Status      : Under development
// File        : MMVBLinkMsgProc.cpp
// Author(s)   : Shawn Min 
// Create Date : 08-02-2001
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

#include "JCC/JCCL3Msg.h"
#include "JCC/JCCUtil.h"
#include "JCC/ludbApi.h"

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

// Global Variable for current MM Connections

extern MMEntry_t 		mmEntries[];
extern Boolean			mmOpStatus;

// Global variable for Message Queue
extern MSG_Q_ID 		mmMsgQId;


void MM_VBLINK_PageRequestProc(VblinkMsg_t *vbInMsg)
{
	DBG_FUNC("MM_VBLINK_PageReqProc", MM_LAYER);
	DBG_ENTER();
	
	short	mmId;
	
	if(!mmOpStatus){
		DBG_WARNING("[MM] page request Id(%x) ignored due to disabled btsOpState\n", vbInMsg ->message_data.pageReq.req_id);
		DBG_LEAVE();
		return;
	}
		
	if ((mmId = getEntry(vbInMsg->message_data.pageReq.imsi)) != JCC_ERROR)
	{
		if (mmEntries[mmId].entryId != (T_CNI_IRT_ID) JCC_UNDEFINED)
		{

			if(mmEntries[mmId].vblinkReqId != vbInMsg ->message_data.pageReq.req_id)
			{
				DBG_TRACE(" Received another Page Request Id (%x) (%x)\n", 
					mmEntries[mmId].vblinkReqId, vbInMsg ->message_data.pageReq.req_id);
				return;
			}

			if(mmEntries[mmId].procData.mmState == MM_CONN_EST)
			{
				DBG_TRACE(" Received Page Request State MM_CONN_EST\n");
				return;
			}
			
		    	mmEntries[mmId].pageTimer->cancelTimer();
			MMProceedRRPageRspToVBLINK(mmId);
		
			DBG_TRACE("Already Save Page Response entry_id: %d\n", mmEntries[mmId].entryId);
			
		}else
		{
		
			mmEntries[mmId].pageReq.imsi 	= vbInMsg ->message_data.pageReq.imsi;
			mmEntries[mmId].vblinkReqId 	= vbInMsg ->message_data.pageReq.req_id;
	//		mmEntries[mmId].ccQId 		= vbInMsg->message_data.pageReq.channel_needed;
			mmEntries[mmId].pageCCTimer->cancelTimer();
    			mmEntries[mmId].pageCCTimer->setTimer((sysClkRateGet() * (int)vbInMsg->message_data.pageReq.paging_timeout)/1000);

			DBG_TRACE("PageRequest from VBLINK reqId(%x) Timer(%d, %d)\n", 
			vbInMsg ->message_data.pageReq.req_id,vbInMsg->message_data.pageReq.paging_timeout,
			(sysClkRateGet() * (int)vbInMsg->message_data.pageReq.paging_timeout)/1000);
	
			sendPageReqToRRM(mmId);
		}
		
		DBG_LEAVE();
		return;

	}


	if ( (mmId = entryAlloc()) == JCC_ERROR )
	{
		
		DBG_ERROR("entryAlloc Failed req_id (%x)\n ",vbInMsg ->message_data.pageReq.req_id );
		// Internal no resources action - send est rej.
		// No entry Id in the IRT table assigned yet. Echo back the Page Req.
		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].pageReq.imsi 	= vbInMsg ->message_data.pageReq.imsi;
	mmEntries[mmId].vblinkReqId 	= vbInMsg ->message_data.pageReq.req_id;
//	mmEntries[mmId].ccQId 		= vbInMsg->message_data.pageReq.channel_needed;
	mmEntries[mmId].pageCCTimer->setTimer((sysClkRateGet() * (int)vbInMsg->message_data.pageReq.paging_timeout)/1000);
    	
	DBG_TRACE("PageRequest from VBLINK reqId(%x) Timer(%d,%d)\n", 
			vbInMsg ->message_data.pageReq.req_id,vbInMsg->message_data.pageReq.paging_timeout,
			(sysClkRateGet() * (int)vbInMsg->message_data.pageReq.paging_timeout)/1000);
	
	sendPageReqToRRM(mmId);

	DBG_LEAVE();
	
}

void MM_VBLINK_MsgProc(VblinkMsg_t *vbInMsg)
{
	DBG_FUNC("MM_VBLINK_MsgProc", MM_LAYER);
	DBG_ENTER();

	switch (vbInMsg->message_type) 
	{
		case  VBLINK_RR_PAGE_REQ:
			
			MM_VBLINK_PageRequestProc(vbInMsg);
			break;

		default:
			DBG_ERROR("Unexpected Msg type from VBLINK: %d\n",  
				vbInMsg->message_type);
	}
	DBG_LEAVE();
}

