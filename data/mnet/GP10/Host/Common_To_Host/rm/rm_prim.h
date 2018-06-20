/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_PRIM_H__
#define __RM_PRIM_H__

#include "MnetModuleId.h"
#include "lapdm\lapdm_l3intf.h"
#include "jcc\JCCL3Msg.h"

// Primitive Defs related to RR module 

#define RR_DATA			INTRA_L3_DATA
#define RR_EST_REQ	        INTRA_L3_RR_EST_REQ
#define RR_EST_CNF		INTRA_L3_RR_EST_CNF
#define RR_REL_REQ		INTRA_L3_RR_REL_REQ
#define RR_REL_CNF		INTRA_L3_RR_REL_CNF 
#define RR_REL_IND		INTRA_L3_RR_REL_IND

#define MDL_ERR_RPT		L23PT_MDL_ERROR
#define DL_EST_IND		L23PT_DL_EST_IND
#define DL_REL_IND		L23PT_DL_REL_IND
#define DL_DATA_IND		L23PT_DL_DATA_IND

#define MPH_INFO_IND		1	
#define OAM_DATA_IND		2	
#define RM_TIMER_IND		3	

//HOADDED
#define DLm_OTHRA_IND		4     //RA carrying Handover Access

#define RRm_EST_REQ		( MODULE_MM<<8  | RR_EST_REQ )
#define RRg_EST_REQ		( MODULE_BSSGP<<8  | RR_EST_REQ )
#define DLm_RA_IND		( 0x0100 | L23PT_DL_RANDOM_ACCESS_IND )
#define MPHm_INFO_IND		( MODULE_L1<<8  | MPH_INFO_IND )
#define OAMm_DATA_IND		( MODULE_OAM<<8 | OAM_DATA_IND )

#endif //__RM_PRIM_H__
