#ifndef JCCComMsg_H
#define JCCComMsg_H

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
// File        : JCCComMsg.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "MnetModuleId.h"

#include "ril3/ie_cause.h"      //CASUE<xxu:03-20-00>
#include "ril3/ie_mobile_id.h"
#include "ril3/msg_paging_response.h"
#include "ril3/msg_cm_service_request.h"
#include "ril3/msg_cipher_mode_command.h"
#include "ril3/msg_cipher_mode_complete.h"

// include Handover Message definitions
#include "jcchomsg.h"
#include "JCCVcMsg.h"
#include "SmsPP/smspp_inf.h"

#include "cc/CCTypes.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

const int JCC_UNDEFINED               = -1;
const int JCC_ERROR                   = -1;
const int JCC_SUCCESS                 =  0;

const int SAPI_MAIN                   =  0;
const int SAPI_SMS                    =  3;

// Typedefs
//

// Primitive Type for Intra L3 Messages
typedef enum {

  // Data Request / Data Indication  ----------------------------

  INTRA_L3_DATA                       = 0,

  // RR Primitives ----------------------------------------------
  INTRA_L3_RR_PRIM_BASE                    = 9,
  INTRA_L3_RR_EST_REQ, 
  INTRA_L3_RR_REL_REQ,
  INTRA_L3_RR_REL_CNF,
  INTRA_L3_RR_EST_CNF,
  INTRA_L3_RR_REL_IND,
  INTRA_L3_RR_EST_REJ,

  // MM Primitives ----------------------------------------------
  INTRA_L3_MM_PRIM_BASE                    = 19,
  INTRA_L3_MM_EST_IND,
  INTRA_L3_MM_EST_REQ,
  INTRA_L3_MM_EST_CNF,
  INTRA_L3_MM_EST_REJ,

  INTRA_L3_MM_REL_REQ,
  INTRA_L3_MM_REL_IND

} IntraL3PrimitiveType_t;

// Message Type for Intra L3 Messages
typedef enum {

  INTRA_L3_NULL_MSG                   = 0,

  // CC <-> RR Messages -----------------------------------------

  INTRA_L3_CC_TO_RR_BASE              = INTRA_L3_NULL_MSG,
  INTRA_L3_RR_CHANN_ASSIGN_CMD,
  INTRA_L3_RR_CHANN_ASSIGN_COM,
  INTRA_L3_RR_CHANN_ASSIGN_FAIL,
  INTRA_L3_RR_HANDOVER_REQUIRED,
  INTRA_L3_RR_HANDOVER_COMMAND,
  INTRA_L3_RR_HANDOVER_COMPLETE,
  INTRA_L3_RR_HANDOVER_FAILURE,
  INTRA_L3_RR_HANDOVER_ACCESS,        //ext-HO <xxu:06-08-01>

  // MM <-> RR Messages -----------------------------------------
  INTRA_L3_MM_TO_RR_BASE              =  9,
  INTRA_L3_RR_PAGE_REQ,
  INTRA_L3_RR_PAGE_RSP,
  INTRA_L3_RR_HANDOVER_REQ,
  INTRA_L3_RR_HANDOVER_REQ_ACK,
  INTRA_L3_RR_HANDOVER_REQ_NACK,
  INTRA_L3_RR_CIPHER_COMMAND,
  INTRA_L3_RR_CIPHER_COMPLETE,

  // MM <-> CC Messages -----------------------------------------

  INTRA_L3_MM_LOCATE_MS               = 20,


  // CM (Connection Management) Messages
  INTRA_L3_CM_BASE                    = 40,
  INTRA_L3_CM_CONN_CMD,
  INTRA_L3_CM_DISC_CMD,


  // CC <-> H323 Messages -----------------------------------------
  INTRA_L3_CC_Q931_BASE               = 60,
  INTRA_L3_CC_Q931_SETUP              = INTRA_L3_CC_Q931_BASE,
  INTRA_L3_CC_Q931_CALL_PROC          = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_PROCEEDING - VOIP_MOBILE_CALL_EVENT_BASE),
  INTRA_L3_CC_Q931_ALERTING           = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_ALERTING - VOIP_MOBILE_CALL_EVENT_BASE),
  INTRA_L3_CC_Q931_CONNECT            = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_CONNECTED - VOIP_MOBILE_CALL_EVENT_BASE),
  INTRA_L3_CC_H245_CHANN_READY        = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_VOICE_CONNECTED - VOIP_MOBILE_CALL_EVENT_BASE),
  INTRA_L3_CC_Q931_DISCONNECT         = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_DISCONNECTED - VOIP_MOBILE_CALL_EVENT_BASE),

  //BCT <xxu:07-13-00> BEGIN
  INTRA_L3_CC_H450_BCT_STATUS         = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_TRANSFER_STATUS - VOIP_MOBILE_CALL_EVENT_BASE),
  //BCT <xxu:07-13-00> END
  
  //Handle Call Progress message
  INTRA_L3_CC_Q931_PROGRESS           = INTRA_L3_CC_Q931_BASE + (VOIP_MOBILE_CALL_EVENT_PROGRESS - VOIP_MOBILE_CALL_EVENT_BASE),

  INTRA_L3_CC_RAS_IP_ADDRESS_RESPONSE,

  //BCT <xxu:08-11-00> BEGIN
  INTRA_L3_CC_RAS_BCT_NUMBER_RESPONSE,
  //BCT <xxu:08-11-00> END

  //BCT <xxu:09-21-00>
  INTRA_L3_CC_BCT_UPDATE_HANDLES     ,


  INTRA_L3_CC_H245_CHANN_SELECT      ,
  INTRA_L3_CC_VC_UPDATE_INFO         ,

  INTRA_L3_SMS_MESSAGE               ,
  
  // LUDB <-> H323 Messages -----------------------------------------
  INTRA_L3_LUDB_RAS_BASE              = 80,
  INTRA_L3_LUDB_RAS_URQ               = INTRA_L3_LUDB_RAS_BASE,

  // LUDB <-> CC Messages -----------------------------------------
  // <Igal: 04-10-01>
  INTRA_L3_LUDB_CC_BASE               = 90,
  INTRA_L3_LUDB_PURGE_PROFILE         = INTRA_L3_LUDB_CC_BASE,

  // Miscellaneous Messages ------------------------------------

  INTRA_L3_TIMEOUT_MSG                = 120

} IntraL3MsgType_t;


