// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ScheduleInfo.cpp
// Author(s)   : Daryl Kaiser
// Create Date : 1/16/2001
// Description : 
//
// *******************************************************************

#include "ScheduleInfo.h"
#include "grr/grr_intf.h" 
#include "vipermib.h"

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleInfo::OkayToAddUser
**
**----------------------------------------------------------------------------*/
bool ScheduleInfo::OkayToAddUser(LinkDirection dir) const
{
    if ( (nbrUsersUL + nbrUsersDL) < MAX_USERS_PER_SLOT  &&
           ( (dir==UL && (nbrUsersUL < MAX_USERS_PER_SLOT_UL) ) || 
             (dir==DL) ) )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ScheduleInfo::UpdateUsersAndCapacity
**
**----------------------------------------------------------------------------*/
void ScheduleInfo::UpdateUsersAndCapacity(LinkDirection dir) 
{
    if ( dir == DL )
    {
        scheduleQueueDL.GetUsersAndCapacity(&nbrUsersDL, &utilizedCapacityDL);
    }
    else
    {
        scheduleQueueUL.GetUsersAndCapacity(&nbrUsersUL, &utilizedCapacityUL);
    }
}

