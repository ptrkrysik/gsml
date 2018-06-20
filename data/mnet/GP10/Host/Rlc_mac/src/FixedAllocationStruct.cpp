// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : FixedAllocationStruct.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "FixedAllocationStruct.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char finalAllocNames[][64] =
{
    "Not the last allocation",
    "Last allocation"
};

char halfDupNames[][64] =
{
    "Not half duplex",
    "Half duplex"
};


// *******************************************************************
// class FixedAllocationStruct
//
// Description:
//    Dynamic Allocation Struct -- GSM04.60 11.2.29
//
// <Fixed Allocation struct > ::= 
//   { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//   < FINAL_ALLOCATION : bit (1) >
//   < DOWNLINK_CONTROL_TIMESLOT: bit (3) >
//   { 0 | 1 < P0 : bit (4) >
//           < BTS_PWR_CTRL_MODE : bit (1) > 
//           < PR_MODE : bit (1) > }
//   { 0    < TIMESLOT_ALLOCATION : bit (8) >
//    | 1   < Power Control Parameters : < Power Control Parameters IE > > }
//   < HALF_DUPLEX_MODE : bit (1) >
//   < TBF Starting Time : < Starting Frame Number Description IE > > 
//   { 0 { 0            -- with length of Allocation Bitmap
//           < BLOCKS_OR_BLOCK_PERIODS : bit (1) >
//           < ALLOCATION_BITMAP_LENGTH : bit (7) > 
//           < ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
//        | 1           -- without length of Allocation Bitmap (fills remainder of the message)
//           < ALLOCATION_BITMAP : bit ** > } 
//        ! < Message escape : 1 bit (*) = <no string> > } ;
// *******************************************************************


RlcMacResult FixedAllocationStruct::EncodeFixedAllocationStruct(BitStreamOut &dataStream)
{
    DBG_FUNC("FixedAllocationStruct::EncodeFixedAllocationStruct", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Fixed Allocation Struct bits into the output bit stream.
    if (isValid) 
    {
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
        
        // Encode Final Allocation bit
        //   < FINAL_ALLOCATION : bit (1) >
        dataStream.InsertBits8((unsigned char)finalAllocation, 1);
        
        // Encode Downlink Control Timeslot
        //   < DOWNLINK_CONTROL_TIMESLOT: bit (3) >
        dataStream.InsertBits8(controlTimeslot, 3);
        
        // Encode PO, BTS_PWR_CTRL_MODE and PR_MODE if valid
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
        
        // Encode Timslot Allocation or Power Control Parameters
        //   { 0 | 1 < RLC_DATA_BLOCKS_GRANTED : bit (8) > }
        if (timeslotAllocation.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = timeslotAllocation.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else if (powerControlParameters.IsValid())
        {
            dataStream.InsertBits8(0, 1);
            result = powerControlParameters.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            result = RLC_MAC_INVALID_IE;
            DBG_ERROR("FixedAllocationStruct::EncodeFixedAllocationStruct Timeslot "
                "Allocation or Power Control Params must be valid\n");
        }
        
        // Encode Half Duplex Mode bit
        //   < HALF_DUPLEX_MODE : bit (1) >
        dataStream.InsertBits8((unsigned char)halfDuplexMode, 1);
             
        // Encode TBF Starting Time
        //   < TBF Starting Time : < Starting Frame Number Description IE > > 
        if (startTime.IsValid())
        {
            result = startTime.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            result = RLC_MAC_INVALID_IE;
            DBG_ERROR("FixedAllocationStruct::EncodeFixedAllocationStruct TBF Start "
                "Time must be valid\n");
        }
        
        // Encode the Timeslot Allocation
        // NOTE: Only supporting length version!
        //   { 0 { 0            -- with length of Allocation Bitmap
        //           < BLOCKS_OR_BLOCK_PERIODS : bit (1) >
        //           < ALLOCATION_BITMAP_LENGTH : bit (7) > 
        //           < ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
        //        | 1           -- without length of Allocation Bitmap (fills remainder of the message)
        //           < ALLOCATION_BITMAP : bit ** > } 
        //        ! < Message escape : 1 bit (*) = <no string> > } ;
        dataStream.InsertBits8(0, 2);
        dataStream.InsertBits8((unsigned char)allocationBitmapType, 1);
        if (allocationBitMap.IsValid())
        {   
            result = allocationBitMap.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        }
        else
        {
            result = RLC_MAC_INVALID_IE;
            DBG_ERROR("FixedAllocationStruct::EncodeFixedAllocationStruct Allocation "
                "BitMap not valid\n");
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("FixedAllocationStruct::EncodeFixedAllocationStruct Dynamic "
            "Allocation Struct not valid\n");
    }
    
    DBG_LEAVE();
    return (result);
}


void FixedAllocationStruct::DisplayFixedAllocationStruct(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tFixed Allocation Struct\n");
        
        // Print out Uplink TFI Assignment
        //   { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
        if (uplinkTFI.IsValid())
        {
            uplinkTFI.DisplayDetails(outObj);
        }
        
        // Print out Final Allocation bit
        outObj->Trace("\tFinal Allocation ---> %s\n", finalAllocNames[finalAllocation]);
        
        // Print out Downlink Control Timeslot
        outObj->Trace("\tDownlink Control Timeslot ---> %d\n", controlTimeslot);
        
        // Print out PO, BTS_PWR_CTRL_MODE and PR_MODE if valid
        if (powerControl.isValid)
        {
            powerControl.po.DisplayDetails(outObj);
            powerControl.btsPowerCtlMode.DisplayDetails(outObj);
            powerControl.prMode.DisplayDetails(outObj);
        }
        
        // Print out Timslot Allocation
        if (timeslotAllocation.IsValid())
        {
            timeslotAllocation.DisplayDetails(outObj);
        }
        
        // Print out Power Control Parameters
        if (powerControlParameters.IsValid())
        {
            powerControlParameters.DisplayDetails(outObj);
        }
        
        // Print out Half Duplex Mode bit
        outObj->Trace("\tHalf Duplex Mode ---> %s\n", halfDupNames[halfDuplexMode]);
             
        // Print out TBF Starting Time
        if (startTime.IsValid())
        {
            startTime.DisplayDetails(outObj);
        }
        
        // Print out the Timeslot Allocation
        if (allocationBitMap.IsValid())
        {   
            allocationBitMap.DisplayDetails(outObj);
        }
    }
}