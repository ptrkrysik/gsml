// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieNLN.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieNLN.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeNLN
//
// Description:
//    NLN IE -- GSM04.60 11.2.10
//              GSM04.08 10.5.2.23
// *******************************************************************

RlcMacResult IeNLN::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeNLN::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack NLN bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(nln, 2);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeNLN::EncodeIe NLN not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeNLN::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tNLN ---> %d\n", nln);
    }
}