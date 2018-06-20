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
// File        : CCapiViperLink.cpp
// Author(s)   : Bhawani Sapkota
// Create Date : 11-24-1999
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (CC, RR)

#include "logging/VCLOGGING.h"

#include "voip/voipapi.h"
#include "jcc/JCCLog.h"
#include "jcc/JCCUtil.h"
#include "CC/CCInt.h"
#include "CC/CCconfig.h"
#include "CC/CCTypes.h"

// converts the message from network byte ordering format
// to the host byte ordering format

static bool
ccCissUnpackVblinkMsg(CISSVblinkMsg_t &cissMsg,
					  unsigned char buffer[],
					  int size
					  )
{
	CISSVblinkMsg_t &orgMsg = (CISSVblinkMsg_t &) *buffer;
	DBG_FUNC("ccCissUnpackVblinkMsg", CC_CISS_LAYER);
	DBG_ENTER();
	
	
	// Check the integrity of the message first
	cissMsg.Magic = ntohl(orgMsg.Magic);
	if (cissMsg.Magic != VOIP_API_MAGIC_VALUE)
	{
		// corrupt message
		
		DBG_ERROR("Corrupt message (magic number = %d)\n", cissMsg.Magic);
		DBG_LEAVE();
		return false;
	}
	
	cissMsg.msgLen = ntohl(orgMsg.msgLen);
	if (cissMsg.msgLen <= 0 
		|| (int) (cissMsg.msgLen + kCissVbLinkMsgHdrLen) !=  size)
	{
		
		DBG_ERROR("Incorrect msg length (expected = %d, actual = %d)\n",
			cissMsg.msgLen+kCissVbLinkMsgHdrLen,
			size
			);
		
		DBG_LEAVE();
		return false;
		// message size is not correct, corrupt message
	}
	
	cissMsg.callIdx = ntohl(orgMsg.callIdx);
	cissMsg.msgType =  (T_CNI_RIL3_MESSAGE_TYPE) ntohl(orgMsg.msgType);
	cissMsg.status  = ntohl(orgMsg.status);
	memcpy(cissMsg.msg, orgMsg.msg, cissMsg.msgLen);
	
	DBG_LEAVE();
	return true;
}

// Actual call back function called by VBLink task
bool
ccCissProcessViperBaseMsg(LUDB_ID ludbIdx, char * buffer, int size)
{
	CISSVblinkMsg_t cissMsg;
	bool status = false;
	int cause;
	
	DBG_FUNC("ccCissProcessViperBaseMsg", CC_CISS_LAYER);
	DBG_ENTER();
	
	DBG_HEXDUMP((unsigned char *) buffer, size);
	status = ccCissUnpackVblinkMsg(cissMsg, (unsigned char *) buffer, size);
	DBG_TRACE("CISS STATUS = %d\n", cissMsg.status);
 

	if (status)
	{

		MSG_Q_ID qid;
		IntraL3Msg_t ccOutMsg;

		memset((void *) &ccOutMsg, 0, sizeof(IntraL3Msg_t));
		
		ccOutMsg.module_id    = MODULE_CISS;

		if ( cissMsg.callIdx < 0  || cissMsg.callIdx >= CC_MAX_CC_CALLS)
        {
			DBG_ERROR("CISS->CC Error : Bad (Transaction Id = %d)\n",  
				cissMsg.callIdx);
			DBG_LEAVE();
			return false;
      }
		
		ccOutMsg.entry_id       = cissMsg.callIdx;
		
		qid = ccSession[cissMsg.callIdx].msgQId;
		
		ccOutMsg.message_type   = (IntraL3MsgType_t) cissMsg.msgType;

		ccOutMsg.l3_data.cissMsg = cissMsg;
		
		
		// send the message.
		if (ERROR == msgQSend(
			qid, 
			(char *) &ccOutMsg,
			sizeof(CISSVblinkMsg_t), 
			NO_WAIT,    //PR1553 <xxu> WAIT_FOREVER,
			MSG_PRI_NORMAL)
			)
		{
			DBG_ERROR("CISS->CC Error : sendCC msgQSend (QID = %p) error\n ", 
                (int)qid);
			status = false;

		} else {
			DBG_TRACE("CISS Log: Sent Message to CC, (Entry ID = %p), (Msg Type = %d)\n",  
                (int)ccOutMsg.entry_id, 
                ccOutMsg.message_type);
			status = true;
		}
	}
	DBG_LEAVE();
	return status;
	
}

