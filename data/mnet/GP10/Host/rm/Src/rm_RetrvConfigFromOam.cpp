/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_RETRVCONFIGFROMOAM_CPP__
#define __RM_RETRVCONFIGFROMOAM_CPP__

//GP2
#if 0

#include "rm\rm_head.h"


// Static Table of OAM Table

// Tables in Basic Package
CellAllocationEntry cellAllocationTable[CellAllocationTableLimit];
PlmnPermittedEntry plmnPermittedTable[PlmnPermittedTableLimit];

// Table in Option Package
NotAllowedAccessClassEntry notAllowedAccessClassTable[NotAllowedAccessClassTableLimit];

// Table in TRX packages
ChannelEntry	firstTrxPackageChannelTable[ChannelTableLimit];
ChannelEntry	secondTrxPackageChannelTable[ChannelTableLimit];

CarrierFrequencyEntry firstRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];
CarrierFrequencyEntry secondRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];

// Tables in Adjacent Cell Package
AdjCell_HandoverEntry adjCell_HandoverTable[HandoverTableLimit];
//PR1223 Begin
AdjCell_HandoverEntry adjCell_HandoverTable_t[HandoverTableLimit];
//PR1223 End
AdjCell_ReselectionEntry adjCell_ReselectionTable[ReselectionTableLimit];


void rm_RetrvConfigFromOam(void)
{
     RDEBUG__(("ENTER-rm_RetrvConfigFromOam\n"));

     
     RM_MEMSET(&rm_OamData, sizeof(rm_OamData));

     // Initialize Table area
     RM_MEMSET(&cellAllocationTable, sizeof(cellAllocationTable));
     RM_MEMSET(&plmnPermittedTable, sizeof(plmnPermittedTable));
     RM_MEMSET(&notAllowedAccessClassTable, sizeof(notAllowedAccessClassTable));
     RM_MEMSET(&firstTrxPackageChannelTable, sizeof(firstTrxPackageChannelTable));
     RM_MEMSET(&secondTrxPackageChannelTable, sizeof(secondTrxPackageChannelTable));

     RM_MEMSET(&firstRadioCarrierPackageFrequencyListTable, sizeof(firstRadioCarrierPackageFrequencyListTable));
     RM_MEMSET(&secondRadioCarrierPackageFrequencyListTable, sizeof(secondRadioCarrierPackageFrequencyListTable));
     RM_MEMSET(&adjCell_HandoverTable, sizeof(adjCell_HandoverTable));
     RM_MEMSET(&adjCell_ReselectionTable, sizeof(adjCell_ReselectionTable));


	 //PR1223 Begin
   	 RM_MEMSET(&adjCell_HandoverTable_t,sizeof(adjCell_HandoverTable));
	 rm_OamData.adjacentCellPackage_t.adjCell_handoverTable =  (Table *) &adjCell_HandoverTable_t;
     //PR1223 End

	 /* This is not needed since it will be done later stage 
    
	  // Do appropriate memory mapping
     rm_OamData.btsBasicPackage.cellAllocationTable->data = (char *) cellAllocationTable;
     rm_OamData.btsBasicPackage.plmnPermittedTable->data = (char *) plmnPermittedTable;
     rm_OamData.btsOptionsPackage.notAllowedAccessClassTable->data = (char *) notAllowedAccessClassTable;
	 
     rm_OamData.transceiverPackage[0].channelTable->data = (char *) firstTrxPackageChannelTable;
     rm_OamData.transceiverPackage[1].channelTable->data = (char *) secondTrxPackageChannelTable;

     rm_OamData.radioCarrierPackage[0].carrierFrequencyList->data = (char *) firstRadioCarrierPackageFrequencyListTable;
     rm_OamData.radioCarrierPackage[1].carrierFrequencyList->data = (char *) secondRadioCarrierPackageFrequencyListTable;
	
     rm_OamData.adjacentCellPackage.adjCell_handoverTable->data = (char *) adjCell_HandoverTable;
     rm_OamData.adjacentCellPackage.adjCell_reselectionTable->data = (char *) adjCell_ReselectionTable;

     */

     //Standard MIB data
     rm_OamGetGsmData();

     //RM-specific data
     rm_OamGetRmConfigData();

     //Generate tuneSynthSetting values
     oam_TrxTuneSynthSetting[0][0] = OAMrm_TRX0_SYNTH_A | (OAMrm_ARFCN(0)&0x3FF);
     oam_TrxTuneSynthSetting[1][0] = OAMrm_TRX1_SYNTH_A | (OAMrm_ARFCN(1)&0x3FF);
     oam_TrxTuneSynthSetting[1][1] = OAMrm_TRX1_SYNTH_B | (OAMrm_ARFCN(1)&0x3FF);

     //Register OAM vars
     rm_OamRegTrapVars();


} /* End of rm_RetrvConfigFromOam() */

//GP2
#endif //#if 0

#endif /* __RM_RETRVCONFIGFROMOAM_CPP__ */
