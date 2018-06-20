// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : NCMeasurementStruct.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __NCMEASUREMENTSTRUCT_H__
#define __NCMEASUREMENTSTRUCT_H__

#include "logging\vclogging.h"
#include "bitstream.h"


// *******************************************************************
// class NCMeasurementStruct
//
// Description:
//    NC Measurement Struct -- GSM04.60 11.2.4
//
// < NC Measurement Parameters struct > ::=
//   < NETWORK_CONTROL_ORDER : bit (2) >
//   { 0 | 1 < NC_NON_DRX_PERIOD : bit (3) >
//           < NC_REPORTING_PERIOD_I : bit (3) >
//           < NC_REPORTING_PERIOD_T : bit (3) > }
//   { 0 | 1 < NC_FREQUENCY_LIST : NC Frequency list struct > } ;
// *******************************************************************

class NCMeasParamStruct {
public:
    
    typedef enum
    {
        NC0,
        NC1,
        NC2,
        Reserved
    } NETWORK_CONTROL_ORDER;
    
    typedef enum
    {
        NO_NON_DRX_MODE,
        NON_DRX_PERIOD_0_24_SECONDS,
        NON_DRX_PERIOD_0_48_SECONDS,
        NON_DRX_PERIOD_0_72_SECONDS,
        NON_DRX_PERIOD_0_96_SECONDS,
        NON_DRX_PERIOD_1_20_SECONDS,
        NON_DRX_PERIOD_1_44_SECONDS,
        NON_DRX_PERIOD_1_92_SECONDS
    } NC_NON_DRX_PERIOD;
    
    typedef enum
    {
        REPORTING_PER_0_48_SECONDS,
        REPORTING_PER_0_96_SECONDS,
        REPORTING_PER_1_96_SECONDS,
        REPORTING_PER_3_84_SECONDS,
        REPORTING_PER_7_68_SECONDS,
        REPORTING_PER_15_36_SECONDS,
        REPORTING_PER_30_72_SECONDS,
        REPORTING_PER_61_44_SECONDS
    } NC_REPORTING_PERIOD;
    
    NETWORK_CONTROL_ORDER       netCtlOrder;
    struct PeriodInfoStruct
    {
        NC_NON_DRX_PERIOD           nonDrxPeriod;
        NC_REPORTING_PERIOD         reportingPeriodI;
        NC_REPORTING_PERIOD         reportingPeriodT;
        bool                        isValid;
    } periodInfo;
    bool                        isValid;
    
    // Default constructor
    NCMeasParamStruct() : isValid(FALSE), netCtlOrder(NC0) 
    {
        bzero((char *)&periodInfo, sizeof(periodInfo));
    }
    
    RlcMacResult EncodeNCMeasParamStruct(BitStreamOut &dataStream);
    void DisplayNCMeasParamStruct(DbgOutput *outObj);
};

#endif