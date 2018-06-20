#ifndef JCCState_H
#define JCCState_H

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
// File        : JCCState.h
// Author(s)   : Bhava Nelakanti
// Create Date : 10-19-98
// Description : interface specification for -  JCCState
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// *******************************************************************
// forward declarations.
// *******************************************************************

//BCT <xxu:09-22-00>
typedef enum 
{
   ANN_PLAY_BASE = 0,
   ANN_PLAY_OFF,
   ANN_PLAY_ON,
   ANN_PLAY_WAITING
 
} AnnStates_t;

// CH<xxu:11-10-99> aux state definitions for Call Hold
typedef enum 
{
  CH_ST_BASE  = 0,

  CH_ST_IDLE, 
  CH_ST_HREQ,
  CH_ST_HELD,
  CH_ST_RREQ

} ChStates_t;

// State definitions for Channel
typedef enum 
{
  RES_ST_BASE                        = 0,

  RES_ST_IDLE,
  RES_ST_ASS_REQ,
  RES_ST_ASSIGNED,
  RES_ST_REL_REQ

} ResStates_t;

// HO<xxu:01-26-00> handover types definition
typedef enum
{  
   //0 means invalid state !

   HO_TY_BASE=1,
   HO_TY_HANDOVER = HO_TY_BASE,
   HO_TY_HANDBACK,
   HO_TY_HANDMSC3,

  // ext-HO <chenj:07-01-2001>
  // Handover Types for external handover
   HO_TY_EXT_HANDOVER,
   HO_TY_EXT_HANDBACK,
   HO_TY_EXT_HANDMSC3,
   HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR,
   HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3

} HandoverTypes_t;

// State definitions for Channel
typedef enum 
{
  HAND_ST_BASE                        = 0,

  HAND_ST_IDLE                        = HAND_ST_BASE,

  HAND_ST_ANCH_VC_IP_ADDRESS_REQ,
  HAND_ST_ANCH_SND_HO_REQ,
  HAND_ST_ANCH_RCV_HO_REQ_ACK,
  HAND_ST_ANCH_HO_COMPLT,

  HAND_ST_ANCH_RCV_HB_REQ,
  HAND_ST_ANCH_RCV_HB_REQ_ACK,			  //ext-HO <xxu:06-30-01>
  HAND_ST_ANCH_SND_HB_REQ,                //ext-HO <xxu:06-30-01>
  HAND_ST_ANCH_SND_HB_REQ_ACK,

  //HO<xxu:01-27-00> states for H3
  HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ,	  //After send VC IP Address Req to VB
  HAND_ST_ANCH_SND_H3_REQ,                //Recv VC IP Address Rsp, then send Perform Handover to MSC-B'
  HAND_ST_ANCH_RCV_H3_REQ_ACK,            //Recv PERFORM_HANDOVER_ACK from MSC-B' thus sending a PERFORM
                                          //_HANDBACK_ACK to MSC-B
  HAND_ST_TRG_RCV_HO_REQ,
  HAND_ST_TRG_SND_HO_REQ_ACK,
  HAND_ST_TRG_HO_COMPLT,

  HAND_ST_TRG_SND_HB_REQ,
  HAND_ST_TRG_RCV_HB_REQ_ACK,

  // ext-HO <chenj: 05-29-01>
  HAND_ST_ANCH_PERF_EHO_REQ,
  HAND_ST_ANCH_RCV_EHO_REQ_ACK,
  HAND_ST_ANCH_RCV_EHO_REQ_NACK,
  HAND_ST_ANCH_RCV_EHO_HB_REQ,
  HAND_ST_ANCH_SND_EHB_REQ_ACK,
  HAND_ST_ANCH_EHO_H3_VC_IP_ADDRESS_REQ,
  HAND_ST_ANCH_EHO_SND_H3_REQ,
  HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK,
  HAND_ST_ANCH_PERF_EHO_H3_REQ,
  HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK,
  HAND_ST_ANCH_RCV_EHO_H3_REQ_NACK,
  HAND_ST_ANCH_EHO_COMPLT,   // Make sure HAND_ST_ANCH_EHO_COMPLT is the LAST state!  (affects MAX_HO_SRC_STATES)

} HandoverStates_t;

// State definitions for CC Half Call State Machine
typedef enum 
{
  HC_ST_BASE                     = 0,

  HC_ST_FREE,
  HC_ST_NULL,
  HC_ST_ACTIVE,
  HC_ST_RELEASE_REQ,
  HC_ST_DISCONNECT_IND,

  // 
  // State definitions for CC Orig Half Call State Machine
  // OrigHalfCallStates
  OHC_ST_BASE                     = HC_ST_DISCONNECT_IND,

  OHC_ST_AUTH_CALL_SETUP,
  OHC_ST_OFFERING_CALL,
  OHC_ST_CALL_PROCEEDING,
  OHC_ST_ANSWER_PENDING,

  // Mobile only
  OHC_ST_CONNECT_INDICATION,   

  // State definitions for CC Term Half Call State Machine
  // TermHalfCallStates
  THC_ST_BASE                     = OHC_ST_CONNECT_INDICATION,

  THC_ST_AUTH_TERMINATION ,
  THC_ST_PRESENTING_CALL,
  THC_ST_ALERTING,

  // Mobile only
  THC_ST_CALL_CONFIRMED,
  THC_ST_MM_CONN_REQ,

  // ext-HO <chenj:05-15-2001>
  // Call states for external handover
  EHO_ST_FREE,
  EHO_ST_BASE,
  EHO_ST_NULL,
  EHO_ST_OFFERING_CALL,
  EHO_ST_CONFIRMED,
  EHO_ST_ACTIVE,

} HalfCallStates_t;


typedef enum 
{
    SMS_STATE_IDLE          = 0,
    SMS_STATE_MT_CONNECT_MM = 1,
    SMS_STATE_CONNECTED     = 2,
    SMS_STATE_WAIT_CP_ACK   = 3,
    SMS_STATE_DISCONNECT    = 4,

    SMS_STATE_MAX              ,
    SMS_STATE_NONE // This is not a valid state. Used as "same state" indicator

} SmsState_t;


#endif                        // JCCState_H

