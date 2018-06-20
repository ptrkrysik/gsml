// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTMSI.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieTMSI.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeTMSI
//
// Description:
//    TMSI IE -- GSM03.03
// *******************************************************************

RlcMacResult IeTMSI::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeTMSI::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack TMSI bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits32(tmsi, 32);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeTMSI::EncodeIe tmsi not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeTMSI::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tTMSI ---> %#x\n", tmsi);
    }  
}