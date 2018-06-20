// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : SingleBlockAllocationStruct.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __SINGLEBLOCKALLOCATIONSTRUCT_H__
#define __SINGLEBLOCKALLOCATIONSTRUCT_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "ieStartFrameNum.h"
#include "iePO.h"
#include "iePRMode.h"
#include "ieBtsPowerCtlMode.h"


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

class SingleBlockAllocationStruct {
public:
    
    unsigned char               timeslot;
    struct PowerIndicationStruct
    {
        bool                    isValid;
        unsigned char           alpha;
        unsigned char           gamma;
    } powerIndication;
    struct PowerControlStruct
    {
        bool                    isValid;
        IePO                    po;
        IeBtsPowerCtlMode       btsPowerCtlMode;
        IePRMode                prMode;
    } powerControl;
    IeStartFrameNum             startTime;
    bool                        isValid;
    
    // Default constructor
    SingleBlockAllocationStruct() : isValid(FALSE), startTime() 
    {
        powerIndication.isValid = FALSE;
        powerControl.isValid = FALSE;
    }
    
    RlcMacResult EncodeSingleBlockAllocationStruct(BitStreamOut &dataStream);
    void DisplaySingleBlockAllocationStruct(DbgOutput *outObj);
};

#endif