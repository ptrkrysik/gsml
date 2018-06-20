// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePTMSI.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePTMSI.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IePTMSI
//
// Description:
//    PTMSI IE -- GSM03.03
// *******************************************************************

RlcMacResult IePTMSI::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePTMSI::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack PTMSI bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits32(ptmsi, 32);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePTMSI::EncodeIe ptmsi not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IePTMSI::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tPTMSI ---> %#x\n", ptmsi);
    }  
}