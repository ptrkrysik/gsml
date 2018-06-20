// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePRMode.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePRMode.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char prModeNames[][64] = 
{
    "PR Mode A : for one addressed MS",
    "PR Mode B : for all MS"
};

// *******************************************************************
// class IePRMode
//
// Description:
//    PR Mode IE -- GSM04.60 11.2.29
// *******************************************************************

RlcMacResult IePRMode::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePRMode::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack PR Mode bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8((unsigned char)prMode, 1);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePRMode::EncodeIe PR Mode not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IePRMode::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tPR Mode ---> %s\n", prModeNames[prMode]);
    }
}