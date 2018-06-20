#ifndef JCCHoMsg_H
#define JCCHoMsg_H

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
// File        : JCCHoMsg.h
// Author(s)   : Bhava Nelakanti
// Create Date : 05-10-99
// Description : handover message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included MD and IRT headers for messages from MS
#include "JCCComIE.h"

// included MD and IRT headers for messages from MS
#include "lapdm/lapdm_l3intf.h"

// include H323 api file
#include "voip/voipapi.h"

// include ril3 cc types
#include "ril3/ril3_cc_msg.h"

#include "voip/exchangeho.h"
// *******************************************************************
// forward declarations.
// *******************************************************************
//<taiyu:05-18-01> Global Cell ID as defined in GSM 08.08 3.2.2.17

// *******************************************************************
// Constant Definitions
// *******************************************************************

const int MAX_CANDIDATE_CELLS         = 6;

// Typedefs
//

// Handover Request Cause
typedef enum {
  JC_FAILURE_BASE                         = 0,
  JC_FAILURE_RADIO_RESOURCE_UNAVAILABLE   = JC_FAILURE_BASE,
  JC_FAILURE_CIPHERING_ALG_UNSUPPORTED,

  JC_FAILURE_REVERT_TO_OLD,
  JC_FAILURE_THO_TIMEOUT,
  JC_FAILURE_T103_TIMEOUT,

  JC_FAILURE_T204_TIMEOUT,

  JC_FAILURE_TSHO_TIMEOUT,
  JC_FAILURE_T203_TIMEOUT,

  JC_FAILURE_T104_TIMEOUT,

  //HO<xxu:01-27-00>ho fail causes

  JC_FAILURE_ANCH_HO_DISALOWED,
  JC_FAILURE_ANCH_VB_LINK_PROBLEM,
  JC_FAILURE_ANCH_V3_FAILED,
  JC_FAILURE_CSU_SIMPLEX_CONNETION,
  JC_FAILURE_CALL_ALREADY_DROPPED,
  JC_FAILURE_NTGT_CALL_RLS,
  JC_FAILURE_OTGT_CALL_RLS,

  //ext-HO <xxu:06-26-01>
  JC_FAILURE_INPROPER_HANDOVER_STATE,
  JC_FAILURE_ANCH_HO_RTP_HANDLE_NULL,
  JC_FAILURE_ANCH_H3_RTP_HANDLE_NULL,
  JC_FAILURE_A_INTERFACE_MESSAGE_ENCODING_FAILED,
  JC_FAILURE_A_INTERFACE_MESSAGE_DECODING_FAILED,
  JC_FAILURE_VBLINK_HANDBACK_FAILED,
  JC_FAILURE_VBLINK_LOCATION_REQUEST_FAILED,
  JC_FAILURE_A_INTERFACE_MESSAGE_TYPE_INCOMPATIBLE,
  JC_FAILURE_A_HANDOVER_ACKNOWLEDGE_WITHOUT_L3_DATA,
  JC_FAILURE_TARGETn_PERFORM_HANDOVER_REQ_NACK,
  JC_FAILURE_T211_TIMEOUT,
  JC_FAILURE_TARGETo_HANDOVER_FAILURE,
  JC_FAILURE_IRT_TABLE_SET_FAILED,
  JC_FAILURE_IRT_TABLE_GET_FAILED,
  JC_FAILURE_CSU_UNI_CONNECT_FAILED,
  JC_FAILURE_A_HANDOVER_FAILURE,
  JC_FAILURE_HANDBACK_NOT_ALLOWED,
  JC_FAILURE_REMOTE_END_HANDOVER,

  //RETRY<xxu:05-01-00> BEGIN
  JC_FAILURE_TVB_TIMEOUT
  //RETRY<xxu:05-01-00> END

  // Others added in the future as needed

} JcFailureCause_t;

//CIPH<xxu:02-10-00> Begin

//RM <--> MM Ciphering Message
#define CIPHER_KC_LEN	8
typedef struct{
	T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMMAND cmd;
	unsigned char Kc[CIPHER_KC_LEN];
} IntraL3CipherModeCommand_t;

typedef struct{
	T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMPLETE cmp;
} IntraL3CipherModeComplete_t;

//CIPH<xxu:02-10-00> End


// Handover Request
typedef struct {
  L3RMChannelType_t          channelType;
  
  // Encryption Information to be added later

  // Mobile Information to be added when required by RRM
  IntraL3CipherModeCommand_t	cipherCmd;   //CIPH<xxu:02-10-00>

  union
  {
	// Identifies the Handover Request Transaction from MM to RM
	short                      mmId; 
	// Identifies the Handover Request Transaction from CC to MM
	MSG_Q_ID                   qid;
  };

} IntraL3HandoverRequest_t;

// Handover Request Ack
typedef struct {
  T_CNI_LAPDM_L3MessageUnit  handCmd;  // Complete Layer 3 RR Handover Command Message
  
  // Echo back the index value that was received with the request
  union
  {
	// Identifies the Handover Request Transaction from MM to RM
	short                      mmId; 
	// Identifies the Handover Request Transaction from CC to MM
	MSG_Q_ID                   qid;
  };

} IntraL3HandoverRequestAck_t;

// Handover Request Nack
typedef struct {
  JcFailureCause_t            cause;

  // Echo back the index value that was received with the request
  union
  {
	// Identifies the Handover Request Transaction from MM to RM
	short                      mmId; 
	// Identifies the Handover Request Transaction from CC to MM
	MSG_Q_ID                   qid;
  };

} IntraL3HandoverRequestNack_t;

// Handover Required
// <taiyu:05-18-01> Global Cell Id 
typedef struct {
		GlobalCellId_t candGlobalCellId;
		unsigned int   candIsExternal;
    	unsigned int   candIsTried;
} candGlobalCellId_t;

typedef struct {
  unsigned char              hoCause;
  short                      numCandidates; 
  
  // The following shall be in descending priority order
  // index 0 has the highest priority
  candGlobalCellId_t candGlobalCellId[MAX_CANDIDATE_CELLS];
  
} IntraL3HandoverRequired_t;

// Handover Command
typedef struct {
  T_CNI_LAPDM_L3MessageUnit  handCmd;  // Complete Layer 3 RR Handover Command Message
} IntraL3HandoverCommand_t;

// Handover Complete
typedef struct {

} IntraL3HandoverComplete_t;

// Handover Failure - used on the source side from RM to CC
// ext-HO <xxu:06-25-01>
typedef struct {
  JcFailureCause_t             cause;
  int                          rrCause;
} IntraL3HandoverFailure_t;

// Perform Handover
typedef struct {
  UINT16                     sourceInRtpPort;
  L3RMChannelType_t          channelType;
  IntraL3CipherModeCommand_t	cipherCmd;   //CIPH<xxu:02-10-00>
  GlobalCellId_t             globalCellId;
} InterVcPerformHandover_t;

// Perform Handover Ack
typedef struct {
  UINT16                     targetInRtpPort;
  T_CNI_LAPDM_L3MessageUnit  handCmd;  // Complete Layer 3 RR Handover Command Message
} InterVcPerformHandoverAck_t;

// Perform Handover Nack 
typedef struct {
  JcFailureCause_t            cause;
} InterVcPerformHandoverNack_t;

// Abort Handover 
typedef struct {
  JcFailureCause_t            cause;
} InterVcAbortHandover_t;

// Complete Handover 
typedef struct {

} InterVcCompleteHandover_t;

// Anchor Remote Release
typedef struct {
  JcFailureCause_t            cause;
} InterVcReleaseCall_t;


// Perform Handback
typedef struct {
  L3RMChannelType_t           channelType;
  unsigned char               hoCause;      //ext-HO <xxu:06-28-01>
  //GlobalCellId_t              globalCellId; //<taiyu:05-18-01> Global Cell Id
  candGlobalCellId_t          globalCellId;

} InterVcPerformHandback_t;

// Perform Handback Ack
typedef struct {
  T_CNI_LAPDM_L3MessageUnit  handCmd;  // Complete Layer 3 RR Handover Command Message
} InterVcPerformHandbackAck_t;

// Perform Handback Nack 
typedef InterVcPerformHandoverNack_t InterVcPerformHandbackNack_t;

// Abort Handback 
typedef InterVcAbortHandover_t InterVcAbortHandback_t;

// Complete Handback 
typedef struct {

} InterVcCompleteHandback_t;

#endif                                       // JCCHoMsg_H
