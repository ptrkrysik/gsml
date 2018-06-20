// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChannelNeeded.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieChannelNeeded.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char chanNeededNames[][64] =
{
    "Any Channel",
    "SDCCH",
    "TCH/F (Full rate)",
    "TCH/H or TCH/F (Dual rate)"
};

// *******************************************************************
// class IeChannelNeeded
//
// Description:
//    Channel Needed IE -- GSM04.60 11.2.10
// *******************************************************************

RlcMacResult IeChannelNeeded::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeChannelNeeded::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Channel Needed bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8((unsigned char)chanNeeded, 2);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeChannelNeeded::EncodeIe Channel Needed not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeChannelNeeded::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tChannelNeeded ---> %s\n", chanNeededNames[chanNeeded]);
    }
}