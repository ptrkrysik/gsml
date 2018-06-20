// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ScheduleAllocationInfo.cpp
// Author(s)   : Daryl Kaiser
// Create Date : 1/29/2001
// Description : 
//
// *******************************************************************

#include "ScheduleAllocationInfo.h"

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleAllocationInfo::ComputeMultislotOptions  
**
**    PURPOSE: Compute valid multislot options (i.e., combinations of UL and DL
**             time slots) based on user capability. Save results in the
**             calling object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
bool ScheduleAllocationInfo::ComputeMultislotOptions(void)
{
    DBG_FUNC("ScheduleAllocationInfo::ComputeMultislotOptions", RLC_MAC_RES_ALLOC);
    DBG_ENTER();

    /*=========================================================================
    * Validate QoS profile. If any value is out of range, limit it.
    *=========================================================================*/
    if ( multislotClass < 1 )
    {
        DBG_ERROR("  Illegal for QoS for TLLI 0x%x: MultislotClass=%d. Setting value to 1.\n",
            tbf->tlli.GetTLLI(), multislotClass);
        multislotClass = 1;
    }
    else if ( multislotClass > 12 )
    {
        DBG_ERROR("  Illegal for TLLI QoS for 0x%x: MultislotClass=%d. Setting value to 12.\n",
            tbf->tlli.GetTLLI(), multislotClass);
        multislotClass = 12;
    }
    if ( delayClass < 1 )
    {
        if ( direction == DL )
        {
           DBG_ERROR("  Illegal for QoS for TLLI 0x%x: DelayClass=%d. Setting value to 1.\n",
               tbf->tlli.GetTLLI(), delayClass);
        }
        else
        {
           DBG_ERROR("  Illegal for QoS for TLLI 0x%x: RadioPriority=%d. Setting value to 1.\n",
               tbf->tlli.GetTLLI(), delayClass);
        }
        delayClass = 1;
    }
    else if ( delayClass > 4 )
    {
        if ( direction == DL )
        {
           DBG_ERROR("  Illegal for QoS for TLLI 0x%x: DelayClass=%d. Setting value to 4.\n",
               tbf->tlli.GetTLLI(), delayClass);
        }
        else
        {
           DBG_ERROR("  Illegal for QoS for TLLI 0x%x: RadioPriority=%d. Setting value to 4.\n",
               tbf->tlli.GetTLLI(), delayClass);
        }
        delayClass = 4;
    }
    if ( precedenceClass < 1 )
    {
        DBG_ERROR("  Illegal for QoS for TLLI 0x%x: PrecedenceClass=%d. Setting value to 1.\n",
            tbf->tlli.GetTLLI(), precedenceClass);
        precedenceClass = 1;
    }
    else if ( precedenceClass > 3 )
    {
        DBG_ERROR("  Illegal for TLLI QoS for 0x%x: PrecedenceClass=%d. Setting value to 3.\n",
            tbf->tlli.GetTLLI(), precedenceClass);
        precedenceClass = 3;
    }
    if ( peakThroughputClass < 1 )
    {
        DBG_ERROR("  Illegal for QoS for TLLI 0x%x: PeakThroughputClass=%d. Setting value to 1.\n",
            tbf->tlli.GetTLLI(), precedenceClass);
        peakThroughputClass = 1;
    }
    else if ( peakThroughputClass > 9 )
    {
        DBG_ERROR("  Illegal for TLLI QoS for 0x%x: PeakThroughputClass=%d. Setting value to 9.\n",
            tbf->tlli.GetTLLI(), precedenceClass);
        peakThroughputClass = 9;
    }

    /*------------------------------------------------------------
    * For troubleshooting, report the QoS parameters
    *------------------------------------------------------------*/
     if ( direction == DL )
     {
         DBG_TRACE("  QoS for TLLI 0x%x: Multi=%d Delay=%d Prec=%d PkTp=%d\n",
             tbf->tlli.GetTLLI(), multislotClass, delayClass, precedenceClass, peakThroughputClass);
     }
     else
     {
         DBG_TRACE("  QoS for TLLI 0x%x: Multi=%d RadioPriority=%d PkTp=%d\n",
             tbf->tlli.GetTLLI(), multislotClass, delayClass, peakThroughputClass);
     }

    /*=========================================================================
    * Set maximum number of time slots based on multislot class
    *=========================================================================*/
    maxDlSlots = multiMaxDlSlots[multislotClass-1];
    maxUlSlots = multiMaxUlSlots[multislotClass-1];

	/*=========================================================================
    * Compute multislot options based on user capability
    *=========================================================================*/
    int mclass = multislotClass-1;
    int tindex;         // multislot table index
    int cindex;         // configuration index
    int ts;             // time slot 
    MultiSet* multiSet; // convenient struct for accessing multislot params
    /*------------------------------------------------------------
    * Determine number of multislot configurations
    *------------------------------------------------------------*/
    nbrOfCandConfigs = 0;
    for ( tindex=0; tindex<nbrMultiConfig[mclass]; tindex++ )
    {
        multiSet = (MultiSet*)multiConfig[mclass][tindex];
        nbrOfCandConfigs += 1 + multiSet->rangeEndDL - multiSet->rangeStartDL;
    }
    if ( nbrOfCandConfigs > MAX_MULTI_CONFIG_CANDIDATES )
    {
        nbrOfCandConfigs = MAX_MULTI_CONFIG_CANDIDATES;
        DBG_ERROR("  Error: Too many candidate multislot configurations, %d\n", nbrOfCandConfigs);
    }

    /*------------------------------------------------------------
    * Initialize multislot configurations
    *------------------------------------------------------------*/
    for ( cindex=0; cindex<nbrOfCandConfigs; cindex++ )
    {
        candMultiConfig[cindex].valid = FALSE;
        for ( ts=0; ts<8; ts++ )
        {
            candMultiConfig[cindex].allowedTsDL[ts] = FALSE;
            candMultiConfig[cindex].allowedTsUL[ts] = FALSE;
        }
    }
    /*------------------------------------------------------------
    * Compute multislot configurations for this multislot class
    *------------------------------------------------------------*/
    cindex = 0;
    for ( tindex=0; tindex<nbrMultiConfig[mclass]; tindex++ )
    {
        multiSet = (MultiSet*)multiConfig[mclass][tindex];
        int offDL;  // downlink offset
        int offUL = multiSet->slotOffsetUL;
        int porderDL = multiSet->patternOrderDL-1;
        int pindexDL = multiSet->patternIndexDL-1;
        int porderUL = multiSet->patternOrderUL-1;
        int pindexUL = multiSet->patternIndexUL-1;
        int allowedTs;
        /*--------------------------------------------------------
        * Loop over all starting DL time slots
        *--------------------------------------------------------*/
        for ( offDL = multiSet->rangeStartDL;
              offDL <= multiSet->rangeEndDL; offDL++ )
        {
            candMultiConfig[cindex].valid = TRUE;
            /*----------------------------------------------------
            * Enter DL pattern. Note porderDL is zero-based.
            *----------------------------------------------------*/
            for ( ts=0; ts<=porderDL; ts++ )
            {
                allowedTs = offDL + tsPattern[porderDL][pindexDL][ts];
                candMultiConfig[cindex].allowedTsDL[allowedTs] = TRUE;
            }
            /*----------------------------------------------------
            * Enter UL pattern. Note porderUL is zero-based.
            *----------------------------------------------------*/
            for ( ts=0; ts<=porderUL; ts++ )
            {
                allowedTs = offDL + offUL + tsPattern[porderUL][pindexUL][ts];
                candMultiConfig[cindex].allowedTsUL[allowedTs] = TRUE;
            }
            /*----------------------------------------------------
            * Done setting this configuration. Increment to next.
            *----------------------------------------------------*/
            cindex += 1;
        }
    }
    DBG_TRACE("  Total of %d multislot configurations\n", cindex);
    DBG_LEAVE();
    return(TRUE);
}
/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleAllocationInfo::ComputeDesiredNumberOfSlots  
**
**    PURPOSE: Compute the number of UL or DL time slots to allocate this user
**             based on user's multislot capability, but constrained by the
**             user's peak throughput class.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): desired number of slots
**
**----------------------------------------------------------------------------*/
int ScheduleAllocationInfo::ComputeDesiredNumberOfSlots(void) const
{
    int desiredSlots = (direction==DL) ? maxDlSlots : maxUlSlots;\
    int testClass = min(peakThroughputClass, 5);
    switch(testClass)
    {
    case 5:
        desiredSlots = min(desiredSlots, 8);
        break;
    case 4:
        desiredSlots = min(desiredSlots, 4);
        break;
    case 3:
        desiredSlots = min(desiredSlots, 2);
        break;
   default:
        desiredSlots = 1;
        break;
    }
    return(desiredSlots);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleAllocationInfo::ConstrainCandMultiConfig  
**
**    PURPOSE: Exclude candidate multislot configurations that contain time
**             slots that have been disallowed already for this TBF and those
**             that do not include time slots already allocated to the TBF.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ScheduleAllocationInfo::ConstrainCandMultiConfig(void)
{
    int cindex;
    int ts;
    
    /*-------------------------------------------------------------------------
    * Mark invalid all multislot configurations that: (1) include a TS that has
    * been disallowed already for this TBF, or (2) do not include all of the TS
    * already allocated to the TBF up to this point.
    *-------------------------------------------------------------------------*/
    for ( cindex=0; cindex<nbrOfCandConfigs; cindex++ )
    {
        for ( ts=0; candMultiConfig[cindex].valid && ts<MAX_TIMESLOTS; ts++ )
        {
            if ( (allocatedTsDL[ts] && !candMultiConfig[cindex].allowedTsDL[ts]) ||
                 (allocatedTsUL[ts] && !candMultiConfig[cindex].allowedTsUL[ts]) )
            {
                candMultiConfig[cindex].valid = FALSE;
            }
        }
    }
    /*-------------------------------------------------------------------------
    * Initialize list of potential natural timeslots in the TBF structure
    *-------------------------------------------------------------------------*/
    for ( ts=0; ts<MAX_TIMESLOTS; ts++ )
    {
        tbf->naturalTs[ts] = FALSE; 
    }
    /*-------------------------------------------------------------------------
    * Use all surviving multislot configurations to update the list of
    * potential natural timeslots in the TBF structure
    *-------------------------------------------------------------------------*/
    for ( cindex=0; cindex<nbrOfCandConfigs; cindex++ )
    {
        if ( candMultiConfig[cindex].valid )
        {
            for ( ts=0; ts<MAX_TIMESLOTS; ts++ )
            {
                if ( candMultiConfig[cindex].allowedTsDL[ts] &&
                     candMultiConfig[cindex].allowedTsUL[ts] )
                {
                    tbf->naturalTs[ts] = TRUE; 
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleAllocationInfo::IsAllowedTs  
**
**    PURPOSE: Check whether time slot is allowed by any multislot configuration
**
**    INPUT PARAMETERS:
**                      dir - link direction (DL or UL)
**                      ts  - time slot index (0..MAX_TIMESLOTS-1) 
**
**    RETURN VALUE(S): Whether slot is allowed (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool ScheduleAllocationInfo::IsAllowedTs(LinkDirection dir, int ts)
{
    bool status = FALSE;

    for ( int cindex=0; cindex<nbrOfCandConfigs && !status ; cindex++ )
    {
        if ( candMultiConfig[cindex].valid &&
             ( (dir==DL && candMultiConfig[cindex].allowedTsDL[ts]) ||
               (dir==UL && candMultiConfig[cindex].allowedTsUL[ts]) ) )
        { 
            status = TRUE;
        }
    }
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleAllocationInfo::IncludesNaturalTs  
**
**    PURPOSE: Check whether (a) this time slot is a natural slot or (b) the
**             TBF already includes a natural time slot.
**
**    INPUT PARAMETERS:
**                      dir    - link direction (DL or UL)
**                      newTs  - time slot index (0..MAX_TIMESLOTS-1) 
**
**    RETURN VALUE(S): Whether slot is allowed (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool ScheduleAllocationInfo::IncludesNaturalTs(LinkDirection dir, int newTs)
{
    bool status = FALSE;

    /*-------------------------------------------------------------------------
    * If the input slot is a natural time slot, let it be allocated
    *-------------------------------------------------------------------------*/
    if ( tbf->naturalTs[newTs] )
    {
        status = TRUE;
    }
    /*-------------------------------------------------------------------------
    * If the TBF has already been allocated a natural time slot, then allow
    * any other valid time slot be allocated
    *-------------------------------------------------------------------------*/
    for ( int ts=0; ts<MAX_TIMESLOTS && !status; ts++ )
    {
        if ( tbf->naturalTs[ts] && ((dir==DL && allocatedTsDL[ts]) ||
                                    (dir==UL && allocatedTsUL[ts])) )
        {
            status = TRUE;
        } 
    }
    
    return(status);
}


