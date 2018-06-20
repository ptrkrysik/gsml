// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : NCMeasurementStruct.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "NCMeasurementStruct.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char netCtlOrderNames[][64] = 
{
    "NC0",        
    "NC1",        
    "NC2",        
    "Reserved"
};

char nonDrxPerNames[][64] = 
{
    "0.24 sec",        
    "0.48 sec",        
    "0.72 sec",        
    "0.96 sec",        
    "1.20 sec",        
    "1.44 sec",        
    "1.92 sec"
};

char repPerNames[][64] = 
{
    "0.48 sec",        
    "0.96 sec",        
    "1.92 sec",        
    "3.84 sec",        
    "7.68 sec",        
    "15.36 sec",        
    "30.72 sec",        
    "61.44 sec"
};


RlcMacResult NCMeasParamStruct::EncodeNCMeasParamStruct(BitStreamOut &dataStream)
{
    DBG_FUNC("NCMeasParamStruct::EncodeNCMeasParamStruct", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack NC Measurement bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(netCtlOrder, 2);
        if (periodInfo.isValid)
        {
            dataStream.InsertBits8(1,1);
            dataStream.InsertBits8(periodInfo.nonDrxPeriod, 3);
            dataStream.InsertBits8(periodInfo.reportingPeriodI, 3);
            dataStream.InsertBits8(periodInfo.reportingPeriodT, 3);
        } 
        else
        {
            dataStream.InsertBits8(0,1);
        }        
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("NCMeasParamStruct::DisplayNCMeasParamStruct NC Meas Params not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void NCMeasParamStruct::DisplayNCMeasParamStruct(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tNetwork Control Order ---> %s\n", netCtlOrderNames[netCtlOrder]);
        if (periodInfo.isValid)
        {
            outObj->Trace("\tNon DRX Period ----------> %s\n", 
                nonDrxPerNames[periodInfo.nonDrxPeriod]);
            outObj->Trace("\tReporting Period I ------> %s\n", 
                repPerNames[periodInfo.reportingPeriodI]);
            outObj->Trace("\tReporting Period T ------> %s\n", 
                repPerNames[periodInfo.reportingPeriodT]);
        }
    }
}