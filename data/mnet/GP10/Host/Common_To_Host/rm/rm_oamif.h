/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __RM_OAMIF_H__
#define __RM_OAMIF_H__
    
#include "rm_types.h"
#include "rm_const.h"
#include "rm_struct.h"
#include "ril3\ril3_common.h"
#include "mibtags.h"
#include "vipermib.h"
#include "oam_api.h"
#include "AlarmCode.h"
#include "jcc\JCCbcCb.h"

#include "grr\grr_data.h"
    
//
// Slot combination: Mapped to definition given in OAM vipermib.h
//
#define OAMrm_SLOT_COMB_1	     	        tCHFull				//0x01 Type I
#define OAMrm_SLOT_COMB_5	  				bCCHCombined	    //0x05 Type V
#define OAMrm_SLOT_BEACON	       			OAMrm_SLOT_COMB_5	//0x05 Beacon
#define OAMrm_SLOT_UNUSED					0xFF   				//0xFF Unused

#define OAMrm_SLOT_COMB_11				pBCCH				//0x0B type xi
#define OAMrm_SLOT_COMB_12             pCCCH				//0x0C type xii
#define OAMrm_SLOT_COMB_13				pDCHData			//0x0D type xiii

//
// NIC setting values: RM internal use only right now. Only VOIP is used in fact
//
#define OAMrm_NIC_VOIP						0x00
#define OAMrm_NIC_SRGSMT1					0x01
#define OAMrm_NIC_FRGSMT1					0x02
#define OAMrm_NIC_SRGSME1					0x05
#define OAMrm_NIC_FRGSME1					0x06
    
#define OAMrm_HO_ADJCELL_NO_MAX  			16
#define OAMrm_RESEL_ADJCELL_NO_MAX  		16
#define OAMrm_NCC_PERMITTED_MAX 	  		8
#define OAMrm_HO_ADJCELL_ID_NULL    		0
#define OAMrm_RESEL_ADJCELL_ID_NULL 		0
    
    
#define OAMrm_TRX0_SYNTH_A					0x0000			//b12:power switch:0-off 1-on
															//b13:freqhop enable:0-off 1-on
															//b14:trx select:0-trx0 1-trx1
     														//b15:synth select:0-A 1-B
#define OAMrm_TRX1_SYNTH_A					0x4000			//b12:power switch:0-off 1-on
															//b13:freqhop enable:0-off 1-on
															//b14:trx select:0-trx0 1-trx1
     														//b15:synth select:0-A 1-B
#define OAMrm_TRX1_SYNTH_B					0xC000			//b12:power switch:0-off 1-on
															//b13:freqhop enable:0-off 1-on
															//b14:trx select:0-trx0 1-trx1
     														//b15:synth select:0-A 1-B
   
#define OAMrm_GSMCODEC						oam_gsmCodec;
extern  u8         							oam_gsmCodec;
extern  u16 								oam_TrxTuneSynthSetting[2][2];
    
//AlarmAdded
extern  int									rm_LiveSglCfg;
#define OAMrm_MAX_TRXS						2
#define OAMrm_TWDOG							0x1D4C  //2 min
typedef struct {
					u8 tdma1;
					u8 tdma2;
					u8 tdma3;
					u8 tdma4;
					u8 done;
					u8 slot;
					u8 state;
					u8 alive;
					rm_Timer_t TWDOG;

			} rm_TrxMgmt_t;
    
typedef struct {
					u8 totPage;
					u8 curPage;
					u8 morPage;
					u8 data[INTER_CBC_CB_MAX_DAT_LEN];

			} rm_SmscbcMsg_t;
  
extern rm_TrxMgmt_t							rm_TrxMgmt[OAMrm_MAX_TRXS];
extern int									rm_OpState;
extern int									rm_NoOfUsableTchf;
extern int									rm_nbrOfAvailableSDCCHs;
extern int									rm_maxNbrOfBusySDCCHs;
extern int									rm_maxNbrOfBusyTCHs;
    
//AMADDED
extern T_CNI_RIL3_CELL_BARRED_ACCESS		rm_CellBarred;
extern AdministrativeState					rm_AmState00;
extern int									rm_VcIsBarred;

