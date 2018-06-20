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
// File        : MMSendMsg.cpp
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

//#define DISABLE_MD_DOWN_MSG

#include "taskLib.h"
#include "stdLib.h"
#include "string.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCUtil.h"

#include "ril3/ril3_common.h"

// MM Message Constants
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMCcMsgProc.h"

T_CNI_RIL3MM_MSG	      mmToMSOutMsg; // MM output message buffer
IntraL3Msg_t              mmToCCOutMsg; // msg buffer - internal modules 
IntraL3Msg_t              mmToRROutMsg; // msg buffer - internal modules 

// Need to include RR Message Queue
#ifndef _NO_RRM
extern MSG_Q_ID 		rm_MsgQId;
#endif

// OAM Config Data
extern MobilityConfigData				mmCfg;

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];


// ****************************************************************************
void buildMSMsgCommon()
{
	// But, before we start populating the message content, 
	// zap it clean first.
	CNI_RIL3_ZAPMSG(&mmToMSOutMsg, sizeof(T_CNI_RIL3MM_MSG));

	// header portion --------------------------------------------------
	mmToMSOutMsg.header.protocol_descriminator = CNI_RIL3_PD_MM;
	mmToMSOutMsg.header.si_ti = 0;
}  

// ****************************************************************************
T_CNI_RIL3_RESULT sendMSMsg(T_CNI_LAPDM_OID            oid,
							T_CNI_LAPDM_SAPI           sapi)
{
	DBG_FUNC("sendMSMsg", MM_LAYER);
    DBG_ENTER();
	T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

	// call the Encoder function to encode the message
	T_CNI_RIL3_RESULT
	result = CNI_RIL3MM_Encode(	&mmToMSOutMsg,    // input message definition
								&msEncodedMsg  // encoded layer-3 message
							  );

	// check the encoding result before sending the message
	if (result == CNI_RIL3_RESULT_SUCCESS) {
#ifdef DISABLE_MD_DOWN_MSG
	return result;
#endif

		// encoding successful, send the message to LAPDm for delivery
		#ifdef _SOFT_MOB
			if (oid < softMobLapdmOid[0])
		#endif
            PostL3SendMsLog(oid, sapi, &msEncodedMsg);
				CNI_LAPDM_Dl_Data_Request(	oid, 
											sapi, 
											// CT_FACCH_F, CT_SDCCH, 
											&msEncodedMsg);

		DBG_TRACE("Sent Message to LAPDm, Msg Type: %s\n",  
				getRIL3MMTypeStr(mmToMSOutMsg.header.message_type));
	}
	else {
		DBG_ERROR("Encoding MS Message Problem. %d \n ", result);
	}

	DBG_LEAVE();
	return result;
} 

// ****************************************************************************
void sendIntRRMsg (	T_CNI_IRT_ID               entryId,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType)
{
	DBG_FUNC("sendIntRRMsg", MM_LAYER);
    DBG_ENTER();
	#ifndef _NO_RRM

	mmToRROutMsg.module_id = MODULE_MM;
	mmToRROutMsg.primitive_type = prim;
	mmToRROutMsg.message_type = msgType;

	mmToRROutMsg.entry_id = entryId;

	// send the message. 

	#ifdef _SOFT_MOB
	if ((entryId < softMobEntryId[0]) ||
		(entryId > softMobEntryId[CC_MAX_CC_CALLS - 1]))
	#endif
	if (ERROR == msgQSend(	rm_MsgQId, 
							(char *) &mmToRROutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,
							MSG_PRI_NORMAL
							)) {
		DBG_ERROR("sendIntRRMsg msgQSend (QID=%d) error\n ", (int)rm_MsgQId);
	}
	else {
		DBG_TRACE("Sent msg to RR, Prim: %s, Msg: %s\n", 
					getMMPrimStr(prim), getMMMsgTypeStr(msgType));
	}

	#endif
	DBG_LEAVE();
}  

// ****************************************************************************
void sendIntRRDataMsg (	T_CNI_IRT_ID               entryId,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType,
					IntraL3MsgData_t		   l3_data)
{
	DBG_FUNC("sendIntRRDataMsg", MM_LAYER);
    DBG_ENTER();

	mmToRROutMsg.module_id = MODULE_MM;
	mmToRROutMsg.primitive_type = prim;
	mmToRROutMsg.message_type = msgType;
	mmToRROutMsg.l3_data = l3_data;

	mmToRROutMsg.entry_id = entryId;

	// send the message. 
	if (ERROR == msgQSend(	rm_MsgQId, 
							(char *) &mmToRROutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,
							MSG_PRI_NORMAL
							)) {
		DBG_ERROR("sendIntRRMsg msgQSend (QID=%d) error\n ", (int)rm_MsgQId);
	}
	else {
		DBG_TRACE("Sent msg to RR, Prim: %s, Msg: %s\n", 
					getMMPrimStr(prim), getMMMsgTypeStr(msgType));
	}

	DBG_LEAVE();
}  

// ****************************************************************************
void sendIntCCMsg (	short                      mmId,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType)
{
	DBG_FUNC("sendIntCCMsg", MM_LAYER);
    DBG_ENTER();

	MSG_Q_ID	msgQforCC;
	if ((mmId < 0) || (mmId >= mmCfg.mm_max_mm_connections)){
		DBG_ERROR("Invalid mmId %d, sendIntCCMsg aborted\n", mmId);
		DBG_LEAVE();
		return;
	}
	else if(mmEntries[mmId].procData.procType == MM_PROC_MOB_TERM){
		msgQforCC = mmEntries[mmId].ccQId;
	}
	else{
		T_CNI_L3_ID	ccId = CNI_RIL3_IRT_Get_CC_Id(mmEntries[mmId].entryId);
		msgQforCC = ccId.msgq_id;
	}
	mmToCCOutMsg.module_id = MODULE_MM;
	mmToCCOutMsg.primitive_type = prim;
	mmToCCOutMsg.message_type = msgType;
	mmToCCOutMsg.cause = mmEntries[mmId].cause;

	mmToCCOutMsg.entry_id = mmEntries[mmId].entryId;
	if((prim == INTRA_L3_MM_EST_REJ)&&(msgType == INTRA_L3_RR_PAGE_REQ))
		mmToCCOutMsg.l3_data.pageReq = mmEntries[mmId].pageReq;

	// send the message. 
	#ifndef _NO_CC
	if (ERROR == msgQSend(	msgQforCC, 
							(char *) &mmToCCOutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,
							MSG_PRI_NORMAL
							)) {
		DBG_ERROR("sendIntCCMsg msgQSend (QID=%d) error\n ", (int)msgQforCC);
	}
	else {
		DBG_TRACE("Sent msg to CC, Prim: %s, Msg: %s\n",
					getMMPrimStr(prim), getMMMsgTypeStr(msgType));
		DBG_TRACE(" mmId: %d, mmState: %s\n", mmId, getMMEntryState(mmId));
		DBG_TRACE(" msgQforCC: %x\n", msgQforCC);
	}

	#endif
	DBG_LEAVE();
}  

// ****************************************************************************
void bounceIntCCMsg (	MSG_Q_ID                   msgQforCC,
						IntraL3PrimitiveType_t     prim,
						IntraL3MsgType_t           msgType)
{
	DBG_FUNC("bounceIntCCMsg", MM_LAYER);
    DBG_ENTER();

	mmToCCOutMsg.module_id = MODULE_MM;
	mmToCCOutMsg.primitive_type = prim;
	mmToCCOutMsg.message_type = msgType;
	mmToCCOutMsg.entry_id = -1;

	// send the message to CC using given message queue. 
	#ifndef _NO_CC
	if (ERROR == msgQSend(	msgQforCC, 
							(char *) &mmToCCOutMsg, 
							sizeof(struct  IntraL3Msg_t), 
							NO_WAIT,
							MSG_PRI_NORMAL
							)) {
		DBG_ERROR("bounceIntCCMsg msgQSend (QID=%d) error\n ", (int)msgQforCC);
	}
	else {
		DBG_TRACE("Sent msg to CC, Prim: %s, Msg: %s\n",
					getMMPrimStr(prim), getMMMsgTypeStr(msgType));
	}

	#endif
	DBG_LEAVE();
}  
