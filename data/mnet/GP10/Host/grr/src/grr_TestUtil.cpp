/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_TESTUTIL_CPP__
#define __GRR_TESTUTIL_CPP__

#include "grr\grr_head.h"

//Print read-out gsm mib data
void grr_PrintGsmOamData(int selection)
{
     if (selection==0)
     {
         grr_PrintIntBtsBasicPackage();
         grr_PrintIntBtsCellAllocationTable();
         grr_PrintIntBtsBcchConfigurationPackage();
         grr_PrintIntBtsOptionsPackage();
         grr_PrintIntFirstTransceiverPackage();
         grr_PrintIntSecondTransceiverPackage();
         grr_PrintIntFirstTrxRadioCarrierPackage();
         grr_PrintIntSecondTrxRadioCarrierPackage();
         grr_PrintIntAdjacentCellHandoverPackage();
         grr_PrintIntAdjacentCellReselectPackage();
         grr_PrintIntT31xxPackage();
    } else
    {
         grr_PrintBtsBasicPackage(&grr_OamData.btsBasicPackage);
         grr_PrintBtsOptionsPackage(&grr_OamData.btsOptionsPackage);
         grr_PrintFirstTransceiverPackage(&grr_OamData.transceiverPackage[0]);
         grr_PrintSecondTransceiverPackage(&grr_OamData.transceiverPackage[1]);
         grr_PrintFirstRadioCarrierPackage(&grr_OamData.radioCarrierPackage[0]);
         grr_PrintSecondRadioCarrierPackage(&grr_OamData.radioCarrierPackage[1]);
         grr_PrintT31xx(&grr_OamData.t31xx);
         grr_PrintAdjacentCellPackage(&grr_OamData.adjacentCellPackage);
    }

    grr_PrintHandoverControlPackage(&grr_OamData.handoverControlPackage);
    grr_PrintPowerControlPackage(&grr_OamData.powerControlPackage);
    grr_PrintIntGrrConfigData();
}

void grr_PrintCellAllocationTable(Table *tbl)
{
	int i;
	CellAllocationEntry *entry = (CellAllocationEntry *) tbl;
	printf("%s\t%s", "Index", "Cell Allociation");
	
	for (i = 0; i< CellAllocationTableLimit; i++)
	{
		
		printf("%d\t\t%d", i+1, (entry+i)->cellAllocation);
	}
}

void grr_PrintPlmnPermittedTable(Table *tbl)
{
	int i;
	PlmnPermittedEntry *entry = (PlmnPermittedEntry *) tbl;
	printf("%s\t%s", "Index", "Plmn Permitted");
	
	for (i = 0; i< PlmnPermittedTableLimit; i++)
	{
		
		printf("%d\t\t%d", i+1, (entry+i)->plmnPermitted);
	}
}

void grr_PrintNotAllowedAccessClassTable(Table *tbl)
{
	int i;
	NotAllowedAccessClassEntry *entry = (NotAllowedAccessClassEntry *) tbl;
	printf("%s\t%s", "Index", "Not Allowed Access Class");
	
	for (i = 0; i< NotAllowedAccessClassTableLimit ; i++)
	{
		
		printf("%d\t\t%d", i+1, (entry+i)->notAllowedAccessClass);
	}
}

void grr_PrintChannelTable(Table *tbl)
{
	int i;
	ChannelEntry *entry = (ChannelEntry *) tbl;
	printf("%s,%s,%s,%s,%s,%s,%s", "Idx", "ID", "Comb", "fusage", "tch", "a.state", "o.state");
	
	for (i = 0; i<ChannelTableLimit ; i++)
	{
		
		printf("%d,%d,%d,%d,%d,%d,%d", i+1, (entry+i)->channelID,
			(entry+i)->channelCombination,
			(entry+i)->channelFrequencyUsage,
			(entry+i)->channelTsc,
			(entry+i)->channelAdministrativeState,
			(entry+i)->channelOperationalState);
	}
}

void grr_PrintCarrierFrequencyTable(Table *tbl)
{
	int i;
	CarrierFrequencyEntry *entry = (CarrierFrequencyEntry *) tbl;
	printf("%s\t%s", "Index", "Carrier Frequency");
	
	for (i = 0; i< CarrierFrequencyTableLimit; i++)
	{
		
		printf("%d\t\t%d", i+1, (entry+i)->carrierFrequency);
	}
}

void grr_PrintAdjCellHandoverTable(Table *tbl)
{
	int i;
	AdjCell_HandoverEntry *entry = (AdjCell_HandoverEntry *) tbl;
	printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", "Idx", "ID", "mcc", "mnc", "lac", "ci", "bcch",
		"ncc", "cid", "sync", "hoPl", "hoM", "msTx", "rxLev");
	
	for (i = 0; i< HandoverTableLimit; i++)
	{
		
		printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
			i+1, 
			(entry+i)->adjCell_handoverCellID,
			(entry+i)->adjCell_mcc,
			(entry+i)->adjCell_mnc,
			(entry+i)->adjCell_lac,
			(entry+i)->adjCell_ci,
			(entry+i)->adjCell_bCCHFrequency,
			(entry+i)->adjCell_ncc,
			(entry+i)->adjCell_cid,
			(entry+i)->adjCell_synchronized,
			(entry+i)->adjCell_hoPriorityLevel,
			(entry+i)->adjCell_hoMargin,
			(entry+i)->adjCell_msTxPwrMaxCell,
			(entry+i)->adjCell_rxLevMinCell);
	}
}

void grr_PrintAdjCellReselectionTable(Table *tbl)
{
	int i;
	AdjCell_ReselectionEntry *entry = (AdjCell_ReselectionEntry *) tbl;
	printf("%s\t%s\t%s", "Index", "ID", "Reselection BCCH Frequency");
	
	for (i = 0; i< ReselectionTableLimit; i++)
	{
		
		printf("%d\t\t%d\t\t%d", i+1, (entry+i)->adjCell_reselectionCellID,
			(entry+i)->adjCell_reselectionBCCHFrequency);
	}
}


