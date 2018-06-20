#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <vxWorks.h>
//#include <semlib.h>
#include <time.h>
#include <wdlib.h>
//#include <sysLib.h>
//#include <taskLib.h>
//#include <fioLib.h>

#ifndef UNIT_TEST_PMCLASS
#include "vipermib.h"
#endif

#include "defs.h"
#include "apm/include/apmdefs.h"

#include "apm/include/msgproc.h"

#include "oam_api.h"
#include "pm/pm_class.h"

static bool initialized = false;
/*
 * Global variables
 */
static WDOG_ID	  g_pm_timer;
static int g_timer_ticks_required;
PmSourceToAmsMsg amsMsg;
static unsigned long reqd_interval_in_secs;
void pm_timer_handler(int arg);
/*
 * Storage allocation for the classes
 */
PM_CellMeasurmentPackage			PM_CellMeasurement;
PM_CallControlMeasurmentPackage	PM_CCMeasurement;


#define PM_SET_MIB_VALUE(x,y)  if ((status=oam_setMibIntVar(MODULE_PM,x,y))!=STATUS_OK) { \
                                DBG_ERROR("Error setting PM value into MIB, error %d\n",status); \
                                }
/***************************/

/*
 * pm_reset_all_counters
 */

void
pm_reset_all_counters()
{
        /*
         * reset all the counters for the current interval
         * the PM_CellMeasurement package
         */
        PM_CellMeasurement.meanPCHAGCHQueueLength.reset();
        PM_CellMeasurement.attTransOfPagingMessagesThePCH.reset();
        PM_CellMeasurement.unsuccTransOfPagingMessagesThePCH.reset();
        PM_CellMeasurement.attImmediateAssingProcs.reset();
        PM_CellMeasurement.succImmediateAssingProcs.reset();
        PM_CellMeasurement.attImmediateAssingProcsPerCauseTable.reset();
        PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.reset();
        PM_CellMeasurement.nbrOfPagesDiscardedFromPCHQueue.reset();
        PM_CellMeasurement.meanDurationOfSuccPagingProcs.reset();
        PM_CellMeasurement.nbrOfAvailableTCHs.reset();
        PM_CellMeasurement.meanNbrOfBusyTCHs.reset();
        PM_CellMeasurement.maxNbrOfBusyTCHs.reset();
        PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.reset();
        PM_CellMeasurement.attTCHSeizures.reset();
        PM_CellMeasurement.succTCHSeizures.reset();
        PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.reset();
        PM_CellMeasurement.allAvailableTCHAllocatedTime.reset();
        PM_CellMeasurement.meanTCHBusyTime.reset();
        PM_CellMeasurement.meanTCHQueueLength.reset();        
        PM_CellMeasurement.nbrOfLostRadioLinksTCH.reset();
        PM_CellMeasurement.nbrOfAvailableSDCCHs.reset();
        PM_CellMeasurement.meanNbrOfBusySDCCHs.reset();
        PM_CellMeasurement.maxNbrOfBusySDCCHs.reset();
        PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.reset();
        PM_CellMeasurement.allAvailableSDCCHAllocatedTime.reset();
        PM_CellMeasurement.meanSDCCHQueueLength.reset();
        PM_CellMeasurement.nbrOfLostRadioLinksSDCCH.reset();
        PM_CellMeasurement.relativeTimeDLPowerControlAtMax.reset();
        PM_CellMeasurement.relativeTimeULPowerControlAtMax.reset();
        PM_CellMeasurement.succInternalHDOsIntraCell.reset();
        PM_CellMeasurement.unsuccInternalHDOsIntraCell.reset();


        /*
         * reset all the counters for the current interval
         * the PM_CCMeasurement package
         */
        PM_CCMeasurement.nbrOfClassMarkUpdates.reset();
        PM_CCMeasurement.attMobileOriginatingCalls.reset();
        PM_CCMeasurement.succMobileOriginatingCalls.reset();
        PM_CCMeasurement.ansMobileOriginatingCalls.reset();
        PM_CCMeasurement.attMobileTerminatingCalls.reset();
        PM_CCMeasurement.succMobileTerminatingCalls.reset();
        PM_CCMeasurement.ansMobileTerminatingCalls.reset();
        PM_CCMeasurement.attMobileEmergencyCalls.reset();
        PM_CCMeasurement.succMobileEmergencyCalls.reset();
        PM_CCMeasurement.ansMobileEmergencyCalls.reset();
        PM_CCMeasurement.attCipheringModeControlProcs.reset();
        PM_CCMeasurement.succCipheringModeControlProcs.reset();
        PM_CCMeasurement.attInterrogationOfHLRsForRouting.reset();
        PM_CCMeasurement.succInterrogationOfHLRsMSRNObtained.reset();
        PM_CCMeasurement.succInterrogationOfHLRsCallForwarding.reset();
        PM_CCMeasurement.attOpForMobileOriginatingPointToPointSMs.reset();
        PM_CCMeasurement.succOpForMobileOriginatingPointToPointSMs.reset();
        PM_CCMeasurement.attOpForMobileTerminatingPointToPointSMs.reset();
        PM_CCMeasurement.succOpForMobileTerminatingPointToPointSMs.reset();
        PM_CCMeasurement.meanTimeToCallSetupService.reset();
        PM_CCMeasurement.meanTimeToLocationUpdateService.reset();
        PM_CCMeasurement.transSubIdentifiedWithIMSI.reset();
        PM_CCMeasurement.imsiDetachProcs.reset();
        PM_CCMeasurement.imsiAttachProcs.reset();
        PM_CCMeasurement.attIncomingInterMSCHDOs.reset();
        PM_CCMeasurement.succIncomingInterMSCHDOs.reset();
        PM_CCMeasurement.attOutgoingInterMSCHDOs.reset();
        PM_CCMeasurement.succOutgoingInterMSCHDOs.reset();
        PM_CCMeasurement.externalHDOsPerCauseTable.reset();
        PM_CCMeasurement.unsuccExternHDOsWithReconnectionPerMSC.reset();
        PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.reset();

}

/*
 * pm_reset_all_counters
 */

void
pm_advance_interval()
{
		int oldValue;
        /*
         * Advance the intervals for all the variables in
         * the PM_CellMeasurement package
         */
        PM_CellMeasurement.meanPCHAGCHQueueLength.advanceInterval();
        PM_CellMeasurement.attTransOfPagingMessagesThePCH.advanceInterval();
        PM_CellMeasurement.unsuccTransOfPagingMessagesThePCH.advanceInterval();
        PM_CellMeasurement.attImmediateAssingProcs.advanceInterval();
        PM_CellMeasurement.succImmediateAssingProcs.advanceInterval();
        PM_CellMeasurement.attImmediateAssingProcsPerCauseTable.advanceInterval();
        PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.advanceInterval();
        PM_CellMeasurement.nbrOfPagesDiscardedFromPCHQueue.advanceInterval();
        PM_CellMeasurement.meanDurationOfSuccPagingProcs.advanceInterval();
        PM_CellMeasurement.meanDurationOfSuccPagingProcs.reset();

		oldValue = PM_CellMeasurement.nbrOfAvailableTCHs.getValue();
        PM_CellMeasurement.nbrOfAvailableTCHs.advanceInterval();
		PM_CellMeasurement.nbrOfAvailableTCHs.setValue(oldValue);

        PM_CellMeasurement.meanNbrOfBusyTCHs.advanceInterval();
        PM_CellMeasurement.maxNbrOfBusyTCHs.advanceInterval();
        PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.advanceInterval();
        PM_CellMeasurement.attTCHSeizures.advanceInterval();
        PM_CellMeasurement.succTCHSeizures.advanceInterval();
        PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.advanceInterval();
        PM_CellMeasurement.allAvailableTCHAllocatedTime.advanceInterval();

        PM_CellMeasurement.meanTCHBusyTime.advanceInterval();
        PM_CellMeasurement.meanTCHBusyTime.reset();

        PM_CellMeasurement.meanTCHQueueLength.advanceInterval();        
        PM_CellMeasurement.nbrOfLostRadioLinksTCH.advanceInterval();

		oldValue = PM_CellMeasurement.nbrOfAvailableSDCCHs.getValue();
        PM_CellMeasurement.nbrOfAvailableSDCCHs.advanceInterval();
		PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(oldValue);

        PM_CellMeasurement.meanNbrOfBusySDCCHs.advanceInterval();
        PM_CellMeasurement.maxNbrOfBusySDCCHs.advanceInterval();
        PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.advanceInterval();
        PM_CellMeasurement.allAvailableSDCCHAllocatedTime.advanceInterval();

        PM_CellMeasurement.meanSDCCHQueueLength.advanceInterval();
        PM_CellMeasurement.meanSDCCHQueueLength.reset();

        PM_CellMeasurement.nbrOfLostRadioLinksSDCCH.advanceInterval();
        PM_CellMeasurement.relativeTimeDLPowerControlAtMax.advanceInterval();
        PM_CellMeasurement.relativeTimeULPowerControlAtMax.advanceInterval();
        PM_CellMeasurement.succInternalHDOsIntraCell.advanceInterval();
        PM_CellMeasurement.unsuccInternalHDOsIntraCell.advanceInterval();


        /*
         * Advance the intervals for all the variables in
         * the PM_CCMeasurement package
         */
        PM_CCMeasurement.nbrOfClassMarkUpdates.advanceInterval();
        PM_CCMeasurement.attMobileOriginatingCalls.advanceInterval();
        PM_CCMeasurement.succMobileOriginatingCalls.advanceInterval();
        PM_CCMeasurement.ansMobileOriginatingCalls.advanceInterval();
        PM_CCMeasurement.attMobileTerminatingCalls.advanceInterval();
        PM_CCMeasurement.succMobileTerminatingCalls.advanceInterval();
        PM_CCMeasurement.ansMobileTerminatingCalls.advanceInterval();
        PM_CCMeasurement.attMobileEmergencyCalls.advanceInterval();
        PM_CCMeasurement.succMobileEmergencyCalls.advanceInterval();
        PM_CCMeasurement.ansMobileEmergencyCalls.advanceInterval();
        PM_CCMeasurement.attCipheringModeControlProcs.advanceInterval();
        PM_CCMeasurement.succCipheringModeControlProcs.advanceInterval();
        PM_CCMeasurement.attInterrogationOfHLRsForRouting.advanceInterval();
        PM_CCMeasurement.succInterrogationOfHLRsMSRNObtained.advanceInterval();
        PM_CCMeasurement.succInterrogationOfHLRsCallForwarding.advanceInterval();
        PM_CCMeasurement.attOpForMobileOriginatingPointToPointSMs.advanceInterval();
        PM_CCMeasurement.succOpForMobileOriginatingPointToPointSMs.advanceInterval();
        PM_CCMeasurement.attOpForMobileTerminatingPointToPointSMs.advanceInterval();
        PM_CCMeasurement.succOpForMobileTerminatingPointToPointSMs.advanceInterval();

        PM_CCMeasurement.meanTimeToCallSetupService.advanceInterval();
        PM_CCMeasurement.meanTimeToCallSetupService.reset();

        PM_CCMeasurement.meanTimeToLocationUpdateService.advanceInterval();
        PM_CCMeasurement.meanTimeToLocationUpdateService.reset();

        PM_CCMeasurement.transSubIdentifiedWithIMSI.advanceInterval();
        PM_CCMeasurement.imsiDetachProcs.advanceInterval();
        PM_CCMeasurement.imsiAttachProcs.advanceInterval();
        PM_CCMeasurement.attIncomingInterMSCHDOs.advanceInterval();
        PM_CCMeasurement.succIncomingInterMSCHDOs.advanceInterval();
        PM_CCMeasurement.attOutgoingInterMSCHDOs.advanceInterval();
        PM_CCMeasurement.succOutgoingInterMSCHDOs.advanceInterval();
        PM_CCMeasurement.externalHDOsPerCauseTable.advanceInterval();
        PM_CCMeasurement.unsuccExternHDOsWithReconnectionPerMSC.advanceInterval();
        PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.advanceInterval();
}



