// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMAIO.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMAIO.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeMAIO
//
// Description:
//    MAIO IE -- GSM04.60 12.8
// *******************************************************************
RlcMacResult IeMAIO::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeMAIO::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack MAIO bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(maio, 7);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeMAIO::EncodeIe MAIO not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeMAIO::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMAIO ---> %d\n", maio);
    }  
}