void grr_PrintBtsBasicPackage(BtsBasicPackage *btsBasicPackage)
{
	printf("\nBts Bacic Package");

	char *cp;
	
	printf("bts_ncc= %d",	btsBasicPackage->bts_ncc);
	printf("bts_cid= %d",	btsBasicPackage->bts_cid);
	printf("btsID= %d",	btsBasicPackage->btsID);
	
	grr_PrintCellAllocationTable(btsBasicPackage->cellAllocationTable);
	
	printf("gsmdcsIndicator= %d",	btsBasicPackage->gsmdcsIndicator);
	cp = (char *) &btsBasicPackage->bts_mcc;
	printf("bts_mcc= %d%d%d",	cp[0], cp[1], cp[2]);
	cp = (char *) &btsBasicPackage->bts_mnc;
	printf("bts_mnc= %d%d%x",	cp[0], cp[1], cp[2]);	// kevinlim 05/11/01

	printf("bts_lac= %d",	btsBasicPackage->bts_lac);
	printf("bts_ci= %d",	btsBasicPackage->bts_ci);
	printf("cellReselectHysteresis= %d",	btsBasicPackage->cellReselectHysteresis);
	printf("ny1= %d",	btsBasicPackage->ny1);
	
	grr_PrintPlmnPermittedTable (btsBasicPackage->plmnPermittedTable);
	
	printf("radioLinkTimeout= %d",	btsBasicPackage->radioLinkTimeout);
	printf("relatedTranscoder= %d",	btsBasicPackage->relatedTranscoder);
	printf("rxLevAccessMin= %d",	btsBasicPackage->rxLevAccessMin);
	printf("bts_administrativeState= %d",	btsBasicPackage->bts_administrativeState);

	printf("bts_alarmStatus= %d",	btsBasicPackage->bts_alarmStatus);
	printf("bts_operationalState= %d",	btsBasicPackage->bts_operationalState);
	printf("maxNumberRetransmissions= %d",	btsBasicPackage->maxNumberRetransmissions);
	printf("mSTxPwrMaxCCH= %d",	btsBasicPackage->mSTxPwrMaxCCH);
	printf("numberOfSlotsSpreadTrans= %d",	btsBasicPackage->numberOfSlotsSpreadTrans);
	printf("noOfBlocksForAccessGrant= %d",	btsBasicPackage->noOfBlocksForAccessGrant);
	printf("noOfMultiframesBetweenPaging= %d",btsBasicPackage->noOfMultiframesBetweenPaging);
	
}


void grr_PrintBtsOptionsPackage(BtsOptionsPackage *btsOptionsPackage)
{
	printf("\nBts Optioons Package");
	
	printf("allowIMSIAttachDetach= %d",	btsOptionsPackage->allowIMSIAttachDetach);
	printf("callReestablishmentAllowed= %d",	btsOptionsPackage->callReestablishmentAllowed);
	printf("cellBarred= %d",	btsOptionsPackage->cellBarred);
	printf("dtxDownlink= %d",	btsOptionsPackage->dtxDownlink);
	printf("dtxUplink= %d",	btsOptionsPackage->dtxUplink);
	printf("emergencyCallRestricted= %d",	btsOptionsPackage->emergencyCallRestricted);
	
	grr_PrintNotAllowedAccessClassTable(btsOptionsPackage->notAllowedAccessClassTable);
	
	printf("timerPeriodicUpdateMS= %d",	btsOptionsPackage->timerPeriodicUpdateMS);
	printf("maxQueueLength= %d",	btsOptionsPackage->maxQueueLength);
	printf("msPriorityUsedInQueueing= %d",	btsOptionsPackage->msPriorityUsedInQueueing);
	printf("timeLimitCall= %d",	btsOptionsPackage->timeLimitCall);
	printf("timeLimitHandover= %d",	btsOptionsPackage->timeLimitHandover);
}

void grr_PrintTransceiverPackage(TransceiverPackage *transceiverPackage)
{
	
	printf("basebandTransceiverID= %d",	transceiverPackage->basebandTransceiverID);
	
	printf("relatedRadioCarrier= %d",	transceiverPackage->relatedRadioCarrier);
	printf("basebandAdministrativeState= %d",	transceiverPackage->basebandAdministrativeState);
	printf("basebandAlarmStatus= %d",	transceiverPackage->basebandAlarmStatus);
	printf("basebandOperationalState= %d",	transceiverPackage->basebandOperationalState);
	
	grr_PrintChannelTable(transceiverPackage->channelTable);
}

void grr_PrintFirstTransceiverPackage(FirstTransceiverPackage *firstTransceiverPackage)
{
	
	printf("\nFirst Transceiver Package");
	grr_PrintTransceiverPackage(firstTransceiverPackage);
}

void grr_PrintSecondTransceiverPackage(SecondTransceiverPackage *secondTransceiverPackage)
{
	
	printf("\nSecond Transceiver Package");
	grr_PrintTransceiverPackage(secondTransceiverPackage);
}

void grr_PrintRadioCarrierPackage(RadioCarrierPackage *radioCarrierPackage)
{
	
	grr_PrintCarrierFrequencyTable(radioCarrierPackage->carrierFrequencyList);
	
	printf("powerClass= %d",	radioCarrierPackage->powerClass);
	printf("radioCarrierID= %d",	radioCarrierPackage->radioCarrierID);
	printf("txPwrMaxReduction= %d",	radioCarrierPackage->txPwrMaxReduction);
	printf("carrier_administrativeState= %d",	radioCarrierPackage->carrier_administrativeState);
	printf("carrier_alarmStatus= %d",	radioCarrierPackage->carrier_alarmStatus);
	printf("carrier_operationalState= %d",	radioCarrierPackage->carrier_operationalState);
	
}

void grr_PrintFirstRadioCarrierPackage(FirstRadioCarrierPackage *firstRadioCarrierPackage)
{
	
	printf("\nFirst Radio Carrier Package");
	grr_PrintRadioCarrierPackage(firstRadioCarrierPackage);
}

void grr_PrintSecondRadioCarrierPackage(SecondRadioCarrierPackage *secondRadioCarrierPackage)
{
	
	printf("\nSecond Radio Carrier Package");
	grr_PrintRadioCarrierPackage(secondRadioCarrierPackage);
}


void grr_PrintT31xx(T31xx *t31xx)
{
	
	printf("\nT31xx Timers");
	
	printf("t3101= %d",	t31xx->t3101);
	printf("t3103= %d",	t31xx->t3103);
	printf("t3105= %d",	t31xx->t3105);
	printf("t3107= %d",	t31xx->t3107);
	printf("t3109= %d",	t31xx->t3109);
	printf("t3111= %d",	t31xx->t3111);
	printf("t3113= %d",	t31xx->t3113);
	
}

void grr_PrintAdjacentCellPackage(AdjacentCellPackage *adjacentCellPackage)
{
	printf("\nAdjacent Cell Package");
	
	grr_PrintAdjCellHandoverTable(adjacentCellPackage->adjCell_handoverTable);
	grr_PrintAdjCellReselectionTable(adjacentCellPackage->adjCell_reselectionTable);
}


void grr_PrintHandoverControlPackage(HandoverControlPackage *handoverControlPackage)
{
     printf("\nHandover Control Package");
	
     printf("handoverControlID= %d",handoverControlPackage->handoverControlID);
     printf("enableOptHandoverProcessing= %d",handoverControlPackage->enableOptHandoverProcessing);
	
     printf("%x:%x", handoverControlPackage->hoAveragingAdjCellParam, &mnetMib.hoAveragingAdjCellParam);

     printf("hoAveragingAdjCellParamHreqave= %d",handoverControlPackage->hoAveragingAdjCellParam->hreqave);
     printf("hoAveragingAdjCellParamHreqt= %d",handoverControlPackage->hoAveragingAdjCellParam->hreqt);
     printf("hoAveragingAdjCellParamWeighting= %d",handoverControlPackage->hoAveragingAdjCellParam->weighting);
	
     printf("hoAveragingDistParamHreqave= %d",handoverControlPackage->hoAveragingDistParam->hreqave);
     printf("hoAveragingDistParamHreqt= %d",handoverControlPackage->hoAveragingDistParam->hreqt);
	
     printf("hoAveragingLevParamHreqave= %d",handoverControlPackage->hoAveragingLevParam->hreqave);
     printf("hoAveragingLevParamHreqt= %d",handoverControlPackage->hoAveragingLevParam->hreqt);
     printf("hoAveragingLevParamWeighting= %d",handoverControlPackage->hoAveragingLevParam->weighting);
	
     printf("hoAveragingQualParamHreqave= %d",handoverControlPackage->hoAveragingQualParam->hreqave);
     printf("hoAveragingQualParamHreqt= %d",handoverControlPackage->hoAveragingQualParam->hreqt);
     printf("hoAveragingQualParamWeighting= %d",handoverControlPackage->hoAveragingQualParam->weighting);
	
     printf("hoMarginDef= %d",handoverControlPackage->hoMarginDef);

     printf("hoThresholdDistParamTimeadv= %d",handoverControlPackage->hoThresholdDistParam->timeadv);
     printf("hoThresholdDistParamP8= %d",handoverControlPackage->hoThresholdDistParam->p8);
     printf("hoThresholdDistParamN8= %d",handoverControlPackage->hoThresholdDistParam->n8);
	
     printf("hoThresholdInterfaceParamRxLevelUL= %d",handoverControlPackage->hoThresholdInterfaceParam->rxLevelUL);
     printf("hoThresholdInterfaceParamRxLevelDL= %d",handoverControlPackage->hoThresholdInterfaceParam->rxLevelDL);
     printf("hoThresholdInterfaceParamPx= %d",handoverControlPackage->hoThresholdInterfaceParam->px);
     printf("hoThresholdInterfaceParamNx= %d",handoverControlPackage->hoThresholdInterfaceParam->nx);
	
     printf("hoThresholdLevParamRxLevelUL= %d",handoverControlPackage->hoThresholdLevParam->rxLevelUL);
     printf("hoThresholdLevParamRxLevelDL= %d",handoverControlPackage->hoThresholdLevParam->rxLevelDL);
     printf("hoThresholdLevParamPx= %d",handoverControlPackage->hoThresholdLevParam->px);
     printf("hoThresholdLevParamNx= %d",handoverControlPackage->hoThresholdLevParam->nx);
 
     printf("hoThresholdQualParamRxQualUL= %d",handoverControlPackage->hoThresholdQualParam->rxQualUL);
     printf("hoThresholdQualParamRxQualDL= %d",handoverControlPackage->hoThresholdQualParam->rxQualDL);
     printf("hoThresholdQualParamPx= %d",handoverControlPackage->hoThresholdQualParam->px);
     printf("hoThresholdQualParamNx= %d",handoverControlPackage->hoThresholdQualParam->nx);
	
     printf("interferenceAveragingParamAveragingPeriod= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamAveragingPeriod);
     printf("interferenceAveragingParamThresholdBoundary0= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary0);
     printf("interferenceAveragingParamThresholdBoundary1= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary1);
     printf("interferenceAveragingParamThresholdBoundary2= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary2);
     printf("interferenceAveragingParamThresholdBoundary3= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary3);
     printf("interferenceAveragingParamThresholdBoundary4= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary4);
     printf("interferenceAveragingParamThresholdBoundary5= %d",handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary5);
	
     printf("msTxPwrMaxCellDef= %d",handoverControlPackage->mxTxPwrMaxCellDef);
     printf("rxLevMinCellDef= %d",handoverControlPackage->rxLevMinCellDef);
}

void grr_PrintPowerControlPackage(PowerControlPackage *powerControlPackage)
{
     printf("\nPower Control Package");
	
     printf("pcAveragingLevHreqave= %d",powerControlPackage->pcAveragingLev->hreqave);
     printf("pcAveragingLevHreqt= %d",powerControlPackage->pcAveragingLev->hreqt);
     printf("pcAveragingLevWeighting= %d",powerControlPackage->pcAveragingLev->weighting);
	
     printf("pcAveragingQualHreqave= %d",powerControlPackage->pcAveragingQual->hreqave);
     printf("pcAveragingQualHreqt= %d",powerControlPackage->pcAveragingQual->hreqt);
     printf("pcAveragingQualWeighting= %d",powerControlPackage->pcAveragingQual->weighting);
	
     printf("pcLowerThresholdLevParamRxLevelUL= %d",powerControlPackage->pcLowerThresholdLevParam->rxLevelUL);
     printf("pcLowerThresholdLevParamRxLevelDL= %d",powerControlPackage->pcLowerThresholdLevParam->rxLevelDL);
     printf("pcLowerThresholdLevParamPx= %d",powerControlPackage->pcLowerThresholdLevParam->px);
     printf("pcLowerThresholdLevParamNx= %d",powerControlPackage->pcLowerThresholdLevParam->nx);
	
     printf("pcLowerThresholdQualParamRxQualUL= %d",powerControlPackage->pcLowerThresholdQualParam->rxQualUL);
     printf("pcLowerThresholdQualParamRxQualDL= %d",powerControlPackage->pcLowerThresholdQualParam->rxQualDL);
     printf("pcLowerThresholdQualParamPx= %d",powerControlPackage->pcLowerThresholdQualParam->px);
     printf("pcLowerThresholdQualParamNx= %d",powerControlPackage->pcLowerThresholdQualParam->nx);
	
     printf("pcUpperThresholdLevParamRxLevelUL= %d",powerControlPackage->pcUpperThresholdLevParam->rxLevelUL);
     printf("pcUpperThresholdLevParamRxLevelDL= %d",powerControlPackage->pcUpperThresholdLevParam->rxLevelDL);
     printf("pcUpperThresholdLevParamPx= %d",powerControlPackage->pcUpperThresholdLevParam->px);
     printf("pcUpperThresholdLevParamNx= %d",powerControlPackage->pcUpperThresholdLevParam->nx);
	
     printf("pcUpperThresholdQualParamRxQualUL= %d",powerControlPackage->pcUpperThresholdQualParam->rxQualUL);
     printf("pcUpperThresholdQualParamRxQualDL= %d",powerControlPackage->pcUpperThresholdQualParam->rxQualDL);
     printf("pcUpperThresholdQualParamPx= %d",powerControlPackage->pcUpperThresholdQualParam->px);
     printf("pcUpperThresholdQualParamNx= %d",powerControlPackage->pcUpperThresholdQualParam->nx);
     printf("powerControlInterval= %d",powerControlPackage->powerControlInterval);
     printf("powerIncrStepSize= %d",powerControlPackage->powerIncrStepSize);
     printf("powerRedStepSize= %d",powerControlPackage->powerRedStepSize);
}

void grr_PrintIntBtsBasicPackage(void)
{
     //Administrative states
     printf("Bts Administrative State(%d,%d) Operational State(%d,%d)\n",
             OAMgrr_BTS_AMSTATE,
             grr_OamData.btsBasicPackage.bts_administrativeState,
             OAMgrr_BTS_OPSTATE,
             grr_OamData.btsBasicPackage.bts_operationalState
        );

     printf("Base Station Identity Code: BSIC(%x,%x) NCC(%d,%d) BCC(%d,%d)\n",
             OAMgrr_POCKETBTS_BSIC,
             ((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc<<3)| 
             (unsigned char)(grr_OamData.btsBasicPackage.bts_cid)),
             OAMgrr_NCC,
             ((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc)),
             OAMgrr_BCC,
             ((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
           );

     printf("Global Cell ID: MCC(%d,%d,%d)(%d,%d,%d) MNC(%d,%d)(%d,%d) LAC(%x,%x) CI ie btsId (%d,%d)\n",
             OAMgrr_MCC(0), OAMgrr_MCC(1), OAMgrr_MCC(2),
             ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mcc))[0],
             ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mcc))[1],
             ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mcc))[2],
             OAMgrr_MNC(0), OAMgrr_MNC(1),
             ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[0],
             ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[1],
             OAMgrr_LAC, 
             ((short)(grr_OamData.btsBasicPackage.bts_lac)),
	       OAMgrr_CELLID,
	       ((unsigned int)(grr_OamData.btsBasicPackage.btsID))
           ); 

     printf("Cell Reselect Hysteresis in 2dB step: (%d, %d)\n", 
             OAMgrr_CELL_RESEL_HYSTERESIS, 
             ((unsigned char)(grr_OamData.btsBasicPackage.cellReselectHysteresis))
         );

     printf("Max Number of Repetitions of Physical Info.(ny1):(%d, %d)\n", 
             OAMgrr_NY1,
             ((unsigned char)(grr_OamData.btsBasicPackage.ny1))
           );


     printf("Radio Indicator: (%d, %d)\n", 
             OAMgrr_GSMDCS_INDICATOR,
 	     grr_OamData.btsBasicPackage.gsmdcsIndicator
         );

     printf("NccPermitted Setting 0th(%d,%d) 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d) 4th(%d,%d) 5th(%d,%d) 6th(%d,%d) 7th(%d,%d)\n",
             OAMgrr_NCC_PERMITTED(0), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[0]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(1), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[1]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(2), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[2]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(3), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[3]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(4), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[4]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(5), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[5]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(6), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[6]. 
             plmnPermitted),
             OAMgrr_NCC_PERMITTED(7), 
             (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[7]. 
             plmnPermitted)
         );

     printf("Radio Link Timeout in 4 SACCH frame unit:(%d,%d)\n",
             OAMgrr_RADIO_LINK_TIMEOUT, 
             ((unsigned char)(grr_OamData.btsBasicPackage.radioLinkTimeout))
         );

     printf("Minimum Receive Level Access:(%d, %d)\n",
             OAMgrr_MS_RELEV_ACCESS_MIN,
             ((unsigned char)(grr_OamData.btsBasicPackage.rxLevAccessMin))
           );

}

void grr_PrintIntBtsBcchConfigurationPackage(void)
{
 
     printf("Max Number Retransmission(1 2 4 7) on RACH:(%d, %d)\n", 
             OAMgrr_RA_RETRY_MAX, 
             ((unsigned char)(grr_OamData.btsBasicPackage.maxNumberRetransmissions))
         );
  
     printf("MS Tx Power Max CCCH:(%d, %d)\n",
             OAMgrr_MS_TX_PWR_MAX_CCH,
             ((unsigned char)(grr_OamData.btsBasicPackage.mSTxPwrMaxCCH))
           );

     printf("No. of Blocks for Access Grant:(%d, %d)\n", 
             OAMgrr_BS_AG_BLK_RES,
             ((unsigned char)(grr_OamData.btsBasicPackage.noOfBlocksForAccessGrant))
           );

  
     printf("No. of Multi-frames Between Paging:(%d, %d)\n",
             OAMgrr_BS_PA_MFRMS,
             ((unsigned char)(grr_OamData.btsBasicPackage.noOfMultiframesBetweenPaging))
           );

     printf("No. of Slots Spread Trans (Tx Integer):(%d, %d)\n", 
             OAMgrr_RA_TX_INTEGER, 
             ((unsigned char)(grr_OamData.btsBasicPackage.numberOfSlotsSpreadTrans))
         );

     printf("Cell Reselect Offset:(%d, %d)\n", 
             OAMgrr_CELL_RESELECT_OFFSET,
             ((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][3]))
         );

     printf("Temporary Offset:(%d, %d)\n", 
             OAMgrr_TEMP_OFFEST,
             ((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][4]))
         );

     printf("Penalty Offset:(%d, %d)\n",  
             OAMgrr_PENALTY_TIME,
             ((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][5]))
         );

     printf("Cell Bar Qualify:(%d, %d)\n",  
             OAMgrr_CELL_BAR_QUALIFY,
             grr_OamData.grrConfigData.rm_nim[0][6]
         );
}

void grr_PrintIntBtsCellAllocationTable(void)
{
     printf("1st ARFCN in cell allocation table:(%d,%d)\n",
             OAMgrr_CA_ARFCN(0),
 	     (((CellAllocationEntry*)(grr_OamData.btsBasicPackage.cellAllocationTable))[0].cellAllocation)
         );
     printf("2nd ARFCN in cell allocation table:(%d,%d)\n",
             OAMgrr_CA_ARFCN(1),
 	     (((CellAllocationEntry*)(grr_OamData.btsBasicPackage.cellAllocationTable))[1].cellAllocation)
         );
     printf("3rd ARFCN in cell allocation table:(%d,%d)\n",
             OAMgrr_CA_ARFCN(2),
 	     (((CellAllocationEntry*)(grr_OamData.btsBasicPackage.cellAllocationTable))[2].cellAllocation)
         );
}

void grr_PrintIntBtsOptionsPackage(void)
{
     printf("Use  IMSI attach/detach procedure? Setting(%d, %d)\n",
             OAMgrr_ATT_STATE,
             ((T_CNI_RIL3_ATT)(grr_OamData.btsOptionsPackage.allowIMSIAttachDetach))
         );

     printf("Cell Barred? (%d, %d) overLoadBarring(%d,%d) currentBarringState(%d)\n", 
             OAMgrr_RA_CELL_BARRED_STATE, 
             ((T_CNI_RIL3_CELL_BARRED_ACCESS)(grr_OamData.btsOptionsPackage.cellBarred)),
              OAMgrr_OVERLOAD_CELLBAR
         );

     printf("Restablishment Allowed? (%d, %d)\n", 
             OAMgrr_RA_REST_ALLOWED_STATE, 
             ((T_CNI_RIL3_REESTABLISHMENT_ALLOWED) 
             (grr_OamData.btsOptionsPackage.callReestablishmentAllowed))
         );

     printf("Downlink DTX is available in the BTS(downlink)? (%d, %d)\n",
             OAMgrr_MS_DNLINK_DTX_STATE, 
             ((int)(grr_OamData.btsOptionsPackage.dtxDownlink))
         );

     printf("Discontinuous Transmission (DTX) mode to be used by the Mobile Stations(uplink): (%d, %d)\n",
             OAMgrr_MS_UPLINK_DTX_STATE, 
             ((T_CNI_RIL3_DTX_BCCH)(grr_OamData.btsOptionsPackage.dtxUplink))
         );

     printf("Is IMSI required for Emergency calls? (%d, %d)\n", 
             OAMgrr_IMSI_REQUIRED_FOR_EC, 
             ((unsigned char)(grr_OamData.grrConfigData.rm_nim[1][0]))
         );

     printf("Emergency calls restricted to MSs belonging to access classes from 11 to 15? (%d, %d)\n", 
             OAMgrr_RA_EC_ALLOWED_STATE, 
             ((T_CNI_RIL3_EMERGENCY_CALL_ALLOWED) 
             (grr_OamData.btsOptionsPackage.emergencyCallRestricted))
         );

     printf("MS Access Classes not allowed to access the cell: c0(%d,%d) c1(%d,%d) c2(%d,%d) c3(%d,%d) c4(%d,%d)\n",
             OAMgrr_RA_AC_ALLOWED_STATE(0), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[0].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(1), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[1].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(2), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[2].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(3), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[3].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(4),
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[4].notAllowedAccessClass))
         );

     printf("MS Access Classes not allowed to access the cell: c5(%d,%d) c6(%d,%d) c7(%d,%d) c8(%d,%d) c9(%d,%d)\n",
             OAMgrr_RA_AC_ALLOWED_STATE(5), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[5].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(6), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[6].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(7), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[7].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(8), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[8].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(9), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[9].notAllowedAccessClass))
          );

     printf("MS Access Classes not allowed to access the cell: c10(%d,%d) c11(%d,%d) c12(%d,%d) c13(%d,%d) c14(%d,%d) c15(%d,%d)\n",
             OAMgrr_RA_AC_ALLOWED_STATE(10), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[10].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(11), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[11].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(12), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[12].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(13), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[13].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(14), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[14].notAllowedAccessClass)),
             OAMgrr_RA_AC_ALLOWED_STATE(15), 
             ((unsigned char)(((NotAllowedAccessClassEntry*)
             (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[15].notAllowedAccessClass))
         );

     printf("Periodic Location Update Interval (T3212 Deci Hours): (%d, %d)\n",
             OAMgrr_T3212, 
             ((unsigned char)(grr_OamData.btsOptionsPackage.timerPeriodicUpdateMS))
           );

}

