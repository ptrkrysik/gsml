// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMACMode.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMACMode.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char macModeNames[][64] =
{
    "Dynamic Allocation",
    "Extended Dynamic Allocation",
    "Fixed Allocation, not half duplex mode",
    "Fixed Allocation, half duplex mode"
};

// *******************************************************************
// class IeMacMode
//
// Description:
//    Mac Mode IE -- GSM04.60 11.2.7
// *******************************************************************

RlcMacResult IeMacMode::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeMacMode::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack MAC mode bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8((unsigned char)macMode, 2);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeMacMode::EncodeIe failure %x\n", result);
    }
    
    DBG_LEAVE();
    return (result);
}


void IeMacMode::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMAC Mode ---> %s\n", macModeNames[macMode]);
    }
}