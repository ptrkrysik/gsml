// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : SingleBlockAllocationStruct.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "SingleBlockAllocationStruct.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class SingleBlockAllocationStruct
//
// Description:
//    Single Block Allocation Struct -- GSM04.60 11.2.29
//
// <Single Block Allocation struct > ::=
//   < TIMESLOT_NUMBER : bit (3) > 
//   { 0 | 1 < ALPHA : bit (4) >
//   < GAMMA_TN : bit (5) >}
//   { 0 | 1 < P0 : bit (4) >
//           < BTS_PWR_CTRL_MODE : bit (1) > 
//           < PR_MODE : bit (1) > }
//   < TBF Starting Time : < Starting Frame Number Description IE > > ;
// *******************************************************************


RlcMacResult SingleBlockAllocationStruct::EncodeSingleBlockAllocationStruct(BitStreamOut &dataStream)
{
    DBG_FUNC("SingleBlockAllocationStruct::EncodeNCMeasParamStruct", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Single Block Allocation Struct bits into the output bit stream.
    if (isValid) 
    {
        // Encode Timeslot Number Allocation bit
        //   < TIMESLOT_NUMBER : bit (3) > 
        dataStream.InsertBits8(timeslot, 3);
        
        // Encode ALPHA and GAMMA if valid
        //   { 0 | 1 < ALPHA : bit (4) >
        //   < GAMMA_TN : bit (5) >}
        if (powerIndication.isValid)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(powerIndication.alpha, 4);
            dataStream.InsertBits8(powerIndication.gamma, 5);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
        
        // Encode PO, BTS Power Control Mode and PR Mode
        //   { 0 | 1 < P0 : bit (4) >
        //           < BTS_PWR_CTRL_MODE : bit (1) > 
        //           < PR_MODE : bit (1) > }
        if (powerControl.isValid)
        {
            dataStream.InsertBits8(1, 1);
            result = powerControl.po.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
            result = powerControl.btsPowerCtlMode.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
            result = powerControl.prMode.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
        
        // Encode TBF Starting Time
        //   < TBF Starting Time : < Starting Frame Number Description IE > > ;
        if (startTime.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = startTime.EncodeIe(dataStream);
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
        DBG_ERROR("SingleBlockAllocationStruct::EncodeSingleBlockAllocationStruct Dynamic "
            "Allocation Struct not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void SingleBlockAllocationStruct::DisplaySingleBlockAllocationStruct(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tSingle Block Allocation Struct\n");
        
        // Print out Timeslot Number Allocation bit
        outObj->Trace("\tTimeslot ---> %d\n", timeslot);
        
        // Print out ALPHA and GAMMA if valid
        if (powerIndication.isValid)
        {
            outObj->Trace("\tAlpha ---> %f\n", 
                (powerIndication.alpha > 10) ? 1.0 : (float) (powerIndication.alpha * 0.1));
            outObj->Trace("\tGamma ---> %d dB\n", powerIndication.gamma<<1) ;
        }
        
        // Print out PO, BTS Power Control Mode and PR Mode
        if (powerControl.isValid)
        {
            powerControl.po.DisplayDetails(outObj);
            powerControl.btsPowerCtlMode.DisplayDetails(outObj);
            powerControl.prMode.DisplayDetails(outObj);
        }
        
        // Print out TBF Starting Time
        if (startTime.IsValid())
        {
            startTime.DisplayDetails(outObj);
        }
    }
}