//PMADDED
extern int									rm_NoOfBusySdcch4;
extern int									rm_NoOfBusyTchf;
extern int 									rm_NoOfIdleTchf;
    
typedef struct {
					int allocated;
					int startTime;

			} rm_AllChanAvailable_t;
    
extern rm_AllChanAvailable_t				rm_AllAvailableTchf;  
extern rm_AllChanAvailable_t				rm_AllAvailableSdcch4;
    
typedef struct { 
					BtsBasicPackage			btsBasicPackage;
					BtsOptionsPackage		btsOptionsPackage;
					TransceiverPackage		transceiverPackage[2];
			        RadioCarrierPackage		radioCarrierPackage[2];
			        T31xx					t31xx;
					AdjacentCellPackage		adjacentCellPackage;
    				AdjacentCellPackage		adjacentCellPackage_t;
			        HandoverControlPackage	handoverControlPackage;	
					PowerControlPackage		powerControlPackage;
					ResourceConfigData  	grrConfigData;
					int						rm_IQSwap;

			} rm_OamData_t;

//GP2  
//extern rm_OamData_t   grr_OamData;
    
//
// BtsBasicPackage
//
  
//1. info
//					<NA>

//2. bts_ncc
#define OAMrm_NCC	\
					((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc))
    				//5:oam_Ncc, bts_ncc may be combined with 
