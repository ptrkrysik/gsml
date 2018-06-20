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
// File        : MMTypes.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************
#ifndef MMTypes_H
#define MMTypes_H

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

// include Timer defs
#include "jcc/JCCTimer.h"

// include BSSGP defs
#include "bssgp/bssgp_api.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

//typedefs

// Data Structures needed inside the MM Module

typedef enum {

  MM_PROC_NULL					= 0,

  MM_PROC_LUDB					= 1,
  MM_PROC_MOB_ORIG				= 2,
  MM_PROC_MOB_TERM				= 3

} MMProcedureType_t;

typedef enum {

  // COMMON
  MM_START					= 0,
  MM_CLEAR_REQ,
  MM_CONN_EST,
  MM_EMERGENCY,

  // Ciphering Authentication
  MM_CIPHER_CMD,
  MM_AUTH_LOCUP,
  MM_AUTH_ORIG,
  MM_AUTH_TERM,

  // LUDB OPERATION
  MM_WAIT4REG,
  MM_WAIT4UNREG,
  MM_WAIT4SPI,
  
  // MOB_ORIG

  // MOB_TERM              
  MM_PAGE_REQ,
  MM_HAND_REQ,

} MMState_t;

typedef struct {
  MMProcedureType_t          procType;
  MMState_t                  mmState;
} MMProcedureData_t;

typedef struct {
  T_CNI_LAPDM_OID            oid;
  T_CNI_LAPDM_SAPI           sapi;
  T_CNI_IRT_ID               entryId;
  MMProcedureData_t          procData;
  IntraL3PageRequest_t       pageReq;
  T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST	cmServReq;
  T_CNI_RIL3RRM_MSG_PAGING_RESPONSE		pageRsp;
  MSG_Q_ID                   ccQId;			// added for handover request/resp
 
  bool                       free;
  
  JCCTimer                   *pageTimer;
  JCCTimer                   *pageCCTimer;
  JCCTimer                   *authTimer;
  JCCTimer					 *identTimer;
  T_CNI_RIL3_IE_MOBILE_ID    mobileId;	   // add mobile Id . -gz
  short			     		 ludbId;	   // add ludbaId index. -gz
  short						 emerg_ludb;
  short						 mmId2;
  char						 lup_cksn;
  unsigned long				 pagingTick;
  unsigned long				 locupTick;
  T_CNI_RIL3_CAUSE_VALUE	 cause;

  //SMS <xxu:06-01-00> Begin
  int						 hoConn;
  //SMS <xxu:06-01-00> End

  int                        	pageRetries;   // Count the number of page re-tries, max = 3.
  unsigned long			vblinkReqId; // VBLINK Page request ID shmin 08/02/01
  
    //GPRS combined location update Begin
  bool                       	gsInitiated;
  BSSGP_BVCI			bvci;
  //GPRS combined location update End

  } MMEntry_t;

#endif                                       // MMTypes_H


