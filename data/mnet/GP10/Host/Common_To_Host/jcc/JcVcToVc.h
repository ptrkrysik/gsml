#ifndef JcVcToVc_H
#define JcVcToVc_H

// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : JCVcToVc.h
// Author(s)   : Bhava Nelakanti
// Create Date : 05-10-99
// Description : ViperCell to ViperCell Message Structures 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "MnetModuleId.h"

// include Handover Message definitions
#include "JCCHoMsg.h"

#include "JCCComMsg.h"

#include "cc/CCTypes.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

// Typedefs
//

typedef MNET_MODULE_ID             JcModuleId_t;
typedef short                      JcSubId_t;

// Message Type for Inter ViperCell Messages
typedef enum {

  INTER_VC_NULL_MSG                   = 0,

  // CC <-> CC Handover Messages -----------------------------------------

  INTER_VC_CC_BASE                    = INTER_VC_NULL_MSG,
  INTER_VC_CC_PERFORM_HANDOVER,
  INTER_VC_CC_PERFORM_HANDOVER_ACK,
  INTER_VC_CC_PERFORM_HANDOVER_NACK,
  INTER_VC_CC_ABORT_HANDOVER,
  INTER_VC_CC_COMPLETE_HANDOVER,

  INTER_VC_CC_RLS_CALL,
  INTER_VC_CC_POST_HANDOVER_DATA_EVENT,
  INTER_VC_CC_POST_HANDOVER_MOB_EVENT,

  INTER_VC_CC_PERFORM_HANDBACK,
  INTER_VC_CC_PERFORM_HANDBACK_ACK,
  INTER_VC_CC_PERFORM_HANDBACK_NACK,
  INTER_VC_CC_ABORT_HANDBACK,
  INTER_VC_CC_COMPLETE_HANDBACK,

  
} InterVcMsgType_t;

union InterVcPostHoMobEventData_t
{
  T_CNI_RIL3MD_CCMM_MSG      msInMsg;
  T_CNI_LAPDM_L3MessageUnit  msEncodedMsg;
  IntraL3Msg_t               intraL3Msg;  
};

union InterVcMsgData_t
{
  InterVcPerformHandover_t        perfHandoverReq;
  InterVcPerformHandoverAck_t     perfHandoverAck;
  InterVcPerformHandoverNack_t    perfHandoverNack;
  InterVcAbortHandover_t          abortHandover;
  InterVcCompleteHandover_t       compltHandover;
  InterVcReleaseCall_t            rlsCall;
  InterVcPostHoMobEventData_t     postHoMobEventData;

  InterVcPerformHandback_t        perfHandbackReq;
  InterVcPerformHandbackAck_t     perfHandbackAck;
  InterVcPerformHandbackNack_t    perfHandbackNack;
  InterVcAbortHandback_t          abortHandback;
  InterVcCompleteHandback_t       compltHandback;
} ;

#define VIPERCELL_MAGIC_VALUE		0x1234

// Message format between subsystems at two different ViperCells */

typedef struct {
  // Header

  JcVipercellId_t            origVcId;       // origination ViperCell Id 
  UINT32                     origVcAddress;  // origination ViperCell IP address
  JcModuleId_t               origModuleId;   // origination Module Id
  JcSubId_t                  origSubId;      // optional origination Sub Id

  JcVipercellId_t            destVcId;       // destination ViperCell Id 
  UINT32                     destVcAddress;  // destination ViperCell IP address
  JcModuleId_t               destModuleId;   // destination Module Id
  JcSubId_t                  destSubId;      // optional destination Sub Id

  InterVcMsgType_t           msgType;
  InterVcMsgData_t           msgData;

  int						 Magic;			// This should always be set to
											// VIPERCELL_MAGIC_VALUE for Msg integrity validation
} InterVcMsg_t;

typedef struct
{
  T_CNI_MODULE_ID            module_id;      // origin of a message 
  
  InterVcMsg_t               vcToVcMsg;
} VcCcMsg_t;

// The API call to send an inter-Vipercell message from any module
bool JcMsgSendToVipercell(InterVcMsg_t *msg);

// The API call for the CC Module to receive an inter-Vipercell message 
bool JcMsgRcvByCC(InterVcMsg_t *msg);

#endif                                       // JcVcToVc_H
