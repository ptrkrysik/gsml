// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMANumber.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMANumber.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeMANumber
//
// Description:
//    MA Number IE -- GSM04.60 12.8
// *******************************************************************
RlcMacResult IeMANumber::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeMANumber::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack MA Number bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(maNum, 4);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeMANumber::EncodeIe MANumber not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeMANumber::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMA Number ---> %d\n", maNum);
    }  
}


