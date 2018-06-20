// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChannelCodingCommand.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieChannelCodingCommand.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char codingNames[][64] = 
{
    "CS-1",
    "CS-2",
    "CS-3",
    "CS-4"
};

// *******************************************************************
// class IeChannelCodingCommand
//
// Description:
//    PR Mode IE -- GSM04.60 11.2.29
// *******************************************************************

RlcMacResult IeChannelCodingCommand::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeChannelCodingCommand::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Channel Coding Command bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8((unsigned char)coding, 2);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeChannelCodingCommand::EncodeIe Channel Coding Command not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IeChannelCodingCommand::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tCHANNEL CODING COMMAND ---> %s\n", codingNames[coding]);
    }
}