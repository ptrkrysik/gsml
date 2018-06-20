// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketTBFRelease.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETTBFRELEASE_H__
#define __MSGPACKETTBFRELEASE_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieGlobalTFI.h"


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

class MsgPacketTBFRelease : public DownlinkMsgBase {
public:

    typedef enum
    {
        TBF_NOT_RELEASED,
        TBF_RELEASED
    } TBF_RELEASE_INDICATION;
    
    typedef enum
    {
        NORMAL_RELEASE,
        ABNORMAL_RELEASE
    } TBF_RELEASE_CAUSE;
    
    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                        pageMode;
    IeGlobalTFI                       globalTFI;
    TBF_RELEASE_INDICATION          uplinkRel;
    TBF_RELEASE_INDICATION          downlinkRel;
    TBF_RELEASE_CAUSE               relCause; 

    // Default constructor
    MsgPacketTBFRelease() : DownlinkMsgBase(RlcMacMsgType::PACKET_TBF_RELEASE),
        pageMode(), globalTFI(), uplinkRel(TBF_NOT_RELEASED), 
            downlinkRel(TBF_NOT_RELEASED) , relCause(NORMAL_RELEASE)
    {
    }
   
    // Default destructor
    ~MsgPacketTBFRelease() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif