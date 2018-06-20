// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DynamicAllocationStruct.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "DynamicAllocationStruct.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char allocTypeNames[][64] =
{
    "Dynamic Allocation",
    "Extended Dynamic Allocation"
};

char usfGranularityNames[][64] =
{
    "One RLC/MAC block",
    "Four RLC/MAC blocks"
};


// *******************************************************************
// class DynamicAllocationStruct
//
// Description:
//    Dynamic Allocation Struct -- GSM04.60 11.2.29
//
// <Dynamic Allocation struct > ::= 
//   < Extended Dynamic Allocation : bit (1) >
//   { 0 | 1 < P0 : bit (4) > 
//           < PR_MODE : bit (1) > }
//   < USF_GRANULARITY : bit (1) >
//   { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//   { 0 | 1 < RLC_DATA_BLOCKS_GRANTED : bit (8) > }
//   { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
//   { 0                -- Timeslot Allocation
//       { 0 | 1 < USF_TN0 : bit (3) > }
//       { 0 | 1 < USF_TN1 : bit (3) > }
//       { 0 | 1 < USF_TN2 : bit (3) > }
//       { 0 | 1 < USF_TN3 : bit (3) > }
//       { 0 | 1 < USF_TN4 : bit (3) > }
//       { 0 | 1 < USF_TN5 : bit (3) > }
//       { 0 | 1 < USF_TN6 : bit (3) > }
//       { 0 | 1 < USF_TN7 : bit (3) > }
//   | 1                -- Timeslot Allocation with Power Control Parameters
//       < ALPHA : bit (4) >
//       { 0 | 1    < USF_TN0 : bit (3) >
//               < GAMMA_TN0 : bit (5) > }
//       { 0 | 1 < USF_TN1 : bit (3) >
//               < GAMMA_TN1 : bit (5) > }
//       { 0 | 1 < USF_TN2 : bit (3) >
//               < GAMMA_TN2 : bit (5) > }
//       { 0 | 1 < USF_TN3 : bit (3) >
//               < GAMMA_TN3 : bit (5) > }
//       { 0 | 1 < USF_TN4 : bit (3) >
//               < GAMMA_TN4 : bit (5) > }
//       { 0 | 1 < USF_TN5 : bit (3) >
//               < GAMMA_TN5 : bit (5) > }
//       { 0 | 1 < USF_TN6 : bit (3) >
//               < GAMMA_TN6 : bit (5) > }
//       { 0 | 1 < USF_TN7 : bit (3) >
//               < GAMMA_TN7 : bit (5) > } } ;
// *******************************************************************


