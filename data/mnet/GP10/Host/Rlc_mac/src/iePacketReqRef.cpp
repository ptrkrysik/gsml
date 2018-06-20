// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePacketReqRef.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePacketReqRef.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IePacketReqRef
//
// Description:
//    Packet Request Ref IE -- GSM04.60 12.11
//                             GSM04.08 10.5.2.38
//
// < Packet Request Reference IE > ::=
//	 < RANDOM_ACCESS_INFORMATION value : bit (11) >
//	 < FRAME_NUMBER : bit (16) > ;
//
// *******************************************************************

RlcMacResult IePacketReqRef::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePacketReqRef::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Packet Request Reference bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits16(randAccInfo, 11);
        dataStream.InsertBits8(T1, 5);
        dataStream.InsertBits8(T3, 6);
        dataStream.InsertBits8(T2, 5);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePacketReqRef::EncodeIe Packet Req Ref failure\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IePacketReqRef::DisplayDetails(DbgOutput *outObj)
{
   if (isValid)
   {
      outObj->Trace("\tPacket Request Ref\n"
                    "\t\tRandom Access Info --> %#x\n"
                    "\t\tFrame Number T1 -----> %#x\n"
                    "\t\tFrame Number T2 -----> %#x\n"
                    "\t\tFrame Number T3 -----> %#x\n",
                    randAccInfo, T1, T2, T3);
   }
}