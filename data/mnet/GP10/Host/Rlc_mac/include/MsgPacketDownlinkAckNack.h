// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketDownlinkAckNack.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETDOWNLINKACKNACK_H__
#define __MSGPACKETDOWNLINKACKNACK_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "ieAckNackDescription.h"
#include "ieChannelRequestDescription.h"
#include "ieTFI.h"


// *******************************************************************
// PACKET DOWNLINK ACK NACK
//
// GSM 04.60 11.2.6
//
// < Packet Downlink Ack/Nack message content > ::=	
//      < DOWNLINK_TFI : bit (5) >	
//      < Ack/Nack Description : < Ack/Nack Description IE > >	
//      { 0 | 1 < Channel Request Description : < Channel Request Description IE > > }	
//      < Channel Quality Report : < Channel Quality Report struct > >	
//      < padding bits > ;
//
//      < Channel Quality Report struct > ::=	
//          < C_VALUE : bit (6) >	
//          < RXQUAL : bit (3) >	
//          < SIGN_VAR : bit (6) >	
//          { 0 | 1 < I_LEVEL_TN0 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN1 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN2 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN3 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN4 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN5 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN6 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN7 : bit (4) > } ;
//
// *******************************************************************

class MsgPacketDownlinkAckNack : public UplinkMsgBase {
public:

        
    // Message type is included in MsgBase.
   
    // Message contents
    IeTFI                           dlTfi;
    IeAckNackDescription            ackNackDescription;
    IeChannelRequestDescription     chanReqDescription;
    unsigned char                   cValue;
    unsigned char                   rxqual;
    unsigned char                   signVar;
    struct ILevelStruct
    {
        unsigned char               iLevel;
        bool                        isValid;
    } ILevel[MAX_TIMESLOTS];

    // Default constructor
    MsgPacketDownlinkAckNack() : UplinkMsgBase(RlcMacMsgType::PACKET_DOWNLINK_ACK_NACK),
        ackNackDescription(), chanReqDescription(), cValue(0), rxqual(0), signVar(0)
    {
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            ILevel[i].isValid = FALSE;
        }
    }
   
    // Default destructor
    ~MsgPacketDownlinkAckNack() {}
   
    // Decode input stream of bits into message.
    RlcMacResult DecodeMsg(BitStreamIn &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif