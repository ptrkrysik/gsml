/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMGETRMGSMDATA_CPP__
#define __RM_OAMGETRMGSMDATA_CPP__

//GP2
#if 0

#include "rm/rm_head.h"
#include "oam_api.h"
#include "stdarg.h"

void DBGPRN(char * format, ...) 
{
#ifdef DEBUG
	va_list marker;
	
	va_start( marker, format );     /* Initialize variable arguments. */
	vprintf(format, marker);
	printf("\n");
	va_end( marker );              /* Reset variable arguments.      */
#endif
	return;
	
}

// Static Table of OAM Table

// Tables in Basic Package
extern CellAllocationEntry cellAllocationTable[CellAllocationTableLimit];
extern PlmnPermittedEntry plmnPermittedTable[PlmnPermittedTableLimit];

// Table in Option Package
extern NotAllowedAccessClassEntry notAllowedAccessClassTable[NotAllowedAccessClassTableLimit];

// Table in TRX packages
extern ChannelEntry	firstTrxPackageChannelTable[ChannelTableLimit];
extern ChannelEntry	secondTrxPackageChannelTable[ChannelTableLimit];

extern CarrierFrequencyEntry firstRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];
extern CarrierFrequencyEntry secondRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];

// Tables in Adjacent Cell Package
extern AdjCell_HandoverEntry adjCell_HandoverTable[HandoverTableLimit];

//PR1223 Begin
extern AdjCell_HandoverEntry adjCell_HandoverTable[HandoverTableLimit];
//PR1223 End
extern AdjCell_ReselectionEntry adjCell_ReselectionTable[ReselectionTableLimit];


