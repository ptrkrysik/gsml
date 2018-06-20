// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : Multislot.h
// Author(s)   : Daryl Kaiser
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __SCHEDULEALLOCATION_H__
#define __SCHEDULEALLOCATION_H__

#include "RlcMacCommon.h"
#include "RlcMacResult.h"
#include "Tbf.h"


// *******************************************************************
// class ScheduleAllocationInfo
//
// Description: A transient object created when scheduling a new user
//    
// *******************************************************************
class ScheduleAllocationInfo {

public:

    ScheduleAllocationInfo(int mclass, int dclass, int pclass,
                           int ptclass, Tbf* tbfArg, LinkDirection dir,
                           bool* allocatedTsOppositeDirection) : 
        multislotClass(mclass), delayClass(dclass), precedenceClass(pclass),
        peakThroughputClass(ptclass), tbf(tbfArg), direction(dir),
        maxDlSlots(0), maxUlSlots(0), nbrOfCandConfigs(0)
    {
        if ( allocatedTsOppositeDirection )
        {
            if ( dir==DL )
            {
                for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
                {
                    allocatedTsDL[ts] = FALSE;
                    allocatedTsUL[ts] = allocatedTsOppositeDirection[ts];
                }
            }
            else
            {
                for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
                {
                    allocatedTsDL[ts] = allocatedTsOppositeDirection[ts];
                    allocatedTsUL[ts] = FALSE;
                }
            }
        }
        else
        {
            for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
            {
                allocatedTsDL[ts] = FALSE;
                allocatedTsUL[ts] = FALSE;
            }
        }
    }

    ~ScheduleAllocationInfo() {}
   
    int GetMultislotClass(void) const
    {
        return (multislotClass);
    }
    int GetDelayClass(void) const
    {
        return (delayClass);
    }
    int GetPrecedenceClass(void) const
    {
        return (precedenceClass);
    }
    int GetPeakThroughputClass(void) const
    {
        return (peakThroughputClass);
    }
    Tbf* GetTbf(void) const
    {
        return (tbf);
    }
    LinkDirection GetDirection(void) const
    {
        return (direction);
    }
    void AddSlotAllocation(void) 
    {
        if ( direction == DL )
        {
            allocatedTsDL[slotAlloc.ts] = TRUE;
        }
        else
        {
            allocatedTsUL[slotAlloc.ts] = TRUE;
        }
    }
    void InitializeNewSlotAllocation(void) 
    {
        slotAlloc.trx = -1;
        slotAlloc.ts = -1;
        slotAlloc.entries = -1;
    }
    void SetNewSlotAllocation(int trx, int ts, int entries) 
    {
        slotAlloc.trx = trx;
        slotAlloc.ts = ts;
        slotAlloc.entries = entries;
    }
    void GetNewSlotAllocation(int* trx, int* ts, int* entries) const
    {
        *trx = slotAlloc.trx;
        *ts = slotAlloc.ts;
        *entries = slotAlloc.entries;
    }
    int GetNewSlotEntries(void) const
    {
        return(slotAlloc.entries);
    }
    bool ComputeMultislotOptions(void);
    int  ComputeDesiredNumberOfSlots(void) const;
    void ConstrainCandMultiConfig(void);
    bool IsAllowedTs(LinkDirection dir, int ts);
    bool IncludesNaturalTs(LinkDirection dir, int newTs);

	/*-------------------------------------------------------------------------
    * Multislot Configuration Generator Tables
    *-------------------------------------------------------------------------*/
    enum { PAT_ORDER =  8 };
    enum { PAT_INDEX = 35 };
    enum { PAT_SLOTS =  8 };
    enum { MULTI_CLASS =  12 };
    enum { MULTI_INDEX =  28 };
    enum { MULTI_SETS  =  28 };
    enum { MULTI_PARMS =  7 };
    enum { MAX_MULTI_CONFIG_CANDIDATES = 173 };

    static const int   nbrTsPatterns[PAT_ORDER];
    static const char  tsPattern[PAT_ORDER][PAT_INDEX][PAT_SLOTS];
    static const int   nbrMultiConfig[MULTI_CLASS];
    static const char* multiConfig[MULTI_CLASS][MULTI_INDEX];
    static const char  ms[MULTI_SETS][MULTI_PARMS]; //multislot param sets
    static const char  multiMaxDlSlots[MULTI_CLASS];
    static const char  multiMaxUlSlots[MULTI_CLASS];

    struct MultiSet
    {
        const char rangeStartDL;
        const char rangeEndDL;
        const char patternOrderDL;
        const char patternIndexDL;
        const char slotOffsetUL;
        const char patternOrderUL;
        const char patternIndexUL;
    };
	/*-------------------------------------------------------------------------
    * Candidate multislot configurations for new TBF
    *-------------------------------------------------------------------------*/
    struct MultislotConfig
    {
        bool valid;
        bool allowedTsDL[MAX_TIMESLOTS];
        bool allowedTsUL[MAX_TIMESLOTS];
    };
    int             nbrOfCandConfigs;
    MultislotConfig candMultiConfig[MAX_MULTI_CONFIG_CANDIDATES];

private:

    int multislotClass;
    int delayClass;
    int precedenceClass;
    int peakThroughputClass;
    Tbf* tbf;
    LinkDirection direction;
    int maxDlSlots;
    int maxUlSlots;
    bool allocatedTsDL[MAX_TIMESLOTS];
    bool allocatedTsUL[MAX_TIMESLOTS];
	/*-------------------------------------------------------------------------
    * New slot allocation (temporary for communicating each new slot)
    *-------------------------------------------------------------------------*/
    struct SlotAllocation
    {
        int trx;
        int ts;
        int entries; 
    };
    SlotAllocation slotAlloc; 
};


#endif
