// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTFI.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieTFI.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeTFI
//
// Description:
//    TFI IE -- GSM04.60 12.15
//
// *******************************************************************

RlcMacResult IeTFI::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeTFI::EncodeIe", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack TFI bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(tfi, 5);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeTFI::EncodeIe TFI not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}

RlcMacResult IeTFI::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeTFI::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Extract TFI bits from the input bit stream.
    if ((result = dataStream.ExtractBits8(tfi, 5)) == RLC_MAC_SUCCESS)
    {
        isValid = TRUE;
    }
    else
    {
        DBG_ERROR("IeTFI::DecodeIe TFI extraction failed\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeTFI::DisplayDetails(DbgOutput *outObj)
{
   if (isValid)
   {
      outObj->Trace("\tTFI ---> %#x\n", tfi);
   }
}