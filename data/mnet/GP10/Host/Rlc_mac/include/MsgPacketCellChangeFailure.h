// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketCellChangeFailure.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETCELLCHANGEFAILURE_H__
#define __MSGPACKETCELLCHANGEFAILURE_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "ieTLLI.h"
#include "ieARFCN.h"
#include "ieBSIC.h"


// *******************************************************************
// PACKET CELL CHANGE FAILURE
//
// GSM 04.60 11.2.26
//
// < Packet Cell Change Failure message content > ::=
//	 < TLLI : bit (32) >
//	 < ARFCN : bit (10) >
//	 < BSIC : bit (6) >
//	 < CAUSE : bit (4) >
//	 < padding bits > ;
//
// *******************************************************************

class MsgPacketCellChangeFailure : public UplinkMsgBase {
public:
    
    typedef enum
    {
        FREQUENCY_NOT_IMPLEMENTED,
        NO_RESPONSE_ON_TARGET_CELL,
        REJECT_ON_TARGET_CELL,
        ON_GOING_CS_CONNECTION,
        ANONYMOUS_ACCESS,
        MS_IN_GMM_STANDBY,
        FORCED_TO_STANDBY,
        INVALID_CAUSE
    } FAILURE_CAUSE;
    
    // Message type is included in MsgBase.
   
    // Message contents
    IeTLLI                          tlli;
    IeARFCN                         arfcn;
    IeBSIC                          bsic;
    FAILURE_CAUSE                   failCause; 

    // Default constructor
    MsgPacketCellChangeFailure() : UplinkMsgBase(RlcMacMsgType::PACKET_CELL_CHANGE_FAILURE),
        tlli(), arfcn(), bsic(), failCause(INVALID_CAUSE)
    {
    }
   
    // Default destructor
    ~MsgPacketCellChangeFailure() {}
   
    // Decode input stream of bits into message.
    RlcMacResult DecodeMsg(BitStreamIn &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif