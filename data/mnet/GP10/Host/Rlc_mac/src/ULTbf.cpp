// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ULTbf.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************

#include "RlcMacTask.h"
#include "ScheduleAllocationInfo.h"
#include "ULTbf.h"
#include "USFTable.h"

#include "grr\grr_intf.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

JC_STATUS ULTbf::EstablishULTbf(
    IeAccessType                      *accessType,
    IeTLLI                            *tlli, 
    IeChannelRequestDescription       *chanReqDesc,
    IeMSRadioAccessCapability         *radioAccessCapability
)
{
    DBG_FUNC("ULTbf::EstablishULTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Initialize tbf request info.
    accType = *accessType;
    chanReq = *chanReqDesc;
    raCapability = *radioAccessCapability;
    Tbf::tlli = *tlli;
    
    // Initialize the RLC link
    
    // Clear out allocated resources. Open up allowed resources.
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        allocatedTs[i] = FALSE;
        usf[i] = 0;
    }
    trx = NULL_TRX;
    
    if ((status = rlcLink.EstablishRLCLink(this, 
        (ULRlcLink::RLC_MODE)chanReqDesc->rlcMode.GetRlcMode())) == JC_OK)
    {
        SetTbfState(ALLOCATED);
    }
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS ULTbf::ReallocateTbf(
    IeAccessType                      *accessType,
    IeChannelRequestDescription       *chanReqDesc,
    IeMSRadioAccessCapability         *radioAccessCapability
)
{
    DBG_FUNC("ULTbf::ReallocateTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    accType = *accessType;
    chanReq = *chanReqDesc;
    raCapability = *radioAccessCapability;
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS ULTbf::ReleaseULTbf()
{
    DBG_FUNC("ULTbf::ReleaseULTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Release resources with the scheduler.
    RlcMacTask::theRlcMacTask->ULReleaseResourceRequest(this);
    // Release resources with the dsps.
    RlcMacTask::theRlcMacTask->SendULDeactivationMsg(this);
    SetTbfState(FREE);
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS ULTbf::AbnormalReleaseULTbf()
{
    DBG_FUNC("ULTbf::AbnormalReleaseULTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    status = rlcLink.AbnormalRelease();
        
    DBG_LEAVE();
    return (status);
}


void ULTbf::ShowTBFInfo()
{
    printf("Uplink TBF %#x - State(%d)\n", this, GetTbfState());
    printf("\tTLLI = %#x   TFI = %d\n", tlli.GetTLLI(), tfi.GetTFI());
    printf("\tAllocated Timeslots on TRX(%d):", trx);
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        if (allocatedTs[i] == TRUE)
        {
            printf(" TS(%d) USF(%d)", i, usf[i]);
        }
    }
    printf("\n");
    printf("\tTAI Timeslot (%d)   TAI(%d)\n", taiTs, tai);
    rlcLink.ShowRlcLinkInfo();
    printf("\n");
}

       
/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULTbf::GetMultislotClass
**
**    PURPOSE: Get multislot class sent from GPRS Server and stored in DLTbf 
**
**    INPUT PARAMETERS: none 
**
**    RETURN VALUE(S): mobile's multislot class, 1-18
**
**----------------------------------------------------------------------------*/
int ULTbf::GetMultislotClass(void)
{
    if ( raCapability.IsValid() )
    { 
        for ( int i=0; i<MAX_ACCESS_TECHNOLOGIES; i++ )
        {
             if ( raCapability.MSRACapabilityValuePart[i].isValid &&
                  raCapability.MSRACapabilityValuePart[i].accessTechnologyType ==
                     (IeMSRadioAccessCapability::ACCESS_TECHNOLOGY_TYPE)grr_GetRadioAccessTechnology() )
                 return (raCapability.MSRACapabilityValuePart[i].GPRSCapability.gprsMultislotClass);
        }
    }
    // If multislot class is not present, return Multislot Class 1 
    return(1);
}