void grr_PrintIntFirstTransceiverPackage(void)
{
     int i;

     //Trx 0 Administrative state
     printf("Transceriver 0 Administrative State: (%d,%d)\n",
             OAMgrr_TRX_AMSTATE(0),
             grr_OamData.transceiverPackage[0].basebandAdministrativeState
         );

     //Trx 0 Operational state
     printf("Transceiver 0 Operational State: (%d,%d)\n",
             OAMgrr_TRX_OPSTATE(0), 
             grr_OamData.transceiverPackage[0].basebandOperationalState
         );

     for (i=0;i<8;i++)
     {
     printf("Transceiver 0 TS-%d States: i, Admin State(%d,%d) Opera State(%d,%d) Chan Combination(%d,%d) TSC(%d,%d)\n",
             OAMgrr_CHN_AMSTATE(0,i),
             ((ChannelEntry*)
             (grr_OamData.transceiverPackage[0].channelTable))[i].channelAdministrativeState,
             OAMgrr_CHN_OPSTATE(0,i),
             ((EnableDisable)(((ChannelEntry*)
             (grr_OamData.transceiverPackage[0].channelTable))[i].channelOperationalState)),
             OAMgrr_TRX_SLOT_COMB(0,i),
             ((unsigned char)(((ChannelEntry*) 
             (grr_OamData.transceiverPackage[0].channelTable))[i].channelCombination)),
             OAMgrr_TRX_SLOT_TSC(0,i),
             ((unsigned char)(((ChannelEntry*) 
             (grr_OamData.transceiverPackage[0].channelTable))[i].channelTsc))
        );
     }
}

void grr_PrintIntSecondTransceiverPackage(void)
{
     int i;

     //Trx 0 Administrative state
     printf("Transceriver 1 Administrative State: (%d,%d)\n",
             OAMgrr_TRX_AMSTATE(1),
             grr_OamData.transceiverPackage[1].basebandAdministrativeState
         );

     //Trx 0 Operational state
     printf("Transceiver 1 Operational State: (%d,%d)\n",
             OAMgrr_TRX_OPSTATE(1), 
             grr_OamData.transceiverPackage[1].basebandOperationalState
         );

     for (i=0;i<8;i++)
     {
     printf("Transceiver 1 TS-%d States: i, Admin State(%d,%d) Opera State(%d,%d) Chan Combination(%d,%d) TSC(%d,%d)\n",
             OAMgrr_CHN_AMSTATE(1,i),
             ((ChannelEntry*)
             (grr_OamData.transceiverPackage[1].channelTable))[i].channelAdministrativeState,
             OAMgrr_CHN_OPSTATE(1,i),
             ((EnableDisable)(((ChannelEntry*)
             (grr_OamData.transceiverPackage[1].channelTable))[i].channelOperationalState)),
             OAMgrr_TRX_SLOT_COMB(1,i),
             ((unsigned char)(((ChannelEntry*) 
             (grr_OamData.transceiverPackage[1].channelTable))[i].channelCombination)),
             OAMgrr_TRX_SLOT_TSC(1,i),
             ((unsigned char)(((ChannelEntry*) 
             (grr_OamData.transceiverPackage[1].channelTable))[i].channelTsc))
        );
     }
}

void grr_PrintIntT31xxPackage(void)
{
     printf("Immediate Assignment(T3101): (%d,%d)\n",
             OAMgrr_T3101, grr_OamData.t31xx.t3101
         );
     printf("Handover Command(T3103): (%d,%d)\n",
             OAMgrr_T3103, grr_OamData.t31xx.t3103
         );
     printf("physical Information(T3105): (%d,%d)\n",  
             OAMgrr_T3105, grr_OamData.t31xx.t3105
         );
     printf("Assignment Command(T3107): (%d,%d)\n",
             OAMgrr_T3107, grr_OamData.t31xx.t3107
         );
     printf("lower layer failure(T3109): (%d,%d)\n",
	     OAMgrr_T3109, grr_OamData.t31xx.t3109
         );
     printf("Channel Activation Delay(T3111): (%d,%d)\n",
             OAMgrr_T3111, grr_OamData.t31xx.t3111
         );
     printf("Paging Messages(T3113): (%d,%d)\n",
             OAMgrr_T3111, grr_OamData.t31xx.t3113);
}

void grr_PrintIntFirstTrxRadioCarrierPackage(void)
{
     printf("Trx-0 Radio Carrier Configuration: pwrClass(%d,%d) rc(%d,%d) MaxTxPwrReduction(%d,%d)\n",
             OAMgrr_TRX_RC_NO(0),
             ((unsigned short)(grr_OamData.radioCarrierPackage[grr_OamData.transceiverPackage[0].relatedRadioCarrier].
                               radioCarrierID)),
             OAMgrr_TRX_RC_MAXPWR_CUT(0),
             ((unsigned short)(grr_OamData.radioCarrierPackage[grr_OamData.transceiverPackage[0].relatedRadioCarrier].
                               txPwrMaxReduction)),
             OAMgrr_TRX_RC_ARFCN(0,0), 
             ((unsigned short)(((CarrierFrequencyEntry*)
                                (grr_OamData.radioCarrierPackage[grr_OamData.transceiverPackage[0].relatedRadioCarrier].
                                 carrierFrequencyList))[0].carrierFrequency))
          );
}

void grr_PrintIntSecondTrxRadioCarrierPackage(void)
{
     printf("Trx 1 First Radio Carrier Configuration: pwrClass(%d,%d) rc #(%d,%d) MaxTxPwrReduction(%d,%d) Freq(%d,%d)\n",
             OAMgrr_TRX_RC_PWR_CLASS(1),
             ((unsigned short)(grr_OamData.radioCarrierPackage[
             grr_OamData.transceiverPackage[1].relatedRadioCarrier].powerClass)),
             OAMgrr_TRX_RC_NO(1),
             ((unsigned short)(grr_OamData.radioCarrierPackage[
             grr_OamData.transceiverPackage[1].relatedRadioCarrier].radioCarrierID)),
             OAMgrr_TRX_RC_MAXPWR_CUT(1),
             ((unsigned short)(grr_OamData.radioCarrierPackage[
             grr_OamData.transceiverPackage[1].relatedRadioCarrier].txPwrMaxReduction)),
             OAMgrr_TRX_RC_ARFCN(1,0), 
             ((unsigned short)(((CarrierFrequencyEntry*)(grr_OamData.radioCarrierPackage[
             grr_OamData.transceiverPackage[1].relatedRadioCarrier
             ].carrierFrequencyList))[0].carrierFrequency))
         );
}


void grr_PrintIntAdjacentCellHandoverPackage(void)
{
     int i;
     for (i=0;i<16;i++)
     {
     printf("Adjacent Cell Handover Package from MIB %d-th adjacent cell data: selected(%d,%d) networkId(%d,%d) cellId(%d,%d)\n",
             i,
             OAMgrr_HO_ADJCELL_ID(i),
  	       ((unsigned int)(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_handoverCellID)),
             OAMgrr_NCELL_NETWORKID(i),
             ((short)((((AdjCell_HandoverEntry*)
             (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_handoverCellID))>>16),
             OAMgrr_NCELL_CELLID(i),
	       ((short)(((AdjCell_HandoverEntry*)
  	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_handoverCellID))
         );
     printf("mcc(%d,%d,%d) mnc(%d,%d,%x) lac(%d,%d) ci(%d,%d) freq(%d,%d) ncc(%d,%d) bcc(%d,%d) margin(%d,%d) maxTxPwr(%d,%d) minRxLev(%d,%d)\n",
             OAMgrr_HO_ADJCELL_MCC(i,0),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[0],
             OAMgrr_HO_ADJCELL_MCC(i,1),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[1],
             OAMgrr_HO_ADJCELL_MCC(i,2),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[2],
             OAMgrr_HO_ADJCELL_MNC(i,0),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[0],
             OAMgrr_HO_ADJCELL_MNC(i,1),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[1],
             OAMgrr_HO_ADJCELL_MNC(i,2),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[2],
             OAMgrr_HO_ADJCELL_LAC(i),
             ((short)((((AdjCell_HandoverEntry*)
             (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_lac))),
             OAMgrr_HO_ADJCELL_CI(i),
             ((short)((((AdjCell_HandoverEntry*)
             (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_ci))),
	       OAMgrr_HO_ADJCELL_BCCH_ARFCN(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_bCCHFrequency)),
	       OAMgrr_HO_ADJCELL_NCC(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_ncc)),
             OAMgrr_HO_ADJCELL_BCC(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_cid)),
	       OAMgrr_HO_ADJCELL_HO_MARGIN(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_hoMargin)),
	       OAMgrr_HO_ADJCELL_MSTXPWR_MAX_CELL(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_msTxPwrMaxCell)),
	       OAMgrr_HO_ADJCELL_RXLEV_MIN_CELL(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_rxLevMinCell))
         );

     printf("Adjacent Cell Handover Package After Compression %d-th adjacent cell data: selected(%d,%d) networkId(%d,%d) cellId(%d,%d)\n",
             i,
             OAMgrr_HO_ADJCELL_ID_t(i),
  	       ((unsigned int)(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_handoverCellID)),
             OAMgrr_NCELL_NETWORKID_t(i),
             ((short)((((AdjCell_HandoverEntry*)
             (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_handoverCellID))>>16),
             OAMgrr_NCELL_CELLID_t(i),
	       ((short)(((AdjCell_HandoverEntry*)
  	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_handoverCellID))
         );
     printf("mcc(%d,%d,%d) mnc(%d,%d,%x) lac(%d,%d) ci(%d,%d) freq(%d,%d) ncc(%d,%d) bcc(%d,%d) margin(%d,%d) maxTxPwr(%d,%d) minRxLev(%d,%d)\n",
             OAMgrr_HO_ADJCELL_MCC_t(i,0),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_mcc))[0],
             OAMgrr_HO_ADJCELL_MCC_t(i,1),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_mcc))[1],
             OAMgrr_HO_ADJCELL_MCC_t(i,2),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_mcc))[2],
             OAMgrr_HO_ADJCELL_MNC_t(i,0),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_mnc))[0],
             OAMgrr_HO_ADJCELL_MNC_t(i,1),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_mnc))[1],
             OAMgrr_HO_ADJCELL_MNC_t(i,2),
             ((unsigned char*)&(((AdjCell_HandoverEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_mnc))[2],
             OAMgrr_HO_ADJCELL_LAC_t(i),
             ((short)((((AdjCell_HandoverEntry*)
             (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_lac))),
             OAMgrr_HO_ADJCELL_CI_t(i),
             ((short)((((AdjCell_HandoverEntry*)
             (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_ci))),
	       OAMgrr_HO_ADJCELL_BCCH_ARFCN_t(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_bCCHFrequency)),
	       OAMgrr_HO_ADJCELL_NCC(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_ncc)),
             OAMgrr_HO_ADJCELL_BCC_t(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_cid)),
	       OAMgrr_HO_ADJCELL_HO_MARGIN_t(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_hoMargin)),
	       OAMgrr_HO_ADJCELL_MSTXPWR_MAX_CELL_t(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_msTxPwrMaxCell)),
	       OAMgrr_HO_ADJCELL_RXLEV_MIN_CELL_t(i),
 	       ((short)(((AdjCell_HandoverEntry*)
 	       (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_rxLevMinCell))
         );

    }
}

void grr_PrintIntAdjacentCellReselectPackage(void)
{
     int i;
     for (i=0;i<16;i++)
     {
     printf("Adjacent Cell Reselect Package from MIB %d-th adjacent cell data: selected(%d,%d) freq(%d,%d) cellId(%d,%d)\n",
             i,
             OAMgrr_RESEL_ADJCELL_ID(i),
  	       ((unsigned int)(((AdjCell_ReselectionEntry*)
	       (grr_OamData.adjacentCellPackage_t.adjCell_reselectionTable))[i].adjCell_reselectionCellID)),
	       OAMgrr_RESEL_ADJCELL_BCCH_ARFCN(i),
 	       ((short)(((AdjCell_ReselectionEntry*)
 	       (grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[i].adjCell_reselectionBCCHFrequency))
         );
     }
}

void grr_PrintIntGrrConfigData(void)
{
     int i,j;

     printf("Grr Module Configuration Package MIB parameters:\n");

     printf("OAMgrr_T3L01: %x, %x\n",
             OAMgrr_T3L01,
             ((int)(grr_OamData.grrConfigData.rm_t3L01))
           );
             //0xFF
     printf("OAMgrr_BS_CC_CHANS: %x, %x\n",
             OAMgrr_BS_CC_CHANS,
             ((unsigned char)(grr_OamData.grrConfigData.rm_bsCcChans))
           );
             //0x01
     printf("OAMgrr_BSCCCHSDCCH_COMB: %x, %x\n",
             OAMgrr_BSCCCHSDCCH_COMB,
             ((unsigned char)(grr_OamData.grrConfigData.rm_bsCcchSdcchComb)));   
             //0x01 
     printf("OAMgrr_AIRINTERFACE: %x, %x\n",
             OAMgrr_AIRINTERFACE,
	       ((unsigned char)(grr_OamData.grrConfigData.rm_airInterface)));         
             //0x02
     printf("OAMgrr_POCKETBTS_TSC: %x, %x\n",
             OAMgrr_POCKETBTS_TSC,
             ((unsigned char)(grr_OamData.grrConfigData.rm_viperCellTsc)));         
             //0x05
     printf("OAMgrr_SLOTS_PER_TRX: %x, %x\n",
             OAMgrr_SLOTS_PER_TRX,
             ((unsigned char)(grr_OamData.grrConfigData.rm_maxSlotPerTrx)));        
             //0x08
     printf("OAMgrr_MAX_TRXS: %x, %x\n",
             OAMgrr_MAX_TRXS,
             ((unsigned char)(grr_OamData.grrConfigData.rm_maxTrxs)));              
             //0x02
     printf("OAMgrr_MAX_TCHFS: %x, %x\n",
             OAMgrr_MAX_TCHFS,
             ((unsigned char)(grr_OamData.grrConfigData.rm_maxTchfs)));             
             //0x0f
     printf("OAMgrr_MAX_SDCCH4S: %x, %x\n",
             OAMgrr_MAX_SDCCH4S,
             ((unsigned char)(grr_OamData.grrConfigData.rm_maxSdcch4s)));           
             //0x04
     printf("OAMgrr_TRX_ONOFF_MAP: %x, %x\n",
             OAMgrr_TRX_ONOFF_MAP,
             ((unsigned char)(grr_OamData.grrConfigData.rm_trxOnOffMap)));          
             //0x01
     printf("OAMgrr_NETWORKIFCONFIG: %x, %x\n",
             OAMgrr_NETWORKIFCONFIG,
             ((unsigned char)(grr_OamData.grrConfigData.rm_networkIfConfig)));      
             //0x00
     printf("OAMgrr_BCCH_TRX: %x, %x\n",
             OAMgrr_BCCH_TRX,
             ((unsigned char)(grr_OamData.grrConfigData.rm_bcchTrx)));              
             //0x00
     printf("OAMgrr_PREALPHA_TRX: %x, %x\n",
             OAMgrr_PREALPHA_TRX,
             ((unsigned char)(grr_OamData.grrConfigData.rm_preAlphaTrx)));          
             //0x00
     printf("OAMgrr_CCCH_SDCCH_COMB: %x, %x\n",
             OAMgrr_CCCH_SDCCH_COMB,
             ((unsigned char)(grr_OamData.grrConfigData.rm_ccchBcchComb)));         
             //0x01
     printf("OAMgrr_CCCH_CONF: %x, %x\n",
             OAMgrr_CCCH_CONF,
	       ((unsigned char)(grr_OamData.grrConfigData.rm_ccchConf)));             
             //0x01 
             //FS:T_CNI_RIL3_CCCH_CONF:CNI_RIL3_CCCH_CONF_ONE_CCCH_COMBINED_SDCCHS=1
     for (i=0;i<2;i++)
     for (j=0;j<8;j++)
     {
          printf("OAMgrr_NIM(%d,%d): %x, %x\n", i,j,
                  OAMgrr_NIM(i,j),
	           ((unsigned char)(grr_OamData.grrConfigData.rm_nim[i][j])));
                 //rm_nim[2][8]: all elements<-0xFF
     }

     printf("OAMgrr_IQ_SWAP: %d, %d\n", OAMgrr_IQ_SWAP, grr_OamData.grr_IQSwap);

}

void grr_SetDbgDump(void)
{
grr_DbgDump=1;
}

void grr_ResetDbgDump(void)
{
grr_DbgDump=0;
}

void grr_SetGprsCtrl(void)
{
grr_GprsCtrl=1;
grr_PassOamMsParams(0);
grr_PassOamMsParams(1);
}

void grr_ResetGprsCtrl(void)
{
grr_GprsCtrl=0;
grr_PassOamMsParams(0);
grr_PassOamMsParams(1);
}

void grr_GprsSetting(void)
{
printf ("OAMgrr_GPRS_RA_COLOR          = (%d)\n", OAMgrr_GPRS_RA_COLOR          ); //0
printf ("OAMgrr_SI13_POS               = (%d)\n", OAMgrr_SI13_POS               ); //0
printf ("OAMgrr_GPRS_NETWORK_MODE      = (%d)\n", OAMgrr_GPRS_NETWORK_MODE      ); //1
printf ("OAMgrr_CBCH_TRX_SLOT          = (%d)\n", OAMgrr_CBCH_TRX_SLOT          ); //0
printf ("OAMgrr_CBCH_TRX		   = (%d)\n", OAMgrr_CBCH_TRX               ); //0
printf ("OAMgrr_BCCH_CHANGE_MARK       = (%d)\n", OAMgrr_BCCH_CHANGE_MARK       ); //0
printf ("OAMgrr_SI_CHANGE_FIELD	   = (%d)\n", OAMgrr_SI_CHANGE_FIELD        ); //0
printf ("OAMgrr_GPRS_RAC               = (%d)\n", OAMgrr_GPRS_RAC               ); //1
printf ("OAMgrr_GPRS_NMO               = (%d)\n", OAMgrr_GPRS_NMO               ); //1
printf ("OAMgrr_GPRS_T3168             = (%d)\n", OAMgrr_GPRS_T3168             ); //4
printf ("OAMgrr_GPRS_T3192             = (%d)\n", OAMgrr_GPRS_T3192             ); //7
printf ("OAMgrr_GPRS_DRX_TIMER_MAX     = (%d)\n", OAMgrr_GPRS_DRX_TIMER_MAX     ); //7
printf ("OAMgrr_GPRS_ACCESS_BURST_TYPE = (%d)\n", OAMgrr_GPRS_ACCESS_BURST_TYPE ); //0
printf ("OAMgrr_GPRS_CONTROL_ACK_TYPE  = (%d)\n", OAMgrr_GPRS_CONTROL_ACK_TYPE  ); //0
printf ("OAMgrr_GPRS_BS_CV_MAX         = (%d)\n", OAMgrr_GPRS_BS_CV_MAX         ); //7
printf ("OAMgrr_GPRS_PAN_DEC           = (%d)\n", OAMgrr_GPRS_PAN_DEC           ); //1
printf ("OAMgrr_GPRS_PAN_INC           = (%d)\n", OAMgrr_GPRS_PAN_INC           ); //1
printf ("OAMgrr_GPRS_PAN_MAX           = (%d)\n", OAMgrr_GPRS_PAN_MAX           ); //4
printf ("OAMgrr_GPRS_ALPHA             = (%d)\n", OAMgrr_GPRS_ALPHA             ); //10
printf ("OAMgrr_GPRS_T_AVG_W           = (%d)\n", OAMgrr_GPRS_T_AVG_W           ); //15
printf ("OAMgrr_GPRS_T_AVG_T           = (%d)\n", OAMgrr_GPRS_T_AVG_T           ); //5
printf ("OAMgrr_GPRS_PC_MEAS_CHAN      = (%d)\n", OAMgrr_GPRS_PC_MEAS_CHAN      ); //1
printf ("OAMgrr_GPRS_N_AVG_I           = (%d)\n", OAMgrr_GPRS_N_AVG_I           ); //4
}

void grr_SetTrxSlotCombination(int trx, int slot, int comb)
{

MibTag tag;

printf("current valid channel combinations\n");
printf("1-TCH/F + FACCH/F + SACCH/F\n");
printf("2-TCH/H(0,1) + FACCH/H(0,1) + SACCH/H(0,1)n");
printf("3-TCH/H(0,0) + FACCH/H(0,1) + SACCH/H(0,1) + TCH/H(1,1)n");
printf("4-FCCH + SCH + BCCH + CCCHn");
printf("5-FCCH + SCH + BCCH + CCCH + SDCCH/4(0..3) + SACCH/C4(0..3)n");
printf("6-BCCH + CCCHn");
printf("7-combination 5 with CBCH as 05.02 Note 1n");
printf("8-combination 3 with CBCH as 05.02 Note 1n");
printf("11-combination XI for GPRSn");
printf("12-combination XII for GPRSn");
printf("13-combination XIII for GPRS\n");

OAMgrr_TRX_SLOT_COMBa(trx,slot)= (ChannelCombination_t) comb;

if (trx==0) tag = MIBT_channelCombination_0;
else        tag = MIBT_channelCombination_1;
 
if ( STATUS_OK!=oam_setTblEntryField(MODULE_GRR, tag, slot, (ChannelCombination)comb) )
{
    printf("grr_SetTrxSlotCombination: oam_setTblEntryField failed for setting trx(%d) slot(%d) comb(%d)\n",
            trx, slot, comb);
} else
    grr_SendDspSlotActivate( (unsigned char) trx, (unsigned char) slot);
}

#endif // __GRR_TESTUTIL_CPP__
