// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MeasurementMappingStruct.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MeasurementMappingStruct.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class MeasurementMappingStruct
//
// Description:
//    Measurement Mapping Struct -- GSM04.60 11.2.7
//
// < Measurement Mapping struct > ::=
//   < Measurement Starting Time : < Starting Frame Number Description IE > >
//   < MEASUREMENT_INTERVAL : bit (5) >
//   < MEASUREMENT_BITMAP : bit (8) > ;
// *******************************************************************


RlcMacResult MeasurementMappingStruct::EncodeMeasurementMappingStruct(BitStreamOut &dataStream)
{
    DBG_FUNC("MeasurementMappingStruct::EncodeNCMeasParamStruct", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack NC Measurement bits into the output bit stream.
    if (isValid) 
    {
        result = measStartTime.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        dataStream.InsertBits8(measInterval, 5);
        dataStream.InsertBits8(measBitMap, 8);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("MeasurementMappingStruct::EncodeNCMeasParamStruct Measurement"
            "Starting Time not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void MeasurementMappingStruct::DisplayMeasurementMappingStruct(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMeasurement Mapping Struct\n");
        measStartTime.DisplayDetails(outObj);
        outObj->Trace("\tMEAUSUREMENT_INTERVAL ---> %d\n", measInterval);
        outObj->Trace("\tMEAUSUREMENT_BITMAP ---> %d\n", measBitMap);
    }
}