// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketDownlinkDummyControlBlock.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketDownlinkDummyControlBlock.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET DOWNLINK DUMMY CONTROL BLOCK
//
// GSM 04.60 11.2.7
//
// < Packet Downlink Dummy Control Block message content > ::=
//	 < PAGE_MODE : bit (2) >
//	 { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
//	 < padding bits >
//	  ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************



RlcMacResult MsgPacketDownlinkDummyControlBlock::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketDownlinkDummyControlBlock::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
      
    // Encode Persistence Level ie.
    //   { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
    if (persistenceLevel.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = persistenceLevel.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
        
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketDownlinkDummyControlBlock::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketDownlinkDummyControlBlock::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Persistence Level ie.
        if (persistenceLevel.IsValid())
        {
            persistenceLevel.DisplayDetails(&MsgDisplayDetailObj);
        }
    }
   
    DBG_LEAVE();
}





