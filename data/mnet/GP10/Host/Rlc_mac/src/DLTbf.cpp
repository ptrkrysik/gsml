// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLTbf.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************

#include "RlcMacTask.h"
#include "DLTbf.h"

#include "grr\grr_intf.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


JC_STATUS DLTbf::EstablishDLTbf(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg, bool ctrlAck)
{
    DBG_FUNC("DLTbf::EstablishDLTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Initialize tbf request info.
    qosProfile = pUnitDataMsg->qos_profile;    
    qosParams = pUnitDataMsg->qos_params;    
    raCapability = pUnitDataMsg->radio_access_cap;    
    drxParams = pUnitDataMsg->drx_params;    
    msId = pUnitDataMsg->imsi;    
    priority = pUnitDataMsg->priority;    
    tlli.SetTLLI(pUnitDataMsg->tlli.tlli);
    controlAck = ctrlAck;
    
    // Clear out allocated resources. Open up allowed resources.
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        allocatedTs[i] = FALSE;
    }
    trx = NULL_TRX;
    
    // Initialize the RLC link
    if ((status = rlcLink.EstablishRLCLink(this, 
        (DLRlcLink::RLC_MODE)pUnitDataMsg->qos_profile.a_bit)) == JC_OK)
    {
        SetTbfState(ACTIVATING);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS DLTbf::ReestablishDLTbf(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg, bool ctrlAck)
{
    DBG_FUNC("DLTbf::ReestablishDLTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Initialize tbf request info.
    qosProfile = pUnitDataMsg->qos_profile;    
    qosParams = pUnitDataMsg->qos_params;    
    raCapability = pUnitDataMsg->radio_access_cap;    
    drxParams = pUnitDataMsg->drx_params;    
    msId = pUnitDataMsg->imsi;    
    priority = pUnitDataMsg->priority;    
    tlli.SetTLLI(pUnitDataMsg->tlli.tlli);
    controlAck = ctrlAck;
    
    // Initialize the RLC link
    if ((status = rlcLink.ReestablishRLCLink(this, 
        (DLRlcLink::RLC_MODE)pUnitDataMsg->qos_profile.a_bit)) == JC_OK)
    {
        SetTbfState(ACTIVATING);
    }
    
    DBG_LEAVE();
    return (status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLTbf::UpdateTSSpecificData
**
**    PURPOSE: This function will be called after timeslots are allocated.  Any
**      data that requires timeslot information should be set here.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLTbf::UpdateTSSpecificData()
{
    DBG_FUNC("DLTbf::UpdateTSSpecificData", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    int numTs = 0;
    
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (allocatedTs[i]) numTs++;
    }
    
    currNumAckTags = numTs * 5;
    
    rlcLink.UpdateTSSpecificData();
    
    DBG_LEAVE();
    return;
}


JC_STATUS DLTbf::ReleaseDLTbf(bool deactivateDsp)
{
    DBG_FUNC("DLTbf::ReleaseDLTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Release resources with the scheduler.
    RlcMacTask::theRlcMacTask->DLReleaseResourceRequest(this);
    // Release resources with the dsps if required.
    if (deactivateDsp)
        RlcMacTask::theRlcMacTask->SendDLDeactivationMsg(this);
    SetTbfState(FREE);
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS DLTbf::InternalReleaseDLTbf()
{
    DBG_FUNC("DLTbf::ReleaseDLTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Release resources with the dsps if required.
    RlcMacTask::theRlcMacTask->SendDLDeactivationMsg(this);
    SetTbfState(RELEASING);
    // Scramble the TLLI.
    tlli.SetTLLI(0xffffffff);
    // Scramble the controlAck tag.
    ClearControlAckTag();
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS DLTbf::AbnormalReleaseDLTbf()
{
    DBG_FUNC("DLTbf::AbnormalReleaseDLTbf", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    status = rlcLink.AbnormalRelease();
    
    DBG_LEAVE();
    return (status);
}


void DLTbf::ShowTBFInfo()
{
    printf("Downlink TBF %#x - State(%d)\n", this, GetTbfState());
    printf("\tTLLI = %#x   TFI = %d\n", tlli.GetTLLI(), tfi.GetTFI());
    printf("\tAllocated Timeslots on TRX(%d):", trx);
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        if (allocatedTs[i] == TRUE)
        {
            printf(" TS(%d)", i);
        }            
    }
    printf("\n");
    printf("\tTAI Timeslot (%d)   TAI(%d)\n", taiTs, tai);
    rlcLink.ShowRlcLinkInfo();
    printf("\n");
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLTbf::GetMultislotClass
**
**    PURPOSE: Get multislot class sent from GPRS Server and stored in DLTbf 
**
**    INPUT PARAMETERS: none 
**
**    RETURN VALUE(S): mobile's multislot class, 1-18
**
**----------------------------------------------------------------------------*/
int DLTbf::GetMultislotClass(void) const
{
    if (raCapability.ie_present)
    {
        for ( int i=0; i<raCapability.number_of_ra_capabilities; i++ )
        {
            if ( raCapability.ms_ra_capability[i].access_technology_type == 
                    grr_GetRadioAccessTechnology() )
                if ((raCapability.ms_ra_capability[i].multislot_capability.present) &&
                        (raCapability.ms_ra_capability[i].multislot_capability.gprs_params_present))
                    return (raCapability.ms_ra_capability[i].multislot_capability.gprs_multi_slot_class);
        }
    }
    // If multislot class is not present, return Multislot Class 1 
    return (1);
}

