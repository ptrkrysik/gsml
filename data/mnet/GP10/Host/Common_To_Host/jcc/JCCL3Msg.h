#ifndef JCCL3Msg_H
#define JCCL3Msg_H

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
// File        : JCCL3Msg.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// include Handover Message definitions
#include "jccComMsg.h"
#include "jcvctovc.h"
//#include "jccehomsg.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

union CCMMInputMsg_t
{
  InterHoaVcMsg_t			 hoaCcMsg;
  IntraL3Msg_t               intraL3Msg;
  T_CNI_RIL3MD_CCMM_MSG      mdMsg     ;
  VcCcMsg_t                  vcCcMsg   ;
  SmsVblinkMsg_t             smsVbMsg  ;
} ;

#endif                                       // JCCL3Msg_H
