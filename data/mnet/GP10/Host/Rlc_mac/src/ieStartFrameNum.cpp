// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieStartFrameNum.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieStartFrameNum.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeStartFrameNum
//
// Description:
//    Start Frame Number IE -- GSM04.60 12.21
//
// *******************************************************************

RlcMacResult IeStartFrameNum::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeStartFrameNum::EncodeIe", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Start Frame Number bits into the output bit stream.
    if (isValid)
    {
        if (startFrameType == ABSOLUTE_FRAME_NUM)
        {
            dataStream.InsertBits8(0, 1);
            dataStream.InsertBits8(T1, 5);
            dataStream.InsertBits8(T3, 6);
            dataStream.InsertBits8(T2, 5);
        }
        else
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits16(k, 13);
        }   
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeStartFrameNum::EncodeIe Start Frame not valid\n");
    }

    DBG_LEAVE();
    return (result);
}


void IeStartFrameNum::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        if (startFrameType == ABSOLUTE_FRAME_NUM)
        {
            outObj->Trace("\tAbsolute Start Frame Number\n");
            outObj->Trace("\t\tT1 ---> %d\n", T1);
            outObj->Trace("\t\tT2 ---> %d\n", T2);
            outObj->Trace("\t\tT3 ---> %d\n", T3);
        }
        else
        {
            outObj->Trace("\tRelative Start Frame Number\n");
            outObj->Trace("\t\tk ---> %d\n", k);
        }
    }
}