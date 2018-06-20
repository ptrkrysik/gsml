// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : Tbf.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __TBF_H__
#define __TBF_H__

#include "RlcMacCommon.h"
#include "ieTFI.h"
#include "ieTLLI.h"

#define NULL_TRX     -1
#define INVALID_TAI  0xff

// *******************************************************************
// class Tbf
//
// Description
//    
// *******************************************************************
class Tbf {
public:
    typedef enum
    {
        FREE,
        ALLOCATED,
        INACTIVE,
        ACTIVATING,
        REALLOCATING,
        RELEASING
    } TBF_STATE;

    // Constructor
    Tbf() : tbfState(FREE), trx(NULL_TRX), tai(INVALID_TAI)
    {
        // Clear out allocated resources.
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            allocatedTs[i] = FALSE;
        }
        taiTs = 0xff;
        tlli.SetTLLI(0xffffffff);
        tfi.SetTFI(0x1f);
    };

    // Destructor
    ~Tbf() {};
     
    TBF_STATE GetTbfState() { return(tbfState); }
    void SetTbfState(TBF_STATE state) { tbfState = state; }
    
    IeTFI                           tfi;
    IeTLLI                          tlli;
    unsigned char                   taiTs;
    unsigned char                   tai;

    // Allocated resources
    int                             trx;
    bool                            allocatedTs[MAX_TIMESLOTS];
    bool                            naturalTs[MAX_TIMESLOTS]; 
      
protected:
    // Control message acknowledgement tags.
    static unsigned char            sysCtlAckCount;
private:
    TBF_STATE                       tbfState;
};


#endif