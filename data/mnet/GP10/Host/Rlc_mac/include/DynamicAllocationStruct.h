// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DynamicAllocationStruct.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __DYNAMICALLOCATIONSTRUCT_H__
#define __DYNAMICALLOCATIONSTRUCT_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "RlcMacCommon.h"
#include "ieStartFrameNum.h"
#include "ieRLCBlocksGranted.h"
#include "iePO.h"
#include "iePRMode.h"
#include "ieTFI.h"


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

typedef struct
{
	bool	isValid;
	unsigned char usf;
} TimeslotAllocationEntry;

typedef struct
{
	bool	isValid;
    unsigned char usf;
	unsigned char gamma;
} TimeslotAllocationWithPowerEntry;

class DynamicAllocationStruct {
public:
    
    typedef enum
    {
        DYNAMIC_ALLOCATION,
        EXTENDED_DYNAMIC_ALLOCATION,
    } ALLOCATION_TYPE;
    
    typedef enum
    {
        ONE_RLC_MAC_BLOCK,
        FOUR_RLC_MAC_BLOCKS
    } USF_GRANULARITY;
    
    ALLOCATION_TYPE             allocType;
    IePO                        po;
    IePRMode                    prMode;
    USF_GRANULARITY             usfGranularity;
    IeRLCBlocksGranted          rlcBlocksGranted;
    IeTFI                       uplinkTFI;
    IeStartFrameNum             startTime;
    struct TimeslotAllocationStruct
    {
        bool                    isValid;
        TimeslotAllocationEntry tsAllocEntry[MAX_TIMESLOTS];
    } timeslotAllocation;
    struct TimeslotAllocationWithPowerStruct
    {
        bool                    isValid;
        unsigned char           alpha;
        TimeslotAllocationWithPowerEntry tsAllocWithPowerEntry[MAX_TIMESLOTS];
    } timeslotAllocationWithPower;
    bool                        isValid;
    
    // Default constructor
    DynamicAllocationStruct() : isValid(FALSE), po(), prMode(), uplinkTFI(),
        startTime(), rlcBlocksGranted(), allocType(EXTENDED_DYNAMIC_ALLOCATION),
        usfGranularity(ONE_RLC_MAC_BLOCK) 
    {
        timeslotAllocation.isValid = FALSE;
        timeslotAllocationWithPower.isValid = FALSE;
    }
    
    RlcMacResult EncodeDynamicAllocationStruct(BitStreamOut &dataStream);
    void DisplayDynamicAllocationStruct(DbgOutput *outObj);
};

#endif