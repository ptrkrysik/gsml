// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : msgPacketPowerCtlTimingAdvance.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "msgPacketPowerCtlTimingAdvance.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET POWER CONTROL / TIMING ADVANCE
//
// GSM 04.60 11.2.13
//
// < Packet Power Control/Timing Advance message content > ::=
//	 < PAGE_MODE : bit (2) >
//	 {	 { 0	< Global TFI : < Global TFI IE > >
//		 | 110	< TQI : bit (16) >
//		 | 111	< Packet Request Reference : < Packet Request Reference IE > > }
//		 { 0	-- Message escape
//			{	{ 0 | 1 < Global Power Control Parameters : < Global Power Control Parameters IE >> }
//				{ 0	< Global Packet Timing Advance : < Global Packet Timing Advance IE > >
//					< Power Control Parameters : < Power Control Parameters IE > >
//				 | 1 { 0 < Global Packet Timing Advance : < Global Packet Timing Advance IE > >
//					 | 1 < Power Control Parameters : < Power Control parameters IE > > } }
//				< padding bits >
//				 ! < Non-distribution part error : bit (*) = < no string > > }
//			 ! < Message escape : 1 bit (*) = <no string> > }
//		 ! < Address information part error : bit (*) = < no string > > }
//	 ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************



RlcMacResult MsgPacketPowerCtlTimingAdvance::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketPowerCtlTimingAdvance::EncodeMsg", RLC_MAC);
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
    //	 {	 { 0	< Global TFI : < Global TFI IE > >
    if (globalTFI.IsValid())
    {
        dataStream.InsertBits8(0, 1);
        result = globalTFI.EncodeIe(dataStream);
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
    // Encode Packet Request Reference if valid.
    //		 | 111	< Packet Request Reference : < Packet Request Reference IE > > }
    else if (packetReqRef.IsValid())
    {
        dataStream.InsertBits8(7, 3);
        result = packetReqRef.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        DBG_ERROR("MsgPacketPowerCtlTimingAdvance::EncodeMsg must have valid "
            "Global TFI, TQI or Packet Request Ref\n");
        result = RLC_MAC_INVALID_IE; 
    }
    
    //		 { 0	-- Message escape
    dataStream.InsertBits8(0, 1);
    
    // Encode Global Power Control Parameters ie.
    //			{	{ 0 | 1 < Global Power Control Parameters : < Global Power Control Parameters IE >> }
    if (globalPowerControlParams.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = globalPowerControlParams.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }

    // Encode Global Packet Timing Advance and Power Control Parameters.
    //				{ 0	< Global Packet Timing Advance : < Global Packet Timing Advance IE > >
    //					< Power Control Parameters : < Power Control Parameters IE > >
    //				 | 1 { 0 < Global Packet Timing Advance : < Global Packet Timing Advance IE > >
    //					 | 1 < Power Control Parameters : < Power Control parameters IE > > } }
    if ((globalTA.IsValid()) && (powerControlParameters.IsValid()))  
    {
        dataStream.InsertBits8(0, 1);
        result = globalTA.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = powerControlParameters.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result); 
    }
    else 
    {
        dataStream.InsertBits8(1, 1);
        if (globalTA.IsValid())
        {
            dataStream.InsertBits8(0, 1);
            result = globalTA.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else if (powerControlParameters.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = powerControlParameters.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result); 
        }
        else
        {
            DBG_ERROR("MsgPacketPowerCtlTimingAdvance::EncodeMsg must have valid "
                "Global TA or Power Control Params\n");
            result = RLC_MAC_INVALID_IE; 
        }
    }
   
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketPowerCtlTimingAdvance::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketPowerCtlTimingAdvance::DisplayMsgDetail", RLC_MAC);
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
        // Print out Global TQI if valid.
        else if (tqi.IsValid())
        {
            tqi.DisplayDetails(&MsgDisplayDetailObj);
        }
        // Print out Packet Request Reference if valid.
        else if (packetReqRef.IsValid())
        {
            packetReqRef.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out Global Power Control Parameters ie.
        if (globalPowerControlParams.IsValid())
        {
            globalPowerControlParams.DisplayDetails(&MsgDisplayDetailObj);
        }

        // Print out Global Packet Timing Advance and Power Control Parameters.
        if ((globalTA.IsValid()) && (powerControlParameters.IsValid()))  
        {
            globalTA.DisplayDetails(&MsgDisplayDetailObj);
            powerControlParameters.DisplayDetails(&MsgDisplayDetailObj);
        }
        else 
        {
            if (globalTA.IsValid())
            {
                globalTA.DisplayDetails(&MsgDisplayDetailObj);
            }
            else if (powerControlParameters.IsValid())
            {
                powerControlParameters.DisplayDetails(&MsgDisplayDetailObj);
            }
        }
    }
   
    DBG_LEAVE();
}





