// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketUplinkAckNack.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketUplinkAckNack.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET UPLINK ACK NACK
//
// GSM 04.60 11.2.28
//
// < Packet Uplink Ack/Nack message content > ::=
//	 < PAGE MODE : bit (2) >
//	 {	00 < UPLINK_TFI : bit (5) >
//		 { 0		-- Message escape
//			 {	 < CHANNEL_CODING_COMMAND : bit (2) >
//				 < Ack/Nack Description : < Ack/Nack Description IE > >
//				 { 0 | 1	< CONTENTION_RESOLUTION_TLLI : bit (32) > }
//				 { 0 | 1	< Packet Timing Advance : < Packet Timing Advance IE > > }
//				 { 0 | 1	< Power Control Parameters : < Power Control Parameters IE > > }
//				 { 0 | 1	< Extension Bits : Extension Bits IE > }				-- sub-clause 12.26
//				 { 0 | 1	< Fixed Allocation Parameters : < Fixed Allocation struct > > }
//				 < padding bits >
//				 ! < Non-distribution part error : bit (*) = < no string > > }
//			 ! < Message escape : 1 bit (*) = <no string> > }
//		 ! < Address information part error : bit (*) = < no string > > }
//	 ! < Distribution part error : bit (*) = < no string > > ;
//
// < Fixed Allocation struct > ::=
//	 < FINAL_ALLOCATION : bit (1) >
//	 { 0	-- Repeat Allocation
//		 < TS_OVERRIDE : bit (8) >
//	 | 1		-- Allocation with Allocation bitmap
//		 < TBF Starting Time : < Starting Frame Number Description IE > >
//		 { 0 | 1	<TIMESLOT_ALLOCATION : bit (8) > }
//		 { 0 { 0		-- with length of Allocation Bitmap
//				 < BLOCKS_OR_BLOCK_PERIODS : bit (1) >
//				 < ALLOCATION_BITMAP_LENGTH : bit (7) >
//				 < ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
//			 | 1 		-- without length of Allocation Bitmap (fills remainder of the message)
//				 < ALLOCATION_BITMAP : bit ** > } } 
//			  ! < Message escape : 1 bit (*) = <no string> >;
//
// *******************************************************************



RlcMacResult MsgPacketUplinkAckNack::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketUplinkAckNack::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //   < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
      
    // Encode Uplink TFI.
    //	 {	00 < UPLINK_TFI : bit (5) >
    dataStream.InsertBits8(0, 2);
    result = uplinkTFI.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);

    //      { 0     -- Message escape
    dataStream.InsertBits8(0, 1);
    
    // Encode Channel Coding Command ie.
    //          {   < CHANNEL_CODING_COMMAND : bit (2) >
    result = chanCoding.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode ACK/NACK Description ie.
    //				 < Ack/Nack Description : < Ack/Nack Description IE > >
    result = ackNackDescription.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode Contention Resolution TLLI ie.
    //				 { 0 | 1	< CONTENTION_RESOLUTION_TLLI : bit (32) > }
    if (contentionResTLLI.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = contentionResTLLI.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Packet Timing Advance ie.
    //				 { 0 | 1	< Packet Timing Advance : < Packet Timing Advance IE > > }
    if (timingAdvance.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = timingAdvance.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Power Control Parameters ie.
    //				 { 0 | 1	< Power Control Parameters : < Power Control Parameters IE > > }
    if (powerControlParameters.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = powerControlParameters.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Not supporting extension bits.
    //				 { 0 | 1	< Extension Bits : Extension Bits IE > }				-- sub-clause 12.26
    dataStream.InsertBits8(0, 1);
    
    // Encode Fixed Allocation struct.
    //				 { 0 | 1	< Fixed Allocation Parameters : < Fixed Allocation struct > > }
    if (fixedAllocation.isValid)
    {
        dataStream.InsertBits8(1, 1);
        result = fixedAllocation.EncodeFixedAllocationStruct(dataStream);
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



void MsgPacketUplinkAckNack::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketUplinkAckNack::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Uplink TFI
        if (uplinkTFI.IsValid())
        {
            uplinkTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Channel Coding Command ie.
        chanCoding.DisplayDetails(&MsgDisplayDetailObj);
   
        // Print out ACK/NACK Description ie.
        ackNackDescription.DisplayDetails(&MsgDisplayDetailObj);
   
        // Print out Contention Resolution TLLI ie.
        if (contentionResTLLI.IsValid())
        {
            contentionResTLLI.DisplayDetails(&MsgDisplayDetailObj);
        }
   
        // Print out Packet Timing Advance ie.
        if (timingAdvance.IsValid())
        {
            timingAdvance.DisplayDetails(&MsgDisplayDetailObj);
        }
       
        // Print out Power Control Parameters ie.
        if (powerControlParameters.IsValid())
        {
            powerControlParameters.DisplayDetails(&MsgDisplayDetailObj);
        }
                
        // Print out Fixed Allocation.
        if (fixedAllocation.isValid)
        {
            fixedAllocation.DisplayFixedAllocationStruct(&MsgDisplayDetailObj);
        }
    }
   
    DBG_LEAVE();
}





