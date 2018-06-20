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
#include "Rlc_mac/RlcMacIntf.h"


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ProcessRmMdMsgInd
**
**    PURPOSE: This function decodes the MD message primitive type and
**      initiates the appropriate action.
**
**    INPUT PARAMETERS: msgReq - MD message request
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ProcessRmMdMsgInd(T_CNI_RIL3MD_RRM_MSG *msgReq)
{
    DBG_FUNC("RlcMacTask::ProcessRmMdMsgInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    
    
    switch (msgReq->primitive_type)
    {
        case L23PT_DL_RANDOM_ACCESS_IND :
        {
            status = ProcessChanReq(msgReq);
        }
        break;
        default :
        {
            DBG_WARNING("RlcMacTask::ProcessRmMdMsgInd: Unkown primitive type %#x\n",
                msgReq->primitive_type);
            status = JC_INVALID_MSG;        
        } 
    }
    
    DBG_LEAVE();
    return (status);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::ProcessChanReq
**
**    PURPOSE: A CHANNEL REQUEST was received on the RACH channel.  This
**      request was forwarded from RM and handled here.  The layer 1 message
**      will be decoded.  If decoded successfully then a single block allocation
**      will be granted.
**
**      For now no one phase access requests will be granted.
**
**    INPUT PARAMETERS: msgReq - layer 1 message request
**
**    RETURN VALUE(S): JC_OK
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::ProcessChanReq(T_CNI_RIL3MD_RRM_MSG *msgReq)
{
    DBG_FUNC("RlcMacTask::ProcessChanReq", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST chanReq;

    // Use RIL3 to decode the channel request message.
    chanReq.neci = CNI_RIL3_NECI_NOT_SUPPORTED;  // Needed by Ril3
    T_CNI_RIL3_RESULT ril3Result = CNI_RIL3RRM_DecodeChannelRequest (
                         msgReq->l3_data.buffer,
                         &chanReq );
         
    // The message was decoded properly.  Handle the two possible data
    // establishment causes:
    //    1. One phase packet access
    //    2. Single block packet access             
    if (ril3Result == CNI_RIL3_RESULT_SUCCESS)      
    {
        DBG_TRACE("RLC/MAC: Received CHANNEL REQUEST on CCCH - estCause(%#x)\n",
            chanReq.estCause);
        switch (chanReq.estCause)
        {
            // For now only a two-phase access is allowed.  Therefore only a 
            // single block allocation will be granted here.
            case CNI_RIL3_EST_CALL_ONE_PHASE_PACKET_ACCESS :
            case CNI_RIL3_EST_CALL_SINGLE_BLOCK_PACKET_ACCESS :
            {
                status = ULSingleBlockAllocation(&chanReq, msgReq);
            }
            break;
            default :
                DBG_WARNING("RlcMacTask::ProcessChanReq: Unkown establishment cause %#x\n",
                    chanReq.estCause);
            status = JC_INVALID_MSG;        
        }
    }
    else
    {
        DBG_WARNING("RlcMacTask::ProcessChanReq: RIL3 decode failed\n");
        status = JC_INVALID_MSG;        
    }
    
    DBG_LEAVE();
    return (status);
}














