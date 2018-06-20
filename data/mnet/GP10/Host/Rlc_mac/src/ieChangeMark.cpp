// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChangeMark.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieChangeMark.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeChangeMark
//
// Description:
//    Change Mark IE -- GSM04.60 12.8
// *******************************************************************
RlcMacResult IeChangeMark::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeChangeMark::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Pack Change Mark bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(changeMark, 2);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeChangeMark::EncodeIe ChangeMark failure %x\n", result);
    }
    
    DBG_LEAVE();
    return (result);
}


RlcMacResult IeChangeMark::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeChangeMark::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Extract CHANGE_MARK bits from the input bit stream.
    result = dataStream.ExtractBits8(changeMark, 2);
    RLC_MAC_RESULT_CHECK(result);
    isValid = TRUE;
    
    DBG_LEAVE();
    return (result);
}


void IeChangeMark::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tChange Mark ---> %d\n", changeMark);
    }  
}


