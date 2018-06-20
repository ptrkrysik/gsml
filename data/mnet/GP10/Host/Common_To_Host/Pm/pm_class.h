#ifndef __PMCLASS_H__
#define __PMCLASS_H__

#include "pm/pm_common.h"

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : pm_class.h
// Author(s)   : MindTree
// Create Date : 5/15/01
// Description :  PM GP10 Specifoc Implementation
//
// *******************************************************************
/*
 * The PM_CellMeasurmentPackage class declaration   
 */
class PM_CellMeasurmentPackage
{				/* {viperCellGsmMib 10} */
public:
	    PM_StatisticCounter		meanPCHAGCHQueueLength;			            /* {cellMeasurmentPackage 1} */
	    PM_SimpleCounter		attTransOfPagingMessagesThePCH;		        /* {cellMeasurmentPackage 2} */
	    PM_SimpleCounter		unsuccTransOfPagingMessagesThePCH;	        /* {cellMeasurmentPackage 3} */
	    PM_SimpleCounter		attImmediateAssingProcs;		            /* {cellMeasurmentPackage 4} */
	    PM_SimpleCounter		succImmediateAssingProcs;		            /* {cellMeasurmentPackage 5} */
	    PM_PerCauseCounter		attImmediateAssingProcsPerCauseTable;	    /* {cellMeasurmentPackage 6} */
	    PM_PerCauseCounter		succImmediateAssingProcsPerCauseTable;	    /* {cellMeasurmentPackage 7} */
	    PM_SimpleCounter		nbrOfPagesDiscardedFromPCHQueue;	        /* {cellMeasurmentPackage 8} */
	    PM_StatisticCounter		meanDurationOfSuccPagingProcs;		        /* {cellMeasurmentPackage 9} */
	    PM_SimpleCounter		nbrOfAvailableTCHs;			                /* {cellMeasurmentPackage 10} */
	    PM_StatisticCounter		meanNbrOfBusyTCHs;			                /* {cellMeasurmentPackage 11} */
	    PM_SimpleCounter		maxNbrOfBusyTCHs;			                /* {cellMeasurmentPackage 12} */
	    PM_StatisticCounter		meanNbrOfIdleTCHsPerInterferenceBand;	    /* {cellMeasurmentPackage 13} */
	    PM_SimpleCounter		attTCHSeizures;				                /* {cellMeasurmentPackage 14} */
	    PM_SimpleCounter		succTCHSeizures;			                /* {cellMeasurmentPackage 15} */
	    PM_SimpleCounter		attTCHSeizuresMeetingTCHBlockedState;	    /* {cellMeasurmentPackage 16} */
	    PM_AccumTimeCounter		allAvailableTCHAllocatedTime;		        /* {cellMeasurmentPackage 17} */
	    PM_StatisticCounter		meanTCHBusyTime;			                /* {cellMeasurmentPackage 18} */
	    PM_StatisticCounter		meanTCHQueueLength;			                /* {cellMeasurmentPackage 19} */
	    PM_SimpleCounter		nbrOfLostRadioLinksTCH;			            /* {cellMeasurmentPackage 20} */
	    PM_SimpleCounter		nbrOfAvailableSDCCHs;			            /* {cellMeasurmentPackage 21} */
	    PM_StatisticCounter		meanNbrOfBusySDCCHs;			            /* {cellMeasurmentPackage 22} */
	    PM_SimpleCounter		maxNbrOfBusySDCCHs;			                /* {cellMeasurmentPackage 23} */
	    PM_SimpleCounter		attSDCCHSeizuresMeetingSDCCHBlockedState;   /* {cellMeasurmentPackage 24} */
	    PM_AccumTimeCounter		allAvailableSDCCHAllocatedTime;		        /* {cellMeasurmentPackage 25} */
	    PM_StatisticCounter		meanSDCCHQueueLength;			            /* {cellMeasurmentPackage 26} */
	    PM_SimpleCounter		nbrOfLostRadioLinksSDCCH;		            /* {cellMeasurmentPackage 27} */
	    PM_SimpleCounter		relativeTimeDLPowerControlAtMax;	        /* {cellMeasurmentPackage 28} */
	    PM_SimpleCounter		relativeTimeULPowerControlAtMax;	        /* {cellMeasurmentPackage 29} */
	    PM_SimpleCounter		succInternalHDOsIntraCell;		            /* {cellMeasurmentPackage 30} */
	    PM_SimpleCounter		unsuccInternalHDOsIntraCell;		        /* {cellMeasurmentPackage 31} */
};


/*
 * The PM_CallControlMeasurmentPackage class declaration   
 */
struct PM_CallControlMeasurmentPackage
{		/* {viperCellGsmMib 11} */
public:
	    PM_SimpleCounter		nbrOfClassMarkUpdates;		                /* {callControlMeasurmentPackage 1} */
	    PM_SimpleCounter		attMobileOriginatingCalls;		            /* {callControlMeasurmentPackage 2} */
	    PM_SimpleCounter		succMobileOriginatingCalls;		            /* {callControlMeasurmentPackage 3} */
	    PM_SimpleCounter		ansMobileOriginatingCalls;		            /* {callControlMeasurmentPackage 4} */
	    PM_SimpleCounter		attMobileTerminatingCalls;		            /* {callControlMeasurmentPackage 5} */
	    PM_SimpleCounter		succMobileTerminatingCalls;		            /* {callControlMeasurmentPackage 6} */
	    PM_SimpleCounter		ansMobileTerminatingCalls;		            /* {callControlMeasurmentPackage 7} */
	    PM_SimpleCounter		attMobileEmergencyCalls;		            /* {callControlMeasurmentPackage 8} */
	    PM_SimpleCounter		succMobileEmergencyCalls;		            /* {callControlMeasurmentPackage 9} */
	    PM_SimpleCounter		ansMobileEmergencyCalls;		            /* {callControlMeasurmentPackage 10} */
	    PM_SimpleCounter		attCipheringModeControlProcs;		        /* {callControlMeasurmentPackage 11} */
	    PM_SimpleCounter		succCipheringModeControlProcs;		        /* {callControlMeasurmentPackage 12} */
	    PM_SimpleCounter		attInterrogationOfHLRsForRouting;	        /* {callControlMeasurmentPackage 13} */
	    PM_SimpleCounter		succInterrogationOfHLRsMSRNObtained;	    /* {callControlMeasurmentPackage 14} */
	    PM_SimpleCounter		succInterrogationOfHLRsCallForwarding;	    /* {callControlMeasurmentPackage 15} */
	    PM_SimpleCounter		attOpForMobileOriginatingPointToPointSMs;   /* {callControlMeasurmentPackage 16} */
	    PM_SimpleCounter		succOpForMobileOriginatingPointToPointSMs;  /* {callControlMeasurmentPackage 17} */
	    PM_SimpleCounter		attOpForMobileTerminatingPointToPointSMs;   /* {callControlMeasurmentPackage 18} */
	    PM_SimpleCounter		succOpForMobileTerminatingPointToPointSMs;  /* {callControlMeasurmentPackage 19} */
	    PM_StatisticCounter		meanTimeToCallSetupService;		            /* {callControlMeasurmentPackage 20} */
	    PM_StatisticCounter		meanTimeToLocationUpdateService;	        /* {callControlMeasurmentPackage 21} */
	    PM_SimpleCounter		transSubIdentifiedWithIMSI;		            /* {callControlMeasurmentPackage 22} */
	    PM_SimpleCounter		imsiDetachProcs;			                /* {callControlMeasurmentPackage 23} */
	    PM_SimpleCounter		imsiAttachProcs;			                /* {callControlMeasurmentPackage 24} */
	    PM_SimpleCounter		attIncomingInterMSCHDOs;		            /* {callControlMeasurmentPackage 25} */
	    PM_SimpleCounter		succIncomingInterMSCHDOs;		            /* {callControlMeasurmentPackage 26} */
	    PM_SimpleCounter		attOutgoingInterMSCHDOs;		            /* {callControlMeasurmentPackage 27} */
	    PM_SimpleCounter		succOutgoingInterMSCHDOs;		            /* {callControlMeasurmentPackage 28} */
	    PM_PerCauseCounter		externalHDOsPerCauseTable;		            /* {callControlMeasurmentPackage 29} */
	    PM_SimpleCounter		unsuccExternHDOsWithReconnectionPerMSC;	    /* {callControlMeasurmentPackage 30} */
	    PM_SimpleCounter		unsuccExternHDOsWithLossOfConnectionPerMSC; /* {callControlMeasurmentPackage 31} */
};

void pm_update_mib(void);
int pm_map_cause_to_index(CauseCode cause);
CauseCode pm_map_index_to_cause(int index);
char *pm_map_cause_code_to_str(CauseCode cause);

extern PM_CellMeasurmentPackage			PM_CellMeasurement;
extern PM_CallControlMeasurmentPackage	PM_CCMeasurement;


#endif /* __PMCLASS_H__ */