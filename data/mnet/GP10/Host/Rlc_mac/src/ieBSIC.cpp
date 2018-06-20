// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieBSIC.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieBSIC.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeBSIC
//
// Description:
//    BSIC IE -- GSM04.60 11.2.4
// *******************************************************************

RlcMacResult IeBSIC::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeBSIC::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack BSIC bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(bsic, 6);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeBSIC::EncodeIe BSIC not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


RlcMacResult IeBSIC::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeBSIC::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Extract BSIC bits from the input bit stream.
    if ((result = dataStream.ExtractBits8(bsic, 6)) == RLC_MAC_SUCCESS)
    {
        isValid = TRUE;
    }
    else
    {
        DBG_ERROR("IeBSIC::DecodeIe BSIC extraction failed\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeBSIC::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tBSIC ---> %o %o\n", bsic>>3, bsic);
    }  
}