STATUS rm_OamGetBtsBasicPackage(void)
{
	STATUS ret;
	
	RDEBUG__(("ENTER-rm_OamGetBtsBasicPackage\n")); 
	
	// step 1: get the bts basic package structure first
	ret = getMibStruct(MIB_btsBasicPackage, (u8*)&rm_OamData.btsBasicPackage,
		sizeof(BtsBasicPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsBasicPackage: btsBasicPackage = %d\n", ret));
		return ret;
	}
	
	// step 2: get the cell allocation table
	ret = getMibTbl(MIBT_cellAllocationEntry, (void *) &cellAllocationTable,
		sizeof(cellAllocationTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsBasicPackage:  cellAllocationTable = %d\n",ret));
		return ret;
	}
	
	// we need to do this since it would be rewritten in step 1.
	
	rm_OamData.btsBasicPackage.cellAllocationTable= (Table *) &cellAllocationTable;
	
	// step 3: get plmnPermitted Table
	ret = getMibTbl(MIBT_plmnPermittedEntry, (void *) &plmnPermittedTable , sizeof(plmnPermittedTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsBasicPackage:  plmnPermittedTable = %d\n",ret));
		return ret;
	}
	
	// and finally do this
	rm_OamData.btsBasicPackage.plmnPermittedTable= (Table *) &plmnPermittedTable;

	// Convert mcc to byte string
	getByteAryByInt(rm_OamData.btsBasicPackage.bts_mcc, 
		(char *) &rm_OamData.btsBasicPackage.bts_mcc,
		3);

	// Convert mnc to byte string
	getByteAryByInt(rm_OamData.btsBasicPackage.bts_mnc, 
		(char *) &rm_OamData.btsBasicPackage.bts_mnc,
		3);
      // Convert txInteger from GUI to RM internal value
      int temp;
      temp = rm_OamData.btsBasicPackage.maxNumberRetransmissions;
      if (temp==1)
          rm_OamData.btsBasicPackage.maxNumberRetransmissions=0;
      else if (temp==2)
          rm_OamData.btsBasicPackage.maxNumberRetransmissions=1;
      else if (temp==4)
          rm_OamData.btsBasicPackage.maxNumberRetransmissions=2;
      else if (temp==7)
          rm_OamData.btsBasicPackage.maxNumberRetransmissions=3;
      else 
          EDEBUG__(("ERROR@rm_OamGetBtsBasicPackage:invalid NoOfReTrans=%d\n",
			   rm_OamData.btsBasicPackage.maxNumberRetransmissions));
 
      rm_OamData.btsBasicPackage.numberOfSlotsSpreadTrans -= 3; //Tx Integer:12

      rm_OamData.btsBasicPackage.noOfMultiframesBetweenPaging -= 2; //3
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_rm_OamGetBtsBasicPackage: BTS Basic Package retrieved successfully\n"));
#endif

	//PR1101:

      int i;

      switch(OAMrm_GSMDCS_INDICATOR)
      {
      case 0: //GSM900
              if (OAMrm_MS_TX_PWR_MAX_CCHa <=39 && OAMrm_MS_TX_PWR_MAX_CCHa >=5)
              {
                  i = OAMrm_MS_TX_PWR_MAX_CCHa/2;
                  OAMrm_MS_TX_PWR_MAX_CCHa = 21 - i;
              } else
              {
             	EDEBUG__(("ERROR@rm_rm_OamGetBtsBasicPackage: (radio system %d, mSTxPwrMaxCch %d)\n",
                             OAMrm_GSMDCS_INDICATOR,
                             OAMrm_MS_TX_PWR_MAX_CCHa));
                  OAMrm_MS_TX_PWR_MAX_CCHa = 7; //29dBm
              }
              break;

      case 1: //DCS1800
              if (OAMrm_MS_TX_PWR_MAX_CCHa <=30 && OAMrm_MS_TX_PWR_MAX_CCHa >=0)
              {
                  i = OAMrm_MS_TX_PWR_MAX_CCHa/2;
                  //Not needed now
                  //if (OAMrm_MS_PWR_OFFSET>=0 && OAMrm_MS_PWR_OFFSETa<=6)
                  //{
                  //    OAMrm_MS_PWR_OFFSETa = OAMrm_MS_PWR_OFFSETa/2;
                  //} else
                  //    OAMrm_MS_PWR_OFFSETa = 0;
                  OAMrm_MS_TX_PWR_MAX_CCHa = 15 - i;
              } else
              {
             	EDEBUG__(("ERROR@rm_rm_OamGetBtsBasicPackage: (radio system %d, mSTxPwrMaxCch %d)\n",
                             OAMrm_GSMDCS_INDICATOR,
                             OAMrm_MS_TX_PWR_MAX_CCHa));
                  OAMrm_MS_TX_PWR_MAX_CCHa = 3; //24dBm
                  //Not needed now
                  //OAMrm_MS_PWR_OFFSETa = 0;
              }
              break;

      case 2: //PCS1900
              if (OAMrm_MS_TX_PWR_MAX_CCHa == 32)
              {
                  OAMrm_MS_TX_PWR_MAX_CCHa = 31; //32dBm
              } else if (OAMrm_MS_TX_PWR_MAX_CCHa == 33)
              {
                  OAMrm_MS_TX_PWR_MAX_CCHa = 31; //33dBm
              } else if (OAMrm_MS_TX_PWR_MAX_CCHa <=30 && OAMrm_MS_TX_PWR_MAX_CCHa >=0)
              {
                  i = OAMrm_MS_TX_PWR_MAX_CCHa/2;
                  OAMrm_MS_TX_PWR_MAX_CCHa = 15 - i;
              } else
              {
             	EDEBUG__(("ERROR@rm_rm_OamGetBtsBasicPackage: (radio system %d, mSTxPwrMaxCch %d)\n",
                             OAMrm_GSMDCS_INDICATOR,
                             OAMrm_MS_TX_PWR_MAX_CCHa));
                  OAMrm_MS_TX_PWR_MAX_CCHa = 3; //24dBm
              }
              break;
            
      default: //Assume a proper value but give debug information
           	  EDEBUG__(("ERROR@rm_rm_OamGetBtsBasicPackage: (radio system %d, mSTxPwrMaxCch %d)\n",
                        OAMrm_GSMDCS_INDICATOR,
                        OAMrm_MS_TX_PWR_MAX_CCHa));
              OAMrm_MS_TX_PWR_MAX_CCHa = 7;
              //Not needed now
              //OAMrm_MS_PWR_OFFSETa = 0;
              break;
      }

	return STATUS_OK;
}

STATUS rm_OamGetBtsOptionPackage(void)
{
	STATUS ret;
	
	ret = getMibStruct(MIB_btsOptionsPackage, (u8*)&rm_OamData.btsOptionsPackage,
		sizeof(BtsOptionsPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsOptionPackage: btsOptionsPackage = %d\n",ret));
		return ret;
	}   
	
	// step 2: get the cell allocation table
	ret = getMibTbl(MIBT_notAllowedAccessClassEntry, (void *) &notAllowedAccessClassTable ,
		sizeof(notAllowedAccessClassTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsOptionPackage:  notAllowedAccessClassTable = %d\n",ret));
		return ret;
	}
	
	// And finally do this
	rm_OamData.btsOptionsPackage.notAllowedAccessClassTable= (Table *) &notAllowedAccessClassTable;
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_rm_OamGetBtsBasicPackage: BTS Basic Options retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}



STATUS rm_OamGetBtsFirstTrxPackage(void)
{
	STATUS ret;
	
	ret = getMibStruct(MIB_firstTransceiverPackage, (u8*)&rm_OamData.transceiverPackage[0],
		sizeof(FirstTransceiverPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsFirstTrxPackage: firstTransceiverPackage = %d\n",ret));
		return ret;
	}   
	
	// step 2: get the cell allocation table
	ret = getMibTbl(MIBT_channel_0_Entry, (void *) &firstTrxPackageChannelTable ,
		sizeof(firstTrxPackageChannelTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsFirstTrxPackage:  firstTrxPackageChannelTable = %d\n",ret));
		return ret;
	}
	
	// And finally do this
	rm_OamData.transceiverPackage[0].channelTable= (Table *) &firstTrxPackageChannelTable;
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_OamGetBtsFirstTrxPackage: BTS 1st TRX package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}


STATUS rm_OamGetBtsSecondTrxPackage(void)
{
	STATUS ret;
	
	// step 1: get the 2nd TRX Package first
	ret = getMibStruct(MIB_secondTransceiverPackage, (u8*)&rm_OamData.transceiverPackage[1],
		sizeof(SecondTransceiverPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsSecondTrxPackage: secondTransceiverPackage = %d\n",ret));
		return ret;
	}   
	
	// step 2: get the cell allocation table
	ret = getMibTbl(MIBT_channel_1_Entry, (void *) &secondTrxPackageChannelTable ,
		sizeof(secondTrxPackageChannelTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsSecondTrxPackage: secondTrxPackageChannelTable  = %d\n",ret));
		return ret;
	}
	
	// And finally do this
	rm_OamData.transceiverPackage[1].channelTable= (Table *) &secondTrxPackageChannelTable;
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_OamGetBtsSecondTrxPackage: BTS 2nd TRX package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}



STATUS rm_OamGetBtsFirstRadioCarrierPackage(void)
{
	STATUS ret;
	
	// step 1: get the 2nd Radio Carrier Package first
	ret = getMibStruct(MIB_firstRadioCarrierPackage, (u8*) &rm_OamData.radioCarrierPackage[0],
		sizeof(FirstRadioCarrierPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetRmGsmData: firstRadioCarrierPackage = %d\n",ret));
		return ret;
	}
	
	// step 2: get the radio frequency list for the first carrier package
	ret = getMibTbl(MIBT_carrierFrequency_0_Entry, (void *) &firstRadioCarrierPackageFrequencyListTable ,
		sizeof(firstRadioCarrierPackageFrequencyListTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsFirstTrxPackage:  firstRadioCarrierPackageFrequencyListTable = %d\n",ret));
		return ret;
	}
	
	// And finally do this
	rm_OamData.radioCarrierPackage[0].carrierFrequencyList= (Table *) &firstRadioCarrierPackageFrequencyListTable;
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_OamGetBtsFirstTrxPackage: 1st radio carrier package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}


STATUS rm_OamGetBtsSecondRadioCarrierPackage(void)
{
	STATUS ret;
	
	// step 1: get the 2nd Radio Carrier Package first
	ret = getMibStruct(MIB_secondRadioCarrierPackage, (u8*) &rm_OamData.radioCarrierPackage[1],
		sizeof(SecondRadioCarrierPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetRmGsmData: secondRadioCarrierPackage = %d\n",ret));
		return ret;
	}
	
	// step 2: get the radio frequency list for the first carrier package
	ret = getMibTbl(MIBT_carrierFrequency_1_Entry, (void *) &secondRadioCarrierPackageFrequencyListTable ,
		sizeof(secondRadioCarrierPackageFrequencyListTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetBtsFirstTrxPackage:  secondRadioCarrierPackageFrequencyListTable = %d\n",ret));
		return ret;
	}
	
	// And finally do this
	rm_OamData.radioCarrierPackage[1].carrierFrequencyList= (Table *) &secondRadioCarrierPackageFrequencyListTable;
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_OamGetBtsFirstTrxPackage: 2nd radio carrier package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}


STATUS rm_OamGetT31xxTimerStructure(void)
{
	STATUS ret;
	
	ret = getMibStruct(MIB_t31xxPackage, (u8*)&rm_OamData.t31xx,
		sizeof(T31xx));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_OamGetT31xxTimerStructure: t31xx = %d\n",ret));
		return ret;
	}   
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_OamGetT31xxTimerStructure: T31xx timer structure retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}

STATUS rm_adjacentCellPackage(void)
{
	STATUS ret;
      int i,j;
	
	// step 1: get adjacent cell Package first
	ret = getMibStruct(MIB_adjacentCellPackage, (u8*)&rm_OamData.adjacentCellPackage,
		sizeof(AdjacentCellPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_adjacentCellPackage: adjacentCellPackage = %d\n",ret));
		return ret;
	}   
	
	// step 2: get handover table list
	ret = getMibTbl(MIBT_adjCell_handoverEntry, (void *) &adjCell_HandoverTable,
		sizeof(adjCell_HandoverTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_adjacentCellPackage:  adjCell_HandoverTable = %d\n",ret));
		return ret;
	}
	
	// And
	rm_OamData.adjacentCellPackage.adjCell_handoverTable= (Table *) &adjCell_HandoverTable;
      //Compress the HandoverTable to remove any empty cell rows
      i=0;
      while (i<OAMrm_HO_ADJCELL_NO_MAX)
      {
           if (OAMrm_HO_ADJCELL_ID(i)==OAMrm_HO_ADJCELL_ID_NULL)
           {
               for (j=i+1;j<OAMrm_HO_ADJCELL_NO_MAX;j++)
               {
                   if (OAMrm_HO_ADJCELL_ID(j) != OAMrm_HO_ADJCELL_ID_NULL)
                   {
                       memcpy(&adjCell_HandoverTable[i],&adjCell_HandoverTable[j],
                              sizeof(AdjCell_HandoverEntry));
                       adjCell_HandoverTable[i].adjCell_handoverIndex=i;			
                       memset(&adjCell_HandoverTable[j],0,sizeof(AdjCell_HandoverEntry));
			     adjCell_HandoverTable[j].adjCell_handoverIndex=j;
                       break;
                   }
               }
           }
           i++;
      }                   
	
	// step 3: get handover table list
	ret = getMibTbl(MIBT_adjCell_reselectionEntry, (void *) &adjCell_ReselectionTable,
		sizeof(adjCell_ReselectionTable));
	
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_adjacentCellPackage:  adjCell_ReselectionTable = %d\n",ret));
		return ret;
	}
	
	// And finally do this
	rm_OamData.adjacentCellPackage.adjCell_reselectionTable= (Table *) &adjCell_ReselectionTable;
      i=0;
      while (i<OAMrm_RESEL_ADJCELL_NO_MAX)
      {
           if (OAMrm_RESEL_ADJCELL_ID(i)==OAMrm_RESEL_ADJCELL_ID_NULL)
           {
               for (j=i+1;j<OAMrm_RESEL_ADJCELL_NO_MAX;j++)
               {
                   if (OAMrm_RESEL_ADJCELL_ID(j) != OAMrm_RESEL_ADJCELL_ID_NULL)
                   {
                       memcpy(&adjCell_ReselectionTable[i],&adjCell_ReselectionTable[j],
                              sizeof(AdjCell_ReselectionEntry));
                       adjCell_ReselectionTable[i].adjCell_reselectionIndex = i;			
                       memset(&adjCell_ReselectionTable[j],0,sizeof(AdjCell_ReselectionEntry));
			     adjCell_ReselectionTable[j].adjCell_reselectionIndex = j;
                       break;
                   }
               }
           }
           i++;
      }                   	
#ifdef DEBUG
	EDEBUG__(("UT-rm_OamGetBtsFirstTrxPackage: adjacent cell package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}

STATUS rm_handoverControlPackage(void)
{
	STATUS ret;
	
	ret = getMibStruct(MIB_handoverControlPackage, (u8*)&rm_OamData.handoverControlPackage,
		sizeof(HandoverControlPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_handoverControlPackage: handoverControlPackage = %d\n",ret));
		return ret;
	} 
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_handoverControlPackage: handover control package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}


STATUS rm_powerControlPackage(void)
{
	STATUS ret;
	
	
	ret = getMibStruct(MIB_powerControlPackage, (u8*)&rm_OamData.powerControlPackage,
		sizeof(PowerControlPackage));
	if (ret != STATUS_OK)  
	{
		EDEBUG__(("ERROR-rm_powerControlPackage: powerControlPackage = %d\n",ret));
		return ret;
	}
	
#ifdef DEBUG
	EDEBUG__(("UT-rm_powerControlPackage: power control package retrieved successfully\n"));
#endif
	
	return STATUS_OK;
}


void DBG_Print_cellAllocationTable(Table *tbl)
{
	int i;
	CellAllocationEntry *entry = (CellAllocationEntry *) tbl;
	DBGPRN("%s\t%s", "Index", "Cell Allociation");
	
	for (i = 0; i< CellAllocationTableLimit; i++)
	{
		
		DBGPRN("%d\t\t%d", i+1, (entry+i)->cellAllocation);
	}
}

void DBG_Print_plmnPermittedTable(Table *tbl)
{
	int i;
	PlmnPermittedEntry *entry = (PlmnPermittedEntry *) tbl;
	DBGPRN("%s\t%s", "Index", "Plmn Permitted");
	
	for (i = 0; i< PlmnPermittedTableLimit; i++)
	{
		
		DBGPRN("%d\t\t%d", i+1, (entry+i)->plmnPermitted);
	}
}

void DBG_Print_notAllowedAccessClassTable(Table *tbl)
{
	int i;
	NotAllowedAccessClassEntry *entry = (NotAllowedAccessClassEntry *) tbl;
	DBGPRN("%s\t%s", "Index", "Not Allowed Access Class");
	
	for (i = 0; i< NotAllowedAccessClassTableLimit ; i++)
	{
		
		DBGPRN("%d\t\t%d", i+1, (entry+i)->notAllowedAccessClass);
	}
}

void DBG_Print_channelTable(Table *tbl)
{
	int i;
	ChannelEntry *entry = (ChannelEntry *) tbl;
	DBGPRN("%s,%s,%s,%s,%s,%s,%s", "Idx", "ID", "Comb", "fusage", "tch", "a.state", "o.state");
	
	for (i = 0; i<ChannelTableLimit ; i++)
	{
		
		DBGPRN("%d,%d,%d,%d,%d,%d,%d", i+1, (entry+i)->channelID,
			(entry+i)->channelCombination,
			(entry+i)->channelFrequencyUsage,
			(entry+i)->channelTsc,
			(entry+i)->channelAdministrativeState,
			(entry+i)->channelOperationalState);
	}
}

void DBG_Print_carrierFrequencyTable(Table *tbl)
{
	int i;
	CarrierFrequencyEntry *entry = (CarrierFrequencyEntry *) tbl;
	DBGPRN("%s\t%s", "Index", "Carrier Frequency");
	
	for (i = 0; i< CarrierFrequencyTableLimit; i++)
	{
		
		DBGPRN("%d\t\t%d", i+1, (entry+i)->carrierFrequency);
	}
}

void DBG_Print_adjCell_handoverTable(Table *tbl)
{
	int i;
	AdjCell_HandoverEntry *entry = (AdjCell_HandoverEntry *) tbl;
	DBGPRN("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", "Idx", "ID", "mcc", "mnc", "lac", "ci", "bcch",
		"ncc", "cid", "sync", "hoPl", "hoM", "msTx", "rxLev");
	
	for (i = 0; i< HandoverTableLimit; i++)
	{
		
		DBGPRN("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
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

void DBG_Print_adjCell_reselectionTable(Table *tbl)
{
	int i;
	AdjCell_ReselectionEntry *entry = (AdjCell_ReselectionEntry *) tbl;
	DBGPRN("%s\t%s\t%s", "Index", "ID", "Reselection BCCH Frequency");
	
	for (i = 0; i< ReselectionTableLimit; i++)
	{
		
		DBGPRN("%d\t\t%d\t\t%d", i+1, (entry+i)->adjCell_reselectionCellID,
			(entry+i)->adjCell_reselectionBCCHFrequency);
	}
}


void rm_print_btsBasicPackage(BtsBasicPackage *btsBasicPackage)
{
	DBGPRN("\nBts Bacic Package");

	char *cp;
	
	DBGPRN("bts_ncc= %d",	btsBasicPackage->bts_ncc);
	DBGPRN("bts_cid= %d",	btsBasicPackage->bts_cid);
	DBGPRN("btsID= %d",	btsBasicPackage->btsID);
	
	DBG_Print_cellAllocationTable(btsBasicPackage->cellAllocationTable);
	
	DBGPRN("gsmdcsIndicator= %d",	btsBasicPackage->gsmdcsIndicator);
	cp = (char *) &btsBasicPackage->bts_mcc;
	DBGPRN("bts_mcc= %d%d%d",	cp[0], cp[1], cp[2]);
	cp = (char *) &btsBasicPackage->bts_mnc;
	DBGPRN("bts_mnc= %d%d",	cp[0],cp[1],cp[2]);

	DBGPRN("bts_lac= %d",	btsBasicPackage->bts_lac);
	DBGPRN("bts_ci= %d",	btsBasicPackage->bts_ci);
	DBGPRN("cellReselectHysteresis= %d",	btsBasicPackage->cellReselectHysteresis);
	DBGPRN("ny1= %d",	btsBasicPackage->ny1);
	
	DBG_Print_plmnPermittedTable (btsBasicPackage->plmnPermittedTable);
	
	DBGPRN("radioLinkTimeout= %d",	btsBasicPackage->radioLinkTimeout);
	DBGPRN("relatedTranscoder= %d",	btsBasicPackage->relatedTranscoder);
	DBGPRN("rxLevAccessMin= %d",	btsBasicPackage->rxLevAccessMin);
	DBGPRN("bts_administrativeState= %d",	btsBasicPackage->bts_administrativeState);
	DBGPRN("bts_alarmStatus= %d",	btsBasicPackage->bts_alarmStatus);
	DBGPRN("bts_operationalState= %d",	btsBasicPackage->bts_operationalState);
	DBGPRN("maxNumberRetransmissions= %d",	btsBasicPackage->maxNumberRetransmissions);
	DBGPRN("mSTxPwrMaxCCH= %d",	btsBasicPackage->mSTxPwrMaxCCH);
	DBGPRN("numberOfSlotsSpreadTrans= %d",	btsBasicPackage->numberOfSlotsSpreadTrans);
	DBGPRN("noOfBlocksForAccessGrant= %d",	btsBasicPackage->noOfBlocksForAccessGrant);
	DBGPRN("noOfMultiframesBetweenPaging= %d",btsBasicPackage->noOfMultiframesBetweenPaging);
	
}


void rm_print_btsOptionsPackage(BtsOptionsPackage *btsOptionsPackage)
{
	DBGPRN("\nBts Optioons Package");
	
	DBGPRN("allowIMSIAttachDetach= %d",	btsOptionsPackage->allowIMSIAttachDetach);
	DBGPRN("callReestablishmentAllowed= %d",	btsOptionsPackage->callReestablishmentAllowed);
	DBGPRN("cellBarred= %d",	btsOptionsPackage->cellBarred);
	DBGPRN("dtxDownlink= %d",	btsOptionsPackage->dtxDownlink);
	DBGPRN("dtxUplink= %d",	btsOptionsPackage->dtxUplink);
	DBGPRN("emergencyCallRestricted= %d",	btsOptionsPackage->emergencyCallRestricted);
	
	DBG_Print_notAllowedAccessClassTable(btsOptionsPackage->notAllowedAccessClassTable);
	
	DBGPRN("timerPeriodicUpdateMS= %d",	btsOptionsPackage->timerPeriodicUpdateMS);
	DBGPRN("maxQueueLength= %d",	btsOptionsPackage->maxQueueLength);
	DBGPRN("msPriorityUsedInQueueing= %d",	btsOptionsPackage->msPriorityUsedInQueueing);
	DBGPRN("timeLimitCall= %d",	btsOptionsPackage->timeLimitCall);
	DBGPRN("timeLimitHandover= %d",	btsOptionsPackage->timeLimitHandover);
}

void rm_print_transceiverPackage(TransceiverPackage *transceiverPackage)
{
	
	DBGPRN("basebandTransceiverID= %d",	transceiverPackage->basebandTransceiverID);
	
	DBGPRN("relatedRadioCarrier= %d",	transceiverPackage->relatedRadioCarrier);
	DBGPRN("basebandAdministrativeState= %d",	transceiverPackage->basebandAdministrativeState);
	DBGPRN("basebandAlarmStatus= %d",	transceiverPackage->basebandAlarmStatus);
	DBGPRN("basebandOperationalState= %d",	transceiverPackage->basebandOperationalState);
	
	DBG_Print_channelTable(transceiverPackage->channelTable);
}

void rm_print_firstTransceiverPackage(FirstTransceiverPackage *firstTransceiverPackage)
{
	
	DBGPRN("\nFirst Transceiver Package");
	rm_print_transceiverPackage(firstTransceiverPackage);
}

void rm_print_secondTransceiverPackage(SecondTransceiverPackage *secondTransceiverPackage)
{
	
	DBGPRN("\nSecond Transceiver Package");
	rm_print_transceiverPackage(secondTransceiverPackage);
}

void rm_print_radioCarrierPackage(RadioCarrierPackage *radioCarrierPackage)
{
	
	DBG_Print_carrierFrequencyTable(radioCarrierPackage->carrierFrequencyList);
	
	DBGPRN("powerClass= %d",	radioCarrierPackage->powerClass);
	DBGPRN("radioCarrierID= %d",	radioCarrierPackage->radioCarrierID);
	DBGPRN("txPwrMaxReduction= %d",	radioCarrierPackage->txPwrMaxReduction);
	DBGPRN("carrier_administrativeState= %d",	radioCarrierPackage->carrier_administrativeState);
	DBGPRN("carrier_alarmStatus= %d",	radioCarrierPackage->carrier_alarmStatus);
	DBGPRN("carrier_operationalState= %d",	radioCarrierPackage->carrier_operationalState);
	
}

void rm_print_firstRadioCarrierPackage(FirstRadioCarrierPackage *firstRadioCarrierPackage)
{
	
	DBGPRN("\nFirst Radio Carrier Package");
	rm_print_radioCarrierPackage(firstRadioCarrierPackage);
}

void rm_print_secondRadioCarrierPackage(SecondRadioCarrierPackage *secondRadioCarrierPackage)
{
	
	DBGPRN("\nSecond Radio Carrier Package");
	rm_print_radioCarrierPackage(secondRadioCarrierPackage);
}


void rm_print_t31xx(T31xx *t31xx)
{
	
	DBGPRN("\nT31xx Timers");
	
	DBGPRN("t3101= %d",	t31xx->t3101);
	DBGPRN("t3103= %d",	t31xx->t3103);
	DBGPRN("t3105= %d",	t31xx->t3105);
	DBGPRN("t3107= %d",	t31xx->t3107);
	DBGPRN("t3109= %d",	t31xx->t3109);
	DBGPRN("t3111= %d",	t31xx->t3111);
	DBGPRN("t3113= %d",	t31xx->t3113);
	
}

void rm_print_adjacentCellPackage(AdjacentCellPackage *adjacentCellPackage)
{
	DBGPRN("\nAdjacent Cell Package");
	
	DBG_Print_adjCell_handoverTable(adjacentCellPackage->adjCell_handoverTable);
	DBG_Print_adjCell_reselectionTable(adjacentCellPackage->adjCell_reselectionTable);
}



void rm_print_handoverControlPackage(HandoverControlPackage *handoverControlPackage)
{
	DBGPRN("\nHandover Control Package");
	
	DBGPRN("handoverControlID= %d",	handoverControlPackage->handoverControlID);
	DBGPRN("enableOptHandoverProcessing= %d",	handoverControlPackage->enableOptHandoverProcessing);
	
	DBGPRN("%x:%x", handoverControlPackage->hoAveragingAdjCellParam, &mnetMib.hoAveragingAdjCellParam);

	DBGPRN("hoAveragingAdjCellParamHreqave= %d",	handoverControlPackage->hoAveragingAdjCellParam->hreqave);
	DBGPRN("hoAveragingAdjCellParamHreqt= %d",	handoverControlPackage->hoAveragingAdjCellParam->hreqt);
	DBGPRN("hoAveragingAdjCellParamWeighting= %d",	handoverControlPackage->hoAveragingAdjCellParam->weighting);
	
	DBGPRN("hoAveragingDistParamHreqave= %d",	handoverControlPackage->hoAveragingDistParam->hreqave);
	DBGPRN("hoAveragingDistParamHreqt= %d",	handoverControlPackage->hoAveragingDistParam->hreqt);
	
	DBGPRN("hoAveragingLevParamHreqave= %d",	handoverControlPackage->hoAveragingLevParam->hreqave);
	DBGPRN("hoAveragingLevParamHreqt= %d",	handoverControlPackage->hoAveragingLevParam->hreqt);
	DBGPRN("hoAveragingLevParamWeighting= %d",	handoverControlPackage->hoAveragingLevParam->weighting);
	
	DBGPRN("hoAveragingQualParamHreqave= %d",	handoverControlPackage->hoAveragingQualParam->hreqave);
	DBGPRN("hoAveragingQualParamHreqt= %d",	handoverControlPackage->hoAveragingQualParam->hreqt);
	DBGPRN("hoAveragingQualParamWeighting= %d",	handoverControlPackage->hoAveragingQualParam->weighting);
	
	DBGPRN("hoMarginDef= %d",	handoverControlPackage->hoMarginDef);
	
	DBGPRN("hoThresholdDistParamTimeadv= %d",	handoverControlPackage->hoThresholdDistParam->timeadv);
	DBGPRN("hoThresholdDistParamP8= %d",	handoverControlPackage->hoThresholdDistParam->p8);
	DBGPRN("hoThresholdDistParamN8= %d",	handoverControlPackage->hoThresholdDistParam->n8);
	
	DBGPRN("hoThresholdInterfaceParamRxLevelUL= %d",	handoverControlPackage->hoThresholdInterfaceParam->rxLevelUL);
	DBGPRN("hoThresholdInterfaceParamRxLevelDL= %d",	handoverControlPackage->hoThresholdInterfaceParam->rxLevelDL);
	DBGPRN("hoThresholdInterfaceParamPx= %d",	handoverControlPackage->hoThresholdInterfaceParam->px);
	DBGPRN("hoThresholdInterfaceParamNx= %d",	handoverControlPackage->hoThresholdInterfaceParam->nx);
	
	DBGPRN("hoThresholdLevParamRxLevelUL= %d",	handoverControlPackage->hoThresholdLevParam->rxLevelUL);
	DBGPRN("hoThresholdLevParamRxLevelDL= %d",	handoverControlPackage->hoThresholdLevParam->rxLevelDL);
	DBGPRN("hoThresholdLevParamPx= %d",	handoverControlPackage->hoThresholdLevParam->px);
	DBGPRN("hoThresholdLevParamNx= %d",	handoverControlPackage->hoThresholdLevParam->nx);
	
	DBGPRN("hoThresholdQualParamRxQualUL= %d",	handoverControlPackage->hoThresholdQualParam->rxQualUL);
	DBGPRN("hoThresholdQualParamRxQualDL= %d",	handoverControlPackage->hoThresholdQualParam->rxQualDL);
	DBGPRN("hoThresholdQualParamPx= %d",	handoverControlPackage->hoThresholdQualParam->px);
	DBGPRN("hoThresholdQualParamNx= %d",	handoverControlPackage->hoThresholdQualParam->nx);
	
	DBGPRN("interferenceAveragingParamAveragingPeriod= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamAveragingPeriod);
	DBGPRN("interferenceAveragingParamThresholdBoundary0= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary0);
	DBGPRN("interferenceAveragingParamThresholdBoundary1= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary1);
	DBGPRN("interferenceAveragingParamThresholdBoundary2= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary2);
	DBGPRN("interferenceAveragingParamThresholdBoundary3= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary3);
	DBGPRN("interferenceAveragingParamThresholdBoundary4= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary4);
	DBGPRN("interferenceAveragingParamThresholdBoundary5= %d",	handoverControlPackage->interferenceAveragingParam->interferenceAveragingParamThresholdBoundary5);
	
	DBGPRN("msTxPwrMaxCellDef= %d",	handoverControlPackage->mxTxPwrMaxCellDef);
	DBGPRN("rxLevMinCellDef= %d",	handoverControlPackage->rxLevMinCellDef);
	
}

void rm_print_powerControlPackage(PowerControlPackage *powerControlPackage)
{
	DBGPRN("\nPower Control Package");
	
	DBGPRN("pcAveragingLevHreqave= %d",	powerControlPackage->pcAveragingLev->hreqave);
	DBGPRN("pcAveragingLevHreqt= %d",	powerControlPackage->pcAveragingLev->hreqt);
	DBGPRN("pcAveragingLevWeighting= %d",	powerControlPackage->pcAveragingLev->weighting);
	
	DBGPRN("pcAveragingQualHreqave= %d",	powerControlPackage->pcAveragingQual->hreqave);
	DBGPRN("pcAveragingQualHreqt= %d",	powerControlPackage->pcAveragingQual->hreqt);
	DBGPRN("pcAveragingQualWeighting= %d",	powerControlPackage->pcAveragingQual->weighting);
	
	DBGPRN("pcLowerThresholdLevParamRxLevelUL= %d",	powerControlPackage->pcLowerThresholdLevParam->rxLevelUL);
	DBGPRN("pcLowerThresholdLevParamRxLevelDL= %d",	powerControlPackage->pcLowerThresholdLevParam->rxLevelDL);
	DBGPRN("pcLowerThresholdLevParamPx= %d",	powerControlPackage->pcLowerThresholdLevParam->px);
	DBGPRN("pcLowerThresholdLevParamNx= %d",	powerControlPackage->pcLowerThresholdLevParam->nx);
	
	DBGPRN("pcLowerThresholdQualParamRxQualUL= %d",	powerControlPackage->pcLowerThresholdQualParam->rxQualUL);
	DBGPRN("pcLowerThresholdQualParamRxQualDL= %d",	powerControlPackage->pcLowerThresholdQualParam->rxQualDL);
	DBGPRN("pcLowerThresholdQualParamPx= %d",	powerControlPackage->pcLowerThresholdQualParam->px);
	DBGPRN("pcLowerThresholdQualParamNx= %d",	powerControlPackage->pcLowerThresholdQualParam->nx);
	
	DBGPRN("pcUpperThresholdLevParamRxLevelUL= %d",	powerControlPackage->pcUpperThresholdLevParam->rxLevelUL);
	DBGPRN("pcUpperThresholdLevParamRxLevelDL= %d",	powerControlPackage->pcUpperThresholdLevParam->rxLevelDL);
	DBGPRN("pcUpperThresholdLevParamPx= %d",	powerControlPackage->pcUpperThresholdLevParam->px);
	DBGPRN("pcUpperThresholdLevParamNx= %d",	powerControlPackage->pcUpperThresholdLevParam->nx);
	
	DBGPRN("pcUpperThresholdQualParamRxQualUL= %d",	powerControlPackage->pcUpperThresholdQualParam->rxQualUL);
	DBGPRN("pcUpperThresholdQualParamRxQualDL= %d",	powerControlPackage->pcUpperThresholdQualParam->rxQualDL);
	DBGPRN("pcUpperThresholdQualParamPx= %d",	powerControlPackage->pcUpperThresholdQualParam->px);
	DBGPRN("pcUpperThresholdQualParamNx= %d",	powerControlPackage->pcUpperThresholdQualParam->nx);
	DBGPRN("powerControlInterval= %d",	powerControlPackage->powerControlInterval);
	DBGPRN("powerIncrStepSize= %d",	powerControlPackage->powerIncrStepSize);
	DBGPRN("powerRedStepSize= %d",	powerControlPackage->powerRedStepSize);
}


// print the retrieved value and this is used for debug only
void rm_printOamGsmData(void)
{
	rm_print_btsBasicPackage(&rm_OamData.btsBasicPackage);
	rm_print_btsOptionsPackage(&rm_OamData.btsOptionsPackage);
	rm_print_firstTransceiverPackage(&rm_OamData.transceiverPackage[0]);
	rm_print_secondTransceiverPackage(&rm_OamData.transceiverPackage[1]);
	rm_print_firstRadioCarrierPackage(&rm_OamData.radioCarrierPackage[0]);
	rm_print_secondRadioCarrierPackage(&rm_OamData.radioCarrierPackage[1]);
	rm_print_t31xx(&rm_OamData.t31xx);
	rm_print_adjacentCellPackage(&rm_OamData.adjacentCellPackage);
	rm_print_handoverControlPackage(&rm_OamData.handoverControlPackage);
	rm_print_powerControlPackage(&rm_OamData.powerControlPackage);

}


// retrieve all the GSM MIB data
void rm_OamGetGsmData(void)
{
	rm_OamGetBtsBasicPackage();
	rm_OamGetBtsOptionPackage();
	rm_OamGetBtsFirstTrxPackage();
	rm_OamGetBtsSecondTrxPackage();
	rm_OamGetBtsFirstRadioCarrierPackage();
	rm_OamGetBtsSecondRadioCarrierPackage();
	rm_OamGetT31xxTimerStructure();
	rm_adjacentCellPackage();
	rm_handoverControlPackage();
	rm_powerControlPackage();
//	rm_printOamGsmData();
}

//GP2
#endif //#if 0

#endif /* __RM_OAMGETRMGSMDATA_CPP__ */
