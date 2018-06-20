// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ScheduleQueue.cpp
// Author(s)   : Daryl Kaiser
// Create Date : 12/22/2000
// Description : 
//
// *******************************************************************

#include "ScheduleQueue.h"

const int ScheduleQueue::nbrEntries[QUEUE_NUM_LEVELS] = {7,9,4,12};
const float ScheduleQueue::capacityPerEntry[QUEUE_NUM_LEVELS] =
			{float(0.125),float(0.0125),float(0.0025),float(0.0002)};

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::ScheduleQueue
**
**    PURPOSE: Constructor. 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
ScheduleQueue::ScheduleQueue(LinkDirection dir) : direction(dir)
{
    int level;
    int index;
    /*-------------------------------------------------------------------------
    * Initialize master scheduler queue entries at all levels. Indices, too.
    *-------------------------------------------------------------------------*/
    for ( level=0; level<QUEUE_NUM_LEVELS; level++ )
    {
        for ( index=0; index<QUEUE_MAX_SIZE; index++ )
        {
            currentIndex[level] = -1;
            queue[level][index].userTbf = NULL_TBF;
            queue[level][index].precedence = 0;
            queue[level][index].entryNumber = 0;
            queue[level][index].importance = NULL_IMPORTANCE;
        }
    }
    /*-------------------------------------------------------------------------
    * Initialize temporary duplicate queue entries at all levels
    *-------------------------------------------------------------------------*/
    for ( level=0; level<QUEUE_NUM_LEVELS; level++ )
    {
        for ( index=0; index<QUEUE_MAX_SIZE; index++ )
        {
            savedQueue[level][index].userTbf = NULL_TBF;
            savedQueue[level][index].precedence = 0;
            savedQueue[level][index].entryNumber = 0;
            savedQueue[level][index].importance = NULL_IMPORTANCE;
        }
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::FindAvailableEntries
**
**    PURPOSE: Find the number of schedule queue entries available to user.
**             Optionally go out of requested delay class (queue level).
**
**    INPUT PARAMETERS: 
**                      desiredLevel    - queue level (0..QUEUE_NUM_LEVELS-1)
**                      precedence      - precedencd class (1-3)
**                      tryOtherLevels  - try other than desired level (T/F)
**                      
**    RETURN VALUE(S): number of available queue entries
**
**----------------------------------------------------------------------------*/
int ScheduleQueue::FindAvailableEntries (Tbf* pTbf,
                                         int desiredLevel,
                                         int precedence,
                                         bool tryOtherLevels)
{
    int     rankIndex[QUEUE_MAX_SIZE];
    int     rankImportance[QUEUE_MAX_SIZE];
    Tbf*    rankTbf[QUEUE_MAX_SIZE];
    int     availableEntries = 0;
    int     entryNumber;

    /*-------------------------------------------------------------------------
    * Rank importance values for the desired queue level from worst to best
    *-------------------------------------------------------------------------*/
    Rank(desiredLevel, rankIndex, rankImportance, rankTbf);

    /*-------------------------------------------------------------------------
    * Find # of queue entries that new user would get
    *-------------------------------------------------------------------------*/
    int newImportance;
    bool isAvailable = TRUE;
    for ( entryNumber=0;
          isAvailable && entryNumber<nbrEntries[desiredLevel];
          entryNumber++ )
    {	
	    newImportance = (entryNumber==0) ? precedence : 10*precedence + entryNumber; 
	    if ( newImportance >= rankImportance[entryNumber] )
	    {
            isAvailable = FALSE;
        }
        else if ( pTbf != rankTbf[entryNumber] )
        {
            availableEntries += 1;
        }
    }
    return(availableEntries);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::AllocateEntries
**
**    PURPOSE: Allocate schedule queue entries to user.
**             Optionally go out of requested delay class (queue level).
**
**    INPUT PARAMETERS: 
**                      pTbf            - ptr to DL or UL TBF structure
**                      desiredLevel    - queue level (0..QUEUE_NUM_LEVELS-1)
**                      precedence      - precedence class (1-3)
**                      tryOtherLevels  - try other than desired level (T/F)
**                      
**    RETURN VALUE(S): number of allocated queue entries
**
**----------------------------------------------------------------------------*/
int ScheduleQueue::AllocateEntries (Tbf *pTbf,
                                    int desiredLevel,
                                    int precedence,
                                    bool tryOtherLevels)
{
    int     rankIndex[QUEUE_MAX_SIZE];
    int     rankImportance[QUEUE_MAX_SIZE];
    Tbf*    rankTbf[QUEUE_MAX_SIZE];
    int     entryNumber;

    /*-------------------------------------------------------------------------
    * Rank importance values for the desired queue level from worst to best
    *-------------------------------------------------------------------------*/
    Rank(desiredLevel, rankIndex, rankImportance, rankTbf);

    /*-------------------------------------------------------------------------
    * Allocate as many queue entries as user is entitled to receive
    *-------------------------------------------------------------------------*/
    bool madeAllocation = TRUE;
    for ( entryNumber=0;
          madeAllocation && entryNumber<nbrEntries[desiredLevel];
          entryNumber++ )
    {
        madeAllocation = AllocateOneEntry(desiredLevel, rankIndex[entryNumber], pTbf,
                             precedence, entryNumber);
    }
    return(entryNumber-1);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::AllocateOneEntry
**
**    PURPOSE: Compute importance of user's next entry. If it beats the 
**             importance of the existing queue entry, overwrite it.
**
**    INPUT PARAMETERS: 
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      index           - queue index (0..QUEUE_MAX_SIZE-1)
**                      pTbf            - ptr to DL or UL TBF structure
**                      precedence      - new user's precedence class (1-3)
**                      entryNumber     - new user's entry number (1st, 2nd,..)
**                      
**    RETURN VALUE(S): whether new entry was allocated (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool ScheduleQueue::AllocateOneEntry (int level,
                                      int index,
                                      Tbf *pTbf,
                                      int precedence,
                                      int entryNumber)
{
	int newImportance = (entryNumber==0) ? precedence : 10*precedence + entryNumber; 
	if ( newImportance < queue[level][index].importance )
	{
        SetEntry(level, index, pTbf, precedence,  entryNumber, newImportance);
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::DellocateEntries
**
**    PURPOSE: Dellocate schedule queue entries from user and rebalance
**             remaining entries in the queue.
**
**    INPUT PARAMETERS: 
**                      pTbf            - ptr to DL or UL TBF structure
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      
**    RETURN VALUE(S): number of queue entries deallocated
**
**----------------------------------------------------------------------------*/
int ScheduleQueue::DeallocateEntries (Tbf *pTbf, int level)
{
    /*-------------------------------------------------------------------------
    * Deallocate all entries for this user
    *-------------------------------------------------------------------------*/
    int entries = 0;
    for ( int index=0; index<QUEUE_MAX_SIZE; index++ )
    {
        if ( queue[level][index].userTbf == pTbf )
        {
            entries += 1;
            queue[level][index].userTbf = NULL_TBF;
            queue[level][index].precedence = 0;
            queue[level][index].entryNumber = 0;
            queue[level][index].importance = NULL_IMPORTANCE;
        }
    }
    return(entries);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::BalanceEntries
**
**    PURPOSE: Balance schedule queue entries based on precedence of all
**             remaining users.
**
**    INPUT PARAMETERS: 
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      
**    RETURN VALUE(S): number of allocated queue entries
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::BalanceEntries (int level)
{
    /*---------------------------------------------------------------------
    * Find all remaining users at this queue level
    *---------------------------------------------------------------------*/
    Tbf* remainingTbf[QUEUE_MAX_SIZE];
    int  remainingPrecedence[QUEUE_MAX_SIZE];
    int  highestEntry[QUEUE_MAX_SIZE];
    GetAllTbf(level, remainingTbf, remainingPrecedence, highestEntry);

    /*---------------------------------------------------------------------
    * For each remaining user, allocate as many entries as possible
    **** for now, ignore peak throughput
    *---------------------------------------------------------------------*/
    int entryNumber;
    for ( int user=0; user<QUEUE_MAX_SIZE; user++ )
    {
        if ( remainingTbf[user] != NULL_TBF )
        {
            /*-------------------------------------------------------------
            * Rank importance values for this queue from worst to best
            *-------------------------------------------------------------*/
            int  rankIndex[QUEUE_MAX_SIZE];
            int  rankImportance[QUEUE_MAX_SIZE];
            Tbf* rankTbf[QUEUE_MAX_SIZE];
            Rank(level, rankIndex, rankImportance, rankTbf);

            bool madeAllocation = TRUE;
            for ( entryNumber=0;
                  madeAllocation && entryNumber<QUEUE_MAX_SIZE;
                  entryNumber++ )
            {
                madeAllocation = AllocateOneEntry(level, rankIndex[entryNumber],
                                remainingTbf[user], remainingPrecedence[user],
                                entryNumber+highestEntry[user]+1);
            }
        }
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::GetEntryTbf
**
**    PURPOSE: Get TBF pointer of one queue entry
**
**    INPUT PARAMETERS: 
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      index           - queue index (0..QUEUE_MAX_SIZE-1)
**                      
**    RETURN VALUE(S): ptr to TBF
**
**----------------------------------------------------------------------------*/
Tbf* ScheduleQueue::GetEntryTbf (int level, int index) const
{
    return(queue[level][index].userTbf);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::GetCurrentEntryTbf
**
**    PURPOSE: Get TBF pointer of current queue entry (using currentIndex)
**
**    INPUT PARAMETERS: 
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      
**    RETURN VALUE(S): ptr to TBF
**
**----------------------------------------------------------------------------*/
Tbf* ScheduleQueue::GetCurrentEntryTbf (int level) const
{
    return(queue[level][currentIndex[level]].userTbf);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::GetAllTbf
**
**    PURPOSE: Compile a list of all TBFs in a particular queue. Also list
**             the precedence and highest allocated entry for the user.
**
**    INPUT PARAMETERS: 
**             level           - queue level (0..QUEUE_NUM_LEVELS-1)
**             tbf[]           - array of TBF pointers (size=QUEUE_MAX_SIZE)
**             precedence[]    - array of user precedence (same size)
**             highestEntry[]  - array of user's highest entry (ditto)
**                      
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int ScheduleQueue::GetAllTbf(int level,
                             Tbf **tbf,
                             int *precedence,
                             int *highestEntry)
{
    int i;
    int nbrRemainingUsers(0);

    /*---------------------------------------------------------------------
    * Initialize output to null entries
    *---------------------------------------------------------------------*/
    for ( i=0; i<QUEUE_MAX_SIZE; i++ )
    {
        tbf[i] = NULL_TBF;
    }
    /*---------------------------------------------------------------------
    * Search through all schedule queue entries looking for unique TBFs
    *---------------------------------------------------------------------*/
    for ( i=0; i<QUEUE_MAX_SIZE; i++ )
    {
        if ( queue[level][i].userTbf != NULL_TBF )
        {
            /*-------------------------------------------------------------
            * Check whether TBF is already in tbf[] list
            *-------------------------------------------------------------*/
            int foundTbf = FALSE;
            for ( int k=0; k<nbrRemainingUsers && !foundTbf; k++)
            {
                if ( tbf[k] == queue[level][i].userTbf )
                {
                    foundTbf = TRUE;
                    /*----------------------------------------------------
                    * It's in the list, now save highest entry
                    *----------------------------------------------------*/
                    if ( queue[level][i].entryNumber > highestEntry[k] )
                    {
                        highestEntry[k] = queue[level][i].entryNumber;
                    }
                }
            }
            /*-------------------------------------------------------------
            * If TBF is not yet in tbf[] list, enter it now
            *-------------------------------------------------------------*/
            if ( !foundTbf )
            {
                tbf[nbrRemainingUsers] = queue[level][i].userTbf;
                precedence[nbrRemainingUsers] = queue[level][i].precedence;
                highestEntry[nbrRemainingUsers] = queue[level][i].entryNumber;
                nbrRemainingUsers += 1;
                /*-------------------------------------------------------------
                * Check that each TBF is not in FREE state. If so, it's a bug.
                *-------------------------------------------------------------*/
                if ( queue[level][i].userTbf->GetTbfState() == Tbf::FREE )
                {
	                printf("Error found by ScheduleQueue::GetAllTbf: TBF is in FREE state\n");
                }
            }
        }
    }
    return(nbrRemainingUsers);
}

    /*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::GetEntryImportance
**
**    PURPOSE: Get importance value of one queue entry
**
**    INPUT PARAMETERS: 
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      index           - queue index (0..QUEUE_MAX_SIZE-1)
**                      
**    RETURN VALUE(S): importance value
**
**----------------------------------------------------------------------------*/
int ScheduleQueue::GetEntryImportance (int level, int index) const
{
    return(queue[level][index].importance);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::SetEntry
**
**    PURPOSE: Set TBF and importance of one queue entry
**
**    INPUT PARAMETERS: 
**                      level           - queue level (0..QUEUE_NUM_LEVELS-1)
**                      index           - queue index (0..QUEUE_MAX_SIZE-1)
**                      pTbf            - ptr to DL or UL TBF structure
**                      precedence      - precedence class (1-3)
**                      entryNumber     - entry for this user (1st, 2nd, etc)
**                      importance      - importance value
**                      
**    RETURN VALUE(S): number of available queue entries
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::SetEntry (int level, int index, Tbf *pTbf, int precedence,
                              int entryNumber, int importance)
{
    queue[level][index].userTbf = pTbf; 
    queue[level][index].precedence = precedence; 
    queue[level][index].entryNumber = entryNumber; 
    queue[level][index].importance = importance; 
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::ResetCurrentIndex
**
**    PURPOSE: Reset current index of one queue level
**
**    INPUT PARAMETERS: 
**                      level  - queue level (0..QUEUE_NUM_LEVELS-1)
**                      
**    RETURN VALUE(S): number of available queue entries
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::ResetCurrentIndex (int level)
{
    currentIndex[level] = -1; 
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::IncrementCurrentIndex
**
**    PURPOSE: Increment current index of one queue level
**
**    INPUT PARAMETERS: 
**                      level  - queue level (0..QUEUE_NUM_LEVELS-1)
**                      
**    RETURN VALUE(S): indicates whether index is still within queue bounds
**
**----------------------------------------------------------------------------*/
bool ScheduleQueue::IncrementCurrentIndex (int level)
{
    currentIndex[level] += 1;
    return (currentIndex[level] < nbrEntries[level]);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::Rank
**
**    PURPOSE: Rank one queue level by importance value from worst to best
**
**    INPUT:
**                      level   - queue level (0..QUEUE_NUM_LEVELS-1)
**    OUTPUT:
**                      rank[]    - array of importance values
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::Rank(int level, int *rankIndex, int *rankImportance, Tbf **rankTbf)
{
    int index;
    int localCopy[QUEUE_MAX_SIZE];

    /*-------------------------------------------------------------------------
    * Construct a local copy of importance values for the desired queue level
    *-------------------------------------------------------------------------*/
    for ( index=0; index<QUEUE_MAX_SIZE; index++ )
    {
        localCopy[index] = queue[level][index].importance;
    }
    /*-------------------------------------------------------------------------
    * In each of nbrEntries[level] passes, find and mark worst importance
    *-------------------------------------------------------------------------*/
    for ( int pass=0; pass<nbrEntries[level]; pass++)
    {
        rankImportance[pass] = 0;
        rankTbf[pass] = NULL_TBF;
        for ( index=0; index<nbrEntries[level]; index++ )
        {
            if ( localCopy[index] > rankImportance[pass] )
            {
                rankIndex[pass] = index;
                rankImportance[pass] = localCopy[index];
                rankTbf[pass] = queue[level][index].userTbf;  
            }
        }
        localCopy[rankIndex[pass]] = -1; // done with this entry
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::ScheduleNextBlock
**
**    PURPOSE: Schedule the next RLC block from the queues of one slot
**
**    INPUT: none
**
**    OUTPUT:   pTbf - ptr to scheduled UL or DL TBF structure
**
**    RETURN VALUE(S): indicates whether entry was assigned (T) or not (F)
**
**----------------------------------------------------------------------------*/
bool ScheduleQueue::ScheduleNextBlock (Tbf **ppTbf)
{
    DBG_FUNC("ScheduleQueue::ScheduleNextBlock", RLC_MAC_SCHEDULER);
    DBG_ENTER();

    bool empty[QUEUE_NUM_LEVELS];

    /*-------------------------------------------------------------------------
    * First, determine which queues contain valid TBFs and which are empty.
    * If the queue's first entry is a null TBF, then the entire queue is empty.
    *-------------------------------------------------------------------------*/
    for ( int level=0; level<QUEUE_NUM_LEVELS; level++)
    {
        if ( GetEntryTbf(level,0) == NULL_TBF ) empty[level] = TRUE;
        else                                    empty[level] = FALSE;
    }

    DBG_TRACE("  Empty list = %d,%d,%d,%d\n", empty[0],empty[1],empty[2],empty[3]);

    /*-------------------------------------------------------------------------
    * Take at most two passes through the queue hierarchy for this time slot
    * looking for a TBF in ALLOCATED state. When found, set valid TBF pointer.
    *-------------------------------------------------------------------------*/
    *ppTbf = NULL_TBF;
    for ( int pass=0; pass<2 && *ppTbf==NULL_TBF; pass++ )
    {
        /*-------------------------------------------------------------------------
        * Starting from lowest delay class queue, look for a non-null TBF to send
        *-------------------------------------------------------------------------*/
        for ( level=0; level<QUEUE_NUM_LEVELS && *ppTbf==NULL_TBF; level++)
        {
            /*-------------------------------------------------------------------------
            * If the first queue entry is null, they're all null. Skip the entire queue. 
            *-------------------------------------------------------------------------*/
            if ( !empty[level] )
            {
                /*-------------------------------------------------------------------------
                * Increment this queue's entry index until finding a TBF that is ALLOCATED.
                *-------------------------------------------------------------------------*/
                while ( *ppTbf==NULL_TBF && IncrementCurrentIndex(level) )
                {
                    *ppTbf = GetCurrentEntryTbf(level);
                    
    DBG_TRACE("  Pass=%d, Level=%d, index=%d, TLLI=0x%x, TbfState=%d\n",
                pass,level,currentIndex[level],(*ppTbf)->tlli.GetTLLI(),(*ppTbf)->GetTbfState());
                
                    if ( (*ppTbf)->GetTbfState() != Tbf::ALLOCATED )
                    {
                        *ppTbf = NULL_TBF;
                    }
                }
                /*-------------------------------------------------------------------------
                * If no TBF was found in ALLOCATED state, the entry index was left one
                * position beyond the queue end. Reset it and loop to next queue level.
                *-------------------------------------------------------------------------*/
                if ( *ppTbf==NULL_TBF )
                {
                    ResetCurrentIndex(level);
                }
            }
        }
    }
    DBG_TRACE("  Leaving with (*ppTbf)=%d\n", (*ppTbf));
    DBG_LEAVE();
    return (*ppTbf!=NULL_TBF);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::GetUsersAndCapacity
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::GetUsersAndCapacity(int *nbrUsers, float *utilizedCapacity) 
{
    /*---------------------------------------------------------------------
    * Find # of users at each queue level
    *---------------------------------------------------------------------*/
    Tbf* tbf[QUEUE_MAX_SIZE];
    int  precedence[QUEUE_MAX_SIZE];
    int  highestEntry[QUEUE_MAX_SIZE];
    int  usersAtLevel;
    *nbrUsers = 0;
    *utilizedCapacity = 0.0;
    for ( int level=0; level<QUEUE_NUM_LEVELS; level++ )
    {
        usersAtLevel = GetAllTbf(level, tbf, precedence, highestEntry);
        *nbrUsers += usersAtLevel;
        *utilizedCapacity += capacityPerEntry[level] * usersAtLevel;
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::CopyQueue
**
**    PURPOSE: Make exact copies of the current master schedule queues and
**             save them in the savedQueue temporary storage in this object
**
**    INPUT: none
**
**    OUTPUT:  none
**
**    RETURN VALUE(S): pointer to the copied queues
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::CopyQueue(void)
{
    int level;
    int index;
    /*-------------------------------------------------------------------------
    * Copy master scheduler queue entries into temporary savedQueue
    *-------------------------------------------------------------------------*/
    for ( level=0; level<QUEUE_NUM_LEVELS; level++ )
    {
        for ( index=0; index<QUEUE_MAX_SIZE; index++ )
        {
            savedQueue[level][index].userTbf = queue[level][index].userTbf;
            savedQueue[level][index].precedence = queue[level][index].precedence;
            savedQueue[level][index].entryNumber = queue[level][index].entryNumber;
            savedQueue[level][index].importance = queue[level][index].importance;
        }
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleQueue::RestoreQueue
**
**    PURPOSE: Restore the master schedule queues to their original states, 
**             as recorded in savedQueue temporary storage in this object 
**
**    INPUT: none
**
**    OUTPUT:  none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ScheduleQueue::RestoreQueue(void)
{
    int level;
    int index;
    /*-------------------------------------------------------------------------
    * Restore master scheduler queue entries using values in savedQueue
    *-------------------------------------------------------------------------*/
    for ( level=0; level<QUEUE_NUM_LEVELS; level++ )
    {
        for ( index=0; index<QUEUE_MAX_SIZE; index++ )
        {
            queue[level][index].userTbf = savedQueue[level][index].userTbf;
            queue[level][index].precedence = savedQueue[level][index].precedence;
            queue[level][index].entryNumber = savedQueue[level][index].entryNumber;
            queue[level][index].importance = savedQueue[level][index].importance;
        }
    }
}

