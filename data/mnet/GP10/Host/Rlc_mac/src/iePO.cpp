// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePO.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePO.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IePO
//
// Description:
//    PO IE -- GSM04.60 11.2.29
// *******************************************************************

RlcMacResult IePO::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePO::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack PO bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(po, 4);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePO::EncodeIe PO not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IePO::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tPO ---> %d dB\n", po*2);
    }
}