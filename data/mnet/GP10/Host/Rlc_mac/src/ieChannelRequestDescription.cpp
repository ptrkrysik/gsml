// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChannelRequestDescription.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieChannelRequestDescription.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


char llcPduTypeNames[][64] =
{
    "LLC PDU is SACK or ACK",
    "LLC PDU is not SACK or ACK"
};

// *******************************************************************
// class IeChannelRequestDescription
//
// Description:
//    Channel Request Description IE -- GSM04.60 12.7
//
// < Channel Request Description IE > ::=   
//      < PEAK_THROUGHPUT_CLASS : bit (4) > 
//      < RADIO_PRIORITY : bit (2) >    
//      < RLC_MODE : bit (1) >  
//      < LLC_PDU_TYPE : bit (1) > 
//      < RLC_OCTET_COUNT : bit (16) > ;
// *******************************************************************


RlcMacResult IeChannelRequestDescription::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeChannelRequestDescription::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;

    // Extract the Channel Request Description from the input bit stream.
    result = dataStream.ExtractBits8(peakThroughputClass, 4);
    RLC_MAC_RESULT_CHECK(result);
    
    result = dataStream.ExtractBits8(radioPriority, 2);
    RLC_MAC_RESULT_CHECK(result);
    
    result = rlcMode.DecodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    llcPduType = (LLC_PDU_TYPE)val;
    
    result = dataStream.ExtractBits16(rlcOctetCount, 16);
    RLC_MAC_RESULT_CHECK(result);
    
    isValid = TRUE;       
    DBG_LEAVE();
    return (result);
}


void IeChannelRequestDescription::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tChannel Request Description\n");
        outObj->Trace("\tPeak Thoughput Class ---> %d\n", peakThroughputClass);
        outObj->Trace("\tRadio Priority ---> %d\n", radioPriority);
        rlcMode.DisplayDetails(outObj);
        outObj->Trace("\tLLC PDU Type ---> %s\n", llcPduTypeNames[llcPduType]);
        outObj->Trace("\tRLC Octet Count ---> %d\n", rlcOctetCount);
    }
}