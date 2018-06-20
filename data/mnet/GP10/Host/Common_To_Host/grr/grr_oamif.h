/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_OAMIF_H__
#define __GRR_OAMIF_H__

#include "grr_head.h"

//G2TEST
#define OAMgrr_GPRS_FEATURE_SETTING     grr_GprsCtrl

#define OAMgrr_GPRS_RA_COLOR			0
#define OAMgrr_SI13_POS					0
#define OAMgrr_GPRS_NETWORK_MODE        1
#define OAMgrr_CBCH_TRX_SLOT			0
#define OAMgrr_CBCH_TRX				    0
#define OAMgrr_BCCH_CHANGE_MARK			0
#define OAMgrr_SI_CHANGE_FIELD			0
#define OAMgrr_GPRS_RAC					0x01
#define OAMgrr_GPRS_NMO					0x01
#define OAMgrr_GPRS_T3168				4
#define OAMgrr_GPRS_T3192				7
#define OAMgrr_GPRS_DRX_TIMER_MAX		7
#define OAMgrr_GPRS_ACCESS_BURST_TYPE	0
#define OAMgrr_GPRS_CONTROL_ACK_TYPE	0
#define OAMgrr_GPRS_BS_CV_MAX			7
#define OAMgrr_GPRS_PAN_DEC				1
#define OAMgrr_GPRS_PAN_INC				1
#define OAMgrr_GPRS_PAN_MAX				4

#define OAMgrr_GPRS_ALPHA				10
#define OAMgrr_GPRS_T_AVG_W				15
#define OAMgrr_GPRS_T_AVG_T				5
#define OAMgrr_GPRS_PC_MEAS_CHAN		1
#define OAMgrr_GPRS_N_AVG_I				4

//
// Slot combination: Mapped to definition given in OAM vipermib.h
//
#define OAMgrr_SLOT_COMB_1	     	    tCHFull		        //0x01 Type I
#define OAMgrr_SLOT_COMB_5	    		bCCHCombined	    //0x05 Type V
#define OAMgrr_SLOT_COMB_11				pBCCH				//0x0B type xi
#define OAMgrr_SLOT_COMB_12             pCCCH				//0x0C type xii
#define OAMgrr_SLOT_COMB_13				pDCHData			//0x0D type xiii
#define OAMgrr_SLOT_BEACON	      	    OAMgrr_SLOT_COMB_5  //0x05 Beacon
#define OAMgrr_SLOT_UNUSED			    0xFF   		        //0xFF Unused
#define OAMgrr_TRX_SLOT_COMB_1	   	    tCHFull		        //0x01 Type I
#define OAMgrr_TRX_SLOT_COMB_5	   		bCCHCombined	    //0x05 Type V
#define OAMgrr_TRX_SLOT_COMB_13	   		pDCHData		    //0x0D Type XIII
#define OAMgrr_TRX_SLOT_BEACON	   	    OAMgrr_SLOT_COMB_5  //0x05 Beacon
#define OAMgrr_TRX_SLOT_UNUSED		    0xFF   		        //0xFF Unused

    
#define OAMgrr_HO_ADJCELL_NO_MAX  		16
#define OAMgrr_RESEL_ADJCELL_NO_MAX  	16

#define OAMgrr_NCC_PERMITTED_MAX 	  	8

#define OAMgrr_HO_ADJCELL_ID_NULL    	0
#define OAMgrr_RESEL_ADJCELL_ID_NULL 	0
  
//
// BtsBasicPackage
//
  
//1. info
//<NA>

//2. bts_ncc
#define OAMgrr_NCC	\
				((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc))
    				//5:oam_Ncc, bts_ncc may be combined with 
