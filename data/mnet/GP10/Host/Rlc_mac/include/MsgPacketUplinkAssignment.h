// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketUplinkAssignment.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETUPLINKASSIGNMENT_H__
#define __MSGPACKETUPLINKASSIGNMENT_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "iePersistenceLevel.h"
#include "ieTLLI.h"
#include "iePacketReqRef.h"
#include "ieGlobalTFI.h"
#include "ieTQI.h"
#include "iePacketTimingAdvance.h"
#include "ieFrequencyParameters.h"
#include "ieChannelCodingCommand.h"
#include "DynamicAllocationStruct.h"
#include "SingleBlockAllocationStruct.h"
#include "FixedAllocationStruct.h"


// *******************************************************************
// PACKET UPLINK ASSIGNMENT
//
// GSM 04.60 11.2.29
//
// < Packet Uplink Assignment message content > ::=
//	 < PAGE_MODE : bit (2) >
//	 { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
//	 {	 {  0	< Global TFI : < Global TFI IE > >
//		 | 10	< TLLI : bit (32) >
//		 | 110	< TQI : bit (16) >
//		 | 111	< Packet Request Reference : < Packet Request Reference IE > > }
//		 { 0		-- Message escape
//			{	< CHANNEL_CODING_COMMAND : bit (2) >
//				< TLLI_BLOCK_CHANNEL_CODING : bit (1) >
//				< Packet Timing Advance : < Packet Timing Advance IE > >
//				{ 0 | 1	< Frequency Parameters : < Frequency Parameters IE > > }
//				{ 01	<Dynamic Allocation : < Dynamic Allocation struct > >
//				 | 10	<Single Block Allocation : < Single Block Allocation struct > >
//				 | 00	< extension >
//				 | 11	< Fixed allocation : < Fixed Allocation struct > > }
//				< padding bits >
//				 ! < Non-distribution part error : bit (*) = < no string > > } 
//			 ! < Message escape : 1 bit (*) = <no string> > }
//		 ! < Address information part error : bit (*) = < no string > > } 
//	 ! < Distribution part error : bit (*) = < no string > > ;
//
// <extension> ::=	-- Future extension can be done by modifying this structure
//	 null ;
//
// <Dynamic Allocation struct > ::= 
//	 < Extended Dynamic Allocation : bit (1) >
// 	 { 0 | 1 < P0 : bit (4) > 
// 			< PR_MODE : bit (1) > }
//	 < USF_GRANULARITY : bit (1) >
//	 { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//	 { 0 | 1 < RLC_DATA_BLOCKS_GRANTED : bit (8) > }
//	 { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
//	 { 0						-- Timeslot Allocation
//		{ 0 | 1 < USF_TN0 : bit (3) > }
//		{ 0 | 1 < USF_TN1 : bit (3) > }
//		{ 0 | 1 < USF_TN2 : bit (3) > }
//		{ 0 | 1 < USF_TN3 : bit (3) > }
//		{ 0 | 1 < USF_TN4 : bit (3) > }
//		{ 0 | 1 < USF_TN5 : bit (3) > }
//		{ 0 | 1 < USF_TN6 : bit (3) > }
//		{ 0 | 1 < USF_TN7 : bit (3) > }
//	 | 1						-- Timeslot Allocation with Power Control Parameters
//		< ALPHA : bit (4) >
//		{ 0 | 1	< USF_TN0 : bit (3) >
//				< GAMMA_TN0 : bit (5) > }
//		{ 0 | 1	< USF_TN1 : bit (3) >
//				< GAMMA_TN1 : bit (5) > }
//		{ 0 | 1	< USF_TN2 : bit (3) >
//				< GAMMA_TN2 : bit (5) > }
//		{ 0 | 1	< USF_TN3 : bit (3) >
//				< GAMMA_TN3 : bit (5) > }
//		{ 0 | 1	< USF_TN4 : bit (3) >
//				< GAMMA_TN4 : bit (5) > }
//		{ 0 | 1	< USF_TN5 : bit (3) >
//				< GAMMA_TN5 : bit (5) > }
//		{ 0 | 1	< USF_TN6 : bit (3) >
//	 			< GAMMA_TN6 : bit (5) > }
//		{ 0 | 1	< USF_TN7 : bit (3) >
//				< GAMMA_TN7 : bit (5) > } } ;
//
// <Single Block Allocation struct > ::=
//	 < TIMESLOT_NUMBER : bit (3) > 
//	 { 0 | 1 < ALPHA : bit (4) >
//	 < GAMMA_TN : bit (5) >}
//	 { 0 | 1 < P0 : bit (4) >
//			 < BTS_PWR_CTRL_MODE : bit (1) > 
// 			 < PR_MODE : bit (1) > }
//	 < TBF Starting Time : < Starting Frame Number Description IE > > ;
//
// <Fixed Allocation struct > ::= 
//	 { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//	 < FINAL_ALLOCATION : bit (1) >
//	 < DOWNLINK_CONTROL_TIMESLOT: bit (3) >
//	 { 0 | 1 < P0 : bit (4) >
//			< BTS_PWR_CTRL_MODE : bit (1) > 
// 			< PR_MODE : bit (1) > }
//	 { 0	< TIMESLOT_ALLOCATION : bit (8) >
//	 | 1	< Power Control Parameters : < Power Control Parameters IE > > }
// 	 < HALF_DUPLEX_MODE : bit (1) >
//	 < TBF Starting Time : < Starting Frame Number Description IE > > 
//	 { 0	{ 0			-- with length of Allocation Bitmap
//			< BLOCKS_OR_BLOCK_PERIODS : bit (1) >
//			< ALLOCATION_BITMAP_LENGTH : bit (7) > 
//			< ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
//		 | 1			-- without length of Allocation Bitmap (fills remainder of the message)
//			< ALLOCATION_BITMAP : bit ** > } 
//		 ! < Message escape : 1 bit (*) = <no string> > } ;
//
// *******************************************************************