// messages sent from CM to LAPDM (DL)
typedef enum
{
   L3L2_DL_EST_REQ = 1

} L3L2MsgType_t;


// Resource (Channel) Assignment Command 
typedef struct {
  L3RMChannelType_t          channelType;  
} IntraL3ChannAssignCmd_t;

// Paging Request
typedef struct {
  T_CNI_RIL3_IE_MOBILE_ID    imsi; 
  MSG_Q_ID                   qid;
} IntraL3PageRequest_t;

typedef T_CNI_RIL3RRM_MSG_PAGING_RESPONSE IntraL3PageResponse_t;

typedef T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST IntraL3CMServiceRequest_t;

//CIPH<xxu:02-10-00>moved to jcchomsg.h Begin
// RM <--> MM Ciphering Message
//#define CIPHER_KC_LEN	8
//typedef struct{
//	T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMMAND cmd;
//	unsigned char Kc[CIPHER_KC_LEN];
//} IntraL3CipherModeCommand_t;

//typedef struct{
//	T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMPLETE cmp;
//} IntraL3CipherModeComplete_t;
//CIPH<xxu:02-10-00> moved to jcchomsg.h End

// CISS <--> CC Message 
typedef struct 
{
	long Magic;
	long callIdx;
	long msgType;
	long status;                 // holds cause if any
	long msgLen;
	unsigned char msg[CNI_LAPDM_MAX_L3MSG_LENGTH];
}CISSVblinkMsg_t;

#define  kCissVbLinkMsgHdrLen  (sizeof(long) * 5)   

// Intra L3 Setup Message
typedef struct {
  VOIP_API_CALL_OFFERING_MSG callOffering;
  VOIP_MOBILE_HANDLE         mobHandle;
} IntraL3Setup_t;

// Not needed with the new CSU - should be removed with CSU regression testing
// Connection Command 
typedef struct {
  HJCRTPSESSION              hrtpHandle;  // RTP Handle

} IntraL3ConnectCmd_t;

//RR and MM Release Indication 
typedef struct {
  T_CNI_RIL3_CAUSE_VALUE cause;

} IntraL3RrRelInd_t, IntraL3MmRelInd_t;

union IntraL3MsgData_t
{
  IntraL3ChannAssignCmd_t       channAssignCmd;
  IntraL3PageRequest_t          pageReq; 
  IntraL3PageResponse_t         pageRsp;
  IntraL3CMServiceRequest_t     servReq;
  IntraL3Setup_t                setup;
  IntraL3ConnectCmd_t           connectCmd;

  // Handover Messages
  IntraL3HandoverRequest_t      handReq;
  IntraL3HandoverRequestAck_t   handReqAck;
  IntraL3HandoverRequestNack_t  handReqNack;
  IntraL3HandoverRequired_t     handReqd;
  IntraL3HandoverCommand_t      handCmd;
  IntraL3HandoverComplete_t     handCmplt;
  IntraL3HandoverFailure_t      handFail;

  // Ciphering
  IntraL3CipherModeCommand_t	cipherCmd;
  IntraL3CipherModeComplete_t	cipherCmplt;

  // Voip Messages
  VBLINK_API_VC_IP_ADDRESS_RESPONSE_MSG  ipAddressResponse;
  VBLINK_API_VC_BCT_NUMBER_RESPONSE_MSG  bctNumberResponse;
  CCRemMsgData_t                voipMsg;
  CISSVblinkMsg_t               cissMsg;
  SmsVblinkMsg_t                smsVbMsg;

  IntraL3InfoVC_t               statusVCMsg;
};

// Message format between RM, MM and CC */

typedef struct {
  T_CNI_MODULE_ID            module_id;      // origin of a message 
  T_CNI_IRT_ID               entry_id;       // entry ID in IRT table
  union
  {
    IntraL3PrimitiveType_t   primitive_type; // Intra-L3 primitive type
    VOIP_CALL_HANDLE         call_handle;    // call handle
    VOIP_MOBILE_HANDLE       mob_handle;     // mobile handle

	int						 signature;      // CDR <xxu:08-22-00>
  };
  IntraL3MsgType_t           message_type;   // Intra-L3 Message type
  IntraL3MsgData_t           l3_data;        // Enclosed Message

  // T_CNI_LAPDM_SAPI        sapi;           // associated SAPI

  //CASUE<xxu:03-20-00> cause value if it is release indication
  T_CNI_RIL3_CAUSE_VALUE     cause;

  unsigned short		     H323CRV;        // CDR <xxu:08-24-00>
  unsigned short             callleg_id;     // ext-HO <chenj:06-25-01>

} IntraL3Msg_t;

#endif                                       // JCCComMsg_H
