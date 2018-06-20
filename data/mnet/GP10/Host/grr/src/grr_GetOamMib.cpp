/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_GETOAMMIB_CPP__
#define __GRR_GETOAMMIB_CPP__

#include "grr\grr_head.h"

void grr_GetOamBtsBasicPackage(void)
{
  int    i;
  STATUS ret;

  DBG_FUNC("grr_GetOamBtsBasicPackage", GRR_LAYER_OAM);
  DBG_ENTER();
	
  //step 1: get the bts basic package structure first
  ret = getMibStruct(MIB_btsBasicPackage, (unsigned char*)&grr_OamData.btsBasicPackage,
	               sizeof(BtsBasicPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsBasicPackage: getMibStruct failed to get btsBasicPackage err(%d)\n", ret);
      printf("grr_GetOamBtsBasicPackage: getMibStruct failed to get btsBasicPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  // step 2: get the cell allocation table
  ret = getMibTbl(MIBT_cellAllocationEntry, (void *) &cellAllocationTable,
	            sizeof(cellAllocationTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsBasicPackage: getMibTbl failed to get cell allocation table err(%d)\n", ret);
      printf("grr_GetOamBtsBasicPackage: getMibTbl failed to get cell allocation table err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  grr_OamData.btsBasicPackage.cellAllocationTable= (Table *) &cellAllocationTable;
	
  //step 3: get plmnPermitted Table
  ret = getMibTbl(MIBT_plmnPermittedEntry, (void *) &plmnPermittedTable,
                  sizeof(plmnPermittedTable));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsBasicPackage: getMibTbl failed to get plmnPermittedTable err(%d)\n", ret);
      printf("grr_GetOamBtsBasicPackage: getMibTbl failed to get plmnPermittedTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }

  grr_OamData.btsBasicPackage.plmnPermittedTable= (Table *) &plmnPermittedTable;

  // Convert mcc to byte string
  getByteAryByInt( grr_OamData.btsBasicPackage.bts_mcc, 
		       (char*)&grr_OamData.btsBasicPackage.bts_mcc, 3);

  // get mnc directly via oam api kevinlim 05/11/01
  oam_getMnc((UINT8 *)&grr_OamData.btsBasicPackage.bts_mnc);

  // Convert rach control parameters from GUI form to internal form
  //
  
  //maxNumberRetransmissions
  i = grr_OamData.btsBasicPackage.maxNumberRetransmissions;
  if (i==1)        grr_OamData.btsBasicPackage.maxNumberRetransmissions=0;
  else if (i==2)   grr_OamData.btsBasicPackage.maxNumberRetransmissions=1;
  else if (i==4)   grr_OamData.btsBasicPackage.maxNumberRetransmissions=2;
  else if (i==7)   grr_OamData.btsBasicPackage.maxNumberRetransmissions=3;
  else 
  {
      DBG_WARNING("grr_GetOamBtsBasicPackage: invalid maxNumberRetransmissions(%d)\n",i);
      grr_OamData.btsBasicPackage.maxNumberRetransmissions=3;  //default to 3
  }
 
  //Tx Integer
  grr_OamData.btsBasicPackage.numberOfSlotsSpreadTrans -= 3;

  //noOfMultiframesBetweenPaging
  grr_OamData.btsBasicPackage.noOfMultiframesBetweenPaging -= 2;
	
  switch(OAMgrr_GSMDCS_INDICATOR)
  {
  case 0: //GSM900
          if (OAMgrr_MS_TX_PWR_MAX_CCH <=39 && OAMgrr_MS_TX_PWR_MAX_CCH >=5)
          {
              i = OAMgrr_MS_TX_PWR_MAX_CCH/2;
              OAMgrr_MS_TX_PWR_MAX_CCHa = 21 - i;
          } else
          {
              DBG_WARNING("grr_GetOamBtsBasicPackage: radioSystem(%d) mSTxPwrMaxCch (%d)\n",
                           OAMgrr_GSMDCS_INDICATOR,
                           OAMgrr_MS_TX_PWR_MAX_CCH);
              OAMgrr_MS_TX_PWR_MAX_CCHa = 7; //29dBm
          }
          break;

  case 1: //DCS1800
          if ( (OAMgrr_MS_TX_PWR_MAX_CCHa<=30) && (OAMgrr_MS_TX_PWR_MAX_CCHa>=0) )
          {
              i = OAMgrr_MS_TX_PWR_MAX_CCH/2;
              //Not needed now
              //if (OAMgrr_MS_PWR_OFFSET>=0 && OAMgrr_MS_PWR_OFFSET<=6)
              //{
              //    OAMgrr_MS_PWR_OFFSETa = OAMgrr_MS_PWR_OFFSET/2;
              //} else
              //    OAMgrr_MS_PWR_OFFSETa = 0;
              OAMgrr_MS_TX_PWR_MAX_CCHa = 15 - i;
          } else
          {
              DBG_WARNING("grr_GetOamBtsBasicPackage: radioSystem(%d) mSTxPwrMaxCch (%d)\n",
                           OAMgrr_GSMDCS_INDICATOR,
                           OAMgrr_MS_TX_PWR_MAX_CCHa);
              OAMgrr_MS_TX_PWR_MAX_CCHa = 3; //24dBm
              //Not needed now
              //OAMgrr_MS_PWR_OFFSETa = 0;
          }
          break;

  case 2: //PCS1900
          if (OAMgrr_MS_TX_PWR_MAX_CCH == 32)
          {
              OAMgrr_MS_TX_PWR_MAX_CCHa = 31; //32dBm
          } else if (OAMgrr_MS_TX_PWR_MAX_CCH == 33)
          {
              OAMgrr_MS_TX_PWR_MAX_CCHa = 31; //33dBm
          } else if ((OAMgrr_MS_TX_PWR_MAX_CCHa<=30) && (OAMgrr_MS_TX_PWR_MAX_CCHa>=0))
          {
              i = OAMgrr_MS_TX_PWR_MAX_CCH/2;
              OAMgrr_MS_TX_PWR_MAX_CCHa = 15 - i;
          } else
          {
              DBG_WARNING("grr_GetOamBtsBasicPackage: radioSystem(%d) mSTxPwrMaxCch (%d)\n",
                           OAMgrr_GSMDCS_INDICATOR,
                           OAMgrr_MS_TX_PWR_MAX_CCH);
              OAMgrr_MS_TX_PWR_MAX_CCH = 3; //24dBm
          }
          break;
            
   default: //Assume a proper value but give debug information
          DBG_WARNING("grr_GetOamBtsBasicPackage: radioSystem(%d) mSTxPwrMaxCch (%d)\n",
                       OAMgrr_GSMDCS_INDICATOR,
                       OAMgrr_MS_TX_PWR_MAX_CCH);
          OAMgrr_MS_TX_PWR_MAX_CCHa = 7;
          //Not needed now
          //OAMgrr_MS_PWR_OFFSETa = 0;
          break;
   }

   DBG_LEAVE();
}

void grr_GetOamBtsOptionPackage(void)
{
  STATUS ret;

  DBG_FUNC("grr_GetOamBtsOptionPackage", GRR_LAYER_OAM);
  DBG_ENTER();
	
  //step 1: get btsOptionsPackage structure first
  ret = getMibStruct(MIB_btsOptionsPackage, (unsigned char*)&grr_OamData.btsOptionsPackage,
                     sizeof(BtsOptionsPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsOptionPackage: getMibStruct failed to get btsOptionsPackage err(%d)\n", ret);
      printf("grr_GetOamBtsOptionPackage: getMibStruct failed to get btsOptionsPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }   
	
  // step 2: get notAllowedAccessClassTable table
  ret = getMibTbl(MIBT_notAllowedAccessClassEntry, (void *) &notAllowedAccessClassTable ,
		      sizeof(notAllowedAccessClassTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsOptionPackage: getMibTbl failed to get notAllowedAccessClassTable err(%d)\n", ret);
      printf("grr_GetOamBtsOptionPackage: getMibTbl failed to get notAllowedAccessClassTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  grr_OamData.btsOptionsPackage.notAllowedAccessClassTable= (Table *) &notAllowedAccessClassTable;
	
  DBG_LEAVE();
}

void grr_GetOamBtsFirstTrxPackage(void)
{
  STATUS ret;

  DBG_FUNC("grr_GetOamBtsFirstTrxPackage", GRR_LAYER_OAM);
  DBG_ENTER();

  //step 1: get transceiverPackage structure first
  ret = getMibStruct(MIB_firstTransceiverPackage, (unsigned char*)&grr_OamData.transceiverPackage[0],
		         sizeof(FirstTransceiverPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsFirstTrxPackage: getMibStruct failed to get firstTransceiverPackage err(%d)\n", ret);
      printf("grr_GetOamBtsFirstTrxPackage: getMibStruct failed to get firstTransceiverPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }   
	
  // step 2: get Channel Table
  ret = getMibTbl(MIBT_channel_0_Entry, (void *) &firstTrxPackageChannelTable ,
		      sizeof(firstTrxPackageChannelTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsFirstTrxPackage: getMibTbl failed to get firstTrxPackageChannelTable err(%d)\n", ret);
      printf("grr_GetOamBtsFirstTrxPackage: getMibTbl failed to get firstTrxPackageChannelTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
		
  grr_OamData.transceiverPackage[0].channelTable= (Table *) &firstTrxPackageChannelTable;
	
  DBG_LEAVE();

}

void grr_GetOamBtsSecondTrxPackage(void)
{
  STATUS ret;

  DBG_FUNC("grr_GetOamBtsSecondTrxPackage", GRR_LAYER_OAM);
  DBG_ENTER();

  //step 1: get transceiverPackage structure first
  ret = getMibStruct(MIB_secondTransceiverPackage, (unsigned char*)&grr_OamData.transceiverPackage[1],
		         sizeof(SecondTransceiverPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsSecondTrxPackage: getMibStruct failed to get secondTransceiverPackage err(%d)\n", ret);
      printf("grr_GetOamBtsSecondTrxPackage: getMibStruct failed to get secondTransceiverPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }   
	
  // step 2: get the Channel Table
  ret = getMibTbl(MIBT_channel_1_Entry, (void *) &secondTrxPackageChannelTable ,
		      sizeof(secondTrxPackageChannelTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsSecondTrxPackage: getMibTbl failed to get secondTrxPackageChannelTable err(%d)\n", ret);
      printf("grr_GetOamBtsSecondTrxPackage: getMibTbl failed to get secondTrxPackageChannelTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
		
  grr_OamData.transceiverPackage[1].channelTable= (Table *) &secondTrxPackageChannelTable;
	
  DBG_LEAVE();

}

void grr_GetOamBtsFirstRadioCarrierPackage(void)
{
  STATUS ret;

  DBG_FUNC("grr_GetOamBtsFirstRadioCarrierPackage", GRR_LAYER_OAM);
  DBG_ENTER();

  //step 1: get the 2nd Radio Carrier Package first
  ret = getMibStruct(MIB_firstRadioCarrierPackage, (unsigned char*) &grr_OamData.radioCarrierPackage[0],
		         sizeof(FirstRadioCarrierPackage));
  if (ret != STATUS_OK)  
  { 
      DBG_ERROR("grr_GetOamBtsFirstRadioCarrierPackage: getMibStruct failed to get firstRadioCarrierPackage err(%d)\n", ret);
      printf("grr_GetOamBtsFirstRadioCarrierPackage: getMibStruct failed to get firstRadioCarrierPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  //step 2: get the radio frequency list for the first carrier package
  ret = getMibTbl(MIBT_carrierFrequency_0_Entry, (void *) &firstRadioCarrierPackageFrequencyListTable ,
		      sizeof(firstRadioCarrierPackageFrequencyListTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsFirstRadioCarrierPackage: getMibTbl failed to get firstRadioCarrierPackageFrequencyListTable err(%d)\n", ret);
      printf("grr_GetOamBtsFirstRadioCarrierPackage: getMibTbl failed to get firstRadioCarrierPackageFrequencyListTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  grr_OamData.radioCarrierPackage[0].carrierFrequencyList= (Table *) &firstRadioCarrierPackageFrequencyListTable;

  DBG_LEAVE();

}

void grr_GetOamBtsSecondRadioCarrierPackage(void)
{
  STATUS ret;

  DBG_FUNC("grr_GetOamBtsSecondRadioCarrierPackage", GRR_LAYER_OAM);
  DBG_ENTER();
	
  //step 1: get the 2nd Radio Carrier Package first
  ret = getMibStruct(MIB_secondRadioCarrierPackage, (unsigned char*) &grr_OamData.radioCarrierPackage[1],
		         sizeof(SecondRadioCarrierPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsSecondRadioCarrierPackage: getMibStruct failed to get secondRadioCarrierPackage err(%d)\n", ret);
      printf("grr_GetOamBtsSecondRadioCarrierPackage: getMibStruct failed to get secondRadioCarrierPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }

  //step 2: get the radio frequency list for the first carrier package
  ret = getMibTbl(MIBT_carrierFrequency_1_Entry, (void *) &secondRadioCarrierPackageFrequencyListTable ,
		      sizeof(secondRadioCarrierPackageFrequencyListTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamBtsSecondRadioCarrierPackage: getMibTbl failed to get secondRadioCarrierPackageFrequencyListTable err(%d)\n", ret);
      printf("grr_GetOamBtsSecondRadioCarrierPackage: getMibTbl failed to get secondRadioCarrierPackageFrequencyListTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
		
  grr_OamData.radioCarrierPackage[1].carrierFrequencyList= (Table *) &secondRadioCarrierPackageFrequencyListTable;
	
  DBG_LEAVE();
}

void grr_GetOamT31xxTimerStructure(void)
{
  STATUS    ret;
  int 	msPerTick;

  DBG_FUNC("grr_GetOamT31xxTimerStructure", GRR_LAYER_OAM);
  DBG_ENTER();
	
  ret = getMibStruct(MIB_t31xxPackage, (unsigned char*)&grr_OamData.t31xx,
		         sizeof(T31xx));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamT31xxTimerStructure: getMibStruct failed to get t31xx err(%d)\n", ret);
      printf("grr_GetOamT31xxTimerStructure: getMibStruct failed to get t31xx err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }   
	
  msPerTick = 1000/sysClkRateGet();
  grr_OamData.t31xx.t3101 = (grr_OamData.t31xx.t3101*10)/msPerTick;
  grr_OamData.t31xx.t3105 = (grr_OamData.t31xx.t3105*10)/msPerTick;	
  grr_OamData.t31xx.t3109 = (grr_OamData.t31xx.t3109*10)/msPerTick;	
  grr_OamData.t31xx.t3111 = (grr_OamData.t31xx.t3111*10)/msPerTick;	

  DBG_LEAVE();
}

void grr_GetOamAdjacentCellPackage(void)
{
  STATUS ret;
  int i,j;

  DBG_FUNC("grr_GetOamAdjacentCellPackage", GRR_LAYER_OAM);
  DBG_ENTER();

  //step 1: get adjacent cell Package first
  ret = getMibStruct(MIB_adjacentCellPackage, (unsigned char*)&grr_OamData.adjacentCellPackage,
		         sizeof(AdjacentCellPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamAdjacentCellPackage: getMibStruct failed to get adjacentCellPackage err(%d)\n", ret);
      printf("grr_GetOamAdjacentCellPackage: getMibStruct failed to get adjacentCellPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  //step 2: get handover table list
  ret = getMibTbl(MIBT_adjCell_handoverEntry, (void *) &adjCell_HandoverTable,
		      sizeof(adjCell_HandoverTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamAdjacentCellPackage: getMibTbl failed to get adjCell_HandoverTable err(%d)\n", ret);
      printf("grr_GetOamAdjacentCellPackage: getMibTbl failed to get adjCell_HandoverTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  grr_OamData.adjacentCellPackage.adjCell_handoverTable= (Table *) &adjCell_HandoverTable;
  
  //Compress all empty rows in the handover adjacent cell table
  i=0;
  while (i<OAMgrr_HO_ADJCELL_NO_MAX)
  {
      if (OAMgrr_HO_ADJCELL_ID(i)==OAMgrr_HO_ADJCELL_ID_NULL)
      {
          for (j=i+1;j<OAMgrr_HO_ADJCELL_NO_MAX;j++)
          {
               if (OAMgrr_HO_ADJCELL_ID(j) != OAMgrr_HO_ADJCELL_ID_NULL)
               {
                   memcpy(&adjCell_HandoverTable[i], &adjCell_HandoverTable[j],
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
	
  //step 3: get reslection table list
  ret = getMibTbl(MIBT_adjCell_reselectionEntry, (void *) &adjCell_ReselectionTable,
	 	      sizeof(adjCell_ReselectionTable));
	
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamAdjacentCellPackage: getMibTbl failed to get adjCell_ReselectionTable err(%d)\n", ret);
      printf("grr_GetOamAdjacentCellPackage: getMibTbl failed to get adjCell_ReselectionTable err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBTBL_FAILED);
      DBG_LEAVE();
      assert(0);
  }
	
  grr_OamData.adjacentCellPackage.adjCell_reselectionTable= (Table *) &adjCell_ReselectionTable;

  //Compress all empty rows in the reselection adjacent cell table
  i=0;
  while (i<OAMgrr_RESEL_ADJCELL_NO_MAX)
  {
      if (OAMgrr_RESEL_ADJCELL_ID(i)==OAMgrr_RESEL_ADJCELL_ID_NULL)
      {
          for (j=i+1;j<OAMgrr_RESEL_ADJCELL_NO_MAX;j++)
          {
               if (OAMgrr_RESEL_ADJCELL_ID(j) != OAMgrr_RESEL_ADJCELL_ID_NULL)
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
 
  DBG_LEAVE();
}


void grr_GetOamHandoverControlPackage(void)
{
  STATUS ret;
  
  DBG_FUNC("grr_GetOamHandoverControlPackage", GRR_LAYER_OAM);
  DBG_ENTER();

  ret = getMibStruct(MIB_handoverControlPackage, (unsigned char*)&grr_OamData.handoverControlPackage,
		         sizeof(HandoverControlPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamHandoverControlPackage: getMibStruct failed to get handoverControlPackage err(%d)\n", ret);
      printf("grr_GetOamHandoverControlPackage: getMibStruct failed to get handoverControlPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  } 

  DBG_LEAVE();
	
}

void grr_GetOamPowerControlPackage(void)
{
  STATUS ret;

  DBG_FUNC("grr_GetOamPowerControlPackage", GRR_LAYER_OAM);
  DBG_ENTER();
	
  ret = getMibStruct(MIB_powerControlPackage, (unsigned char*)&grr_OamData.powerControlPackage,
		         sizeof(PowerControlPackage));
  if (ret != STATUS_OK)  
  {
      DBG_ERROR("grr_GetOamPowerControlPackage: getMibStruct failed to get powerControlPackage err(%d)\n", ret);
      printf("grr_GetOamPowerControlPackage: getMibStruct failed to get powerControlPackage err(%d)\n", ret);
      grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
      DBG_LEAVE();
      assert(0);
  }

  DBG_LEAVE();
	
}

//Get standard GSM MIB data
void grr_GetOamGsmData(void)
{
     DBG_FUNC("grr_GetOamGsmData", GRR_LAYER_OAM);
     DBG_ENTER();

     // Initialize Table area
     GRR_MEMSET(&cellAllocationTable, sizeof(cellAllocationTable));
     GRR_MEMSET(&plmnPermittedTable, sizeof(plmnPermittedTable));
     GRR_MEMSET(&notAllowedAccessClassTable, sizeof(notAllowedAccessClassTable));
     GRR_MEMSET(&firstTrxPackageChannelTable, sizeof(firstTrxPackageChannelTable));
     GRR_MEMSET(&secondTrxPackageChannelTable, sizeof(secondTrxPackageChannelTable));

     GRR_MEMSET(&firstRadioCarrierPackageFrequencyListTable, sizeof(firstRadioCarrierPackageFrequencyListTable));
     GRR_MEMSET(&secondRadioCarrierPackageFrequencyListTable, sizeof(secondRadioCarrierPackageFrequencyListTable));
     GRR_MEMSET(&adjCell_HandoverTable, sizeof(adjCell_HandoverTable));
     GRR_MEMSET(&adjCell_ReselectionTable, sizeof(adjCell_ReselectionTable));

     GRR_MEMSET(&adjCell_HandoverTable_t,sizeof(adjCell_HandoverTable));
     grr_OamData.adjacentCellPackage_t.adjCell_handoverTable =  (Table *) &adjCell_HandoverTable_t;

     grr_GetOamBtsBasicPackage();
     grr_GetOamBtsOptionPackage();
     grr_GetOamBtsFirstTrxPackage();
     grr_GetOamBtsSecondTrxPackage();
     grr_GetOamBtsFirstRadioCarrierPackage();
     grr_GetOamBtsSecondRadioCarrierPackage();
     grr_GetOamT31xxTimerStructure();
     grr_GetOamAdjacentCellPackage();
     grr_GetOamHandoverControlPackage();
     grr_GetOamPowerControlPackage();

     DBG_LEAVE();
}

void grr_GetOamRrmData(void)
{
   STATUS ret;

   DBG_FUNC("grr_GetOamRrmData", GRR_LAYER_OAM);
   DBG_ENTER();
   
   ret = getMibStruct(MIB_resourceConfigData, (unsigned char*)&grr_OamData.grrConfigData,
                      sizeof(ResourceConfigData));

   if (ret != STATUS_OK)
   {
       DBG_ERROR("grr_GetOamRrmData: getMibStruct failed to get oam rrm data, err(%d)\n",ret);
       printf("grr_GetOamRrmData: getMibStruct failed to get oam rrm data, err(%d)\n",ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED);
       assert(0);
   }

   DBG_LEAVE();
}

void grr_GetOamData(void)
{
   DBG_FUNC("grr_GetOamData", GRR_LAYER_OAM);
   DBG_ENTER();

   GRR_MEMSET(&grr_OamData, sizeof(grr_OamData));

   //Get OAM data    
   grr_GetOamGsmData();
   grr_GetOamRrmData();
    
   DBG_LEAVE();
}

#endif //__GRR_GETOAMMIB_CPP__

