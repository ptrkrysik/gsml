// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MacAllocation.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#include "RlcMacTask.h"
#include "Rlc_mac/RlcMacIntf.h"
#include "dsp/dsphmsg.h"
#include "ril3/ril3_common.h"
#include "Grr/grr_intf.h"
#include "bssgp\bssgp_unitdata.h"
#include "MsgPacketAccessReject.h"
#include "MsgPacketUplinkAssignment.h"
#include "MsgPacketDownlinkAssignment.h"

extern void sendDsp(unsigned char *buffer, int len);


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULSingleBlockAllocation
**
**    PURPOSE: Perform uplink single block allocation.  An IMMEDIATE ASSIGNMENT
**      message will be constructed and sent over the AGCH found on the BCCH.
**      A single block allocation does not require a TBF to be created and
**      therefore no MAC scheduling will occur.  The single block will simply
**      be granted.
**
**      Because the host has no knowledge of frame numbers the DSP will fill
**      in the starting time IE.
**
**    INPUT PARAMETERS: chanReq - decoded channel request
**                      msgReq - original layer 1 message request
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ULSingleBlockAllocation(T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST *chanReq,
    T_CNI_RIL3MD_RRM_MSG *msgReq)
{
    DBG_FUNC("RlcMacTask::SingleBlockAllocation", RLC_MAC);
    DBG_ENTER();
   
    JC_STATUS status = JC_OK;
    
    DBG_TRACE("RLC/MAC: Attempting to establish a new uplink TBF using Single block allocation access\n");
    
    int trx, ts;
    // Build an immediate assignment for a single block assigment.
    T_CNI_LAPDM_L3MessageUnit l3Msg;
    T_CNI_RIL3RRM_MSG ril3RrMsg;
    
    // Call the scheduler to get a timeslot and trx.  If resources are available
    // then send an IMMEDIATE ASSIGNMENT otherwise send an IMMEDIATE ASSIGNMENT REJECT.
    if (ULSingleBlockRequest(&trx, &ts))
    {
        T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT *pImmAsgnMsg = &ril3RrMsg.immediateAssignment;
        unsigned char tsc;
        unsigned short arfcn;

        bzero((char *)&ril3RrMsg, sizeof(T_CNI_RIL3RRM_MSG));

        // Populate header
        pImmAsgnMsg->header.protocol_descriminator = CNI_RIL3_PD_RRM;
        pImmAsgnMsg->header.si_ti                  = 0;
        pImmAsgnMsg->header.message_type           = CNI_RIL3RRM_MSGID_IMMEDIATE_ASSIGNMENT;

        // Populate Page Mode
        pImmAsgnMsg->pageMode.ie_present = TRUE;
        pImmAsgnMsg->pageMode.pm         = CNI_RIL3_NORMAL_PAGING;

        // Populate Dedicated Mode or TBF
        pImmAsgnMsg->dmtbf.td_bit           = 1;
        pImmAsgnMsg->dmtbf.downlink_bit     = 0;
        pImmAsgnMsg->dmtbf.tma_bit          = 0;

        // Populate Packet Channel Description
        pImmAsgnMsg->packetChannelDescription.ie_present    = TRUE;
        // Call GRR to get tsc and arfcn.
        tsc = grr_GetTrxSlotTsc(trx, ts);
        arfcn = grr_GetTrxSlotArfcn(trx, ts);
        
        pImmAsgnMsg->packetChannelDescription.TN                = ts;
        pImmAsgnMsg->packetChannelDescription.TSC               = tsc;
        pImmAsgnMsg->packetChannelDescription.hopping_indicator = CNI_RIL3_PCD_HOPPING_INDICATOR_NO_HOPPING;
        pImmAsgnMsg->packetChannelDescription.arfcn             = arfcn;
        pImmAsgnMsg->packetChannelDescription.change_mark_1_selector = FALSE;

        // Populate Request Reference
        pImmAsgnMsg->reqReference.ie_present       = TRUE;     
        pImmAsgnMsg->reqReference.randomAccessInfo = msgReq->l3_data.buffer[0];
        pImmAsgnMsg->reqReference.T1               = (msgReq->l3_data.buffer[1]>>3) & 0x1f;
        pImmAsgnMsg->reqReference.T2               = (msgReq->l3_data.buffer[2]) & 0x1f;
        pImmAsgnMsg->reqReference.T3               = ((msgReq->l3_data.buffer[1]<<3) & 0x38) |
                                                    ((msgReq->l3_data.buffer[2]>>5) & 0x07);
                                                 
        // Populate Timing Advance
        pImmAsgnMsg->timingAdvance.ie_present = TRUE;
        pImmAsgnMsg->timingAdvance.value      = msgReq->l3_data.buffer[3];

        // Populate Mobile Allocation
        // NOTE: Frequency hopping not used so set length to zero
        pImmAsgnMsg->mobileAllocation.ie_present = TRUE;
        pImmAsgnMsg->mobileAllocation.numRFfreq  = 0;

        // Populate Starting Time
        // NOTE: Starting time for packet data is set in the IA Rest Octets
        pImmAsgnMsg->startingTime.ie_present = FALSE;

        // Populate IA Rest Octets
        pImmAsgnMsg->iaRestOctets.ie_present = TRUE;
        pImmAsgnMsg->iaRestOctets.p = CNI_RIL3_PACKET_ASSIGNMENT_UPLINK;
        pImmAsgnMsg->iaRestOctets.pua.is_multi_block_allocation = FALSE;
        pImmAsgnMsg->iaRestOctets.pua.sba.is_alpha_present = TRUE;
        pImmAsgnMsg->iaRestOctets.pua.sba.alpha = 0xa;  // 1.0
        pImmAsgnMsg->iaRestOctets.pua.sba.gamma = 0x16; // 44dbm
        pImmAsgnMsg->iaRestOctets.pua.sba.flag_pbp = FALSE;
        // NOTE: No valid starting time set here.  DSP will fill this in later.
        
        DBG_TRACE("RLC/MAC: Sending uplink single block allocation trx(%d) ts(%d).\n",
            trx, ts);
    }
    // Send IMMEDIATE ASSIGNMENT REJECT.
    else
    {
        T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT_REJECT *pImmAsgnMsgRej = 
            &ril3RrMsg.immediateAssignmentReject;
        unsigned char tsc;
        unsigned short arfcn;

        bzero((char *)&ril3RrMsg, sizeof(T_CNI_RIL3RRM_MSG));

        // Populate header
        pImmAsgnMsgRej->header.protocol_descriminator   = CNI_RIL3_PD_RRM;
        pImmAsgnMsgRej->header.si_ti                    = 0;
        pImmAsgnMsgRej->header.message_type             = CNI_RIL3RRM_MSGID_IMMEDIATE_ASSIGNMENT_REJECT;

        // Populate Page Mode
        pImmAsgnMsgRej->pageMode.ie_present = TRUE;
        pImmAsgnMsgRej->pageMode.pm         = CNI_RIL3_NORMAL_PAGING;

        for (int i=0; i < 4; i++)
        {
            // Populate Request Reference
            pImmAsgnMsgRej->reqReference[i].ie_present       = TRUE;     
            pImmAsgnMsgRej->reqReference[i].randomAccessInfo = chanReq->randomReference;
            pImmAsgnMsgRej->reqReference[i].T1               = (msgReq->l3_data.buffer[1]>>3) & 0x1f;
            pImmAsgnMsgRej->reqReference[i].T2               = (msgReq->l3_data.buffer[2]) & 0x1f;
            pImmAsgnMsgRej->reqReference[i].T3               = ((msgReq->l3_data.buffer[1]<<3) & 0x38) |
                                                                ((msgReq->l3_data.buffer[2]>>5) & 0x07);
                                                            
            // Populate Wait Indication
            pImmAsgnMsgRej->waitIndication[i].ie_present     = TRUE;     
            pImmAsgnMsgRej->waitIndication[i].T3122          = 5;   // T3142
        }
                                                         
        DBG_TRACE("RLC/MAC: No resources available sending IMMEDIATE ASSIGN REJECT.\n");
    }
    
    //Call RR message encoding functionality
    T_CNI_RIL3_RESULT ril3Result = CNI_RIL3RRM_Encode( &ril3RrMsg, &l3Msg);

    if (ril3Result == CNI_RIL3_RESULT_SUCCESS)
    {
        // Build DSP message and send it
        unsigned char dspMsg[L1RM_MAXMSG_LEN];
        int length = 4;     // Skip the length field for now
        dspMsg[length++] = RR_MANAGE_MSG;
        dspMsg[length++] = PH_DATA_REQ>>8;  // msb
        dspMsg[length++] = PH_DATA_REQ;     // lsb
        dspMsg[length++] = 0;               // always TRX 0
        dspMsg[length++] = 0x90;            // AGCH
        dspMsg[length++] = 28;
        dspMsg[length++] = 0x20;            // void link
        memcpy(&dspMsg[length], l3Msg.buffer, l3Msg.msgLength);
        length += l3Msg.msgLength - 4;
        dspMsg[3] = length>>24;
        dspMsg[2] = length>>16;
        dspMsg[1] = length>>8;
        dspMsg[0] = length;
    
        PostL3SendMsLog( &l3Msg );
        sendDsp(dspMsg, length+4);
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessChanReq: RIL3 ImmediateAssignReject encode failed\n");
        status = JC_INVALID_MSG;        
    }  
    
    DBG_LEAVE();
    return(status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULOnePhaseAllocation
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: chanReq - decoded channel request
**                      msgReq - original layer 1 message request
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ULOnePhaseAllocation(T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST *chanReq,
    T_CNI_RIL3MD_RRM_MSG *msgReq) 
 {
    DBG_FUNC("RlcMacTask::ULOnePhaseAllocation", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    IeAccessType                      accessType;
    IeTLLI                            tlli; 
    IeChannelRequestDescription       chanReqDesc;
    IeMSRadioAccessCapability         radioAccessCapability;
    
    DBG_TRACE("RLC/MAC: Attempting to establish a new uplink TBF using One-phase access\n");
    
    ULTbf *pUlTbf;
    
    // For now just use a dummy tlli.
    tlli.SetTLLI(0x12345678);
    
    // Find an unused TBF object.
    pUlTbf = tbfPool.EstablishULTbf(&accessType, &tlli, &chanReqDesc, &radioAccessCapability);
    
    // If an uplink TBF is available attempt to allocate the requested resources.
    if (pUlTbf)
    {
        // Request resources from the scheduler.  If resources are
        // available send an IMMEDIATE ASSIGNMENT message..
        if (ULResourceRequest(pUlTbf))
        {
            DBG_TRACE("RLC/MAC: Uplink TBF resources allocated\n");
            pUlTbf->rlcLink.UpdateTSSpecificData();
            SendULActivationMsg(pUlTbf);
            ULCCCHImmediateAssignment(pUlTbf, chanReq, msgReq);
            // Kick start RTS mechanism.
            for (int i = 0; i < MAX_TIMESLOTS; i++)
            {
                if (pUlTbf->allocatedTs[i])
                {
                    RlcMacTask::theRlcMacTask->SendDownlinkDummyControlBlock(pUlTbf->trx, i);
                }
            } 
        }
        // If no resources are available then send an IMMEDIATE ASSIGN REJECT message.
        else
        {
            DBG_TRACE("RLC/MAC: NO uplink resources available...rejecting access!!\n");
            pUlTbf->ReleaseULTbf();        
        }
    }
    // If no uplink TBF is available then send an IMMEDIATE ASSIGN REJECT message.
    else
    {
        DBG_TRACE("RLC/MAC: NO uplink TBF available...rejecting access!!\n");
    }

    DBG_LEAVE();
    return(status);
 }


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ULCCCHImmediateAssignment
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pUlTbf - uplink TBF
**                      chanReq - decoded channel request
**                      msgReq - original layer 1 message request
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ULCCCHImmediateAssignment(ULTbf *pUlTbf, 
    T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST *chanReq, T_CNI_RIL3MD_RRM_MSG *msgReq)
{
    DBG_FUNC("RlcMacTask::ULCCCHImmediateAssignment", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Build an immediate assignment for an uplink TBF.
    T_CNI_LAPDM_L3MessageUnit l3Msg;
    T_CNI_RIL3RRM_MSG ril3RrMsg;
    T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT *pImmAsgnMsg = &ril3RrMsg.immediateAssignment;

    bzero((char *)&ril3RrMsg, sizeof(T_CNI_RIL3RRM_MSG));

    // Populate header
    pImmAsgnMsg->header.protocol_descriminator = CNI_RIL3_PD_RRM;
    pImmAsgnMsg->header.si_ti                  = 0;
    pImmAsgnMsg->header.message_type           = CNI_RIL3RRM_MSGID_IMMEDIATE_ASSIGNMENT;

    // Populate Page Mode
    pImmAsgnMsg->pageMode.ie_present = TRUE;
    pImmAsgnMsg->pageMode.pm         = CNI_RIL3_NORMAL_PAGING;

    // Populate Dedicated Mode or TBF
    pImmAsgnMsg->dmtbf.td_bit           = 1;
    pImmAsgnMsg->dmtbf.downlink_bit     = 1;
    pImmAsgnMsg->dmtbf.tma_bit          = 0;

    // Populate Packet Channel Description
    pImmAsgnMsg->packetChannelDescription.ie_present    = TRUE;
    unsigned char tsc;
    unsigned short arfcn;
    int ts;
    
    // Find the allocated TS.
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        if (pUlTbf->allocatedTs[i] == TRUE)
        {
            ts = i;
        }
    }
    // Call GRR to get tsc and arfcn.
    tsc = grr_GetTrxSlotTsc(pUlTbf->trx, ts);
    arfcn = grr_GetTrxSlotArfcn(pUlTbf->trx, ts);
    
    pImmAsgnMsg->packetChannelDescription.TN                = ts;
    pImmAsgnMsg->packetChannelDescription.TSC               = tsc;
    pImmAsgnMsg->packetChannelDescription.hopping_indicator = CNI_RIL3_PCD_HOPPING_INDICATOR_NO_HOPPING;
    pImmAsgnMsg->packetChannelDescription.arfcn             = arfcn;
    pImmAsgnMsg->packetChannelDescription.change_mark_1_selector = FALSE;

    // Populate Request Reference
    pImmAsgnMsg->reqReference.ie_present       = TRUE;     
    pImmAsgnMsg->reqReference.randomAccessInfo = msgReq->l3_data.buffer[0];
    pImmAsgnMsg->reqReference.T1               = (msgReq->l3_data.buffer[1]>>3) & 0x1f;
    pImmAsgnMsg->reqReference.T2               = (msgReq->l3_data.buffer[2]) & 0x1f;
    pImmAsgnMsg->reqReference.T3               = ((msgReq->l3_data.buffer[1]<<3) & 0x38) |
                                                    ((msgReq->l3_data.buffer[2]>>5) & 0x07);
                                             
    // Populate Timing Advance
    // This IE will be invalidated by the IA Rest Octets
    pImmAsgnMsg->timingAdvance.ie_present = TRUE;
    pImmAsgnMsg->timingAdvance.value      = 0;

    // Populate Mobile Allocation
    // NOTE: Frequency hopping not used so set length to zero
    pImmAsgnMsg->mobileAllocation.ie_present = TRUE;
    pImmAsgnMsg->mobileAllocation.numRFfreq  = 0;

    // Populate Starting Time
    // NOTE: Starting time for packet data is set in the IA Rest Octets
    pImmAsgnMsg->startingTime.ie_present = FALSE;

    // Populate IA Rest Octets
    pImmAsgnMsg->iaRestOctets.ie_present = TRUE;
    pImmAsgnMsg->iaRestOctets.p = CNI_RIL3_PACKET_ASSIGNMENT_UPLINK;
    pImmAsgnMsg->iaRestOctets.pua.is_multi_block_allocation = TRUE;
    pImmAsgnMsg->iaRestOctets.pua.mba.tfi_assignment = pUlTbf->tfi.GetTFI();
    pImmAsgnMsg->iaRestOctets.pua.mba.polling = FALSE;
    pImmAsgnMsg->iaRestOctets.pua.mba.is_fix_allocation = FALSE;
    pImmAsgnMsg->iaRestOctets.pua.mba.da.usf = pUlTbf->usf[ts];
    pImmAsgnMsg->iaRestOctets.pua.mba.da.usf_granularity = 0;
    pImmAsgnMsg->iaRestOctets.pua.mba.da.flag_pp = FALSE;
    pImmAsgnMsg->iaRestOctets.pua.mba.channel_coding_command = 0;
    pImmAsgnMsg->iaRestOctets.pua.mba.tlli_block_channel_coding = 0;
    pImmAsgnMsg->iaRestOctets.pua.mba.is_alpha_present = TRUE;
    pImmAsgnMsg->iaRestOctets.pua.mba.alpha = 0xa;  // 1.0
    pImmAsgnMsg->iaRestOctets.pua.mba.gamma = 0x16; // 44dbm
    pImmAsgnMsg->iaRestOctets.pua.mba.is_timing_advance_index_present = TRUE;
    pImmAsgnMsg->iaRestOctets.pua.mba.timing_advance_index = pUlTbf->tai;
    pImmAsgnMsg->iaRestOctets.pua.mba.is_tbf_starting_time_present = FALSE;
    pImmAsgnMsg->iaRestOctets.pua.mba.tbf_starting_time = 0;
    // NOTE: No valid starting time set here.  DSP will fill this in later.
    
    //Call RR message encoding functionality
    T_CNI_RIL3_RESULT ril3Result = CNI_RIL3RRM_Encode( &ril3RrMsg, &l3Msg);

    if (ril3Result == CNI_RIL3_RESULT_SUCCESS)
    {
        // Build DSP message and send it
        unsigned char dspMsg[L1RM_MAXMSG_LEN];
        int length = 4;     // Skip the length field for now
        dspMsg[length++] = RR_MANAGE_MSG;
        dspMsg[length++] = PH_DATA_REQ>>8;  // msb
        dspMsg[length++] = PH_DATA_REQ;     // lsb
        dspMsg[length++] = 0;               // always TRX 0
        dspMsg[length++] = 0x90;            // AGCH
        dspMsg[length++] = 28;
        dspMsg[length++] = 0x20;            // void link
        memcpy(&dspMsg[length], l3Msg.buffer, l3Msg.msgLength);
        length += l3Msg.msgLength - 4;
        dspMsg[3] = length>>24;
        dspMsg[2] = length>>16;
        dspMsg[1] = length>>8;
        dspMsg[0] = length;
    
        PostL3SendMsLog( &l3Msg );
        sendDsp(dspMsg, length+4);
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessChanReq: RIL3 ImmediateAssign encode failed\n");
        status = JC_INVALID_MSG;        
    }
    
    DBG_LEAVE();
    return(status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::AllocateULResources
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: msg - Packet Resource Request Message
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::AllocateULResources(MsgPacketResourceRequest &msg, 
    int trx, int ts) 
 {
    DBG_FUNC("RlcMacTask::AllocateULResources", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // First check to see if an uplink TBF is not already ongoing.  This is
    // determined by the MS sending the uplink TFI as its identity.
    if (msg.globalTfi.IsValid()) 
    {
        DBG_TRACE("RLC/MAC: Attempting to allocate uplink resources tfi(%#x)\n", msg.globalTfi.GetGlobalTFI());
        // Uplink TBF already established so try and reallocate resources.
        if (msg.globalTfi.GetGlobalTFIType() == IeGlobalTFI::UPLINK_TFI)
        {
            // Try and reallocate resources.
            ULTbf *pUlTbf = tbfPool.FindULTbf(msg.globalTfi); 
            if (pUlTbf)
            {
                pUlTbf->ReallocateTbf(&msg.accessType, &msg.chanReqDescription, &msg.msRadioAccessCapability);
                if (ULResourceReallocRequest(pUlTbf))
                {
                    DBG_TRACE("RLC/MAC: Uplink TBF resources reallocated tfi(%3x)", msg.globalTfi.GetGlobalTFI());
                    pUlTbf->rlcLink.UpdateTSSpecificData();
                    SendULReconfigurationMsg(pUlTbf);
                    SendPacketUplinkAssignment(trx, ts, pUlTbf, &msg.globalTfi, MS_ID_GLOBAL_TFI);
                }
                // If no resources are available then send a PACKET ACCESS REJECT message.
                else
                {
                    DBG_TRACE("RLC/MAC: NO uplink resources available...rejecting access!! tfi(%#x)\n",
                        msg.globalTfi.GetGlobalTFI());
                    SendPacketAccessReject(trx, ts, &msg.globalTfi, MS_ID_GLOBAL_TFI);
                }
            }
            else
            {
                DBG_TRACE("RLC/MAC: Can't find uplink TBF...rejecting access!! tfli(%#x)\n",
                    msg.globalTfi.GetGlobalTFI());
                SendPacketAccessReject(trx, ts, &msg.globalTfi, MS_ID_GLOBAL_TFI);
            }
        }
        // Downlink TBF already established.
        else
        {
            DBG_TRACE("RLC/MAC: Attempting to establish a new uplink TBF, downlink TBF exists tfi(%#x)\n",
                msg.globalTfi.GetGlobalTFI());
            DLTbf *pDlTbf = tbfPool.FindDLTbf(msg.globalTfi);
            if (pDlTbf)
            {
                ULTbf *pUlTbf = tbfPool.EstablishULTbf(&msg.accessType, &pDlTbf->tlli,
                    &msg.chanReqDescription, &msg.msRadioAccessCapability);
                if (pUlTbf)
                {
                    // Request resources from the scheduler.  If resources are
                    // available send a PACKET UPLINK ASSIGNMENT message.
                    if (ULResourceRequest(pUlTbf, pDlTbf))
                    {
                        DBG_TRACE("RLC/MAC: Uplink TBF resources allocated tfi(%#x)\n",
                            msg.globalTfi.GetGlobalTFI());
                        pUlTbf->rlcLink.UpdateTSSpecificData();        
                        SendULActivationMsg(pUlTbf);
                        SendPacketUplinkAssignment(trx, ts, pUlTbf, &msg.globalTfi, MS_ID_GLOBAL_TFI);
                    }
                    // If no resources are available then send a PACKET ACCESS REJECT message.
                    else
                    {
                        DBG_TRACE("RLC/MAC: NO uplink resources available...rejecting access!! tfi(%#x)\n",
                            msg.globalTfi.GetGlobalTFI());
                        SendPacketAccessReject(trx, ts, &msg.globalTfi, MS_ID_GLOBAL_TFI);
                        pUlTbf->ReleaseULTbf();        
                    }
                }
                else
                {
                    DBG_TRACE("RLC/MAC: NO uplink TBF available...rejecting access!! tfi(%#x)\n",
                        msg.globalTfi.GetGlobalTFI());
                    SendPacketAccessReject(trx, ts, &msg.globalTfi, MS_ID_GLOBAL_TFI);
                }
            }
            else
            {
                DBG_TRACE("RLC/MAC: Can't find downlink TBF...rejecting access!! tfi(%#x)\n",
                    msg.globalTfi.GetGlobalTFI());
                SendPacketAccessReject(trx, ts, &msg.globalTfi, MS_ID_GLOBAL_TFI);
            }        
        }
    }
    else
    {
        DBG_TRACE("RLC/MAC: Attempting to establish a new uplink TBF, no downlink TBF tlli(%#x)\n",
            msg.tlli.GetTLLI());
    
        ULTbf *pUlTbf;
        
        // If we got a request with a TLLI and one exists then it must be a stale TBF.
        // For this case we will release the TBF immediately.
        if ((pUlTbf = tbfPool.FindULTbf((unsigned long)msg.tlli.GetTLLI())))
        {
            pUlTbf->ReleaseULTbf();
        }
                
        // Find an unused TBF object.
        pUlTbf = tbfPool.EstablishULTbf(&msg.accessType, &msg.tlli,
            &msg.chanReqDescription, &msg.msRadioAccessCapability);
    
        // If an uplink TBF is available attempt to allocate the requested resources.
        if (pUlTbf)
        {
            // Request resources from the scheduler.  If resources are
            // available send a PACKET UPLINK ASSIGNMENT message..
            if (ULResourceRequest(pUlTbf))
            {
                DBG_TRACE("RLC/MAC: Uplink TBF resources allocated tlli(%#x)\n", msg.tlli.GetTLLI());
                pUlTbf->rlcLink.UpdateTSSpecificData();
                SendULActivationMsg(pUlTbf);
                SendPacketUplinkAssignment(trx, ts, pUlTbf, &msg.tlli, MS_ID_TLLI);
            }
            // If no resources are available then send a PACKET ACCESS REJECT message.
            else
            {
                DBG_TRACE("RLC/MAC: NO uplink resources available...rejecting access!! tlli(%#x)\n",
                    msg.tlli.GetTLLI());
                SendPacketAccessReject(trx, ts, &msg.tlli, MS_ID_TLLI);
                pUlTbf->ReleaseULTbf();        
            }
        }
        // If no uplink TBF is available then send a PACKET ACCESS REJECT message.
        else
        {
            DBG_TRACE("RLC/MAC: NO uplink TBF available...rejecting access!! tlli(%#x)\n", 
                msg.tlli.GetTLLI());
            SendPacketAccessReject(trx, ts, &msg.tlli, MS_ID_TLLI);
        }
    }

    DBG_LEAVE();
    return(status);
 }





/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketAccessReject
**
**    PURPOSE: Build and transmit a Packet Access Reject message.
**
**    INPUT PARAMETERS: trx - trx to transmit message
**                      ts - timeslot to transmit message
**                      pIe - pointer to the IE containing the MS id
**                      idType - type of MS id (TLLI, Global TFI)
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPacketAccessReject(int trx, int ts, IeBase *pIe, 
    MS_ID_TYPE idType) 
{
    DBG_FUNC("RlcMacTask::SendPacketAccessReject", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader             rejMsgHdr;
    MsgPacketAccessReject   rejMsg;

    // Assemble RLC/MAC header.
    rejMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    // For Packet Access Reject RRBP is ignored by the MS so just set it to anything.
    rejMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
    rejMsgHdr.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Access Reject.
    rejMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    switch (idType)
    {
        case MS_ID_TLLI:
        {
            rejMsg.reject.tlli = *(IeTLLI *)pIe;
        }
        break;
        case MS_ID_GLOBAL_TFI:
        {
            rejMsg.reject.globalTFI = *(IeGlobalTFI *)pIe;
        }
        break;
    }
    
    rejMsg.reject.isValid = TRUE;
    rejMsg.additionalReject.isValid = FALSE;
    
    if ((status = SendPacchMsg(trx, ts, &rejMsgHdr, &rejMsg, CTL_ACK_TAG_NOT_APPLICABLE,
        RLC_MAC_NORMAL_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketAccessReject: failed %#x\n",
            status);       
    }
    
    DBG_LEAVE();
    return(status);
 }



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketUplinkAssignment
**
**    PURPOSE: Build and transmit a Packet Uplink Assignment message.
**
**    INPUT PARAMETERS: trx - trx to transmit message
**                      ts - timeslot to transmit message
**                      pUlTbf - pointer to the uplink tbf
**                      pIe - pointer to the IE containing the MS id
**                      idType - type of MS id (TLLI, Global TFI)
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPacketUplinkAssignment(int trx, int ts, ULTbf *pUlTbf,
    IeBase *pIe, MS_ID_TYPE idType) 
 {
    DBG_FUNC("RlcMacTask::SendPacketUplinkAssignment", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader                 asnMsgHdr;
    MsgPacketUplinkAssignment   asnMsg;

    // Assemble RLC/MAC header.
    asnMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    // RRBP will not be valid so just set it to anything.
    asnMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
    asnMsgHdr.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Uplink Assignment.
    asnMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    switch (idType)
    {
        case MS_ID_TLLI:
        {
            asnMsg.tlli = *(IeTLLI *)pIe;
        }
        break;
        case MS_ID_GLOBAL_TFI:
        {
            asnMsg.globalTFI = *(IeGlobalTFI *)pIe;
        }
        break;
    }
    asnMsg.chanCoding.SetChannelCodingCommand(IeChannelCodingCommand::CS_1);
    asnMsg.tlliBlockCoding = MsgPacketUplinkAssignment::USE_COMMANDED_CS_FOR_DATA_BLOCK_WITH_TLLI;
    asnMsg.timingAdvance.SetPacketTimingAdvance(pUlTbf->tai, pUlTbf->taiTs);
    // Call GRR to get frequency parameters.
    unsigned char tsc;
    unsigned short arfcn;
    tsc = grr_GetTrxSlotTsc(trx, ts);
    arfcn = grr_GetTrxSlotArfcn(trx, ts);
    
    asnMsg.frequencyParameters.tsc.SetTSC(tsc);
    asnMsg.frequencyParameters.arfcn.SetARFCN(arfcn);
    asnMsg.frequencyParameters.SetFrequencyParameters();   
    
    asnMsg.dynamicAllocation.allocType = DynamicAllocationStruct::DYNAMIC_ALLOCATION; 
    asnMsg.dynamicAllocation.usfGranularity = DynamicAllocationStruct::ONE_RLC_MAC_BLOCK;
    asnMsg.dynamicAllocation.uplinkTFI = pUlTbf->tfi;
    //asnMsg.dynamicAllocation.startTime.SetStartFrameNum(RELATIVE_START_TIME_VAL);
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pUlTbf->allocatedTs[i] == TRUE)
        {
            asnMsg.dynamicAllocation.timeslotAllocationWithPower.tsAllocWithPowerEntry[i].usf = pUlTbf->usf[i];
            asnMsg.dynamicAllocation.timeslotAllocationWithPower.tsAllocWithPowerEntry[i].gamma = 0x16;
            asnMsg.dynamicAllocation.timeslotAllocationWithPower.tsAllocWithPowerEntry[i].isValid = TRUE;
        }
        else
        {
            asnMsg.dynamicAllocation.timeslotAllocationWithPower.tsAllocWithPowerEntry[i].isValid = FALSE;
        }
        asnMsg.dynamicAllocation.timeslotAllocationWithPower.alpha = 0xa;      // 1.0
        asnMsg.dynamicAllocation.timeslotAllocationWithPower.isValid = TRUE;
    }
    
    asnMsg.dynamicAllocation.isValid = TRUE;
    
    if ((status = SendPacchMsg(trx, ts, &asnMsgHdr, &asnMsg, CTL_ACK_TAG_NOT_APPLICABLE,
        RLC_MAC_NORMAL_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketUplinkAssignment: failed %#x\n",
            status);       
    }
    
    // Now make sure the RTS mechanism is triggered for all allocated timeslots.
    for (i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pUlTbf->allocatedTs[i])
        {
            if (!(RlcMacTask::theRlcMacTask->SendDownlinkDataBlock(pUlTbf->trx, i)))
            {
                RlcMacTask::theRlcMacTask->SendDownlinkDummyControlBlock(pUlTbf->trx, i);
            }
        }
    } 
    
    DBG_LEAVE();
    return(status);
 }


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketTBFRelease
**
**    PURPOSE: Build and transmit a Pacet TBF Release message.
**
**    INPUT PARAMETERS: pDlTbf - pointer to downlink TBF
**                      pUlTbf - pointer to uplink TBF
**                      cause - tbf release cause (normal or abnormal)
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPacketTBFRelease(DLTbf *pDlTbf, ULTbf *pUlTbf,
    MsgPacketTBFRelease::TBF_RELEASE_CAUSE cause) 
{
    DBG_FUNC("RlcMacTask::SendPacketTBFRelease", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader             relMsgHdr;
    MsgPacketTBFRelease     relMsg;
    int                     trx, ts;

    // Assemble RLC/MAC header.
    relMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    // For Packet Access Reject RRBP is ignored by the MS so just set it to anything.
    relMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
    relMsgHdr.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Access Reject.
    relMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    
    // If the uplink TBF is valid then send on the PACCH of the uplink.
    if (pUlTbf)
    {
        relMsg.globalTFI.SetGlobalTFI(pUlTbf->tfi.GetTFI(), IeGlobalTFI::UPLINK_TFI);
        relMsgHdr.SetTFI(pUlTbf->tfi.GetTFI());
        relMsgHdr.SetDirectionBit(DLMacHeader::UPLINK_TBF);
        TSSelectionRequest(&trx, &ts, pUlTbf);
        relMsg.uplinkRel = MsgPacketTBFRelease::TBF_RELEASED;
        relMsg.downlinkRel = MsgPacketTBFRelease::TBF_NOT_RELEASED;
    }
    else
    {
        relMsg.globalTFI.SetGlobalTFI(pDlTbf->tfi.GetTFI(), IeGlobalTFI::DOWNLINK_TFI);
        relMsgHdr.SetTFI(pDlTbf->tfi.GetTFI());
        relMsgHdr.SetDirectionBit(DLMacHeader::DOWNLINK_TBF);
        TSSelectionRequest(&trx, &ts, pDlTbf);
        relMsg.uplinkRel = MsgPacketTBFRelease::TBF_NOT_RELEASED;
        relMsg.downlinkRel = MsgPacketTBFRelease::TBF_RELEASED;
    }
    
    relMsg.relCause = cause;
    
    if ((status = SendPacchMsg(trx, ts, &relMsgHdr, &relMsg, CTL_ACK_TAG_NOT_APPLICABLE,
        RLC_MAC_NORMAL_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketTBFRelease: failed %#x\n",
            status);       
    }
    
    DBG_LEAVE();
    return(status);
 }



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::CheckDLMsgQueues
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pUnitDataMsg - BSSGP unit data message
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::CheckDLMsgQueues() 
{
    DBG_FUNC("RlcMacTask::CheckDLMsgQueues", RLC_MAC);
    DBG_ENTER();
    JC_STATUS status = JC_OK;
    BSSGP_DL_UNITDATA_MSG *pDlUnitData;
    bool queueNotEmpty;
    unsigned long firstTlli;
    
    for (int qos = BSSGP_NETWORK_QOS_LEVEL_1; qos < BSSGP_MAX_NETWORK_QOS_LEVELS; qos++)
    {
        queueNotEmpty = bssgp_udt_check_buff_by_qos((BSSGP_NETWORK_QOS_LEVEL)qos, &pDlUnitData);
        if (queueNotEmpty)
        {
            firstTlli = pDlUnitData->tlli.tlli;
        }
        while (queueNotEmpty)
        {
            // If we have run out of resources then quit.
            if (AllocateDLResources(pDlUnitData) == RESOURCES_UNAVAILABLE) break;
            
            // Check next tlli at current QOS level.
            queueNotEmpty = bssgp_udt_check_buff_by_qos((BSSGP_NETWORK_QOS_LEVEL)qos, &pDlUnitData);
            
            // If we circled around to the same tlli then quit.
            if (firstTlli ==  pDlUnitData->tlli.tlli) break;
        }
    }
    DBG_LEAVE();
    return(status);
 }




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::AllocateDLResources
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pUnitDataMsg - BSSGP unit data message
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
DL_ALLOC_RES RlcMacTask::AllocateDLResources(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg) 
{
    DBG_FUNC("RlcMacTask::AllocateDLResources", RLC_MAC);
    DBG_ENTER();
    
    DL_ALLOC_RES allocRes = RESOURCES_UNAVAILABLE;
    
    DBG_TRACE("RLC/MAC: Attempting to establish a new downlink TBF - tlli(%#x)\n",
        pUnitDataMsg->tlli.tlli);
    // First see if the mobile already has an ongoing TBF.
    DLTbf *pDlTbf = tbfPool.FindDLTbf((unsigned long)pUnitDataMsg->tlli.tlli);
    ULTbf *pUlTbf = tbfPool.FindULTbf((unsigned long)pUnitDataMsg->tlli.tlli);
    
    if (!pDlTbf)
    {
        // Try and assign a TBF object for this downlink request.  If none are
        // available then just leave for now.  Either we will reattempt later
        // or the PDU lifetime will expire.
        pDlTbf = tbfPool.EstablishDLTbf(pUnitDataMsg, FALSE);
        
        if (pDlTbf)
        {
            // No uplink TBF exists so send an IMMEDIATE ASSIGNMENT on the CCCH
            if (!pUlTbf)
            {

                // Request only a single PDCH since we must send an IMMEDIATE
                // ASSIGNMENT.  If no resource is available just leave and we
                // will reattempt later.
                if ((DLSingleTSResourceRequest(pDlTbf)) && (pDlTbf->rlcLink.GetNextLLCPdu()))
                {
                    // Let the DSP know about this TBF.
                    pDlTbf->UpdateTSSpecificData();
                    SendDLActivationMsg(pDlTbf);
                    DLCCCHImmediateAssignment(pDlTbf);
                    allocRes = RESOURCES_ALLOCATED;
                } 
                else
                {
                    pDlTbf->ReleaseDLTbf(FALSE);
                }
            }
            // If an uplink TBF exists then we can send a DOWNLINK ASSIGNMENT
            // message on the PACCH.  Because we are sending a DOWNLINK ASSIGNEMENT
            // we can allocate more than one timeslot.
            else
            {
                // Request resources from the scheduler.  If resources are
                // available send a DOWNLINK ASSIGNEMNT on the PACCH. If none are
                // available then just leave for now.  Either we will reattempt later
                // or the PDU lifetime will expire.
                if ((DLResourceRequest(pDlTbf, pUlTbf)) && (pDlTbf->rlcLink.GetNextLLCPdu()))
                {
                    // Let the DSP know about this TBF.
                    pDlTbf->UpdateTSSpecificData();
                    SendDLActivationMsg(pDlTbf);
                    SendPacketDownlinkAssignment(pDlTbf, pUlTbf, FALSE);
                    allocRes = RESOURCES_ALLOCATED;
                } 
                else
                {
                    pDlTbf->ReleaseDLTbf(FALSE);
                }
            } 
        }
    }
    else
    {
        // If a downlink TBF is already ongoing and not inactive just leave.  If the
        // TBF is inactive and Timer T3193 is still runnig then a new downlink TBF can 
        // be establish using the old one.
        if ((pDlTbf->GetTbfState() == Tbf::INACTIVE) && (pDlTbf->rlcLink.IsT3193Running()))
        {
            if (pDlTbf->ReestablishDLTbf(pUnitDataMsg, TRUE) == JC_OK)
            {
                // Note that the return code from the realloc call does not need to be
                // checked.  Either we get more resources or keep the ones we have.  In
                // either case we need to send a downlink assignment.
                if (!pUlTbf)
                {
                    DLResourceReallocRequest(pDlTbf);
                }
                else
                {
                    DLResourceReallocRequest(pDlTbf, pUlTbf);
                }
                
                pDlTbf->UpdateTSSpecificData();
                SendDLReconfigurationMsg(pDlTbf);
                
                if (pDlTbf->rlcLink.GetNextLLCPdu())
                {
                    pDlTbf->rlcLink.CancelT3193();
                    SendPacketDownlinkAssignment(pDlTbf, 0, TRUE);
                    allocRes = RESOURCES_ALLOCATED;
                }
                else
                {
                    DBG_WARNING("RLC/MAC: Unable to extract PDU from BSSGP - tlli(%#x)\n",
                        pUnitDataMsg->tlli.tlli);
                }
            }
        }
        else
        {
            allocRes = TBF_ACTIVE;
        }
    }
    
    DBG_LEAVE();
    return(allocRes);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketDownlinkAssignment
**
**    PURPOSE:  Build and transmit a Packet Downlink Assignment message.
**
**    INPUT PARAMETERS: pDlTbf - pointer to downlink TBF
**                      pUlTbf - pointer to uplink TBF
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPacketDownlinkAssignment(DLTbf *pDlTbf, ULTbf *pUlTbf,
    bool controlAck) 
 {
    DBG_FUNC("RlcMacTask::SendPacketDownlinkAssignment", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader                     asnMsgHdr;
    MsgPacketDownlinkAssignment     asnMsg;
    GammaStruct gammas[MAX_NUM_GAMMAS];

    // Assemble RLC/MAC header.
    asnMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    // RRBP is set.
    asnMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
    asnMsgHdr.SetSPBit(DLMacHeader::RRBP_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Downlink Assignment.
    asnMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    // If the uplink TBF is valid then send on the PACCH of the uplink.
    if (pUlTbf)
    {
        asnMsg.globalTFI.SetGlobalTFI(pUlTbf->tfi.GetTFI(), IeGlobalTFI::UPLINK_TFI);
        asnMsgHdr.SetTFI(pUlTbf->tfi.GetTFI());
        asnMsgHdr.SetDirectionBit(DLMacHeader::UPLINK_TBF);
    }
    else
    {
        asnMsg.globalTFI.SetGlobalTFI(pDlTbf->tfi.GetTFI(), IeGlobalTFI::DOWNLINK_TFI);
        asnMsgHdr.SetTFI(pDlTbf->tfi.GetTFI());
        asnMsgHdr.SetDirectionBit(DLMacHeader::DOWNLINK_TBF);
    }
    asnMsg.macMode.SetMacMode(IeMacMode::DYNAMIC_ALLOCATION);
    if (pDlTbf->qosProfile.a_bit == RADIO_INT_USES_RLC_MAC_ARQ)
    {
        asnMsg.rlcMode.SetRlcMode(IeRlcMode::RLC_ACKNOWLEDGED);
    }
    else
    {
        asnMsg.rlcMode.SetRlcMode(IeRlcMode::RLC_UNACKNOWLEDGED);
    }
    asnMsg.controlAck = controlAck;  // Only true if T3192 is running
    
    unsigned char allocation = 0;
    unsigned char tsBit = 0x80;
    unsigned char tsc;
    unsigned short arfcn;
    
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pDlTbf->allocatedTs[i] == TRUE)
        {
            allocation |= tsBit;
            // Call GRR to get frequency parameters.
            tsc = grr_GetTrxSlotTsc(pDlTbf->trx, i);
            arfcn = grr_GetTrxSlotArfcn(pDlTbf->trx, i);
            
            // Setup gammas for power control
            gammas[i].gamma = 0x16;
            gammas[i].isValid = TRUE;
        }
        else
        {
            gammas[i].isValid = FALSE;
        }
        tsBit >>= 1;
    }
    asnMsg.tsAllocation.SetTimeslotAllocation(allocation);
    asnMsg.timingAdvance.SetPacketTimingAdvance(pDlTbf->tai, pDlTbf->taiTs);
    
    asnMsg.frequencyParameters.tsc.SetTSC(tsc);
    asnMsg.frequencyParameters.arfcn.SetARFCN(arfcn);
    asnMsg.frequencyParameters.SetFrequencyParameters();   
    asnMsg.dlTFIAssignment = pDlTbf->tfi;
    
    // Power control
    asnMsg.powerControlParameters.SetPowerControlParameters(0xa, gammas);
        
    int xmitTs, xmitTrx;
    
    // If there is an uplink TBF send on the uplink PACCH.
    if (pUlTbf)
    {
        TSSelectionRequest(&xmitTrx, &xmitTs, pUlTbf);
    }
    // Otherwise send on the downlink PACCH.
    else
    {
        TSSelectionRequest(&xmitTrx, &xmitTs, pDlTbf);
    }

    // Setup a control ack tag so that the PACKET CONTROL ACK message can
    // be connected to this downlink TBF.
    pDlTbf->SetControlAckTag();
    
    if ((status = SendPacchMsg(xmitTrx, xmitTs, &asnMsgHdr, &asnMsg, pDlTbf->GetControlAckTag(),
        RLC_MAC_ACCESS_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketDownlinkAssignment: failed %#x\n",
            status);       
    }
    
    DBG_LEAVE();
    return(status);
 }



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketDownlinkAssignment
**
**    PURPOSE:  Build and transmit a Packet Downlink Assignment message.  This
**      version is used for reallocation.
**
**    INPUT PARAMETERS: pDlTbf - pointer to downlink TBF
**                      trx - trx to send assignment
**                      ts - timeslot to send assignment
**                      controlAck - controlAck indication
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPacketDownlinkAssignment(DLTbf *pDlTbf, int trx, int ts,
    bool controlAck) 
 {
    DBG_FUNC("RlcMacTask::SendPacketDownlinkAssignment", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader                     asnMsgHdr;
    MsgPacketDownlinkAssignment     asnMsg;

    // Assemble RLC/MAC header.
    asnMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_OPT_OCTETS);
    // RRBP is set.
    asnMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
    asnMsgHdr.SetSPBit(DLMacHeader::RRBP_VALID);
    asnMsgHdr.SetRBSN(0);
    asnMsgHdr.SetRTI(0);
    asnMsgHdr.SetFinalSegment(DLMacHeader::FINAL_SEGMENT);
    asnMsgHdr.SetAddressControl(DLMacHeader::TFI_D_PRESENT);
    asnMsgHdr.SetPowerReduction(DLMacHeader::MODEA_NOT_USABLE_OR_MODEB_24_30_DB);
    // USF will be filled in later.
    
    // Assemble Packet Downlink Assignment.
    asnMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    
    asnMsg.globalTFI.SetGlobalTFI(pDlTbf->tfi.GetTFI(), IeGlobalTFI::DOWNLINK_TFI);
    asnMsgHdr.SetTFI(pDlTbf->tfi.GetTFI());
    asnMsgHdr.SetDirectionBit(DLMacHeader::DOWNLINK_TBF);
        
    asnMsg.macMode.SetMacMode(IeMacMode::DYNAMIC_ALLOCATION);
    if (pDlTbf->qosProfile.a_bit == RADIO_INT_USES_RLC_MAC_ARQ)
    {
        asnMsg.rlcMode.SetRlcMode(IeRlcMode::RLC_ACKNOWLEDGED);
    }
    else
    {
        asnMsg.rlcMode.SetRlcMode(IeRlcMode::RLC_UNACKNOWLEDGED);
    }
    asnMsg.controlAck = controlAck;  // Only true if T3192 is running
    
    unsigned char allocation = 0;
    unsigned char tsBit = 0x80;
    unsigned char tsc;
    unsigned short arfcn;
    
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pDlTbf->allocatedTs[i] == TRUE)
        {
            allocation |= tsBit;
            // Call GRR to get frequency parameters.
            tsc = grr_GetTrxSlotTsc(pDlTbf->trx, i);
            arfcn = grr_GetTrxSlotArfcn(pDlTbf->trx, i);
        }
        tsBit >>= 1;
    }
    asnMsg.tsAllocation.SetTimeslotAllocation(allocation);
    asnMsg.timingAdvance.SetPacketTimingAdvance(pDlTbf->tai, pDlTbf->taiTs);
    
    asnMsg.frequencyParameters.tsc.SetTSC(tsc);
    asnMsg.frequencyParameters.arfcn.SetARFCN(arfcn);
    asnMsg.frequencyParameters.SetFrequencyParameters();   
    asnMsg.dlTFIAssignment = pDlTbf->tfi;
    
    // Start frame number needs to be just slightly longer than the RRBP.
    asnMsg.tbfStartingTime.SetStartFrameNum(2);
    
    // Power control ??
        
    // Setup a control ack tag so that the PACKET CONTROL ACK message can
    // be connected to this downlink TBF.
    pDlTbf->SetControlAckTag();
    
    if ((status = SendPacchMsg(trx, ts, &asnMsgHdr, &asnMsg, pDlTbf->GetControlAckTag(),
        RLC_MAC_ACCESS_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketDownlinkAssignment: failed %#x\n",
            status);       
    }
    
    DBG_LEAVE();
    return(status);
 }




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DLCCCHImmediateAssignment
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pDlTbf - downlink TBF
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::DLCCCHImmediateAssignment(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::DLCCCHImmediateAssignment", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Build an immediate assignment for a downlink TBF.
    T_CNI_LAPDM_L3MessageUnit l3Msg;
    T_CNI_RIL3RRM_MSG ril3RrMsg;
    T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT *pImmAsgnMsg = &ril3RrMsg.immediateAssignment;

    bzero((char *)&ril3RrMsg, sizeof(T_CNI_RIL3RRM_MSG));

    // Populate header
    pImmAsgnMsg->header.protocol_descriminator = CNI_RIL3_PD_RRM;
    pImmAsgnMsg->header.si_ti                  = 0;
    pImmAsgnMsg->header.message_type           = CNI_RIL3RRM_MSGID_IMMEDIATE_ASSIGNMENT;

    // Populate Page Mode
    pImmAsgnMsg->pageMode.ie_present = TRUE;
    pImmAsgnMsg->pageMode.pm         = CNI_RIL3_NORMAL_PAGING;

    // Populate Dedicated Mode or TBF
    pImmAsgnMsg->dmtbf.td_bit           = 1;
    pImmAsgnMsg->dmtbf.downlink_bit     = 1;
    pImmAsgnMsg->dmtbf.tma_bit          = 0;

    // Populate Packet Channel Description
    pImmAsgnMsg->packetChannelDescription.ie_present    = TRUE;
    unsigned char tsc;
    unsigned short arfcn;
    int ts;
    
    // Find the allocated TS.
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        if (pDlTbf->allocatedTs[i] == TRUE)
        {
            ts = i;
            break;
        }
    }
    // Call GRR to get tsc and arfcn.
    tsc = grr_GetTrxSlotTsc(pDlTbf->trx, ts);
    arfcn = grr_GetTrxSlotArfcn(pDlTbf->trx, ts);
    
    pImmAsgnMsg->packetChannelDescription.TN                = ts;
    pImmAsgnMsg->packetChannelDescription.TSC               = tsc;
    pImmAsgnMsg->packetChannelDescription.hopping_indicator = CNI_RIL3_PCD_HOPPING_INDICATOR_NO_HOPPING;
    pImmAsgnMsg->packetChannelDescription.arfcn             = arfcn;
    pImmAsgnMsg->packetChannelDescription.change_mark_1_selector = FALSE;

    // Populate Request Reference
    // For DL TBF this is not used.  However, the frame number needs to be
    // set such that it could not be confused with a response to a
    // CHANNEL REQUEST.
    
    // Setup a control ack tag so that the PACKET CONTROL ACK message can
    // be connected to this downlink TBF.  For immediate assign the tag
    // is going into the random access field.
    pDlTbf->SetControlAckTag();
    
    pImmAsgnMsg->reqReference.ie_present       = TRUE;     
    pImmAsgnMsg->reqReference.randomAccessInfo = pDlTbf->GetControlAckTag();
    pImmAsgnMsg->reqReference.T1               = 0;
    pImmAsgnMsg->reqReference.T2               = 0;
    pImmAsgnMsg->reqReference.T3               = 0;
                                             
    // Populate Timing Advance
    // This IE will be invalidated by the IA Rest Octets
    pImmAsgnMsg->timingAdvance.ie_present = TRUE;
    pImmAsgnMsg->timingAdvance.value      = 0;

    // Populate Mobile Allocation
    // NOTE: Frequency hopping not used so set length to zero
    pImmAsgnMsg->mobileAllocation.ie_present = TRUE;
    pImmAsgnMsg->mobileAllocation.numRFfreq  = 0;

    // Populate Starting Time
    // NOTE: Starting time for packet data is set in the IA Rest Octets
    pImmAsgnMsg->startingTime.ie_present = FALSE;

    // Populate IA Rest Octets
    pImmAsgnMsg->iaRestOctets.ie_present = TRUE;
    pImmAsgnMsg->iaRestOctets.p = CNI_RIL3_PACKET_ASSIGNMENT_DOWNLINK;
    pImmAsgnMsg->iaRestOctets.pda.tlli = pDlTbf->tlli.GetTLLI();
    pImmAsgnMsg->iaRestOctets.pda.flag_tr = TRUE;
    pImmAsgnMsg->iaRestOctets.pda.tfi_assignment = pDlTbf->tfi.GetTFI();
    if (pDlTbf->qosProfile.a_bit == RADIO_INT_USES_RLC_MAC_ARQ)
    {
        pImmAsgnMsg->iaRestOctets.pda.rlc_mode = IeRlcMode::RLC_ACKNOWLEDGED;
    }
    else
    {
        pImmAsgnMsg->iaRestOctets.pda.rlc_mode = IeRlcMode::RLC_UNACKNOWLEDGED;
    }
    pImmAsgnMsg->iaRestOctets.pda.gamma = 0x16;     // 44dbm
    pImmAsgnMsg->iaRestOctets.pda.flag_tr_a = TRUE; // Alpha is present
    pImmAsgnMsg->iaRestOctets.pda.alpha = 0xa;      // 1.0
    pImmAsgnMsg->iaRestOctets.pda.polling = 1;      // enable polling
    pImmAsgnMsg->iaRestOctets.pda.ta_valid = 0;     // TA IE not valid
    pImmAsgnMsg->iaRestOctets.pda.is_timing_advance_index_present = TRUE; 
    pImmAsgnMsg->iaRestOctets.pda.timing_advance_index = pDlTbf->tai;
    // Zero out the start time for now, the DSP will fill in the real value.
    pImmAsgnMsg->iaRestOctets.pda.is_tbf_starting_time_present = TRUE;
    pImmAsgnMsg->iaRestOctets.pda.tbf_starting_time = 0;  
    pImmAsgnMsg->iaRestOctets.pda.flag_pbp = FALSE;
    
    //Call RR message encoding functionality
    T_CNI_RIL3_RESULT ril3Result = CNI_RIL3RRM_Encode( &ril3RrMsg, &l3Msg);

    if (ril3Result == CNI_RIL3_RESULT_SUCCESS)
    {
        // Build DSP message and send it
        unsigned char dspMsg[L1RM_MAXMSG_LEN];
        int length = 4;     // Skip the length field for now
        dspMsg[length++] = RR_MANAGE_MSG;
        dspMsg[length++] = PH_DATA_REQ>>8;  // msb
        dspMsg[length++] = PH_DATA_REQ;     // lsb
        dspMsg[length++] = 0;               // always TRX 0
        dspMsg[length++] = 0x90;            // AGCH
        dspMsg[length++] = 28;
        dspMsg[length++] = 0x20;            // void link
        memcpy(&dspMsg[length], l3Msg.buffer, l3Msg.msgLength);
        length += l3Msg.msgLength - 4;
        dspMsg[3] = length>>24;
        dspMsg[2] = length>>16;
        dspMsg[1] = length>>8;
        dspMsg[0] = length;
    
        PostL3SendMsLog( &l3Msg );
        sendDsp(dspMsg, length+4);
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessChanReq: RIL3 ImmediateAssign encode failed\n");
        status = JC_INVALID_MSG;        
    }
    
    DBG_LEAVE();
    return(status);
}


