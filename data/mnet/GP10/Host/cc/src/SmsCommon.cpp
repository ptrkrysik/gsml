// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : SmsCommon.cpp
// Author(s)   : Igal Gutkin
// Create Date : 01-20-2000
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files. SMS common functions
// *******************************************************************
//

#include <vxworks.h>
#include <string.h>

#include "Logging\vcmodules.h"

#include "CC/CCSessionHandler.h"

#include "CC/SmsLeg.h"


// call the vbLink routine to send message
static bool ccSmsSendToViberBase(LUDB_ID ludbIdx, SmsVblinkMsg_t smsMsg)
{
	unsigned char buffer[sizeof (SmsVblinkMsg_t)];
	int msgLen;
	bool status = false;
	
	DBG_FUNC  ("ccSmsSendToViberBase", CC_SMS_LAYER);
	DBG_ENTER ();
/*
	if (ccSmsPackVblinkMsg(buffer, msgLen, cissMsg))
	{
		DBG_TRACE("Ludb Index = %d, Buffer Len = %d\n", ludbIdx, msgLen);
		DBG_HEXDUMP(buffer, msgLen);
		status = VBLinkMsToNetCiss(ludbIdx, buffer, msgLen);
	}
*/	
	DBG_LEAVE  ();
	return status;
}
