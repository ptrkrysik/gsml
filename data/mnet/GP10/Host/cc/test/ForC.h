#ifndef ForC_H
#define ForC_H
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
// File        : ForC.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included VxWorks headers
#include "vxWorks.h"
#include "JCC/JCCLog.h"
#include "JCC/JCCL3Msg.h"

#include "lapdm/lapdm_l3intf.h"

#include "MM/MMConfig.h"
#include "MM/MMInt.h"
#include "RIL3/ril3irt_db.h"

#include "CC/CallConfig.h"
#include "CC/CCconfig.h"
#include "CC/CCInt.h"
#include "CC/CCTypes.h"

#include "JCC/LUDBInstance.h"
#include "JCC/LUDBConfig.h"

#ifdef __cplusplus
extern "C" {
#endif


extern void populateLUDB();

extern void tccInit();

extern void tmmPrint();

extern void tccPrint();

extern void tmmInit();


extern void ccTest();

extern void irtDataPrint();

void fromMS(short entryId,
	    T_CNI_LAPDM_L2L3PrimitiveType prim,
	    unsigned char buff0, 
	    unsigned char buff1);
extern
void fromRRtoMM(short                      entryId,
		IntraL3PrimitiveType_t     prim,
		IntraL3MsgType_t           msgType);
extern
void fromRRtoCC(short                      entryId,
		IntraL3PrimitiveType_t     prim,
		IntraL3MsgType_t           msgType,
		short                      msSide);


#ifdef __cplusplus
}
#endif

#endif   // ForC_H
