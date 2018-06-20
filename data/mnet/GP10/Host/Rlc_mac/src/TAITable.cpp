// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : TAITable.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "TAITable.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"


TAITable::TAITable()
{
    // Initialize TAI entries to point to zero.
    for (int i=0; i < MAX_TAI_PER_TS; i++)
    {
        pTAIEntry[i] = 0;
    }
}


unsigned char TAITable::AllocateTAI(Tbf *pTbf)
{
    DBG_FUNC("TAITable::AllocateTAI", RLC_MAC);
    DBG_ENTER();
    
    // Search through the list of TAI entries.  If an entry is zero then
    // it is free.
    for (int i=0; i < MAX_TAI_PER_TS; i++)
    {
        if (pTAIEntry[i] == 0)
        {
            pTAIEntry[i] = pTbf;
            DBG_LEAVE();
            return (i);
        }
    }
    
    DBG_LEAVE();
    return (INVALID_TAI);
}


void TAITable::ReleaseTAI(unsigned char tai)
{
    DBG_FUNC("TAITable::ReleaseTAI", RLC_MAC);
    DBG_ENTER();
    
    if (tai < MAX_TAI_PER_TS)
    {
        pTAIEntry[tai] = 0;
    }
    
    DBG_LEAVE();
}


void TAITable::ShowTAITable()
{
    for (int i=0; i < MAX_TAI_PER_TS; i++)
    {
        printf("TAI %2d : TBF - %x\n", i, pTAIEntry[i]);
    }
}
