// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketPollingRequest.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETPOLLINGREQUEST_H__
#define __MSGPACKETPOLLINGREQUEST_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieGlobalTFI.h"
#include "ieTLLI.h"
#include "ieTQI.h"


// *******************************************************************
// PACKET POLLING REQUEST
//
// GSM 04.60 11.2.12
//
// < Packet Polling Request message content > ::=
//   < PAGE_MODE : bit (2) >
//   {  { 0 < Global TFI : < Global TFI IE > >
//      | 10    < TLLI : bit (32) >
//      | 110   < TQI : bit (16) > }
//      < TYPE_OF_ACK : bit (1) >
//      {   < padding bits >
//          ! < Non-distribution part error : bit (*) = < no string > > }
//       ! < Address information part error : bit (*) = < no string > > }
//    ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************

class MsgPacketPollingRequest : public DownlinkMsgBase {
public:

    typedef enum
    {
        ACK_MSG_FOUR_ACCESS_BURSTS,
        ACK_MSG_RLCMAC_CONTROL_BLOCK,
        INVALID_TYPE_OF_ACK=-1
    } TYPE_OF_ACK;
    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode          pageMode;
    IeGlobalTFI         globalTFI;
    IeTLLI              tlli;
    IeTQI               tqi;
    TYPE_OF_ACK         typeOfAck;
                                                 
    // Default constructor
    MsgPacketPollingRequest() : DownlinkMsgBase(RlcMacMsgType::PACKET_POLLING_REQUEST),
        pageMode(), globalTFI(), tlli(), tqi(), typeOfAck(INVALID_TYPE_OF_ACK)
    {
    }
   
    // Default destructor
    ~MsgPacketPollingRequest() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif