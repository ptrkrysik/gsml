// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacIntf.h
// Author(s)   : Tim Olson
// Create Date : 11/14/2000
// Description : 
//
// *******************************************************************
#ifndef __RLCMACINTF_H__
#define __RLCMACINTF_H__

#include "Os/JCMsgQueue.h"
#include "ril3/ril3md.h"
#include "rm/rml1_intf.h"
#include "bssgp/bssgp_api.h"
#include "vipermib.h"

extern JCMsgQueue *RlcMacMsgQ;


//----------------------------------------------------------------------------
// RLC/MAC Interface Functions
//----------------------------------------------------------------------------

// Configuration API
JC_STATUS RlcMacGrrReadyToGo(void);
JC_STATUS RlcMacConfigureTS(int trx, int timeslot, int chanComb);

// Admin state change API
JC_STATUS RlcMacTSAdminState(int trx, int timeSlot, AdministrativeState state);
JC_STATUS RlcMacTRXAdminState(int trx, AdministrativeState state);
JC_STATUS RlcMacGPAdminState(AdministrativeState state);

// Operational state change API
JC_STATUS RlcMacTSOpState(int trx, int timeSlot, EnableDisable state);
JC_STATUS RlcMacTRXOpState(int trx, EnableDisable state);
JC_STATUS RlcMacGPOpState(EnableDisable state);


// Msg processing API
JC_STATUS RlcMacL1MsgInd(unsigned char *msg, int len);
JC_STATUS RlcMacRmMdMsgInd(T_CNI_RIL3MD_RRM_MSG *msg);
JC_STATUS RlcMacBssgpUnitDataRequest(BSSGP_DL_UNITDATA_MSG *msg);

// Timers
JC_STATUS RlcMacT3169Expiry(int tbfPointer);
JC_STATUS RlcMacT3195Expiry(int tbfPointer);
JC_STATUS RlcMacT3191Expiry(int tbfPointer);
JC_STATUS RlcMacT3193Expiry(int tbfPointer);
JC_STATUS RlcMacULActivityTimerExpiry(int tbfPointer);
JC_STATUS RlcMacDLActivityTimerExpiry(int tbfPointer);

//----------------------------------------------------------------------------
// RLC/MAC Interface Structures
//
// All RLC/MAC messages are packed into these structures by the interface
// functions and sent to the RLC/MAC task.
//----------------------------------------------------------------------------


typedef enum
{
    RLC_MAC_TIMESLOT,
    RLC_MAC_TRX,
    RLC_MAC_GP
} RLC_MAC_OAM_OBJ;

struct RlcMacL1MsgIndType
{
    int             len;
    unsigned char   l1Msg[L1RM_MAXMSG_LEN];
};

struct RlcMacAdminStateChangeType
{
    AdministrativeState     adminState;
    RLC_MAC_OAM_OBJ         oamObj;
    int                     timeSlot;
    int                     trx;
};

struct RlcMacOpStateChangeType
{
    EnableDisable           opState;
    RLC_MAC_OAM_OBJ         oamObj;
    int                     timeSlot;
    int                     trx;
};

struct RlcMacTSConfiguration
{
    int         trx;
    int         timeslot;
    int         chanComb;
};

#endif