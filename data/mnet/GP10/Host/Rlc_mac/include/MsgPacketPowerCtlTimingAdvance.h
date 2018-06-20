// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketPowerCtlTimingAdvance.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETPOWERCTLTIMINGADVANCE_H__
#define __MSGPACKETPOWERCTLTIMINGADVANCE_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieGlobalTFI.h"
#include "ieTQI.h"
#include "iePacketReqRef.h"
#include "ieGlobalPowerControlParms.h"
#include "iePowerControlParameters.h"
#include "ieGlobalPacketTimingAdvance.h"


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

class MsgPacketPowerCtlTimingAdvance : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                  pageMode;
    IeGlobalTFI                 globalTFI;
    IeTQI                       tqi;
    IePacketReqRef              packetReqRef;
    IeGlobalPowerControlParms   globalPowerControlParams;
    IeGlobalPacketTimingAdvance globalTA;
    IePowerControlParameters    powerControlParameters;

    // Default constructor
    MsgPacketPowerCtlTimingAdvance() : 
        DownlinkMsgBase(RlcMacMsgType::PACKET_POWER_CONTROL_TIMING_ADVANCE),
        pageMode(), globalTFI(), tqi(), globalPowerControlParams(), 
        packetReqRef(), globalTA(), powerControlParameters() 
    {
    }
   
    // Default destructor
    ~MsgPacketPowerCtlTimingAdvance() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif