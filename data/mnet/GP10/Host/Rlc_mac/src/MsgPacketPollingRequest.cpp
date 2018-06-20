// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketPollingRequest.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketPollingRequest.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char typeOfAckNames[][64] = 
{
    "Packet Control Ack sent as four access bursts",
    "Packet Control Ack sent as RLC/MAC control block"
};

// *******************************************************************
// PACKET POLLING REQUEST
//
// GSM 04.60 11.2.12
//
// < Packet Polling Request message content > ::=
//	 < PAGE_MODE : bit (2) >
//	 {	{ 0	< Global TFI : < Global TFI IE > >
//		| 10	< TLLI : bit (32) >
//		| 110	< TQI : bit (16) > }
//		< TYPE_OF_ACK : bit (1) >
//		{	< padding bits >
//			! < Non-distribution part error : bit (*) = < no string > > }
//		 ! < Address information part error : bit (*) = < no string > > }
//	  ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************



RlcMacResult MsgPacketPollingRequest::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketPollingRequest::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
          
    // Encode Global TFI if valid.
    //	 {	{ 0	< Global TFI : < Global TFI IE > >
    if (globalTFI.IsValid())
    {
        dataStream.InsertBits8(0, 1);
        result = globalTFI.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    // Encode Global TLLI if valid.
    //		| 10	< TLLI : bit (32) >
    else if (tlli.IsValid())
    {
        dataStream.InsertBits8(2, 2);
        result = tlli.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    // Encode Global TQI if valid.
    //		| 110	< TQI : bit (16) > }
    else if (tqi.IsValid())
    {
        dataStream.InsertBits8(6, 3);
        result = tqi.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        DBG_ERROR("MsgPacketPollingRequest::EncodeMsg must have valid "
            "Global TFI, TLLI or TQI\n");
        result = RLC_MAC_INVALID_IE; 
    }
    
    if (typeOfAck != INVALID_TYPE_OF_ACK)
    {
        dataStream.InsertBits8((unsigned char)typeOfAck, 1);
    }
    else
    {
        DBG_ERROR("MsgPacketPollingRequest::EncodeMsg must have valid "
            "TYPE_OF_ACK parameter\n");
        result = RLC_MAC_INVALID_IE; 
    }
        
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketPollingRequest::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketPollingRequest::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Global TFI if valid.
        if (globalTFI.IsValid())
        {
            globalTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Global TLLI if valid.
        else if (tlli.IsValid())
        {
            tlli.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Global TQI if valid.
        else if (tqi.IsValid())
        {
            tqi.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        if (typeOfAck != INVALID_TYPE_OF_ACK)
        {
            MsgDisplayDetailObj.Trace("\tTYPE_OF_ACK ---> %s\n", typeOfAckNames[typeOfAck]);
        }    
    }
   
    DBG_LEAVE();
}





