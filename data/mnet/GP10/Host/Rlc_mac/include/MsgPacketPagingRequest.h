// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketPagingRequest.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETPAGINGREQUEST_H__
#define __MSGPACKETPAGINGREQUEST_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "iePersistenceLevel.h"
#include "ieNLN.h"
#include "ieTMSI.h"
#include "iePTMSI.h"
#include "ieMobileIdentity.h"
#include "ieChannelNeeded.h"
#include "ieeMLPPPriority.h"

#define MAX_NUM_REPEATED_PAGES 4

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

class MsgPacketPagingRequest : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                      pageMode;
    IePersistenceLevel              persistenceLevel;
    IeNLN                           nln;
    struct RepeatedPageInfoStruct
    {
        bool                        isValid;
        struct TBFPageStruct
        {
            bool                    isValid;
            IePTMSI                 ptmsi;
            IeMobileIdentity        mobileId;
        } TBFPage;
        struct RRConnPageStruct
        {
            bool                    isValid;
            IeTMSI                  tmsi;
            IeMobileIdentity        mobileId;
            IeChannelNeeded         chanNeeded;
            IeeMLPPPriority         emlpPriority;
        } RRConnPage;
    } RepeatedPageInfo[MAX_NUM_REPEATED_PAGES];

    // Default constructor
    MsgPacketPagingRequest() : DownlinkMsgBase(RlcMacMsgType::PACKET_PAGING_REQUEST),
        pageMode(), persistenceLevel(), nln() 
    {
        for (int i = 0; i < MAX_NUM_REPEATED_PAGES; i++)
        {
            RepeatedPageInfo[i].isValid = FALSE;
            RepeatedPageInfo[i].TBFPage.isValid = FALSE;
            RepeatedPageInfo[i].RRConnPage.isValid = FALSE;
        }
    }
   
    // Default destructor
    ~MsgPacketPagingRequest() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif