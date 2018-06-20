// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketUplinkDummyControlBlock.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETUPLINKDUMMYCONTROLBLOCK_H__
#define __MSGPACKETUPLINKDUMMYCONTROLBLOCK_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "ieTLLI.h"


// *******************************************************************
// PACKET UPLINK DUMMY CONTROL BLOCK
//
// GSM 04.60 11.2.8b
//
// < Packet Uplink Dummy Control Block message content > ::=	
//      < TLLI : bit (32) >	
//      < padding bits > ;
//
// *******************************************************************

class MsgPacketUplinkDummyControlBlock : public UplinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Message contents
    IeTLLI                            tlli;

    // Default constructor
    MsgPacketUplinkDummyControlBlock() : UplinkMsgBase(RlcMacMsgType::PACKET_UPLINK_DUMMY_CONTROL_BLOCK),
        tlli()
    {
    }
   
    // Default destructor
    ~MsgPacketUplinkDummyControlBlock() {}
   
    // Decode input stream of bits into message.
    RlcMacResult DecodeMsg(BitStreamIn &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif