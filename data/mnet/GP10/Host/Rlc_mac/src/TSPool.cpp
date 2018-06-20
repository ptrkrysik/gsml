// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : TSPool.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "TSPool.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"



TSPool::TSPool()
{
    for (int i=0; i < MAX_TRX; i++)
    {
        // Initialize TFI table to be all unallocated.
        for (int k=0; k < MAX_TFI; k++)
        {
            sysUlTFI[i][k] = 0;
            sysDlTFI[i][k] = 0;
        }
    }
}

//----------------------------------------------------------------------------
// TAI functions
//----------------------------------------------------------------------------
unsigned char TSPool::AllocateTAI(int trx, int ts, Tbf *pTbf)
{
    DBG_FUNC("TSPool::AllocateTAI", RLC_MAC);
    DBG_ENTER();
    
    unsigned char tai;
    
    if (IsTrxTsValid(trx, ts))
    {
        tai = sysTS[trx][ts].AllocateTAI(pTbf);
        pTbf->tai = tai;
        pTbf->taiTs = ts;
    }
    else
    {
        DBG_WARNING("TSPool::AllocateTAI: Invalid trx (%d) or ts (%d)\n", trx, ts);
        tai = INVALID_TAI;
    }
    
    DBG_LEAVE();
    return (tai);
}


void TSPool::ReleaseTAI(int trx, int ts, unsigned char tai)
{
    DBG_FUNC("TSPool::ReleaseTAI", RLC_MAC);
    DBG_ENTER();
    
    if (IsTrxTsValid(trx, ts))
    {
        sysTS[trx][ts].ReleaseTAI(tai);
    }
    else
    {
        DBG_WARNING("TSPool::ReleaseTAI: Invalid trx (%d) or ts (%d)\n", trx, ts);
    }
    
    DBG_LEAVE();
}

void TSPool::ShowTAITable(int trx)
{
    if (IsTrxValid(trx))
    {
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            printf("TAI Table for Trx(%d) Timeslot(%d)\n", trx, i);
            sysTS[trx][i].ShowTAITable();
        }
    }
    else
    {
        printf("Invalid trx (%d)\n", trx);
    }
}

void TSPool::ShowTAITable(int trx, int ts)
{
    if (IsTrxTsValid(trx, ts))
    {
        printf("TAI Table for Trx(%d) Timeslot(%d)\n", trx, ts);
        sysTS[trx][ts].ShowTAITable();
    }
    else
    {
        printf("Invalid trx (%d) or ts(%d)\n", trx, ts);
    }
}



//----------------------------------------------------------------------------
// USF functions
//----------------------------------------------------------------------------
unsigned char TSPool::AllocateUSF(int trx, int ts, ULTbf *pTbf)
{
    DBG_FUNC("TSPool::AllocateUSF", RLC_MAC);
    DBG_ENTER();
    
    unsigned char usf;
    
    if (IsTrxTsValid(trx, ts))
    {
        usf = sysTS[trx][ts].AllocateUSF(pTbf);
        pTbf->usf[ts] = usf;
    }
    else
    {
        DBG_WARNING("TSPool::AllocateTUSF: Invalid trx (%d) or ts (%d)\n", trx, ts);
        usf = INVALID_USF;
    }
    
    DBG_LEAVE();
    return (usf);
}


void TSPool::ReleaseUSF(int trx, int ts, unsigned char usf)
{
    DBG_FUNC("TSPool::ReleaseUSF", RLC_MAC);
    DBG_ENTER();
    
    if (IsTrxTsValid(trx, ts))
    {
        sysTS[trx][ts].ReleaseUSF(usf);
    }
    else
    {
        DBG_WARNING("TSPool::ReleaseUSF: Invalid trx (%d) or ts (%d)\n", trx, ts);
    }
    
    DBG_LEAVE();
}

void TSPool::ShowUSFTable(int trx)
{
    if (IsTrxValid(trx))
    {
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            printf("USF Table for Trx(%d) Timeslot(%d)\n", trx, i);
            sysTS[trx][i].ShowUSFTable();
        }
    }
    else
    {
        printf("Invalid trx (%d)\n", trx);
    }
}

void TSPool::ShowUSFTable(int trx, int ts)
{
    if (IsTrxTsValid(trx, ts))
    {
        printf("USF Table for Trx(%d) Timeslot(%d)\n", trx, ts);
        sysTS[trx][ts].ShowUSFTable();
    }
    else
    {
        printf("Invalid trx (%d) or ts(%d)\n", trx, ts);
    }
}




//----------------------------------------------------------------------------
// TFI functions
//----------------------------------------------------------------------------
unsigned char TSPool::AllocateULTFI(int trx, ULTbf *pUlTbf)
{
    DBG_FUNC("TSPool::AllocateULTFI", RLC_MAC);
    DBG_ENTER();
    
    unsigned char tfi;
    
    if (IsTrxValid(trx))
    {
        for (tfi = 0; tfi < MAX_TFI; tfi++)
        {
            if (!sysUlTFI[trx][tfi])
            {
                sysUlTFI[trx][tfi] = pUlTbf;
                pUlTbf->tfi.SetTFI(tfi);
                break;
            }
        }
    }
    else
    {
        DBG_WARNING("TSPool::AllocateULTFI: Invalid trx (%d)\n", trx);
        tfi = INVALID_TFI;
    }
    
    DBG_LEAVE();
    return (tfi);
}
 
void TSPool::ReleaseULTFI(int trx, unsigned char tfi)
{
    DBG_FUNC("TSPool::ReleaseULTFI", RLC_MAC);
    DBG_ENTER();
    
    if (IsTrxValid(trx) && IsTfiValid(tfi))
    {
        sysUlTFI[trx][tfi] = 0;
    }
    else
    {
        DBG_WARNING("TSPool::ReleaseULTFI: Invalid trx (%d) or tfi (%d)\n", trx, tfi);
    }
    
    DBG_LEAVE();
}
 
ULTbf *TSPool::FindULTbf(int trx, unsigned char tfi) 
{
    DBG_FUNC("TSPool::FindULTbf", RLC_MAC);
    DBG_ENTER();
    
    if (IsTrxValid(trx) && (tfi < MAX_TFI))
    {
        return (sysUlTFI[trx][tfi]);
    }
    else
    {
        return (0);
    }
    DBG_LEAVE();
}
 
void TSPool::ShowULTFITable(int trx)
{
    if (IsTrxValid(trx))
    {
        printf("Uplink TFI Table for Trx(%d)\n", trx);
        for (int i = 0; i < MAX_TFI; i++)
        {
            printf("TFI %d = %#x\n", i, sysUlTFI[trx][i]);
        }
    }
    else
    {
        printf("Invalid trx (%d)\n", trx);
    }
}


unsigned char TSPool::AllocateDLTFI(int trx, DLTbf *pDlTbf)
{
    DBG_FUNC("TSPool::AllocateDLTFI", RLC_MAC);
    DBG_ENTER();
    
    unsigned char tfi;
    
    if (IsTrxValid(trx))
    {
        for (tfi = 0; tfi < MAX_TFI; tfi++)
        {
            if (!sysDlTFI[trx][tfi])
            {
                sysDlTFI[trx][tfi] = pDlTbf;
                pDlTbf->tfi.SetTFI(tfi);
                break;
            }
        }
    }
    else
    {
        DBG_WARNING("TSPool::AllocateDLTFI: Invalid trx (%d)\n", trx);
        tfi = INVALID_TFI;
    }
    
    DBG_LEAVE();
    return (tfi);
}
 
void TSPool::ReleaseDLTFI(int trx, unsigned char tfi)
{
    DBG_FUNC("TSPool::ReleaseDLTFI", RLC_MAC);
    DBG_ENTER();
    
    if (IsTrxValid(trx) && IsTfiValid(tfi))
    {
        sysDlTFI[trx][tfi] = 0;
    }
    else
    {
        DBG_WARNING("TSPool::ReleaseDLTFI: Invalid trx (%d) or tfi (%d)\n", trx, tfi);
    }
    
    DBG_LEAVE();
}
 
DLTbf *TSPool::FindDLTbf(int trx, unsigned char tfi) 
{
    DBG_FUNC("TSPool::FindDLTbf", RLC_MAC);
    DBG_ENTER();
    
    if (IsTrxValid(trx) && (tfi < MAX_TFI))
    {
        return (sysDlTFI[trx][tfi]);
    }
    else
    {
        return (0);
    }
    DBG_LEAVE();
}
 
void TSPool::ShowDLTFITable(int trx)
{
    if (IsTrxValid(trx))
    {
        printf("Downlink TFI Table for Trx(%d)\n", trx);
        for (int i = 0; i < MAX_TFI; i++)
        {
            printf("TFI %d = %#x\n", i, sysDlTFI[trx][i]);
        }
    }
    else
    {
        printf("Invalid trx (%d)\n", trx);
    }
}

//----------------------------------------------------------------------------
// DL signalling queue functions
//----------------------------------------------------------------------------
JC_STATUS TSPool::QueueDLSignallingMsg
(
    int trx, 
    int ts, 
    MsgPacchReq *l1Msg    
)
{
    DBG_FUNC("TSPool::QueueDLSignallingMsg", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_ERROR;
    
    if (IsTrxTsValid(trx, ts))
    {
        status = sysTS[trx][ts].QueueDLSignallingMsg(l1Msg);
    }
    else
    {
        DBG_WARNING("TSPool::QueueDLSignallingMsg: Invalid trx (%d) or ts (%d)\n", trx, ts);
    }
    
    DBG_LEAVE();
    return(status);
}


MsgPacchReq *TSPool::DequeueDLSignallingMsg
(
    int trx, 
    int ts 
)
{
    DBG_FUNC("TSPool::DequeueDLSignallingMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgPacchReq *pPacchMsg;
    
    if (IsTrxTsValid(trx, ts))
    {
        pPacchMsg = sysTS[trx][ts].DequeueDLSignallingMsg();
    }
    else
    {
        DBG_WARNING("TSPool::DequeueDLSignallingMsg: Invalid trx (%d) or ts (%d)\n", trx, ts);
        pPacchMsg = 0;
    }
    
    DBG_LEAVE();
    return(pPacchMsg);
}



//----------------------------------------------------------------------------
// Channel Combination functions
//----------------------------------------------------------------------------

void TSPool::ShowChannelComb(int trx)
{
    if (IsTrxValid(trx))
    {
        printf("Channel Combinations for Trx(%d)\n", trx);
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            printf("\tTimeslot(%d) = %d\n", i, sysTS[trx][i].GetChannelComb());
        }
    }
    else
    {
        printf("Invalid trx (%d)\n", trx);
    }
}

void TSPool::ShowChannelComb(int trx, int ts)
{
    if (IsTrxTsValid(trx, ts))
    {
        printf("Channel Combination for Trx(%d) Timeslot(%d) = %d\n", trx, ts, 
            sysTS[trx][ts].GetChannelComb());
    }
    else
    {
        printf("Invalid trx (%d) or ts(%d)\n", trx, ts);
    }
}


//----------------------------------------------------------------------------
// General Timeslot functions
//----------------------------------------------------------------------------

void TSPool::ShowTimeslotInfo(int trx)
{
    if (IsTrxValid(trx))
    {
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            printf("Trx (%d) Timeslot (%d) Information\n", trx, i);
            sysTS[trx][i].ShowTimeslotInfo();
        }
    }
    else
    {
        printf("Invalid trx (%d)\n", trx);
    }
}


void TSPool::ShowTimeslotInfo(int trx, int ts)
{
    if (IsTrxTsValid(trx, ts))
    {
        printf("Trx (%d) Timeslot (%d) Information\n", trx, ts);
        sysTS[trx][ts].ShowTimeslotInfo();
    }
    else
    {
        printf("Invalid trx (%d) or ts(%d)\n", trx, ts);
    }
}
