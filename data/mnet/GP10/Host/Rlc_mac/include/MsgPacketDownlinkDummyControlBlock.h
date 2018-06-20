// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketDownlinkDummyControlBlock.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETDOWNLINKDUMMYCONTROL_BLOCK_H__
#define __MSGPACKETDOWNLINKDUMMYCONTROL_BLOCK_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "iePersistenceLevel.h"


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

class MsgPacketDownlinkDummyControlBlock : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                      pageMode;
    IePersistenceLevel              persistenceLevel;

    // Default constructor
    MsgPacketDownlinkDummyControlBlock() : 
        DownlinkMsgBase(RlcMacMsgType::PACKET_DOWNLINK_DUMMY_CONTROL_BLOCK),
        pageMode(), persistenceLevel() 
    {
    }
   
    // Default destructor
    ~MsgPacketDownlinkDummyControlBlock() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif