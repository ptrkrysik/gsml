
// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacIntf.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "Rlc_mac/RlcMacIntf.h"
#include "GP10MsgTypes.h"
#include "GP10Err.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"
#include "dsp\dsphmsg.h"


// Main message queue for the RLC/MAC task
JCMsgQueue *RlcMacMsgQ;


//----------------------------------------------------------------------------
// RLC/MAC Interface Functions
//----------------------------------------------------------------------------
JC_STATUS RlcMacGrrReadyToGo(void)
{
    DBG_FUNC("RlcMacGrrReadyToGo", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_READY_TO_GO,
                        MODULE_RM,
                        (char *)0,
                        0,
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacGrrReadyToGo: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacConfigureTS(int trx, int timeslot, int chanComb)
{
    DBG_FUNC("RlcMacTSConfigure", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    RlcMacTSConfiguration configMsg;
    
    configMsg.trx       = trx;
    configMsg.timeslot  = timeslot;
    configMsg.chanComb  = chanComb;
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_TIMESLOT_CONFIG,
                        MODULE_RM,
                        (char *)&configMsg,
                        sizeof(RlcMacTSConfiguration),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacTSConfigure: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacTSAdminState(int trx, int timeSlot, AdministrativeState state)
{
    DBG_FUNC("RlcMacTSAdminState", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    RlcMacAdminStateChangeType adminChangeMsg;
    
    adminChangeMsg.adminState = state;
    adminChangeMsg.oamObj     = RLC_MAC_TIMESLOT;
    adminChangeMsg.trx        = trx;
    adminChangeMsg.timeSlot   = timeSlot;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_ADMIN_STATE_CHANGE,
                        MODULE_RM,
                        (char *)&adminChangeMsg,
                        sizeof(RlcMacAdminStateChangeType),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacTSAdminState: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS RlcMacTRXAdminState(int trx, AdministrativeState state)
{
    DBG_FUNC("RlcMacTRXAdminState", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    RlcMacAdminStateChangeType adminChangeMsg;
    
    adminChangeMsg.adminState = state;
    adminChangeMsg.oamObj     = RLC_MAC_TRX;
    adminChangeMsg.trx        = trx;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_ADMIN_STATE_CHANGE,
                        MODULE_RM,
                        (char *)&adminChangeMsg,
                        sizeof(RlcMacAdminStateChangeType),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacTRXAdminState: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS RlcMacGPAdminState(AdministrativeState state)
{
    DBG_FUNC("RlcMacGPAdminState", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    RlcMacAdminStateChangeType adminChangeMsg;
    
    adminChangeMsg.adminState = state;
    adminChangeMsg.oamObj     = RLC_MAC_GP;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_ADMIN_STATE_CHANGE,
                        MODULE_RM,
                        (char *)&adminChangeMsg,
                        sizeof(RlcMacAdminStateChangeType),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacGPAdminState: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacTSOpState(int trx, int timeSlot, EnableDisable state)
{
    DBG_FUNC("RlcMacTSOpState", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    RlcMacOpStateChangeType opChangeMsg;
    
    opChangeMsg.opState    = state;
    opChangeMsg.oamObj     = RLC_MAC_TIMESLOT;
    opChangeMsg.trx        = trx;
    opChangeMsg.timeSlot   = timeSlot;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_OP_STATE_CHANGE,
                        MODULE_RM,
                        (char *)&opChangeMsg,
                        sizeof(RlcMacOpStateChangeType),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacTSOpState: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS RlcMacTRXOpState(int trx, EnableDisable state)
{
    DBG_FUNC("RlcMacTRXOpState", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    RlcMacOpStateChangeType opChangeMsg;
    
    opChangeMsg.opState    = state;
    opChangeMsg.oamObj     = RLC_MAC_TRX;
    opChangeMsg.trx        = trx;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_OP_STATE_CHANGE,
                        MODULE_RM,
                        (char *)&opChangeMsg,
                        sizeof(RlcMacOpStateChangeType),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacTRXOpState: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}



JC_STATUS RlcMacGPOpState(EnableDisable state)
{
    DBG_FUNC("RlcMacGPOpState", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    RlcMacOpStateChangeType opChangeMsg;
    
    opChangeMsg.opState    = state;
    opChangeMsg.oamObj     = RLC_MAC_GP;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_OP_STATE_CHANGE,
                        MODULE_RM,
                        (char *)&opChangeMsg,
                        sizeof(RlcMacOpStateChangeType),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacGPOpState: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}

JC_STATUS RlcMacRmMdMsgInd(T_CNI_RIL3MD_RRM_MSG *chanReq)
{
    DBG_FUNC("RlcMacRmMdMsgInd", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    // Send message the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_RM_MD_MSG_IND,
                        MODULE_RM,
                        (char *)chanReq,
                        sizeof(T_CNI_RIL3MD_RRM_MSG),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacRmMdMsgInd: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}



JC_STATUS RlcMacL1MsgInd(unsigned char *msg, int len)
{
    DBG_FUNC("RlcMacL1MsgInd", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    RlcMacL1MsgIndType rlcMacMsg;
    
    // Populate Layer 1 Message Request structure.
    rlcMacMsg.len = len;
    for (int i=0; i < len; i++)
    {
        rlcMacMsg.l1Msg[i] = msg[i];
    }
    
    if ((msg[1] == ((unsigned char)(PH_READY2SEND_IND>>8))) && 
        (msg[2] == ((unsigned char)PH_READY2SEND_IND)))
    {
        // Send message to the main RLC/MAC message queue.
        if ((status = RlcMacMsgQ->JCMsgQSend(
                            (JCMsgQueue *)0,              // No reply needed 
                            RLCMAC_L1_RTS,
                            MODULE_L1,
                            (char *)&rlcMacMsg,
                            sizeof(RlcMacL1MsgIndType),
                            JC_NO_WAIT,
                            JC_MSG_PRI_URGENT)) != JC_OK)
        {
            DBG_ERROR("RlcMacL1MsgInd: Unable to send message status(%x)\n",
                status);
        }
    }
    else
    {
        // Send message to the main RLC/MAC message queue.
        if ((status = RlcMacMsgQ->JCMsgQSend(
                            (JCMsgQueue *)0,              // No reply needed 
                            RLCMAC_L1_MSG_IND,
                            MODULE_L1,
                            (char *)&rlcMacMsg,
                            sizeof(RlcMacL1MsgIndType),
                            JC_NO_WAIT,
                            JC_MSG_PRI_NORMAL)) != JC_OK)
        {
            DBG_ERROR("RlcMacL1MsgInd: Unable to send message status(%x)\n",
                status);
        }
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacBssgpUnitDataRequest(BSSGP_DL_UNITDATA_MSG *msg)
{
    DBG_FUNC("RlcMacBssgpUnitDataRequest", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_DL_PDU_REQ,
                        MODULE_RM, // Needs to be BSSGP when its added.
                        (char *)msg,
                        sizeof(BSSGP_DL_UNITDATA_MSG),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacBssgpUnitDataRequest: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacT3169Expiry(int tbfPointer)
{
    DBG_FUNC("RlcMacT3169Expiry", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    char msg[sizeof(int)];
    
    for (int i = 0; i < sizeof(int); i++)
    {
        msg[i] = tbfPointer >> (i * 8);
    }
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_T3169_EXPIRY,
                        MODULE_RLCMAC,
                        msg,
                        sizeof(int),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacT3169Expiry: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacT3195Expiry(int tbfPointer)
{
    DBG_FUNC("RlcMacT3195Expiry", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    char msg[sizeof(int)];
    
    for (int i = 0; i < sizeof(int); i++)
    {
        msg[i] = tbfPointer >> (i * 8);
    }
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_T3195_EXPIRY,
                        MODULE_RLCMAC,
                        msg,
                        sizeof(int),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacT3195Expiry: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacT3191Expiry(int tbfPointer)
{
    DBG_FUNC("RlcMacT3191Expiry", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    char msg[sizeof(int)];
    
    for (int i = 0; i < sizeof(int); i++)
    {
        msg[i] = tbfPointer >> (i * 8);
    }
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_T3191_EXPIRY,
                        MODULE_RLCMAC,
                        msg,
                        sizeof(int),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacT3191Expiry: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacT3193Expiry(int tbfPointer)
{
    DBG_FUNC("RlcMacT3193Expiry", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    char msg[sizeof(int)];
    
    for (int i = 0; i < sizeof(int); i++)
    {
        msg[i] = tbfPointer >> (i * 8);
    }
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_T3193_EXPIRY,
                        MODULE_RLCMAC,
                        msg,
                        sizeof(int),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacT3193Expiry: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacDLActivityTimerExpiry(int tbfPointer)
{
    DBG_FUNC("RlcMacDLActivityTimerExpiry", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    char msg[sizeof(int)];
    
    for (int i = 0; i < sizeof(int); i++)
    {
        msg[i] = tbfPointer >> (i * 8);
    }
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_DL_ACTIVITY_TIMER_EXPIRY,
                        MODULE_RLCMAC,
                        msg,
                        sizeof(int),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacDLActivityTimerExpiry: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}


JC_STATUS RlcMacULActivityTimerExpiry(int tbfPointer)
{
    DBG_FUNC("RlcMacULActivityTimerExpiry", RLC_MAC_INTF);
    DBG_ENTER();
    
    JC_STATUS status;
    
    char msg[sizeof(int)];
    
    for (int i = 0; i < sizeof(int); i++)
    {
        msg[i] = tbfPointer >> (i * 8);
    }
    
    // Send message to the main RLC/MAC message queue.
    if ((status = RlcMacMsgQ->JCMsgQSend(
                        (JCMsgQueue *)0,              // No reply needed 
                        RLCMAC_UL_ACTIVITY_TIMER_EXPIRY,
                        MODULE_RLCMAC,
                        msg,
                        sizeof(int),
                        JC_NO_WAIT,
                        JC_MSG_PRI_NORMAL)) != JC_OK)
    {
        DBG_ERROR("RlcMacULActivityTimerExpiry: Unable to send message status(%x)\n",
            status);
    }
    
    DBG_LEAVE();
    return (status);
}
