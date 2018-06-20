// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacOamProc.cpp
// Author(s)   : Tim Olson
// Create Date : 4/11/2001
// Description : 
//
// *******************************************************************


#include "RlcMacTask.h"
#include "Rlc_Mac/RlcMacIntf.h"



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DecodeOamAdminMsgInd
**
**    PURPOSE: The Oam message indication is decoded.  If decoding is
**      successful the appropriate action is taken.
**
**    INPUT PARAMETERS: msgReq - oam message indication structure
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::DecodeOamAdminMsgInd(RlcMacAdminStateChangeType *msgReq)
{
    DBG_FUNC("RlcMacTask::DecodeOamMsgInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    switch (msgReq->oamObj)
    {
        case RLC_MAC_TIMESLOT:
        {
            switch (msgReq->adminState)
            {
                case unlocked:
                {
                    DBG_TRACE("RLC/MAC: Received Unlock Req - TRX(%d) TS(%d)\n",
                        msgReq->trx, msgReq->timeSlot);
                    // See if any downlink data can be sent now.
                    CheckDLMsgQueues();
                }    
                break;
                case locked:
                {
                    DBG_TRACE("RLC/MAC: Received Lock Req - TRX(%d) TS(%d)\n",
                        msgReq->trx, msgReq->timeSlot);
                    // Drop all current TBFs on specified timeslot
                    tbfPool.AbnormalReleaseTs(msgReq->trx, msgReq->timeSlot);
                }    
                break;
                case shuttingDown:
                {
                    DBG_TRACE("RLC/MAC: Received Shutdown Req - TRX(%d) TS(%d)\n",
                        msgReq->trx, msgReq->timeSlot);
                    // Don't need to do anything.        
                }    
                break; 
            }
        }
        break;   
        case RLC_MAC_TRX:
        {
            switch (msgReq->adminState)
            {
                case unlocked:
                {
                    DBG_TRACE("RLC/MAC: Received Unlock Req - TRX(%d)\n",
                        msgReq->trx);
                    // See if any downlink data can be sent now.
                    CheckDLMsgQueues();
                }    
                break;
                case locked:
                {
                    DBG_TRACE("RLC/MAC: Received Lock Req - TRX(%d)\n",
                        msgReq->trx);
                    // Drop all current TBFs on specified trx
                    tbfPool.AbnormalReleaseTrx(msgReq->trx);
                }    
                break;
                case shuttingDown:
                {
                    DBG_TRACE("RLC/MAC: Received Shutdown Req - TRX(%d)\n",
                        msgReq->trx);
                    // Don't need to do anything.        
                }    
                break; 
            }
        }   
        break;         
        case RLC_MAC_GP:   
        {
            switch (msgReq->adminState)
            {
                case unlocked:
                {
                    DBG_TRACE("RLC/MAC: Received GP Unlock Req\n");
                    // See if any downlink data can be sent now.
                    CheckDLMsgQueues();
                }    
                break;
                case locked:
                {
                    DBG_TRACE("RLC/MAC: Received GP Lock Req\n");
                    // Drop all current TBFs
                    tbfPool.AbnormalReleaseBts();
                }    
                break;
                case shuttingDown:
                {
                    DBG_TRACE("RLC/MAC: Received GP Shutdown Req\n");
                    // Don't need to do anything.
                }    
                break; 
            }
        }            
        break;         
        default:
        {
            DBG_WARNING("RlcMacTask::DecodeOamMsgInd: Invalid oam obj %#x\n",
                msgReq->oamObj);
            return (JC_ERROR);
        }            
    }
    
    DBG_LEAVE();
    return(status);
}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::DecodeOamOpMsgInd
**
**    PURPOSE: The Oam message indication is decoded.  If decoding is
**      successful the appropriate action is taken.
**
**    INPUT PARAMETERS: msgReq - oam message indication structure
**
**    RETURN VALUE(S): JC_SUCCESS
**                     JC_ERROR
**
**----------------------------------------------------------------------------*/
JC_STATUS RlcMacTask::DecodeOamOpMsgInd(RlcMacOpStateChangeType *msgReq)
{
    DBG_FUNC("RlcMacTask::DecodeOamOpMsgInd", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status = JC_OK;
    
    switch (msgReq->oamObj)
    {
        case RLC_MAC_TIMESLOT:
        {
            switch (msgReq->opState)
            {
                case opStateEnabled:
                {
                    DBG_TRACE("RLC/MAC: Received Op state enable Req - TRX(%d) TS(%d)\n",
                        msgReq->trx, msgReq->timeSlot);
                    // See if any downlink data can be sent now.
                    CheckDLMsgQueues();
                }    
                break;
                case opStateDisabled:
                {
                    DBG_TRACE("RLC/MAC: Received Op state disable Req - TRX(%d) TS(%d)\n",
                        msgReq->trx, msgReq->timeSlot);
                    // Drop all current TBFs on specified timeslot
                    tbfPool.AbnormalReleaseTs(msgReq->trx, msgReq->timeSlot);
                }    
                break;
            }
        }
        break;   
        case RLC_MAC_TRX:
        {
            switch (msgReq->opState)
            {
                case opStateEnabled:
                {
                    DBG_TRACE("RLC/MAC: Received Op state enable Req - TRX(%d)\n",
                        msgReq->trx);
                    // See if any downlink data can be sent now.
                    CheckDLMsgQueues();
                }    
                break;
                case opStateDisabled:
                {
                    DBG_TRACE("RLC/MAC: Received Op state disable Req - TRX(%d)\n",
                        msgReq->trx);
                    // Drop all current TBFs on specified trx
                    tbfPool.AbnormalReleaseTrx(msgReq->trx);
                }    
                break;
            }
        }   
        break;         
        case RLC_MAC_GP:   
        {
            switch (msgReq->opState)
            {
                case opStateEnabled:
                {
                    DBG_TRACE("RLC/MAC: Received GP Op state disable Req\n");
                    // See if any downlink data can be sent now.
                    CheckDLMsgQueues();
                }    
                break;
                case opStateDisabled:
                {
                    DBG_TRACE("RLC/MAC: Received GP Op state enable Req\n");
                    // Drop all current TBFs
                    tbfPool.AbnormalReleaseBts();
                }    
                break;
            }
        }            
        break;         
        default:
        {
            DBG_WARNING("RlcMacTask::DecodeOamMsgInd: Invalid oam obj %#x\n",
                msgReq->oamObj);
            return (JC_ERROR);
        }            
    }
    
    DBG_LEAVE();
    return(status);
}







