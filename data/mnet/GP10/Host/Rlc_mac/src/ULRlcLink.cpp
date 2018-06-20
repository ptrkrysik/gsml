// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ULRlcLink.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "ULRlcLink.h"
#include "RlcMacTask.h"
#include "MsgPacketUplinkAckNack.h"
#include "bssgp\bssgp_util.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"

void HexDumper(UINT8 *buffer, int length );

// Static uplink data block pool.
ULDataBlockPool ULRlcLink::sysUlDataBlockPool;

int ULRlcLink::N3103_MAX = 5;
int ULRlcLink::N3101_MAX = 8;

ULRlcLink::ULRlcLink() : k(RCV_WINDOW_SIZE_K)
{
    rlcMode = ACKNOWLEDGED_RLC_MODE;
    
    // V(R) and V(Q) are initially set to 0.
    vR = 0;
    vQ = 0;
    
    // Set V(N) to INVALID.
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        vN[i] = INVALID;
        pUlBlocks[i] = 0;
    }
    
    firstDataBlock = 0;
    lastDataBlock = LAST_BLOCK_UNKNOWN;
    numRLCBlocksSinceLastAck = MAX_RLC_BLOCKS_BETWEEN_ACKS;
    endOfTBF = FALSE;
    chanCoding.SetChannelCodingCommand(IeChannelCodingCommand::CS_1);
    pAssociatedTbf = 0;
    
    pT3169 = new JCCTimer((FUNCPTR)ULRlcLink::T3169TimerExpiry, 0);
    pActivityTimer = new JCCTimer((FUNCPTR)ULRlcLink::ActivityTimerExpiry, 0);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::EstablishRLCLink
**
**    PURPOSE: Initialize all the RLC state information for a new uplink TBF.
**
**    INPUT PARAMETERS: mode - RLC Mode (acked or not acked)
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS ULRlcLink::EstablishRLCLink(ULTbf *pUlTbf, RLC_MODE mode)
{
    DBG_FUNC("ULRlcLink::EstablishRLCLink", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    rlcMode = mode;
    
    // V(R) and V(Q) are initially set to 0.
    vR = 0;
    vQ = 0;
    
    // Set V(N) to INVALID.
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        vN[i] = INVALID;
        pUlBlocks[i] = 0;
    }
    
    firstDataBlock = 0;
    lastDataBlock = LAST_BLOCK_UNKNOWN;
    numRLCBlocksSinceLastAck = MAX_RLC_BLOCKS_BETWEEN_ACKS;
    endOfTBF = FALSE;
    
    // For now we will start with channel coding set to CS-0
    chanCoding.SetChannelCodingCommand(IeChannelCodingCommand::CS_1);
       
    // Hang on to the tbf object. 
    pAssociatedTbf = pUlTbf;
    pUlTbf->ClearControlAckTag();
              
    // Clear counters.
    n3103 = 0;
    n3101 = 0;
    
	// Start Activity Timer
	pActivityTimer->setTimerMSec(MAX_ALLOWED_INACTIVE_TIME, (int)pAssociatedTbf);

    DBG_TRACE("RLC/MAC: Uplink RLC link established tbf(%#x)\n", pUlTbf);
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        DBG_TRACE("\tRLC Mode (ACKNOWLEDGED) k = %d  V(R) = %d  V(Q) = %d\n", 
            k, vR, vQ);
    else    
        DBG_TRACE("\tRLC Mode (UNACKNOWLEDGED) k = %d  V(R) = %d  V(Q) = %d\n", 
            k, vR, vQ);
    
    DBG_LEAVE();
    return (status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::UpdateTSSpecificData
**
**    PURPOSE: This function will be called after timeslots are allocated.  Any
**      data that requires timeslot information should be set here.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::UpdateTSSpecificData()
{
    DBG_FUNC("ULRlcLink::UpdateTSSpecificData", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    int numTs = 0;
    
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pAssociatedTbf->allocatedTs[i]) numTs++;
    }
    
    n3101FirstBlkBuffer = numTs * 4;
    
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
JC_STATUS ULRlcLink::AbnormalRelease()
{
    DBG_FUNC("ULRlcLink::AbnormalRelease", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Inactivate the tbf so that no more scheduling occurs.
    pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
    
    // Send a PACKET TBF RELEASE message.
    status = RlcMacTask::theRlcMacTask->SendPacketTBFRelease(0, pAssociatedTbf, 
        MsgPacketTBFRelease::ABNORMAL_RELEASE);
    
    // Use T3169 to allow enough time to send the deactivate message.
    pT3169->cancelTimer();
    pT3169->setTimerMSec(T3169_VALUE, (int)pAssociatedTbf);
    
    DBG_LEAVE();
    return (status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::UpdateRcvStateVariables
**
**    PURPOSE: Update the receive state variables V(Q), V(R) and V(N)
**      appropriately.
**
**    INPUT PARAMETERS: pUlDataBlock - pointer to the received data block
**
**    RETURN VALUE(S): TRUE - if data block is in receive window
**                     FALSE - if data block is not in receive window
**
**----------------------------------------------------------------------------*/
bool ULRlcLink::UpdateRcvStateVariables(ULDataBlock *pUlDataBlock)
{
    DBG_FUNC("ULRlcLink::UpdateRcvStateVariables", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    // Handle acknowledged mode.
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
    {
        // Verify that the BSN is in the active window.
        //   [V(Q) <= BSN < V(Q) + k]
        int rcvDiff = vQ - pUlDataBlock->ulHeader.GetBSN();
        if (rcvDiff > 0) rcvDiff = RLC_MOD_VAL - rcvDiff;
        if (rcvDiff < k)
        {
            // Update V(R) based on the BSN
            // In RLC acknowledged mode:
            //   V(R) is set to [BSN'+1] mod 128 provided [V(R) <= BSN' < V(Q) + k]
            if ((vR <= pUlDataBlock->ulHeader.GetBSN()) || ((vR > pUlDataBlock->ulHeader.GetBSN()) &&
                 (vQ > pUlDataBlock->ulHeader.GetBSN())))
            {
                int tmpVR = vR;
                
                vR = (pUlDataBlock->ulHeader.GetBSN() + 1) % RLC_MOD_VAL;
                
                // Since V(R) moved update the V(N) state of any elements that are
                // less than V(R)-k mod 128
                while (tmpVR != vR)
                {
                    vN[(tmpVR+(k+1)) % RLC_MOD_VAL] = INVALID;
                    tmpVR = ++tmpVR % RLC_MOD_VAL;
                }
            }
            
            // Update V(N) to RECEIVED for newly received block
            vN[pUlDataBlock->ulHeader.GetBSN() % RLC_MOD_VAL] = RECEIVED;
            
            // If this BSN is at the start of the window then update V(Q)
            if (pUlDataBlock->ulHeader.GetBSN() % RLC_MOD_VAL == vQ)
            {
                // Find the next non-received BSN or V(R).  
                while (vQ != vR)
                {
                    vQ = ++vQ % RLC_MOD_VAL;
                    if (vN[vQ] == INVALID) break;
                } 
            }
        }
        // BSN is out of the active window so throw it on the floor.
        else
        {
            DBG_LEAVE();
            return (FALSE);
        }
    }
    // Handle unacknowledged mode.
    else
    {
        // Don't do any updating if we already received this block.  NOTE the
        // only time this should happen is on the last block.
        if (vR != (pUlDataBlock->ulHeader.GetBSN()) + 1 % RLC_MOD_VAL)
        {
            // Update V(R) based on the BSN
            // In RLC unacknowledged mode:
            //   V(R) is set to [BSN'+1] mod 128
            vR = (pUlDataBlock->ulHeader.GetBSN()) + 1 % RLC_MOD_VAL;
        
            // Update V(Q)
            //   If [V(R) - V(Q)] mod 128 > k then V(Q) = [V(R) - k] mod 128
            if (vR >= vQ)
            {   
                if ((vR - vQ) % RLC_MOD_VAL > k)
                {
                    vQ = vR - k;
                }    
            }
            else
            {
                if (vR + (RLC_MOD_VAL - vQ) > k)
                {
                    vQ = (RLC_MOD_VAL - (k - vR)) % RLC_MOD_VAL; 
                }
            }
        }
        else
        {
            DBG_LEAVE();
            return (FALSE);
        }
    }
    
    DBG_LEAVE();
    return (TRUE);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::SendULAckNack
**
**    PURPOSE: Send a PACKET UPLINK ACK/NACK message.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::SendULAckNack()
{
    DBG_FUNC("ULRlcLink::SendULAckNack", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DLMacHeader             ackMsgHdr;
    MsgPacketUplinkAckNack  ackMsg;
    int                     trx, ts;
    unsigned char           controlAckTag = CTL_ACK_TAG_NOT_APPLICABLE;

    RlcMacTask::theRlcMacTask->TSSelectionRequest(&trx, &ts, pAssociatedTbf);
    // Assemble RLC/MAC header.
    ackMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
    ackMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
    ackMsgHdr.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
    // USF will be filled in later.
    
    // Assemble Packet Uplink Ack/Nack.
    ackMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    ackMsg.uplinkTFI = pAssociatedTbf->tfi;
    ackMsg.chanCoding = chanCoding;
    
    // If we have seen the last block see if we can set the final bit
    IeAckNackDescription::FINAL_ACK_INDICATION finalAck;
    if (endOfTBF == TRUE)
    {
        // For acknowledged mode we must verify that all blocks were received.
        if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        {
            if (vR == vQ)
            {
                finalAck = IeAckNackDescription::TBF_COMPLETE;
                // Add a tag to the PACCH IND message sent to the DSP.  This tag will be
                // returned in the PACCH REQ or PACCH NACK message.
                controlAckTag = pAssociatedTbf->SetControlAckTag();
                // Set the state to inactive so it won't be scheduled.
                pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
                // Set the Relative Reserve Bit Period to valid.
                ackMsgHdr.SetSPBit(DLMacHeader::RRBP_VALID);
            }
            else
            {
                finalAck = IeAckNackDescription::TBF_INCOMPLETE;
            }
        }
        // For unacknowledged mode once we see the last block the TBF is over.
        else
        {
            finalAck = IeAckNackDescription::TBF_COMPLETE;
            // Add a tag to the PACCH IND message sent to the DSP.  This tag will be
            // returned in the PACCH REQ or PACCH NACK message.
            controlAckTag = pAssociatedTbf->SetControlAckTag();
            // Set the state to inactive so it won't be scheduled.
            pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
            // Set the Relative Reserve Bit Period to valid.
            ackMsgHdr.SetSPBit(DLMacHeader::RRBP_VALID);
        } 
    }
    // Not the end of the TBF
    else
    {
        finalAck = IeAckNackDescription::TBF_INCOMPLETE;
    }
    
    // Build the received block bitmap.
    unsigned char rrb[RRB_SIZE];
    int vrTmp = vR;
    bzero((char *)rrb, RRB_SIZE);
    for (int i = RRB_SIZE-1; i >= 0; i--)
    {
        unsigned char bitPos = 1;
        while (bitPos)
        {
            if (--vrTmp < 0)
                vrTmp += RLC_MOD_VAL;
            if (vN[vrTmp] == RECEIVED)
                rrb[i] |= bitPos;    
            bitPos <<= 1;
        }
    } 
    
    ackMsg.ackNackDescription.SetAckNackDescription(finalAck, (unsigned char)vR, rrb);
    
    DBG_TRACE("RLC/MAC: Sending UPLINK ACK/NACK - tbf(%#x)\n", pAssociatedTbf);
    
    if (RlcMacTask::theRlcMacTask->SendPacchMsg(trx, ts, &ackMsgHdr, &ackMsg, controlAckTag,
        RLC_MAC_ACCESS_BURST) != JC_OK)
    {
        DBG_WARNING("ULRlcLink::SendULAckNack: failed\n");       
    }

    // Reset the acknowledgement counter.
    numRLCBlocksSinceLastAck = MAX_RLC_BLOCKS_BETWEEN_ACKS;
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ReceivePacketControlAck
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::ReceivePacketControlAck(MsgPacketControlAcknowledgement *pCtrlMsg)
{
    DBG_FUNC("ULRlcLink::ReceivePacketControlAck", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: Received PACKET CONTROL ACK - tbf(%#x)\n", pAssociatedTbf);
    
    // Canel T3169.
    pT3169->cancelTimer();
    
	// Cancel Activity Timer
	pActivityTimer->cancelTimer();

    // Resources may now be released.
    ReleaseDataBlocks(0, RLC_MOD_VAL);
    pAssociatedTbf->ReleaseULTbf();
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ProcessT3169Expiry
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::ProcessT3169Expiry()
{
    DBG_FUNC("ULRlcLink::ProcessT3169Expiry", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: T3169 Expired - TBF(%#x)\n", pAssociatedTbf);
    // Canel T3169.
    pT3169->cancelTimer();

	// Cancel Activity Timer
    pActivityTimer->cancelTimer();

    // Resources may now be released.
    ReleaseDataBlocks(0, RLC_MOD_VAL);
    pAssociatedTbf->ReleaseULTbf();
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ProcessActivityTimerExpiry
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::ProcessActivityTimerExpiry()
{
    DBG_FUNC("ULRlcLink::ProcessActivityTimerExpiry", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: ActivityTimer Expired - TBF(%#x)\n", pAssociatedTbf);
    
    // Resources may now be released.
    ReleaseDataBlocks(0, RLC_MOD_VAL);
    pAssociatedTbf->ReleaseULTbf();
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ReceivePacketControlNack
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::ReceivePacketControlNack()
{
    DBG_FUNC("ULRlcLink::ReceivePacketControlNack", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: Received PACKET CONTROL NACK - tbf(%#x)\n", pAssociatedTbf);
    
    // If we have not reached N3203max then send another uplink ack nack with
    // the final bit set.
    if (++n3103 < N3103_MAX)
    {
        SendULAckNack();
    }
    else
    {
        pT3169->setTimerMSec(T3169_VALUE, (int)pAssociatedTbf);
        DBG_TRACE("RLC/MAC: Starting T3169 for %d seconds - TBF(%#x)\n", 5, pAssociatedTbf);
    }
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ReassembleULPDU
**
**    PURPOSE: Reassemble an uplink PDU(s).
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::ReassembleULPDU()
{
    DBG_FUNC("ULRlcLink::ReassembleULPDU", RLC_MAC_RLC_LINK);
    DBG_ENTER();

    unsigned char pduBuf[MAX_RLC_PDU_SIZE];
    unsigned char *pPduBuf = pduBuf;

    int currDataBlock = firstDataBlock;
    bool firstLoop;
    
    while (1)
    {
        firstLoop = TRUE;
        do {
            // Increment if this is not the first loop.
            if (!firstLoop)    
            {
                currDataBlock = ++currDataBlock % RLC_MOD_VAL;
            }
                
            // If there is no data block then we must be in unacknowledged mode.
            if (pUlBlocks[currDataBlock])
            {
                if (pUlBlocks[currDataBlock]->ulHeader.GetExtensionBit() == ULMacHeader::EXTENSION_OCTET_FOLLOWS)
                {
                    unsigned char length; 
                    ULMacHeader::MORE_BIT more;
                    ULMacHeader::EXTENSION_BIT extBit;
                    pUlBlocks[currDataBlock]->ulHeader.GetCurrentLengthOctet(&length, &more, &extBit);
                    
                    if (length != INVALID_LENGTH_VAL)
                    { 
                        // For M=0 and E=1 there is no more data for this PDU and no more extension octets.
                        // There is an exception case where the length is equal to zero.  For this case the last
                        // octet for the PDU is in the next RLC block.  The rest of this length equal zero
                        // case will be handled below in the lastDataBlock section.
                        if (((more == ULMacHeader::NO_MORE_LLC_DATA) && (extBit == ULMacHeader::NO_EXTENSION_OCTET)) &&
                            (length == 0))
                        {
                            // When the length is zero all the RLC data bytes in the RLC block are for this PDU.
                            // The final octet is in the next RLC data block.
                            // Grab all the rest of the octets in this RLC block.
                            int len = pUlBlocks[currDataBlock]->GetDataBlockLength() - 
                                pUlBlocks[currDataBlock]->streamIn.GetCurrentPosition();
                            pUlBlocks[currDataBlock]->streamIn.ExtractBytesN(pPduBuf, len); 
                            pPduBuf += len;
                        }
                        // For M=1 and E=x another PDU follows. 
                        // This RLC block contains the start and end of the current PDU.  
                        else
                        {
                            // For this case the bytes indicated by length are for the current
                            // PDU.  So here we just copy out those bytes.
                            pUlBlocks[currDataBlock]->streamIn.ExtractBytesN(pPduBuf, length);
                            pPduBuf += length;                             
                        }                    
                    }
                    // No more extension octets so the remaining octets must be for this PDU.
                    else
                    {
                        // This is a case where there must have previously been a PDU end in this
                        // RLC block and the remaining bytes are for another PDU.
                        int len = pUlBlocks[currDataBlock]->GetDataBlockLength() - 
                            pUlBlocks[currDataBlock]->streamIn.GetCurrentPosition();
                        pUlBlocks[currDataBlock]->streamIn.ExtractBytesN(pPduBuf, len); 
                        pPduBuf += len;
                    }
                }
                // No extension bit so just get all the bytes in the RLC block.
                else
                {
                    // For this case all the remaining bytes in this RLC block are for the current
                    // PDU.  So here we just copy out the remaining bytes.
                    int len = pUlBlocks[currDataBlock]->GetDataBlockLength() - 
                        pUlBlocks[currDataBlock]->streamIn.GetCurrentPosition();
                    pUlBlocks[currDataBlock]->streamIn.ExtractBytesN(pPduBuf, len); 
                    pPduBuf += len;
                }
            }
            // In unacknowledged mode and no data block was received.
            else
            {
                // Add in fill bits.
                bfill ((char *)pPduBuf, chanCoding.GetNumFillBytes(), 0);
                pPduBuf += chanCoding.GetNumFillBytes();
            }
            
            firstLoop = FALSE;
        } while (currDataBlock != lastDataBlock);
    
        //---------------------------------------------------------------------
        // PDU is reassembled
        //---------------------------------------------------------------------
    
        // We have processed the end of this PDU so send it to BSSGP.
        SendBSSGPUnitData(pduBuf, pPduBuf - pduBuf);
    
        // Update the start and end data pointer and release data blocks.
        if (ScanDataBlocks(currDataBlock))
        {
            continue;
        }
        else
        {
            break;
        }  
    }
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::IsPDUAssemblyNeeded
**
**    PURPOSE: Check whether packet reassembly can happen now.
**
**    INPUT PARAMETERS: pUlDataBlock - pointer to the received data block
**
**    RETURN VALUE(S): TRUE - if it is time for packet reassembly
**                     FALSE - if it is NOT time for packet reassembly
**
**----------------------------------------------------------------------------*/
bool ULRlcLink::IsPDUAssemblyNeeded(ULDataBlock *pUlDataBlock)
{
    DBG_FUNC("ULRlcLink::IsPDUAssemblyNeeded", RLC_MAC_RLC_LINK);
    DBG_ENTER();

    bool reassemblyNeeded = FALSE;
    
    // If we have not seen the last RLC block for a given PDU then see if this
    // is the last block.
    if (lastDataBlock == LAST_BLOCK_UNKNOWN)
    {
        // We are at the last RLC block for a PDU if CV=0 or if there is a length
        // indicator with a non-zero length value.
        if (pUlDataBlock->ulHeader.GetCountdownVal() == 0)
        {
            // Record the last RLC block position.
            lastDataBlock = pUlDataBlock->ulHeader.GetBSN() % RLC_MOD_VAL;
        }
        else if (pUlDataBlock->ulHeader.GetExtensionBit() == ULMacHeader::EXTENSION_OCTET_FOLLOWS)
        {
            unsigned char length; 
            ULMacHeader::MORE_BIT more;
            ULMacHeader::EXTENSION_BIT extBit;
            if (pUlDataBlock->ulHeader.GetCurrentLengthOctet(&length, &more, &extBit))
            {
                if ((length != 0) && (length != INVALID_LENGTH_VAL))
                {
                // Record the last RLC block position.
                lastDataBlock = pUlDataBlock->ulHeader.GetBSN() % RLC_MOD_VAL;
                }
            }
        }
    }
        
    // If we have seen the last RLC block for a given PDU then check to 
    // see if all the blocks have been received for acknowledged mode.
    if (lastDataBlock != LAST_BLOCK_UNKNOWN)
    {
        // For acknowledged mode we must have received all the RLC blocks
        // to reassemble.
        if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        {
            int rcvDiff = vQ - lastDataBlock;
    
            if (rcvDiff < 0) rcvDiff += RLC_MOD_VAL;
            // If all the RLC blocks have been received then reassemble.
            if (rcvDiff < k)
            {
                reassemblyNeeded = TRUE;
            }
        }
        // The unacknowledged mode PDU can be reassembled now.
        else
        {
            reassemblyNeeded = TRUE;
        }
    }
    
    DBG_LEAVE();
    return (reassemblyNeeded);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ScanDataBlocks
**
**    PURPOSE: Check to see if another PDU reassembly can occur.  To do this
**      we must look at the M and E bits of the current data block and if
**      necessary scan forward in subsequently received data blocks.
**
**    INPUT PARAMETERS: currDataBlock - index of the current data block
**
**    RETURN VALUE(S): TRUE - if more reassembly can occur
**                     FALSE - if NO more reassembly can occur
**
**----------------------------------------------------------------------------*/
bool ULRlcLink::ScanDataBlocks(int currDataBlock)
{
    DBG_FUNC("ULRlcLink::ScanDataBlocks", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    // Check to see if the last block processed has an extension bit.
    if (pUlBlocks[currDataBlock]->ulHeader.GetExtensionBit() == ULMacHeader::EXTENSION_OCTET_FOLLOWS)
    {
        unsigned char length; 
        ULMacHeader::MORE_BIT more;
        ULMacHeader::EXTENSION_BIT extBit;
        pUlBlocks[currDataBlock]->ulHeader.GetCurrentLengthOctet(&length, &more, &extBit);
        
        // M=1 and E=0.  There is more data to come and an extension follows.  Scanning
        // below will detect if the PDU is ready to reassemble.  In any case the current
        // block is the start and the end is unkown at this time.
        //
        // M=1 and E=1. There is more data but no more extensions.  The next PDU starts in this 
        // RLC block, but does not end there.  Scanning below will detect if the rest of the 
        // data blocks have been received.
        if (((more == ULMacHeader::MORE_LLC_DATA) && (extBit == ULMacHeader::EXTENSION_OCTET_FOLLOWS)) ||
           ((more == ULMacHeader::MORE_LLC_DATA) && (extBit == ULMacHeader::NO_EXTENSION_OCTET)))
        {
            // Advance to the next length octet.
            pUlBlocks[currDataBlock]->ulHeader.GetNextLengthOctet(&length, &more, &extBit);
        }
        // M=0 and E=1. No more data to come and no extension.  
        else if ((more == ULMacHeader::NO_MORE_LLC_DATA) && (extBit == ULMacHeader::NO_EXTENSION_OCTET))
        {
            // The new starting block will be the next data block.
            ++currDataBlock % RLC_MOD_VAL;
        }        
    }
    // No extension octet.
    else
    {
        // The new starting block will be the next data block.
        ++currDataBlock % RLC_MOD_VAL;
        // No end is known yet...we will have to scan forward.
        lastDataBlock = LAST_BLOCK_UNKNOWN;
    }
    
    // Free all data blocks up to but not including the current data block.
    ReleaseDataBlocks(firstDataBlock, currDataBlock);
    // Move the first data block to be the current one.
    firstDataBlock = currDataBlock;
    // No end is known yet...we will have to scan forward. 
    lastDataBlock = LAST_BLOCK_UNKNOWN;
  
    // Scan ahead to see if we can reassemble more PDUs
    while (currDataBlock != vR)
    {
        if (pUlBlocks[currDataBlock])
        {
            if (IsPDUAssemblyNeeded(pUlBlocks[currDataBlock]))
            {
                // Keep reassembling.
                return(TRUE);
            }
        }
        currDataBlock = ++currDataBlock % RLC_MOD_VAL;
    }
    
    DBG_LEAVE();
    return (FALSE);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ReceiveULDataBlock
**
**    PURPOSE: Accept a new uplink RLC block and process it.
**
**    INPUT PARAMETERS: pUlDataBlock - pointer to the received data block
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS ULRlcLink::ReceiveULDataBlock(ULDataBlock *pUlDataBlock)
{
    DBG_FUNC("ULRlcLink::ReceiveULDataBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    // Clear N3101 since a data bock was received.  Also clear the extra allowance
    // in N3101 before the first block has been received.
    n3101 = 0;
    n3101FirstBlkBuffer = 0;
    
	// Start Activity Timer
	pActivityTimer->setTimerMSec(MAX_ALLOWED_INACTIVE_TIME, (int)pAssociatedTbf);

    // Update the receive variables V(Q), V(R) and V(N).
    if (UpdateRcvStateVariables(pUlDataBlock))
    {
        DBG_TRACE("RLC/MAC: Uplink RLC block received tbf(%#x)\n", pAssociatedTbf);
        DBG_TRACE("\tBSN = %d  V(R) = %d   V(Q) = %d\n", pUlDataBlock->ulHeader.GetBSN(),
            vR, vQ); 
    
        // Insert data block into list of blocks for reassembly.
        if (pUlBlocks[pUlDataBlock->ulHeader.GetBSN() % RLC_MOD_VAL])
        {
            // This RLC block has already been received so just throw it away.
            ULRlcLink::sysUlDataBlockPool.ReturnULDataBlock(pUlDataBlock);
        }
        else
        {
            pUlBlocks[pUlDataBlock->ulHeader.GetBSN() % RLC_MOD_VAL] = pUlDataBlock;
        }
    
        // Check to see if an UPLINK ACK/NACK message is needed and send one.
        // Three reasons to send an ack/nack:
        //   1. Periodically send one - based on numRLCBlocksSinceLastAck
        //   2. We just received the block with the final BSN.
        //   3. We previously received the PDU with the final BSN, but just now
        //      acknowledged all other blocks.
        if ((!(--numRLCBlocksSinceLastAck)) || pUlDataBlock->ulHeader.GetCountdownVal() == 0 ||
            ((endOfTBF == TRUE) && (vQ = vR)))
        {
            // Remember that we have seen the PDU with the final BSN.
            if (pUlDataBlock->ulHeader.GetCountdownVal() == 0)
            {
                endOfTBF = TRUE;
            }
            
            SendULAckNack();
        }
        
        // Check to see if a PDU assembly can occur.
        if (IsPDUAssemblyNeeded(pUlDataBlock))
        {
            ReassembleULPDU();
        }
    }
    // Received block was not in the receive window so throw it away.
    else
    {
        DBG_TRACE("RLC/MAC: Uplink RLC block received tbf(%#x)\n", pAssociatedTbf);
        DBG_TRACE("\tOutside rcv window or already received: BSN = %d\n", 
            pUlDataBlock->ulHeader.GetBSN()); 
        ULRlcLink::sysUlDataBlockPool.ReturnULDataBlock(pUlDataBlock);
    }
    
    DBG_LEAVE();
    return (status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ReleaseDataBlocks
**
**    PURPOSE: Give back data blocks to the uplink data block pool.  Note that
**      the endBlock will not be released.  So to release a single block 
**      startBlock must be one less than endBlock.
**
**    INPUT PARAMETERS: startBlock - index of the starting data block
**                      endBlock - index of the ending data block
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::ReleaseDataBlocks(int startBlock, int endBlock)
{
    DBG_FUNC("ULRlcLink::ReleaseDataBlocks", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    if (endBlock > RLC_MOD_VAL) endBlock = RLC_MOD_VAL;
    
    for (int i = startBlock; i != endBlock; i = ++i % (RLC_MOD_VAL+1))
    {
        if (pUlBlocks[i] && (i < RLC_MOD_VAL))
        {
            ULRlcLink::sysUlDataBlockPool.ReturnULDataBlock(pUlBlocks[i]);
            pUlBlocks[i] = 0;
        } 
    }
        
    DBG_LEAVE();
    return;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::SendBSSGPUnitData
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pBuf - pointer to the uplink PDU
**                      length - length of the PDU
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void ULRlcLink::SendBSSGPUnitData(unsigned char *pBuf, int len)
{
    DBG_FUNC("ULRlcLink::SendBSSGPUnitData", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DBG_TRACE("RLC/MAC: Sending BSSGP uplink pdu (%#x) length (%d)\n", pBuf, len);
    HexDumper(pBuf, len);
    
    BSSGP_API_MSG msg;
    bzero((char *)&msg, sizeof(BSSGP_API_MSG));
    
    // Grab a buffer from BSSGP for the LLC PDU.
    char *pdu = bssgp_util_get_rx_buff(len);
    
    if (pdu)
    {
        // Build bssgp message.
        msg.msg_type = BSSGP_API_MSG_UL_UNITDATA;
        msg.magic = BSSGP_MAGIC;
        msg.msg.ul_unitdata.tlli.tlli = pAssociatedTbf->tlli.GetTLLI();
        msg.msg.ul_unitdata.tlli.ie_present = TRUE;
        msg.msg.ul_unitdata.qos_profile;
        msg.msg.ul_unitdata.qos_profile.ie_present = TRUE;
        // msg.msg.ul_unitdata.qos_profile.bucket_leak_rate[2]; -- filled in by BSSGP
        msg.msg.ul_unitdata.qos_profile.precedence_class = (CNI_RIL3_QOS_PRECEDENCE_CLASS)pAssociatedTbf->chanReq.radioPriority;
        msg.msg.ul_unitdata.qos_profile.a_bit = (QOS_PROFILE_A_BIT)rlcMode;
        if (pAssociatedTbf->accType.GetAccessType() == IeAccessType::TWO_PHASE)
        {
            msg.msg.ul_unitdata.qos_profile.t_bit = SDU_CONTAINS_DATA;
        }
        else
        {
            msg.msg.ul_unitdata.qos_profile.t_bit = SDU_CONTAINS_SIGNALLING;
        }
        msg.msg.ul_unitdata.qos_profile.cr_bit = (QOS_PROFILE_CR_BIT)pAssociatedTbf->chanReq.llcPduType;
    
        //msg.msg.ul_unitdata.cell_id; -- filled in by BSSGP
        bcopy((char *)pBuf, pdu, len);
        msg.msg.ul_unitdata.pdu.data = (unsigned char *)pdu;	
        msg.msg.ul_unitdata.pdu.data_len = len;	
        //msgmsg.ul_unitdata.bvci; -- filled in by BSSGP
    }
    else
    {
        DBG_WARNING("ULRlcLink::SendBSSGPUnitData: no BSSGP buf available for %d byte pdu!\n",
            len);
    }
    
    if (!bssgp_api_send_msg(msg))
    {
        DBG_WARNING("ULRlcLink::SendBSSGPUnitData: bssgp_api_send_msg failed!\n");
    }
        
    DBG_LEAVE();
    return;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::IncrementN3101
**
**    PURPOSE:  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): TRUE - if N3101 has not reached max
**                     FALSE - if N3101 has reached max
**
**----------------------------------------------------------------------------*/
bool ULRlcLink::IncrementN3101()
{
    DBG_FUNC("ULRlcLink::IncrementN3101", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    bool ret = TRUE;
    
    // NOTE: An extra number of counts are allowed before the first RLC block
    // is received.  This is because RLC/MAC does not no when the assignment
    // was sent to the MS.  This means some uplink blocks may be reserved
    // before the MS see the assignement.  This extra buffer should prevent
    // the TBF from being released.
    if (++n3101 >= (N3101_MAX + n3101FirstBlkBuffer))
    {
        // N3101 has reached max so stop scheduling this TBF and start T3169.
        pAssociatedTbf->SetTbfState(Tbf::INACTIVE);
        pT3169->setTimerMSec(T3169_VALUE, (int)pAssociatedTbf);
        DBG_TRACE("RLC/MAC: N3101 = N3101Max - TBF(%#x)\n", pAssociatedTbf);
    }
        
    DBG_LEAVE();
    return (ret);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::T3169TimerExpiry
**
**    PURPOSE:  Send a T3169 expiry message to the specified uplink TBF.
**
**    INPUT PARAMETERS: tbfPointer - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int ULRlcLink::T3169TimerExpiry(int tbfPointer)
{
    if (tbfPointer)
    {
        RlcMacT3169Expiry(tbfPointer);
    }
    return(OK);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: ULRlcLink::ActivityTimerExpiry
**
**    PURPOSE:  Send an activity timer expiry message to the specified uplink TBF.
**
**    INPUT PARAMETERS: tbfPointer - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int ULRlcLink::ActivityTimerExpiry(int tbfPointer)
{
    if (tbfPointer)
    {
        RlcMacULActivityTimerExpiry(tbfPointer);
    }
    return(OK);
}


void ULRlcLink::ShowRlcLinkInfo()
{
    printf("Uplink RLC link information:\n");
    if (rlcMode == ACKNOWLEDGED_RLC_MODE)
        printf("\tRLC Mode (ACKNOWLEDGED)");
    else    
        printf("\tRLC Mode (UNACKNOWLEDGED)");
    printf(" k = %d   V(R) = %d   V(Q) = %d\n", k, vR, vQ); 
    printf("\tV(N) : R=Received, I=INVALID\n");
    printf("\t"); 
    for (int i = 0; i < RLC_MOD_VAL; i++)
    {
        if (((i % 32) == 0) && (i != 0)) printf("\n\t");    
        if (vN[i] == RECEIVED)
            printf("R");
        else
            printf("I");
    }
    printf("\n");  
    
    printf("\tfirstDataBlock (%d) lastDataBlock(%d)\n",
        firstDataBlock, lastDataBlock);
    printf("\tnumRLCBlocksSinceLastAck(%d) endOfTbf(%d)\n",
        numRLCBlocksSinceLastAck, endOfTBF);
    printf("\tAssociated TBF (%#x)\n", pAssociatedTbf);
    switch (chanCoding.GetChannelCodingCommand())
    {
        case IeChannelCodingCommand::CS_1 : printf("\tCurrent Channel Coding: CS_1\n"); break;
        case IeChannelCodingCommand::CS_2 : printf("\tCurrent Channel Coding: CS_2\n"); break;
        case IeChannelCodingCommand::CS_3 : printf("\tCurrent Channel Coding: CS_3\n"); break;
        case IeChannelCodingCommand::CS_4 : printf("\tCurrent Channel Coding: CS_4\n"); break;  
    }
    printf("\tN3101 = %d  N3103 = %d\n", n3101, n3103);
}
