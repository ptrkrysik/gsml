// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : FixedAllocationStruct.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __FIXEDALLOCATIONSTRUCT_H__
#define __FIXEDALLOCATIONSTRUCT_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "RlcMacCommon.h"
#include "ieStartFrameNum.h"
#include "ieRLCBlocksGranted.h"
#include "iePO.h"
#include "iePRMode.h"
#include "ieTFI.h"
#include "ieAllocationBitMap.h"
#include "iePowerControlParameters.h"
#include "ieTimeslotAllocation.h"
#include "ieBtsPowerCtlMode.h"


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


class FixedAllocationStruct {
public:
    
    typedef enum
    {
        NOT_LAST_ALLOCATION,
        LAST_ALLOCATION
    } FINAL_ALLOCATION;
    
    typedef enum
    {
        NOT_HALF_DUPLEX,
        HALF_DUPLEX
    } HALF_DUPLEX_MODE;
    
    typedef enum
    {
        BLOCKS,
        BLOCK_PERIODS
    } BLOCKS_OR_BLOCK_PERIODS;
    
    IeTFI                       uplinkTFI;
    FINAL_ALLOCATION            finalAllocation;
    unsigned char               controlTimeslot;
    struct PowerControlStruct
    {
        bool                    isValid;
        IePO                    po;
        IeBtsPowerCtlMode       btsPowerCtlMode;
        IePRMode                prMode;
    } powerControl;
    IeTimeslotAllocation        timeslotAllocation;
    IePowerControlParameters    powerControlParameters;
    HALF_DUPLEX_MODE            halfDuplexMode;
    IeStartFrameNum             startTime;
    BLOCKS_OR_BLOCK_PERIODS     allocationBitmapType;
    IeAllocationBitMap          allocationBitMap;
    bool                        isValid;
    
    // Default constructor
    FixedAllocationStruct() : isValid(FALSE), uplinkTFI(), 
        finalAllocation(NOT_LAST_ALLOCATION), controlTimeslot(0), 
        timeslotAllocation(), powerControlParameters(), halfDuplexMode(HALF_DUPLEX),
        startTime(), allocationBitmapType(BLOCKS), allocationBitMap()
    {
        powerControl.isValid = FALSE;
    }
    
    RlcMacResult EncodeFixedAllocationStruct(BitStreamOut &dataStream);
    void DisplayFixedAllocationStruct(DbgOutput *outObj);
};

#endif