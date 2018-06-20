// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieRLCBlocksGranted.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieRLCBlocksGranted.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeRLCBlocksGranted
//
// Description:
//    RLC Blocks Granted IE -- GSM04.60 11.2.29
//
// *******************************************************************

RlcMacResult IeRLCBlocksGranted::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeRLCBlocksGranted::EncodeIe", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack RLC Blocks Granted bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(rlcBlocksGranted, 8);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeRLCBlocksGranted::EncodeIe RLC Blocks Granted not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeRLCBlocksGranted::DisplayDetails(DbgOutput *outObj)
{
   if (isValid)
   {
      outObj->Trace("\tRLC Blocks Granted ---> %#x\n", rlcBlocksGranted);
   }
}