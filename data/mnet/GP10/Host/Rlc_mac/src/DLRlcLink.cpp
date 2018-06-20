// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLRlcLink.cpp
// Author(s)   : Tim Olson
// Create Date : 1/5/2001
// Description : 
//
// *******************************************************************


#include <ctype.h>
#include "DLRlcLink.h"
#include "RlcMacTask.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"
#include "bssgp\bssgp_unitdata.h"
#include "MsgPacketPowerCtlTimingAdvance.h"
#include "MsgPacketPollingRequest.h"
#include "MsgPacketResourceRequest.h"

void HexDumper(UINT8 *buffer, int length );

// Static uplink data block pool.
DLDataBlockPool DLRlcLink::sysDlDataBlockPool;

int DLRlcLink::dlAssignCntMax = 3;

int RlcMacNumBlocksBetweenPolls = 8;

DLRlcLink::DLRlcLink() : k(RCV_WINDOW_SIZE_K)
{
    rlcMode = ACKNOWLEDGED_RLC_MODE;
    
    // V(S), V(A) and V(CS) are initially set to 0.
    vS = 0;
    vA = 0;
    vCS = 0;
    
    // Set V(B) to INVALID.
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        vB[i] = INVALID;
        pDlBlocks[i] = 0;
    }
    
    pAssociatedTbf = 0;
    lastPendingAckXmit = 0;
    lastDataBlock = LAST_BLOCK_UNKNOWN;
    dlAssignCnt = 0;
    
    pT3195 = new JCCTimer((FUNCPTR)DLRlcLink::T3195TimerExpiry, 0);
    pT3191 = new JCCTimer((FUNCPTR)DLRlcLink::T3191TimerExpiry, 0);
    pT3193 = new JCCTimer((FUNCPTR)DLRlcLink::T3193TimerExpiry, 0);
    pActivityTimer = new JCCTimer((FUNCPTR)DLRlcLink::ActivityTimerExpiry, 0);
    isT3193Running = FALSE;
    
    // For now we will start with channel coding set to CS-0
    chanCoding.SetChannelCodingCommand(IeChannelCodingCommand::CS_1);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::EstablishRLCLink
**
**    PURPOSE: Initialize all the RLC state information for a new downlink TBF.
**
**    INPUT PARAMETERS: pDlTbf - pointer to tbf object
**                      mode - RLC Mode (acked or not acked)
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS DLRlcLink::EstablishRLCLink(DLTbf *pDlTbf, RLC_MODE mode)
{
    DBG_FUNC("DLRlcLink::EstablishRLCLink", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Hang on to the tbf object. 
    pAssociatedTbf = pDlTbf;
   
    // Clear downlink PDU pointer.
    llcPdu = 0;
    
    rlcMode = mode;
    
    // V(S), V(A) and V(CS) are initially set to 0.
    vS = 0;
    vA = 0;
    vCS = 0;
    
    // Set V(B) to INVALID.
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        vB[i] = INVALID;
        pDlBlocks[i] = 0;
    }
    
    lastPendingAckXmit = 0;
    lastDataBlock = LAST_BLOCK_UNKNOWN;
    lastBlockSentOnPollTs = FALSE;
    dlAssignCnt = 0;
    pollOnActivation = FALSE;
    
    // For now we will start with channel coding set to CS-1
    chanCoding.SetChannelCodingCommand(IeChannelCodingCommand::CS_1);
    
    pDlTbf->ClearControlAckTag();
    
    // Clear counters.
    n3105 = 0;
    numBlocksSincePoll = RlcMacNumBlocksBetweenPolls;
    
    // Cancel all timers
    pT3195->cancelTimer();
    pT3191->cancelTimer();
    pT3193->cancelTimer();
    isT3193Running = FALSE;
    
	// Start Activity Timer
	pActivityTimer->setTimerMSec(MAX_ALLOWED_INACTIVE_TIME, (int)pAssociatedTbf);

    DBG_TRACE("RLC/MAC: Downlink RLC link established tbf(%#x)\n", pDlTbf);
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        DBG_TRACE("\tRLC Mode (ACKNOWLEDGED) V(S) = %d  V(A) = %d  V(CS) = %d\n", 
            vS, vA, vCS);
    else    
        DBG_TRACE("\tRLC Mode (UNACKNOWLEDGED) V(S) = %d  V(A) = %d  V(CS) = %d\n", 
            vS, vA, vCS);
                
    DBG_LEAVE();
    return (status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ReestablishRLCLink
**
**    PURPOSE: Initialize all the RLC state information for an existing DL TBf
**      whose T3192 timer is still running.
**
**    INPUT PARAMETERS: pDlTbf - pointer to tbf object
**                      mode - RLC Mode (acked or not acked)
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS DLRlcLink::ReestablishRLCLink(DLTbf *pDlTbf, RLC_MODE mode)
{
    DBG_FUNC("DLRlcLink::ReestablishRLCLink", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Hang on to the tbf object. 
    pAssociatedTbf = pDlTbf;
   
    // Clear downlink PDU pointer.
    llcPdu = 0;
    
    rlcMode = mode;
    
    // V(S), V(A) and V(CS) are initially set to 0.
    vS = 0;
    vA = 0;
    vCS = 0;
    
    // Set V(B) to INVALID.
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        vB[i] = INVALID;
        pDlBlocks[i] = 0;
    }
    
    lastPendingAckXmit = 0;
    lastDataBlock = LAST_BLOCK_UNKNOWN;
    lastBlockSentOnPollTs = FALSE;
    dlAssignCnt = 0;
    pollOnActivation = FALSE;
    
    // For now we will start with channel coding set to CS-1
    chanCoding.SetChannelCodingCommand(IeChannelCodingCommand::CS_1);
    
    pDlTbf->ClearControlAckTag();
    
    // Clear counters.
    n3105 = 0;
    numBlocksSincePoll = RlcMacNumBlocksBetweenPolls;
    
    // Cancel all timers
    pT3195->cancelTimer();
    pT3191->cancelTimer();
    
	// Start Activity Timer
	pActivityTimer->setTimerMSec(MAX_ALLOWED_INACTIVE_TIME, (int)pAssociatedTbf);

    DBG_TRACE("RLC/MAC: Downlink RLC link reestablished tbf(%#x)\n", pDlTbf);
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        DBG_TRACE("\tRLC Mode (ACKNOWLEDGED) V(S) = %d  V(A) = %d  V(CS) = %d\n", 
            vS, vA, vCS);
    else    
        DBG_TRACE("\tRLC Mode (UNACKNOWLEDGED) V(S) = %d  V(A) = %d  V(CS) = %d\n", 
            vS, vA, vCS);
                
    DBG_LEAVE();
    return (status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::UpdateTSSpecificData
**
**    PURPOSE: This function will be called after timeslots are allocated.  Any
**      data that requires timeslot information should be set here.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::UpdateTSSpecificData()
{
    DBG_FUNC("DLRlcLink::UpdateTSSpecificData", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    int numTs = 0;
    
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pAssociatedTbf->allocatedTs[i]) numTs++;
    }
    
    N3105_MAX = numTs * 5;
    
    DBG_LEAVE();
    return;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::AbnormalRelease
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS DLRlcLink::AbnormalRelease()
{
    DBG_FUNC("DLRlcLink::AbnormalRelease", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Inactivate the tbf so that no more scheduling occurs.
    pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
    
    // Send a PACKET TBF RELEASE message.
    status = RlcMacTask::theRlcMacTask->SendPacketTBFRelease(pAssociatedTbf, 
        0, MsgPacketTBFRelease::ABNORMAL_RELEASE);
    
    // Use T3195 to allow enough time to send the deactivate message.
    pT3195->cancelTimer();
    pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);
    
    DBG_LEAVE();
    return (status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::GetDLRlcBlock
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: ctlAckTag - optional tag used for acknowledged control msgs
**                      burstType - if this message is acknowledged then burstType
**                          indicates whether the response is normal or access bursts
**                      reqTrx - trx requested for sending this block
**                      reqTs - ts requested for sending this block
**
**    RETURN VALUE(S): pointer to data block if available
**                     0 - block not available
**
**----------------------------------------------------------------------------*/
DLDataBlock *DLRlcLink::GetDLRlcBlock(unsigned char &ctrlAckTag, char &burstType,
    int reqTrx, int reqTs) 
{
    DBG_FUNC("DLRlcLink::GetDLRlcBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DLDataBlock *pDlDataBlock = 0;
    
    // If we are in acknowledged mode then check to see if any NACKED blocks
    // need to be retransmitted.
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
    {
        if ((pDlDataBlock = RetransmitNackedDLRlcBlock()))
        {
            if (SetPollingBit(pDlDataBlock, ctrlAckTag, burstType, reqTrx, reqTs))
                pDlDataBlock = 0;
            goto BlockFound;    
        }
    }
    
    // Next see if we can can send a new RLC block.
    if ((pDlDataBlock = BuildDLRlcBlock()))
    {
        if (SetPollingBit(pDlDataBlock, ctrlAckTag, burstType, reqTrx, reqTs))
                pDlDataBlock = 0;
            goto BlockFound;    
    }
    
    // Finally see if we can transmit any PENDING ACK blocks.
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
    {
        if ((pDlDataBlock = RetransmitPendingAckDLRlcBlock()))
        {
            if (SetPollingBit(pDlDataBlock, ctrlAckTag, burstType, reqTrx, reqTs))
                pDlDataBlock = 0;
            goto BlockFound;    
        }
    }

BlockFound:      
    if (pDlDataBlock)
    {
        DBG_TRACE("RLC/MAC: Sending downlink RLC block - tbf(%#x)\n", pAssociatedTbf);
        DBG_TRACE("\tBSN = %d  V(S) = %d   V(A) = %d   V(CS) = %d\n", 
            pDlDataBlock->dlHeader.GetBSN(), vS, vA, vCS); 
    }
    else
    {
        DBG_TRACE("RLC/MAC: No downlink RLC blocks available - tbf(%#x) trx(%d) ts(%d)\n", 
            pAssociatedTbf, reqTrx, reqTs);
    }
      
	// Start Activity Timer
	pActivityTimer->setTimerMSec(MAX_ALLOWED_INACTIVE_TIME, (int)pAssociatedTbf);
        
    DBG_LEAVE();
    return (pDlDataBlock);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::SetPollingBit
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pDlDataBlock - current data block
**                      ctlAckTag - optional tag used for acknowledged control msgs
**                      burstType - if this message is acknowledged then burstType
**                          indicates whether the response is normal or access bursts
**                      reqTrx - trx requested for sending this block
**                      reqTs - ts requested for sending this block
**
**    RETURN VALUE(S): TRUE - the downlink radio link is lost or no block available
**                     FALSE - downlink radio link is ok
**
**----------------------------------------------------------------------------*/
bool DLRlcLink::SetPollingBit(DLDataBlock *pDlDataBlock, 
    unsigned char &ctrlAckTag, char &burstType, int reqTrx, int reqTs) 
{
    DBG_FUNC("DLRlcLink::SetPollingBit", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    bool downlinkFail = FALSE;
    ctrlAckTag = CTL_ACK_TAG_NOT_APPLICABLE;
    burstType = RLC_MAC_NORMAL_BURST;
    
    // If this is the last block then the RRBP is alreay set to valid.  A
    // control ack tag needs to be saved though.  Inactivate the the TBF
    // so no more blocks are transmitted. 
    if ((lastDataBlock != LAST_BLOCK_UNKNOWN) && (pDlDataBlock == pDlBlocks[lastDataBlock]))
    {
        // If the final block was not sent on a valid polling timeslot, then
        // don't send anything.
        if (RlcMacTask::theRlcMacTask->PollingTsCheck(reqTrx, reqTs, pAssociatedTbf))
        {
            // If we have reached the max 3105 count then internally release the
            // TBF.  When T3195 expires release the resources.
            if (n3105++ >= N3105_MAX)
            {
                pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);
                pAssociatedTbf->InternalReleaseDLTbf();
                downlinkFail = TRUE;
                DBG_TRACE("RLC/MAC: N3105 = N3105Max - TBF(%#x)\n", pAssociatedTbf);
            }
            else
            {
                // Reset polling counter.
                numBlocksSincePoll = RlcMacNumBlocksBetweenPolls;
        
                pAssociatedTbf->SetControlAckTag();
                ctrlAckTag = pAssociatedTbf->GetControlAckTag();
        
                if (rlcMode == UNACKNOWLEDGED_RLC_MODE)
                {
                    burstType = RLC_MAC_ACCESS_BURST;
                }
            
                // Inactivate the link so that no more data blocks are transmitted.
                pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
                lastBlockSentOnPollTs = TRUE;
            }
        }
        // Set downlinkFail to TRUE so no data block is returned.
        else
        {
            downlinkFail = TRUE;
        }
    }
    else 
    {
        // Default to not valid.
        pDlDataBlock->dlHeader.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
        
        // If the count was zero or is now zero see if we can set RRB.
        if ((numBlocksSincePoll ? --numBlocksSincePoll : numBlocksSincePoll) <= 0)
        {
            // If the requested timeslot matches the allowed timeslot then set RRB.
            if (RlcMacTask::theRlcMacTask->PollingTsCheck(reqTrx, reqTs, pAssociatedTbf))
            {
                pDlDataBlock->dlHeader.SetSPBit(DLMacHeader::RRBP_VALID);
                numBlocksSincePoll = RlcMacNumBlocksBetweenPolls;
                if (n3105++ >= N3105_MAX)
                {
                    pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);
                    pAssociatedTbf->InternalReleaseDLTbf();
                    downlinkFail = TRUE;
                    DBG_TRACE("RLC/MAC: N3105 = N3105Max - TBF(%#x)\n", pAssociatedTbf);
                }
                else
                {
                    pDlDataBlock->dlHeader.SetSPBit(DLMacHeader::RRBP_VALID);
                    numBlocksSincePoll = RlcMacNumBlocksBetweenPolls;
                } 
            }
        }
    }
    
    DBG_LEAVE();
    return(downlinkFail);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::BuildDLRlcBlock
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): pointer to data block if available
**                     0 - block not available
**
**----------------------------------------------------------------------------*/
DLDataBlock *DLRlcLink::BuildDLRlcBlock() 
{
    DBG_FUNC("DLRlcLink::BuildDLRlcBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DLDataBlock *pDlDataBlock = 0;
    
    // First determine if another data block is needed.  Check to see if we
    // already sent a block with the FBI set.
    if (lastDataBlock == LAST_BLOCK_UNKNOWN)
    {
        // If more data left in the current PDU or there is another PDU
        // available and we are not stalled then build a new
        // RLC data block.
        if (((vS != ((vA + k) % RLC_MOD_VAL))) || (rlcMode == UNACKNOWLEDGED_RLC_MODE))
        {
            // Free data block in unacknowledged mode if needed.
            if (rlcMode == UNACKNOWLEDGED_RLC_MODE)
            {
                if (pDlBlocks[vS])
                {
                    sysDlDataBlockPool.ReturnDLDataBlock(pDlBlocks[vS]);
                }
            }
            // Grab a new data block.
            pDlDataBlock = sysDlDataBlockPool.GetDLDataBlock();
            pDlBlocks[vS] = pDlDataBlock;
            
            // Set the coding rate for this data block.
            pDlDataBlock->chanCoding = chanCoding;
            
            // Assemble RLC/MAC header.
            pDlDataBlock->dlHeader.SetPayloadType(DLMacHeader::RLC_DATA_BLOCK);
            pDlDataBlock->dlHeader.SetRRBP(DLMacHeader::FN_N_13);
            pDlDataBlock->dlHeader.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
            // USF will be filled in later.
            pDlDataBlock->dlHeader.SetPowerReduction(DLMacHeader::MODEA_NOT_USABLE_OR_MODEB_24_30_DB);
            pDlDataBlock->dlHeader.SetTFI(pAssociatedTbf->tfi.GetTFI());
            // Final Block Indicator set a little later.
            pDlDataBlock->dlHeader.SetBSN(vS);
            // Start off assuming there are no extension octets.
            pDlDataBlock->dlHeader.SetExtensionBit(DLMacHeader::NO_EXTENSION_OCTET);
            // Start off  assuming not the last block.
            pDlDataBlock->dlHeader.SetFinalBlockIndicator(DLMacHeader::NOT_LAST_RLC_BLOCK);
            // Initialize the length values for the empty data block.
            pDlDataBlock->streamOutLen = 0;
            // Start with the minimum header size.
            pDlDataBlock->dlHeaderLen = DLMacHeader::GetDataHdrSize();

            int bytesCopied;
            while (!SegmentLLCPdu(pDlDataBlock, bytesCopied))
            {
                // This happens if a PDU plus length indicator fit exactly in one RLC block
                // and more PDUs are to come.
                if (bytesCopied == 0)
                {
                    // If length octet has been added then set it so that no extension octets
                    // will follow.
                    if (pDlDataBlock->dlHeader.GetCurrLengthOctetIndex())
                    {
                        // Must back up by one to get to the last length indicator.
                        pDlDataBlock->dlHeader.DecrementCurrLengthOctetIndex();
                        pDlDataBlock->dlHeader.SetCurrLengthOctetExtension(DLMacHeader::NO_EXTENSION_OCTET);
                    }
                    break;
                }
                
                // If another PDU is queued up then grab it.
                if (GetNextLLCPdu())
                {
                    // Add a length indicator for the last group of bytes copied.  For now
                    // default it so that a length indicator will follow.
                    pDlDataBlock->dlHeader.SetLengthOctet(bytesCopied, DLMacHeader::MORE_LLC_DATA, 
                        DLMacHeader::EXTENSION_OCTET_FOLLOWS);
                    pDlDataBlock->dlHeader.IncrementCurrLengthOctetIndex();        
                    pDlDataBlock->dlHeaderLen++;
                    
                    // Try and reallocate more resources.
                    ReallocateDLResources();
                }
                // No more PDU's.
                else
                {
                    // Add a length indicator for the last group of bytes copied.
                    pDlDataBlock->dlHeader.SetLengthOctet(bytesCopied, DLMacHeader::NO_MORE_LLC_DATA, 
                        DLMacHeader::NO_EXTENSION_OCTET);
                    pDlDataBlock->dlHeaderLen++;
                    pDlDataBlock->dlHeader.SetFinalBlockIndicator(DLMacHeader::LAST_RLC_BLOCK);
                    pDlDataBlock->dlHeader.SetSPBit(DLMacHeader::RRBP_VALID);
                    lastDataBlock = vS;
                    pT3191->setTimerMSec(T3191_VALUE, (int)pAssociatedTbf);
                    break;
                }
            }
            
            // Update the value of V(S) and V(B).
            vB[vS] = PENDING_ACK;
            vS = ++vS % RLC_MOD_VAL;
        }
    }
        
    DBG_LEAVE();
    return (pDlDataBlock);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::SegmentLLCPdu
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pDlDataBlock - pointer to current data block
**
**    RETURN VALUE(S): TRUE - if PDU is filled
**                     FALSE - if PDU is not completely filled
**
**----------------------------------------------------------------------------*/
bool DLRlcLink::SegmentLLCPdu(DLDataBlock *pDlDataBlock, int &bytesCopied) 
{
    DBG_FUNC("DLRlcLink::SegmentLLCPdu", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    bool rlcBlockFull = FALSE;
    BSSGP_DL_UNITDATA_MSG *pDlUnitData;
    
    // First copy as much of the PDU as possible..
    bytesCopied = min(pDlDataBlock->chanCoding.GetRLCDataBlockSize() - 
        pDlDataBlock->dlHeaderLen - pDlDataBlock->streamOutLen, llcPduLen - currPduByte);
    bcopy(llcPdu + currPduByte, (char *)pDlDataBlock->streamOut + pDlDataBlock->streamOutLen, 
        bytesCopied);
    pDlDataBlock->streamOutLen += bytesCopied;
    currPduByte += bytesCopied;

    // We were able to fill an entire RLC block
    if ((pDlDataBlock->streamOutLen + pDlDataBlock->dlHeaderLen == 
        pDlDataBlock->chanCoding.GetRLCDataBlockSize()) && bytesCopied)
    {
        // A special case may exist if an LLC PDU ends on an RLC block
        // boundary.
        if (llcPduLen == currPduByte)
        {
            // If another LLC PDU is waiting then we have to add a length indicator.
            if (MoreLLCPdusAvailable(&pDlUnitData))
            {
                // Remove a byte from the RLC block.  It will be sent in the next
                // RLC block.  Add a length octet of zero.
                currPduByte--;
                pDlDataBlock->streamOutLen--;
                pDlDataBlock->dlHeader.SetFinalBlockIndicator(DLMacHeader::NOT_LAST_RLC_BLOCK);
                pDlDataBlock->dlHeader.SetExtensionBit(DLMacHeader::EXTENSION_OCTET_FOLLOWS);
                pDlDataBlock->dlHeader.SetLengthOctet(0, 
                    DLMacHeader::MORE_LLC_DATA, DLMacHeader::NO_EXTENSION_OCTET);
                pDlDataBlock->dlHeaderLen++;
            }
            // If no more PDUs are waiting then we can end the TBF.
            else
            {
                pDlDataBlock->dlHeader.SetFinalBlockIndicator(DLMacHeader::LAST_RLC_BLOCK);
                pDlDataBlock->dlHeader.SetSPBit(DLMacHeader::RRBP_VALID);
                // If length octet has been added then set it so that no extension octets
                // will follow.
                if (pDlDataBlock->dlHeader.GetCurrLengthOctetIndex())
                {
                    // Must back up by one to get to the last length indicator.
                    pDlDataBlock->dlHeader.DecrementCurrLengthOctetIndex();
                    pDlDataBlock->dlHeader.SetCurrLengthOctetExtension(DLMacHeader::NO_EXTENSION_OCTET);
                }
                lastDataBlock = vS;
                pT3191->setTimerMSec(T3191_VALUE, (int)pAssociatedTbf);
            }
        }
        else
        {
            // If length octet has been added then set it so that no extension octets
            // will follow.
            if (pDlDataBlock->dlHeader.GetCurrLengthOctetIndex())
            {
                // Must back up by one to get to the last length indicator.
                pDlDataBlock->dlHeader.DecrementCurrLengthOctetIndex();
                pDlDataBlock->dlHeader.SetCurrLengthOctetExtension(DLMacHeader::NO_EXTENSION_OCTET);
            }
        }
        
        rlcBlockFull = TRUE;
    }
    // The block was not filled completely so for sure we will have at least one
    // length indicator.
    else
    {
        // Make sure the extension bit was set.
        pDlDataBlock->dlHeader.SetExtensionBit(DLMacHeader::EXTENSION_OCTET_FOLLOWS);
    }
    
    DBG_LEAVE();
    return (rlcBlockFull);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::RetransmitNackedDLRlcBlock
**
**    PURPOSE:  Search V(B) and retransmit the oldest block that in the 
**      NACKED state.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): pointer to data block if available
**                     0 - block not available
**
**----------------------------------------------------------------------------*/
DLDataBlock *DLRlcLink::RetransmitNackedDLRlcBlock() 
{
    DBG_FUNC("DLRlcLink::RetransmitNackedDLRlcBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();

    // Search V(B) from V(A) to V(S).  If any blocks are in the NACKED state
    // then resend it.
    int currDataBlock = vA;
    DLDataBlock *pDlDataBlock = 0;
    
    while (currDataBlock != vS)
    {
        if (vB[currDataBlock] == NACKED)
        {
            pDlDataBlock = pDlBlocks[currDataBlock];
            vB[currDataBlock] = PENDING_ACK;
            lastPendingAckXmit = vA;
            // If we are retransmitting the last data block then restart T3191.
            if (currDataBlock == lastDataBlock)
            {
                pT3191->setTimerMSec(T3191_VALUE, (int)pAssociatedTbf);
            }
            
            break;
        }  
        currDataBlock = ++currDataBlock % RLC_MOD_VAL;
    }
    
    DBG_LEAVE();
    return(pDlDataBlock);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::RetransmitPendingAckDLRlcBlock
**
**    PURPOSE:  Search V(B) and retransmit the oldest block that in the 
**      PENDING_ACK state.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): pointer to data block if available
**                     0 - block not available
**
**----------------------------------------------------------------------------*/
DLDataBlock *DLRlcLink::RetransmitPendingAckDLRlcBlock() 
{
    DBG_FUNC("DLRlcLink::RetransmitPendingAckDLRlcBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();

    // Search V(B) from V(A) to V(S).  If any blocks are in the PENDING_ACK state
    // then resend it.
    int currDataBlock = lastPendingAckXmit;
    DLDataBlock *pDlDataBlock = 0;
    
    // If the last block is known but is has not been sent on a valid polling
    // timeslot then send it again.
    if ((lastDataBlock != LAST_BLOCK_UNKNOWN) && !lastBlockSentOnPollTs)
    {
        pDlDataBlock = pDlBlocks[lastDataBlock];  
        pT3191->setTimerMSec(T3191_VALUE, (int)pAssociatedTbf);
    }
    else
    {
        while (currDataBlock != vS)
        {
            if (vB[currDataBlock] == PENDING_ACK)
            {
                pDlDataBlock = pDlBlocks[currDataBlock];
                // If we are retransmitting the last data block then restart T3191.
                if (currDataBlock == lastDataBlock)
                {
                    pT3191->setTimerMSec(T3191_VALUE, (int)pAssociatedTbf);
                }
                lastPendingAckXmit = ++currDataBlock % RLC_MOD_VAL;
                if (lastPendingAckXmit == vS) lastPendingAckXmit = vA;
                return(pDlDataBlock);
            } 
            currDataBlock = ++currDataBlock % RLC_MOD_VAL; 
        }
    
        // If there are no blocks set to PENDING ACK then retransmit the last.
        if (lastDataBlock != LAST_BLOCK_UNKNOWN)
        {
            pDlDataBlock = pDlBlocks[lastDataBlock];  
            pT3191->setTimerMSec(T3191_VALUE, (int)pAssociatedTbf);
        }  
    }
    
    DBG_LEAVE();
    return(pDlDataBlock);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::GetNextLLCPdu
**
**    PURPOSE: Check BSSGPs buffers to see if there are more PDUs for this
**      TLLI.  If there is on then get it. 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): TRUE - if there are more PDUs for this TLLI.
**                     FALSE - if no more PDUs for this TLLI.
**
**----------------------------------------------------------------------------*/
bool DLRlcLink::GetNextLLCPdu()
{
    DBG_FUNC("DLRlcLink::GetNextLLCPdu", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    BSSGP_DL_UNITDATA_MSG unitDataMsg;
    unsigned char *pPdu;
    bool ret = FALSE;
    
    pPdu = bssgp_udt_get_buff_by_tlli(pAssociatedTbf->tlli.GetTLLI(), &unitDataMsg);
    
    if (pPdu)
    {
        // If a prior PDU exists return it to the bssgp pool.
        if (llcPdu)
        {
            bssgp_util_return_rx_buff(llcPdu);
        }
        
        // Save LLC PDU
        llcPdu = (char *)unitDataMsg.pdu.data;
        llcPduLen = unitDataMsg.pdu.data_len;
        currPduByte = 0;
        ret = TRUE;
        DBG_TRACE("DLRlcLink: Processing downlink PDU - TLLI(%#x)\n", pAssociatedTbf->tlli.GetTLLI());
        HexDumper(unitDataMsg.pdu.data, unitDataMsg.pdu.data_len);
    }
    
    DBG_LEAVE();
    return (ret);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::MoreLLCPdusAvailable
**
**    PURPOSE:  Check BSSGPs buffers to see if there are more PDUs for this
**      TLLI.
**
**    INPUT PARAMETERS: pDlUnitData - pointer to the new data unit.
**
**    RETURN VALUE(S): TRUE - if there are more PDUs for this TLLI.
**                     FALSE - if no more PDUs for this TLLI.
**
**----------------------------------------------------------------------------*/
bool DLRlcLink::MoreLLCPdusAvailable(BSSGP_DL_UNITDATA_MSG **pDlUnitData)
{
    DBG_FUNC("DLRlcLink::MoreLLCPdusAvailable", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    DBG_LEAVE();
    return (bssgp_udt_check_buff_by_tlli(pAssociatedTbf->tlli.GetTLLI(), pDlUnitData));
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ReallocateDLResources
**
**    PURPOSE:  Check to see if it would be worth adding more resources for
**      this TBF.  If so then send a new DOWNLINK ASSIGNMENT.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): TRUE - if resources were reallocated.
**                     FALSE - otherwise.
**
**----------------------------------------------------------------------------*/
bool DLRlcLink::ReallocateDLResources()
{
    DBG_FUNC("DLRlcLink::ReallocateDLResources", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    bool allocated = FALSE;
    int trx, ts;
    
    DBG_TRACE("RLC_MAC: Attempting to reallocate resources for DL Tbf (%#x)\n", pAssociatedTbf);
    // Save the trx and a timeslot from the old allocation needed to send the 
    // potential DOWNLINK ASSIGNMENT.
    RlcMacTask::theRlcMacTask->TSSelectionRequest(&trx, &ts, pAssociatedTbf);
    
    ULTbf *pUlTbf = RlcMacTask::theRlcMacTask->tbfPool.FindULTbf(
        (unsigned long)pAssociatedTbf->tlli.GetTLLI());
    
    // Check if there is an uplink TBF ongoing.
    if (pUlTbf)
    {
        allocated = RlcMacTask::theRlcMacTask->DLResourceReallocRequest(pAssociatedTbf, pUlTbf);
    }
    else
    {
        allocated = RlcMacTask::theRlcMacTask->DLResourceReallocRequest(pAssociatedTbf);
    }
    
    if (allocated)
    {
        // Update any timslot quantity information.
        pAssociatedTbf->UpdateTSSpecificData();
        
        // Set the tbf to reallocating so that no more scheduling occurs.
        pAssociatedTbf->SetTbfState(Tbf::REALLOCATING);
        
        // Send a DOWNLINK ASSIGNMENT to the mobile.
        RlcMacTask::theRlcMacTask->SendPacketDownlinkAssignment(pAssociatedTbf, trx, ts, FALSE);
    }
    
    DBG_LEAVE();
    return (allocated);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ReceiveDLAckNack
**
**    PURPOSE:  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ReceiveDLAckNack(MsgPacketDownlinkAckNack *pCtrlMsg)
{
    DBG_FUNC("DLRlcLink::ReceiveDLAckNack", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: Received PACKET DOWNLINK ACK/NACK - TBF(%#x)\n",
        pAssociatedTbf);
    
    if (pAssociatedTbf->GetTbfState() != Tbf::RELEASING)
    {
        pT3191->cancelTimer();
        n3105 = 0;
    
        // If there is a channel request then go assign some uplink resources.
        if (pCtrlMsg->chanReqDescription.IsValid())
        {
            // Build a PACKET RESOURCE REQUEST message using the IE's from the
            // DOWNLINK ACK/NACK message.
            MsgPacketResourceRequest reqMsg;
            reqMsg.globalTfi.SetGlobalTFI(pAssociatedTbf->tfi.GetTFI(), IeGlobalTFI::DOWNLINK_TFI);
            reqMsg.msRadioAccessCapability = pAssociatedTbf->raCapability;
            reqMsg.chanReqDescription = pCtrlMsg->chanReqDescription;
        
            int ts, trx;
            RlcMacTask::theRlcMacTask->TSSelectionRequest(&trx, &ts, pAssociatedTbf);
        
            RlcMacTask::theRlcMacTask->AllocateULResources(reqMsg, trx, ts);
        }
    
        // Send DL ack nack information to the DSP's.
        RlcMacTask::theRlcMacTask->SendDLAckNackInfoMsg(pAssociatedTbf->trx, 
            pAssociatedTbf, pCtrlMsg);
        
        if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        {
            IeAckNackDescription::FINAL_ACK_INDICATION finalAckInd;
            unsigned char ssn;
            unsigned char rrb[RRB_SIZE];
    
            pCtrlMsg->ackNackDescription.GetAckNackDescription(&finalAckInd, &ssn, rrb);
    
            // The final ACK indication can acknowledge the entire TBF.
            if (finalAckInd == IeAckNackDescription::TBF_COMPLETE)
            {
                // Update V(B) to show all blocks have been acked.
                while (vA != vS)
                {
                    vB[vA] = ACKED;
                    vA = ++vA % RLC_MOD_VAL;
                }
        
                // Start T3193 so that TBF can be released.
                pT3193->setTimerMSec(T3193_VALUE, (int)pAssociatedTbf);
                isT3193Running = TRUE;
        
                // If more PDUs are available for this MS then go ahead and allocate
                // more resources now.
                BSSGP_DL_UNITDATA_MSG *pDlUnitData;
                if (MoreLLCPdusAvailable(&pDlUnitData))
                {
                    RlcMacTask::theRlcMacTask->AllocateDLResources(pDlUnitData);  
                }
            }
            else
            {
                // Update V(B) according to the SSN and RRB values.
                int vBIndex = (ssn == 0) ? RLC_MOD_VAL - 1 : ssn - 1;
                int rrbIndex = RRB_SIZE-1;
                unsigned char bitPos = 1;
                int tmpVA = ssn;
        
                while (1)
                {
                    if (rrb[rrbIndex] & bitPos)
                    {
                        vB[vBIndex] = ACKED;
                    }
                    else
                    {
                        vB[vBIndex] = NACKED;
                        tmpVA = vBIndex;
                    }
            
                    if (!(bitPos <<= 1))
                    {
                        // Next byte in RRB.
                        bitPos = 1;
                        rrbIndex--;
                    }
            
                    if (vBIndex == vA) break;
                    else if (--vBIndex < 0) vBIndex = RLC_MOD_VAL - 1;
            
                }
        
                // Invalidate blocks outside the transmit window and update 
                // V(A) to be the oldest NACKED block.
                vBIndex = vA;
                while (vBIndex != tmpVA)
                {
                    vB[vBIndex] = INVALID;
                    sysDlDataBlockPool.ReturnDLDataBlock(pDlBlocks[vBIndex]);
                    pDlBlocks[vBIndex] = 0;
                    vBIndex = ++vBIndex % RLC_MOD_VAL;
                }
                vA = tmpVA;
            
                // If the last block has been sent and the current state is not
                // ALLOCATED then we need to start up the TBF again.
                if ((lastDataBlock != LAST_BLOCK_UNKNOWN) &&
                    (pAssociatedTbf->GetTbfState() != Tbf::ALLOCATED))
                {
                    // Enable downlink scheduling for this TBF.
                    pAssociatedTbf->SetTbfState(Tbf::ALLOCATED);
                    lastBlockSentOnPollTs = FALSE;
        
                    // Make sure the RTS mechanism is triggered.
                    for (int i = 0; i < MAX_TIMESLOTS; i++)
                    {
                        if (pAssociatedTbf->allocatedTs[i])
                        {
                            RlcMacTask::theRlcMacTask->SendDownlinkDataBlock(pAssociatedTbf->trx, i);
                        }
                    } 
                }
            }
        }  
    }
    
    DBG_LEAVE();
    return;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ProcessT3195Expiry
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ProcessT3195Expiry()
{
    DBG_FUNC("DLRlcLink::ProcessT3195Expiry", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: T3195 Expired - TBF(%#x)\n", pAssociatedTbf);
    // Canel T3195.
    pT3195->cancelTimer();

	// Cancel Activity Timer
    pActivityTimer->cancelTimer();
    
    // Release DL resources.
    pAssociatedTbf->ReleaseDLTbf(FALSE);
    ReleaseDataBlocks();
    
    RlcMacTask::theRlcMacTask->CheckDLMsgQueues();
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ProcessT3191Expiry
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ProcessT3191Expiry()
{
    DBG_FUNC("DLRlcLink::ProcessT3191Expiry", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: T3191 Expired - TBF(%#x)\n", pAssociatedTbf);
    // Canel T3191.
    pT3191->cancelTimer();
    
	// Cancel Activity Timer
    pActivityTimer->cancelTimer();

    // Release DL resources.
    pAssociatedTbf->ReleaseDLTbf(TRUE);
    ReleaseDataBlocks();
    
    RlcMacTask::theRlcMacTask->CheckDLMsgQueues();
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ProcessT3193Expiry
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ProcessT3193Expiry()
{
    DBG_FUNC("DLRlcLink::ProcessT3193Expiry", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: T3193 Expired - TBF(%#x)\n", pAssociatedTbf);
    // Canel T3193.
    pT3193->cancelTimer();
    isT3193Running = FALSE;
    
	// Cancel Activity Timer
    pActivityTimer->cancelTimer();

    // Release DL resources.
    pAssociatedTbf->ReleaseDLTbf(TRUE);
    ReleaseDataBlocks();
    
    RlcMacTask::theRlcMacTask->CheckDLMsgQueues();
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ProcessActivityTimerExpiry
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ProcessActivityTimerExpiry()
{
    DBG_FUNC("DLRlcLink::ProcessActivityTimerExpiry", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: Activity Timer Expired - TBF(%#x)\n", pAssociatedTbf);
    
    // Release DL resources.
    pAssociatedTbf->ReleaseDLTbf(TRUE);
    ReleaseDataBlocks();
    
    RlcMacTask::theRlcMacTask->CheckDLMsgQueues();
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ReceivePacketControlAck
**
**    PURPOSE:  The Packet Control Ack can be received in two cases:
**              1. As an acknowledgement to the initial assignment.  For this
**                 case the TBF state will be ACTIVATING regardless of RLC mode.
**              2. Confirmation of the end of the TBF in unacknowledged mode.
**
**      NOTE: The packet control acknowledgement may be sent as access bursts.
**          For this case the DSP will measure the timing advance.
**
**    INPUT PARAMETERS: pCtrlMsg - the packet control ack message
**                      ta - timing advance measured by the DSP's.
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ReceivePacketControlAck(MsgPacketControlAcknowledgement *pCtrlMsg,
    unsigned char ta)
{
    DBG_FUNC("DLRlcLink::ReceivePacketControlAck", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: Received PACKET CONTROL ACK - TBF(%#x)\n",
        pAssociatedTbf);
    
    // If the current tbf state is ACTIVATING then we must be confirming the
    // downlink assigment.
    if (pAssociatedTbf->GetTbfState() == Tbf::ACTIVATING)
    {
        // Enable downlink scheduling for this TBF.
        pAssociatedTbf->SetTbfState(Tbf::ALLOCATED);
        
        // If a valid timing advance exists send it to the MS.
        if (ta != 0xff)
        {
            SendPacketPowerControlTimingAdvance(ta);
        }
    
        // Make sure the RTS mechanism is triggered.
        for (int i = 0; i < MAX_TIMESLOTS; i++)
        {
            if (pAssociatedTbf->allocatedTs[i])
            {
                RlcMacTask::theRlcMacTask->SendDownlinkDataBlock(pAssociatedTbf->trx, i);
            }
        } 
    }
    else if (pAssociatedTbf->GetTbfState() == Tbf::REALLOCATING)
    {
        // Enable downlink scheduling for this TBF.
        pAssociatedTbf->SetTbfState(Tbf::ALLOCATED);
    
        // Update the DSP's for the new allocation.
        RlcMacTask::theRlcMacTask->SendDLReconfigurationMsg(pAssociatedTbf);
        
        // Make sure the RTS mechanism is triggered.
        for (int i = 0; i < MAX_TIMESLOTS; i++)
        {
            if (pAssociatedTbf->allocatedTs[i])
            {
                RlcMacTask::theRlcMacTask->SendDownlinkDataBlock(pAssociatedTbf->trx, i);
            }
        } 
    }
    // This must be the conclusion of an unacknowledged transfer.
    else if (rlcMode == UNACKNOWLEDGED_RLC_MODE)
    {
        // Inactivate the link so that no more data blocks are transmitted.
        pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
        
        // If more PDUs are available for this MS then go ahead and allocate
        // more resources now.
        BSSGP_DL_UNITDATA_MSG *pDlUnitData;
        if (MoreLLCPdusAvailable(&pDlUnitData))
        {
            RlcMacTask::theRlcMacTask->AllocateDLResources(pDlUnitData);  
        }
        else
        {
            // Start T3193 so that TBF can be released.
            pT3193->setTimerMSec(T3193_VALUE, (int)pAssociatedTbf);
            isT3193Running = TRUE;
        }
    }
    else
    {
        DBG_WARNING("DLRlcLink::ReceivePacketControlAck: Received PACKET CONTROL ACK "
            "while ALLOCATED and in ACKNOWLEDGED mode - TBF(%#x)\n", pAssociatedTbf);
    }
    
    DBG_LEAVE();
    return;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ReceivePacketControlNack
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ReceivePacketControlNack()
{
    DBG_FUNC("DLRlcLink::ReceivePacketControlNack", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    // The mobile did not respond to the polling request in the downlink
    // assignment message so end this TBF by starting T3195.
    DBG_TRACE("RLC/MAC: Received PACKET CONTROL NACK - TBF(%#x)\n",
        pAssociatedTbf);
        
    // If the current tbf state is ACTIVATING then we were unable to confirm the 
    // downlink assigment.  Try and send the downlink activation a few times.
    if (pAssociatedTbf->GetTbfState() == Tbf::ACTIVATING)
    {
        // Try sending a PACKET POLLING REQUEST before giving up.  Ericsson phones won't poll from
        // the IMMEDIATE ASSIGNMENT message and require a specific polling message.
        if (pollOnActivation == FALSE)
        {
            pollOnActivation = TRUE;
            SendPacketPollingRequest();
        }
        else if (++dlAssignCnt < dlAssignCntMax)
        {
            // Set polling flag back to false for this assignment reattempt.
            pollOnActivation = FALSE;
            
            ULTbf *pUlTbf = RlcMacTask::theRlcMacTask->tbfPool.FindULTbf((unsigned long)pAssociatedTbf->tlli.GetTLLI());
            // No uplink TBF exists so send an IMMEDIATE ASSIGNMENT on the CCCH
            if (!pUlTbf)
            {
                // If more than one timeslot was allocated we need to free the
                // resources and reallocate just one timeslot since only a single
                // timeslot can be assigned in the IMMEDIATE ASSIGNMENT.
                int numTs = 0;
                for (int i = 0; i < MAX_TIMESLOTS; i++)
                {
                    if (pAssociatedTbf->allocatedTs[i])
                    {
                        if (++numTs > 1) break;
                    }
                }
                
                // If more than one timeslot was allocated then reallocate.
                if (numTs > 1)
                {
                    // Release resources with the scheduler.
                    RlcMacTask::theRlcMacTask->DLReleaseResourceRequest(pAssociatedTbf);
                    // Allocate just one timeslot.
                    if (RlcMacTask::theRlcMacTask->DLSingleTSResourceRequest(pAssociatedTbf))
                    {
                        // Let the DSP know about this change.
                        pAssociatedTbf->UpdateTSSpecificData();
                        RlcMacTask::theRlcMacTask->SendDLReconfigurationMsg(pAssociatedTbf);
                    }
                    else
                    {
                        pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);
                    }
                }
                
                RlcMacTask::theRlcMacTask->DLCCCHImmediateAssignment(pAssociatedTbf);  
            }
            else
            {
                RlcMacTask::theRlcMacTask->SendPacketDownlinkAssignment(pAssociatedTbf, pUlTbf, FALSE);
            }
        }
        else
        {
            pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);   
        }
    }
    // A reallocation attempt must have failed so just release the TBF.
    else if (pAssociatedTbf->GetTbfState() == Tbf::REALLOCATING)
    {
        pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);
        pAssociatedTbf->InternalReleaseDLTbf(); 
    }
    else
    {
        // This must be the end of an unacknowledged mode transfer.  Since we did
        // not get a PACKET CONTROL ACK from the MS just release this TBF.
        if (rlcMode == UNACKNOWLEDGED_RLC_MODE)
        {
            pT3195->setTimerMSec(T3195_VALUE, (int)pAssociatedTbf);
            pAssociatedTbf->InternalReleaseDLTbf(); 
        }
        // In acknowledged mode we must have not gotten a PACKET DOWNLINK ACK/NACK.  Activate
        // the TBF and try again.
        else
        {
            // Enable downlink scheduling for this TBF.
            pAssociatedTbf->SetTbfState(Tbf::ALLOCATED);
            lastBlockSentOnPollTs = FALSE;
        
            // Make sure the RTS mechanism is triggered.
            for (int i = 0; i < MAX_TIMESLOTS; i++)
            {
                if (pAssociatedTbf->allocatedTs[i])
                {
                    RlcMacTask::theRlcMacTask->SendDownlinkDataBlock(pAssociatedTbf->trx, i);
                }
            } 
        }
    }
            
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketPowerControlTimingAdvance
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: ta - timing advance
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS DLRlcLink::SendPacketPowerControlTimingAdvance(unsigned char ta) 
{
    DBG_FUNC("RlcMacTask::SendPacketPowerControlTimingAdvance", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader                         msgHdr;
    MsgPacketPowerCtlTimingAdvance      msg;

    // Assemble RLC/MAC header.
    msgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    msgHdr.SetRRBP(DLMacHeader::FN_N_13);
    msgHdr.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Power Control Timing Advance.
    msg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    
    msg.globalTFI.SetGlobalTFI(pAssociatedTbf->tfi.GetTFI(), IeGlobalTFI::DOWNLINK_TFI);
    msg.globalTA.SetGlobalPacketTimingAdvance(ta);
    
    int ts, trx;
    
    RlcMacTask::theRlcMacTask->TSSelectionRequest(&trx, &ts, pAssociatedTbf);
    
    if ((status = RlcMacTask::theRlcMacTask->SendPacchMsg(trx, ts, &msgHdr, &msg, 
        CTL_ACK_TAG_NOT_APPLICABLE, RLC_MAC_NORMAL_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketPowerControlTimingAdvance: failed %#x\n",
            status);       
    }
    
    DBG_LEAVE();
    return(status);
 }


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacketPollingRequest
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS DLRlcLink::SendPacketPollingRequest() 
{
    DBG_FUNC("RlcMacTask::SendPacketPollingRequest", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DLMacHeader                         msgHdr;
    MsgPacketPollingRequest             msg;

    // Assemble RLC/MAC header.
    msgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    msgHdr.SetRRBP(DLMacHeader::FN_N_13);
    msgHdr.SetSPBit(DLMacHeader::RRBP_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Polling Request.
    msg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    
    msg.globalTFI.SetGlobalTFI(pAssociatedTbf->tfi.GetTFI(), IeGlobalTFI::DOWNLINK_TFI);
    msg.typeOfAck = MsgPacketPollingRequest::ACK_MSG_FOUR_ACCESS_BURSTS;
    
    int ts, trx;
    
    RlcMacTask::theRlcMacTask->TSSelectionRequest(&trx, &ts, pAssociatedTbf);
    
    // Setup a control ack tag so that the PACKET CONTROL ACK message can
    // be connected to this downlink TBF.
    pAssociatedTbf->SetControlAckTag();
    
    if ((status = RlcMacTask::theRlcMacTask->SendPacchMsg(trx, ts, &msgHdr, &msg, 
        pAssociatedTbf->GetControlAckTag(), RLC_MAC_ACCESS_BURST)) != JC_OK)
    {
        DBG_WARNING("RlcMacTask::SendPacketPollingRequest: failed %#x\n",
            status);       
    }
    
    DBG_LEAVE();
    return(status);
 }


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::T3195TimerExpiry
**
**    PURPOSE:  Send a T3195 expiry message to the specified downlink TBF.
**
**    INPUT PARAMETERS: tbfPointer - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int DLRlcLink::T3195TimerExpiry(int tbfPointer)
{
    if (tbfPointer)
    {
        RlcMacT3195Expiry(tbfPointer);
    }
    return(OK);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::T3191TimerExpiry
**
**    PURPOSE:  Send a T3191 expiry message to the specified downlink TBF.
**
**    INPUT PARAMETERS: tbfPointer - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int DLRlcLink::T3191TimerExpiry(int tbfPointer)
{
    if (tbfPointer)
    {
        RlcMacT3191Expiry(tbfPointer);
    }
    return(OK);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::T3193TimerExpiry
**
**    PURPOSE:  Send a T3193 expiry message to the specified downlink TBF.
**
**    INPUT PARAMETERS: tbfPointer - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int DLRlcLink::T3193TimerExpiry(int tbfPointer)
{
    if (tbfPointer)
    {
        RlcMacT3193Expiry(tbfPointer);
    }
    return(OK);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ActivityTimerExpiry
**
**    PURPOSE:  Send an activity timer expiry message to the specified downlink TBF.
**
**    INPUT PARAMETERS: tbfPointer - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int DLRlcLink::ActivityTimerExpiry(int tbfPointer)
{
    if (tbfPointer)
    {
        RlcMacDLActivityTimerExpiry(tbfPointer);
    }
    return(OK);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DLRlcLink::ReleaseDataBlocks
**
**    PURPOSE: Give back data blocks to the downlink data block pool.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DLRlcLink::ReleaseDataBlocks()
{
    DBG_FUNC("DLRlcLink::ReleaseDataBlocks", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    // Return all data blocks to the pool.
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        if (pDlBlocks[i] && (i < RLC_MOD_VAL))
        {
            sysDlDataBlockPool.ReturnDLDataBlock(pDlBlocks[i]);
            pDlBlocks[i] = 0;
        } 
    }
    
    if (llcPdu)
    {
        bssgp_util_return_rx_buff(llcPdu);
    }
        
    DBG_LEAVE();
    return;
}


void DLRlcLink::ShowRlcLinkInfo()
{
    printf("Downlink RLC link information:\n");
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        printf("\tRLC Mode (ACKNOWLEDGED)");
    else    
        printf("\tRLC Mode (UNACKNOWLEDGED)");
    printf(" k = %d   V(S) = %d   V(A) = %d   V(CS) =  %d\n", k, vS, vA, vCS); 
    printf("\tV(B) : N=NACKED, A=ACKED, P=PENDING_ACK, I=INVALID\n");
    printf("\t"); 
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        if (vB[i] == NACKED)
            printf("N");
        else if (vB[i] == PENDING_ACK)
            printf("P");
        else if (vB[i] == ACKED)
            printf("A");    
        else
            printf("I");    
        if (((i % 32) == 0) && (i != 0)) printf("\n\t");    
    }
    printf("\n");  
    
    printf("\tAssociated TBF (%#x)\n", pAssociatedTbf);
    switch (chanCoding.GetChannelCodingCommand())
    {
        case IeChannelCodingCommand::CS_1 : printf("\tCurrent Channel Coding: CS_1\n"); break;
        case IeChannelCodingCommand::CS_2 : printf("\tCurrent Channel Coding: CS_2\n"); break;
        case IeChannelCodingCommand::CS_3 : printf("\tCurrent Channel Coding: CS_3\n"); break;
        case IeChannelCodingCommand::CS_4 : printf("\tCurrent Channel Coding: CS_4\n"); break;  
    }
    printf("\tN3105 = %d  dlAssignCnt = %d\n", n3105, dlAssignCnt);
}



void HexDumper(UINT8 *buffer, int length )
{     
    int size;     
    int i;     
    int dwPos = 0;
    static char line[255];
    int pos;

    DBG_FUNC("HexDumper", RLC_MAC_RLC_LINK);
    DBG_ENTER();

    while (length > 0) 
    { 
        //         
        // Take min of 16 or length         
        //                  
        size = min(16, length );          
        //         
        // Build text line        
        //                  
        pos = sprintf(line, "  %04X ", dwPos );          
        for (i = 0; i < size; i++) 
        {             
            pos += sprintf(line+pos, "%02X ", buffer[i] );             
        }          
        //         
        // Add spaces for short lines         
        //                  
        while (i < 16) 
        {             
            pos += sprintf(line+pos, "   " );             
            i++;         
        }          
        //         
        // Add ASCII chars         
        //                  
        for (i = 0; i < size; i++) 
        {             
            if (isprint(buffer[i])) 
            {                 
                pos += sprintf(line+pos, "%c", buffer[i] );                 
            } else {                 
                pos += sprintf(line+pos, "." );             
            }         
        }          
        
        DBG_TRACE((line));
        
        //         
        // Advance positions         
        // 
        
        length -= size;         
        dwPos += size;         
        buffer += size;     
    }
    DBG_LEAVE();
}
