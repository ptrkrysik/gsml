// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMABitMap.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMABitMap.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IeMABitMap
//
// Description:
//    MA Bit Map IE -- GSM04.60 12.10a
//
// *******************************************************************

RlcMacResult IeMABitMap::SetMABitMap(unsigned char len, unsigned char *map)
{
    if (len <= (maLength >> 3))
    {
        maLength = len;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < ((len+1+8) >> 3); i++)
    {
        maBitMap[i] = map[i];
    }
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}




RlcMacResult IeMABitMap::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeMABitMap::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack MA Bit Map bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(maLength, 6);
        for (int i = 0; i < ((maLength + 1) >> 3); i++)
        {
            dataStream.InsertBits8(maBitMap[i], 8);
        }
        
        if ((maLength+1) % 8)
        {
            dataStream.InsertBits8(maBitMap[(maLength+1)/8] >>(8-(maLength+1)%8), 
                (maLength+1)%8);
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeMABitMap::EncodeIe MA Bitmap not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeMABitMap::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMA Bit Map\n");
        if (maLength > 0)
        {
            for (int i = 0; i < ((maLength + 1) >> 3); i++)
            {
                outObj->Trace("\t\t%x\n", maBitMap[i]);
            }
        }
    }
}