RlcMacResult DynamicAllocationStruct::EncodeDynamicAllocationStruct(BitStreamOut &dataStream)
{
    DBG_FUNC("DynamicAllocationStruct::EncodeDynamicAllocationStruct", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Dynamic Allocation Struct bits into the output bit stream.
    if (isValid) 
    {
        // Encode Extended Dynamic Allocation bit
        //   < Extended Dynamic Allocation : bit (1) >
        dataStream.InsertBits8((unsigned char)allocType, 1);
        
        // Encode PO and PR_MODE if valid
        //   { 0 | 1 < P0 : bit (4) > 
        //           < PR_MODE : bit (1) > }
        if ((po.IsValid()) && (prMode.IsValid()))
        {
            dataStream.InsertBits8(1, 1);
            result = po.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
            result = prMode.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
        
        // Encode USF Granularity bit
        //   < USF_GRANULARITY : bit (1) >
        dataStream.InsertBits8((unsigned char)usfGranularity, 1);
        
        // Encode Uplink TFI Assignment
        //   { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
        if (uplinkTFI.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = uplinkTFI.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
        
        // Encode RLC Data Blocks Granted
        //   { 0 | 1 < RLC_DATA_BLOCKS_GRANTED : bit (8) > }
        if (rlcBlocksGranted.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = rlcBlocksGranted.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
        
        // Encode TBF Starting Time
        //   { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
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
        
        // Encode the Timeslot Allocation
        //   { 0                -- Timeslot Allocation
        //       { 0 | 1 < USF_TN0 : bit (3) > }
        //       { 0 | 1 < USF_TN1 : bit (3) > }
        //       { 0 | 1 < USF_TN2 : bit (3) > }
        //       { 0 | 1 < USF_TN3 : bit (3) > }
        //       { 0 | 1 < USF_TN4 : bit (3) > }
        //       { 0 | 1 < USF_TN5 : bit (3) > }
        //       { 0 | 1 < USF_TN6 : bit (3) > }
        //       { 0 | 1 < USF_TN7 : bit (3) > }
        //   | 1                -- Timeslot Allocation with Power Control Parameters
        //       < ALPHA : bit (4) >
        //       { 0 | 1    < USF_TN0 : bit (3) >
        //               < GAMMA_TN0 : bit (5) > }
        //       { 0 | 1 < USF_TN1 : bit (3) >
        //               < GAMMA_TN1 : bit (5) > }
        //       { 0 | 1 < USF_TN2 : bit (3) >
        //               < GAMMA_TN2 : bit (5) > }
        //       { 0 | 1 < USF_TN3 : bit (3) >
        //               < GAMMA_TN3 : bit (5) > }
        //       { 0 | 1 < USF_TN4 : bit (3) >
        //               < GAMMA_TN4 : bit (5) > }
        //       { 0 | 1 < USF_TN5 : bit (3) >
        //               < GAMMA_TN5 : bit (5) > }
        //       { 0 | 1 < USF_TN6 : bit (3) >
        //               < GAMMA_TN6 : bit (5) > }
        //       { 0 | 1 < USF_TN7 : bit (3) >
        //               < GAMMA_TN7 : bit (5) > } } ;
        if (timeslotAllocation.isValid)
        {
            dataStream.InsertBits8(0, 1);
            for (int i = 0; i < MAX_TIMESLOTS; i++)
            {
                if (timeslotAllocation.tsAllocEntry[i].isValid)
                {
                    dataStream.InsertBits8(1, 1);
                    dataStream.InsertBits8(timeslotAllocation.tsAllocEntry[i].usf, 3);
                }
                else
                {
                    dataStream.InsertBits8(0, 1);
                }
            }
        }
        else if (timeslotAllocationWithPower.isValid)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(timeslotAllocationWithPower.alpha, 4);
            for (int i = 0; i < MAX_TIMESLOTS; i++)
            {
                if (timeslotAllocationWithPower.tsAllocWithPowerEntry[i].isValid)
                {
                    dataStream.InsertBits8(1, 1);
                    dataStream.InsertBits8(
                        timeslotAllocationWithPower.tsAllocWithPowerEntry[i].usf, 3);
                    dataStream.InsertBits8(
                        timeslotAllocationWithPower.tsAllocWithPowerEntry[i].gamma, 5);
                }
                else
                {
                    dataStream.InsertBits8(0, 1);
                }
            }
        }
        else
        {
            result = RLC_MAC_INVALID_IE;
            DBG_ERROR("DynamicAllocationStruct::EncodeDynamicAllocationStruct Timeslot "
                "Allocation Struct not valid\n");
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("DynamicAllocationStruct::EncodeDynamicAllocationStruct Dynamic "
            "Allocation Struct not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void DynamicAllocationStruct::DisplayDynamicAllocationStruct(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tDynamic Allocation Struct\n");
        
        // Print out Extended Dynamic Allocation bit
        outObj->Trace("\tExtended Dynamic Allocation bit ---> %s\n", 
            allocTypeNames[allocType]);
        
        // Print out PO and PR_MODE if valid
        if ((po.IsValid()) && (prMode.IsValid()))
        {
            po.DisplayDetails(outObj);
        }
       
        // Print out PR_MODE if valid
        if ((po.IsValid()) && (prMode.IsValid()))
        {
            prMode.DisplayDetails(outObj);
        }
        
        // Print out USF Granularity bit
        outObj->Trace("\tUSF Granularity bit ---> %s\n", 
            usfGranularityNames[usfGranularity]);
        
        // Print out Uplink TFI Assignment
        if (uplinkTFI.IsValid())
        {
            uplinkTFI.DisplayDetails(outObj);
        }
        
        // Print out RLC Data Blocks Granted
        if (rlcBlocksGranted.IsValid())
        {
            rlcBlocksGranted.DisplayDetails(outObj);
        }
        
        // Print out TBF Starting Time
        if (startTime.IsValid())
        {
            startTime.DisplayDetails(outObj);
        }
        
        // Print out the Timeslot Allocation
        if (timeslotAllocation.isValid)
        {
            for (int i = 0; i < MAX_TIMESLOTS; i++)
            {
                if (timeslotAllocation.tsAllocEntry[i].isValid)
                {
                    outObj->Trace("\tUSF TS %d ---> %d\n", i, 
                        timeslotAllocation.tsAllocEntry[i].usf); 
                }
            }
        }
        else if (timeslotAllocationWithPower.isValid)
        {
            outObj->Trace("\tAlpha ---> %f\n", 
                (timeslotAllocationWithPower.alpha > 10) ? 1.0 : 
                (float) (timeslotAllocationWithPower.alpha * 0.1));
            for (int i = 0; i < MAX_TIMESLOTS; i++)
            {
                if (timeslotAllocationWithPower.tsAllocWithPowerEntry[i].isValid)
                {
                    outObj->Trace("\tUSF TS %d ---> %d\n", i, 
                        timeslotAllocationWithPower.tsAllocWithPowerEntry[i].usf);
                    outObj->Trace("\tGamma TS %d ---> %d dB\n", i, 
                        timeslotAllocationWithPower.tsAllocWithPowerEntry[i].gamma<<1) ;
                }
            }
        }
    }
}