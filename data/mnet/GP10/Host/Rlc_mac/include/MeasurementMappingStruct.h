// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MeasurementMappingStruct.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MEASUREMENTMAPPINGSTRUCT_H__
#define __MEASUREMENTMAPPINGSTRUCT_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "ieStartFrameNum.h"


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

class MeasurementMappingStruct {
public:
    
    IeStartFrameNum               measStartTime;
    unsigned char               measInterval;
    unsigned char               measBitMap;
    bool                        isValid;
    
    // Default constructor
    MeasurementMappingStruct() : isValid(FALSE), measStartTime() {}
    
    RlcMacResult EncodeMeasurementMappingStruct(BitStreamOut &dataStream);
    void DisplayMeasurementMappingStruct(DbgOutput *outObj);
};

#endif