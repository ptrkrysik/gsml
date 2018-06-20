// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketControlAcknowledgement.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketControlAcknowledgement.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char controlAckNames[][80] = 
{
    "Reserved",
    "one block received with RBSN=1 and no block with the same RTI value and RBSN=0",
    "one block received with RBSN=0 and no block with the same RTI value and RBSN=1",
    "two blocks received with same RTI, one RBSN=1 and the other RBSN=0"
};

// *******************************************************************
// PACKET CONTROL ACKNOWLEDGEMENT
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

RlcMacResult MsgPacketControlAcknowledgement::DecodeMsg(BitStreamIn &dataStream)
{
    DBG_FUNC("MsgPacketControlAcknowledgement::DecodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Message Type gets decoded earlier.
   
    // Decode the TLLI ie if this is not access burst mode.
    //   < TLLI : bit (32) >
    if (msgType.msgType == RlcMacMsgType::PACKET_CONTROL_ACK)
    {
        result = tlli.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);
    }
   
    // Decode the control acknowledgement.
    //	 < CTRL_ACK : bit (2) >
    unsigned char ack;
    result = dataStream.ExtractBits8(ack, 2);
    RLC_MAC_RESULT_CHECK(result);
    ctrlAck = (CTRL_ACK)ack;    
        
    DBG_LEAVE();
    return (result);
}


void MsgPacketControlAcknowledgement::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketControlAcknowledgement::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayUplinkMsgType());
      
        // Print out the TLLI.
        tlli.DisplayDetails(&MsgDisplayDetailObj);              
        
        // Print out the control ack.
        MsgDisplayDetailObj.Trace("\tControl Ack: %s\n", controlAckNames[ctrlAck]);
    }
   
    DBG_LEAVE();
}

