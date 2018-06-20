// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGlobalPacketTimingAdvance.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieGlobalPacketTimingAdvance.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IeGlobalPacketTimingAdvance
//
// Description:
//    Timing Advance IE -- GSM04.60 12.12a
//
// < Global Packet Timing Advance IE > ::=
//          { 0 | 1 < TIMING_ADVANCE_VALUE : bit (6) > }
//          { 0 | 1 < UPLINK_TIMING_ADVANCE_INDEX : bit (4) >
//                  < UPLINK_TIMING_ADVANCE_TIMESLOT_NUMBER : bit (3) > }
//          { 0 | 1 < DOWNLINK_TIMING_ADVANCE_INDEX : bit (4) >
//                  < DOWNLINK_TIMING_ADVANCE_TIMESLOT_NUMBER : bit (3) > }
//
// *******************************************************************

RlcMacResult IeGlobalPacketTimingAdvance::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeGlobalPacketTimingAdvance::EncodeIe", RLC_MAC);
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
    
        if (isUplinkTaIndexValid)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(uplinkTaIndex, 4); 
            dataStream.InsertBits8(uplinkTsNumber, 3);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        } 
         
        if (isDownlinkTaIndexValid)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(downlinkTaIndex, 4); 
            dataStream.InsertBits8(downlinkTsNumber, 3);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }  
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeGlobalPacketTimingAdvance::EncodeIe Global TA invalid failure\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeGlobalPacketTimingAdvance::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tGlobal Packet Timing Advance\n");
        
        if (isTaValueValid)
        {
            outObj->Trace("\t\tTA Value ---> %d\n", taValue);
        }
        
        if (isUplinkTaIndexValid)
        {
            outObj->Trace("\t\tUplink TA Index ---> %d\n", uplinkTaIndex);
            outObj->Trace("\t\tUplink Timeslot ---> %d\n", uplinkTsNumber);
        }
        
        if (isDownlinkTaIndexValid)
        {
            outObj->Trace("\t\tDownlink TA Index ---> %d\n", downlinkTaIndex);
            outObj->Trace("\t\tDownlink Timeslot ---> %d\n", downlinkTsNumber);
        }
    }
}