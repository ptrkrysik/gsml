// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGlobalTFI.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieGlobalTFI.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char tfiTypeNames[][64] = 
{
   "UPLINK",        
   "DOWNLINK"      
};


// *******************************************************************
// class IeGlobalTFI
//
// Description:
//    Global TFI IE -- GSM04.60 12.10
//
// < Global TFI IE > ::=
//   { 0 < UPLINK_TFI : bit (5) >
//   | 1 < DOWNLINK_TFI : bit (5) > } ;
//
// *******************************************************************

RlcMacResult IeGlobalTFI::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeGlobalTFI::EncodeIe", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Global TFI bits into the output bit stream.
    // < Global TFI IE > ::=
    //   { 0 < UPLINK_TFI : bit (5) >
    //   | 1 < DOWNLINK_TFI : bit (5) > } ;
    if (isValid) 
    {
        dataStream.InsertBits8((unsigned char)tfiType, 1);
        dataStream.InsertBits8(globalTFI, 5);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeGlobalTFI::EncodeIe tfiType not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}

RlcMacResult IeGlobalTFI::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeGlobalTFI::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;

    // Extract Global TFI bits from the input bit stream.
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    tfiType = (RLC_MAC_TFI_TYPE)val;
    
    result = dataStream.ExtractBits8(globalTFI, 5);
    RLC_MAC_RESULT_CHECK(result);
    
    isValid = TRUE;
    DBG_LEAVE();
    return (result);
}


void IeGlobalTFI::DisplayDetails(DbgOutput *outObj)
{
   if (isValid)
   {
      outObj->Trace("\tGlobal TFI ---> %#x, %s\n", globalTFI, tfiTypeNames[tfiType]);
   }
}