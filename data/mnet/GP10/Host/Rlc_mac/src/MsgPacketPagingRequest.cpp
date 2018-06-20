// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : msgPacketPagingRequest.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketPagingRequest.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET PAGING REQUEST
//
// GSM 04.60 11.2.10
//
// < Packet Paging Request message content > ::=
//	 < PAGE_MODE : bit (2) >
//	 { 0 | 1 < PERSISTENCE_LEVEL : bit (4) >* 4}
//	 { 0 | 1 < NLN : bit (2) > }
//	 {	{ 1 < Repeated Page info : < Repeated Page info struct > > } ** 0
//		< padding bits > } //		-- truncation at end of message allowed, bits '0' assumed
//	 ! < Distribution part error : bit (*) = < no string > > ;
//
// < Repeated Page info struct > ::=
//	 { 0										-- Page request for TBF establishment
//		 { 0 < PTMSI : bit (32) >
//		 | 1 < Length of Mobile Identity contents : bit (4) >
//			< Mobile Identity : octet (val (Length of Mobile Identity contents)) > }
//	 | 1											-- Page request for RR conn. establishment
//		 { 0 < TMSI : bit (32) >
//		 | 1 < Length of Mobile Identity contents : bit (4) >
//			 < Mobile Identity : octet (val (Length of Mobile Identity contents)) > }
//		 < CHANNEL_NEEDED : bit (2) >
//		 { 0 | 1 < eMLPP_PRIORITY : bit (3) > } }
//	 ! < Ignore : bit (*) = <no string> > ;
//
// *******************************************************************



