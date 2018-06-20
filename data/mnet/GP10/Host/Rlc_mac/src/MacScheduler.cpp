// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MacScheduler.cpp
// Author(s)   : Daryl Kaiser
// Create Date : 12/22/2000
// Description : 
//
// *******************************************************************

#include "RlcMacTask.h"
#include "Timeslot.h"
#include "ScheduleInfo.h"
#include "ScheduleAllocationInfo.h"
#include "Grr/grr_intf.h"
#include "vipermib.h"

//******************************************************************************
//  MAC Resource Allocators/Deallocators
//******************************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DLResourceRequest (2 versions)
**
**    PURPOSE: Assign downlink resources to a TBF.
**
**    INPUT PARAMETERS: 
**                      pDlTbf  - ptr to DL TBF structure
**                      pUlTbf  - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether user was allocated (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::DLResourceRequest(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::DLResourceRequest(DL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();
    
    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pDlTbf->GetMultislotClass(),
                                           pDlTbf->GetDelayClass(),
                                           pDlTbf->GetPrecedenceClass(),
                                           pDlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pDlTbf,
                                           DL,
                                           NULL);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUser(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

bool RlcMacTask::DLResourceRequest(DLTbf *pDlTbf, ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::DLResourceRequest(DL&UL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    /*-------------------------------------------------------------------------
    * Copy the TRX and allowed slots from the existing UL TBF to the new DL TBF
    *-------------------------------------------------------------------------*/
    pDlTbf->trx = pUlTbf->trx;

    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pDlTbf->GetMultislotClass(),
                                           pDlTbf->GetDelayClass(),
                                           pDlTbf->GetPrecedenceClass(),
                                           pDlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pDlTbf,
                                           DL,
                                           pUlTbf->allocatedTs);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUser(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DLSingleTSResourceRequest
**
**    PURPOSE: Assign downlink resources to a TBF, only in one time slot.
**
**    INPUT PARAMETERS: pDlTbf - ptr to DL TBF structure
**
**    RETURN VALUE(S): indicates whether user was released (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::DLSingleTSResourceRequest(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::DLSingleTSResourceRequest", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(1, // Multislot Class 1 = one TS only
                                           pDlTbf->GetDelayClass(),
                                           pDlTbf->GetPrecedenceClass(),
                                           pDlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pDlTbf,
                                           DL,
                                           NULL);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUser(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULResourceRequest (2 versions)
**
**    PURPOSE: Assign uplink resources to a TBF.
**
**    INPUT PARAMETERS:  pUlTbf - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether user was allocated (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::ULResourceRequest(ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::ULResourceRequest(UL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pUlTbf->GetMultislotClass(),
                                           pUlTbf->GetDelayClass(),
                                           pUlTbf->GetPrecedenceClass(),
                                           pUlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pUlTbf,
                                           UL, 
                                           NULL);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUser(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

bool RlcMacTask::ULResourceRequest(ULTbf *pUlTbf, DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::ULResourceRequest(UL&DL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pUlTbf->GetMultislotClass(),
                                           pUlTbf->GetDelayClass(),
                                           pUlTbf->GetPrecedenceClass(),
                                           pUlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pUlTbf,
                                           UL,
                                           pDlTbf->allocatedTs);
    /*-------------------------------------------------------------------------
    * Copy the TRX from the existing TBF to the new TBF. 
    *-------------------------------------------------------------------------*/
    pUlTbf->trx = pDlTbf->trx;

    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUser(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DLResourceReallocRequest (2 versions)
**
**    PURPOSE: Reallocate downlink resources to a TBF.
**
**    INPUT PARAMETERS: 
**                      pDlTbf  - ptr to DL TBF structure
**                      pUlTbf  - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether user was allocated (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::DLResourceReallocRequest(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::DLResourceReallocRequest(DL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();
    
    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pDlTbf->GetMultislotClass(),
                                           pDlTbf->GetDelayClass(),
                                           pDlTbf->GetPrecedenceClass(),
                                           pDlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pDlTbf,
                                           DL, 
                                           NULL);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUserIfBetterQoS(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

bool RlcMacTask::DLResourceReallocRequest(DLTbf *pDlTbf, ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::DLResourceReallocRequest(DL&UL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    /*-------------------------------------------------------------------------
    * Copy the TRX and allowed slots from the existing UL TBF to the new DL TBF
    *-------------------------------------------------------------------------*/
    pDlTbf->trx = pUlTbf->trx;
     
    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pDlTbf->GetMultislotClass(),
                                           pDlTbf->GetDelayClass(),
                                           pDlTbf->GetPrecedenceClass(),
                                           pDlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pDlTbf,
                                           DL, 
                                           pUlTbf->allocatedTs);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUserIfBetterQoS(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULResourceReallocRequest (2 versions)
**
**    PURPOSE: Reallocate uplink resources to a TBF.
**
**    INPUT PARAMETERS: 
**                      pDlTbf  - ptr to DL TBF structure
**                      pUlTbf  - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether user was allocated (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::ULResourceReallocRequest(ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::ULResourceReallocRequest(UL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pUlTbf->GetMultislotClass(),
                                           pUlTbf->GetDelayClass(),
                                           pUlTbf->GetPrecedenceClass(),
                                           pUlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pUlTbf,
                                           UL,
                                           NULL);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUserIfBetterQoS(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

bool RlcMacTask::ULResourceReallocRequest(ULTbf *pUlTbf, DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::ULResourceReallocRequest(UL&DL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    /*-------------------------------------------------------------------------
    * Copy the TRX and allowed slots from the existing DL TBF to the new UL TBF
    *-------------------------------------------------------------------------*/
    pUlTbf->trx = pDlTbf->trx;

    bool status = FALSE;
    ScheduleAllocationInfo  schedAllocInfo(pUlTbf->GetMultislotClass(),
                                           pUlTbf->GetDelayClass(),
                                           pUlTbf->GetPrecedenceClass(),
                                           pUlTbf->GetPeakThroughputClass(),
                                           (Tbf*)pUlTbf,
                                           UL,
                                           pDlTbf->allocatedTs);
    /*-------------------------------------------------------------------------
    * Determine multislot options for this user, then allocate time slots
    *-------------------------------------------------------------------------*/
    if ( schedAllocInfo.ComputeMultislotOptions() )
    {
        status = AllocateUserIfBetterQoS(&schedAllocInfo);
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DLReleaseResourceRequest
**
**    PURPOSE: Release downlink resources of a TBF.
**
**    INPUT PARAMETERS: pDlTbf - ptr to DL TBF structure
**
**    RETURN VALUE(S): indicates whether user was released (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::DLReleaseResourceRequest(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::DLReleaseResourceRequest", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = DeallocateUser((Tbf*)pDlTbf, DL);

    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULReleaseResourceRequest
**
**    PURPOSE: Release uplink resources of a TBF.
**
**    INPUT PARAMETERS: pUlTbf - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether user was released (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::ULReleaseResourceRequest(ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::ULReleaseResourceRequest", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = DeallocateUser((Tbf*)pUlTbf, UL);

    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULSingleBlockRequest
**
**    PURPOSE: Choose TRX and TS for uplink single block by finding the slot
**             whose most heavily loaded direction is utilizing the least of
**             its total capacity.
**
**    INPUT PARAMETERS: pUlTbf - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether GPRS TRX/TS was found (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::ULSingleBlockRequest(int *bestTrx, int *bestTs)
{
    DBG_FUNC("RlcMacTask::ULSingleBlockRequest", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    float bestCapacity = 1.0;
    bool status = FALSE;

    for ( int trx=0; trx<MAX_TRX; trx++ )
    {
        for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
        {
            if ( grr_GetTrxSlotChanComb(trx, ts) == pDCHData &&
                 grr_GetTrxSlotOpState(trx, ts) == opStateEnabled )
            {
                ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(trx, ts); 
                float capUL = scheduleInfo->GetUtilizedCapacity(UL);
                float capDL = scheduleInfo->GetUtilizedCapacity(DL);
                float utilizedCapacity = max(capUL, capDL);
                if ( utilizedCapacity < bestCapacity )
                {
                    *bestTrx = trx;
                    *bestTs = ts;
                    bestCapacity = utilizedCapacity;
                    status = TRUE;
                }
            }
        }
    }
    DBG_LEAVE();
    return(status);
}


//******************************************************************************
//  MAC Schedulers
//******************************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::TSSelectionRequest (2 versions)
**
**    PURPOSE: Choose TRX and TS for control message on existing DL or UL TBF
**
**    INPUT PARAMETERS: 
**                      pDlTbf  - ptr to DL TBF structure
**                      pUlTbf  - ptr to UL TBF structure
**
**    OUTPUT PARAMETERS: 
**                      trx, ts
**
**    RETURN VALUE(S): indicates whether GPRS TRX/TS was found (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::TSSelectionRequest(int *trx, int *ts, DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::TSSelectionRequest(DL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = GetNaturalSlot((Tbf*)pDlTbf, trx, ts);

    DBG_LEAVE();
    return(status); 
}

bool RlcMacTask::TSSelectionRequest(int *trx, int *ts, ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::TSSelectionRequest(UL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = GetNaturalSlot((Tbf*)pUlTbf, trx, ts);

    DBG_LEAVE();
    return(status); 
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::PollingTsCheck 
**
**    PURPOSE: Choose TRX and TS for control message on existing DL or UL TBF
**
**    INPUT PARAMETERS:
**                      trx     - TRX
**                      ts      - time slot 
**                      pDlTbf  - ptr to DL TBF structure
**
**    RETURN VALUE(S): indicates whether GPRS TRX/TS was found (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::PollingTsCheck(int trx, int ts, DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::PollingTsCheck(DL)", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = (pDlTbf->trx == trx) && pDlTbf->naturalTs[ts];

    DBG_LEAVE();
    return(status); 
}
/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ScheduleRLCBlock (3 versions)
**
**    PURPOSE: Schedule the next RLC block(s) on specified TRX and TS. One
**             method for each direction, and one for simultaneous scheduling.
**
**    INPUT PARAMETERS:
**                      trx     - transceiver index (0-MAX_TRX)
**                      ts      - time slot index (0-MAX_TIMESLOTS)
**                      pDlTbf  - ptr to DL TBF structure
**                      pUlTbf  - ptr to UL TBF structure
**
**    RETURN VALUE(S): indicates whether entry was scheduled (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::ScheduleRLCBlock(int trx, int ts, DLTbf **ppDlTbf)
{
    DBG_FUNC("RlcMacTask::ScheduleRLCBlock(DL)", RLC_MAC_SCHEDULER);
    DBG_ENTER();

    DBG_TRACE("  Scheduling downlink TRX %d, TS %d\n", trx, ts);

    ScheduleQueue *scheduleQueue = tsPool.GetQueue(trx, ts, DL); 
    bool status = scheduleQueue->ScheduleNextBlock((Tbf**)ppDlTbf);

    DBG_LEAVE();
    return(status);
}

bool RlcMacTask::ScheduleRLCBlock(int trx, int ts, ULTbf **ppUlTbf)
{
    DBG_FUNC("RlcMacTask::ScheduleRLCBlock(UL)", RLC_MAC_SCHEDULER);
    DBG_ENTER();

    DBG_TRACE("  Scheduling uplink TRX %d, TS %d\n", trx, ts);

    ScheduleQueue *scheduleQueue = tsPool.GetQueue(trx, ts, UL); 
    bool status = scheduleQueue->ScheduleNextBlock((Tbf**)ppUlTbf);

    DBG_LEAVE();
    return(status);
}

bool RlcMacTask::ScheduleRLCBlock(int trx, int ts, DLTbf **ppDlTbf, ULTbf **ppUlTbf)
{
    DBG_FUNC("RlcMacTask::ScheduleRLCBlock(DL&UL)", RLC_MAC_SCHEDULER);
    DBG_ENTER();

    DBG_TRACE("  Scheduling downlink/uplink TRX %d, TS %d\n", trx, ts);

    ScheduleQueue *scheduleQueueDL = tsPool.GetQueue(trx, ts, DL); 
    ScheduleQueue *scheduleQueueUL = tsPool.GetQueue(trx, ts, UL); 
    bool  sentDL = scheduleQueueDL->ScheduleNextBlock((Tbf**)ppDlTbf);
    bool  sentUL = scheduleQueueUL->ScheduleNextBlock((Tbf**)ppUlTbf);

    DBG_LEAVE();
    return(sentDL & sentUL);
}
    


//******************************************************************************
//  MAC Scheduler Helpers
//******************************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::AllocateUser
**
**    PURPOSE: Allocate time slots to DL or UL TBF
**
**    INPUT PARAMETERS: schedAllocInfo - ptr to object for allocating schedule
**
**    RETURN VALUE(S): indicates whether entry was assigned (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::AllocateUser (ScheduleAllocationInfo *schedAllocInfo)
{
    DBG_FUNC("RlcMacTask::AllocateUser", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = FALSE;
    int  slotCount = 0;
    int  desiredSlots = schedAllocInfo->ComputeDesiredNumberOfSlots();

    /*-------------------------------------------------------------------------
    * First eliminate any multislot configurations that include slots which
    * have been disallowed already for this TBF, perhaps due to contraints in
    * the opposite direction TBF.
    *-------------------------------------------------------------------------*/
    schedAllocInfo->ConstrainCandMultiConfig();

    /*-------------------------------------------------------------------------
    * Assign user to the best available slot up to desired # of slots
    *-------------------------------------------------------------------------*/
    while ( slotCount<desiredSlots && AllocateOneSlot(schedAllocInfo) )
    {
        status = TRUE;
        slotCount += 1;
    }    
    /*-------------------------------------------------------------------------
    * If bandwidth was allocated...
    *-------------------------------------------------------------------------*/
    if ( status )
    {
        Tbf* tbf = schedAllocInfo->GetTbf();
        LinkDirection dir = schedAllocInfo->GetDirection();
        /*---------------------------------------------------------------------
        * Assign a TFI to the new user
        *---------------------------------------------------------------------*/
        if ( dir == DL )
        {
            tsPool.AllocateDLTFI(tbf->trx, (DLTbf*)tbf);
        }
        else
        {
            tsPool.AllocateULTFI(tbf->trx, (ULTbf*)tbf);
        }
        /*---------------------------------------------------------------------
        * Assign a TAI on the first allocated slot and a USF on each slot
        *---------------------------------------------------------------------*/
        bool allocatedTai = FALSE;
        for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
        {
            if ( tbf->allocatedTs[ts] )
            {
                if ( !allocatedTai )
                {
                    allocatedTai = TRUE;
                    tsPool.AllocateTAI(tbf->trx, ts, tbf);
                }
                if ( dir == UL )
                {
                    tsPool.AllocateUSF(tbf->trx, ts, (ULTbf*)tbf);
                }
            }
        }
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::AllocateUserIfBetterQoS
**
**    PURPOSE: Allocate time slots to an existing DL or UL TBF if the result
**             is better quality of service, defined here as more allocated
**             time slots or more schedule queue entries at the desired level,
**             which translates to higher data throughput.
**
**    INPUT PARAMETERS: schedAllocInfo - ptr to object for allocating schedule
**
**    RETURN VALUE(S): indicates whether entry was assigned (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::AllocateUserIfBetterQoS (ScheduleAllocationInfo *schedAllocInfo)
{
    DBG_FUNC("RlcMacTask::AllocateUserIfBetterQoS", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    bool status = FALSE;
    int  newSlotCount  = 0;
    int  oldSlotCount  = 0;
    int  newEntryCount = 0;
    int  oldEntryCount = 0;
    int  nbrEntries    = 0;
    int  desiredSlots = schedAllocInfo->ComputeDesiredNumberOfSlots();
    Tbf*          tbf = schedAllocInfo->GetTbf();
    LinkDirection dir = schedAllocInfo->GetDirection();
    int  desiredLevel = schedAllocInfo->GetDelayClass() - 1;
    bool originallyAllocatedTs[MAX_TIMESLOTS];

    /*-------------------------------------------------------------------------
    * First eliminate any multislot configurations that include slots which
    * have been disallowed already for this TBF, perhaps due to contraints in
    * the opposite direction TBF.
    *-------------------------------------------------------------------------*/
    schedAllocInfo->ConstrainCandMultiConfig();

    /*-------------------------------------------------------------------------
    * On each TS of the affected user's TRX, copy the current master schedule
    * queue allocations of all users and deallocate the affected user's queue
    * entries. Later, if the new allocation does not improve this user's QoS, 
    * we'll restore the allocations of all users to their original state.
    *-------------------------------------------------------------------------*/
    for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
    {
        ScheduleQueue *scheduleQueue = tsPool.GetQueue(tbf->trx, ts, dir);
        scheduleQueue->CopyQueue();
        /*------------------------------------------------------------------
        * Save the TBF's originally allocated slots
        *------------------------------------------------------------------*/
        originallyAllocatedTs[ts] = tbf->allocatedTs[ts]; 
        /*------------------------------------------------------------------
        * Deallocate affected user's allocated schedule queue entries
        *------------------------------------------------------------------*/
        if ( tbf->allocatedTs[ts] )
        {
            /*--------------------------------------------------------------------
            * For each slot allocated to this user, search all queue levels
            *--------------------------------------------------------------------*/
            ScheduleQueue *scheduleQueue = tsPool.GetQueue(tbf->trx, ts, dir);
            for ( int level=0; level<ScheduleQueue::QUEUE_NUM_LEVELS; level++ )
            {
                /*----------------------------------------------------------------
                * Deallocate entries at this queue level, rebalance remaining
                *----------------------------------------------------------------*/
                nbrEntries = scheduleQueue->DeallocateEntries(tbf, level);
                if ( nbrEntries > 0 )
                {
                    scheduleQueue->BalanceEntries(level);
                    /*-----------------------------------------------------------
                    * Add up affected user's slots and entries at desired level
                    *-----------------------------------------------------------*/
                    if ( level==desiredLevel )
                    {
                        oldSlotCount += 1;
                        oldEntryCount += nbrEntries;
                    }
                }
            }
            /*---------------------------------------------------------------------
            * Update # of users and utilized capacity in this slot and direction
            *---------------------------------------------------------------------*/
            ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(tbf->trx, ts);
            scheduleInfo->UpdateUsersAndCapacity(dir);
        }
        /*------------------------------------------------------------------
        * Clear out the TBF's allocated slots
        *------------------------------------------------------------------*/
        tbf->allocatedTs[ts] = FALSE;
    }
    /*-------------------------------------------------------------------------
    * Look for a better QoS assignment and count the number of entries
    *-------------------------------------------------------------------------*/
    while ( newSlotCount<desiredSlots && AllocateOneSlot(schedAllocInfo) )
    {
        int newTrx, newTs, newEntries;
        schedAllocInfo->GetNewSlotAllocation(&newTrx, &newTs, &newEntries);
        newEntryCount += newEntries;
        newSlotCount += 1;
    }
    
    /*-------------------------------------------------------------------------
    * If more slots or more total queue entries can be allocated...
    *-------------------------------------------------------------------------*/
    if ( newSlotCount>oldSlotCount || newEntryCount>oldEntryCount )
    {
        status = TRUE;
        /*---------------------------------------------------------------------
        * Keep the user's TFI, but release the user's old TAI and USFs
        *---------------------------------------------------------------------*/
        tsPool.ReleaseTAI(tbf->trx, tbf->taiTs, tbf->tai);
        if ( dir == UL )
        {
            for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
            {
                tsPool.ReleaseUSF(tbf->trx, ts, ((ULTbf*)tbf)->usf[ts]);
            }
        }
        /*---------------------------------------------------------------------
        * Assign a new TAI on the 1st allocated slot and a new USF on each slot
        *---------------------------------------------------------------------*/
        bool allocatedTai = FALSE;
        for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
        {
            if ( tbf->allocatedTs[ts] )
            {
                if ( !allocatedTai )
                {
                   allocatedTai = TRUE;
                   tsPool.AllocateTAI(tbf->trx, ts, tbf);
                }
                if ( dir == UL )
                {
                   tsPool.AllocateUSF(tbf->trx, ts, (ULTbf*)tbf);
                }
            }
        }
    }
    /*-------------------------------------------------------------------------
    * If QoS cannot be improved, restore the master schedule queues to their
    * original states. This leaves all user allocations on this TRX as they
    * were before attempting AllocateUserIfBetterQoS. Also restore the TBF's
    * original allocatedTS array.
    *-------------------------------------------------------------------------*/
    else
    {
        status = FALSE;
        for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
        {
            ScheduleQueue *scheduleQueue = tsPool.GetQueue(tbf->trx, ts, dir);
            scheduleQueue->RestoreQueue();
            tbf->allocatedTs[ts] = originallyAllocatedTs[ts];
        }
    }
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::AllocateOneSlot
**
**    PURPOSE: Allocate schedule queue entries to user in time slot that
**             gives highest throughput within requested delay class
**
**    INPUT PARAMETERS: schedAllocInfo - ptr to object for allocating schedule
**
**    RETURN VALUE(S): indicates whether entry was assigned (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::AllocateOneSlot (ScheduleAllocationInfo *schedAllocInfo) 
{
    DBG_FUNC("RlcMacTask::AllocateOneSlot", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    int desiredLevel = schedAllocInfo->GetDelayClass() - 1;
    int precedence = schedAllocInfo->GetPrecedenceClass();
    LinkDirection dir = schedAllocInfo->GetDirection();
    Tbf*  tbf = schedAllocInfo->GetTbf();

    /*-------------------------------------------------------------------------
    * Find the best TRX/TS (the TS allowing the most queue entries)
    *-------------------------------------------------------------------------*/
    FindBestSlot (schedAllocInfo);
    
    /*-------------------------------------------------------------------------
    * Allocate queue entries in the best TRX/TS and record the TRX and TS
    * in the TBF object. Also in the TBF object, update the allowed UL and DL
    * slots based on the new allocation. Update the scheduler's user count and 
    * utilized capacity for the slot.
    *-------------------------------------------------------------------------*/
    int bestTrx, bestTs, bestEntries;
    schedAllocInfo->GetNewSlotAllocation(&bestTrx, &bestTs, &bestEntries);
    if ( bestEntries > 0 )
    {
        ScheduleQueue *scheduleQueue = tsPool.GetQueue(bestTrx, bestTs, dir); 
        if ( scheduleQueue->AllocateEntries(tbf, desiredLevel, precedence, FALSE) )
        {
            DBG_TRACE("  Allocated %d schedule queue entries\n", bestEntries);
            tbf->trx = bestTrx;
            tbf->allocatedTs[bestTs] = TRUE;
            UpdateAllowedSlots(schedAllocInfo);
            ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(bestTrx, bestTs);
            scheduleInfo->UpdateUsersAndCapacity(dir);
        }
        else
        {
            DBG_TRACE("  Error allocating in RlcMacTask::AllocateOneSlot (see code)\n");
        }
    }
    DBG_LEAVE();
    return(bestEntries>0);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::FindBestSlot
**
**    PURPOSE: Find time slot that gives highest throughput within requested
**             delay class
**
**    INPUT PARAMETERS: schedAllocInfo - ptr to object for allocating schedule
**
**    RETURN VALUE(S): indicates whether any queue entries could be assigned
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::FindBestSlot (ScheduleAllocationInfo *schedAllocInfo) 
{
    DBG_FUNC("RlcMacTask::FindBestSlot", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    int availableEntries;
    int desiredLevel = schedAllocInfo->GetDelayClass() - 1;
    int precedence = schedAllocInfo->GetPrecedenceClass();
    LinkDirection dir = schedAllocInfo->GetDirection();
    Tbf*  tbf = schedAllocInfo->GetTbf();
    int   trx = tbf->trx;

    if ( dir == DL )
    {
        DBG_TRACE("  DL Desired queue level = %d, Precedence = %d\n", desiredLevel, precedence);
    }
    else
    {
        DBG_TRACE("  UL Desired queue level = %d, Precedence = %d\n", desiredLevel, precedence);
    }
    
    /*-------------------------------------------------------------------------
    * Initialize slotAlloc, a temporary structure for saving best slot info
    *-------------------------------------------------------------------------*/
    schedAllocInfo->InitializeNewSlotAllocation();

    /*-------------------------------------------------------------------------
    * Look for time slot allowing the most queue entries
    *-------------------------------------------------------------------------*/
    for ( int localTrx=0; localTrx<MAX_TRX; localTrx++ )
    {
        /*-------------------------------------------------------------------------
        * If no slots have been assigned, trx is null. Consider all TRX. Once first
        * slot has been assigned, make sure subsequent slots are on the same TRX.
        *-------------------------------------------------------------------------*/
        if ( trx==NULL_TRX || trx==localTrx )
        {
            for ( int tsCount=0; tsCount<MAX_TIMESLOTS; tsCount++ )
            {
                /*-------------------------------------------------------------------------
                * For DL, check from TS0 to TS7, which prefers lower numbered slots in case
                * of a tie. For UL, check from TS7 to TS0, which prefers higher slots.
                *-------------------------------------------------------------------------*/
                int ts = (dir==DL) ? (tsCount) : (MAX_TIMESLOTS-tsCount-1);
                ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(localTrx, ts);
    
    // temporary debug trace -- remove soon             
    DBG_TRACE("  Look at trx%d, ts%d: okayToAdd=%d, allowed=%d, natural=%d, users(DL,UL)=%d,%d\n",
    localTrx, ts, scheduleInfo->OkayToAddUser(dir), schedAllocInfo->IsAllowedTs(dir, ts),
    schedAllocInfo->IncludesNaturalTs(dir, ts), 
    scheduleInfo->GetNbrUsers(DL), scheduleInfo->GetNbrUsers(UL));

                if ( grr_GetBtsAmState()                 == unlocked       &&
                     grr_GetTrxAmState(localTrx)         == unlocked       &&
                     grr_GetTrxSlotAmState(localTrx, ts) == unlocked       &&
                     grr_GetBtsOpState()                 == opStateEnabled &&
                     grr_GetTrxOpState(localTrx)         == opStateEnabled && 
                     grr_GetTrxSlotOpState(localTrx, ts) == opStateEnabled &&
                     grr_GetTrxSlotChanComb(localTrx, ts)== pDCHData       &&
                     scheduleInfo->OkayToAddUser(dir)                      &&
                     schedAllocInfo->IsAllowedTs(dir, ts)                  &&
                     schedAllocInfo->IncludesNaturalTs(dir, ts) )
                {
    // temporary debug trace -- remove soon             
    DBG_TRACE("  Allowed to use trx=%d, ts=%d. Looking for space in schedule queue...\n", localTrx, ts);
    
                    ScheduleQueue *scheduleQueue = tsPool.GetQueue(localTrx, ts, dir); 
                    if ( (availableEntries=scheduleQueue->
                            FindAvailableEntries(tbf, desiredLevel, precedence, FALSE))
                            > schedAllocInfo->GetNewSlotEntries() )
                    {
                        schedAllocInfo->SetNewSlotAllocation(localTrx, ts, availableEntries);
                    }
                }
            }
        }
    }

    // temporary debug trace -- remove soon             
    int bestTrx, bestTs, bestEntries;
    schedAllocInfo->GetNewSlotAllocation(&bestTrx, &bestTs, &bestEntries);
    DBG_TRACE("  bestEntries=%d, bestTrx=%d, bestTs=%d\n", bestEntries, bestTrx, bestTs);

    DBG_LEAVE();
    return(schedAllocInfo->GetNewSlotEntries()>0);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DeallocateUser  
**
**    PURPOSE: Release downlink or uplink resources from a TBF.
**
**    INPUT PARAMETERS: dir - link direction (DL or UL)
**
**    RETURN VALUE(S): indicates whether user was allocated (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::DeallocateUser (Tbf* tbf, LinkDirection dir)
{
    bool status = TRUE;

    /*-------------------------------------------------------------------------
    * Loop over all TS on which this TBF was allocated (on assigned trx)
    *-------------------------------------------------------------------------*/
    for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
    {
        if ( tbf->allocatedTs[ts] )
        {
            /*--------------------------------------------------------------------
            * For each slot allocated to this user, search all queue levels
            *--------------------------------------------------------------------*/
            ScheduleQueue *scheduleQueue = tsPool.GetQueue(tbf->trx, ts, dir);
            for ( int level=0; level<ScheduleQueue::QUEUE_NUM_LEVELS; level++ )
            {
                /*----------------------------------------------------------------
                * Deallocate entries at this queue level, rebalance remaining
                *----------------------------------------------------------------*/
                if ( scheduleQueue->DeallocateEntries(tbf, level) )
                {
                    scheduleQueue->BalanceEntries(level);
                }
            }
            /*---------------------------------------------------------------------
            * Update # of users and utilized capacity in this slot and direction
            *---------------------------------------------------------------------*/
            ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(tbf->trx, ts);
            scheduleInfo->UpdateUsersAndCapacity(dir);
            /*---------------------------------------------------------------------
            * If uplink, release the USF on this slot
            *---------------------------------------------------------------------*/
            if ( dir == UL )
            {
                tsPool.ReleaseUSF(tbf->trx, ts, ((ULTbf*)tbf)->usf[ts]);
            }
        }
        /*-------------------------------------------------------------------------
        * Clear the TBF's allocated resources
        *-------------------------------------------------------------------------*/
        tbf->allocatedTs[ts] = FALSE;
    }
    /*-------------------------------------------------------------------------
    * Release the TAI and TFI
    *-------------------------------------------------------------------------*/
    tsPool.ReleaseTAI(tbf->trx, tbf->taiTs, tbf->tai);
    if ( dir == DL )
    {
        tsPool.ReleaseDLTFI(tbf->trx, tbf->tfi.GetTFI());
    }
    else
    {
        tsPool.ReleaseULTFI(tbf->trx, tbf->tfi.GetTFI());
    }
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::GetLeastUtilizedSlot  
**
**    PURPOSE: Get least utilized time slot allocated to TBF
**
**    INPUT PARAMETERS: 
**                      pTbf    - ptr to TBF structure
**                      trx     - transceiver index (0..MAX_TRX-1)
**                      ts      - time slot index (0..MAX_TIMESLOTS-1)
**
**    RETURN VALUE(S): indicates whether slot was found (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::GetLeastUtilizedSlot(Tbf* tbf, int *trxArg, int *tsArg) 
{
    float bestCapacity = 1.0;
    bool status = FALSE;

    *trxArg = tbf->trx;

    for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
    {
        if ( tbf->allocatedTs[ts] )
        {
            ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(tbf->trx, ts); 
            float capUL = scheduleInfo->GetUtilizedCapacity(UL);
            float capDL = scheduleInfo->GetUtilizedCapacity(DL);
            float utilizedCapacity = max(capUL, capDL);
            if ( utilizedCapacity < bestCapacity )
            {
                *tsArg = ts;
                bestCapacity = utilizedCapacity;
                status = TRUE;
            }
        }
    }
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::GetNaturalSlot  
**
**    PURPOSE: Get natural time slot for polling TBF
**
**    INPUT PARAMETERS: 
**                      pTbf    - ptr to TBF structure
**                      trx     - transceiver index (0..MAX_TRX-1)
**                      ts      - time slot index (0..MAX_TIMESLOTS-1)
**
**    RETURN VALUE(S): indicates whether slot was found (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::GetNaturalSlot(Tbf* tbf, int *trxArg, int *tsArg) 
{
    bool status = FALSE;

    *trxArg = tbf->trx;

    for ( int ts=0; ts<MAX_TIMESLOTS && !status; ts++ )
    {
        if ( tbf->naturalTs[ts] )
        {
            *tsArg = ts;
            status = TRUE;
        }
    }
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::GetBtsGprsConfig  
**
**    PURPOSE: Get BTS GPRS configuration, including time slots that support
**             GPRS and current utilized GPRS capacity on each of the slots.
**
**    INPUT PARAMETERS: trx  - transceiver index (0..MAX_TRX-1) 
**
**    OUTPUT PARAMETERS:
**                      tsMap  - boolean array indicating GPRS for each slot
**                      cap    - utilized GPRS capacity on each slot
**
**    RETURN VALUE(S): number of GPRS time slots on specified TRX
**
**----------------------------------------------------------------------------*/
int RlcMacTask::GetBtsGprsConfig(int trx, bool* tsMap, float* capUL,
                                 float* capDL) 
{
    int nbrGprsTs(0);

    for ( int ts=0; ts<MAX_TIMESLOTS; ts++ )
    {
       if ( grr_GetBtsAmState()            == unlocked       &&
            grr_GetTrxAmState(trx)         == unlocked       &&
            grr_GetTrxSlotAmState(trx, ts) == unlocked       &&
            grr_GetBtsOpState()            == opStateEnabled &&
            grr_GetTrxOpState(trx)         == opStateEnabled && 
            grr_GetTrxSlotOpState(trx, ts) == opStateEnabled &&
            grr_GetTrxSlotChanComb(trx, ts)== pDCHData       )
        {
            ScheduleInfo *scheduleInfo = tsPool.GetScheduleInfo(trx, ts); 
            nbrGprsTs += 1;
            tsMap[ts] = TRUE;
            capUL[ts] = scheduleInfo->GetUtilizedCapacity(UL);
            capDL[ts] = scheduleInfo->GetUtilizedCapacity(DL);
        }
        else
        {
            tsMap[ts] = FALSE;
            capUL[ts] = 1.0;
            capDL[ts] = 1.0;
        }
    }
    return(nbrGprsTs);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::GetBestTrx  
**
**    PURPOSE: Find TRX with most available GPRS capacity on one slot
**
**    INPUT PARAMETERS: trx  - transceiver index (0..MAX_TRX-1) 
**
**    OUTPUT PARAMETERS:
**                      tsMap  - boolean array indicating GPRS for each slot
**                      cap    - utilized GPRS capacity on each slot
**
**    RETURN VALUE(S): number of GPRS time slots on specified TRX
**
**----------------------------------------------------------
int RlcMacTask::GetBestTrx(int* bestTrx, bool* tsMap, float* capUL,
                                 float* capDL) const
{
    float  lowestUtilizedCapacity(1.0);
    bool*  tsMap[MAX_TRX][MAX_TIMESLOTS];
    float  capDL[MAX_TIMESLOTS];
    float  capUL[MAX_TIMESLOTS];
    float* cap = (direction==DL) ? capDL : capUL;
    for ( int trx=0; trx<MAX_TRX; trx++ )
    {
        GetBtsGprsConfig(trx, tsMap[trx], capDL, capUL);
        for ( ts=0; ts<MAX_TIMESLOTS; ts++ )
        {
            if ( cap[ts] < lowestUtilizedCapacity )
            {
                lowestUtilizedCapacity = cap[ts];
                *bestTrx = trx; 
            }
        }       ......................>>> delete this ?
    }
}
--------------------------------------------------------------*/


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::UpdateAllowedSlots  
**
**    PURPOSE: Update the allowed UL and DL time slots based on the information
**             in the schedule allocation object and the TBF's allocated slots.
**             Note that the schedule allocation object reflects the mobile's
**             multislot capability not constrained by the BTS configuration.
**
**    INPUT PARAMETERS: schedAllocInfo - ptr to object for allocating schedule
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::UpdateAllowedSlots(ScheduleAllocationInfo *schedAllocInfo)
{
    /*-------------------------------------------------------------------------
    * Mark newly allocated slot in Schedule Allocation Info structure
    *-------------------------------------------------------------------------*/
    schedAllocInfo->AddSlotAllocation(); 

    /*-------------------------------------------------------------------------
    * Mark invalid all multislot configurations that do not include the
    * time slots allocated to the TBF up to this point. Also update list
    * of potential natural timeslots in TBF structure.
    *-------------------------------------------------------------------------*/
    schedAllocInfo->ConstrainCandMultiConfig();
}





