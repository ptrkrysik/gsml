// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTSC.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieTSC.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeTSC
//
// Description:
//    TSC IE -- GSM04.60 12.8
// *******************************************************************
RlcMacResult IeTSC::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeTSC::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack TSC bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(tsc, 3);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeTSC::EncodeIe TSC not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeTSC::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tTSC ---> %d\n", tsc);
    }  
}


