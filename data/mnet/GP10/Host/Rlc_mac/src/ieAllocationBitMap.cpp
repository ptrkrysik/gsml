// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieAllocationBitMap.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieAllocationBitMap.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IeAllocationBitMap
//
// Description:
//    Allocation Bit Map IE -- GSM04.60 12.4
//
// *******************************************************************

RlcMacResult IeAllocationBitMap::SetAllocationBitMap(unsigned char len, unsigned char *map)
{
    if (len <= (MAX_ALLOCATION_BITMAP_LEN >> 3))
    {
        allocLength = len;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < len++; i++)
    {
        allocBitMap[i] = map[i];
    }
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}




RlcMacResult IeAllocationBitMap::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeAllocationBitMap::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Allocation Bit Map bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8((allocLength<<3), 7);
        for (int i = 0; i < allocLength; i++)
        {
            dataStream.InsertBits8(allocBitMap[i], 8);
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeAllocationBitMap::EncodeIe Allocation Bitmap not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeAllocationBitMap::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tAllocation Bit Map\n");
        if (allocLength > 0)
        {
            for (int i = 0; i < allocLength; i++)
            {
                outObj->Trace("\t\t%x\n", allocBitMap[i]);
            }
        }
    }
}