/*
 * pm_update_mib
 * Function for internal debugging purposes only
 */
/******************************/
void
pm_update_mib()
{
    DBG_FUNC("pm_update_mib",PM_LAYER);
    DBG_ENTER();
    STATUS status;
    int i;
    struct {
        int     cause_code;
        int     cause_value;
    }pm_percause_mib_entry;
    time_t long_time;


    PM_SET_MIB_VALUE(MIB_meanPCHAGCHQueueLength,PM_CellMeasurement.meanPCHAGCHQueueLength.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanPCHAGCHQueueLength =
		htonl(PM_CellMeasurement.meanPCHAGCHQueueLength.getMeanValue());

    PM_SET_MIB_VALUE(MIB_attTransOfPagingMessagesThePCH,PM_CellMeasurement.attTransOfPagingMessagesThePCH.getValue());
	amsMsg.body.pmData.src.gp10PmData.attTransOfPagingMessagesThePCH =
		htonl(PM_CellMeasurement.attTransOfPagingMessagesThePCH.getValue());

    PM_SET_MIB_VALUE(MIB_unsuccTransOfPagingMessagesThePCH,PM_CellMeasurement.unsuccTransOfPagingMessagesThePCH.getValue());
	amsMsg.body.pmData.src.gp10PmData.unsuccTransOfPagingMessagesThePCH =
		htonl(PM_CellMeasurement.unsuccTransOfPagingMessagesThePCH.getValue());

    PM_SET_MIB_VALUE(MIB_attImmediateAssingProcs,PM_CellMeasurement.attImmediateAssingProcs.getValue());
	amsMsg.body.pmData.src.gp10PmData.attImmediateAssingProcs =
		htonl(PM_CellMeasurement.attImmediateAssingProcs.getValue());

    PM_SET_MIB_VALUE(MIB_succImmediateAssingProcs,PM_CellMeasurement.succImmediateAssingProcs.getValue());
	amsMsg.body.pmData.src.gp10PmData.succImmediateAssingProcs =
		htonl(PM_CellMeasurement.succImmediateAssingProcs.getValue());

    PM_SET_MIB_VALUE(MIB_nbrOfPagesDiscardedFromPCHQueue,PM_CellMeasurement.nbrOfPagesDiscardedFromPCHQueue.getValue());
	amsMsg.body.pmData.src.gp10PmData.nbrOfPagesDiscardedFromPCHQueue =
		htonl(PM_CellMeasurement.nbrOfPagesDiscardedFromPCHQueue.getValue());

    PM_SET_MIB_VALUE(MIB_meanDurationOfSuccPagingProcs,PM_CellMeasurement.meanDurationOfSuccPagingProcs.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanDurationOfSuccPagingProcs =
		htonl(PM_CellMeasurement.meanDurationOfSuccPagingProcs.getMeanValue());

    PM_SET_MIB_VALUE(MIB_nbrOfAvailableTCHs,PM_CellMeasurement.nbrOfAvailableTCHs.getValue());
	amsMsg.body.pmData.src.gp10PmData.nbrOfAvailableTCHs =
		htonl(PM_CellMeasurement.nbrOfAvailableTCHs.getValue());

    PM_SET_MIB_VALUE(MIB_meanNbrOfBusyTCHs,PM_CellMeasurement.meanNbrOfBusyTCHs.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanNbrOfBusyTCHs =
		htonl(PM_CellMeasurement.meanNbrOfBusyTCHs.getMeanValue());

    PM_SET_MIB_VALUE(MIB_maxNbrOfBusyTCHs,PM_CellMeasurement.maxNbrOfBusyTCHs.getMaxValue());
	amsMsg.body.pmData.src.gp10PmData.maxNbrOfBusyTCHs =
		htonl(PM_CellMeasurement.maxNbrOfBusyTCHs.getMaxValue());

    PM_SET_MIB_VALUE(MIB_meanNbrOfIdleTCHsPerInterferenceBand,PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanNbrOfIdleTCHsPerInterferenceBand =
		htonl(PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.getMeanValue());

    PM_SET_MIB_VALUE(MIB_attTCHSeizures,PM_CellMeasurement.attTCHSeizures.getValue());
	amsMsg.body.pmData.src.gp10PmData.attTCHSeizures =
		htonl(PM_CellMeasurement.attTCHSeizures.getValue());

    PM_SET_MIB_VALUE(MIB_succTCHSeizures,PM_CellMeasurement.succTCHSeizures.getValue());
	amsMsg.body.pmData.src.gp10PmData.succTCHSeizures =
		htonl(PM_CellMeasurement.succTCHSeizures.getValue());

    PM_SET_MIB_VALUE(MIB_attTCHSeizuresMeetingTCHBlockedState,PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.getValue());
	amsMsg.body.pmData.src.gp10PmData.attTCHSeizuresMeetingTCHBlockedState =
		htonl(PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.getValue());

    PM_SET_MIB_VALUE(MIB_allAvailableTCHAllocatedTime,PM_CellMeasurement.allAvailableTCHAllocatedTime.getValue());
	amsMsg.body.pmData.src.gp10PmData.allAvailableTCHAllocatedTime =
		htonl(PM_CellMeasurement.allAvailableTCHAllocatedTime.getValue());

    PM_SET_MIB_VALUE(MIB_meanTCHBusyTime,PM_CellMeasurement.meanTCHBusyTime.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanTCHBusyTime =
		htonl(PM_CellMeasurement.meanTCHBusyTime.getMeanValue());

    PM_SET_MIB_VALUE(MIB_meanTCHQueueLength,PM_CellMeasurement.meanTCHQueueLength.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanTCHQueueLength =
		htonl(PM_CellMeasurement.meanTCHQueueLength.getMeanValue());

    PM_SET_MIB_VALUE(MIB_nbrOfLostRadioLinksTCH,PM_CellMeasurement.nbrOfLostRadioLinksTCH.getValue());
	amsMsg.body.pmData.src.gp10PmData.nbrOfLostRadioLinksTCH =
		htonl(PM_CellMeasurement.nbrOfLostRadioLinksTCH.getValue());

    PM_SET_MIB_VALUE(MIB_nbrOfAvailableSDCCHs,PM_CellMeasurement.nbrOfAvailableSDCCHs.getValue());
	amsMsg.body.pmData.src.gp10PmData.nbrOfAvailableSDCCHs =
		htonl(PM_CellMeasurement.nbrOfAvailableSDCCHs.getValue());

    PM_SET_MIB_VALUE(MIB_meanNbrOfBusySDCCHs,PM_CellMeasurement.meanNbrOfBusySDCCHs.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanNbrOfBusySDCCHs =
		htonl(PM_CellMeasurement.meanNbrOfBusySDCCHs.getMeanValue());

    PM_SET_MIB_VALUE(MIB_maxNbrOfBusySDCCHs,PM_CellMeasurement.maxNbrOfBusySDCCHs.getMaxValue());
	amsMsg.body.pmData.src.gp10PmData.maxNbrOfBusySDCCHs =
		htonl(PM_CellMeasurement.maxNbrOfBusySDCCHs.getMaxValue());

    PM_SET_MIB_VALUE(MIB_attSDCCHSeizuresMeetingSDCCHBlockedState,PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.getValue());
	amsMsg.body.pmData.src.gp10PmData.attSDCCHSeizuresMeetingSDCCHBlockedState =
		htonl(PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.getValue());

    PM_SET_MIB_VALUE(MIB_allAvailableSDCCHAllocatedTime,PM_CellMeasurement.allAvailableSDCCHAllocatedTime.getValue());
	amsMsg.body.pmData.src.gp10PmData.allAvailableSDCCHAllocatedTime =
		htonl(PM_CellMeasurement.allAvailableSDCCHAllocatedTime.getValue());

    PM_SET_MIB_VALUE(MIB_meanSDCCHQueueLength,PM_CellMeasurement.meanSDCCHQueueLength.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanSDCCHQueueLength =
		htonl(PM_CellMeasurement.meanSDCCHQueueLength.getMeanValue());

    PM_SET_MIB_VALUE(MIB_nbrOfLostRadioLinksSDCCH,PM_CellMeasurement.nbrOfLostRadioLinksSDCCH.getValue());
	amsMsg.body.pmData.src.gp10PmData.nbrOfLostRadioLinksSDCCH =
		htonl(PM_CellMeasurement.nbrOfLostRadioLinksSDCCH.getValue());

    PM_SET_MIB_VALUE(MIB_relativeTimeDLPowerControlAtMax,PM_CellMeasurement.relativeTimeDLPowerControlAtMax.getValue());
	amsMsg.body.pmData.src.gp10PmData.relativeTimeDLPowerControlAtMax =
		htonl(PM_CellMeasurement.relativeTimeDLPowerControlAtMax.getValue());

    PM_SET_MIB_VALUE(MIB_relativeTimeULPowerControlAtMax,PM_CellMeasurement.relativeTimeULPowerControlAtMax.getValue());
	amsMsg.body.pmData.src.gp10PmData.relativeTimeULPowerControlAtMax =
		htonl(PM_CellMeasurement.relativeTimeULPowerControlAtMax.getValue());

    PM_SET_MIB_VALUE(MIB_succInternalHDOsIntraCell,PM_CellMeasurement.succInternalHDOsIntraCell.getValue());
	amsMsg.body.pmData.src.gp10PmData.succInternalHDOsIntraCell =
		htonl(PM_CellMeasurement.succInternalHDOsIntraCell.getValue());

    PM_SET_MIB_VALUE(MIB_unsuccInternalHDOsIntraCell,PM_CellMeasurement.unsuccInternalHDOsIntraCell.getValue());
	amsMsg.body.pmData.src.gp10PmData.unsuccInternalHDOsIntraCell =
		htonl(PM_CellMeasurement.unsuccInternalHDOsIntraCell.getValue());
    
    for(i=0;i<PM_CURRENT_MAX_CAUSES;i++)
	{
        pm_percause_mib_entry.cause_code = pm_map_index_to_cause(i);
        pm_percause_mib_entry.cause_value =
			PM_CellMeasurement.attImmediateAssingProcsPerCauseTable.getValue(i);
		amsMsg.body.pmData.src.gp10PmData.attImmediateAssingProcsPerCauseTable[i] =
			htonl(PM_CellMeasurement.attImmediateAssingProcsPerCauseTable.getValue(i));
        if ((status=oam_setMibTblEntry(MODULE_PM,
			MIBT_attImmediateAssingProcsPerCauseEntry, i, 
        	(void *)&pm_percause_mib_entry,sizeof(pm_percause_mib_entry)))!=STATUS_OK)
		{
            DBG_ERROR("Unable to set MIB table entry for PM per cause counter code %s\n",
            	pm_map_cause_code_to_str(pm_percause_mib_entry.cause_code));
        }
    }

    for(i=0;i<PM_CURRENT_MAX_CAUSES;i++) {
        pm_percause_mib_entry.cause_code = pm_map_index_to_cause(i);
        pm_percause_mib_entry.cause_value = PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.getValue(i);
		amsMsg.body.pmData.src.gp10PmData.succImmediateAssingProcsPerCauseTable[i] =
			htonl(PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.getValue(i));
        if ((status=oam_setMibTblEntry(MODULE_PM, MIBT_succImmediateAssingProcsPerCauseEntry, i, (void *)&pm_percause_mib_entry,sizeof(pm_percause_mib_entry)))!=STATUS_OK) {
            DBG_ERROR("Unable to set MIB table entry for PM per cause counter code %s\n",
            	pm_map_cause_code_to_str(pm_percause_mib_entry.cause_code));
        }
    }
       

    PM_SET_MIB_VALUE(MIB_nbrOfClassMarkUpdates,PM_CCMeasurement.nbrOfClassMarkUpdates.getValue());
	amsMsg.body.pmData.src.gp10PmData.nbrOfClassMarkUpdates =
		htonl(PM_CCMeasurement.nbrOfClassMarkUpdates.getValue());
    
    PM_SET_MIB_VALUE(MIB_attMobileOriginatingCalls,PM_CCMeasurement.attMobileOriginatingCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.attMobileOriginatingCalls =
		htonl(PM_CCMeasurement.attMobileOriginatingCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_succMobileOriginatingCalls,PM_CCMeasurement.succMobileOriginatingCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.succMobileOriginatingCalls =
		htonl(PM_CCMeasurement.succMobileOriginatingCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_ansMobileOriginatingCalls,PM_CCMeasurement.ansMobileOriginatingCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.ansMobileOriginatingCalls =
		htonl(PM_CCMeasurement.ansMobileOriginatingCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_attMobileTerminatingCalls,PM_CCMeasurement.attMobileTerminatingCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.attMobileTerminatingCalls =
		htonl(PM_CCMeasurement.attMobileTerminatingCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_succMobileTerminatingCalls,PM_CCMeasurement.succMobileTerminatingCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.succMobileTerminatingCalls =
		htonl(PM_CCMeasurement.succMobileTerminatingCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_ansMobileTerminatingCalls,PM_CCMeasurement.ansMobileTerminatingCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.ansMobileTerminatingCalls =
		htonl(PM_CCMeasurement.ansMobileTerminatingCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_attMobileEmergencyCalls,PM_CCMeasurement.attMobileEmergencyCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.attMobileEmergencyCalls =
		htonl(PM_CCMeasurement.attMobileEmergencyCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_succMobileEmergencyCalls,PM_CCMeasurement.succMobileEmergencyCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.succMobileEmergencyCalls =
		htonl(PM_CCMeasurement.succMobileEmergencyCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_ansMobileEmergencyCalls,PM_CCMeasurement.ansMobileEmergencyCalls.getValue());
	amsMsg.body.pmData.src.gp10PmData.ansMobileEmergencyCalls =
		htonl(PM_CCMeasurement.ansMobileEmergencyCalls.getValue());
    
    PM_SET_MIB_VALUE(MIB_attCipheringModeControlProcs,PM_CCMeasurement.attCipheringModeControlProcs.getValue());
	amsMsg.body.pmData.src.gp10PmData.attCipheringModeControlProcs =
		htonl(PM_CCMeasurement.attCipheringModeControlProcs.getValue());
    
    PM_SET_MIB_VALUE(MIB_succCipheringModeControlProcs,PM_CCMeasurement.succCipheringModeControlProcs.getValue());
	amsMsg.body.pmData.src.gp10PmData.succCipheringModeControlProcs =
		htonl(PM_CCMeasurement.succCipheringModeControlProcs.getValue());
    
    PM_SET_MIB_VALUE(MIB_attInterrogationOfHLRsForRouting,PM_CCMeasurement.attInterrogationOfHLRsForRouting.getValue());
	amsMsg.body.pmData.src.gp10PmData.attInterrogationOfHLRsForRouting =
		htonl(PM_CCMeasurement.attInterrogationOfHLRsForRouting.getValue());
    
    PM_SET_MIB_VALUE(MIB_succInterrogationOfHLRsMSRNObtained,PM_CCMeasurement.succInterrogationOfHLRsMSRNObtained.getValue());
	amsMsg.body.pmData.src.gp10PmData.succInterrogationOfHLRsMSRNObtained =
		htonl(PM_CCMeasurement.succInterrogationOfHLRsMSRNObtained.getValue());
    
    PM_SET_MIB_VALUE(MIB_succInterrogationOfHLRsCallForwarding,PM_CCMeasurement.succInterrogationOfHLRsCallForwarding.getValue());
	amsMsg.body.pmData.src.gp10PmData.succInterrogationOfHLRsCallForwarding =
		htonl(PM_CCMeasurement.succInterrogationOfHLRsCallForwarding.getValue());
    
    PM_SET_MIB_VALUE(MIB_attOpForMobileOriginatingPointToPointSMs,PM_CCMeasurement.attOpForMobileOriginatingPointToPointSMs.getValue());
	amsMsg.body.pmData.src.gp10PmData.attOpForMobileOriginatingPointToPointSMs =
		htonl(PM_CCMeasurement.attOpForMobileOriginatingPointToPointSMs.getValue());
    
    PM_SET_MIB_VALUE(MIB_succOpForMobileOriginatingPointToPointSMs,PM_CCMeasurement.succOpForMobileOriginatingPointToPointSMs.getValue());
	amsMsg.body.pmData.src.gp10PmData.succOpForMobileOriginatingPointToPointSMs =
		htonl(PM_CCMeasurement.succOpForMobileOriginatingPointToPointSMs.getValue());
    
    PM_SET_MIB_VALUE(MIB_attOpForMobileTerminatingPointToPointSMs,PM_CCMeasurement.attOpForMobileTerminatingPointToPointSMs.getValue());
	amsMsg.body.pmData.src.gp10PmData.attOpForMobileTerminatingPointToPointSMs =
		htonl(PM_CCMeasurement.attOpForMobileTerminatingPointToPointSMs.getValue());
    
    PM_SET_MIB_VALUE(MIB_succOpForMobileTerminatingPointToPointSMs,PM_CCMeasurement.succOpForMobileTerminatingPointToPointSMs.getValue());
	amsMsg.body.pmData.src.gp10PmData.succOpForMobileTerminatingPointToPointSMs =
		htonl(PM_CCMeasurement.succOpForMobileTerminatingPointToPointSMs.getValue());
    
    PM_SET_MIB_VALUE(MIB_meanTimeToCallSetupService,PM_CCMeasurement.meanTimeToCallSetupService.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanTimeToCallSetupService =
		htonl(PM_CCMeasurement.meanTimeToCallSetupService.getMeanValue());
    
    PM_SET_MIB_VALUE(MIB_meanTimeToLocationUpdateService,PM_CCMeasurement.meanTimeToLocationUpdateService.getMeanValue());
	amsMsg.body.pmData.src.gp10PmData.meanTimeToLocationUpdateService =
		htonl(PM_CCMeasurement.meanTimeToLocationUpdateService.getMeanValue());
    
    PM_SET_MIB_VALUE(MIB_transSubIdentifiedWithIMSI,PM_CCMeasurement.transSubIdentifiedWithIMSI.getValue());
	amsMsg.body.pmData.src.gp10PmData.transSubIdentifiedWithIMSI =
		htonl(PM_CCMeasurement.transSubIdentifiedWithIMSI.getValue());
    
    PM_SET_MIB_VALUE(MIB_imsiDetachProcs,PM_CCMeasurement.imsiDetachProcs.getValue());
	amsMsg.body.pmData.src.gp10PmData.imsiDetachProcs =
		htonl(PM_CCMeasurement.imsiDetachProcs.getValue());
    
    PM_SET_MIB_VALUE(MIB_imsiAttachProcs,PM_CCMeasurement.imsiAttachProcs.getValue());
	amsMsg.body.pmData.src.gp10PmData.imsiAttachProcs =
		htonl(PM_CCMeasurement.imsiAttachProcs.getValue());
    
    PM_SET_MIB_VALUE(MIB_attIncomingInterMSCHDOs,PM_CCMeasurement.attIncomingInterMSCHDOs.getValue());
	amsMsg.body.pmData.src.gp10PmData.attIncomingInterMSCHDOs =
		htonl(PM_CCMeasurement.attIncomingInterMSCHDOs.getValue());
    
    PM_SET_MIB_VALUE(MIB_succIncomingInterMSCHDOs,PM_CCMeasurement.succIncomingInterMSCHDOs.getValue());
	amsMsg.body.pmData.src.gp10PmData.succIncomingInterMSCHDOs =
		htonl(PM_CCMeasurement.succIncomingInterMSCHDOs.getValue());
    
    PM_SET_MIB_VALUE(MIB_attOutgoingInterMSCHDOs,PM_CCMeasurement.attOutgoingInterMSCHDOs.getValue());
	amsMsg.body.pmData.src.gp10PmData.attOutgoingInterMSCHDOs =
		htonl(PM_CCMeasurement.attOutgoingInterMSCHDOs.getValue());
    
    PM_SET_MIB_VALUE(MIB_succOutgoingInterMSCHDOs,PM_CCMeasurement.succOutgoingInterMSCHDOs.getValue());
	amsMsg.body.pmData.src.gp10PmData.succOutgoingInterMSCHDOs =
		htonl(PM_CCMeasurement.succOutgoingInterMSCHDOs.getValue());
    
    PM_SET_MIB_VALUE(MIB_unsuccExternHDOsWithReconnectionPerMSC,PM_CCMeasurement.unsuccExternHDOsWithReconnectionPerMSC.getValue());
	amsMsg.body.pmData.src.gp10PmData.unsuccExternHDOsWithReconnectionPerMSC =
		htonl(PM_CCMeasurement.unsuccExternHDOsWithReconnectionPerMSC.getValue());
    
    PM_SET_MIB_VALUE(MIB_unsuccExternHDOsWithLossOfConnectionPerMSC,PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.getValue());
	amsMsg.body.pmData.src.gp10PmData.unsuccExternHDOsWithLossOfConnectionPerMSC =
		htonl(PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.getValue());

	
    // Not needed for now
    //for(i=0;i<PM_MAX_CAUSES;i++) {
        //pm_percause_mib_entry.cause_code = i;
        //pm_percause_mib_entry.cause_value = PM_CCMeasurement.externalHDOsPerCauseTable.getValue(i);
        //if ((status=oam_setMibTblEntry(MODULE_PM, MIBT_externalHDOsPerCauseEntry, i, (void *)&pm_percause_mib_entry,sizeof(pm_percause_mib_entry)))!=STATUS_OK) {
        //    DBG_ERROR("Unable to set MIB table entry for PM per cause counter\n");
        //}
    //}
	
    
    //Set the timestamp now
    time(&long_time);


    //Deduct 15 minutes or however long we have been collecting
    long_time -= reqd_interval_in_secs;
    if ((status=oam_setMibIntVar(MODULE_PM,MIB_meanPCHAGCHQueueLength,long_time))!=STATUS_OK) { 
        DBG_ERROR("Error setting PM timestamp into MIB, error %d\n",status); 
    }
    
    DBG_LEAVE();
}
/****************************/

/*
 * pm_print_values
 */
int
pm_print_values()
{
	int i;
	printf("****************************************************************************\n");
    printf("meanPCHAGCHQueueLength 							: %d\n", PM_CellMeasurement.meanPCHAGCHQueueLength.getMeanValue());
    printf("attTransOfPagingMessagesThePCH 					: %d\n",PM_CellMeasurement.attTransOfPagingMessagesThePCH.getValue());
    printf("unsuccTransOfPagingMessagesThePCH 				: %d\n",PM_CellMeasurement.unsuccTransOfPagingMessagesThePCH.getValue());
    printf("attImmediateAssingProcs 						: %d\n",PM_CellMeasurement.attImmediateAssingProcs.getValue());
    printf("succImmediateAssingProcs 						: %d\n",PM_CellMeasurement.succImmediateAssingProcs.getValue());
    printf("nbrOfPagesDiscardedFromPCHQueue 				: %d\n",PM_CellMeasurement.nbrOfPagesDiscardedFromPCHQueue.getValue());
    printf("meanDurationOfSuccPagingProcs 					: %d\n",PM_CellMeasurement.meanDurationOfSuccPagingProcs.getMeanValue());
    printf("nbrOfAvailableTCHs 								: %d\n",PM_CellMeasurement.nbrOfAvailableTCHs.getValue());
    printf("meanNbrOfBusyTCHs 								: %d\n",PM_CellMeasurement.meanNbrOfBusyTCHs.getMeanValue());
    printf("maxNbrOfBusyTCHs 								: %d\n",PM_CellMeasurement.maxNbrOfBusyTCHs.getMaxValue());
    printf("meanNbrOfIdleTCHsPerInterferenceBand 			: %d\n",PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.getMeanValue());
    printf("attTCHSeizures 									: %d\n",PM_CellMeasurement.attTCHSeizures.getValue());
    printf("succTCHSeizures 								: %d\n",PM_CellMeasurement.succTCHSeizures.getValue());
    printf("attTCHSeizuresMeetingTCHBlockedState 			: %d\n",PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.getValue());
    printf("allAvailableTCHAllocatedTime 					: %d\n",PM_CellMeasurement.allAvailableTCHAllocatedTime.getValue());
    printf("meanTCHBusyTime 								: %d\n",PM_CellMeasurement.meanTCHBusyTime.getMeanValue());
    printf("meanTCHQueueLength 								: %d\n",PM_CellMeasurement.meanTCHQueueLength.getMeanValue());
    printf("nbrOfLostRadioLinksTCH 							: %d\n",PM_CellMeasurement.nbrOfLostRadioLinksTCH.getValue());
    printf("nbrOfAvailableSDCCHs 							: %d\n",PM_CellMeasurement.nbrOfAvailableSDCCHs.getValue());
    printf("meanNbrOfBusySDCCHs 							: %d\n",PM_CellMeasurement.meanNbrOfBusySDCCHs.getMeanValue());
    printf("maxNbrOfBusySDCCHs 								: %d\n",PM_CellMeasurement.maxNbrOfBusySDCCHs.getMaxValue());
    printf("attSDCCHSeizuresMeetingSDCCHBlockedState 		: %d\n",PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.getValue());
    printf("allAvailableSDCCHAllocatedTime 					: %d\n",PM_CellMeasurement.allAvailableSDCCHAllocatedTime.getValue());
    printf("meanSDCCHQueueLength 							: %d\n",PM_CellMeasurement.meanSDCCHQueueLength.getMeanValue());
    printf("nbrOfLostRadioLinksSDCCH 						: %d\n",PM_CellMeasurement.nbrOfLostRadioLinksSDCCH.getValue());
    printf("relativeTimeDLPowerControlAtMax 				: %d\n",PM_CellMeasurement.relativeTimeDLPowerControlAtMax.getValue());
    printf("relativeTimeULPowerControlAtMax 				: %d\n",PM_CellMeasurement.relativeTimeULPowerControlAtMax.getValue());
    printf("succInternalHDOsIntraCell 						: %d\n",PM_CellMeasurement.succInternalHDOsIntraCell.getValue());
    printf("unsuccInternalHDOsIntraCell 					: %d\n",PM_CellMeasurement.unsuccInternalHDOsIntraCell.getValue());
    printf("nbrOfClassMarkUpdates 							: %d\n",PM_CCMeasurement.nbrOfClassMarkUpdates.getValue());
    printf("attMobileOriginatingCalls 						: %d\n",PM_CCMeasurement.attMobileOriginatingCalls.getValue());
    printf("succMobileOriginatingCalls 						: %d\n",PM_CCMeasurement.succMobileOriginatingCalls.getValue());
    printf("ansMobileOriginatingCalls 						: %d\n",PM_CCMeasurement.ansMobileOriginatingCalls.getValue());
    printf("attMobileTerminatingCalls 						: %d\n",PM_CCMeasurement.attMobileTerminatingCalls.getValue());
    printf("succMobileTerminatingCalls 						: %d\n",PM_CCMeasurement.succMobileTerminatingCalls.getValue());
    printf("ansMobileTerminatingCalls 						: %d\n",PM_CCMeasurement.ansMobileTerminatingCalls.getValue());
    printf("attMobileEmergencyCalls 						: %d\n",PM_CCMeasurement.attMobileEmergencyCalls.getValue());
    printf("succMobileEmergencyCalls 						: %d\n",PM_CCMeasurement.succMobileEmergencyCalls.getValue());
    printf("ansMobileEmergencyCalls 						: %d\n",PM_CCMeasurement.ansMobileEmergencyCalls.getValue());
    printf("attCipheringModeControlProcs 					: %d\n",PM_CCMeasurement.attCipheringModeControlProcs.getValue());
    printf("succCipheringModeControlProcs 					: %d\n",PM_CCMeasurement.succCipheringModeControlProcs.getValue());
    printf("attInterrogationOfHLRsForRouting 				: %d\n",PM_CCMeasurement.attInterrogationOfHLRsForRouting.getValue());
    printf("succInterrogationOfHLRsMSRNObtained 			: %d\n",PM_CCMeasurement.succInterrogationOfHLRsMSRNObtained.getValue());
    printf("succInterrogationOfHLRsCallForwarding 			: %d\n",PM_CCMeasurement.succInterrogationOfHLRsCallForwarding.getValue());
    printf("attOpForMobileOriginatingPointToPointSMs 		: %d\n",PM_CCMeasurement.attOpForMobileOriginatingPointToPointSMs.getValue());
    printf("succOpForMobileOriginatingPointToPointSMs 		: %d\n",PM_CCMeasurement.succOpForMobileOriginatingPointToPointSMs.getValue());
    printf("attOpForMobileTerminatingPointToPointSMs 		: %d\n",PM_CCMeasurement.attOpForMobileTerminatingPointToPointSMs.getValue());
    printf("succOpForMobileTerminatingPointToPointSMs 		: %d\n",PM_CCMeasurement.succOpForMobileTerminatingPointToPointSMs.getValue());
    printf("meanTimeToCallSetupService 						: %d\n",PM_CCMeasurement.meanTimeToCallSetupService.getMeanValue());
    printf("meanTimeToLocationUpdateService 				: %d\n",PM_CCMeasurement.meanTimeToLocationUpdateService.getMeanValue());
    printf("transSubIdentifiedWithIMSI 						: %d\n",PM_CCMeasurement.transSubIdentifiedWithIMSI.getValue());
    printf("imsiDetachProcs 								: %d\n",PM_CCMeasurement.imsiDetachProcs.getValue());
    printf("imsiAttachProcs 								: %d\n",PM_CCMeasurement.imsiAttachProcs.getValue());
    printf("attIncomingInterMSCHDOs 						: %d\n",PM_CCMeasurement.attIncomingInterMSCHDOs.getValue());
    printf("succIncomingInterMSCHDOs 						: %d\n",PM_CCMeasurement.succIncomingInterMSCHDOs.getValue());
    printf("attOutgoingInterMSCHDOs 						: %d\n",PM_CCMeasurement.attOutgoingInterMSCHDOs.getValue());
    printf("succOutgoingInterMSCHDOs 						: %d\n",PM_CCMeasurement.succOutgoingInterMSCHDOs.getValue());
    printf("unsuccExternHDOsWithReconnectionPerMSC 			: %d\n",PM_CCMeasurement.unsuccExternHDOsWithReconnectionPerMSC.getValue());
    printf("unsuccExternHDOsWithLossOfConnectionPerMSC 		: %d\n",PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.getValue());

	printf("-------------- AttImmediateAssingProcsPerCause Table Entries ---------------\n");
    for(i=0;i<PM_CURRENT_MAX_CAUSES;i++) {
        printf("Cause %s : Counter value %d\n",
        	pm_map_cause_code_to_str(pm_map_index_to_cause(i)),
        	PM_CellMeasurement.attImmediateAssingProcsPerCauseTable.getValue(i));
    }



	printf("------------------ SuccImmediateAssingProcsPerCause Table Entries ----------\n");
    for(i=0;i<PM_CURRENT_MAX_CAUSES;i++) {
        printf("Cause %s : Counter value %d\n",
        	pm_map_cause_code_to_str(pm_map_index_to_cause(i)),
        	PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.getValue(i));
    }

        

	/*
	 * Not needed for now
    for(i=0;i<PM_MAX_CAUSES;i++) {
        printf(" MIBT_externalHDOsPerCauseEntry no %d = %d\n",i,PM_CCMeasurement.externalHDOsPerCauseTable.getValue(i));
    }
	*/

	printf("****************************************************************************\n");

	return(0);
}


/*
 * pm_map_cause_to_index
 */
int
pm_map_cause_to_index(CauseCode cause)
{
	int index= PM_MAX_CAUSES;

	switch(cause) {
	case  160: index = 0; break;
		/* Emergency call */
	case 192: index = 1; break;
		/*	Call re-establishment; TCH/F was in use */
	case 128: index = 2; break;
		/* Answer to paging */
	case 16: index = 3; break;
		/* Answer to paging */
	case 32: index = 4; break;
		/* Answer to paging */
	case 224: index = 5; break;
		/* Originating call and TCH/F is needed */
	case 0: index = 6; break;
		/* Location updating */
	case 12: index = 7; break;
		/* Other procedures which can be completed with
			an SDCCH and the network sets NECI bit to 1 */
	default: return PM_CURRENT_MAX_CAUSES;
	}
	return(index);
}


/*
 * pm_map_index_to_cause
 */
CauseCode 
pm_map_index_to_cause(int index)
{
	CauseCode cause;

	switch(index) {
	case  0: cause = 160; break;
		/* Emergency call */
	case 1: cause = 192; break;
		/*	Call re-establishment; TCH/F was in use */
	case 2: cause = 128; break;
		/* Answer to paging : Full Rate only*/
	case 3: cause = 16; break;
		/* Answer to paging : SDCCH only*/
	case 4: cause = 32; break;
		/* Answer to paging : Dual rate mobile*/
	case 5: cause = 224; break;
		/* Originating call and TCH/F is needed */
	case 6: cause = 0; break;
		/* Location updating */
	case 7: cause = 12; break;
		/* Other procedures which can be completed with
			an SDCCH and the network sets NECI bit to 1 */
	}
	return(cause);
}


/*
 * pm_map_cause_code_to_str
 */
char *
pm_map_cause_code_to_str(CauseCode cause)
{
	static char cause_str[100];

	switch(cause) {
	case  160: strncpy(cause_str,"Emergeny call",100); break;
		/* Emergency call */
	case 192: strncpy(cause_str,"Call re-establishment; TCH/F was in use",100); break;
		/*	Call re-establishment; TCH/F was in use */
	case 128: strncpy(cause_str,"Answer to paging : Full Rate only",100);  break;
		/* Answer to paging : Full Rate Only*/
	case 16: strncpy(cause_str,"Answer to paging : SDCCH only",100);  break;
		/* Answer to paging : SDCCH Only*/
	case 32: strncpy(cause_str,"Answer to paging : Dual Rate Mobile",100);  break;
		/* Answer to paging */
	case 224: strncpy(cause_str,"Originating call and TCH/F is needed",100);  break;
		/* Originating call and TCH/F is needed */
	case 0: strncpy(cause_str,"Location updating",100);  break;
		/* Location updating */
	case 12: strncpy(cause_str,"Procs completed with SDCCH and NECI bit = 1",100);  break;
		/* Other procedures which can be completed with
			an SDCCH and the network sets NECI bit to 1 */
	default : strncpy(cause_str,"Unknown",100);break;
	}
	return(cause_str);
}
