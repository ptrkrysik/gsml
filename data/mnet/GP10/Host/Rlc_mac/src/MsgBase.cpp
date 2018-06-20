// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgBase.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#include "MsgBase.h"
#include "logging\vcmodules.h"

DbgOutput MsgBase::MsgDisplayObj("RLC/MAC Msg", RLC_MAC_MSG);
DbgOutput MsgBase::MsgDisplayDetailObj("RLC/MAC Msg Detail", RLC_MAC_MSG_DETAIL);


void DownlinkMsgBase::DisplayMsgShort(BitStreamOut *dataStream)
{
    MsgDisplayObj.Trace(msgType.DisplayDownlinkMsgType());
    if (dataStream)
    {
        MsgDisplayObj.HexDump(dataStream->GetBitStream(),
            dataStream->GetStreamLen());
    }
}




void UplinkMsgBase::DisplayMsgShort(BitStreamIn *dataStream)
{
    MsgDisplayObj.Trace(msgType.DisplayUplinkMsgType());
    if (dataStream)
    {
        MsgDisplayObj.HexDump(dataStream->GetBitStream(),
            dataStream->GetStreamLen());
    }
}