//3. bts_cid
#define OAMrm_BCC	\
					((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
    				//5:oam_Bcc

//3a. comb of 2&3:(bts_ncc<<3)|(bts_cid)
#define OAMrm_BTS_BSIC  \
					((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc<<3)| \
					(unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
					//0x2d: oam_BTSBsic
//4. btsID
#define OAMrm_CELLID \
					((unsigned int)(grr_OamData.btsBasicPackage.btsID))
    				//0x0001: hi-byte network ID, lo-byte cell ID

//5. cellAllocationTable[0]
#define OAMrm_BCCH_ARFCN \
    				(((CellAllocationEntry*) \
					(grr_OamData.btsBasicPackage.cellAllocationTable))[0].cellAllocation)
					//661:1st element of cellAllocation table is BCCH ARFCN, oam_BcchArfcn
//6. gsmdcsIndicator
#define OAMrm_GSMDCS_INDICATOR \
					grr_OamData.btsBasicPackage.gsmdcsIndicator

//7. bts_mcc
#define OAMrm_MCC(X) \
					((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mcc))[X]
					//001: bts_mcc is an integer which will be converted to char string, oam_Mcc
//8. bts_mnc
#define OAMrm_MNC(X) \
					((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[X]
					//01: bts_mnc is an integer which will be converted to char string, oam_Mnc
//9. bts_lac
#define OAMrm_LAC	 \
					((short)(grr_OamData.btsBasicPackage.bts_lac))
					//1:oam_Lac[X]
//10. cellReselectHysteresis
#define OAMrm_CELL_RESEL_HYSTERESIS \
					((unsigned char)(grr_OamData.btsBasicPackage.cellReselectHysteresis))
					//2:4 dBm
//11. ny1
#define OAMrm_NY1	\
					((unsigned char)(grr_OamData.btsBasicPackage.ny1))
					//3
//12. plmnPermittedTable
#define OAMrm_NCC_PERMITTED(X) \
					(((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[X]. \
					plmnPermitted)
    				//1 for X=5, 0 for other X values, X:0-7
//13. radioLinkTimeout
#define OAMrm_RADIO_LINK_TIMEOUT \
					((unsigned char)(grr_OamData.btsBasicPackage.radioLinkTimeout))
					//3: ie 16
//14. relatedTranscoder
//					<NA>

//15. rxLevAccessMin
#define OAMrm_MS_RELEV_ACCESS_MIN \
					((unsigned char)(grr_OamData.btsBasicPackage.rxLevAccessMin))
					//6:104dBm
 //16. bts_administrativeState
#define OAMrm_BTS_AMSTATE \
					grr_OamData.btsBasicPackage.bts_administrativeState
					//Controled by administrator
//17. bts_alarmStatus
//					<NA>

//18. bts_operationalState
#define OAMrm_BTS_OPSTATE \
					grr_OamData.btsBasicPackage.bts_operationalState
					//not decided by RM
//19. maxNumberRetransmissions
#define OAMrm_RA_RETRY_MAX \
					((unsigned char)(grr_OamData.btsBasicPackage.maxNumberRetransmissions))
					//3:retry 7 times
//20. mSTxPwrMaxCCH
#define OAMrm_MS_TX_PWR_MAX_CCH \
					((unsigned char)(grr_OamData.btsBasicPackage.mSTxPwrMaxCCH))
					//0:30dBm, previous value is 11 i.e. 8dBm, oam_MsTxPwrMax

//20a. PR1101
#define OAMrm_MS_TX_PWR_MAX_CCHa \
					grr_OamData.btsBasicPackage.mSTxPwrMaxCCH
					//0:30dBm, previous value is 11 i.e. 8dBm, oam_MsTxPwrMax

//21. numberOfSlotsSpreadTrans
#define OAMrm_RA_TX_INTEGER \
					((unsigned char)(grr_OamData.btsBasicPackage.numberOfSlotsSpreadTrans))
					//7:spread 10 time slots
//22. noOfBlocksForAccessGrant
#define OAMrm_BS_AG_BLK_RES \
					((unsigned char)(grr_OamData.btsBasicPackage.noOfBlocksForAccessGrant))
            //0:oam_bsAgBlkRes

//23. noOfMultiframesBetweenPaging
#define OAMrm_BS_PA_MFRMS	\
					((unsigned char)(grr_OamData.btsBasicPackage.noOfMultiframesBetweenPaging))
    				//1:oam_bsPaMfrms
    
//
// BtsOptionsPackage
//

//1. info
//					<NA>

//2. allowIMSIAttachDetach
#define OAMrm_ATT_STATE		\
					((T_CNI_RIL3_ATT)(grr_OamData.btsOptionsPackage.allowIMSIAttachDetach))
					//1:ie CNI_RIL3_ATT_IMSI_ATACH_DETACH_ALLOWED,oam_att
//3. callReestablishmentAllowed
#define OAMrm_RA_REST_ALLOWED_STATE \
					((T_CNI_RIL3_REESTABLISHMENT_ALLOWED) \
					(grr_OamData.btsOptionsPackage.callReestablishmentAllowed))
					//1:not allowed
//4. cellBarred
#define OAMrm_RA_CELL_BARRED_STATE \
					((T_CNI_RIL3_CELL_BARRED_ACCESS)(grr_OamData.btsOptionsPackage.cellBarred))
					//0:not barred
#define OAMrm_RA_CELL_BARRED_STATEa \
					grr_OamData.btsOptionsPackage.cellBarred
					//0:not barred
//5. dtxDownlink
#define OAMrm_MS_DNLINK_DTX_STATE \
					((int)(grr_OamData.btsOptionsPackage.dtxDownlink))
					//2:downlink DTX control ON/OFF

//5a. dtxUpLink
#define OAMrm_MS_UPLINK_DTX_STATE \
					((T_CNI_RIL3_DTX_SACCH)(grr_OamData.btsOptionsPackage.dtxUplink))
					//2:uplink DTX not allowed for MS

#define OAMrm_MS_UPLINK_DTX_STATEa \
					((T_CNI_RIL3_DTX_BCCH)(grr_OamData.btsOptionsPackage.dtxUplink))
					//2:uplink DTX not allowed for MS

//6. emergencyCallRestricted
#define OAMrm_RA_EC_ALLOWED_STATE \
					((T_CNI_RIL3_EMERGENCY_CALL_ALLOWED) \
					(grr_OamData.btsOptionsPackage.emergencyCallRestricted))
					//1:only allowed to special class mobiles

//7. notAllowedAccessClassTable
#define OAMrm_RA_AC_ALLOWED_STATE(X) \
					((unsigned char)(((NotAllowedAccessClassEntry*) \
					(grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[X].notAllowedAccessClass))
					//1:barred

//8. timerPeriodicUpdateMS
#define OAMrm_T3212	\
					((unsigned char)(grr_OamData.btsOptionsPackage.timerPeriodicUpdateMS))
					//0:no periodic LUP needed, oam_T3212
//9. maxQueueLength
//					<NA>

//10. msPriorityUsedInQueueing
//					<NA>

//11. timeLimitCall
//					<NA>

//12. timeLimitHandover
//					<NA>

//
// AdjacentCellPackage
//

//1.   adjCell_handoverTable
//1.1  adjCell_handoverTable:adjCell_handoverIndex
//					<NA>
//1.2  adjCell_handoverTable:adjCell_handoverCellID

#define OAMrm_NCELL_NETWORKID(X) \
					((short)((((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))>>16)
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
#define OAMrm_NCELL_CELLID(X) \
					((short)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
#define OAMrm_HO_ADJCELL_ID(X) \
					((unsigned int)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_handoverCellID))
#define OAMrm_HO_ADJCELL_ID_t(X) \
					((unsigned int)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31

//1.3  adjCell_handoverTable:adjCell_mcc
#define OAMrm_NCELL_MCC(X) \
            ((unsigned int)((((AdjCell_HandoverEntry*) \
            (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_mcc)))

//1.4  adjCell_handoverTable:adjCell_mnc
#define OAMrm_NCELL_MNC(X) \
            ((unsigned int)((((AdjCell_HandoverEntry*) \
            (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_mnc)))

//1.5  adjCell_handoverTable:adjCell_lac
#define OAMrm_NCELL_LAC(X) \
            ((unsigned short)(((AdjCell_HandoverEntry*) \
            (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_lac))

//1.6  adjCell_handoverTable:adjCell_ci
#define OAMrm_NCELL_CI(X) \
            ((unsigned short)((((AdjCell_HandoverEntry*) \
            (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_ci)))

//1.7  adjCell_handoverTable:adjCell_bCCHFrequency
#define OAMrm_HO_ADJCELL_BCCH_ARFCN(X) \
					((short)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_bCCHFrequency))
					//561 for X=0,761 for X=1, 0 for other X values, X:0-31

#define OAMrm_HO_ADJCELL_BCCH_ARFCN_t(X) \
					((short)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_bCCHFrequency))
					//561 for X=0,761 for X=1, 0 for other X values, X:0-31

#define OAMrm_NARFCN(X) \
					((signed short)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_bCCHFrequency))
					//561: neighboring cell frequency for monitoring for handover purpose

//1.8  adjCell_handoverTable:adjCell_ncc
//					<TBD>
//1.9  adjCell_handoverTable:adjCell_cid
//					<TBD>
//1.10 adjCell_handoverTable:adjCell_synchronized
//					<TBD>
//1.11 adjCell_handoverTable:adjCell_hoPriorityLevel
//					<TBD>
//1.12 adjCell_handoverTable:adjCell_hoMargin
//					<TBD>
//1.13 adjCell_handoverTable:adjCell_msTxPwrMaxCell
//					<TBD>
//1.14 adjCell_handoverTable:adjCell_rxLevMinCell
//					<TBD>
//1.15 adjCell_handoverTable:adjCell_isExternal
#define OAMrm_NCELL_ISEXTERNAL(X) \
            ((unsigned int)((((AdjCell_HandoverEntry*) \
            (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_isExternal)))

//2.   adjCell_reselectionTable
//2.1  adjCell_reselectionTable:adjCell_reselectionIndex
//					<TBD>

//2.2  adjCell_reselectionTable:adjCell_reselectionCellID
#define OAMrm_RESEL_ADJCELL_ID(X) \
					((unsigned int)(((AdjCell_ReselectionEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[X].adjCell_reselectionCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values
//2.3  adjCell_reselectionTable:adjCell_reselectionBCCHFrequency
#define OAMrm_RESEL_ADJCELL_BCCH_ARFCN(X) \
					((short)(((AdjCell_ReselectionEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[X].adjCell_reselectionBCCHFrequency))
					//561 for X=0,761 for X=1, 0 for other X values
    
//
// BtsTimerPackage:T31xx
//
 
//1. info
//					<TBD>

//2. t3101
#define OAMrm_T3101 \
					((int)(grr_OamData.t31xx.t3101))
    				//0x2FF:oam_T3101
//3. t3103
//					<TBD>

//4. t3105
#define OAMrm_T3105 \
					((int)(grr_OamData.t31xx.t3105))
					//2:16.6 by 2 ms
//5. t3107
//					<TBD>
//6. t3109
#define OAMrm_T3109 \
					((int)(grr_OamData.t31xx.t3109))
    				//0x1FF:oam_T3109
//7. t3111
#define OAMrm_T3111 \
					((int)(grr_OamData.t31xx.t3111))
    				//0x2FF:oam_T3111
//8. t3113
//					<TBD>
    
//
// TransceiverPackage
//

//1. info
//					<TBD>
//2. basebandTransceiverID
//					<TBD>

//3. relatedRadioCarrier
#define OAMrm_TRX_RC(X) \
					((unsigned char)(grr_OamData.transceiverPackage[X].relatedRadioCarrier))
    
//4. basebandAdministrativeState
#define OAMrm_TRX_AMSTATE(X) \
					grr_OamData.transceiverPackage[X].basebandAdministrativeState
//5. basebandAlarmStatus
//					<TBD>

//6. basebandOperationalState
#define OAMrm_TRX_OPSTATE(X) \
			        grr_OamData.transceiverPackage[X].basebandOperationalState

//7.  channelTable
//7.1 channelTable:channelIndex
//					<TBD>
//7.2 channelTable:channelID
//					<TBD>

//7.3 channelTable:channelCombination
#define OAMrm_TRX_SLOT_COMB(X,Y) \
					((unsigned char)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y].channelCombination))
    				//5 for (X,Y)=(0,0), 1 for other (X,Y) pairs, X:0-1,Y:0-7
//7.4 channelTable:channelFrequencyUsage
//					<TBD>
//7.5 channelTable:channelTsc
//					<TBD>
//7.6 channelTable:channelAdministrativeState
#define OAMrm_CHN_AMSTATE(X,Y) \
					((AdministrativeState)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelAdministrativeState))
#define OAMrm_CHN_AMSTATEa(X,Y) \
					((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelAdministrativeState
					//Check needed
//7.7 channelTable:channelOperationalState
#define OAMrm_CHN_OPSTATE(X,Y) \
					((EnableDisable)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelOperationalState))
#define OAMrm_CHN_OPSTATEa(X,Y) \
					((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelOperationalState
					//CheckNeeded
             
//
// radioCarrierPackage
//

//1. info
//					<TBD>
//2. carrierFrequencyList
//2.1 carrierFrequencyList:carrierFrequencyIndex
//					<TBD>
//2.2 carrierFrequencyList:carrierFrequency
#define OAMrm_ARFCN(X) \
					((unsigned short)(((CarrierFrequencyEntry*)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].carrierFrequencyList))[0].carrierFrequency))
					//661 for X=0; 561 for X=1, X:0-1
    
//3. powerClass
//					<TBD>
//4. radioCarrierID
//					<TBD>
//5. txPwrMaxReduction
//					<TBD>
//6. carrier_administrativeState
#define OAMrm_RC_AMSTATE(X) \
					grr_OamData.radioCarrierPackage[X].carrier_administrativeState
//7. carrier_alarmStatus
//					<TBD>

//8. carrier_operationalState
#define OAMrm_RC_OPSTATE(X) \
					grr_OamData.radioCarrierPackage[X].carrier_operationalState
    
//
// Bts System Feature Control
//
#define OAMrm_CBCH_OPSTATE           	rm_CbchOpState
				        //grr_OamData.grrConfigData.rm_nim[0][1]
#define OAMrm_CB_FEATURE_SETTING        grr_OamData.grrConfigData.rm_nim[0][1]
#define OAMrm_BCCH_CARRIER_phTRX    	grr_OamData.grrConfigData.rm_nim[0][0]
#define OAMrm_HOPPING_CTRL          	((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][2]))
#define OAMrm_HOPPING_CTRLa          	grr_OamData.grrConfigData.rm_nim[0][2]
#define OAMrm_I_Q_SWAP                  grr_OamData.grrIQSwap
  
//C2 parameter components
#define OAMrm_CELL_RESELECT_OFFSET	((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][3]))
#define OAMrm_CELL_RESELECT_OFFSETa	grr_OamData.grrConfigData.rm_nim[0][3]
#define OAMrm_TEMP_OFFEST		((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][4]))
#define OAMrm_TEMP_OFFESTa		grr_OamData.grrConfigData.rm_nim[0][4]
#define OAMrm_PENALTY_TIME		((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][5]))
#define OAMrm_PENALTY_TIMEa		grr_OamData.grrConfigData.rm_nim[0][5]
#define OAMrm_CELL_BAR_QUALIFY		grr_OamData.grrConfigData.rm_nim[0][6]
#define OAMrm_OVERLOAD_CELLBAR 		grr_OamData.grrConfigData.rm_nim[1][1]

extern u16				oam_phTrxSelectBit[2];         //b14
extern u16				oam_phTrxSynthSelectBit[2];    //b15
extern u16				oam_phTrxHoppingSelectBit[2];  //b13
extern u16				oam_phTrxPwrSelectBit[2];      //b12
    
#define OAMrm_phTRX_SELECT(X)       	oam_phTrxSelectBit[X]
#define OAMrm_phTRX_SYNTH_SELECT(X) 	oam_phTrxSynthSelectBit[X]
#define OAMrm_phTRX_HOPPING(X)      	oam_phTrxHoppingSelectBit[X]
#define OAMrm_phTRX_PWR_SELECT(X)   	oam_phTrxPwrSelectBit[X]
    
#define OAMrm_CCCH_CONF	  		((T_CNI_RIL3_CCCH_CONF)1)
#define OAMrm_BS_CC_CHANS 		1
#define OAMrm_BSCCCHSDCCH_COMB 		1
#define OAMrm_CCCH_SDCCH_COMB 		1
#define OAMrm_T3L01 			555
#define OAMrm_T3L02 			4
#define OAMrm_SYSINFO_ONOFF_MAP 	0

    
#define OAMrm_AIRINTERFACE \
					((unsigned char)(grr_OamData.btsBasicPackage.gsmdcsIndicator))
   
extern u8 oam_CellReselectOffset;
extern u8 oam_TemporaryOffset;
extern u8 oam_PenaltyTime;
  
#define OAMrm_BTS_TSC 	      		((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
#define OAMrm_SLOTS_PER_TRX  		8
#define OAMrm_MAX_TRXS 			2
#define OAMrm_MAX_TCHFS  		(OAMrm_MAX_TRXS*8)
#define OAMrm_MAX_SDCCH4S  		4
#define OAMrm_TRX_ONOFF_MAP 		3
#define OAMrm_NETWORKIFCONFIG  		0
#define OAMrm_BCCH_TRX 			0
#define OAMrm_PREALPHA_TRX 		0
#define OAMrm_NIM(X,Y) 			0
#define OAMrm_SYNCH_CHANINFO 		0
    
//
// RM Configuration Package
//
#define OAMrm_TRX_SYNTH_SETTINGMSB(X,Y) \
			((unsigned char)(oam_TrxTuneSynthSetting[X][Y]>>8))
#define OAMrm_TRX_SYNTH_SETTINGLSB(X,Y) \
            ((unsigned char)(oam_TrxTuneSynthSetting[X][Y]))
#define OAMrm_TRX_ULFPGADELAY_MSB(X) \
      		((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[X][0]>>8))
            //rm_trxFpgaDelay[2][2] all elements<-0x00
#define OAMrm_TRX_ULFPGADELAY_LSB(X) \
            ((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[X][0]))
            //defined above
#define OAMrm_TRX_DLFPGADELAY_MSB(X) \
      	  ((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[X][1]>>8)) 
            //defined above
#define OAMrm_TRX_DLFPGADELAY_LSB(X) \
            ((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[X][1]))
            //defined above
#define OAMrm_UPLINK_SGAINCTRL_SETTING(X) \
            ((unsigned short)(grr_OamData.grrConfigData.rm_SGainCtrlSetting[X][0]))
            //rm_SGainCtrlSetting[X][0]<-0x180
#define OAMrm_DOWNLINK_SGAINCTRL_SETTING(X) \
            ((unsigned char)(grr_OamData.grrConfigData.rm_SGainCtrlSetting[X][1]))
            //rm_SGainCtrlSetting[X][1]<-0x80

//
// OAM-related function prototypes
//
void rm_RetrvConfigFromOam					(void); //Retrieve all RM-related data stored in OAM
void rm_OamGetRmConfigData					(void); //Retrieve all RM-specific conf data from OAM
void rm_OamGetGsmData						(void); //Retrieve all GSM standard conf data from RR
STATUS rm_OamGetBtsBasicPackage				(void); //Retrieval of standard GSM: BtsBasicPackage
STATUS rm_OamGetBtsOptionPackage			(void); //Retrieval of standard GSM: BtsOptionPackage
STATUS rm_OamGetBtsFirstTrxPackage			(void); //Retrieval of standard GSM: BtsFirstTrxPackage
STATUS rm_OamGetBtsSecondTrxPackage			(void); //Retrieval of standard GSM: BtsSecondTrxPackage
STATUS rm_OamGetBtsFirstRadioCarrierPackage (void); //Retrieval of standard GSM: BtsFirstRadioCarrierPackage
STATUS rm_OamGetBtsSecondRadioCarrierPackage(void); //Retrieval of standard GSM: BtsSecondRadioCarrierPackage
STATUS rm_OamGetT31xxTimerStructure			(void); //Retrieval of standard GSM: T31xxTimer
STATUS rm_adjacentCellPackage				(void); //Retrieval of standard GSM: adjacentCellPackage
STATUS rm_handoverControlPackage			(void); //Retrieval of standard GSM: handoverControlPackage
STATUS rm_powerControlPackage				(void); //Retrieval of standard GSM: powerControlPackage
    	
// OAM test functions
void rm_TestPrintOamRmConfigData			(void); //Print all RM-related data stored in OAM MIB
void rm_TestPrintOamRmStdGsmData			(void); //Print all RM-related standard GSM mib data
void intg_SetOverloadCellbarOption			(int);
void rm_printOamGsmData						(void); //Print all GSM standard conf data from RR
void rm_print_btsBasicPackage				(BtsBasicPackage*);			  //Print standard GSM: BtsBasicPackage
void rm_print_btsOptionsPackage				(BtsOptionsPackage*);		  //Print standard GSM: BtsOptionsPackagevoid 
void rm_print_firstTransceiverPackage		(FirstTransceiverPackage*);   //Print standard GSM: FirstTransceiverPackage
void rm_print_secondTransceiverPackage		(SecondTransceiverPackage*);  //Print standard GSM: SecondTransceiverPackage
void rm_print_firstRadioCarrierPackage		(FirstRadioCarrierPackage*);  //Print standard GSM: FirstRadioCarrierPackage
void rm_print_secondRadioCarrierPackage		(SecondRadioCarrierPackage*); //Print standard GSM: SecondRadioCarrierPackage
void rm_print_t31xx							(T31xx*);					  //Print standard GSM: T31xx
void rm_print_adjacentCellPackage			(AdjacentCellPackage*);		  //Print standard GSM: AdjacentCellPackage
void rm_print_handoverControlPackage		(HandoverControlPackage*);    //Print standard GSM: HandoverControlPackage
void rm_print_powerControlPackage			(PowerControlPackage*);       //Print standard GSM: PowerControlPackage
    
// Oam Message to DSP
u8 rm_OamGetTotalAvailableHoNCells			(void);
void rm_OamSendHoNCellToDsp					(u8 trxNo, u8 nCellIdx,u8 TotalAvailableNCells);
void rm_OamSendHoSCellToDsp					(s8 trxNo);
void rm_OamSendPcToDsp						(s8 trxNo);
void rm_OamSendHoAllNCellToDsp				();
void rm_OamSendPwrRedStepToDsp				(u8 trxNo);
void rm_SendBtsPackageToTrx					(u8 trx);    

// Register OAM trap variables
void rm_OamRegTrapVars						(void); 
void rm_OamGetTrapVars						(void);
void rm_OamProcTrapBtsBasicPackage			(void);
void rm_OamProcTrapBtsOptionsPackage		(void);
void rm_OamProcTrapTransceiverPackage		(u8);
void rm_OamProcTrapRadioCarrierPackage		(u8, MibTag);
void rm_OamProcTrapT31xx					(void);
void rm_OamProcAdjacentCellPackage			(void);
void rm_OamProcHandoverControlPackage		(void);
void rm_OamProcPowerControlPackage			(void);
void rm_OamProcResourceConfigData			(void);
    
// Alarm
void rm_InitTrxWDog							(u8 trx);
void rm_SetRmOpState						(Alarm_errorCode_t code);
void rm_InitRcPkg							(u8 rc);
void rm_InitTrxPkg							(u8 trx);
void rm_SetOneTrxChnOpState					(u8 trx,u8 slot,EnableDisable state);
void rm_SetAllTrxChnOpState					(u8 trx,EnableDisable state);
void rm_SetTrxOpState						(u8 trx, EnableDisable state);
void rm_SetRcOpState						(u8 rc, EnableDisable state);
void rm_ProcTrxMgmtMsg						(u8 trx);
u8   rm_EnDisableRmTchf						(u8 trx, u8 slot, u8 ackVal);
u8   rm_ProcSlotActRsp						(u8 trx,u8 slot,u8 ackVal);
void rm_ActTimeSlot							(u8 trx,u8 slot);
void rm_PassOpParamsToTrx					(u8 trx);
void rm_SetAllRmTrxChnOpState				(u8 trx, u8 state);
void rm_SendPingToTrx						(u8 trx);
void rm_ProcTrxWDogExp						(u8 trx);

//AM
void rm_ProcBtsOpStateChange				(void);
void rm_ProcBtsAdminStateChange				(void);
void rm_ProcBtsAdminStateLocked				(void);
void rm_ProcTrxAdminStateLocked				(u8 trx);
void rm_ProcTrxSlotAdminStateLocked			(u8 trx, u8 slot);
void rm_LockedAllTrxSlotChans				(u8 trx, u8 slot);
void rm_LockedOneChan						(rm_PhyChanDesc_t *pChan);
void rm_ProcBtsAdminStateUnlocked			(void);
void rm_ProcTrxAdminStateUnlocked			(u8 trx);
void rm_ProcTrxSlotAdminStateUnlocked		(u8 trx, u8 slot);
void rm_UnlockedAllTrxSlotChans				(u8 trx, u8 slot);
void rm_ProcBtsAdminStateShuttingDown		(void);
void rm_ProcTrxAdminStateShuttingDown		(u8 trx, u8 *inUse);
void rm_ProcTrxSlotAdminStateShuttingDown	(u8 trx, u8 slot, u8 *inUse);
void rm_ShuttingDownAllTrxSlotChans			(u8 trx, u8 slot, u8 *inUse);
void rm_SetBtsAdminState					(AdministrativeState state);
void rm_SetTrxAdminState					(u8 trx, AdministrativeState state);
void rm_SetTrxSlotAdminState				(u8 trx, u8 slot, AdministrativeState state);
void rm_SetTrxSlotOpState					(u8 trx, u8 slot, EnableDisable state);
void rm_LockParentTrxSlotIfNeeded			(int trx, int slot);
void rm_LockParentTrxIfNeeded				(int trx);
void rm_LockParentBtsIfNeeded				(void);
void rm_SetBtsAdminStateIfNeeded			(AdministrativeState state);
void rm_SetTrxAdminStateIfNeeded			(int trx, AdministrativeState state);
void rm_SetTrxSlotAdminStateIfNeeded		(int trx, int slot, AdministrativeState state);
void rm_TurnOnRadioIfNeeded					(void);
void intg_TuneC1C2							(u8 reselOffset, u8 tempOffset, u8 penaltyTime);
void rm_SendArfcnToTrx						(u8 trx, int swap);
void rm_TestPrintAmStateData				(void);
void intg_SendSlotActivateToTrx				(u8 trx, u8 slot, u8 comb);
void rm_MakeSureDspIsTicking				(void);

#endif //__RM_OAMIF_H__
