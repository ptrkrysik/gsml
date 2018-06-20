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
// File        : MMTask.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
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
#include "Os/JCTask.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"
#include "JCC/JCCTimer.h"
#include "JCC/LudbMMMsgProc.h"

#include "ril3/ril3_common.h"

// MM Message Constants
#include "MM/MMConfig.h"
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMCcMsgProc.h"
#include "MMMdMsgProc.h"
#include "MMOamMsgProc.h"
#include "MMRmMsgProc.h"
#include "MMSysCmdProc.h"
#include "MMTimerProc.h"
#include "MMSendMsg.h"
#include "MMLudbMsgProc.h"
#include "MMVblinkMsgProc.h"
#include "MMBssgpMsgProc.h"

// OAM Config Data
extern MobilityConfigData	mmCfg;

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];

// Global variable for Message Queue
extern MSG_Q_ID mmMsgQId;

// Used by RRM to create IRT entries.
extern T_CNI_L3_ID mm_id;

JCTask mmTaskObj("MMTask");

// ****************************************************************************
int MMPageTimeout(int timerData)
{   
	DBG_FUNC("MMPageTimeout", MM_LAYER);
	DBG_ENTER();
	//
	// The timerData parameter is of 4 bytes long 
	//	
	// When timer is fired and this function is called by the System Clock ISR, 
	// the timerData is passed back.

	// format a timer expiration message and send to the CC/MM task that
	// is using the timer.

	IntraL3Msg_t	timeoutMsg;

	timeoutMsg.module_id = MODULE_MM;
	timeoutMsg.entry_id =  timerData;

	timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;
	((char *)&timeoutMsg.l3_data)[0] = 1;	// 1 for page 

	// send the message. Since this is invoked by the System Clock ISR,
	// we do NOT want to wait for the message delivery. 
	// Return it right away if cannot send on the message queue.
	if (ERROR == msgQSend(	mmMsgQId, 
							(char *) &timeoutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,	  // this is called by ISR, don't wait
							MSG_PRI_NORMAL
							))
	{
		DBG_ERROR("Page Timer Expiry msgQSend (QID=%d) error\n", (int)mmMsgQId);
	}

	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
// shmin
int MMPageCCTimeout(int timerData)
{   
	DBG_FUNC("MMPageCCTimeout", MM_LAYER);
	DBG_ENTER();

	IntraL3Msg_t		timeoutMsg;

	timeoutMsg.module_id 	= MODULE_MM;
	timeoutMsg.entry_id 	= timerData;
	timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;
	
	((char *)&timeoutMsg.l3_data)[0] = 4;	

	if (ERROR == msgQSend(	mmMsgQId, 
				(char *) &timeoutMsg, 
				sizeof(struct  IntraL3Msg_t), 
				NO_WAIT,	  // this is called by ISR, don't wait
				MSG_PRI_NORMAL
				))
	{
		DBG_ERROR("Page CC Timer Expiry msgQSend (QID=%d) error\n", (int)mmMsgQId);
	}

	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
int MMAuthTimeout(int timerData)
{   
	DBG_FUNC("MMAuthTimeout", MM_LAYER);
	DBG_ENTER();

	IntraL3Msg_t	timeoutMsg;

	timeoutMsg.module_id = MODULE_MM;
	timeoutMsg.entry_id =  timerData;

	timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;
	((char *)&timeoutMsg.l3_data)[0] = 2;	// 2 for auth 

	if (ERROR == msgQSend(	mmMsgQId, 
							(char *) &timeoutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,	  // this is called by ISR, don't wait
							MSG_PRI_NORMAL
							))
	{
		DBG_ERROR("Auth Timer Expiry msgQSend (QID=%d) error\n", (int)mmMsgQId);
	}

	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
int MMIdentTimeout(int timerData)
{   
	DBG_FUNC("MMIdentTimeout", MM_LAYER);
	DBG_ENTER();

	IntraL3Msg_t	timeoutMsg;

	timeoutMsg.module_id = MODULE_MM;
	timeoutMsg.entry_id =  timerData;

	timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;
	((char *)&timeoutMsg.l3_data)[0] = 3;	// 3 for identity request 

	if (ERROR == msgQSend(	mmMsgQId, 
							(char *) &timeoutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,	  // this is called by ISR, don't wait
							MSG_PRI_NORMAL
							))
	{
		DBG_ERROR("Ident Timer Expiry msgQSend (QID=%d) error\n", (int)mmMsgQId);
	}

	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
int MMMultiPgTimeout(int timerData)
{   
	DBG_FUNC("MMAuthTimeout", MM_LAYER);
	DBG_ENTER();

	IntraL3Msg_t	timeoutMsg;

	timeoutMsg.module_id = MODULE_MM;
	timeoutMsg.entry_id =  timerData;

	timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;
	((char *)&timeoutMsg.l3_data)[0] = 3;	// 3 for multiple page 

	if (ERROR == msgQSend(	mmMsgQId, 
							(char *) &timeoutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,	  // this is called by ISR, don't wait
							MSG_PRI_NORMAL
							))
	{
		DBG_ERROR("Auth Timer Expiry msgQSend (QID=%d) error\n", (int)mmMsgQId);
	}

	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
// Data Structure Initialization
void mmInitialize()
{
	short i;

	mmCfgRead();

	for(i=0; i< mmCfg.mm_max_mm_connections;i++)
	{
		mmEntries[i].pageTimer     = new JCCTimer( (FUNCPTR)MMPageTimeout, i );
		mmEntries[i].authTimer     = new JCCTimer( (FUNCPTR)MMAuthTimeout, i );
        	mmEntries[i].pageRetries   = 0;
		mmEntries[i].identTimer    = new JCCTimer( (FUNCPTR)MMIdentTimeout, i);
		mmEntries[i].pageCCTimer   = new JCCTimer( (FUNCPTR)MMPageCCTimeout, i); //shmin
		
		entryInit(mmEntries[i]);
	}

	//Initialize for RRM to enter into IRT
	mm_id.msgq_id = mmMsgQId;
    mm_id.sub_id  = (unsigned short)mmCfg.mm_max_mm_connections;
}


// ****************************************************************************
// entry function for the MM Module (VxWorks Task)
int mmMain()
{

	DBG_FUNC("mmMain", MM_LAYER);
	DBG_ENTER();

	// local message queue buffer
	char	MsgBuf[MM_MAX_MSG_LENGTH];

	// enter infinite loop to wait for message and process message from MS 
	// or other layer-3 modules

	mmTaskObj.JCTaskEnterLoop();

	while(true)
	{
		// 
		if(msgQReceive(mmMsgQId, MsgBuf, MM_MAX_MSG_LENGTH, WAIT_FOREVER)==ERROR)
		{
			// failed to receive message. VxWorks error. Abort the task.
			DBG_ERROR("msgQReceive error\n");
			DBG_LEAVE();
			return ERROR;
		}

		// action based on the sending module
		// 
		switch (MsgBuf[0]) { 
			case T_CNI_MD_ID:
				MM_MD_MsgProc((T_CNI_RIL3MD_CCMM_MSG *)MsgBuf);
				break;	

			case MODULE_RM:
				MM_RM_MsgProc((IntraL3Msg_t *)MsgBuf);
				break;

			case MODULE_CC:
				MM_CC_MsgProc((IntraL3Msg_t *)MsgBuf);
				break;

			case MODULE_MM:
				MM_TIMER_MsgProc((IntraL3Msg_t *)MsgBuf);
				break;

			case MODULE_LUDB:	
				MM_Ludb_MsgProc((LudbMsgStruct *)MsgBuf);
				break;
 
			case MODULE_OAM:
				MM_OAM_MsgProc((TrapMsg *)MsgBuf);
				break;

			case MODULE_VBLINK: // shmin
				MM_VBLINK_MsgProc((VblinkMsg_t *)MsgBuf);
				break;

	            	case MODULE_BSSGP:
	            		MM_Bssgp_MsgProc((BSSGP_API_MSG *)MsgBuf);
                		break;

			default:
				DBG_ERROR("MM Error: Message from unexpected Module: %d\n",  MsgBuf[0]);
				DBG_LEAVE();
				return ERROR;
		}
	}
    mmTaskObj.JCTaskNormExit();
	DBG_LEAVE();
	return 0;
}



