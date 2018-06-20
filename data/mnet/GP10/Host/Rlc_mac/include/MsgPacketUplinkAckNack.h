// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketUplinkAckNack.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETUPLINKACKNACK_H__
#define __MSGPACKETUPLINKACKNACK_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "iePacketTimingAdvance.h"
#include "ieTFI.h"
#include "ieChannelCodingCommand.h"
#include "ieAckNackDescription.h"
#include "FixedAllocationStruct.h"
#include "ieTLLI.h"


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

class MsgPacketUplinkAckNack : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                      pageMode;
    IeTFI                           uplinkTFI;
    IeChannelCodingCommand          chanCoding;
    IeAckNackDescription            ackNackDescription;
    IeTLLI                          contentionResTLLI;
    IePacketTimingAdvance           timingAdvance;
    IePowerControlParameters        powerControlParameters;
    FixedAllocationStruct           fixedAllocation;

    // Default constructor
    MsgPacketUplinkAckNack() : DownlinkMsgBase(RlcMacMsgType::PACKET_UPLINK_ACK_NACK),
        pageMode(), uplinkTFI(), chanCoding(), contentionResTLLI(), ackNackDescription(), 
        timingAdvance(), powerControlParameters(), fixedAllocation() 
    {
    }
   
    // Default destructor
    ~MsgPacketUplinkAckNack() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif