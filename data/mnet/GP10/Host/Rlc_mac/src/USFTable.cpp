// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : USFTable.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "USFTable.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"


USFTable::USFTable()
{
    // Initialize USF entries to point to zero.
    for (int i=0; i < MAX_USF_PER_TS; i++)
    {
        pUSFEntry[i] = 0;
    }
}


unsigned char USFTable::AllocateUSF(ULTbf *pTbf)
{
    DBG_FUNC("USFTable::AllocateUSF", RLC_MAC);
    DBG_ENTER();
    
    // Search through the list of USF entries.  If an entry is zero then
    // it is free.  USF 0 is reserved as a dummy USF.
    for (int i=1; i < MAX_USF_PER_TS; i++)
    {
        if (pUSFEntry[i] == 0)
        {
            pUSFEntry[i] = pTbf;
            DBG_LEAVE();
            return (i);
        }
    }
    
    DBG_LEAVE();
    return (INVALID_USF);
}


void USFTable::ReleaseUSF(unsigned char usf)
{
    DBG_FUNC("USFTable::ReleaseUSF", RLC_MAC);
    DBG_ENTER();
    if (usf < MAX_USF_PER_TS)
    {
        pUSFEntry[usf] = 0;
    }
    
    DBG_LEAVE();
}


void USFTable::ShowUSFTable()
{
    for (int i=0; i < MAX_USF_PER_TS; i++)
    {
        printf("USF %2d : TBF - %x\n", i, pUSFEntry[i]);
    }
}
