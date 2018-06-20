// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieARFCN.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieARFCN.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeARFCN
//
// Description:
//    ARFCN IE -- GSM04.60 11.2.4
// *******************************************************************
RlcMacResult IeARFCN::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeARFCN::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack ARFCN bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits16(arfcn, 10);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeARFCN::EncodeIe ARFCN not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


RlcMacResult IeARFCN::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeARFCN::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Extract ARFCN bits from the input bit stream.
    if ((result = dataStream.ExtractBits16(arfcn, 10)) == RLC_MAC_SUCCESS)
    {
        isValid = TRUE;
    }
    else
    {
        DBG_ERROR("IeARFCN::DecodeIe ARFCN extraction failed\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeARFCN::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tARFCN ---> %d\n", arfcn);
    }  
}


