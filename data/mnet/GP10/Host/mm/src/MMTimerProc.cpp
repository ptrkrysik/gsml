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
// File        : MMTimerProc.cpp
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
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/JCCTimer.h"

// MM Message Constants
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMSendMsg.h"
#include "MMTimerProc.h"
#include "MMPmProc.h"
#include "MMCcMsgProc.h"

#include "RIL3/ie_rrm_common.h"
extern int rm_IsChanAvailable(T_CNI_RIL3_CHANNEL_TYPE channel_type);
extern MMEntry_t mmEntries[];
extern IntraL3Msg_t              mmToCCOutMsg; // msg buffer - internal modules 

// ****************************************************************************
void MM_TIMER_MsgProc(IntraL3Msg_t *mmInMsg)
{
	DBG_FUNC("MM_TIMER_MsgProc", MM_LAYER);
	DBG_ENTER();
	
	// Only timeout message expected
	if ( mmInMsg->message_type != INTRA_L3_TIMEOUT_MSG)
	{
		DBG_TRACE("Unknown message received from Timer, Msg Type: %d\n", 
			mmInMsg->message_type);
		DBG_LEAVE();
		return;
	};

	short mmId = mmInMsg->entry_id; // this entry id contains mm id

	DBG_TRACE("Timeout received for mmId: %d\n", mmId); 

	switch(((char *)&mmInMsg->l3_data)[0])
    	{
		case mmPAGE_REQUEST: 
/* shmin  08/02/01 No paging retries instead by GMC retry

            		mmEntries[mmId].pageRetries++;
            		DBG_TRACE("\nPage Re-Try #%d\n", mmEntries[mmId].pageRetries);
            		if ( mmEntries[mmId].pageRetries < mmMAX_PG_REQUEST )
            		{
                		mmEntries[mmId].pageTimer->setTimer(mmCfg.mm_paging_t3113);
                		sendPageReqToRRM(mmId);
            		}
            		else   // # pages have been sent.....terminate the call attempt
            		{

			    	if(rm_IsChanAvailable(CNI_RIL3_TCH_ACCH_FULL))
				    	mmEntries[mmId].cause = CNI_RIL3_CAUSE_NO_USER_RESPONDING; 
			    	else
				    	mmEntries[mmId].cause = CNI_RIL3_CAUSE_NO_CIRCUIT_OR_CHANNEL_AVAILABLE;
*/
//                		mmEntries[mmId].pageTimer->cancelTimer();
                		mmEntries[mmId].pageRetries = 0;
//                		mmEntries[mmId].pageCCTimer->cancelTimer();

//                		sendIntCCMsg(mmId, INTRA_L3_MM_EST_REJ, INTRA_L3_RR_PAGE_REQ);
//			    		mmIncUnsuccTrnPaging();

				sendIntRRMsg (mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);

                		entryDealloc(mmId);
  //          		}

			break;

		case mmIDENT_REQUEST:
		case mmAUTH_REQUEST:

	        	if (mmEntries[mmId].entryId != (T_CNI_IRT_ID) JCC_UNDEFINED)
				sendIntRRMsg (mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
            		entryDealloc(mmId);

			break;
		
		case mmPAGE_CC_REQUEST: // shmin 08/02/01 

//             		mmEntries[mmId].pageCCTimer->cancelTimer();
               					
//			sendIntRRMsg (mmEntries[mmId].entryId, INTRA_L3_RR_REL_REQ, INTRA_L3_NULL_MSG);
               		
               		entryDealloc(mmId);
		
			break;
    }

	DBG_LEAVE();
}

