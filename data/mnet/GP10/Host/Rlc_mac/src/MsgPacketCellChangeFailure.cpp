// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketCellChangeFailure.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketCellChangeFailure.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char causeNames[][64] = 
{
    "Frequency not implemented",
    "No response on target cell",
    "Imm. Assign Reject or Packet Access Reject on target cell",
    "On going CS connection",
    "Anonymous Access",
    "MS in GMM Standby state",
    "Forced to the Standby state",
    "Reserved"
};

// *******************************************************************
// PACKET CELL CHANGE FAILURE
//
// GSM 04.60 11.2.26
//
// < Packet Cell Change Failure message content > ::=
//	 < TLLI : bit (32) >
//	 < ARFCN : bit (10) >
//	 < BSIC : bit (6) >
//	 < CAUSE : bit (4) >
//	 < padding bits > ;
//
// *******************************************************************

RlcMacResult MsgPacketCellChangeFailure::DecodeMsg(BitStreamIn &dataStream)
{
    DBG_FUNC("MsgPacketCellChangeFailure::DecodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Message Type gets decoded earlier.
   
    // Decode the TLLI ie.
    //	 < TLLI : bit (32) >
    result = tlli.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the ARFCN ie.
    //	 < ARFCN : bit (10) >
    result = arfcn.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the BSIC ie.
    //	 < BSIC : bit (6) >
    result = bsic.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the CAUSE ie.
    //	 < CAUSE : bit (4) >
    unsigned char cause;
    result = dataStream.ExtractBits8(cause, 4);
    RLC_MAC_RESULT_CHECK(result);
    if (failCause > INVALID_CAUSE) 
        failCause = INVALID_CAUSE;
    else
        failCause = (FAILURE_CAUSE)cause;    
   
    DBG_LEAVE();
    return (result);
}


void MsgPacketCellChangeFailure::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketCellChangeFailure::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayUplinkMsgType());
      
        // Print out the TLLI.
        tlli.DisplayDetails(&MsgDisplayDetailObj);
              
        // Print out the ARFCN
        arfcn.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the BSIC
        bsic.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the failure cause.
        MsgDisplayDetailObj.Trace("\tFailure Cause: %s\n", causeNames[failCause]);
    }
   
    DBG_LEAVE();
}

