// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePacketTimingAdvance.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePacketTimingAdvance.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IePacketTimingAdvance
//
// Description:
//    Packet Timing Advance IE -- GSM04.60 12.12
//
// < Packet Timing Advance IE > ::=
//  { 0 | 1 < TIMING_ADVANCE_VALUE : bit (6) > }
//  { 0 | 1 < TIMING_ADVANCE_INDEX : bit (4) >
//          < TIMING_ADVANCE_TIMESLOT_NUMBER : bit (3) > } ;
//
// *******************************************************************

RlcMacResult IePacketTimingAdvance::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePacketTimingAdvance::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Timing Advance bits into the output bit stream.
    if (isValid)
    {
        if (isTaValueValid)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(taValue, 6); 
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
    
        if (isTaIndexValid)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(taIndex, 4); 
            dataStream.InsertBits8(tsNumber, 3); 
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        } 
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePacketTimingAdvance::EncodeIe TA not valid %x\n", result);
    }
    
    DBG_LEAVE();
    return (result);
}


void IePacketTimingAdvance::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tPacket Timing Advance\n");
        
        if (isTaValueValid)
        {
            outObj->Trace("\t\tTA Value ---> %d\n", taValue);
        }
        
        if (isTaIndexValid)
        {
            outObj->Trace("\t\tTA Index ---> %d\n", taIndex);
            outObj->Trace("\t\tTimeslot ---> %d\n", tsNumber);
        }
        
    }
}