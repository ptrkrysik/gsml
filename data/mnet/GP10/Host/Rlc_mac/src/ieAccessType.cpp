// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieAccessType.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieAccessType.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char accessTypeNames[][64] = 
{
    "Two Phase Access Request",
    "Page Response",
    "Cell Update",
    "Mobility Management procedure"
};

// *******************************************************************
// class IeAccessType
//
// Description:
//    ACCESS_TYPE IE -- GSM04.60 11.2.16
// *******************************************************************

RlcMacResult IeAccessType::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeAccessType::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;

    // Extract ACESS_TYPE bits from the input bit stream.
    result = dataStream.ExtractBits8(val, 2);
    RLC_MAC_RESULT_CHECK(result);
    accessType = (ACCESS_TYPE)val;
    isValid = TRUE;
    
    DBG_LEAVE();
    return (result);
}


void IeAccessType::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tACCESS_TYPE ---> %s\n", accessTypeNames[accessType]);
    }
}