RlcMacResult MsgPacketPagingRequest::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketPagingRequest::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //   < PAGE_MODE : bit (2) >
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
    
    // Encode NLN ie.
    //	 { 0 | 1 < NLN : bit (2) > }
    if (nln.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = nln.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Repeated Paging Struct
    //	 {	{ 1 < Repeated Page info : < Repeated Page info struct > > } ** 0
    //
    // < Repeated Page info struct > ::=
    //	 { 0										-- Page request for TBF establishment
    //		 { 0 < PTMSI : bit (32) >
    //		 | 1 < Length of Mobile Identity contents : bit (4) >
    //			< Mobile Identity : octet (val (Length of Mobile Identity contents)) > }
    //	 | 1											-- Page request for RR conn. establishment
    //		 { 0 < TMSI : bit (32) >
    //		 | 1 < Length of Mobile Identity contents : bit (4) >
    //			 < Mobile Identity : octet (val (Length of Mobile Identity contents)) > }
    //		 < CHANNEL_NEEDED : bit (2) >
    //		 { 0 | 1 < eMLPP_PRIORITY : bit (3) > } }
    //	 ! < Ignore : bit (*) = <no string> > ;
    for (int i = 0; i < MAX_NUM_REPEATED_PAGES; i++)
    {
        if (RepeatedPageInfo[i].isValid)
        {
            dataStream.InsertBits8(1, 1);
            if (RepeatedPageInfo[i].TBFPage.isValid)
            {
                dataStream.InsertBits8(0, 1);
                if (RepeatedPageInfo[i].TBFPage.ptmsi.IsValid())
                {
                    dataStream.InsertBits8(0, 1);
                    result = RepeatedPageInfo[i].TBFPage.ptmsi.EncodeIe(dataStream);
                    RLC_MAC_RESULT_CHECK(result);
                }
                else if (RepeatedPageInfo[i].TBFPage.mobileId.IsValid())
                {
                    dataStream.InsertBits8(1, 1);
                    result = RepeatedPageInfo[i].TBFPage.mobileId.EncodeIe(dataStream);
                    RLC_MAC_RESULT_CHECK(result);
                }
                else
                {
                    DBG_ERROR("MsgPacketPagingRequest::EncodeMsg Repeated Page info struct"
                        "for TBF must have valid PTMSI or MobileIdentity\n");
                    result = RLC_MAC_INVALID_IE;
                }
            }
            else if (RepeatedPageInfo[i].RRConnPage.isValid)
            {
                dataStream.InsertBits8(1, 1);
                if (RepeatedPageInfo[i].RRConnPage.tmsi.IsValid())
                {
                    dataStream.InsertBits8(0, 1);
                    result = RepeatedPageInfo[i].RRConnPage.tmsi.EncodeIe(dataStream);
                    RLC_MAC_RESULT_CHECK(result);
                }
                else if (RepeatedPageInfo[i].RRConnPage.mobileId.IsValid())
                {
                    dataStream.InsertBits8(1, 1);
                    result = RepeatedPageInfo[i].RRConnPage.mobileId.EncodeIe(dataStream);
                    RLC_MAC_RESULT_CHECK(result);
                }
                else
                {
                    DBG_ERROR("MsgPacketPagingRequest::EncodeMsg Repeated Page info struct"
                        "for TBF must have valid PTMSI or MobileIdentity\n");
                    result = RLC_MAC_INVALID_IE;
                }
                
                result = RepeatedPageInfo[i].RRConnPage.chanNeeded.EncodeIe(dataStream);
                RLC_MAC_RESULT_CHECK(result);
                
                if (RepeatedPageInfo[i].RRConnPage.emlpPriority.IsValid())
                {
                    dataStream.InsertBits8(1, 1);
                    result = RepeatedPageInfo[i].RRConnPage.emlpPriority.EncodeIe(dataStream);
                    RLC_MAC_RESULT_CHECK(result);
                }
                else
                {
                    dataStream.InsertBits8(0, 1);
                }
            }
            else
            {
                DBG_ERROR("MsgPacketPagingRequest::EncodeMsg Repeated Page info struct"
                    "must have valid TBF or RR Conn info\n");
                result = RLC_MAC_INVALID_IE;
            }
        }
    }

    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketPagingRequest::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketPagingRequest::DisplayMsgDetail", RLC_MAC);
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
    
        // Print out NLN ie.
        if (nln.IsValid())
        {
            nln.DisplayDetails(&MsgDisplayDetailObj);
        }
   
        // Print out Repeated Paging Struct
        for (int i = 0; i < MAX_NUM_REPEATED_PAGES; i++)
        {
            if (RepeatedPageInfo[i].isValid)
            {
                if (RepeatedPageInfo[i].TBFPage.isValid)
                {
                    MsgDisplayDetailObj.Trace("\tRepeated Paging Struct %d - TBF Page\n", i);
                    if (RepeatedPageInfo[i].TBFPage.ptmsi.IsValid())
                    {
                        RepeatedPageInfo[i].TBFPage.ptmsi.DisplayDetails(&MsgDisplayDetailObj);
                    }
                    else if (RepeatedPageInfo[i].TBFPage.mobileId.IsValid())
                    {
                        RepeatedPageInfo[i].TBFPage.mobileId.DisplayDetails(&MsgDisplayDetailObj);
                    }
                }
                else if (RepeatedPageInfo[i].RRConnPage.isValid)
                {
                    MsgDisplayDetailObj.Trace("\tRepeated Paging Struct %d - RR Connection\n", i);
                    if (RepeatedPageInfo[i].RRConnPage.tmsi.IsValid())
                    {
                        RepeatedPageInfo[i].RRConnPage.tmsi.DisplayDetails(&MsgDisplayDetailObj);
                    }
                    else if (RepeatedPageInfo[i].RRConnPage.mobileId.IsValid())
                    {
                        RepeatedPageInfo[i].RRConnPage.mobileId.DisplayDetails(&MsgDisplayDetailObj);
                    }
                
                    RepeatedPageInfo[i].RRConnPage.chanNeeded.DisplayDetails(&MsgDisplayDetailObj);
                
                    if (RepeatedPageInfo[i].RRConnPage.emlpPriority.IsValid())
                    {
                        RepeatedPageInfo[i].RRConnPage.emlpPriority.DisplayDetails(&MsgDisplayDetailObj);
                    }
                }
            }
        }
    }
   
    DBG_LEAVE();
}





