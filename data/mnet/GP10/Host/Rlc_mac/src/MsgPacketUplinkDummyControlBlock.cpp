// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketUplinkDummyControlBlock.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketUplinkDummyControlBlock.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET UPLINK DUMMY CONTROL BLOCK
//
// GSM 04.60 11.2.8b
//
// < Packet Uplink Dummy Control Block message content > ::=	
//      < TLLI : bit (32) >	
//      < padding bits > ;
//
// *******************************************************************

RlcMacResult MsgPacketUplinkDummyControlBlock::DecodeMsg(BitStreamIn &dataStream)
{
    DBG_FUNC("MsgPacketUplinkDummyControlBlock::DecodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Message Type gets decoded earlier.
   
    // Decode the TLLI ie.
    //   < TLLI : bit (32) >
    result = tlli.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    DBG_LEAVE();
    return (result);
}


void MsgPacketUplinkDummyControlBlock::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketUplinkDummyControlBlock::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayUplinkMsgType());
      
        // Print out the TLLI.
        tlli.DisplayDetails(&MsgDisplayDetailObj);              
    }
   
    DBG_LEAVE();
}

