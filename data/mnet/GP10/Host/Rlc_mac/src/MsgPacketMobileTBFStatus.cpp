// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketMobileTBFStatus.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketMobileTBFStatus.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char tbfCauseNames[][80] = 
{
    "Normal event",
    "Status, unspecified",
    "Syntactically incorrect message, non-distribution part error",
    "Syntactically incorrect message, message escape",
    "Message not compatible with current protocol state"
};

// *******************************************************************
// PACKET MOBILE TBF STATUS
//
// GSM 04.60 11.2.9c
//
// < Packet Mobile TBF Status message content > ::=	
//      < GLOBAL TFI : < Global TFI IE > >	
//      < TBF_CAUSE : bit (3) >	
//      { 0 | 1	< STATUS_MESSAGE_TYPE : bit (6) > }	
//      < padding bits > ;
//
// *******************************************************************

RlcMacResult MsgPacketMobileTBFStatus::DecodeMsg(BitStreamIn &dataStream)
{
    DBG_FUNC("MsgPacketMobileTBFStatus::DecodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;
   
    // Message Type gets decoded earlier.
   
    // Decode the Global TFI ie.
    //      < GLOBAL TFI : < Global TFI IE > >	
    result = globalTfi.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the TBF Cause.
    //      < TBF_CAUSE : bit (3) >	
    result = dataStream.ExtractBits8(val, 3);
    RLC_MAC_RESULT_CHECK(result);
    tbfCause = (TBF_CAUSE)val;    
    if (tbfCause > INCOMPATIBLE_WITH_CURRENT_STATE)
    {
        tbfCause = STATUS_UNSPECIFIED;    
    }
        
    // Decode the status message type if present    
    //      { 0 | 1	< STATUS_MESSAGE_TYPE : bit (6) > }	
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val == 1)
    {
        result = StatusMsgType.msgType.DecodeMsgType(dataStream); 
        RLC_MAC_RESULT_CHECK(result);
        StatusMsgType.isValid = TRUE;
    }
    else
    {
        StatusMsgType.isValid = FALSE;
    }
    
    DBG_LEAVE();
    return (result);
}


void MsgPacketMobileTBFStatus::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketMobileTBFStatus::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayUplinkMsgType());
      
        // Print out the Global TFI.
        globalTfi.DisplayDetails(&MsgDisplayDetailObj);              
        
        // Print out the TBF_CAUSE.
        MsgDisplayDetailObj.Trace("\tTBF_CAUSE: %s\n", tbfCauseNames[tbfCause]);
        
        // Print out the Status Message Type if valid
        if (StatusMsgType.isValid)
        {
            MsgDisplayDetailObj.Trace("\tSTATUS_MSG_TYPE ---> %s\n", 
                StatusMsgType.msgType.DisplayDownlinkMsgType());
        }
    }
   
    DBG_LEAVE();
}

