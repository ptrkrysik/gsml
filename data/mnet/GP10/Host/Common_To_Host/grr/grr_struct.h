/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_STRUCT_H__
#define __GRR_STRCUT_H__

#include "grr_head.h"

typedef struct {
		unsigned char state;
		int	          time;
		WDOG_ID	      timer_id;
} grr_Timer_t;

    
typedef struct {
		unsigned char tdma1;
		unsigned char tdma2;
		unsigned char tdma3;
		unsigned char tdma4;
		unsigned char done;
		unsigned char slot;
		unsigned char state;
		unsigned char alive;
		grr_Timer_t   wdog;
} grr_TrxMgmt_t;
    

typedef struct { 
		BtsBasicPackage		btsBasicPackage;
		BtsOptionsPackage	btsOptionsPackage;
		TransceiverPackage	transceiverPackage[2];
	        RadioCarrierPackage	radioCarrierPackage[2];
	        T31xx			t31xx;
		AdjacentCellPackage	adjacentCellPackage;
 		AdjacentCellPackage	adjacentCellPackage_t;
	        HandoverControlPackage	handoverControlPackage;	
		PowerControlPackage	powerControlPackage;
		ResourceConfigData  	grrConfigData;
		int			grr_IQSwap;
		int					grrBtsOpStateOld;
		int					grrBtsAmStateOld;
		int                 grrTrx0AmStateOld;
		int					grrTrx0Slot0AmStateOld;
		int					grrIQSwap;
				
} grr_OamData_t;


#endif //__GRR_STRCUT_H__

