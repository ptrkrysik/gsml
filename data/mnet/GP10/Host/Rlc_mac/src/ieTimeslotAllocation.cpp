// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTimeslotAllocation.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieTimeslotAllocation.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


char allocNames[][64] =
{
    "not allocated",
    "allocated"
};

// *******************************************************************
// class IeTimeslotAllocation
//
// Description:
//    RLC Mode IE -- GSM04.60 12.18
// *******************************************************************

RlcMacResult IeTimeslotAllocation::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeTimeslotAllocation::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Timeslot Allocation bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(tsAllocation, 8);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeTimeslotAllocation::EncodeIe TS Allocation not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeTimeslotAllocation::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tTimeslot Allocation\n");
        for (int i = 0; i < 8; i++)
        {
            outObj->Trace("\t\tTimeslot (%d) ---> %s\n", 
                i, allocNames[(tsAllocation >> (7 - i)) & 1]);
        }        
    }
}