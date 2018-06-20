// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePageMode.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePageMode.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char pageModeNames[][64] = 
{
    "Normal Paging",
    "Extended Paging",
    "Paging Reorganization",
    "Same as before"
};

// *******************************************************************
// class IePageMode
//
// Description:
//    Page Mode IE -- GSM04.60 12.20
// *******************************************************************

RlcMacResult IePageMode::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePageMode::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Page Mode bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8((unsigned char)pageMode, 2);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePageMode::EncodeIe Page Mode not valid %x\n", result);
    }
    
    DBG_LEAVE();
    return (result);
}


void IePageMode::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tPage Mode ---> %s\n", pageModeNames[pageMode]);
    }
}