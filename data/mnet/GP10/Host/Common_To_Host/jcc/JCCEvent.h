#ifndef JCCEvent_H
#define JCCEvent_H

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
// File        : JCCEvent.h
// Author(s)   : Bhava Nelakanti
// Create Date : 10-19-98
// Description : The Event space in the ViperCell
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************


// *******************************************************************
// forward declarations.
// *******************************************************************


typedef enum 
{
  // Internal common event definitions
  // JCCInternalEvents

  JCC_NULL_EVENT                 = 0,

  // Internal event definitions for Call Control
  // CCInternalEvents
  CC_INTERNAL_EVENT_BASE         = 30,

  CC_RELEASE_CALL,
  
  CC_RELEASE_OWN,           //ext-HO <xxu:06-08-01>
  CC_RELEASE_GP_HO,			//ext-HO <xxu:06-30-01>
  CC_RELEASE_GP_HB,			//ext-HO <xxu:06-30-01>
  CC_RELEASE_GP_H3,			//ext-HO <xxu:06-30-01>
  CC_RELEASE_EH_HB,			//ext-HO <xxu:06-30-01>
  CC_RELEASE_GP_EH_HB,		//ext-HO <xxu:06-30-01>

  CC_RELEASE_HOCALLLEG,     //ext-HO <chenj:06-11-01>
  CC_RELEASE_H3CALLLEG,     //ext-HO <chenj:06-11-01>
  CC_RELEASE_ALLHOCALLLEG,  //ext-HO <chenj:06-11-01>

  CC_MESSAGE_PROCESSING_COMPLT,

  CC_INTERNAL_CHANN_EVENT_BASE   = 50,

  CC_RESOURCE_ASSIGN_COMPLT,
  CC_RESOURCE_ASSIGN_FAIL,

  CC_INTERNAL_GEN_EVENT_BASE     = 60,

  CC_TWO_STAGE_DISCONNECT,
  CC_RESOURCE_ASSIGN_REQ,
  CC_RES_RELEASE,

  CC_MAKE_CONNECTION_REQ,
  CC_CONNECT_COMPLT,
  CC_CONNECT_FAIL,
  CC_BREAK_CONNECTION_REQ,
  CC_DISCONNECT_COMPLT,
  CC_DISCONNECT_FAIL,
  CC_APPLY_TONE,

  CC_INTERNAL_CALL_EVENT_BASE    = 90, //Igal: 22.03.00

  CC_MM_CONNECTION_REQ,         // 91
  CC_MM_CONNECTION_CNF,         // 92
  CC_MM_CONNECTION_REJ,         // 93
  CC_MM_CONNECTION_REL,         // 94
  CC_MM_CONNECTION_IND,         // 95 Igal: 22.03.00

  // Local event definitions for CC Half Call State Machine
  // HalfCallLocalEvents

  HC_LOCAL_EVENT_BASE            = 100,

  HC_LOCAL_DISCONNECT_REQ,
  HC_LOCAL_RELEASE_COMPLETE,
  HC_LOCAL_CALL_PROGRESS,
  HC_LOCAL_PROTOCOL_PROBLEM,
  HC_LOCAL_INVALID_MSG_ERR,
  HC_LOCAL_PROTOCOL_TIMEOUT,

  // Mobile only
  HC_LOCAL_RELEASE_REQ,
  HC_LOCAL_STATUS_INFO,
  HC_LOCAL_STATUS_ENQUIRY,

  HC_LOCAL_START_DTMF,
  HC_LOCAL_STOP_DTMF,

  //CH<xxu:11-10-99>
  HC_LOCAL_HOLD_ACT, 
  HC_LOCAL_HOLD_REJ,
  HC_LOCAL_RETV_ACT,
  HC_LOCAL_RETV_REJ,
  HC_LOCAL_HOLD_REQ,
  HC_LOCAL_RETV_REQ, //CH<xxu:Mod 12-13-99>
  HC_LOCAL_CHTR_EXP,
  HC_EXTNL_HOLD_REQ, //exception: handler given in sessionHandler
  HC_EXTNL_RETV_REQ, //exception: handler given in sessionHandler

  // Local event definitions for CC Term Half Call State Machine
  // TermHalfCallEvents

  //CH<xxu:11-24-99> 110->118
  //Status Msg <bn>  118->130
  THC_LOCAL_EVENT_BASE = 130,
  
  THC_LOCAL_ALERTING,
  THC_LOCAL_ANSWER,
 
  // Local event definitions for CC Orig Half Call State Machine
  // OrigHalfCallEvents

  //CH<xxu:11-24-99> 115->121
  //Status Msg <bn>  121->140
  OHC_LOCAL_EVENT_BASE  = 140,


  OHC_LOCAL_ORIGINATION,
  OHC_LOCAL_TERM_ADDRESS,
  OHC_LOCAL_EMERGENCY_ORIGINATION,

  // Mobile only
  OHC_LOCAL_CONNECT_ACK,

  // Remote event definitions for CC Half Call State Machine
  // HalfCallRemoteEvents

  //CH<xxu:11-24-99> 120->125
  //Status Msg <bn>  125->150
  HC_REMOTE_EVENT_BASE = 150,

  HC_REMOTE_DISCONNECT_REQ,
  HC_REMOTE_RELEASE_COMPLETE,
  HC_REMOTE_CALL_PROGRESS,
  HC_REMOTE_CONNECTION_CONTROL,
  HC_REMOTE_CHANN_SELECT,

  HC_REMOTE_DTMF_REQ,

  OHC_REMOTE_ALERTING,
  OHC_REMOTE_ANSWER,

  THC_REMOTE_TERM_ADDRESS,

  // BCT <xxu:07-07-00> BEGIN
  OHC_REMOTE_BCT_INVOKE,
  OHC_REMOTE_BCT_STATUS,
  HC_VB_GET_BCT_NUMBER_RSP,
  // BCT <xxu:07-07-00> END

  // BCT <xxu:09-21-00>
  HC_REMOTE_BCT_UPDATE_HANDLES,

  HC_REMOTE_CALL_PROGRESS_1,

  //Status Msg <bn>  150->170
  HAND_SRC_EVENT_BASE           = 170,  

  HC_VB_GET_VC_ADDRESS_REQ,
  HC_VB_GET_VC_ADDRESS_RSP,

  HAND_SRC_HO_REQD,
  HAND_SRC_HO_FAIL,

  HAND_SRC_TIMER_EXPIRY,

  HAND_PERFORM_HO_ACK,
  HAND_PERFORM_HO_NACK,
  HAND_COMPLT_HO,
  HAND_RLS_CALL,
  HAND_POSTHO_MOB_EVENT,
  HAND_POSTHO_HOA_EVENT,    //ext-HO <xxu:07-03-01>

  //Status Msg <bn>  180->200
  HAND_TRG_EVENT_BASE           = 200,

  HAND_TRG_HO_REQ_ACK,
  HAND_TRG_HO_REQ_NACK,
  HAND_TRG_HO_COMPLT,

  HAND_TRG_TIMER_EXPIRY,

  HAND_PERFORM_HO_REQ,
  HAND_ABORT_HO,

  // ext-HO <chenj:05-29-01>
  HAND_HO_ACCESS,
  HAND_END_HO,  // Make sure HAND_END_HO is the LAST event!  (affects MAX_HO_SRC_EVENTS)

  EXT_VC_FAIL,

  SMS_EVENT_BASE            =   300,
  SMS_EVENT_MD_ERROR_IND    , //301
  SMS_EVENT_CP_DATA         , //302
  SMS_EVENT_CP_ACK          , //303
  SMS_EVENT_CP_ERROR        , //304
  SMS_EVENT_MNSMS_EST_REQ   , //305
  SMS_EVENT_MNSMS_DATA_REQ  , //306
  SMS_EVENT_MNSMS_REL_REQ   , //307
  SMS_EVENT_MNSMS_ABORT_REQ , //308
  SMS_EVENT_TIMEOUT         , //309
  SMS_EVENT_PROTOCOL_ERROR  , //310

  SMS_EVENT_DEFAULT         , //311
  SMS_EVENT_MAX,               //312

  // ext-HO <chenj:05-29-01>
  CC_INTERNAL_EHO_CALL_EVENT_BASE   = 400,
  OHC_EHO_LOCAL_TERM_ADDRESS,
  OHC_EHO_REMOTE_CALL_PROGRESS,
  OHC_EHO_REMOTE_ANSWER,
  HC_EHO_REMOTE_CHANN_SELECT,
  
  //ext-HO <xxu:06-08-01>
  HC_EHO_REMOTE_CONNECTION_CONTROL,
  HC_EHO_LOCAL_CALL_PROGRESS,
  HC_EHO_LOCAL_DISCONNECT_REQ,
  HC_EHO_REMOTE_DISCONNECT_REQ,
  HC_EHO_LOCAL_PROTOCOL_TIMEOUT,

  THC_EHO_REMOTE_TERM_ADDRESS,
  THC_EHO_LOCAL_ANSWER,
  CC_EHO_LAST_EVENT,
  //ext-HO <xxu>

} JCCEvent_t;

#endif                                       // JCCEvent_H
