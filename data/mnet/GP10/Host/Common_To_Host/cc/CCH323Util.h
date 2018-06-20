#ifndef CCH323Util_H
#define CCH323Util_H

// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************


// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CCH23Util.h
// Author(s)   : Bhava Nelakanti
// Create Date : 02-01-98
// Description : Utility functions to send/receive VOIP(H323) messages 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "JCC/JCCL3Msg.h"
#include "CC/CCTypes.h"
#include "CC/CCHalfCall.h"
#include "CC/CallLeg.h"

//ext-HO <chenj:06-06-11>
#include "CC/HOCallLeg.h"

//CDR <xxu:08-23-00>
bool sendH323DisconnectMsg( IntraL3Msg_t *h323InMsg,
                            CdrRecCauseTerm_t cdrCause,
                            T_CNI_RIL3_CAUSE_VALUE ccCause );
                           

bool sendH323Msg(TwoPartyCallLeg     *parent,
                 JCCEvent_t      remoteEvent, 
                 CCRemMsgData_t  remMsgData);

//ext-HO <chenj:06-06-11>
bool sendH323MsgExtHo(HOCallLeg       *parent,
                      JCCEvent_t      remoteEvent, 
                      CCRemMsgData_t  remMsgData,
                      unsigned short    callLegId);

//ext-HO <xxu:06-01-01>
bool sendH323DisconnectMsgExtHo( VOIP_CALL_HANDLE callHandle,
								 T_CNI_RIL3_CAUSE_VALUE ccCause );


#endif                                       // CCH323Util_H
