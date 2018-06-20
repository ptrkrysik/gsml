// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ScheduleInfo.h
// Author(s)   : Daryl Kaiser
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************

#ifndef __SCHEDULEINFO_H__
#define __SCHEDULEINFO_H__

#include "RlcMacCommon.h"
#include "RlcMacResult.h"
#include "ScheduleQueue.h"

// *******************************************************************
// class ScheduleInfo
//
// Description
//    
// *******************************************************************
class ScheduleInfo {

public:

    ScheduleInfo() : scheduleQueueDL(DL), scheduleQueueUL(UL)
    {
        nbrUsersDL = 0;
        nbrUsersUL = 0;
        utilizedCapacityDL = 0.0;
        utilizedCapacityUL = 0.0;
    };

    ~ScheduleInfo() {}
    
    enum { MAX_USERS_PER_SLOT = 16 };
    enum { MAX_USERS_PER_SLOT_UL = 7 };


    int GetNbrUsers(LinkDirection dir) const
    {
        if ( dir == DL )
        {
            return (nbrUsersDL);
        }
        else
        {
            return (nbrUsersUL);
        }
    }
    float GetUtilizedCapacity(LinkDirection dir) const
    {
        if ( dir == DL )
        {
            return (utilizedCapacityDL);
        }
        else
        {
            return (utilizedCapacityUL);
        }
        return (utilizedCapacityUL);
    }
    ScheduleQueue* GetQueue(LinkDirection dir) 
    {
        if ( dir == DL )
        {
            return (&scheduleQueueDL);
        }
        else
        {
            return (&scheduleQueueUL);
        }
    }
    bool OkayToAddUser(LinkDirection dir) const;
    void UpdateUsersAndCapacity(LinkDirection dir);
    int  GetNbrGprsTs(void) const;

private:

    int             nbrUsersDL;
    int             nbrUsersUL;
    float           utilizedCapacityDL;
    float           utilizedCapacityUL;
    ScheduleQueue   scheduleQueueDL;
    ScheduleQueue   scheduleQueueUL;   
};


#endif
