// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketControlAcknowledgement.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETCONTROLACKNOWLEDGEMENT_H__
#define __MSGPACKETCONTROLACKNOWLEDGEMENT_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "ieTLLI.h"


// *******************************************************************
// PACKET CELL CONTROL ACKNOWLEDGEMENT
//
// GSM 04.60 11.2.2
//
// < Packet Control Acknowledgement message content > ::= -- RLC/MAC control block format   
//      < TLLI : bit (32) > 
//      < CTRL_ACK : bit (2) >  
//      < padding bits > ;
//
// < Packet Control Acknowledgement 11 bit message > ::=  -- 11-bit access burst format 
//      < MESSAGE_TYPE : bit (9) == 1111 1100 1 >   
//      < CTRL_ACK : bit (2) > ;
//
// < Packet Control Acknowledgement 8 bit message > ::=   -- 8-bit access burst format  
//      < MESSAGE_TYPE : bit (6) == 0111 11 >   
//      < CTRL_ACK : bit (2) > ;
//
// *******************************************************************

class MsgPacketControlAcknowledgement : public UplinkMsgBase {
public:

    typedef enum
    {
        RESERVED,
        ONE_BLOCK_RBSN_EQ_1,
        ONE_BLOCK_RBSN_EQ_0,
        TWO_BLOCKS_RBSN_EQ_0_AND_1
    } CTRL_ACK;
        
    // Message type is included in MsgBase.
   
    // Message contents
    IeTLLI                          tlli;
    CTRL_ACK                        ctrlAck;

    // Default constructor
    MsgPacketControlAcknowledgement(RlcMacMsgType type) : UplinkMsgBase(type),
        tlli(), ctrlAck(RESERVED)
    {
    }
   
    // Default destructor
    ~MsgPacketControlAcknowledgement() {}
   
    // Decode input stream of bits into message.
    RlcMacResult DecodeMsg(BitStreamIn &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif