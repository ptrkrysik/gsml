// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : TbfPool.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "TbfPool.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

//--------------------------------------------------------------------------
// Functions related to administrative and op state changes
//--------------------------------------------------------------------------

void TbfPool::AbnormalReleaseBts()
{
    DBG_FUNC("TbfPool::AbnormalReleaseBts", RLC_MAC);
    DBG_ENTER();
    
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() != Tbf::FREE)
        {
            sysULTbf[i].AbnormalReleaseULTbf();
        }
        
        if (sysDLTbf[i].GetTbfState() != Tbf::FREE)
        {
            sysDLTbf[i].AbnormalReleaseDLTbf();
        }
    }
    
    DBG_LEAVE();
}


void TbfPool::AbnormalReleaseTrx(int trx)
{
    DBG_FUNC("TbfPool::AbnormalReleaseTrx", RLC_MAC);
    DBG_ENTER();
    
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if ((sysULTbf[i].GetTbfState() != Tbf::FREE) &&
            (sysULTbf[i].trx == trx))
        {
            sysULTbf[i].AbnormalReleaseULTbf();
        }
        
        if ((sysDLTbf[i].GetTbfState() != Tbf::FREE) &&
            (sysDLTbf[i].trx == trx))
        {
            sysDLTbf[i].AbnormalReleaseDLTbf();
        }
    }
    
    DBG_LEAVE();
}


void TbfPool::AbnormalReleaseTs(int trx, int ts)
{
    DBG_FUNC("TbfPool::AbnormalReleaseTs", RLC_MAC);
    DBG_ENTER();
    
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if ((sysULTbf[i].GetTbfState() != Tbf::FREE) &&
            (sysULTbf[i].trx == trx) && (sysULTbf[i].allocatedTs[ts] == TRUE))
        {
            sysULTbf[i].AbnormalReleaseULTbf();
        }
        
        if ((sysDLTbf[i].GetTbfState() != Tbf::FREE) &&
            (sysDLTbf[i].trx == trx) && (sysDLTbf[i].allocatedTs[ts] == TRUE))
        {
            sysDLTbf[i].AbnormalReleaseDLTbf();
        }
    }
    
    DBG_LEAVE();
}


//--------------------------------------------------------------------------
// Functions related to uplink TBF
//--------------------------------------------------------------------------
ULTbf *TbfPool::EstablishULTbf(
    IeAccessType                      *accessType,
    IeTLLI                            *tlli, 
    IeChannelRequestDescription       *chanReqDesc,
    IeMSRadioAccessCapability         *radioAccessCapability
)
{
    DBG_FUNC("TbfPool::EstablishULTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() == Tbf::FREE)
        {
            if (sysULTbf[i].EstablishULTbf(accessType, tlli, chanReqDesc,
                radioAccessCapability) == JC_OK)
            {
                return (&sysULTbf[i]);
            }
            else
            {
                break;
            }
        }
    }
    
    DBG_LEAVE();
    return(0);
}


ULTbf *TbfPool::FindULTbf(IeGlobalTFI &globalTfi)
{
    DBG_FUNC("TbfPool::FindULTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysULTbf[i].tfi.GetTFI() == globalTfi.GetGlobalTFI())
            {
                return (&sysULTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}

ULTbf *TbfPool::FindULTbf(IeTFI &tfi)
{
    DBG_FUNC("TbfPool::FindULTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysULTbf[i].tfi.GetTFI() == tfi.GetTFI())
            {
                return (&sysULTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}



ULTbf *TbfPool::FindULTbf(unsigned long tlli)
{
    DBG_FUNC("TbfPool::FindULTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysULTbf[i].tlli.GetTLLI() == tlli)
            {
                return (&sysULTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}


ULTbf *TbfPool::FindULTbf(unsigned char controlAckTag)
{
    DBG_FUNC("TbfPool::FindULTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysULTbf[i].GetControlAckTag() == controlAckTag)
            {
                return (&sysULTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}





//--------------------------------------------------------------------------
// Functions related to downlink TBF
//--------------------------------------------------------------------------


DLTbf *TbfPool::EstablishDLTbf(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg, bool ctrlAck)
{
    DBG_FUNC("TbfPool::EstablishDLTbf", RLC_MAC);
    DBG_ENTER();
    
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() == Tbf::FREE)
        {
            if (sysDLTbf[i].EstablishDLTbf(pUnitDataMsg, ctrlAck) == JC_OK)
            {
                return (&sysDLTbf[i]);
            }
            else
            {
                break;
            }
        }
    }
    
    DBG_LEAVE();
    return(0);
}



DLTbf *TbfPool::FindDLTbf(unsigned long tlli)
{
    DBG_FUNC("TbfPool::FindDLTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysDLTbf[i].tlli.GetTLLI() == tlli)
            {
                return (&sysDLTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}



DLTbf *TbfPool::FindDLTbf(IeGlobalTFI &globalTfi)
{
    DBG_FUNC("TbfPool::FindDLTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysDLTbf[i].tfi.GetTFI() == globalTfi.GetGlobalTFI())
            {
                return (&sysDLTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}

DLTbf *TbfPool::FindDLTbf(IeTFI &tfi)
{
    DBG_FUNC("TbfPool::FindDLTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysDLTbf[i].tfi.GetTFI() == tfi.GetTFI())
            {
                return (&sysDLTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}


DLTbf *TbfPool::FindDLTbf(unsigned char controlAckTag)
{
    DBG_FUNC("TbfPool::FindDLTbf", RLC_MAC);
    DBG_ENTER();

    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() != Tbf::FREE)
        {
            if (sysDLTbf[i].CheckControlAckTag(controlAckTag))
            {
                return (&sysDLTbf[i]);
            }
        }
    }
    
    DBG_LEAVE();
    return (0);
}






bool TbfPool::AnyActiveTBF(int trx, int ts)
{
    DBG_FUNC("TbfPool::AnyActiveTBF", RLC_MAC);
    DBG_ENTER();
    
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (((sysDLTbf[i].GetTbfState() == Tbf::ALLOCATED) &&
            (sysDLTbf[i].trx == trx) && (sysDLTbf[i].allocatedTs[ts])) ||
            ((sysULTbf[i].GetTbfState() == Tbf::ALLOCATED) &&
            (sysULTbf[i].trx == trx) && (sysULTbf[i].allocatedTs[ts])))
        {
            return (TRUE);
        }
    }
    
    DBG_LEAVE();
    return(FALSE);
}




void TbfPool::ShowULAllocatedTBF()
{
    printf("Allocated Uplink TBFs\n");
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() != Tbf::FREE)
        { 
            sysULTbf[i].ShowTBFInfo();
        }
    }
}

void TbfPool::ShowDLAllocatedTBF()
{
    printf("Allocated Downlink TBFs\n");
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() != Tbf::FREE)
        { 
            sysDLTbf[i].ShowTBFInfo();
        }
    }
}

void TbfPool::ShowULFreeTBF()
{
    printf("Free Uplink TBFs\n");
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysULTbf[i].GetTbfState() == Tbf::FREE)
        { 
            sysULTbf[i].ShowTBFInfo();
        }
    }
}

void TbfPool::ShowDLFreeTBF()
{
    printf("Free Downlink TBFs\n");
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        if (sysDLTbf[i].GetTbfState() == Tbf::FREE)
        { 
            sysDLTbf[i].ShowTBFInfo();
        }
    }
}

void TbfPool::ShowULAllTBF()
{
    printf("All Uplink TBFs\n");
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        sysULTbf[i].ShowTBFInfo();
    }
}

void TbfPool::ShowDLAllTBF()
{
    printf("All Downlink TBFs\n");
    for (int i = 0; i < MAX_TFI * MAX_TRX; i++)
    {
        sysDLTbf[i].ShowTBFInfo();
    }
}
