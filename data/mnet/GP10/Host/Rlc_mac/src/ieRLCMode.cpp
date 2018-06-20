// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieRLCMode.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieRLCMode.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char rlcModeNames[][64] =
{
    "RLC acknowledged mode",
    "RLC unacknowledged mode"
};

// *******************************************************************
// class IeRlcMode
//
// Description:
//    RLC Mode IE -- GSM04.60 11.2.7
// *******************************************************************

RlcMacResult IeRlcMode::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeRlcMode::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack RLC mode bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8((unsigned char)rlcMode, 1);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeRlcMode::EncodeIe RLC Mode not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}

RlcMacResult IeRlcMode::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeRlcMode::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;
    
    // Extract RLC Mode bits from the input bit stream.
    if ((result = dataStream.ExtractBits8(val, 1)) == RLC_MAC_SUCCESS)
    {
        rlcMode = (RLC_MODE)val;
        isValid = TRUE;
    }
    else
    {
        DBG_ERROR("IeRlcMode::DecodeIe tlli extraction failed\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeRlcMode::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tRLC Mode ---> %s\n", rlcModeNames[rlcMode]);
    }
}