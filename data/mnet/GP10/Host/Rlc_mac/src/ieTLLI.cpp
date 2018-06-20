// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTLLI.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieTLLI.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeTLLI
//
// Description:
//    TLLI IE -- GSM04.60 12.16
// *******************************************************************

RlcMacResult IeTLLI::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeTLLI::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack TLLI bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits32(tlli, 32);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeTLLI::EncodeIe tlli not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


RlcMacResult IeTLLI::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeTLLI::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Extract TLLI bits from the input bit stream.
    if ((result = dataStream.ExtractBits32((unsigned long &)tlli, 32)) == RLC_MAC_SUCCESS)
    {
        isValid = TRUE;
    }
    else
    {
        DBG_ERROR("IeTLLI::DecodeIe tlli extraction failed\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeTLLI::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tTLLI ---> %#x\n", tlli);
    }  
}