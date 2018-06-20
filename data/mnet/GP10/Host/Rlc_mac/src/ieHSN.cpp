// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieHSN.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieHSN.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeHSN
//
// Description:
//    HSN IE -- GSM04.60 12.8
// *******************************************************************
RlcMacResult IeHSN::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeHSN::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack HSN bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(hsn, 7);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeHSN::EncodeIe HSN not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeHSN::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tHSN ---> %d\n", hsn);
    }  
}