class MsgPacketUplinkAssignment : public DownlinkMsgBase {
public:

	typedef enum
	{
		USE_CS1_FOR_DATA_BLK_WITH_TLLI,
		USE_COMMANDED_CS_FOR_DATA_BLOCK_WITH_TLLI
	} TLLI_BLOCK_CHANNEL_CODING;
	// Message type is included in MsgBase.
   
	// Distributed message contents
	IePageMode                      pageMode;  
    struct PersistenceLevelStruct
    {
        IePersistenceLevel          persistenceLevel[4];
        bool                        isValid;
    } PersistenceLevelParams;
	IeGlobalTFI                     globalTFI;
	IeTLLI                          tlli;
	IeTQI                           tqi;
	IePacketReqRef                  packetReqRef;
	IePacketTimingAdvance           timingAdvance;
	IeChannelCodingCommand          chanCoding;
	TLLI_BLOCK_CHANNEL_CODING       tlliBlockCoding;
	IeFrequencyParameters			frequencyParameters;
	DynamicAllocationStruct         dynamicAllocation;
	SingleBlockAllocationStruct     singleBlockAllocation;
	FixedAllocationStruct           fixedAllocation;

	// Default constructor
	MsgPacketUplinkAssignment() : DownlinkMsgBase(RlcMacMsgType::PACKET_UPLINK_ASSIGNMENT),
		pageMode(), globalTFI(), tlli(), tqi(), packetReqRef(),
		timingAdvance(), frequencyParameters(), dynamicAllocation(), singleBlockAllocation(),
		fixedAllocation() 
	{
        PersistenceLevelParams.isValid = FALSE;
	}
   
	// Default destructor
	~MsgPacketUplinkAssignment() {}
   
	// Pack IE into an output stream of bits.
	RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
	// Display IE details.
	void DisplayMsgDetail();
};



#endif