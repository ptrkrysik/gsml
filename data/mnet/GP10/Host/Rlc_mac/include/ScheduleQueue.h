// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ScheduleQueue.h
// Author(s)   : Daryl Kaiser
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************

#ifndef __SCHEDULEQUEUE_H__
#define __SCHEDULEQUEUE_H__

#include "RlcMacCommon.h"
#include "Tbf.h"

#define NULL_TBF (Tbf*) 0

// *******************************************************************
// class ScheduleQueue
//
// Description
//    
// *******************************************************************
class ScheduleQueue {

public:

    ScheduleQueue(LinkDirection);
    ~ScheduleQueue() {}
    
    enum { NULL_IMPORTANCE  = 9999 };
    enum { QUEUE_NUM_LEVELS = 4 };
    enum { QUEUE_MAX_SIZE = 12 };

    const LinkDirection direction;
    static const int nbrEntries[QUEUE_NUM_LEVELS];
    static const float capacityPerEntry[QUEUE_NUM_LEVELS];

    int   FindAvailableEntries (Tbf *pTbf, int desiredLevel, int precedence, bool tryOtherLevels);
    int   AllocateEntries (Tbf *pTbf, int desiredLevel, int precedence, bool tryOtherLevels);
    int   DeallocateEntries (Tbf *pTbf, int level);
    void  BalanceEntries (int level);
    bool  ScheduleNextBlock (Tbf **ppTbf);
    void  GetUsersAndCapacity(int *nbrUsers, float *utilizedCapacity); 
    void  CopyQueue(void);
    void  RestoreQueue(void);

private:

    struct Entry
    {
        Tbf *userTbf;
        int precedence;
        int entryNumber;
        int importance;
    };
    int     currentIndex[QUEUE_NUM_LEVELS];
    Entry   queue[QUEUE_NUM_LEVELS][QUEUE_MAX_SIZE];
    Entry   savedQueue[QUEUE_NUM_LEVELS][QUEUE_MAX_SIZE]; // a temp copy

    void Rank(int level, int *rankIndex, int *rankImportance, Tbf **rankTbf);
    bool AllocateOneEntry (int level, int index, Tbf *pTbf,
                           int precedence, int entryNumber);
    void SetEntry (int level, int index, Tbf *pTbf, int precedence,
                   int entryNumber, int importance);
    Tbf* GetEntryTbf (int level, int index) const;
    Tbf* GetCurrentEntryTbf (int level) const;
    int  GetAllTbf(int level, Tbf **ppTbf, int *precedence, int *highestEntry);
    int  GetEntryImportance (int level, int index) const;
    void ResetCurrentIndex (int level);
    bool IncrementCurrentIndex (int level);
};

#endif

