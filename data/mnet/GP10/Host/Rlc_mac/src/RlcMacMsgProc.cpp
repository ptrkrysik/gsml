// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacMsgProc.cpp
// Author(s)   : Tim Olson
// Create Date : 12/7/2000
// Description : 
//
// *******************************************************************


#include "RlcMacTask.h"
#include "Rlc_mac/RlcMacIntf.h"

#include "MsgPacketCellChangeFailure.h"
#include "MsgPacketControlAcknowledgement.h"
#include "MsgPacketDownlinkAckNack.h"
#include "MsgPacketMobileTBFStatus.h"
#include "MsgPacketResourceRequest.h"
#include "MsgPacketUplinkAckNack.h"
#include "MsgPacketUplinkDummyControlBlock.h"


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ProcessULRlcDataBlock
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: rlcMacMsgHeader - MAC/RLC message header
**                      l1MsgBitsIn - message bit passed from layer 1
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ProcessULRlcDataBlock(MsgPdtchInd *l1PdtchInd, 
    ULDataBlock *pUlDataBlock)
 {
    DBG_FUNC("RlcMacTask::ProcessULRlcDataBlock", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    ULTbf *pUlTbf = tsPool.FindULTbf(l1PdtchInd->header.trx, pUlDataBlock->ulHeader.GetTFI());
    
    if (pUlTbf)
    {
        // Deliver uplink data block to the uplink TBF.
        pUlTbf->rlcLink.ReceiveULDataBlock(pUlDataBlock);
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessULRlcDataBlock: No uplink TBF for given TFI %d\n",
            pUlDataBlock->ulHeader.GetTFI());
        status = RLC_MAC_ERROR;
        ULRlcLink::sysUlDataBlockPool.ReturnULDataBlock(pUlDataBlock);
    }
    
    DBG_LEAVE();
    return(status);
 }
    


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DecodeULRlcControlBlock
**
**    PURPOSE: This function decodes an uplink RLC control message.  After
**      determining that the message is valid the appropriate processing
**      is initiated.
**
**    INPUT PARAMETERS: rlcMacMsgHeader - MAC/RLC message header
**                      l1MsgBitsIn - message bit passed from layer 1
**
**    RETURN VALUE(S): JC_SUCCESS - if decoding succeeds
**                     JC_ERROR - for decode failure
**
**----------------------------------------------------------------------------*/
 JC_STATUS RlcMacTask::DecodeULRlcControlBlock(MsgPacchInd *l1PacchInd,
    ULMacHeader &rlcMacMsgHeader, BitStreamIn &l1MsgBitsIn)
 {
    DBG_FUNC("RlcMacTask::ProcessULRlcControlBlock", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    RlcMacMsgType msgType;
    
    if (msgType.DecodeMsgType(l1MsgBitsIn) == RLC_MAC_SUCCESS)
    {
        switch (msgType.msgType)
        {
            case RlcMacMsgType::PACKET_CELL_CHANGE_FAILURE:
            {
                MsgPacketCellChangeFailure ctrlMsg;
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                }
                else
                {
                    DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Cell Change Failure decode failed\n");
                    status = JC_ERROR;
                }
            }
            break;
            case RlcMacMsgType::PACKET_CONTROL_ACK:
            {
                MsgPacketControlAcknowledgement ctrlMsg((RlcMacMsgType::RLC_MAC_UPLINK_MSGS)msgType.msgType);
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                    ULTbf *pUlTbf;
                    DLTbf *pDlTbf;
                    if ((pUlTbf = tbfPool.FindULTbf((unsigned char)l1PacchInd->tbf)))
                    {
                        pUlTbf->rlcLink.ReceivePacketControlAck(&ctrlMsg);
                    }
                    else if ((pDlTbf = tbfPool.FindDLTbf((unsigned char)l1PacchInd->tbf)))
                    {
                        pDlTbf->rlcLink.ReceivePacketControlAck(&ctrlMsg, 0xff);
                    }
                    // In unacknowledged mode the MS terminates the TBF by sending
                    // a packet control ack.  For this case no tag was saved.
                    else if ((l1PacchInd->tbf == 0xff) && 
                        ((pDlTbf = tbfPool.FindDLTbf((unsigned long)ctrlMsg.tlli.GetTLLI()))))
                    {
                        pDlTbf->rlcLink.ReceivePacketControlAck(&ctrlMsg, 0xff);
                    }
                    else
                    {    
                        DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Unkown control ack tag %d\n",
                            l1PacchInd->tbf);
                        status = JC_ERROR;
                    }
                }
                else
                {
                    DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Control Ack decode failed\n");
                    status = JC_ERROR;
                }
            }
            break;
            case RlcMacMsgType::PACKET_DOWNLINK_ACK_NACK:
            {
                MsgPacketDownlinkAckNack ctrlMsg;
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                    DLTbf *pDlTbf;
                    if ((pDlTbf = tbfPool.FindDLTbf(ctrlMsg.dlTfi)))
                    {
                        pDlTbf->rlcLink.ReceiveDLAckNack(&ctrlMsg);
                    }
                    else
                    {
                        DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Unkown downlink TFI %d\n",
                            ctrlMsg.dlTfi.GetTFI());
                        status = JC_ERROR;
                    }
                }
                else
                {
                    DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Downlink Ack Nack decode failed\n");
                    status = JC_ERROR;
                }
            }
            break;
            case RlcMacMsgType::PACKET_UPLINK_DUMMY_CONTROL_BLOCK:
            {
                MsgPacketUplinkDummyControlBlock ctrlMsg;
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                }
                else
                {
                    DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Uplink Dummy Control decode failed\n");
                    status = JC_ERROR;
                }
            }
            break;
            case RlcMacMsgType::PACKET_MEASUREMENT_REPORT:
            {
                DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Measurement Report not implemented\n");
                status = JC_ERROR;
            }
            break;
            case RlcMacMsgType::PACKET_RESOURCE_REQUEST:
            {
                MsgPacketResourceRequest ctrlMsg;
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                    AllocateULResources(ctrlMsg, l1PacchInd->header.trx, 
                        l1PacchInd->channel.typeTs & 0x07);
                }
                else
                {
                    DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Resource Request decode failed\n");
                    status = JC_ERROR;
                }
            }
            break;
            case RlcMacMsgType::PACKET_MOBILE_TBF_STATUS:
            {
                MsgPacketMobileTBFStatus ctrlMsg;
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                }
                else
                {
                    DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet Mobile TBF Status decode failed\n");
                    status = JC_ERROR;
                }
            }
            break;
            case RlcMacMsgType::PACKET_PSI_STATUS:
            {
                DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Packet PSI Status not implemented\n");
                status = JC_ERROR;
            }
            break;
            default:
            {
                DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Unknown MsgType %#x\n",
                    msgType.msgType);
                status = RLC_MAC_ERROR;
            }
            break;
        }     
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessULRlcControlBlock: Unable to decode MsgType\n");
        status = JC_ERROR;
    }
    
    DBG_LEAVE();
    return(status);
 }



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DecodeULRlcControlBlockAccessBurst
**
**    PURPOSE: This function decodes an uplink RLC control message that was
**      sent as access bursts rather than normal bursts.  After
**      determining that the message is valid the appropriate processing
**      is initiated.
**
**    INPUT PARAMETERS: l1PacchAbInd - MAC/RLC message header
**
**    RETURN VALUE(S): JC_SUCCESS - if decoding succeeds
**                     JC_ERROR - for decode failure
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::DecodeULRlcControlBlockAccessBurst(MsgPacchAbInd *l1PacchAbInd)
{
    DBG_FUNC("RlcMacTask::DecodeULRlcControlBlockAccessBurst", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;

    RlcMacMsgType msgType;
    
    // A bitstream will be created from the PACCH AB INDICATION structure
    BitStreamIn l1MsgBitsIn((unsigned char *)l1PacchAbInd->data, 2);
    
    if (msgType.DecodeMsgType(l1MsgBitsIn) == RLC_MAC_SUCCESS)
    {
        switch (msgType.msgType)
        {
            case 0x1d:  // NOTE!!!! SAGEM is sending this for the IMMEDIATE ASSIGNMENT case!!!!
            case RlcMacMsgType::PACKET_CONTROL_ACK_8_BIT:
            {
                MsgPacketControlAcknowledgement ctrlMsg((RlcMacMsgType::RLC_MAC_UPLINK_MSGS)msgType.msgType);
                if (ctrlMsg.DecodeMsg(l1MsgBitsIn) == RLC_MAC_SUCCESS)
                {
                    // If enabled display the short or detailed version of
                    // the message.
                    ctrlMsg.DisplayMsgShort(&l1MsgBitsIn);
                    ctrlMsg.DisplayMsgDetail();
                    ULTbf *pUlTbf;
                    DLTbf *pDlTbf;
                    if ((pUlTbf = tbfPool.FindULTbf((unsigned char)l1PacchAbInd->tag)))
                    {
                        pUlTbf->rlcLink.ReceivePacketControlAck(&ctrlMsg);
                    }
                    else if ((pDlTbf = tbfPool.FindDLTbf((unsigned char)l1PacchAbInd->tag)))
                    {
                        pDlTbf->rlcLink.ReceivePacketControlAck(&ctrlMsg, (unsigned char)l1PacchAbInd->ta);
                    }
                    else
                    {    
                        DBG_WARNING("RlcMacTask::DecodeULRlcControlBlockAccessBurst: Unkown control ack tag %d\n",
                            l1PacchAbInd->tag);
                        status = JC_ERROR;
                    }
                }
                else
                {
                    DBG_WARNING("RlcMacTask::DecodeULRlcControlBlockAccessBurst: Packet Control Ack decode failed\n");
                    status = JC_ERROR;
                }  
            }
            break;
            default:
            {
                DBG_WARNING("RlcMacTask::DecodeULRlcControlBlockAccessBurst: Unknown Control MsgType %#x\n",
                    msgType.msgType);
                status = RLC_MAC_ERROR;
            }
            break;
        }     
    }
    else
    {
        DBG_WARNING("RlcMacTask::DecodeULRlcControlBlockAccessBurst: Unable to decode MsgType\n");
        status = JC_ERROR;
    }

    DBG_LEAVE();
    return(status);
 }

