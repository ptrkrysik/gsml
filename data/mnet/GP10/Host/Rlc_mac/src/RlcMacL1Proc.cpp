// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacRmMdProc.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#include "RlcMacTask.h"
#include "Rlc_Mac/RlcMacIntf.h"
#include "MsgPacketDownlinkDummyControlBlock.h"
#include "Grr/grr_intf.h"

#include "dsp/dsphmsg.h"


extern void sendDsp(unsigned char *buffer, int len);

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DecodeL1MsgInd
**
**    PURPOSE: The layer 1 message indication is decoded.  If decoding is
**      successful the appropriate action is taken.
**
**    INPUT PARAMETERS: msgReq - layer 1 message request structure
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::DecodeL1MsgInd(RlcMacL1MsgIndType *msgReq)
{
    DBG_FUNC("RlcMacTask::DecodeL1MsgInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    msgDiscriminator    l1MsgDisc = (msgDiscriminator)msgReq->l1Msg[0];
    msgType             l1MsgType = (msgType)((msgReq->l1Msg[1] << 8) | (msgReq->l1Msg[2]));
    
    if (l1MsgDisc != PDCH_MANAGE_MSG)
    {
        DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Invalid msg discriminator %d\n",
            l1MsgDisc);
        DBG_LEAVE();        
        return (JC_ERROR);
    }
    
    // Decode each l1 message separately.
    switch (l1MsgType)
    {
        case PH_READY2SEND_IND:
        {
            MsgReadyToSend    l1RTSInd;
            msgDecodeReadyToSend((char *)msgReq->l1Msg, &l1RTSInd);
            DBG_TRACE("RLC/MAC: Received PH_READY2SEND_IND msg - TRX(%d) TS(%#x) count(%d)\n",
                l1RTSInd.header.trx, l1RTSInd.channel.typeTs, l1RTSInd.numBuffs);
            status = ProcessPhRTSInd(&l1RTSInd);
        }
        break;   
        case PH_PRACH_IND:
        {
            MsgPrach    l1PrachInd;
            msgDecodePrach((char *)msgReq->l1Msg, &l1PrachInd);
            DBG_TRACE("RLC/MAC: Received PH_PRACH_IND msg - TRX(%d), TS(%#x), ref(%d)\n"
                "\tt1(%d) t2(%d) t3(%d) ta(%d) power(%d)\n", l1PrachInd.header.trx, 
                l1PrachInd.channel.typeTs, l1PrachInd.packetAccessRef, l1PrachInd.frameNum.t1,
                l1PrachInd.frameNum.t2, l1PrachInd.frameNum.t3, l1PrachInd.ta, l1PrachInd.power);
        }   
        break;         
        case PH_PACCH_IND:   
        {
            MsgPacchInd    l1PacchInd;
            msgDecodePacch((char *)msgReq->l1Msg, &l1PacchInd);
            DBG_TRACE("RLC/MAC: Received PH_PACCH_IND msg - TRX(%d) TS(%#x) tag(%d)\n",
                l1PacchInd.header.trx, l1PacchInd.channel.typeTs, l1PacchInd.tbf);
            status = ProcessPhPacchInd(&l1PacchInd);
        }            
        break;         
        case PH_PDTCH_IND: 
        {
            MsgPdtchInd    l1PdtchInd;
            msgDecodePdtch((char *)msgReq->l1Msg, &l1PdtchInd, msgReq->len);
            DBG_TRACE("RLC/MAC: Received PH_PDTCH_IND msg - TRX(%d) TS(%#x) rate(%d)\n",
                l1PdtchInd.header.trx, l1PdtchInd.channel.typeTs, l1PdtchInd.codeRate);
            status = ProcessPhPdtchInd(&l1PdtchInd);
        }            
        break;         
        case PDCH_TBF_PTC_IND:
        {
            MsgUlPowerTiming    l1PtcInd;
            msgDecodeUlPowerTiming((char *)msgReq->l1Msg, &l1PtcInd);
            DBG_TRACE("RLC/MAC: Received PDCH_TBF_PTC_IND msg - TRX(%#x) TS(%d)\n",
                l1PtcInd.header.trx, l1PtcInd.channel.typeTs);
        }            
        break;         
        case PDCH_UL_TBF_ACT_ACK:
        {
            MsgUlTbfActivationAck    l1UlActAck;
            msgDecodeUlTbfActivationAck((char *)msgReq->l1Msg, &l1UlActAck);
            DBG_TRACE("RLC/MAC: Received PDCH_UL_TBF_ACT_ACK msg - TRX(%d) TLLI(%#x)\n",
                l1UlActAck.header.trx, l1UlActAck.tlli);
        }            
        break;         
        case PDCH_DL_TBF_ACT_ACK:
        {
            MsgDlTbfActivationAck    l1DlActAck;
            msgDecodeDlTbfActivationAck((char *)msgReq->l1Msg, &l1DlActAck);
            DBG_TRACE("RLC/MAC: Received PDCH_DL_TBF_ACT_ACK msg - TRX(%d) TLLI(%#x)\n",
                l1DlActAck.header.trx, l1DlActAck.tlli);
        }            
        break;         
        case PDCH_UL_TBF_DEACT_ACK:
        {
            MsgUlTbfDeactivationAck    l1UlDeactAck;
            msgDecodeUlTbfDeactivationAck((char *)msgReq->l1Msg, &l1UlDeactAck);
            DBG_TRACE("RLC/MAC: Received PDCH_UL_TBF_DEACT_ACK msg - TRX(%d) TLLI(%#x)\n",
                l1UlDeactAck.header.trx, l1UlDeactAck.tlli);
        }            
        break;         
        case PDCH_DL_TBF_DEACT_ACK:
        {
            MsgDlTbfDeactivationAck    l1DlDeactAck;
            msgDecodeDlTbfDeactivationAck((char *)msgReq->l1Msg, &l1DlDeactAck);
            DBG_TRACE("RLC/MAC: Received PDCH_DL_TBF_DEACT_ACK msg - TRX(%d) TLLI(%#x)\n",
                l1DlDeactAck.header.trx, l1DlDeactAck.tlli);
        } 
        break;           
        case PDCH_UL_TBF_RECONFIG_ACK:
        {
            MsgUlTbfReconfigAck    l1UlReconfigAck;
            msgDecodeUlTbfReconfigAck((char *)msgReq->l1Msg, &l1UlReconfigAck);
            DBG_TRACE("RLC/MAC: Received PDCH_UL_TBF_RECONFIG_ACK msg - TRX(%d) TLLI(%#x)\n",
                l1UlReconfigAck.header.trx, l1UlReconfigAck.tlli);
        }            
        break;         
        case PDCH_DL_TBF_RECONFIG_ACK:
        {
            MsgDlTbfReconfigAck    l1DlReconfigAck;
            msgDecodeDlTbfReconfigAck((char *)msgReq->l1Msg, &l1DlReconfigAck);
            DBG_TRACE("RLC/MAC: Received PDCH_DL_TBF_RECONFIG_ACK msg - TRX(%d) TLLI(%#x)\n",
                l1DlReconfigAck.header.trx, l1DlReconfigAck.tlli);
        }            
        break;         
        case PH_PACCH_NACK:
        {
            MsgPacchNack    l1PacchNack;
            msgDecodePacchNack((char *)msgReq->l1Msg, &l1PacchNack);
            DBG_TRACE("RLC/MAC: Received PH_PACCH_NACK msg - TRX(%d) TS(%#x) tag(%d)\n",
                l1PacchNack.header.trx, l1PacchNack.channel.typeTs, l1PacchNack.tbf);
            ULTbf *pUlTbf;
            DLTbf *pDlTbf;
            if ((pUlTbf = tbfPool.FindULTbf((unsigned char)l1PacchNack.tbf)))
            {
                pUlTbf->rlcLink.ReceivePacketControlNack();
            }
            else if ((pDlTbf = tbfPool.FindDLTbf((unsigned char)l1PacchNack.tbf)))
            {
                pDlTbf->rlcLink.ReceivePacketControlNack();
            }
            else
            {
                DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Unkown control ack tag %d\n",
                    l1PacchNack.tbf);
                status = JC_ERROR;
            }
        }            
        break;         
        case PDCH_SINGLE_BLOCK_ASSIGN_ALERT:
        {
            MsgSingleBlockAssignAlert    l1SingleBlockAssignAlert;
            msgDecodeSingleBlockAssignAlert((char *)msgReq->l1Msg, &l1SingleBlockAssignAlert);
            DBG_TRACE("RLC/MAC: Received PDCH_SINGLE_BLOCK_ASSIGN_ALERT msg - TRX(%d), TS(%d)\n"
                "\tframeNum(%d) ARFCN(%d)\n", l1SingleBlockAssignAlert.header.trx, 
                l1SingleBlockAssignAlert.ts, l1SingleBlockAssignAlert.frameNum,
                l1SingleBlockAssignAlert.ARFCN);
            for (int i = 0; i < MAX_TRX; i++)
            {
                if (grr_GetTrxSlotArfcn(i, l1SingleBlockAssignAlert.ts) ==
                    l1SingleBlockAssignAlert.ARFCN)
                {
                    break;
                }
            }
            
            if (i < MAX_TRX)
            {
                SendSingleBlockAssignAlertMsg(i, l1SingleBlockAssignAlert.ts, 
                    l1SingleBlockAssignAlert.frameNum);
            }
            else
            {
                DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Can't match arfcn %d for"
                    "single block assign alert\n", l1SingleBlockAssignAlert.ARFCN);
                status = JC_ERROR;
            }
        }
        break;           
        case PDCH_DL_ASSIGN_ALERT:
        {
            MsgDLAssignAlert    l1DLAssignAlert;
            msgDecodeDLAssignAlert((char *)msgReq->l1Msg, &l1DLAssignAlert);
            DBG_TRACE("RLC/MAC: Received PDCH_DL_ASSIGN_ALERT msg - TRX(%d) TS(%d)\n"
                "\tframeNum(%d) ARFCN(%d) tag(%d)\n", l1DLAssignAlert.header.trx, 
                l1DLAssignAlert.ts, l1DLAssignAlert.frameNum,
                l1DLAssignAlert.ARFCN, l1DLAssignAlert.tag);
            for (int i = 0; i < MAX_TRX; i++)
            {
                if (grr_GetTrxSlotArfcn(i, l1DLAssignAlert.ts) ==
                    l1DLAssignAlert.ARFCN)
                {
                    break;
                }
            }
            
            if (i < MAX_TRX)
            {
                SendDLAssignAlertMsg(i, l1DLAssignAlert.ts, 
                    l1DLAssignAlert.frameNum, l1DLAssignAlert.tag);
            }
            else
            {
                DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Can't match arfcn %d for "
                    "downlink assign alert\n", l1DLAssignAlert.ARFCN);
                status = JC_ERROR;
            }
        }
        break; 
        case PH_PACCH_AB_IND:          
        {
            MsgPacchAbInd    l1PacchAbInd;
            msgDecodePacchAbInd((char *)msgReq->l1Msg, &l1PacchAbInd);
            DBG_TRACE("RLC/MAC: Received PH_PACCH_AB_IND msg - TRX(%d) TS(%#x) tag(%d) ta(%d)\n",
                l1PacchAbInd.header.trx, l1PacchAbInd.channel.typeTs, l1PacchAbInd.tag,
                l1PacchAbInd.ta);
            DecodeULRlcControlBlockAccessBurst(&l1PacchAbInd);
        }    
        break;        
        case PDCH_CODE_RATE_REQ:          
        {
            MsgCodeRateReq    l1CodeRateReq;
            msgDecodeCodeRateReq((char *)msgReq->l1Msg, &l1CodeRateReq);
            if (l1CodeRateReq.tbfType == 1)
            {
                DBG_TRACE("RLC/MAC: Received PDCH_CODE_RATE_REQ msg - tbfType(DL) TFI(%d) Rate(%d)\n",
                    l1CodeRateReq.groupTfi, l1CodeRateReq.codeRate);
                DLTbf *pDlTbf;
                IeGlobalTFI tfi;
                tfi.SetGlobalTFI(l1CodeRateReq.groupTfi, IeGlobalTFI::DOWNLINK_TFI);
                if ((pDlTbf = tbfPool.FindDLTbf(tfi)))
                {
                    pDlTbf->rlcLink.SetCurrentCodingRate(
                        (IeChannelCodingCommand::CHANNEL_CODING_COMMAND)l1CodeRateReq.codeRate);
                }
                else
                {
                    DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Can't match tfi %d for "
                        "CODE RATE REQ\n", l1CodeRateReq.groupTfi);
                    status = JC_ERROR;
                }
            }
            else
            {
                DBG_TRACE("RLC/MAC: Received PDCH_CODE_RATE_REQ msg - tbfType(UL) TFI(%d) Rate(%d)\n",
                    l1CodeRateReq.groupTfi, l1CodeRateReq.codeRate);
                ULTbf *pUlTbf;
                IeGlobalTFI tfi;
                tfi.SetGlobalTFI(l1CodeRateReq.groupTfi, IeGlobalTFI::UPLINK_TFI);
                if ((pUlTbf = tbfPool.FindULTbf(tfi)))
                {
                    pUlTbf->rlcLink.SetCurrentCodingRate(
                        (IeChannelCodingCommand::CHANNEL_CODING_COMMAND)l1CodeRateReq.codeRate);
                }
                else
                {
                    DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Can't match tfi %d for "
                        "CODE RATE REQ\n", l1CodeRateReq.groupTfi);
                    status = JC_ERROR;
                }
            }
        }    
        break;        
        default:
        {
            DBG_WARNING("RlcMacTask::DecodeL1MsgInd: Invalid msg type %#x\n",
                l1MsgType);
            return (JC_ERROR);
        }            
    }
    
    DBG_LEAVE();
    return(status);
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ProcessPhPdtchInd
**
**    PURPOSE: A bitstream object is created from the PDTCH INDICATION
**      message.  The uplink RLC/MAC header will then be decoded and the
**      appropriate action will be taken.
**
**    INPUT PARAMETERS: pL1PdtchInd - pointer to the PDTCH message
**
**    RETURN VALUE(S): JC_OK - no decoding errors
**                     JC_ERROR - decoding errors
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ProcessPhPdtchInd(MsgPdtchInd *pL1PdtchInd)
{
    DBG_FUNC("RlcMacTask::ProcessPhPdtchInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;

    // Grab an uplink data block from the uplink data block pool.
    ULDataBlock *pUlDataBlock = ULRlcLink::sysUlDataBlockPool.GetULDataBlock();
    
    if (pUlDataBlock)
    {
        // A bitstream will be created from the PDTCH INDICATION structure
        pUlDataBlock->streamIn.InitBitStreamIn((unsigned char *)pL1PdtchInd->data, MAX_RLC_DATA_MSG);
        
        // Insert the coding rate into the uplink data block.
        pUlDataBlock->chanCoding.SetChannelCodingCommand(
            (IeChannelCodingCommand::CHANNEL_CODING_COMMAND)pL1PdtchInd->codeRate);
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessPhPdtchInd: Unable to allocate ULDataBlock\n");
        status = JC_ERROR;
        DBG_LEAVE();
        return(status);
    }
    
    // Decode the RLC/MAC message header.
    if (pUlDataBlock->ulHeader.DecodeULMacHeader(pUlDataBlock->streamIn) == RLC_MAC_SUCCESS)
    {
        if (pUlDataBlock->ulHeader.GetPayloadType() == ULMacHeader::RLC_DATA_BLOCK)
        {
            // Display header details if enabled.
            pUlDataBlock->ulHeader.DisplayDetails();
            pUlDataBlock->DisplayDataBlock();
            ProcessULRlcDataBlock(pL1PdtchInd, pUlDataBlock);
        }
        else
        {
            DBG_WARNING("RlcMacTask::ProcessPhPdtchInd: Invalid uplink payload type %d\n",
                pUlDataBlock->ulHeader.GetPayloadType());
            status = JC_ERROR;
            ULRlcLink::sysUlDataBlockPool.ReturnULDataBlock(pUlDataBlock);
        }
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessPhPdtchInd: Unable to decode MAC header\n");
        status = JC_ERROR;
        ULRlcLink::sysUlDataBlockPool.ReturnULDataBlock(pUlDataBlock);
    }
    
    DBG_LEAVE();
    return(status);
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ProcessPhRTSInd
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: pL1RTSInd - pointer to the RTS message
**
**    RETURN VALUE(S): JC_OK 
**                     JC_ERROR 
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ProcessPhRTSInd(MsgReadyToSend *pL1RTSInd)
{
    DBG_FUNC("RlcMacTask::ProcessPhRTSInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;

    // Handle a signalling RTS
    if ((pL1RTSInd->channel.typeTs >> 3) == CHAN_NUM_IE_PACCH)
    {
        bool notDone = TRUE;
        
        // Update the signalling queue RTS count.
        tsPool.IncSignallingRTSCount(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07, pL1RTSInd->numBuffs);
        
        // Send any signalling messages that might be queued.
        while (notDone)
        {
            MsgPacchReq *pPacchMsg = tsPool.DequeueDLSignallingMsg(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07);
            
            // If there is a queued signalling message then send it.
            if (pPacchMsg &&
                (tsPool.GetSignallingRTSCount(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07) > 0))
            {
                // Ask the scheduler for a USF
                ULTbf *pUlTbf;
                while (1)
                {
                    ScheduleRLCBlock(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07, &pUlTbf);
                    // Uplink TBF available
                    if (pUlTbf)
                    {
                        if (pUlTbf->rlcLink.IncrementN3101())
                        {
                            pPacchMsg->data[0] |= pUlTbf->usf[pL1RTSInd->channel.typeTs & 0x07];
                            break;
                        }
                    }
                    // No uplink TBFs to schedule.
                    else
                    {
                        break;
                    }
                }
            
                char msg[HPI_TX_MSG_SIZE];
                int len = msgEncodePacch(pPacchMsg, msg + 4);
                msg[3] = (char)(len >> 24);
                msg[2] = (char)(len >> 16);
                msg[1] = (char)(len >> 8);
                msg[0] = (char)len;
                sendDsp((unsigned char *)msg, len + 4);
                
                // Update RTS count.  If no more RTS available then we are done sending.
                tsPool.DecSignallingRTSCount(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07);
                if (tsPool.GetSignallingRTSCount(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07) == 0)
                {
                    notDone = FALSE;
                }
            }
            else
            {
                // Send a dummy block if needed.  Only send a single dummy block even
                // though more RTS's might be available.
                SendDownlinkDummyControlBlock(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07);
                notDone = FALSE;
            }
        }
    }
    // Handle a data RTS
    else if ((pL1RTSInd->channel.typeTs >> 3) == CHAN_NUM_IE_PDTCH)
    {
        // Update the data queue RTS count.
        tsPool.IncDataRTSCount(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07, 
            pL1RTSInd->numBuffs);

        // Try and send data block(s).  If no data blocks sent then try and send
        // a dummy control block.
        if (!SendDownlinkDataBlock(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07))
        {
            SendDownlinkDummyControlBlock(pL1RTSInd->header.trx, pL1RTSInd->channel.typeTs & 0x07);
        }
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessPhRTSInd: Invalid channel type %#x\n", pL1RTSInd->channel.typeTs);       
    }
    
    DBG_LEAVE();
    return(status);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDownlinkDataBlock
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: trx - trx to send dummy block
**                      ts - timeslot to send dummy block
**
**    RETURN VALUE(S): TRUE - if data block sent 
**                     FALSE - if data block not sent 
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::SendDownlinkDataBlock(int trx, int ts)
{
    DBG_FUNC("RlcMacTask::SendDownlinkDataBlock", RLC_MAC);
    DBG_ENTER();
    
    bool blockSent = FALSE;
    bool notDone = TRUE;
    DLTbf *pFirstDlTbf = 0;
    
    // Make sure we have at least one RTS available.
    if (tsPool.GetDataRTSCount(trx, ts) == 0)
    {
        notDone = FALSE;
    }
    
    while (notDone)
    {
        DLTbf *pDlTbf;
        
        // If available get the next downlink 
        ScheduleRLCBlock(trx, ts, &pDlTbf);
        
        // If a downlink TBF is available then grab a data block to send.
        if (pDlTbf)
        {
            // If the same TBF has been seen twice and a packet is not available
            // then don't loop again.
            if (pDlTbf == pFirstDlTbf)
            {
                notDone = FALSE;
            }
            else
            {
                // Record the first TBF that the scheduler returns.
                if (!pFirstDlTbf)
                {
                    pFirstDlTbf = pDlTbf;
                }
            
                DLDataBlock *pDlDataBlock;
                unsigned char ctrlAckTag;
                char burstType;
            
                // Grab a data block from the DL tbf.
                if ((pDlDataBlock = pDlTbf->rlcLink.GetDLRlcBlock(ctrlAckTag, burstType, trx, ts)))
                {
                    SendPdtchMsg(trx, ts, ctrlAckTag, burstType, pDlDataBlock);
                
                    // Update RTS count.  If no more RTS available then we are done sending.
                    tsPool.DecDataRTSCount(trx, ts);
                    if (tsPool.GetDataRTSCount(trx, ts) == 0)
                    {
                        notDone = FALSE;
                    }
                    blockSent = TRUE;
                    pFirstDlTbf = 0;
                }
            }
        }
        // No downlink TBF available so just return.
        else
        {
            notDone = FALSE;
        }
    }
    DBG_LEAVE();
    return(blockSent);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDownlinkDummyControlBlock
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: trx - trx to send dummy block
**                      ts - timeslot to send dummy block
**
**    RETURN VALUE(S): TRUE - if dummy block sent 
**                     FALSE - if dummy block not sent 
**
**----------------------------------------------------------------------------*/
bool RlcMacTask::SendDownlinkDummyControlBlock(int trx, int ts)
{
    DBG_FUNC("RlcMacTask::SendDownlinkDummyControlBlock", RLC_MAC);
    DBG_ENTER();
    
    bool blockSent = FALSE;
    
    // Only send a dummy block if there are no data messages queued and there are no
    // control message queued.
    if ((tsPool.GetSignallingRTSCount(trx, ts) >= MAX_SIGNALLING_RTS_COUNT) &&
        (tsPool.GetDataRTSCount(trx, ts) >= MAX_DATA_RTS_COUNT) && tbfPool.AnyActiveTBF(trx, ts))
    {
        MsgPacketDownlinkDummyControlBlock dummyMsg;
        DLMacHeader                        dummyMsgHdr;

        // Assemble RLC/MAC header.
        dummyMsgHdr.SetPayloadType(DLMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS);
        // RRBP will not be valid so just set it to anything.
        dummyMsgHdr.SetRRBP(DLMacHeader::FN_N_13);
        dummyMsgHdr.SetSPBit(DLMacHeader::RRBP_NOT_VALID);
        // USF is set later.
    
        // Assemble Packet Downlink Dummy Control Block.
        dummyMsg.pageMode.SetPageMode(IePageMode::NORMAL_PAGING);
    
        if (SendPacchMsg(trx, ts, &dummyMsgHdr, &dummyMsg, CTL_ACK_TAG_NOT_APPLICABLE,
            RLC_MAC_NORMAL_BURST) != JC_OK)
        {
            DBG_WARNING("RlcMacTask::SendDownlinkDummyControlBlock: SendPacchMsg failed\n");       
        }
        
        blockSent = TRUE;
    }
    
    DBG_LEAVE();
    return(blockSent);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ProcessPhPacchInd
**
**    PURPOSE: A bitstream object is created from the PACCH INDICATION
**      message.  The uplink RLC/MAC header will then be decoded and the
**      appropriate action will be taken.
**
**    INPUT PARAMETERS: pL1PacchInd - pointer to the PACCH message
**
**    RETURN VALUE(S): JC_OK - no decoding errors
**                     JC_ERROR - decoding errors
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ProcessPhPacchInd(MsgPacchInd *pL1PacchInd)
{
    DBG_FUNC("RlcMacTask::ProcessPhPacchInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;

    // A bitstream will be created from the PACCH INDICATION structure
    BitStreamIn l1MsgBitsIn((unsigned char *)pL1PacchInd->data, MAX_RLC_CONTROL_MSG);
    
    ULMacHeader rlcMacMsgHeader;
    
    // Decode the RLC/MAC message header.
    if (rlcMacMsgHeader.DecodeULMacHeader(l1MsgBitsIn) == RLC_MAC_SUCCESS)
    {
        // Handle an RLC control block.  Note that in the uplink
        // direction the optional octets are not allowed.
        if (rlcMacMsgHeader.GetPayloadType() == ULMacHeader::RLC_CTRL_BLOCK_NO_OPT_OCTETS)
        {
            // Display header details if enabled.
            rlcMacMsgHeader.DisplayDetails();
            DecodeULRlcControlBlock(pL1PacchInd, rlcMacMsgHeader, l1MsgBitsIn); 
        }
        else
        {
            DBG_WARNING("RlcMacTask::ProcessPhPacchInd: Invalid uplink payload type %d\n",
                rlcMacMsgHeader.GetPayloadType());
            status = JC_ERROR;
        }
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessPhPacchInd: Unable to decode MAC header\n");
        status = JC_ERROR;
    }
    
    DBG_LEAVE();
    return(status);
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendULActivationMsg
**
**    PURPOSE: Build an Uplink TBF Activation message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: pUlTbf - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendULActivationMsg(ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::SendULActivationMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgUlTbfActivation actMsg;
    
    actMsg.header.function = PDCH_MANAGE_MSG;
    actMsg.header.type = PDCH_UL_TBF_ACT_REQ;
    actMsg.header.trx =  pUlTbf->trx;
    actMsg.tlli = (int)pUlTbf->tlli.GetTLLI();
    actMsg.tbfMode = 1; // Dynamic
    actMsg.groupTfi = (char)pUlTbf->tfi.GetTFI();
    actMsg.channel = pUlTbf->rlcLink.GetCurrentCodingRate();
    actMsg.tsAllocation = 0;
    actMsg.usfTsMap = 0;
    int tsMask = 1;
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pUlTbf->allocatedTs[i])
        {
            actMsg.tsAllocation |= tsMask;
        }
        tsMask <<= 1;
        
        actMsg.usfTsMap |= (((int)pUlTbf->usf[i]) << i*4);
    }
    
    actMsg.tai.tai = (char)pUlTbf->tai;
    actMsg.tai.ts = (char)pUlTbf->taiTs;
    // actMsg.startTime
    
    DBG_TRACE("RLC/MAC: Sending PDCH_UL_TBF_ACT_REQ msg - TBF(%#x)\n"
        "\tTLLI(%#x) TFI(%d) TRX(%d) TSBitmap(%#x)\n", pUlTbf, pUlTbf->tlli.GetTLLI(),
        pUlTbf->tfi.GetTFI(), pUlTbf->trx, actMsg.tsAllocation);
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeUlTbfActivation(&actMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendULReconfigurationMsg
**
**    PURPOSE: Build an Uplink TBF Reconfiguration message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: pUlTbf - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendULReconfigurationMsg(ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::SendULReconfigurationMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgUlTbfReconfig reconMsg;
    
    reconMsg.header.function = PDCH_MANAGE_MSG;
    reconMsg.header.type = PDCH_UL_TBF_RECONFIG_REQ;
    reconMsg.header.trx =  pUlTbf->trx;
    reconMsg.tlli = (int)pUlTbf->tlli.GetTLLI();
    reconMsg.tbfMode = 1; // Dynamic
    reconMsg.groupTfi = (char)pUlTbf->tfi.GetTFI();
    reconMsg.channel = pUlTbf->rlcLink.GetCurrentCodingRate();
    reconMsg.tsAllocation = 0;
    reconMsg.usfTsMap = 0;
    int tsMask = 1;
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pUlTbf->allocatedTs[i])
        {
            reconMsg.tsAllocation |= tsMask;
        }
        tsMask <<= 1;
        
        reconMsg.usfTsMap |= (((int)pUlTbf->usf[i]) << i*4);
    }
    
    reconMsg.tai.tai = (char)pUlTbf->tai;
    reconMsg.tai.ts = (char)pUlTbf->taiTs;
    // actMsg.startTime
    
    DBG_TRACE("RLC/MAC: Sending PDCH_UL_TBF_RECONFIG_REQ msg - TBF(%#x)\n"
        "\tTLLI(%#x) TFI(%d) TRX(%d) TSBitmap(%#x)\n", pUlTbf, pUlTbf->tlli.GetTLLI(),
        pUlTbf->tfi.GetTFI(), pUlTbf->trx, reconMsg.tsAllocation);
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeUlTbfReconfig(&reconMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendULDeactivationMsg
**
**    PURPOSE: Build an Uplink TBF Deactivation message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: pUlTbf - pointer to the uplink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendULDeactivationMsg(ULTbf *pUlTbf)
{
    DBG_FUNC("RlcMacTask::SendULDeactivationMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgUlTbfDeactivation deactMsg;
    
    DBG_TRACE("RLC/MAC: Sending PDCH_UL_TBF_DEACT_REQ msg - TBF(%#x) TLLI(%#x)\n",
        pUlTbf, pUlTbf->tlli.GetTLLI());
    
    deactMsg.header.function = PDCH_MANAGE_MSG;
    deactMsg.header.type = PDCH_UL_TBF_DEACT_REQ;
    deactMsg.header.trx =  pUlTbf->trx;
    deactMsg.tlli = (int)pUlTbf->tlli.GetTLLI();
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeUlTbfDeactivation(&deactMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDLActivationMsg
**
**    PURPOSE: Build an Downlink TBF Activation message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: pDlTbf - pointer to the downlink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendDLActivationMsg(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::SendDLActivationMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgDlTbfActivation actMsg;
    
    actMsg.header.function = PDCH_MANAGE_MSG;
    actMsg.header.type = PDCH_DL_TBF_ACT_REQ;
    actMsg.header.trx =  pDlTbf->trx;
    actMsg.tlli = (int)pDlTbf->tlli.GetTLLI();
    actMsg.groupTfi = (char)pDlTbf->tfi.GetTFI();
    actMsg.channel = pDlTbf->rlcLink.GetCurrentCodingRate();
    actMsg.tsAllocation = 0;
    int tsMask = 1;
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pDlTbf->allocatedTs[i])
        {
            actMsg.tsAllocation |= tsMask;
        }
        tsMask <<= 1;
    }
    
    actMsg.ta = 0;  // Need to fix this - incorrect TA type
    actMsg.msPower = 0; // What should this be????
    //actMsg.tai.tai = (char)pDlTbf->tai;
    //actMsg.tai.ts = (char)pDlTbf->taiTs;
    // actMsg.startTime
    
    DBG_TRACE("RLC/MAC: Sending PDCH_DL_TBF_ACT_REQ msg - TBF(%#x)\n"
        "\tTLLI(%#x) TFI(%d) TRX(%d) TSBitmap(%#x)\n", pDlTbf, pDlTbf->tlli.GetTLLI(),
        pDlTbf->tfi.GetTFI(), pDlTbf->trx, actMsg.tsAllocation);
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeDlTbfActivation(&actMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDLReconfigurationMsg
**
**    PURPOSE: Build an Downlink TBF Reconfiguration message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: pDlTbf - pointer to the downlink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendDLReconfigurationMsg(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::SendDLReconfigurationMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgDlTbfReconfig reconMsg;
    
    reconMsg.header.function = PDCH_MANAGE_MSG;
    reconMsg.header.type = PDCH_DL_TBF_RECONFIG_REQ;
    reconMsg.header.trx =  pDlTbf->trx;
    reconMsg.tlli = (int)pDlTbf->tlli.GetTLLI();
    reconMsg.groupTfi = (char)pDlTbf->tfi.GetTFI();
    reconMsg.channel = pDlTbf->rlcLink.GetCurrentCodingRate(); 
    reconMsg.tsAllocation = 0;
    int tsMask = 1;
    for (int i = 0; i < MAX_TIMESLOTS; i++)
    {
        if (pDlTbf->allocatedTs[i])
        {
            reconMsg.tsAllocation |= tsMask;
        }
        tsMask <<= 1;
    }
    
    //reconMsg.tai.tai = (char)pDlTbf->tai;
    //reconMsg.tai.ts = (char)pDlTbf->taiTs;
    // reconMsg.startTime
    
    DBG_TRACE("RLC/MAC: Sending PDCH_DL_TBF_RECONFIG_REQ msg - TBF(%#x)\n"
        "\tTLLI(%#x) TFI(%d) TRX(%d) TSBitmap(%#x)\n", pDlTbf, pDlTbf->tlli.GetTLLI(),
        pDlTbf->tfi.GetTFI(), pDlTbf->trx, reconMsg.tsAllocation);
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeDlTbfReconfig(&reconMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDLDeactivationMsg
**
**    PURPOSE: Build an Downlink TBF Deactivation message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: pDlTbf - pointer to the downlink tbf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendDLDeactivationMsg(DLTbf *pDlTbf)
{
    DBG_FUNC("RlcMacTask::SendDLDeactivationMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgDlTbfDeactivation deactMsg;
    
    DBG_TRACE("RLC/MAC: Sending PDCH_DL_TBF_DEACT_REQ msg - TBF(%#x) TLLI(%#x)\n",
        pDlTbf, pDlTbf->tlli.GetTLLI());
    
    deactMsg.header.function = PDCH_MANAGE_MSG;
    deactMsg.header.type = PDCH_DL_TBF_DEACT_REQ;
    deactMsg.header.trx =  pDlTbf->trx;
    deactMsg.tlli = (int)pDlTbf->tlli.GetTLLI();
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeDlTbfDeactivation(&deactMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendSingleBlockAssignAlertMsg
**
**    PURPOSE: Build a Single Block Assign Alert message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: trx - trx for the single block assignment
**                      ts - timeslot for the single block assignment
**                      framNum - absolute frame number for single block assgn
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendSingleBlockAssignAlertMsg(int trx, int ts, unsigned int frameNum)
{
    DBG_FUNC("RlcMacTask::SendSingleBlockAssignAlertMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgSingleBlockAssignAlertAck assAlertMsg;
    
    DBG_TRACE("RLC/MAC: Sending PDCH_SINGLE_BLOCK_ASSIGN_ALERT_ACK msg - TRX(%d) TS(%d)\n"
        "\tframeNum(%d)", trx, ts, frameNum); 
    
    assAlertMsg.header.function = PDCH_MANAGE_MSG;
    assAlertMsg.header.type = PDCH_SINGLE_BLOCK_ASSIGN_ALERT_ACK;
    assAlertMsg.header.trx =  (char)trx;
    assAlertMsg.channel.typeTs = (char)(ts | (CHAN_NUM_IE_PDTCH << 3));
    assAlertMsg.channel.ccch = 0;
    assAlertMsg.frameNum = frameNum;
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeSingleBlockAssignAlertAck(&assAlertMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDLAssignAlertMsg
**
**    PURPOSE: Build a Downlink Assign Alert message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: trx - trx for the single block assignment
**                      ts - timeslot for the single block assignment
**                      framNum - absolute frame number for single block assgn
**                      tag - tag to identify MS with DL assignment
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendDLAssignAlertMsg(int trx, int ts, unsigned int frameNum, char tag)
{
    DBG_FUNC("RlcMacTask::SendDLAssignAlertMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgDLAssignAlertAck assAlertMsg;
    
    DBG_TRACE("RLC/MAC: Sending PDCH_DL_ASSIGN_ALERT_ACK msg - TRX(%d) TS(%d)"
        "frameNum(%d) tag(%d)\n", trx, ts, frameNum, tag);
    
    assAlertMsg.header.function = PDCH_MANAGE_MSG;
    assAlertMsg.header.type = PDCH_DL_ASSIGN_ALERT_ACK;
    assAlertMsg.header.trx =  (char)trx;
    assAlertMsg.channel.typeTs = (char)(ts | (CHAN_NUM_IE_PDTCH << 3));
    assAlertMsg.channel.ccch = 0;
    assAlertMsg.frameNum = frameNum;
    assAlertMsg.tag = tag;
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeDLAssignAlertAck(&assAlertMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendDLAckNackInfoMsg
**
**    PURPOSE: Build a Downlink ACK/NACK Info message and send it to
**      the dsp.
**
**    INPUT PARAMETERS: trx - trx for the single block assignment
**                      pDlTbf - pointer to the DL TBF
**                      pMsg - pointer to DL ACK/NACK message
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RlcMacTask::SendDLAckNackInfoMsg(int trx, DLTbf *pDlTbf, 
    MsgPacketDownlinkAckNack *pMsg)
{
    DBG_FUNC("RlcMacTask::SendDLAckNackInfoMsg", RLC_MAC);
    DBG_ENTER();
    
    MsgDLAckNackInfo dlAckNackInfoMsg;
    
    DBG_TRACE("RLC/MAC: Sending PDCH_DL_ACK_NACK_INFO msg - TRX(%d) TLLI(%#x) TFI(%d)",
        trx, pDlTbf->tlli.GetTLLI(), pDlTbf->tfi.GetTFI());
    
    dlAckNackInfoMsg.header.function = PDCH_MANAGE_MSG;
    dlAckNackInfoMsg.header.type = PDCH_DL_ACK_NACK_INFO;
    dlAckNackInfoMsg.header.trx =  (char)trx;
    dlAckNackInfoMsg.tlli = (int)pDlTbf->tlli.GetTLLI();
    dlAckNackInfoMsg.groupTfi = (char)pDlTbf->tfi.GetTFI();
    dlAckNackInfoMsg.cValue = pMsg->cValue;
    dlAckNackInfoMsg.rxqual = pMsg->rxqual;
    dlAckNackInfoMsg.iLevelMask = 0;
    int bitSelector = 1;
    for (int i=0; i<MAX_TIMESLOTS; i++)
    {
        dlAckNackInfoMsg.iLevelByTs[i] = (char)pMsg->ILevel[i].iLevel;
        if (pMsg->ILevel[i].isValid)
            dlAckNackInfoMsg.iLevelMask |= bitSelector;
        bitSelector <<= 1;    
    }
    
    // Encode and send layer 1 message.
    char msg[HPI_TX_MSG_SIZE];    
    int len = msgEncodeDLAckNackInfo(&dlAckNackInfoMsg, msg + 4);
    msg[3] = (char)(len >> 24);
    msg[2] = (char)(len >> 16);
    msg[1] = (char)(len >> 8);
    msg[0] = (char)len;
    sendDsp((unsigned char *)msg, len + 4);
    
    DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPacchMsg
**
**    PURPOSE:  Encode the RLC/MAC header and signalling message and transmit
**      on the specified TRX and timeslot.
**
**    INPUT PARAMETERS: trx - trx on which to transmit
**                      ts - timeslot on which to transmit
**                      pDlMsgHdr - pointer to the downlink msg header
**                      pDlMsg - pointer to the downlink signalling message
**                      ctlAckTag - optional tag used for acknowledged control msgs
**                      burstType - if this message is acknowledged then burstType
**                          indicates whether the response is normal or access bursts
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPacchMsg(int trx, int ts, DLMacHeader *pDlMsgHdr,
    DownlinkMsgBase *pDlMsg, unsigned char ctlAckTag, char burstType) 
 {
    DBG_FUNC("RlcMacTask::SendPacchMsg", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DBG_TRACE("RLC/MAC: Sending PH_PACCH_REQ msg - TRX(%d) TS(%d) tag(%d) burst(%d)\n",
        trx, ts, ctlAckTag, burstType);
    
    // If signalling RTS available then the message will be sent immediately so
    // get a USF.
    pDlMsgHdr->SetUSF(0);
    if (tsPool.GetSignallingRTSCount(trx, ts) > 0)
    {
        // Ask the scheduler for a USF
        ULTbf *pUlTbf;
        while (1)
        {
            ScheduleRLCBlock(trx, ts, &pUlTbf);
            // Uplink TBF available
            if (pUlTbf)
            {
                if (pUlTbf->rlcLink.IncrementN3101())
                {
                    pDlMsgHdr->SetUSF(pUlTbf->usf[ts]);
                    break;
                }
            }
            // No uplink TBFs to schedule.
            else
            {
                break;
            }
        }
    }
        
    // Pack the message into an output bitstream.
    BitStreamOut l1MsgOut;
    
    if ((status = pDlMsgHdr->EncodeDLMacHeader(l1MsgOut)) == JC_OK)
    {
        if ((status = pDlMsg->EncodeMsg(l1MsgOut)) == JC_OK)
        {
            MsgPacchReq l1Msg;
            char msg[HPI_TX_MSG_SIZE];
            
            // Build a PACCH message and send it to l1Proxy
            l1MsgOut.PadWithSparePadding(MAX_RLC_CONTROL_MSG);
            l1Msg.header.function = PDCH_MANAGE_MSG;
            l1Msg.header.type = PH_PACCH_REQ;
            l1Msg.header.trx = trx;
            l1Msg.channel.typeTs = ts | (CHAN_NUM_IE_PACCH << 3);
            l1Msg.channel.ccch = 0;
            bcopy((char *)l1MsgOut.GetBitStream(), l1Msg.data, MAX_RLC_CONTROL_MSG);
            l1Msg.tbf = (char)ctlAckTag;
            l1Msg.burstType = burstType;
            
            // If signalling RTS is available send now otherwise queue for later.
            if (tsPool.GetSignallingRTSCount(trx, ts) > 0)
            {
                int len = msgEncodePacch(&l1Msg, msg + 4);
                
                // If enabled display message
                pDlMsg->DisplayMsgShort(&l1MsgOut);
                pDlMsgHdr->DisplayDetails();
                pDlMsg->DisplayMsgDetail();
                msg[3] = (char)(len >> 24);
                msg[2] = (char)(len >> 16);
                msg[1] = (char)(len >> 8);
                msg[0] = (char)len;
                sendDsp((unsigned char *)msg, len + 4);
                tsPool.DecSignallingRTSCount(trx, ts);
            }
            else
            {
                if ((status = tsPool.QueueDLSignallingMsg(trx, ts, &l1Msg)) != JC_OK)
                {
                    DBG_WARNING("RlcMacTask::SendPacchMsg: Unable to queue message %#x\n",
                        status);
                }
            }
        }
        else
        {
            DBG_WARNING("RlcMacTask::SendPacchMsg: RLC/MAC msg encode error %#x\n",
                status);
        }
    }
    else
    {
        DBG_WARNING("RlcMacTask::SendPacchMsg: RLC/MAC header encode error %#x\n",
            status);
    }
    
    DBG_LEAVE();
    return(status);
 }



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::SendPdtchMsg
**
**    PURPOSE:  Encode the RLC/MAC header and append the data message and transmit
**      on the specified TRX and timeslot.
**
**    INPUT PARAMETERS: trx - trx on which to transmit
**                      ts - timeslot on which to transmit
**                      ctlAckTag - optional tag used for acknowledged control msgs
**                      burstType - if this message is acknowledged then burstType
**                          indicates whether the response is normal or access bursts
**                      pDlDataBlock - pointer to the DL Data Block
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::SendPdtchMsg(int trx, int ts, unsigned char ctlAckTag, 
    char burstType, DLDataBlock *pDlDataBlock) 
 {
    DBG_FUNC("RlcMacTask::SendPdtchMsg", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    DBG_TRACE("RLC/MAC: Sending PH_PDTCH_REQ msg - TRX(%d) TS(%d) tag(%d) burst(%d)\n",
        trx, ts, ctlAckTag, burstType);
    
    // Ask the scheduler for a USF
    ULTbf *pUlTbf;
    pDlDataBlock->dlHeader.SetUSF(0);
    while (1)
    {
        ScheduleRLCBlock(trx, ts, &pUlTbf);
        // Uplink TBF available
        if (pUlTbf)
        {
            if (pUlTbf->rlcLink.IncrementN3101())
            {
                pDlDataBlock->dlHeader.SetUSF(pUlTbf->usf[ts]);
                break;
            }
        }
        // No uplink TBFs to schedule.
        else
        {
            break;
        }
    }
    
    // Pack the message into an output bitstream.
    BitStreamOut l1MsgOut;
    
    if ((status = pDlDataBlock->dlHeader.EncodeDLMacHeader(l1MsgOut)) == JC_OK)
    {
        MsgPdtchReq l1Msg;
        char msg[HPI_TX_MSG_SIZE];
        
        // Build a PDTCH message and send it to l1Proxy
        l1Msg.header.function = PDCH_MANAGE_MSG;
        l1Msg.header.type = PH_PDTCH_REQ;
        l1Msg.header.trx = trx;
        l1Msg.channel.typeTs = ts | (CHAN_NUM_IE_PDTCH << 3);
        l1Msg.channel.ccch = 0;
        l1Msg.tbf = ctlAckTag;
        l1Msg.burstType = burstType;
        l1Msg.codeRate = (char)pDlDataBlock->chanCoding.GetChannelCodingCommand();
        
        // Copy header into message
        bcopy((char *)l1MsgOut.GetBitStream(), l1Msg.data, l1MsgOut.GetStreamLen());
        // Copy data into message.
        bcopy((char *)pDlDataBlock->streamOut, l1Msg.data + l1MsgOut.GetStreamLen(), pDlDataBlock->streamOutLen);
        // Fill the remaining data with filler.
        bfill(l1Msg.data + l1MsgOut.GetStreamLen() + pDlDataBlock->streamOutLen, 
            pDlDataBlock->chanCoding.GetRLCDataBlockSize() - (l1MsgOut.GetStreamLen() + pDlDataBlock->streamOutLen), 0x2b);
        
        int len = msgEncodePdtch(&l1Msg, msg + 4, pDlDataBlock->chanCoding.GetRLCDataBlockSize());
        // Display header details if enabled.
        pDlDataBlock->dlHeader.DisplayDetails();
        pDlDataBlock->DisplayDataBlock();
        msg[3] = (char)(len >> 24);
        msg[2] = (char)(len >> 16);
        msg[1] = (char)(len >> 8);
        msg[0] = (char)len;
        sendDsp((unsigned char *)msg, len + 4);
        tsPool.DecDataRTSCount(trx, ts);
    }
    else
    {
        DBG_WARNING("RlcMacTask::SendPdtchMsg: RLC/MAC header encode error %#x\n",
            status);
    }
    
    DBG_LEAVE();
    return(status);
 }








