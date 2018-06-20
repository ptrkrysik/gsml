// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketMobileTBFStatus.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETMOBILETBFSTATUS_H__
#define __MSGPACKETMOBILETBFSTATUS_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "ieGlobalTFI.h"


// *******************************************************************
// PACKET MOBILE TBF STATUS
//
// GSM 04.60 11.2.9c
//
// < Packet Mobile TBF Status message content > ::=	
//      < GLOBAL TFI : < Global TFI IE > >	
//      < TBF_CAUSE : bit (3) >	
//      { 0 | 1	< STATUS_MESSAGE_TYPE : bit (6) > }	
//      < padding bits > ;
//
// *******************************************************************

class MsgPacketMobileTBFStatus : public UplinkMsgBase {
public:

    typedef enum
    {
        NORMAL_EVENT,
        STATUS_UNSPECIFIED,
        NON_DISTRIBUTION_TYPE,
        MESSAGE_ESCAPE,
        INCOMPATIBLE_WITH_CURRENT_STATE
    } TBF_CAUSE;
        
    // Message type is included in MsgBase.
   
    // Message contents
    IeGlobalTFI                     globalTfi;
    TBF_CAUSE                       tbfCause;
    struct
    {
        bool                        isValid;
        RlcMacMsgType               msgType;
    } StatusMsgType;

    // Default constructor
    MsgPacketMobileTBFStatus() : UplinkMsgBase(RlcMacMsgType::PACKET_MOBILE_TBF_STATUS),
        globalTfi(), tbfCause(NORMAL_EVENT)
    {
        StatusMsgType.isValid = FALSE;
    }
   
    // Default destructor
    ~MsgPacketMobileTBFStatus() {}
   
    // Decode input stream of bits into message.
    RlcMacResult DecodeMsg(BitStreamIn &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif