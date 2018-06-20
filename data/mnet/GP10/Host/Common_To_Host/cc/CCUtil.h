#ifndef CCUtil_H
#define CCUtil_H

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
// File        : CCUtil.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (MM, RR)
#include "JCC/JCCComMsg.h"
#include "JCC/JCCLog.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

// Need to include CC Message QueueTypes
#include "CC/CCTypes.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

//ext-HO <xxu:05-16-01>

#ifndef __CC_EXTHO_DEBUG__
#define __CC_EXTHO_DEBUG__

#include "logging/vclogging.h"

extern   DbgOutput ccExtHoEiDbg;
extern   DbgOutput ccExtHoEoDbg;
extern   DbgOutput ccExtHoHoDbg;

#define DBG_ENTERei();   		ccExtHoEiDbg.Enter()
#define DBG_LEAVEei();   		ccExtHoEiDbg.Leave()
#define DBG_TRACEei 		    ccExtHoEiDbg.Trace
#define DBG_CALL_TRACEei		ccExtHoEiDbg.CallTrace
#define DBG_WARNINGei		    ccExtHoEiDbg.Warning
#define DBG_ERRORei		        ccExtHoEiDbg.Error
#define DBG_HEXDUMPei(x,y);     ccExtHoEiDbg.HexDump(x,y)

#define DBG_ENTEReo();   		ccExtHoEoDbg.Enter()
#define DBG_LEAVEeo();   		ccExtHoEoDbg.Leave()
#define DBG_TRACEeo 		    ccExtHoEoDbg.Trace
#define DBG_CALL_TRACEeo		ccExtHoEoDbg.CallTrace
#define DBG_WARNINGeo		    ccExtHoEoDbg.Warning
#define DBG_ERROReo		        ccExtHoEoDbg.Error
#define DBG_HEXDUMPeo(x,y);     ccExtHoEoDbg.HexDump(x,y)

#define DBG_ENTERho();   		ccExtHoHoDbg.Enter()
#define DBG_LEAVEho();   		ccExtHoHoDbg.Leave()
#define DBG_TRACEho 		    ccExtHoHoDbg.Trace
#define DBG_CALL_TRACEho		ccExtHoHoDbg.CallTrace
#define DBG_WARNINGho		    ccExtHoHoDbg.Warning
#define DBG_ERRORho		        ccExtHoHoDbg.Error
#define DBG_HEXDUMPho(x,y);     ccExtHoHoDbg.HexDump(x,y)

extern bool ccCallTrace;
#define DBG_CCDATA(X)			if (ccCallTrace) ccEntryPrint(ccSession[X], JCCPrintf)

#endif //__CC_EXTHO_DEBUG__

// *******************************************************************
// Constant Definitions
// *******************************************************************

//typedefs

// Utility functions

bool
sendRR      (IntraL3PrimitiveType_t     prim,
             IntraL3MsgType_t           msgType,
             T_CNI_IRT_ID               entryId,
             IntraL3Msg_t                *rrOutMsg);
bool
sendMM (IntraL3PrimitiveType_t     prim,
        IntraL3MsgType_t           msgType,
        T_CNI_IRT_ID               entryId,
        IntraL3Msg_t               *mmOutMsg);

void ccEntryInit(CCSession_t *session);

void handleReleaseSession(CCSession_t *session);

short ccCallAlloc();

short mobInCall (const short        ludbIndex);
short mobInCall2(const T_CNI_IRT_ID entryId  );

short initNewMSConn(const T_CNI_IRT_ID               entryId,
                    short                            &callIndex);

void ccEntryPrint(CCSession_t &session,
                  JCCPrintStFn);

void ccDataPrint();

//ext-HO <xxu:06-08-01>
short ccIsHandoverCallSetup(IntraL3Msg_t * );
short ccHoNumberCompare(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER *hoNumber,
						T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER *cpn     ,
						int length								       );
short ccHoNumberCompare(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER *hoNumber,
						T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER *cpn     ,
						int length								       );

						
#endif                                       // CCUtil_H
