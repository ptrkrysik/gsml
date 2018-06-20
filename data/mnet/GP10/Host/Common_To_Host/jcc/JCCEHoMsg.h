#ifndef JCCEHoMsg_H
#define JCCEHoMsg_H

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
// File        : JCCEHoMsg.h
// Author(s)   : Xiaode, Tai and Joe
// Create Date : 05-15-01
// Description : External handover message and interface among
//               applications
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "MnetModuleId.h"

// Instead of including JcVcToVc.h, just define the types that
// are needed in this module.  For the long term, must solve
// this cyclical include problem
//#include "JcVcToVc.h"
// ---------------------------------------------------------------
typedef MNET_MODULE_ID             JcModuleId_t;
typedef short                      JcSubId_t;
// ---------------------------------------------------------------

#include "voip/exchangeho.h"
#include "ril3/ril3md.h"
#include "ril3/ie_called_bcd.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

// *******************************************************************
// Type Definitions
// *******************************************************************

// Perform Handover Request
typedef struct {
   GlobalCellId_t                         globalCellID;
   USHORT                                 hoCause;
   bool                                   hoNumberReqd;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  handoverNumber;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  mscNumber;
   A_INTERFACE_MSG_STRUCT_t               A_HANDOVER_REQUEST;
} InterHoaVcPerformHandover_t;

// End Handover
typedef struct {
   USHORT             reason;
} InterHoaVcEndHandover_t;

// Post Handover Mob Event
typedef struct {
   LAYER3PDU_STRUCT_t LAYER3PDU;
} InterHoaVcPostHoMobEventData_t;

// Perform Handover Ack
typedef struct {
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  handoverNumber;
   A_INTERFACE_MSG_STRUCT_t               A_INTERFACE_MSG;
   USHORT                                 reason;
} InterHoaVcPerformHandoverAck_t;

// Perform Handover Access Message
typedef struct
{
   A_INTERFACE_MSG_STRUCT_t  A_INTERFACE_MSG;
} InterHoaVcHandoverAccess_t;

// Perform Handover Success Message
typedef struct
{
   A_INTERFACE_MSG_STRUCT_t  A_INTERFACE_MSG;
} InterHoaVcHandoverSuccess_t;

// Perform Handback Request Message
typedef struct
{
   GlobalCellId_t                         globalCellID;
   USHORT                                 hoCause;
   bool                                   externalCell;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  mscNumber;
   A_INTERFACE_MSG_STRUCT_t               A_HANDOVER_REQUEST;
} InterHoaVcPerformHandback_t;

// Perform Handback Ack Message
typedef struct
{
   A_INTERFACE_MSG_STRUCT_t  A_INTERFACE_MSG;
   USHORT                    reason;
} InterHoaVcPerformHandbackAck_t;

// Release HOA Message
typedef struct
{
} InterHoaVcReleaseHOA_t;


// Message Type for External Handover Messages
typedef enum {

  INTER_EHO_NULL_MSG                   = 0,
  INTER_EHO_BASE                       = INTER_EHO_NULL_MSG,

  INTER_EHO_PERFORM_HANDOVER_REQUEST,
  INTER_EHO_PERFORM_HANDOVER_ACK,
  INTER_EHO_END_HANDOVER,
  INTER_EHO_HANDOVER_ACCESS,
  INTER_EHO_HANDOVER_SUCCESS,
  INTER_EHO_HANDOVER_MOB_EVENT,
  INTER_EHO_PERFORM_HANDBACK_REQUEST,
  INTER_EHO_PERFORM_HANDBACK_ACK,
  INTER_EHO_COMPLETE_HANDBACK       //ext-HO <xxu:06-08-01>
} InterEHOMsgType_t;


union InterEHOMsgData_t
{
  InterHoaVcPerformHandover_t        perfExtHandoverReq;
  InterHoaVcPerformHandoverAck_t     perfExtHandoverAck;
  InterHoaVcEndHandover_t            extEndHandover;
  InterHoaVcHandoverAccess_t         extHandoverAccess;
  InterHoaVcHandoverSuccess_t        extHandoverSuccess;
  InterHoaVcReleaseHOA_t             extReleaseHOA;
  InterHoaVcPostHoMobEventData_t     postExtHoMobEventData;

  InterHoaVcPerformHandback_t        perfExtHandbackReq;
  InterHoaVcPerformHandbackAck_t     perfExtHandbackAck;
} ;


// Message format between CC and HOA
typedef struct {
  T_CNI_MODULE_ID            module_id;      // origin of a message 

  JcSubId_t                  origSubId;      // optional origination Sub Id
  JcSubId_t                  destSubId;      // optional destination Sub Id

  InterEHOMsgType_t          msgType;
  InterEHOMsgData_t          msgData;

} InterHoaVcMsg_t;

#endif                                       // JCCEHoMsg_H
