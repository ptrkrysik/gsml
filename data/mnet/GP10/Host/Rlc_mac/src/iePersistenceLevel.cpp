// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePersistenceLevel.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePersistenceLevel.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IePersistenceLevel
//
// Description:
//    Page Mode IE -- GSM04.60 12.14
// *******************************************************************

RlcMacResult IePersistenceLevel::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePersistenceLevel::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack persistence level bits into the output bit stream.
    if (isValid)
    {
        for (int i = 0; i < MAX_NUM_RADIO_PRIORITY; i++)
        {
            dataStream.InsertBits8((unsigned char)persistenceLevel[i], 4); 
        } 
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePersistenceLevel::EncodeIe Persistence level not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void IePersistenceLevel::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        for (int i = 0; i < MAX_NUM_RADIO_PRIORITY; i++)
        {
            outObj->Trace("\tPersisence Level %d ---> %d\n", i, persistenceLevel[i]);
        }
    }
}