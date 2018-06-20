// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieBtsPowerCtlMode.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieBtsPowerCtlMode.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char pCtlModeNames[][64] = 
{
    "Mode A",
    "Mode B"
};

// *******************************************************************
// class IeBtsPowerCtlMode
//
// Description:
//    Bts Power Control Mode IE -- GSM04.60 11.2.29
// *******************************************************************

RlcMacResult IeBtsPowerCtlMode::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeBtsPowerCtlMode::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Bts Power Control Mode bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8((unsigned char)pCtlMode, 1);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeBtsPowerCtlMode::EncodeIe power control mode invalid failure\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeBtsPowerCtlMode::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tBts Power Control Mode ---> %s\n", pCtlModeNames[pCtlMode]);
    }
}