//3. bts_cid
#define OAMgrr_BCC	\
					((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
    				//5:oam_Bcc

//3a. comb of 2&3:(bts_ncc<<3)|(bts_cid)
#define OAMgrr_POCKETBTS_BSIC  \
					((unsigned char)(grr_OamData.btsBasicPackage.bts_ncc<<3)| \
					(unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
					//0x2d: oam_PocketBTSBsic
//4. btsID
#define OAMgrr_CELLID \
					((unsigned int)(grr_OamData.btsBasicPackage.btsID))
    				      //0x0001: hi-byte network ID, lo-byte cell ID

//5. cellAllocationTable[0]
#define OAMgrr_BCCH_ARFCN \
    				      (((CellAllocationEntry*) \
					(grr_OamData.btsBasicPackage.cellAllocationTable))[0].cellAllocation)
					//661:1st element of cellAllocation table is BCCH ARFCN, oam_BcchArfcn
#define OAMgrr_CA_ARFCN(X) \
 	     (((CellAllocationEntry*)(grr_OamData.btsBasicPackage.cellAllocationTable))[X].cellAllocation)

//6. gsmdcsIndicator
#define OAMgrr_GSMDCS_INDICATOR \
					grr_OamData.btsBasicPackage.gsmdcsIndicator

//7. bts_mcc
#define OAMgrr_MCC(X) \
					((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mcc))[X]
					//001: bts_mcc is an integer which will be converted to char string, oam_Mcc
//8. bts_mnc
#define OAMgrr_MNC(X) \
					((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[X]
					//01: bts_mnc is an integer which will be converted to char string, oam_Mnc
//9. bts_lac
#define OAMgrr_LAC	 \
					((short)(grr_OamData.btsBasicPackage.bts_lac))
					//1:oam_Lac[X]
//10. cellReselectHysteresis
#define OAMgrr_CELL_RESEL_HYSTERESIS \
					((unsigned char)(grr_OamData.btsBasicPackage.cellReselectHysteresis))
					//2:4 dBm
//11. ny1
#define OAMgrr_NY1	\
					((unsigned char)(grr_OamData.btsBasicPackage.ny1))
					//3
//12. plmnPermittedTable
#define OAMgrr_NCC_PERMITTED(X) \
					(((PlmnPermittedEntry*)(grr_OamData.btsBasicPackage.plmnPermittedTable))[X]. \
					plmnPermitted)
    				//1 for X=5, 0 for other X values, X:0-7
//13. radioLinkTimeout
#define OAMgrr_RADIO_LINK_TIMEOUT \
					((unsigned char)(grr_OamData.btsBasicPackage.radioLinkTimeout))
					//3: ie 16
//14. relatedTranscoder
//					<NA>

//15. rxLevAccessMin
#define OAMgrr_MS_RELEV_ACCESS_MIN ((unsigned char)(grr_OamData.btsBasicPackage.rxLevAccessMin))
					//6:104dBm

//16. bts_administrativeState
#define OAMgrr_BTS_AMSTATEa   grr_OamData.btsBasicPackage.bts_administrativeState
#define OAMgrr_BTS_AMSTATE 	  grr_OamData.btsBasicPackage.bts_administrativeState

//17. bts_alarmStatus
//					<NA>

//18. bts_operationalState
#define OAMgrr_BTS_OPSTATEa   grr_OamData.btsBasicPackage.bts_operationalState
#define OAMgrr_BTS_OPSTATE    grr_OamData.btsBasicPackage.bts_operationalState

//19. maxNumberRetransmissions
#define OAMgrr_RA_RETRY_MAX \
					((unsigned char)(grr_OamData.btsBasicPackage.maxNumberRetransmissions))
					//3:retry 7 times
//20. mSTxPwrMaxCCH
#define OAMgrr_MS_TX_PWR_MAX_CCH \
					((unsigned char)(grr_OamData.btsBasicPackage.mSTxPwrMaxCCH))
					//0:30dBm, previous value is 11 i.e. 8dBm, oam_MsTxPwrMax

//20a. PR1101
#define OAMgrr_MS_TX_PWR_MAX_CCHa \
					grr_OamData.btsBasicPackage.mSTxPwrMaxCCH
					//0:30dBm, previous value is 11 i.e. 8dBm, oam_MsTxPwrMax

//21. numberOfSlotsSpreadTrans
#define OAMgrr_RA_TX_INTEGER \
					((unsigned char)(grr_OamData.btsBasicPackage.numberOfSlotsSpreadTrans))
					//7:spread 10 time slots
//22. noOfBlocksForAccessGrant
#define OAMgrr_BS_AG_BLK_RES \
					((unsigned char)(grr_OamData.btsBasicPackage.noOfBlocksForAccessGrant))
            //0:oam_bsAgBlkRes

//23. noOfMultiframesBetweenPaging
#define OAMgrr_BS_PA_MFRMS	\
					((unsigned char)(grr_OamData.btsBasicPackage.noOfMultiframesBetweenPaging))
    				//1:oam_bsPaMfrms
    
//
// BtsOptionsPackage
//

//1. info
//					<NA>

//2. allowIMSIAttachDetach
#define OAMgrr_ATT_STATE		\
					((T_CNI_RIL3_ATT)(grr_OamData.btsOptionsPackage.allowIMSIAttachDetach))
					//1:ie CNI_RIL3_ATT_IMSI_ATACH_DETACH_ALLOWED,oam_att
//3. callReestablishmentAllowed
#define OAMgrr_RA_REST_ALLOWED_STATE \
					((T_CNI_RIL3_REESTABLISHMENT_ALLOWED) \
					(grr_OamData.btsOptionsPackage.callReestablishmentAllowed))
					//1:not allowed
//4. cellBarred
#define OAMgrr_RA_CELL_BARRED_STATE \
					((T_CNI_RIL3_CELL_BARRED_ACCESS)(grr_OamData.btsOptionsPackage.cellBarred))
					//0:not barred
#define OAMgrr_RA_CELL_BARRED_STATEa \
					grr_OamData.btsOptionsPackage.cellBarred
					//0:not barred
//5. dtxDownlink
#define OAMgrr_MS_DNLINK_DTX_STATE \
					((int)(grr_OamData.btsOptionsPackage.dtxDownlink))
					//2:downlink DTX control ON/OFF

//5a. dtxUpLink
#define OAMgrr_MS_UPLINK_DTX_STATE \
					((T_CNI_RIL3_DTX_SACCH)(grr_OamData.btsOptionsPackage.dtxUplink))
					//2:uplink DTX not allowed for MS

#define OAMgrr_MS_UPLINK_DTX_STATEa \
					((T_CNI_RIL3_DTX_BCCH)(grr_OamData.btsOptionsPackage.dtxUplink))
					//2:uplink DTX not allowed for MS

//6. emergencyCallRestricted
#define OAMgrr_RA_EC_ALLOWED_STATE \
					((T_CNI_RIL3_EMERGENCY_CALL_ALLOWED) \
					(grr_OamData.btsOptionsPackage.emergencyCallRestricted))
					//1:only allowed to special class mobiles

//7. notAllowedAccessClassTable
#define OAMgrr_RA_AC_ALLOWED_STATE(X) \
					((unsigned char)(((NotAllowedAccessClassEntry*) \
					(grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[X].notAllowedAccessClass))
					//1:barred

//8. timerPeriodicUpdateMS
#define OAMgrr_T3212	\
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

#define OAMgrr_NCELL_NETWORKID(X) \
					((short)((((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))>>16)
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
#define OAMgrr_NCELL_CELLID(X) \
					((short)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
#define OAMgrr_NCELL_NETWORKID_t(X) \
					((short)((((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))>>16)
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
#define OAMgrr_NCELL_CELLID_t(X) \
					((short)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31
#define OAMgrr_HO_ADJCELL_ID(X) \
					((unsigned int)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_handoverCellID))
#define OAMgrr_HO_ADJCELL_ID_t(X) \
					((unsigned int)(((AdjCell_HandoverEntry*) \
					(grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_handoverCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values, X:0-31

//1.3  adjCell_handoverTable:adjCell_mcc
#define OAMgrr_HO_ADJCELL_MCC(X,Y)  \
                                      ((unsigned char*)&(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_mcc))[Y]
                    
#define OAMgrr_HO_ADJCELL_MCC_t(X,Y)  \
                                      ((unsigned char*)&(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_mcc))[Y]

//1.4  adjCell_handoverTable:adjCell_mnc
#define OAMgrr_HO_ADJCELL_MNC(X,Y)  \
                                      ((unsigned char*)&(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_mnc))[Y]
                    
#define OAMgrr_HO_ADJCELL_MNC_t(X,Y)  \
                                      ((unsigned char*)&(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_mnc))[Y]

//1.5  adjCell_handoverTable:adjCell_lac
#define OAMgrr_HO_ADJCELL_LAC(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_lac))

#define OAMgrr_HO_ADJCELL_LAC_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_lac))

//1.6  adjCell_handoverTable:adjCell_ci
#define OAMgrr_HO_ADJCELL_CI(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_ci))

#define OAMgrr_HO_ADJCELL_CI_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_ci))

//1.7  adjCell_handoverTable:adjCell_bCCHFrequency
#define OAMgrr_HO_ADJCELL_BCCH_ARFCN(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_bCCHFrequency))

#define OAMgrr_HO_ADJCELL_BCCH_ARFCN_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_bCCHFrequency))

#define OAMgrr_NARFCN(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_bCCHFrequency))

#define OAMgrr_NARFCN_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_bCCHFrequency))

//1.8  adjCell_handoverTable:adjCell_ncc
#define OAMgrr_HO_ADJCELL_NCC(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_ncc))

#define OAMgrr_HO_ADJCELL_NCC_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_ncc))

//1.9  adjCell_handoverTable:adjCell_cid
#define OAMgrr_HO_ADJCELL_BCC(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_cid))

#define OAMgrr_HO_ADJCELL_BCC_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_cid))

//1.10 adjCell_handoverTable:adjCell_synchronized
#define OAMgrr_HO_ADJCELL_HO_SYNCH(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_synchronized))

#define OAMgrr_HO_ADJCELL_HO_SYNCH_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_synchronized))

//1.11 adjCell_handoverTable:adjCell_hoPriorityLevel
#define OAMgrr_HO_ADJCELL_HO_PRILEVEL(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_hoPriorityLevel))

#define OAMgrr_HO_ADJCELL_HO_PRILEVEL_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_hoPriorityLevel))

//1.12 adjCell_handoverTable:adjCell_hoMargin
#define OAMgrr_HO_ADJCELL_HO_MARGIN(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_hoMargin))

#define OAMgrr_HO_ADJCELL_HO_MARGIN_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_hoMargin))

//1.13 adjCell_handoverTable:adjCell_msTxPwrMaxCell
#define OAMgrr_HO_ADJCELL_MSTXPWR_MAX_CELL(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_msTxPwrMaxCell))

#define OAMgrr_HO_ADJCELL_MSTXPWR_MAX_CELL_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_msTxPwrMaxCell))

//1.14 adjCell_handoverTable:adjCell_rxLevMinCell
#define OAMgrr_HO_ADJCELL_RXLEV_MIN_CELL(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[X].adjCell_rxLevMinCell))

#define OAMgrr_HO_ADJCELL_RXLEV_MIN_CELL_t(X)  \
                                      ((unsigned short)(((AdjCell_HandoverEntry*) \
                                      (grr_OamData.adjacentCellPackage_t.adjCell_handoverTable))[X].adjCell_rxLevMinCell))

//2.   adjCell_reselectionTable
//2.1  adjCell_reselectionTable:adjCell_reselectionIndex
//					<TBD>

//2.2  adjCell_reselectionTable:adjCell_reselectionCellID
#define OAMgrr_RESEL_ADJCELL_ID(X) \
					((unsigned int)(((AdjCell_ReselectionEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[X].adjCell_reselectionCellID))
					//2 for X=0, 3 for X=1, 0xFFFFFFFF for other X values
//2.3  adjCell_reselectionTable:adjCell_reselectionBCCHFrequency
#define OAMgrr_RESEL_ADJCELL_BCCH_ARFCN(X) \
					((short)(((AdjCell_ReselectionEntry*) \
					(grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[X].adjCell_reselectionBCCHFrequency))
					//561 for X=0,761 for X=1, 0 for other X values
    
//
// BtsTimerPackage:T31xx
//
 
//1. info
//					<TBD>

//2. t3101
#define OAMgrr_T3101 \
					((int)(grr_OamData.t31xx.t3101))
    				//0x2FF:oam_T3101
//3. t3103
#define OAMgrr_T3103 \
					((int)(grr_OamData.t31xx.t3103))
    				
//4. t3105
#define OAMgrr_T3105 \
					((int)(grr_OamData.t31xx.t3105))
					//2:16.6 by 2 ms
//5. t3107
#define OAMgrr_T3107 \
					((int)(grr_OamData.t31xx.t3107))
    				
//6. t3109
#define OAMgrr_T3109 \
					((int)(grr_OamData.t31xx.t3109))
    				//0x1FF:oam_T3109
//7. t3111
#define OAMgrr_T3111 \
					((int)(grr_OamData.t31xx.t3111))
    				//0x2FF:oam_T3111
//8. t3113
#define OAMgrr_T3113 \
					((int)(grr_OamData.t31xx.t3113))
  
//
// TransceiverPackage
//

//1. info
//					<TBD>
//2. basebandTransceiverID
//					<TBD>

//3. relatedRadioCarrier
#define OAMgrr_TRX_RC(X) \
					((unsigned char)(grr_OamData.transceiverPackage[X].relatedRadioCarrier))
    
//4. basebandAdministrativeState
#define OAMgrr_TRX_AMSTATEa(X) grr_OamData.transceiverPackage[X].basebandAdministrativeState
#define OAMgrr_TRX_AMSTATE(X) grr_OamData.transceiverPackage[X].basebandAdministrativeState

//5. basebandAlarmStatus
//					<TBD>

//6. basebandOperationalState
#define OAMgrr_TRX_OPSTATEa(X) grr_OamData.transceiverPackage[X].basebandOperationalState
#define OAMgrr_TRX_OPSTATE(X)  grr_OamData.transceiverPackage[X].basebandOperationalState

//7.  channelTable
//7.1 channelTable:channelIndex
//					<TBD>
//7.2 channelTable:channelID
//					<TBD>

//7.3 channelTable:channelCombination
#define OAMgrr_TRX_SLOT_CHANCOMB(X,Y) \
					((unsigned char)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y].channelCombination))
#define OAMgrr_TRX_SLOT_COMB(X,Y) \
					((unsigned char)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y].channelCombination))

#define OAMgrr_TRX_SLOT_COMBa(X,Y) \
					(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y].channelCombination)

//7.4 channelTable:channelFrequencyUsage
#define OAMgrr_TRX_SLOT_ARFCN(X,Y) \
					((unsigned short)(((CarrierFrequencyEntry*)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].carrierFrequencyList))[0].carrierFrequency))
//					((unsigned char)(((ChannelEntry*) \
//					(grr_OamData.transceiverPackage[X].channelTable))[Y].channelFrequencyUsage))


//7.5 channelTable:channelTsc
#define OAMgrr_TRX_SLOT_TSC(X,Y) ((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
//					((unsigned char)(((ChannelEntry*) \
//					(grr_OamData.transceiverPackage[X].channelTable))[Y].channelTsc))

//7.6 channelTable:channelAdministrativeState
#define OAMgrr_TRX_SLOT_AMSTATEa(X,Y) \
					((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelAdministrativeState
					//Check needed
#define OAMgrr_CHN_AMSTATEa(X,Y) \
					((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelAdministrativeState
					//Check needed
#define OAMgrr_TRX_SLOT_AMSTATE(X,Y) \
					((AdministrativeState)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelAdministrativeState))
#define OAMgrr_CHN_AMSTATE(X,Y) \
					((AdministrativeState)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelAdministrativeState))

//7.7 channelTable:channelOperationalState
#define OAMgrr_TRX_SLOT_OPSTATEa(X,Y) \
					((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelOperationalState
#define OAMgrr_CHN_OPSTATEa(X,Y) \
					((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelOperationalState
#define OAMgrr_TRX_SLOT_OPSTATE(X,Y) \
					((EnableDisable)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelOperationalState))
#define OAMgrr_CHN_OPSTATE(X,Y) \
					((EnableDisable)(((ChannelEntry*) \
					(grr_OamData.transceiverPackage[X].channelTable))[Y \
					].channelOperationalState))

            
//
// radioCarrierPackage
//

//1. info
//					<TBD>
//2. carrierFrequencyList
//2.1 carrierFrequencyList:carrierFrequencyIndex
//					<TBD>
//2.2 carrierFrequencyList:carrierFrequency
#define OAMgrr_ARFCN(X) \
					((unsigned short)(((CarrierFrequencyEntry*)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].carrierFrequencyList))[0].carrierFrequency))
					//661 for X=0; 561 for X=1, X:0-1
#define OAMgrr_TRX_RC_ARFCN(X,Y) \
					((unsigned short)(((CarrierFrequencyEntry*)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].carrierFrequencyList))[0].carrierFrequency))

//3. powerClass
#define OAMgrr_TRX_RC_PWR_CLASS(X) \
					((unsigned short)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].powerClass))

//4. radioCarrierID
#define OAMgrr_TRX_RC_NO(X) \
					((unsigned short)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].radioCarrierID))

//5. txPwrMaxReduction
#define OAMgrr_TRX_RC_MAXPWR_CUT(X) \
					((unsigned short)(grr_OamData.radioCarrierPackage[ \
					grr_OamData.transceiverPackage[X].relatedRadioCarrier \
					].txPwrMaxReduction))

//6. carrier_administrativeState
#define OAMgrr_RC_AMSTATE(X) \
					grr_OamData.radioCarrierPackage[X].carrier_administrativeState
//7. carrier_alarmStatus
//					<TBD>

//8. carrier_operationalState
#define OAMgrr_RC_OPSTATE(X) \
					grr_OamData.radioCarrierPackage[X].carrier_operationalState
#define OAMgrr_RC_OPSTATEa(X) \
					grr_OamData.radioCarrierPackage[X].carrier_operationalState
    
//
// Bts System Feature Control
//
//auto-detection of RF board
#define OAMgrr_IQ_SWAP                    grr_OamData.grr_IQSwap
#define OAMgrr_BTS_AMSTATEold			  grr_OamData.grrBtsAmStateOld
#define OAMgrr_BTS_OPSTATEold			  grr_OamData.grrBtsOpStateOld
#define OAMgrr_TRX0_AMSTATEold			  grr_OamData.grrTrx0AmStateOld
#define OAMgrr_TRX0SLOT0_AMSTATEold		  grr_OamData.grrTrx0Slot0AmStateOld
//
// GRR Configuration Package
//
#define OAMgrr_CB_FEATURE_SETTING       grr_OamData.grrConfigData.rm_nim[0][1]
#define OAMgrr_BCCH_CARRIER_phTRX    	grr_OamData.grrConfigData.rm_nim[0][0]
#define OAMgrr_TRX_SWAP_SETTING      	grr_OamData.grrConfigData.rm_nim[0][0]
#define OAMgrr_IMSI_REQUIRED_FOR_EC     grr_OamData.grrConfigData.rm_nim[1][0]
#define OAMgrr_CELL_BAR_QUALIFY			grr_OamData.grrConfigData.rm_nim[0][6]
#define OAMgrr_OVERLOAD_CELLBAR 		grr_OamData.grrConfigData.rm_nim[1][1]
#define OAMgrr_HOPPING_CTRL          	grr_OamData.grrConfigData.rm_nim[0][2]
#define OAMgrr_HOPPING_CTRLa          	grr_OamData.grrConfigData.rm_nim[0][2]
#define OAMgrr_CELL_RESELECT_OFFSET		((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][3]))
#define OAMgrr_CELL_RESELECT_OFFSETa	grr_OamData.grrConfigData.rm_nim[0][3]
#define OAMgrr_TEMP_OFFEST				((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][4]))
#define OAMgrr_TEMP_OFFESTa				grr_OamData.grrConfigData.rm_nim[0][4]
#define OAMgrr_PENALTY_TIME				((unsigned char)(grr_OamData.grrConfigData.rm_nim[0][5]))
#define OAMgrr_PENALTY_TIMEa			grr_OamData.grrConfigData.rm_nim[0][5]

//#define OAMgrr_T3L01                    grr_OamData.rmConfigData.rm_t3L01
//#define OAMgrr_BS_CC_CHANS              ((unsigned char)(grr_OamData.grrConfigData.rm_bsCcChans)))       //0x01
//#define OAMgrr_BSCCCHSDCCH_COMB         ((unsigned char)(grr_OamData.grrConfigData.rm_bsCcchSdcchComb))) //0x01
//#define OAMgrr_AIRINTERFACE             ((unsigned char)(grr_OamData.grrConfigData.rm_airInterface)))    //0x02
//#define OAMgrr_POCKETBTS_TSC            ((unsigned char)(grr_OamData.grrConfigData.rm_viperCellTsc)))    //0x05
//#define OAMgrr_SLOTS_PER_TRX            ((unsigned char)(grr_OamData.grrConfigData.rm_maxSlotPerTrx)))   //0x08
//#define OAMgrr_MAX_TRXS                 ((unsigned char)(grr_OamData.grrConfigData.rm_maxTrxs)))         //0x02
//#define OAMgrr_MAX_TCHFS                ((unsigned char)(grr_OamData.grrConfigData.rm_maxTchfs)))        //0x0f
//#define OAMgrr_MAX_SDCCH4S              ((unsigned char)(grr_OamData.grrConfigData.rm_maxSdcch4s)))      //0x04
//#define OAMgrr_TRX_ONOFF_MAP            ((unsigned char)(grr_OamData.grrConfigData.rm_trxOnOffMap)))     //0x03
//#define OAMgrr_NETWORKIFCONFIG          ((unsigned char)(grr_OamData.grrConfigData.rm_networkIfConfig))) //0x00
//#define OAMgrr_BCCH_TRX                 ((unsigned char)(grr_OamData.grrConfigData.rm_bcchTrx)))         //0x00
//#define OAMgrr_PREALPHA_TRX             ((unsigned char)(grr_OamData.grrConfigData.rm_preAlphaTrx)))     //0x00
//#define OAMgrr_CCCH_SDCCH_COMB          ((unsigned char)(grr_OamData.grrConfigData.rm_ccchBcchComb)))    //0x01
//#define OAMgrr_CCCH_CONF                ((unsigned char)(grr_OamData.grrConfigData.rm_ccchConf)))        //0x01
//#define OAMgrr_NIM(X,Y)                 ((unsigned char)(grr_OamData.grrConfigData.rm_nim[i][j]))) 

#define OAMgrr_CCCH_CONF	       		  ((T_CNI_RIL3_CCCH_CONF)1)
#define OAMgrr_BS_CC_CHANS   			  1
#define OAMgrr_BSCCCHSDCCH_COMB 		  1
#define OAMgrr_CCCH_SDCCH_COMB 			  1
#define OAMgrr_T3L01 					  555
#define OAMgrr_T3L02 					  4
#define OAMgrr_SYSINFO_ONOFF_MAP 		  0
#define OAMgrr_AIRINTERFACE 		      ((unsigned char)(grr_OamData.btsBasicPackage.gsmdcsIndicator))
#define OAMgrr_POCKETBTS_TSC 	      	  ((unsigned char)(grr_OamData.btsBasicPackage.bts_cid))
#define OAMgrr_SLOTS_PER_TRX  			  8
#define OAMgrr_MAX_TRXS 				  2
#define OAMgrr_MAX_TCHFS  				  (8*OAMgrr_MAX_TRXS)
#define OAMgrr_MAX_SDCCH4S  			  4
#define OAMgrr_NETWORKIFCONFIG  		  0
#define OAMgrr_NIM(X,Y) 				  0
#define OAMgrr_SYNCH_CHANINFO 			  0
#define OAMgrr_TRX_ONOFF_MAP              3
#define OAMgrr_NETWORKIFCONFIG			  0
#define OAMgrr_BCCH_TRX					  0
#define OAMgrr_PREALPHA_TRX				  0

#define OAMgrr_phTRX_SELECT(X)       	  grr_phTrxSelectBit[X]
#define OAMgrr_phTRX_SYNTH_SELECT(X) 	  grr_phTrxSynthSelectBit[X]
#define OAMgrr_phTRX_HOPPING(X)      	  grr_phTrxHoppingSelectBit[X]
#define OAMgrr_phTRX_PWR_SELECT(X)   	  grr_phTrxPwrSelectBit[X]
#define OAMgrr_WDOG_TIME				  18000

#endif //__GRR_OAMIF_H__
