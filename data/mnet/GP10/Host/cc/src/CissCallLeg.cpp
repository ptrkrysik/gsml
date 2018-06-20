// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 0.1
// Status      : Under development
// File        : CissCallLeg.cpp
// Author(s)   : Bhawani Sapkota
// Create Date : 11-22-1999
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include "CC/CallLeg.h"
#include "CC/CCSessionHandler.h"


#define CC_CISS_LAYER 3

//  #include "cdr.h"
//  #include "CallCDR.h"

// *******************************************************************
// 
// *******************************************************************
bool VBLinkMsToNetCiss(LUDB_ID, unsigned char *, int);



// converts the message from host byte ordering format
// to the network byte ordering
static bool
ccCissPackVblinkMsg(unsigned char buffer[],
					int &size,				
					CISSVblinkMsg_t &cissMsg
					)
{
	CISSVblinkMsg_t dstMsg;
	size = cissMsg.msgLen + kCissVbLinkMsgHdrLen;
	bool status = false;
	
	DBG_FUNC("ccCissPackVblinkMsg", CC_CISS_LAYER);
	DBG_ENTER();
	
	
	if (size > kCissVbLinkMsgHdrLen && size <= sizeof(CISSVblinkMsg_t))
	{
		dstMsg.Magic = htonl(VOIP_API_MAGIC_VALUE);
		dstMsg.callIdx = htonl(cissMsg.callIdx);
		dstMsg.msgType =  htonl(cissMsg.msgType);
		dstMsg.status = htonl(cissMsg.status);
		dstMsg.msgLen =  htonl(cissMsg.msgLen);
		memcpy(dstMsg.msg, cissMsg.msg, cissMsg.msgLen);
		memcpy(buffer, (char  *)&dstMsg, size);
		status = true;
	} else {
		DBG_ERROR("Incorrect message size = %d\n", cissMsg.msgLen);
	}
	
	DBG_LEAVE();
	return status;
}


// call the vbLink routine to send message
static bool ccCissSendToViberBase(LUDB_ID ludbIdx, CISSVblinkMsg_t& cissMsg)
{
	unsigned char buffer[sizeof (CISSVblinkMsg_t)];
	int msgLen;
	bool status = false;
	
	DBG_FUNC("ccCissSendToViberBase", CC_CISS_LAYER);
	DBG_ENTER();

	if (ccCissPackVblinkMsg(buffer, msgLen, cissMsg))
	{
		DBG_TRACE("Ludb Index = %d, Buffer Len = %d\n", ludbIdx, msgLen);
		DBG_HEXDUMP(buffer, msgLen);
		status = VBLinkMsToNetCiss(ludbIdx, buffer, msgLen);
	}
	
	DBG_LEAVE();
	return status;
}

// *******************************************************************
// forward declarations.
// *******************************************************************


CISSCallLeg::CISSCallLeg (CCSessionHandler *session,
						  int callId, 
						  MSG_Q_ID qid,
						  short callLegNum):cissState_(CISS_ST_IDLE)
{
  CallLeg::parent = session;
  CallLeg::callIndex = callId;
  CallLeg::msgQId = qid;
  CallLeg::callLegNum = callLegNum;
  initData();
	return;
	
}


CISSCallLeg::~CISSCallLeg()
{
	return;
}

void
CISSCallLeg::cleanup (T_CNI_RIL3_CAUSE_VALUE cause)
{
	DBG_FUNC("CISSCallLeg::cleanup", CC_CISS_LAYER);
	DBG_ENTER();
	initData();
	DBG_LEAVE();
	return;
}

void
CISSCallLeg::initData ()
{
	DBG_FUNC("CISSCallLeg::initData", CC_CISS_LAYER);
	DBG_ENTER();
	//Just call the parent's routine
	CallLeg::initData ();
	DBG_LEAVE();
	
}

int
CISSCallLeg::handleTimeoutMsg(IntraL3Msg_t    *ccInMsg)
{
  DBG_FUNC("CISSCallLeg::handleTimeoutMsg", CC_CALLLEG_LAYER);
  DBG_ENTER();
  
  DBG_ERROR("Call Leg Error : Unexpected Timeout Message\n");
  
  DBG_LEAVE();
  return(0);
}


int
CISSCallLeg::handleMsMsg(T_CNI_RIL3MD_CCMM_MSG  *msInMsg)
{
	int retval = 0;
	CISSVblinkMsg_t cissVbLinkMsg;
	unsigned char buffer[sizeof(CISSVblinkMsg_t)];
	int msgLen;
	
	DBG_FUNC("CISSCallLeg::handleMsMsg", CC_CISS_LAYER);
	DBG_ENTER();
	
	cissVbLinkMsg.msgLen = msInMsg->l3_data.msgLength;
	if (cissVbLinkMsg.msgLen > CNI_LAPDM_MAX_L3MSG_LENGTH || cissVbLinkMsg.msgLen <=0)
	{
		DBG_ERROR("CISS:Incorrect CISS Msg Len (Message len = %d)\n", cissVbLinkMsg.msgLen);
		retval = 0-CNI_RIL3_CAUSE_INVALID_MANDATORY_INFO;
		this->idle = true;
		this->cissState_ = CISS_ST_IDLE;
	} else {
		
		cissVbLinkMsg.callIdx = this->callIndex;
		memcpy(cissVbLinkMsg.msg, msInMsg->l3_data.buffer, cissVbLinkMsg.msgLen);
		cissVbLinkMsg.msgType = CNI_RIL3SS_MSGID_REGISTER;
		cissVbLinkMsg.status = 0;
		if (ccCissSendToViberBase(this->ludbIndex(), cissVbLinkMsg))
		{
			this->idle = false;
		    this->cissState_ = CISS_ST_VB;
		} else {
			DBG_ERROR("CISS: Error Sending message to ViperBase\n");
			retval = 0-CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
			this->idle = true;
			this->cissState_ = CISS_ST_IDLE;
		}
		
	}
	 
	DBG_LEAVE();
	return retval;
}

int
CISSCallLeg::handleVbMsg(CISSVblinkMsg_t &netInMsg)
{
	int retval;
	
	DBG_FUNC("CISSCallLeg::handleVbMsg", CC_CISS_LAYER);
	DBG_ENTER();
	
	T_CNI_LAPDM_L3MessageUnit l3msg;

	l3msg.msgLength = (unsigned short) netInMsg.msgLen;
	for(int i=0;i<l3msg.msgLength; i++) l3msg.buffer[i]= netInMsg.msg[i];
	
	DBG_TRACE("CISS network message status = %d\n", netInMsg.status);

	DBG_TRACE("CISS-->LAPDm CISS Message to Mobile (Type = %d)\n", netInMsg.msgType);	
	DBG_TRACE( "Oid = %d, Sapimain = %d, length=%d\n", oid(), SAPI_MAIN, l3msg.msgLength);
	DBG_HEXDUMP(l3msg.buffer, l3msg.msgLength);
	
	// send the message to LAPDm for delivery
	// call the session handler sendL2 method
	// if call is handed over to anotehr cell, sendL2 will take care of
	// sending the Mobile message to the correct cell.
	parent->sendL2Msg(&l3msg);


	if (netInMsg.msgType == CNI_RIL3SS_MSGID_RELEASE_COMPLETE)
	{
		retval = 0 - netInMsg.status;
		idle = true;
		cissState_ = CISS_ST_IDLE;
		
	} else {
		idle = false;
		cissState_ = CISS_ST_MS;
		retval = 0;
	}
	DBG_TRACE( "idle = %d, cissState_ = %d, retval = %d\n", idle, cissState_, retval);
    DBG_LEAVE();
	return retval;
}


void
CISSCallLeg::printData (JCCPrintStFn fnPtr)
{
	char * tpClStr1Ptr = "CISS Call Leg :\n";
	char tpClStr2[120];
	
	// Pre-format the call data Strings.
	sprintf(&tpClStr2[0],
		"(callIndex = %d) (callRefValue= %d) (CISS State = %d) (ludbIdx = %d) \n",
		callIndex,
		callRefValue,
		cissState_,
		ludbIndex()
		);
	(*fnPtr)(tpClStr1Ptr);
	(*fnPtr)(&tpClStr2[0]);
	
}






