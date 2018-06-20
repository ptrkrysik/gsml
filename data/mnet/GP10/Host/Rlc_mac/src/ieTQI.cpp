// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTQI.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieTQI.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeTQI
//
// Description:
//    TFI IE -- GSM04.60 12.17
//
// *******************************************************************

RlcMacResult IeTQI::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeTQI::EncodeIe", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack TQI bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits16(tqi, 16);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeTQI::EncodeIe TQI not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeTQI::DisplayDetails(DbgOutput *outObj)
{
   if (isValid)
   {
      outObj->Trace("\tTQI ---> %#x\n", tqi);
   }
}