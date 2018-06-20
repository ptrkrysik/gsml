// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketPDCHRelease.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETPDCHRELEASE_H__
#define __MSGPACKETPDCHRELEASE_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieTimeslotAllocation.h"


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

class MsgPacketPDCHRelease : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                      pageMode;
    IeTimeslotAllocation            tsAllocation;

    // Default constructor
    MsgPacketPDCHRelease() : DownlinkMsgBase(RlcMacMsgType::PACKET_PDCH_RELEASE),
        pageMode(), tsAllocation() 
    {
    }
   
    // Default destructor
    ~MsgPacketPDCHRelease() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif