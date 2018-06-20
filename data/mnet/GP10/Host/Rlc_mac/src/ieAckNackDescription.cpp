// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieAckNackDescription.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieAckNackDescription.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


char finalAckNames[][64] =
{
    "retransmission are requested and TBF is incomplete",
    "no retransmissions requested and all RLC blocks acknowledged"
};

// *******************************************************************
// class IeAckNackDescription
//
// Description:
//    Ack Nack Dscription IE -- GSM04.60 12.3
//
// < Ack/Nack Description IE > ::=
//   < FINAL_ACK_INDICATION : bit (1) >
//   < STARTING_SEQUENCE_NUMBER : bit (7) >
//   < RECEIVED_BLOCK_BITMAP : bit (64) > ;
// *******************************************************************

RlcMacResult IeAckNackDescription::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeAckNackDescription::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack ACK/NACK Description bits into the output bit stream.
    if (isValid)
    {
        //   < FINAL_ACK_INDICATION : bit (1) >
        dataStream.InsertBits8((unsigned char)finalAckIndication, 1);
        
        //   < STARTING_SEQUENCE_NUMBER : bit (7) >
        dataStream.InsertBits8(startingSeqNum, 7);
        
        //   < RECEIVED_BLOCK_BITMAP : bit (64) > ;
        dataStream.InsertBytesN(recBlockBitmap, RRB_SIZE);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeAckNackDescription::EncodeIe ACK/NACK Description not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


RlcMacResult IeAckNackDescription::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeAckNackDescription::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;

    // Extract ACK/NACK Description bits from the input bit stream.
    //   < FINAL_ACK_INDICATION : bit (1) >
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    finalAckIndication = (FINAL_ACK_INDICATION)val;
    
    //   < STARTING_SEQUENCE_NUMBER : bit (7) >
    result = dataStream.ExtractBits8(startingSeqNum, 7);
    RLC_MAC_RESULT_CHECK(result);
    
    //   < RECEIVED_BLOCK_BITMAP : bit (64) > ;
    result = dataStream.ExtractBytesN(recBlockBitmap, RRB_SIZE);
    RLC_MAC_RESULT_CHECK(result);
    
    isValid = TRUE;
    DBG_LEAVE();
    return (result);
}


void IeAckNackDescription::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tACK/NACK Description\n");
        outObj->Trace("\t\tFinal ACK Indication ---> %s\n", finalAckNames[finalAckIndication]);
        outObj->Trace("\t\tStarting Sequence Number ---> %d\n", startingSeqNum);
        outObj->Trace("\t\tReceive Block BitMap\n");
        outObj->Trace("\t\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
            recBlockBitmap[0], recBlockBitmap[1], recBlockBitmap[2], recBlockBitmap[3],
            recBlockBitmap[4], recBlockBitmap[5], recBlockBitmap[6], recBlockBitmap[7]);
    }
}