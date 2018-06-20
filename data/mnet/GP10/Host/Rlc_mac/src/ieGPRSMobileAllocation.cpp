// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGPRSMobileAllocation.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieGPRSMobileAllocation.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IeGPRSMobileAllocation
//
// Description:
//    GPRS Mobile Allocation IE -- GSM04.60 12.10a
//
// < GPRS Mobile Allocation IE > ::=
//   < HSN : bit (6) >
//   { 0 | 1     < RFL number list : < RFL number list struct > > }
//   { 0 < MA_LENGTH : bit (6) >
//       < MA_BITMAP : bit (val(MA_LENGTH) + 1) >
//   | 1 { 0 | 1    < ARFCN index list : < ARFCN index list struct > > } } ;
//
// < RFL number list struct > ::=
//   < RFL_NUMBER : bit (4) >
//   { 0 | 1 < RFL number list struct > } ;
// < ARFCN index list struct > ::=
//   < ARFCN_INDEX : bit (6) >
//   { 0 | 1 < ARFCN index list struct > } ;
// *******************************************************************


RlcMacResult IeGPRSMobileAllocation::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeGPRSMobileAllocation::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack GPRS Mobile Allocation bits into the output bit stream.
    if (isValid)
    {
        result = hsn.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    
        if (rflNumberList.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = rflNumberList.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }

        if (maBitMap.IsValid())
        {
            dataStream.InsertBits8(0, 1);
            result = maBitMap.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else if (arfcnIndexList.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = arfcnIndexList.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        } 
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeGPRSMobileAllocation::EncodeIe GPRS Mobile Allocation not valid\n");
    }
       
    DBG_LEAVE();
    return (result);
}


void IeGPRSMobileAllocation::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tGPRS Mobile Allocation\n");
        if (rflNumberList.IsValid())
        {
            rflNumberList.DisplayDetails(outObj);
        }
        
        if (maBitMap.IsValid())
        {
            maBitMap.DisplayDetails(outObj);
        }
        else if (arfcnIndexList.IsValid())
        {
            arfcnIndexList.DisplayDetails(outObj);
        }
    }
}