/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_TESTPRINTOAMRMSTDGSMDATA_CPP__
#define __RM_TESTPRINTOAMRMSTDGSMDATA_CPP__

#include "rm\rm_head.h"

void rm_TestPrintOamRmStdGsmData(void)
{
int i,j;

/* PR1256 <convert from tick number to 10ms > begin*/

printf("grr_OamData.t31xx.t3101 = %d---%d\n",OAMrm_T3101,grr_OamData.t31xx.t3101);
printf("grr_OamData.t31xx.t3105 = %d---%d\n",OAMrm_T3105,grr_OamData.t31xx.t3105);
printf("grr_OamData.t31xx.t3109 = %d---%d\n",OAMrm_T3109,grr_OamData.t31xx.t3109);
printf("grr_OamData.t31xx.t3111 = %d---%d\n",OAMrm_T3111,grr_OamData.t31xx.t3111);

/* PR1256 <convert from tick number to 10ms > end*/

for (i=0;i<2;i++)
printf("OAMrm_NARFCN(%d):%d,%d\n", 
        i,
        OAMrm_NARFCN(i), 
        ((signed short)(((AdjCell_HandoverEntry*) 
        (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_bCCHFrequency))
      );
      //561: neighboring cell frequency for monitoring for handover purpose

printf("OAMrm_NY1:%d,%d\n", 
        OAMrm_NY1,
        ((unsigned char)(grr_OamData.btsBasicPackage.ny1))
      );
      //3
printf("OAMrm_BCCH_ARFCN:%d,%d\n",
       OAMrm_BCCH_ARFCN,
       (((CellAllocationEntry*)
       (grr_OamData.btsBasicPackage.cellAllocationTable))[0].cellAllocation)
      );
      //661:1st element of cellAllocation table is BCCH ARFCN, oam_BcchArfcn

printf("OAMrm_NCC:%d,%d\n",
        OAMrm_NCC,
        ((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc))
      );
	//5:oam_Ncc, bts_ncc may be combined with 
printf("OAMrm_BCC:%d,%d\n",
        OAMrm_BCC,
        ((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
      );
	//5:oam_Bcc
printf("OAMrm_MS_TX_PWR_MAX_CCH:%d,%d\n",
        OAMrm_MS_TX_PWR_MAX_CCH,
        ((unsigned char)(grr_OamData.btsBasicPackage.mSTxPwrMaxCCH))
      );
      //0:30dBm, previous value is 11 i.e. 8dBm, oam_MsTxPwrMax
printf("OAMrm_MS_RELEV_ACCESS_MIN:%d,%d\n",
        OAMrm_MS_RELEV_ACCESS_MIN,
        ((unsigned char)(grr_OamData.btsBasicPackage.rxLevAccessMin))
      );
      //6: 104dBm
printf("OAMrm_BTS_BSIC:%d,%d\n",
        OAMrm_BTS_BSIC,
        ((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc<<3)| 
         (unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
       );
       //0x2d: oam_BTSBsic
printf("OAMrm_BS_AG_BLK_RES:%d,%d\n", 
        OAMrm_BS_AG_BLK_RES,
        ((unsigned char)(grr_OamData.btsBasicPackage.noOfBlocksForAccessGrant))
       );
       //0:oam_bsAgBlkRes
printf("OAMrm_BS_PA_MFRMS:%d,%d\n",
        OAMrm_BS_PA_MFRMS,
        ((unsigned char)(grr_OamData.btsBasicPackage.noOfMultiframesBetweenPaging))
       );
	 //1:oam_bsPaMfrms
printf("OAMrm_T3212:%d,%d\n",
        OAMrm_T3212, 
        ((unsigned char)(grr_OamData.btsOptionsPackage.timerPeriodicUpdateMS))
       );
       //0:no periodic LUP needed, oam_T3212
for (i=0;i<3;i++)
printf("OAMrm_MCC(%d): %d,%d\n",
        i,
        OAMrm_MCC(i),
        ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mcc))[i]
       );
       //001: bts_mcc is an integer which will be converted to char string, oam_Mcc
for (i=0;i<2;i++)
printf("OAMrm_MNC(%d):%d,%d,",
        i,
        OAMrm_MNC(i),
        ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[i]
       );
if(((unsigned char *)&(grr_OamData.btsBasicPackage.bts_mnc))[2] == 0x0F)
	printf("F\n");
else
	printf("%d\n",
        ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[2]
		);
       //01: bts_mnc is an integer which will be converted to char string, oam_Mnc
printf("OAMrm_LAC:%x,%x\n",
        OAMrm_LAC, 
        ((short)(grr_OamData.btsBasicPackage.bts_lac))
       ); 
       //1:oam_Lac[X]
printf("OAMrm_T3105:%x,%x\n",
        OAMrm_T3105, 
        ((int)(grr_OamData.t31xx.t3105))
       );
       //2:16.6 by 2 ms
printf("OAMrm_T3101:%x,%x\n", 
        OAMrm_T3101, 
        ((int)(grr_OamData.t31xx.t3101))
       );
	 //0x2FF:oam_T3101
printf("OAMrm_T3109:%x,%x\n", 
        OAMrm_T3109, 
        ((int)(grr_OamData.t31xx.t3109))
       );
	 //0x1FF:oam_T3109
printf("OAMrm_T3111:%x,%x\n", 
        OAMrm_T3111, 
        ((int)(grr_OamData.t31xx.t3111))
       );
	 //0x2FF:oam_T3111
for(j=0;j<8;j++)
printf("OAMrm_TRX_SLOT_COMB(0,%d):%d,%d\n",
        j, 
        OAMrm_TRX_SLOT_COMB(0,j),
        ((unsigned char)(((ChannelEntry*) 
        (grr_OamData.transceiverPackage[0].channelTable))[j].channelCombination))
       );
	 //5 for (X,Y)=(0,0), 1 for other (X,Y) pairs
for(j=0;j<8;j++)
printf("OAMrm_TRX_SLOT_COMB(1,%d):%d,%d\n",
        j, 
        OAMrm_TRX_SLOT_COMB(1,j),
        ((unsigned char)(((ChannelEntry*) 
        (grr_OamData.transceiverPackage[1].channelTable))[j].channelCombination))
       );

for (i=0;i<2;i++)
printf("OAMrm_ARFCN(%d):%d,%d,%d\n", 
        i,
        OAMrm_ARFCN(i), 
		grr_OamData.transceiverPackage[i].relatedRadioCarrier,
        ((unsigned short)(((CarrierFrequencyEntry*)(grr_OamData.radioCarrierPackage[
        grr_OamData.transceiverPackage[i].relatedRadioCarrier
        ].carrierFrequencyList))[0].carrierFrequency))
		);
        //661 for X=0; 561 for X=1, X:0-1
for (i=0;i<3;i++)
printf("OAMrm_HO_ADJCELL_BCCH_ARFCNv(%d):%d,%d\n",
        i, 
        OAMrm_HO_ADJCELL_BCCH_ARFCN(i), 
        ((short)(((AdjCell_HandoverEntry*) 
        (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_bCCHFrequency))
       );
       //561 for X=0,761 for X=1, 0 for other X values
for (i=0;i<3;i++)
printf("OAMrm_HO_ADJCELL_BCCH_ARFCNi(%d):%d,%d\n",
        i, 
        OAMrm_HO_ADJCELL_BCCH_ARFCN_t(i), 
        ((short)(((AdjCell_HandoverEntry*) 
        (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_bCCHFrequency))
       );

for (i=0;i<4;i++)
printf("OAMrm_HO_ADJCELL_IDv(%d):%d,%d\n",
        i,
        OAMrm_HO_ADJCELL_ID(i),
        ((unsigned int)(((AdjCell_HandoverEntry*) 
        (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[i].adjCell_handoverCellID))
       );

//PR1223 Begin
for (i=0;i<4;i++)
printf("OAMrm_HO_ADJCELL_IDi(%d):%d,%d\n",
        i,
        OAMrm_HO_ADJCELL_ID_t(i),
        ((unsigned int)(((AdjCell_HandoverEntry*) 
        (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[i].adjCell_handoverCellID))
       );
//PR1223 End


for (i=0;i<3;i++)
printf("OAMrm_RESEL_ADJCELL_BCCH_ARFCN(%d):%d,%d\n", 
        i,
        OAMrm_RESEL_ADJCELL_BCCH_ARFCN(i), 
        ((unsigned int)(((AdjCell_ReselectionEntry*) 
        (grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[i].adjCell_reselectionBCCHFrequency))
       );
       //561 for X=0,761 for X=1, 0 for other X values
for (i=0;i<3;i++)
printf("OAMrm_RESEL_ADJCELL_ID(%d):%d,%d\n",
        i, 
        OAMrm_RESEL_ADJCELL_ID(i), 
        ((unsigned int)(((AdjCell_ReselectionEntry*) 
        (grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[i].adjCell_reselectionCellID))
       ); 
       //2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values
printf("OAMrm_RA_RETRY_MAX:%d,%d\n", 
        OAMrm_RA_RETRY_MAX, 
        ((unsigned char)(grr_OamData.btsBasicPackage.maxNumberRetransmissions))
       );
       //3:retry 7 times
printf("OAMrm_RA_TX_INTEGER:%d,%d\n", 
        OAMrm_RA_TX_INTEGER, 
        ((unsigned char)(grr_OamData.btsBasicPackage.numberOfSlotsSpreadTrans))
       );
       //7:spread 10 time slots
printf("OAMrm_RA_CELL_BARRED_STATE:%d,%d\n", 
        OAMrm_RA_CELL_BARRED_STATE, 
        ((T_CNI_RIL3_CELL_BARRED_ACCESS)(grr_OamData.btsOptionsPackage.cellBarred))
       );
       //0:not barred
printf("OAMrm_ATT_STATE:%d,%d\n",
        OAMrm_ATT_STATE,
        ((T_CNI_RIL3_ATT)(grr_OamData.btsOptionsPackage.allowIMSIAttachDetach))
       );
       //1:ie CNI_RIL3_ATT_IMSI_ATACH_DETACH_ALLOWED,oam_att
printf("OAMrm_RA_REST_ALLOWED_STATE:%d,%d\n", 
        OAMrm_RA_REST_ALLOWED_STATE, 
        ((T_CNI_RIL3_REESTABLISHMENT_ALLOWED) 
        (grr_OamData.btsOptionsPackage.callReestablishmentAllowed))
       );
       //1:not allowed
printf("OAMrm_RA_EC_ALLOWED_STATE:%d,%d\n", 
        OAMrm_RA_EC_ALLOWED_STATE, 
        ((T_CNI_RIL3_EMERGENCY_CALL_ALLOWED) 
        (grr_OamData.btsOptionsPackage.emergencyCallRestricted))
       );
       //1:only allowed to special class mobiles
for (i=0;i<16;i++)
printf("OAMrm_RA_AC_ALLOWED_STATE(%d):%d,%d\n", 
	    i,
        OAMrm_RA_AC_ALLOWED_STATE(i), 
        ((unsigned char)(((NotAllowedAccessClassEntry*)
        (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[i].notAllowedAccessClass))
       );
       //1:barred
for (i=0;i<8;i++)
printf("OAMrm_NCC_PERMITTED(%d):%d,%d\n",
        i, 
        OAMrm_NCC_PERMITTED(i), 
        (((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[i]. 
           plmnPermitted)
       );
	 //1 for X=5, 0 for other X values, X:0-7
printf("OAMrm_CELLID:%x,%x\n", 
        OAMrm_CELLID, 
        ((unsigned int)(grr_OamData.btsBasicPackage.btsID))
       );
	 //0x0001: hi-byte network ID, lo-byte cell ID

printf("OAMrm_RADIO_LINK_TIMEOUT:%d,%d\n", 
        OAMrm_RADIO_LINK_TIMEOUT, 
        ((unsigned char)(grr_OamData.btsBasicPackage.radioLinkTimeout))
       );
       //3: ie 16
printf("OAMrm_MS_UPLINK_DTX_STATE:%d,%d\n", 
        OAMrm_MS_UPLINK_DTX_STATE, 
        ((T_CNI_RIL3_DTX_BCCH)(grr_OamData.btsOptionsPackage.dtxUplink))
       );

printf("OAMrm_MS_DNLINK_DTX_STATE:%d,%d\n", 
        OAMrm_MS_DNLINK_DTX_STATE, 
        ((int)(grr_OamData.btsOptionsPackage.dtxDownlink))
       );

       //2:uplink DTX not allowed for MS
printf("OAMrm_CELL_RESEL_HYSTERESIS:%d,%d\n", 
        OAMrm_CELL_RESEL_HYSTERESIS, 
        ((unsigned char)(grr_OamData.btsBasicPackage.cellReselectHysteresis))
       );
       //2:4 dBm

printf("OAMrm_NCELL_NETWORKID(0):%d, %d\n",
        OAMrm_NCELL_NETWORKID(0),
        ((short)((((AdjCell_HandoverEntry*)
        (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[0].adjCell_handoverCellID))>>16)
       );  
       //2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
printf("OAMrm_NCELL_CELLID(1):%d,%d\n",
        OAMrm_NCELL_CELLID(1),
        ((short)(((AdjCell_HandoverEntry*)
        (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[1].adjCell_handoverCellID))
       );
       //2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
printf("OAMrm_NCELL_CELLID(2):%d,%d\n",
        OAMrm_NCELL_CELLID(2),
        ((short)(((AdjCell_HandoverEntry*)
        (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[2].adjCell_handoverCellID))
       );
       //2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31


//Bts Admin and Op States
printf ("BTS AM & OP STATE: am(%d, %d) op(%d, %d)\n",
         OAMrm_BTS_AMSTATE,
            grr_OamData.btsBasicPackage.bts_administrativeState,
         OAMrm_BTS_OPSTATE,
            grr_OamData.btsBasicPackage.bts_operationalState);

//First TRX Admin and Op States
printf ("First TRX AM & OP STATE: am(%d, %d) op(%d, %d)\n",
         OAMrm_TRX_AMSTATE(0),
         grr_OamData.transceiverPackage[0].basebandAdministrativeState,
         OAMrm_TRX_OPSTATE(0), 
         grr_OamData.transceiverPackage[0].basebandOperationalState);

for (i=0;i<8;i++)
{
printf ("First TRX TS(%d) AM & OP STATE: am(%d, %d) op(%d, %d)\n",
         i,
         OAMrm_CHN_AMSTATE(0,i),
         ((ChannelEntry*)
          (grr_OamData.transceiverPackage[0].channelTable))[i].channelAdministrativeState,
         OAMrm_CHN_OPSTATE(0,i),
         ((EnableDisable)(((ChannelEntry*)
          (grr_OamData.transceiverPackage[0].channelTable))[i].channelOperationalState)) );
}

//Second TRX Admin and Op States
printf ("Second TRX AM & OP STATE: am(%d, %d) op(%d, %d)\n",
         OAMrm_TRX_AMSTATE(1),
         grr_OamData.transceiverPackage[1].basebandAdministrativeState,
         OAMrm_TRX_OPSTATE(1), 
         grr_OamData.transceiverPackage[1].basebandOperationalState);

for (i=0;i<8;i++)
{
printf ("Second TRX TS(%d) AM & OP STATE: am(%d, %d) op(%d, %d)\n",
         i,
         OAMrm_CHN_AMSTATE(1,i),
         ((ChannelEntry*)
          (grr_OamData.transceiverPackage[1].channelTable))[i].channelAdministrativeState,
         OAMrm_CHN_OPSTATE(1,i),
         ((EnableDisable)(((ChannelEntry*)
          (grr_OamData.transceiverPackage[1].channelTable))[i].channelOperationalState)) );
}

printf("Overload Cell Ctrl=%d, MIB Barred Setting=%d, Current Barred Control status=%d\n",
        OAMrm_OVERLOAD_CELLBAR,
        OAMrm_RA_CELL_BARRED_STATE, rm_VcIsBarred);

}

void intg_SetOverloadCellbarOption(int option)
{
    printf("Current Overload Cellbar Option Setting: %d\n", OAMrm_OVERLOAD_CELLBAR);
    OAMrm_OVERLOAD_CELLBAR = option;
    printf("New Overload Cellbar Option Setting: %d\n", OAMrm_OVERLOAD_CELLBAR);

    rm_VcIsBarred = RM_FALSE;
    rm_SendSystemInfo2ToBcchTrx(0,2,OAMrm_RA_CELL_BARRED_STATE);        
    rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);        
    rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
}

#endif /*__RM_TESTPRINTOAMRMSTDGSMDATA_CPP__*/
