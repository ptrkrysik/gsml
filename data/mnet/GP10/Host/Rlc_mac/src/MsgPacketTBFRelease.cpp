// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketTBFRelease.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketTBFRelease.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char tbfRelNames[][64] = 
{
    "TBF not released",
    "TBF released"
};

char tbfRelCauseNames[][64] = 
{
    "Normal release",
    "Abnormal release"
};

// *******************************************************************
// PACKET TBF RELEASE
//
// GSM 04.60 11.2.26
//
// < Packet TBF Release message content > ::=
//	 < PAGE_MODE : bit (2) >
//	 {	0 < GLOBAL_TFI : Global TFI IE >
//		{	< UPLINK_RELEASE : bit (1) >
//			< DOWNLINK_RELEASE : bit (1) >
//			< TBF_RELEASE_CAUSE : bit (4)  = { 0000 | 0010 } >
//			< padding bits >
//			! < Non-distribution part error : bit (*) = < no string > > }
//		! < Address information part error : bit (*) = < no string > > }
//	 ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************



RlcMacResult MsgPacketTBFRelease::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketTBFRelease::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
          
    // Encode Global TFI ie.
    //	 {	0 < GLOBAL_TFI : Global TFI IE >
    if (globalTFI.IsValid())
    {
        dataStream.InsertBits8(0, 1);
        result = globalTFI.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        DBG_ERROR("MsgPacketTBFRelease::EncodeMsg must have valid Global TFI\n");
        result = RLC_MAC_INVALID_IE; 
    }
      
    // Enocde Uplink Release
    //		{	< UPLINK_RELEASE : bit (1) >
    dataStream.InsertBits8((unsigned char)uplinkRel, 1);
    
    // Enocde Downlink Release
    //		{	< DOWNLINK_RELEASE : bit (1) >
    dataStream.InsertBits8((unsigned char)downlinkRel, 1);
    
    // Enocde TBF Release Cause
    //			< TBF_RELEASE_CAUSE : bit (4)  = { 0000 | 0010 } >
    dataStream.InsertBits8((unsigned char)relCause, 1);
    
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketTBFRelease::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketTBFRelease::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Global TFI ie.
        if (globalTFI.IsValid())
        {
            globalTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
      
        // Print out Uplink Release
        MsgDisplayDetailObj.Trace("\tUplink Release --> %s\n", 
            tbfRelNames[uplinkRel]);
    
        // Print out Downlink Release
        MsgDisplayDetailObj.Trace("\tDownlink Release --> %s\n", 
            tbfRelNames[downlinkRel]);
    
        // Print out TBF Release Cause
        MsgDisplayDetailObj.Trace("\tTBF Release Cause --> %s\n", 
            tbfRelCauseNames[relCause]);
    }
   
    DBG_LEAVE();
}





