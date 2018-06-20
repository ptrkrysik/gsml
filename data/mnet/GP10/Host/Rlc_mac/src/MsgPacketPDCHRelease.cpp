// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketPDCHRelease.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketPDCHRelease.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET PDCH RELEASE
//
// GSM 04.60 11.2.11
//
// < Packet PDCH Release message content > ::=
//   < PAGE_MODE : bit (2) >
//   { 0 | 1 < TIMESLOTS_AVAILABLE : bit (8) > }
//   < padding bits >
//    ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************



RlcMacResult MsgPacketPDCHRelease::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketPDCHRelease::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
          
    // Encode TIMESLOTS_AVAILABLE ie.
    //   { 0 | 1 < TIMESLOTS_AVAILABLE : bit (8) > }
    if (tsAllocation.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = tsAllocation.EncodeIe(dataStream);
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



void MsgPacketPDCHRelease::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketPDCHRelease::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Timeslot Allocation ie.
        if (tsAllocation.IsValid())
        {
            tsAllocation.DisplayDetails(&MsgDisplayDetailObj);
        }
    
    }
   
    DBG_LEAVE();
}





