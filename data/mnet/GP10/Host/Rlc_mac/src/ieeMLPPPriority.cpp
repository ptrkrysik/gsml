// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieeMLPPPriority.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieeMLPPPriority.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char eMLPPPriNames[][64] =
{
    "no priority applied",
    "call priority level 4",
    "call priority level 3",
    "call priority level 2",
    "call priority level 1",
    "call priority level 0",
    "call priority level A",
    "call priority level B"
};

// *******************************************************************
// class IeeMLPPPriority
//
// Description:
//    eMLPP Priority IE -- GSM04.60 11.2.10
//                         GSM04.08 10.5.2.24
// *******************************************************************

RlcMacResult IeeMLPPPriority::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeeMLPPPriority::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack eMLPP Priority bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8((unsigned char)emlppPriority, 3);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeeMLPPPriority::EncodeIe eMLPP Priority not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeeMLPPPriority::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\teMLPPPriority ---> %s\n", eMLPPPriNames[emlppPriority]);
    }
}