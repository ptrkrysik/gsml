/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMREGTRAPVARS_CPP__
#define __RM_OAMREGTRAPVARS_CPP__

#include "rm\rm_head.h"

const Alarm_errorCode_t
      rm_DspNotResponding[OAMrm_MAX_TRXS]
      = { EC_RM_DSP_0_NOT_RESPONDING, EC_RM_DSP_1_NOT_RESPONDING};
const Alarm_errorCode_t
      rm_DspNotTickingAnyMore[OAMrm_MAX_TRXS]
      = {EC_RM_DSP_0_NOT_TICKING, EC_RM_DSP_1_NOT_TICKING};
const Alarm_errorCode_t
      rm_TrxRadioOff[OAMrm_MAX_TRXS]
      = { EC_RM_TRX_0_RADIO_OFF, EC_RM_TRX_1_RADIO_OFF};
const Alarm_errorCode_t
      rm_TrxAdminLockedAlarmCode[OAMrm_MAX_TRXS] 
      = {EC_RM_TRX_1_LOCKED,EC_RM_TRX_2_LOCKED};

const Alarm_errorCode_t
      rm_TrxAdminUnlockedAlarmCode[OAMrm_MAX_TRXS]
      = {EC_RM_TRX_1_UNLOCKED,EC_RM_TRX_2_UNLOCKED};

const Alarm_errorCode_t
      rm_TrxAdminShuttingDownAlarmCode[OAMrm_MAX_TRXS]
      = {EC_RM_TRX_1_SHUTTINGDOWN,EC_RM_TRX_2_SHUTTINGDOWN};

const Alarm_errorCode_t
      rm_TrxSlotAdminLockedAlarmCode[OAMrm_MAX_TRXS][OAMrm_SLOTS_PER_TRX]
      = {
          {EC_RM_TRX_1_SLOT_1_LOCKED, EC_RM_TRX_1_SLOT_2_LOCKED,
	     EC_RM_TRX_1_SLOT_3_LOCKED, EC_RM_TRX_1_SLOT_4_LOCKED,
           EC_RM_TRX_1_SLOT_5_LOCKED, EC_RM_TRX_1_SLOT_6_LOCKED,
	     EC_RM_TRX_1_SLOT_7_LOCKED, EC_RM_TRX_1_SLOT_8_LOCKED},
	
	    {EC_RM_TRX_2_SLOT_1_LOCKED, EC_RM_TRX_2_SLOT_2_LOCKED,
	     EC_RM_TRX_2_SLOT_3_LOCKED, EC_RM_TRX_2_SLOT_4_LOCKED,
           EC_RM_TRX_2_SLOT_5_LOCKED, EC_RM_TRX_2_SLOT_6_LOCKED,
	     EC_RM_TRX_2_SLOT_7_LOCKED, EC_RM_TRX_2_SLOT_8_LOCKED}
	  };

const Alarm_errorCode_t
      rm_TrxSlotAdminUnlockedAlarmCode[OAMrm_MAX_TRXS][OAMrm_SLOTS_PER_TRX]
      = {
	    {EC_RM_TRX_1_SLOT_1_UNLOCKED, EC_RM_TRX_1_SLOT_2_UNLOCKED,
	     EC_RM_TRX_1_SLOT_3_UNLOCKED, EC_RM_TRX_1_SLOT_4_UNLOCKED,
           EC_RM_TRX_1_SLOT_5_UNLOCKED, EC_RM_TRX_1_SLOT_6_UNLOCKED,
	     EC_RM_TRX_1_SLOT_7_UNLOCKED, EC_RM_TRX_1_SLOT_8_UNLOCKED},
	
	    {EC_RM_TRX_2_SLOT_1_UNLOCKED, EC_RM_TRX_2_SLOT_2_UNLOCKED,
	     EC_RM_TRX_2_SLOT_3_UNLOCKED, EC_RM_TRX_2_SLOT_4_UNLOCKED,
           EC_RM_TRX_2_SLOT_5_UNLOCKED, EC_RM_TRX_2_SLOT_6_UNLOCKED,
	     EC_RM_TRX_2_SLOT_7_UNLOCKED, EC_RM_TRX_2_SLOT_8_UNLOCKED}
	  };

const Alarm_errorCode_t
      rm_TrxSlotAdminShuttingDownAlarmCode[OAMrm_MAX_TRXS][OAMrm_SLOTS_PER_TRX]
      = {
	    {EC_RM_TRX_1_SLOT_1_SHUTTINGDOWN, EC_RM_TRX_1_SLOT_2_SHUTTINGDOWN,
	     EC_RM_TRX_1_SLOT_3_SHUTTINGDOWN, EC_RM_TRX_1_SLOT_4_SHUTTINGDOWN,
           EC_RM_TRX_1_SLOT_5_SHUTTINGDOWN, EC_RM_TRX_1_SLOT_6_SHUTTINGDOWN,
	     EC_RM_TRX_1_SLOT_7_SHUTTINGDOWN, EC_RM_TRX_1_SLOT_8_SHUTTINGDOWN},
	
	    {EC_RM_TRX_2_SLOT_1_SHUTTINGDOWN, EC_RM_TRX_2_SLOT_2_SHUTTINGDOWN,
	     EC_RM_TRX_2_SLOT_3_SHUTTINGDOWN, EC_RM_TRX_2_SLOT_4_SHUTTINGDOWN,
           EC_RM_TRX_2_SLOT_5_SHUTTINGDOWN, EC_RM_TRX_2_SLOT_6_SHUTTINGDOWN,
	     EC_RM_TRX_2_SLOT_7_SHUTTINGDOWN, EC_RM_TRX_2_SLOT_8_SHUTTINGDOWN}
	  };

//GP2
#if 0
void rm_OamProcTrapRadioCarrierPackage(u8 rcno, MibTag tag)
{
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcTrapRadioCarrierPackage\n"));
	
	// Read out OAM data 
	if (rcno == 0)
	{
		rm_OamGetBtsFirstRadioCarrierPackage();
		
	} else {
		rm_OamGetBtsSecondRadioCarrierPackage();
	}
	
	switch (tag)
	{
        case MIBT_carrierFrequency_0:
        case MIBT_carrierFrequency_1:
             if (OAMrm_BTS_OPSTATE==opStateEnabled)
             {
                 //Tune to new freqs
                 //PR1074
                 if (OAMrm_BTS_AMSTATE != locked && OAMrm_TRX_AMSTATE(0) != locked &&
                     OAMrm_CHN_AMSTATE(0,0) != locked)
                 { 
                     rm_SendTuneSynthToTrx(0,1);
                     if (OAMrm_TRX_AMSTATE(1)!= locked)
                         rm_SendTuneSynthToTrx(1,1);
                 } 
             }
             break;
	case MIB_txPwrMaxReduction_0:
		if (OAMrm_TRX_RC(0) == rcno)
		{
			rm_OamSendPwrRedStepToDsp(0);
		} else {
			rm_OamSendPwrRedStepToDsp(1);
		}
		break;
	case MIB_txPwrMaxReduction_1:
		if (OAMrm_TRX_RC(0) == rcno)
		{
			rm_OamSendPwrRedStepToDsp(0);
		} else {
			rm_OamSendPwrRedStepToDsp(1);
		}
		break;
	default:
            // do nothing for now.
            IDEBUG__(("No message was sent to DSP"));
            break;
 	}	
}
void rm_OamRegTrapVars(void)
{
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamRegTrapVars\n"));
	
	// Reset any trap setting to get a clean start
	if (STATUS_OK != oam_unsetTrapByModule(MODULE_RM))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @unsetTrapByModule\n"));

        //GP2
        return;
    
	// Regsiter to trap BtsBasicPackage
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_btsBasicPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_btsBasicPackage\n"));
	
	// Register to trap BtsOptionsPackage
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_btsOptionsPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_btsOptionsPackage\n"));
	
	// Register to trap two TransceiverPackages
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_firstTransceiverPackage))
	     EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_firstTransceiverPackage\n"));
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_secondTransceiverPackage))
	     EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_secondTransceiverPackage\n"));
	
	// Register to trap two RadioCarrierPackages
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_firstRadioCarrierPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_firstRadioCarrierPackage\n"));
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_secondRadioCarrierPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_secondRadioCarrierPackage\n"));
	
	// Regsiter to trap T31xx timers
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_t31xxPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_t31xxPackage\n"));
	
	// Register to trap AdjacentCellPackage
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_adjacentCellPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_adjacentCellPackage\n"));
	
	// Register to trap HandoverControlPackage
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_handoverControlPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_handoverControlPackage\n"));
	
	if ( STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoAveragingAdjCellPackage) 
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoAveragingDistPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoAveragingLevPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoAveragingQualPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoThresholdDistPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoThresholdInterfacePackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoThresholdLevPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_hoThresholdQualPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_interferenceAveragingPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/Nested sturcture of handoverControlPackage\n"));
	
	// Register to trap PowerControlPackage
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_powerControlPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_powerControlPackage\n"));
	
	if ( STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_pcAveragingLevPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_pcAveragingQualPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_pcLowerThresholdLevPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_pcLowerThresholdQualPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_pcUpperThresholdLevPackage)
		||  STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_pcUpperThresholdQualPackage))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/Nested sturcture of Power Control Package\n"));
	
	
	
	// Register to trap ResourceConfigData 
	if (STATUS_OK != oam_setTrap(MODULE_RM, rm_MsgQId, MIB_resourceConfigData))
		EDEBUG__(("ERROR-rm_OamRegTrapVars: OAM API failed @oam_setTrap/MIB_resourceConfigData\n"));
}

void rm_OamGetTrapVars(void)
{
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamGetTrapVars\n"));
	
	//Process in terms of message type
	switch ( rm_pItcRxOamMsg->msgType )
	{
	case MT_TRAP:
           //Got a trap notification
           switch (MIB_TBL(rm_pItcRxOamMsg->mibTag))
           {
	     case MTBL_btsBasicPackage: 			//0
	   	    rm_OamProcTrapBtsBasicPackage();
		    break;
           case MTBL_btsOptionsPackage: 			//1
		    rm_OamProcTrapBtsOptionsPackage();
		    break;
	     case MTBL_firstTransceiverPackage: 		//2
	          rm_OamProcTrapTransceiverPackage(0);
	          break;
	     case MTBL_secondTransceiverPackage: 		//3
	          rm_OamProcTrapTransceiverPackage(1);
	          break;
	     case MTBL_firstRadioCarrierPackage: 		//4
	          rm_OamProcTrapRadioCarrierPackage(0, rm_pItcRxOamMsg->mibTag);
	  	    break;
	     case MTBL_secondRadioCarrierPackage: 	//5
	          rm_OamProcTrapRadioCarrierPackage(1, rm_pItcRxOamMsg->mibTag);
		    break;
           case MTBL_t31xx: 					//8
	  	    rm_OamProcTrapT31xx();
		    break;
           case MTBL_adjacentCellPackage: 		//9
		    rm_OamProcAdjacentCellPackage();
		    break;
           case MTBL_handoverControlPackage: 		//11
	     case MTBL_hoAveragingAdjCellParam: 		//12
	     case MTBL_hoAveragingDistParam:  		//13
	     case MTBL_hoAveragingLevParam:  		//14
	     case MTBL_hoAveragingQualParam:  		//15
	     case MTBL_hoThresholdDistParam:  		//16
	     case MTBL_hoThresholdInterfaceParam:  	//17
	     case MTBL_hoThresholdLevParam:  		//18
	     case MTBL_hoThresholdQualParam:  		//19
	     case MTBL_interferenceAveragingParam: 	//20	
	          rm_OamProcHandoverControlPackage();
	          break;
           case MTBL_powerControlPackage: 		//21
	     case MTBL_pcAveragingLev:  			//22
	     case MTBL_pcAveragingQual:  			//23
	     case MTBL_pcLowerThresholdLevParam:  	//24
	     case MTBL_pcLowerThresholdQualParam:  	//25
	     case MTBL_pcUpperThresholdLevParam:  	//26
	     case MTBL_pcUpperThresholdQualParam: 	//27
		    rm_OamProcPowerControlPackage();
		    break;
           case MTBL_resourceConfigData: 			//32
	  	    rm_OamProcResourceConfigData();
		    break;
           default:						//??
		    //Received unrecognized message type
		    EDEBUG__(( "ERROR-rm_OamGetTrapVars: unexpected OAM table:%d\n",
				    MIB_TBL(rm_pItcRxOamMsg->mibTag) ));
	   	    break;
           }
           break;
		
	default:
	     //Received unrecognized message type
	     EDEBUG__(( "ERROR-rm_OamGetTrapVars: unexpected OAM msg type:%d\n",
			     rm_pItcRxOamMsg->msgType ));
	     break;
	}        
}

void rm_OamProcTrapBtsBasicPackage(void)
{
	u8 i,j;
      EnableDisable opState;
 
      opState = OAMrm_BTS_OPSTATE;
	
	//Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcTrapBtsBasicPackage\n"));

	// Retrieve BtsBasicPackage
	if ( STATUS_OK != rm_OamGetBtsBasicPackage() )
	{
           //Received unrecognized message tag
           EDEBUG__(( "ERROR-rm_OamProcTrapBtsBasicPackage:failed@rm_OamGetBtsBasicPackage()\n"));
           return;
	}

      //Check if BTS operational state is changed or not
	if (rm_pItcRxOamMsg->mibTag==MIB_bts_operationalState)
      {
	    rm_ProcBtsOpStateChange();
          return;
      }

      //Check if got BTS administrative state changed notifica
	if (rm_pItcRxOamMsg->mibTag==MIB_bts_administrativeState)
      {
	    rm_ProcBtsAdminStateChange();
          return;
      }

	//Blindly update trx-es
	i = 0;
	while (i<OAMrm_MAX_TRXS)
	{ 
		if (rm_TrxMgmt[i].state==RM_TRXMGMT_S3_TRXCONFIG_END)
		{   //trx i is operating right now and update it!
		    rm_SendSacchFillingSI5(i, RM_L1SYSINFO_TYPE_5);	    	    
		    rm_SendSacchFillingSI6(i, RM_L1SYSINFO_TYPE_6);
		    rm_SendTrxConfigToTrx(i);

                //PR1039
                rm_SendBtsPackageToTrx(i);
		    if (i==0) //Assume bcch trx, pass SI
                {
                    if (OAMrm_OVERLOAD_CELLBAR && rm_VcIsBarred)
                    {
                        rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);        
                        rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);        
                        rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);        
                    } else
                    { 
                          rm_VcIsBarred = RM_FALSE;
		              rm_SendAllSystemInfosToBcchTrx(i);
                    } 
                }
		}
		i++;
	}

      return;

#if 0	
	switch (rm_pItcRxOamMsg->mibTag)
	{
	case MIB_bts_ncc:
	case MIB_bts_cid:
	case MIB_btsID:
	case MIB_cellAllocationTable:
	case MIBT_cellAllocationEntry:
	case MIBT_cellAllocationIndex:
	case MIBT_cellAllocation:
	case MIB_gsmdcsIndicator:
	case MIB_bts_mcc:
	case MIB_bts_mnc:
	case MIB_bts_lac:
	case MIB_bts_ci:
	case MIB_cellReselectHysteresis:
	case MIB_ny1:
	case MIB_plmnPermittedTable:
	case MIBT_plmnPermittedEntry:
	case MIBT_plmnPermittedIndex:
	case MIBT_plmnPermitted:
	case MIB_radioLinkTimeout:
	case MIB_relatedTranscoder:
	case MIB_rxLevAccessMin:
	case MIB_bts_administrativeState:
	case MIB_bts_alarmStatus:
	case MIB_bts_operationalState:
	case MIB_maxNumberRetransmissions:
	case MIB_mSTxPwrMaxCCH:
	case MIB_numberOfSlotsSpreadTrans:
	case MIB_noOfBlocksForAccessGrant:
	case MIB_noOfMultiframesBetweenPaging:
	default:
           //Received unrecognized message tag
           EDEBUG__(("ERROR-rm_OamProcTrapBtsBasicPackage: unexpected OAM mib tag:%x\n",
			    rm_pItcRxOamMsg->mibTag));
           break;
	}
#endif

}


void rm_OamProcTrapBtsOptionsPackage(void)
{
	u8 i;
	
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcTrapBtsOptionsPackage\n"));
	
	// Retrieve OamGetBtsOptionPackage
	if ( STATUS_OK != rm_OamGetBtsOptionPackage() )
	{
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcTrapBtsOptionsPackage: failed@rm_OamGetBtsOptionPackage()\n"));
        return;
	}
	
	// Update each trx here
	i = 0;
	while (i<OAMrm_MAX_TRXS)
	{ 
		if (rm_TrxMgmt[i].state==RM_TRXMGMT_S3_TRXCONFIG_END)
		{   //trx i is operating right now and update it!
			rm_SendSacchFillingSI5(i, RM_L1SYSINFO_TYPE_5);	    	    
			rm_SendSacchFillingSI6(i, RM_L1SYSINFO_TYPE_6);
			//rm_SendTrxConfigToTrx(i);

                  //PR1381
                  rm_SendBtsPackageToTrx(i);

			if (i==0) //Assume bcch trx, pass SI
                  {
                    if (OAMrm_OVERLOAD_CELLBAR && rm_VcIsBarred)
                    {
                        rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);        
                        rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);        
                        rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);        
                    } else
                    {
                        rm_VcIsBarred = RM_FALSE;
		            rm_SendAllSystemInfosToBcchTrx(i);
                    } 
                  }   
		}
		i++;
	}
	
	return;

#if 0	
	switch (rm_pItcRxOamMsg->mibTag)
	{
	case MIB_allowIMSIAttachDetach:
	case MIB_callReestablishmentAllowed:
	case MIB_cellBarred:
	case MIB_dtxDownlink:
	case MIB_dtxUplink:
	case MIB_emergencyCallRestricted:
	case MIB_notAllowedAccessClassTable:
	case MIBT_notAllowedAccessClassEntry:
	case MIBT_notAllowedAccessClassIndex:
	case MIBT_notAllowedAccessClass:
	case MIB_timerPeriodicUpdateMS:
	case MIB_maxQueueLength:
	case MIB_msPriorityUsedInQueueing:
	case MIB_timeLimitCall:
	case MIB_timeLimitHandover:
	default:
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcTrapBtsOptionsPackage: unexpected OAM mib tag:%x\n",
			rm_pItcRxOamMsg->mibTag ));
        break;
	}

#endif

}

void rm_OamProcTrapT31xx(void)
{
	u8 i;
	
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcTrapT31xx\n"));
	
	if ( STATUS_OK != rm_OamGetT31xxTimerStructure() )
	{
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcTrapT31xx: failed@rm_OamGetT31xxTimerStructure()\n"));
        return;
	}
	/* PR1256 <convert from tick number to 10ms > begin*/
	int tickNumPerSec = sysClkRateGet ();
	
	rm_OamData.t31xx.t3101 = ( rm_OamData.t31xx.t3101 * 10 ) / tickNumPerSec ;	
	
	rm_OamData.t31xx.t3105 = ( rm_OamData.t31xx.t3105 * 10 ) / tickNumPerSec ;	

	rm_OamData.t31xx.t3109 = ( rm_OamData.t31xx.t3109 * 10 ) / tickNumPerSec ;	

	rm_OamData.t31xx.t3111 = ( rm_OamData.t31xx.t3111 * 10 ) / tickNumPerSec ;	
	
	rm_TestPrintOamRmStdGsmData();
	/* PR1256 <convert from tick number to 10ms > end*/

	// Update timer values accordingly
	for (i=0;i<OAMrm_MAX_TCHFS;i++)
	{
		rm_PhyChanTchf[i].T3101.time  = OAMrm_T3101;
		rm_PhyChanTchf[i].T3105.time  = OAMrm_T3105;
		rm_PhyChanTchf[i].T3109.time  = OAMrm_T3109;
		rm_PhyChanTchf[i].T3111.time  = OAMrm_T3111;
	}
	for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
	{
     	     rm_PhyChanSdcch4[i].T3101.time  = OAMrm_T3101;
	     rm_PhyChanSdcch4[i].T3105.time  = OAMrm_T3105;
	     rm_PhyChanSdcch4[i].T3109.time  = OAMrm_T3109;
	     rm_PhyChanSdcch4[i].T3111.time  = OAMrm_T3111;
	}
	
}

void rm_OamProcAdjacentCellPackage(void)
{
	u8 i,j,ncellNo;
	
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcAdjacentCellPackage\n"));
	
	// Read out OAM data 
	if ( STATUS_OK != rm_adjacentCellPackage() )
	{
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcAdjacentCellPackage: failed@rm_adjacentCellPackage()\n"));
        return;
	}
	
	switch(MIB_ITM(rm_pItcRxOamMsg->mibTag))
	{
	case LEAF_adjCell_reselectionTable:
           if (rm_TrxMgmt[0].state==RM_TRXMGMT_S3_TRXCONFIG_END)
           {
               //Assume trx 0 is bcch trx here
               if (OAMrm_OVERLOAD_CELLBAR && rm_VcIsBarred)
               {
                   rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);
               } else
               {
                   rm_VcIsBarred = RM_FALSE;
                   rm_SendSystemInfo2ToBcchTrx(0,RM_L1SYSINFO_TYPE_2,OAMrm_RA_CELL_BARRED_STATE);
               }
           }
           break;
	case LEAF_adjCell_handoverTable:
        i = 0;
        while ( i<OAMrm_MAX_TRXS )
        { 
            // Pass updated information to both mobile and DSP
            if (rm_TrxMgmt[i].state==RM_TRXMGMT_S3_TRXCONFIG_END)
            {
                rm_SendSacchFillingSI5(i, RM_L1SYSINFO_TYPE_5);	    	    
                ncellNo=rm_OamGetTotalAvailableHoNCells();
                for (j=0;j<OAMrm_HO_ADJCELL_NO_MAX;j++)
					rm_OamSendHoNCellToDsp(i,j,ncellNo);
            }
            i++;
        }
        break;
	default:
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcAdjacentCellPackage: unexpected OAM mib Table tag:%x\n",
			MIB_ITM(rm_pItcRxOamMsg->mibTag) ));
        break;
		
	}
}

void rm_OamProcHandoverControlPackage(void)
{
	int i;
	
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcHandoverControlPackage\n"));
	
	// Read out OAM data 
	if ( STATUS_OK != rm_handoverControlPackage() )
	{
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcHandoverControlPackage: failed@rm_handoverControlPackage()\n"));
        return;
	}
	
	// Update DSP with new HandoverControlPackage
	i = 0;
	while ( i<OAMrm_MAX_TRXS )
	{ 
		// Pass updated information to both mobile and DSP
		if (rm_TrxMgmt[i].state==RM_TRXMGMT_S3_TRXCONFIG_END)
			rm_OamSendHoSCellToDsp(i);
		i++;
	}
}

void rm_OamProcPowerControlPackage(void)
{ 
	int i;
	
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcPowerControlPackage\n"));
	
	// Read out OAM data 
	if ( STATUS_OK != rm_powerControlPackage() )
	{
        // Received unrecognized message tag
        EDEBUG__(( "ERROR-rm_OamProcPowerControlPackage: failed@rm_powerControlPackage()\n"));
        return;
	}
	
	// Update DSP with new power control package
	i = 0;
	while ( i<OAMrm_MAX_TRXS )
	{ 
		// Pass updated information to both mobile and DSP
		if (rm_TrxMgmt[i].state==RM_TRXMGMT_S3_TRXCONFIG_END)
			rm_OamSendPcToDsp(i);
		i++;
	}
}

void rm_OamProcResourceConfigData(void)
{
	int i;
      int opState;
	
	// Monitoring function entrance 
	RDEBUG__(("ENTER-rm_OamProcResourceConfigData\n"));

      opState = OAMrm_CBCH_OPSTATE;
	
	// Read out OAM data 
	rm_OamGetRmConfigData();

      switch (rm_pItcRxOamMsg->mibTag)
      {
      case MIB_rm_nim_0_1:
           //OAMrm_CBCH_OPSTATE=rm_pItcRxOamMsg->val.varVal;
           switch(OAMrm_CBCH_OPSTATE)
           {
           case 0: //Disable CBCH channel
                if (!rm_ChanCbchActive)
                     PDEBUG__(("INFO@rm_OamProcResourceConfigData: rel CBCH but it's not ON\n"));
                rm_ChanCbchActive = RM_FALSE;
                rm_SendRfChanRelease(rm_pCbchChan);
                if (OAMrm_OVERLOAD_CELLBAR && rm_VcIsBarred)
                {
                    rm_SendSystemInfo4ToBcchTrx(0, RM_L1SYSINFO_TYPE_4,CNI_RIL3_CELL_BARRED_ACCESS);
                } else
                {
                    rm_VcIsBarred = RM_FALSE;
                    rm_SendSystemInfo4ToBcchTrx(0, RM_L1SYSINFO_TYPE_4,OAMrm_RA_CELL_BARRED_STATE);
                }
                break;              
           case 1: //Enable CBCH Channel
                if (rm_ChanCbchActive)
                    PDEBUG__(("INFO@rm_OamProcResourceConfigData: turn on a already on cbch\n"));
                if (rm_pCbchChan->usable == RM_PHYCHAN_UNUSABLE)
                {
                    IDEBUG__(("WARNIG@rm_OamProcResourceConfigData: SDCCH(2) unusable: st=%d\n",
                               rm_pCbchChan->usable));
                    return;
                } 

		    //block any SDCCH attempt also activate CBCH 
                rm_ChanCbchActive = RM_FALSE;
                rm_pCbchChan->usable = RM_PHYCHAN_USABLE;
                if (rm_pCbchChan->state == RM_PHYCHAN_FREE)
                {
                    rm_pCbchChan->usable = RM_PHYCHAN_CBCH_USABLE; 
                    if ( rm_pCbchChan->pMSmCtxt->state != RM_S1RM_RUNNING ||
                         rm_pCbchChan->pASmCtxt->state != RM_S1RM_RUNNING)
                    {
                         IDEBUG__(("WARNING@rm_OamProcResourceConfigData:invalid chan state(%d,%d,%d)\n",
                                   rm_pCbchChan,rm_pCbchChan->pMSmCtxt->state,
                                   rm_pCbchChan->pASmCtxt->state));
                         rm_pCbchChan->pMSmCtxt->state = RM_S1RM_RUNNING;
                         rm_pCbchChan->pASmCtxt->state = RM_S1RM_RUNNING;
                   }
                   rm_ActivateCbchChan();
                }
                break;
           default:
                IDEBUG__(("WARNING@rm_OamProcResourceConfigData: invalid cbchOpState=%d\n",
				   OAMrm_CBCH_OPSTATE));
                OAMrm_CBCH_OPSTATE = opState;
                break;
           }
           break;

      case MIB_rm_nim_0_3:
      case MIB_rm_nim_0_4: 
      case MIB_rm_nim_0_5:
      case MIB_rm_nim_0_6:
           if (OAMrm_OVERLOAD_CELLBAR && rm_VcIsBarred)
           {
               rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);
               rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);
           } else
           {
               rm_VcIsBarred = RM_FALSE;
		   rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);
               rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
           }
           break;

      //case MIB_rm_nim_0_7: auth/cipher feature control
      case MIB_rm_nim_1_1:
           rm_VcIsBarred = RM_FALSE;
           rm_SendSystemInfo2ToBcchTrx(0,2,OAMrm_RA_CELL_BARRED_STATE);        
           rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);        
           rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
           break;
         
      default:
           IDEBUG__(("WARNING@rm_OamProcResourceConfigData: invalid MIB var change occuring=%d\n",
 		          rm_pItcRxOamMsg->mibTag));
           break;
      } 

      return;

#if 0
	//Update the system accordingly
	switch(rm_pItcRxOamMsg->mibTag)
	{
	case MIB_rm_SGainCtrlSetting_0_0:
	case MIB_rm_SGainCtrlSetting_0_1:
	case MIB_rm_SGainCtrlSetting_1_0:
	case MIB_rm_SGainCtrlSetting_1_1:
           i = 0;
           while ( i<OAMrm_MAX_TRXS )
           { 
                  // Pass updated information to both mobile and DSP
                  if (rm_TrxMgmt[i].state==RM_TRXMGMT_S3_TRXCONFIG_END)
                  {
                      rm_SendSetDGainToTrx(i);
                      rm_SendSetUGainToTrx(i);
                  }
                  i++;
           }
           break;
	default:
           break;
	}

#endif

}

void rm_OamProcTrapTransceiverPackage(u8 trx)
{
     u8 i,inUse;

     // Monitoring function entrance 
     RDEBUG__(("ENTER@rm_OamProcTrapTransceiverPackage\n"));
     PDEBUG__(("INFO@rm_OamProcTrapTransceiverPackage: trap notif for trx=%d\n", trx));

     // Read the whole Transceiver package if any of them changed
     if (trx == 0)
     {
         if (STATUS_OK != rm_OamGetBtsFirstTrxPackage() )
             EDEBUG__(("ERROR@rm_OamProcTrapTransceiverPackage: error in reading trx0 data\n"));
     }
     else
     {
         if (STATUS_OK != rm_OamGetBtsSecondTrxPackage() )
             EDEBUG__(("ERROR@rm_OamProcTrapTransceiverPackage: error in reading trx1 data\n"));
     }
      
     switch(rm_pItcRxOamMsg->mibTag)
     {
     case MIB_basebandAdministrativeState_0:
	    //Got TRX0 adminState change notification
          switch(rm_pItcRxOamMsg->val.varVal)
          {
          case locked:
               //Treat this as BTS amState locked
               rm_SetBtsAdminState(locked);
               rm_ProcBtsAdminStateLocked();
               break;

          case unlocked:
               //Only unlock trx0
               rm_SetBtsAdminState(unlocked);
               rm_ProcBtsAdminStateUnlocked();
               //rm_ProcTrxAdminStateUnlocked(0);
               break;

	    case shuttingDown:
               //Treat this as BTS amState shuttingdown
               rm_SetBtsAdminState(shuttingDown);
               rm_ProcBtsAdminStateShuttingDown();
               break;
          
          default:
               IDEBUG__(("WARNING@rm_OamProcTrapTransceiverPackage:trx0,invalid amStateValue:%d\n",
				  rm_pItcRxOamMsg->val.varVal));
               break;
	    }
          break;         

     case MIB_basebandAdministrativeState_1:
	    //Got TRX1 adminState change notification
          switch(rm_pItcRxOamMsg->val.varVal)
          {
          case locked:
               rm_ProcTrxAdminStateLocked(1);
               break;
          case unlocked:
		   rm_ProcTrxAdminStateUnlocked(1);
		   break;
          case shuttingDown:
               rm_ProcTrxAdminStateShuttingDown(1, &inUse);
               break;
          default:
               IDEBUG__(("WARNING@rm_OamProcTrapTransceiverPackage:trx1,invalid amStateValue:%d\n",
				  rm_pItcRxOamMsg->val.varVal));
               break;
	    }
          break;
     case MIBT_channelAdministrativeState_0:
	    //Got TRX0 TS adminState change notificat
          switch(rm_pItcRxOamMsg->val.varTbl.value)
          {
          case locked:
		   if (!rm_pItcRxOamMsg->val.varTbl.idx)
               {
                    PDEBUG__(("LOCK Physical Channel (0,0): chnIdx=%d\n",
                               rm_pItcRxOamMsg->val.varTbl.idx)); 
                    OAMrm_BTS_AMSTATE = locked;
                    rm_SetBtsAdminState(locked);
			  rm_ProcBtsAdminStateLocked();
               } else
	              rm_ProcTrxSlotAdminStateLocked(0, rm_pItcRxOamMsg->val.varTbl.idx);
               break;
          case unlocked:
		   if (!rm_pItcRxOamMsg->val.varTbl.idx)
               {
                    PDEBUG__(("UNLOCK Physical Channel (0,0)\n", 
					 rm_pItcRxOamMsg->val.varTbl.idx)); 
                    OAMrm_BTS_AMSTATE = unlocked;
                    rm_SetBtsAdminState(unlocked);
			  rm_ProcBtsAdminStateUnlocked();
               } else
                    rm_ProcTrxSlotAdminStateUnlocked(0, rm_pItcRxOamMsg->val.varTbl.idx);
		   break;
          case shuttingDown:
		   if (!rm_pItcRxOamMsg->val.varTbl.idx)
               {
                    PDEBUG__(("SHUTTING DOWN Physical Channel (0,0)\n",
                               rm_pItcRxOamMsg->val.varTbl.idx)); 
                    OAMrm_BTS_AMSTATE = shuttingDown;
                    rm_SetBtsAdminState(shuttingDown);
			  rm_ProcBtsAdminStateShuttingDown();
               } else
                    rm_ProcTrxSlotAdminStateShuttingDown(0, rm_pItcRxOamMsg->val.varTbl.idx,&inUse);
               break;
          default:
               IDEBUG__(("WARNING@rm_OamProcTrapTransceiverPackage:(0,%d),invalid amStateValue:%d\n",
				  rm_pItcRxOamMsg->val.varTbl.idx,rm_pItcRxOamMsg->val.varTbl.value));
               break;

	    }
          break;
	    
     case MIBT_channelAdministrativeState_1:
	    //Got TRX0 TS adminState change notification
          switch(rm_pItcRxOamMsg->val.varTbl.value)
          {
          case locked:
               rm_ProcTrxSlotAdminStateLocked(1, rm_pItcRxOamMsg->val.varTbl.idx);
               break;
          case unlocked:
               rm_ProcTrxSlotAdminStateUnlocked(1, rm_pItcRxOamMsg->val.varTbl.idx);
		   break;
          case shuttingDown:
               rm_ProcTrxSlotAdminStateShuttingDown(1, rm_pItcRxOamMsg->val.varTbl.idx,&inUse);
               break;
          default:
               IDEBUG__(("WARNING@rm_OamProcTrapTransceiverPackage:(1,%d),invalid amStateValue:%d\n",
				  rm_pItcRxOamMsg->val.varTbl.idx,rm_pItcRxOamMsg->val.varTbl.value));
               break;

	    }
          break;

     default:
          //Handle other trapped notification here
          break;
     }
}

void rm_ProcBtsOpStateChange(void)
{
     int i;

     PDEBUG__(("INFO@rm_ProcBtsOpStateChange:opState=%d\n",
                rm_pItcRxOamMsg->val.varVal));

     switch(rm_pItcRxOamMsg->val.varVal)
     {
     case opStateEnabled:
          //Turn on RF transmission if needed
          if ( OAMrm_BTS_AMSTATE == locked        ||
               OAMrm_TRX_AMSTATE(0) == locked     ||
               OAMrm_CHN_AMSTATE(0,0) == locked  )
               break;
          
          for (i=0;i<OAMrm_MAX_TRXS;i++)
          {
               //PR1074
               if ( ((OAMrm_TRX_ONOFF_MAP)&(1<<i)) &&
                    (OAMrm_TRX_AMSTATE(i) != locked) )
                     rm_SendTuneSynthToTrx(i,1);
          }
          break;

     case opStateDisabled:
          //Turn off RF transmission
          for (i=0;i<OAMrm_MAX_TRXS;i++)
          {
               if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
                     rm_SendTuneSynthToTrx(i,0);
          }
          break;
     default:
          EDEBUG__(("ERROR@rm_ProcBtsOpStateChange:invalid opStateValue:%d\n",
	               rm_pItcRxOamMsg->val.varVal));
          //Block RM itself in this case <TBD>
          OAMrm_BTS_OPSTATE = opStateDisabled;
          break;
     }
}

void  rm_ProcBtsAdminStateChange(void)
{   
      PDEBUG__(("INFO@rm_ProcBtsAdminStateChange\n"));

      //In terms of amState change notif
      switch(rm_pItcRxOamMsg->val.varVal)
      {
      case locked:
           rm_ProcBtsAdminStateLocked();
	     break;

      case unlocked:
           rm_ProcBtsAdminStateUnlocked();
           break;

      case shuttingDown:
	     rm_ProcBtsAdminStateShuttingDown();
           break;

      default:
           IDEBUG__(("WARNING@rm_ProcBtsAdminStateChange:invalid amState:%d\n",
		          rm_pItcRxOamMsg->val.varVal));
           break;
      }
}

void rm_ProcBtsAdminStateLocked(void)
{
     u8 i;
  
     PDEBUG__(("INFO@rm_ProcBtsAdminStateLocked()\n"));

     //Report informational alarm
     rm_SetRmOpState(EC_RM_BTS_LOCKED);

     //Lock all TRXes in existence
     for (i=0; i<OAMrm_MAX_TRXS; i++ )
     { 
          //Check if ith TRX is existent
          if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
     	    {  
	       rm_SetTrxAdminState(i,locked);
	       rm_ProcTrxAdminStateLocked(i);
          }
     }
}

void rm_ProcTrxAdminStateLocked(u8 trx)
{
     u8 i;

     PDEBUG__(("INFO@rm_ProcTrxAdminStateLocked: trx=%d\n", trx));

     //Check if the requested trx existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) )
     {
	   IDEBUG__(("WARNING@rm_ProcTrxAdminStateLocked: invalid trx=%d\n",trx));
	   return;
     }

     //Report informational alarm
     rm_SetRmOpState(rm_TrxAdminLockedAlarmCode[trx]);

     //Turn off RF transmission
     rm_SendTuneSynthToTrx(trx,0);

     //Deactivate all channels on trx
     for (i=0;i<OAMrm_SLOTS_PER_TRX;i++)
     {
	    rm_SetTrxSlotAdminState(trx,i,locked);
          rm_ProcTrxSlotAdminStateLocked(trx,i);
     } 
}

void rm_ProcTrxSlotAdminStateLocked(u8 trx, u8 slot)
{
     PDEBUG__(("INFO@rm_ProcTrxSlotAdminStateLocked:(%d,%d)\n",trx, slot));

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
	   slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_ProcTrxSlotAdminStateLocked: invalid (%d,%d)\n",
		        trx,slot));
	   return;
     }

     //Report informational alarm
     rm_SetRmOpState(rm_TrxSlotAdminLockedAlarmCode[trx][slot]);

     //Lock func channels on (trx,slot)
     rm_LockedAllTrxSlotChans(trx,slot);

     //Check if no TCHF or SDCCH4 channels available

     if (RM_SMCTXT_NULL == rm_PreAllocSmCtxt(RM_PHYCHAN_SDCCH4) )
     { 
         if (!rm_AllAvailableSdcch4.allocated)
         {
             rm_AllAvailableSdcch4.allocated = RM_TRUE;
             PM_CellMeasurement.allAvailableSDCCHAllocatedTime.start();

             //rm_AllAvailableSdcch4.startTime = tickGet();
         }
     }

     if (RM_SMCTXT_NULL == rm_PreAllocSmCtxt(RM_PHYCHAN_TCHF_S) )
     { 
         if (!rm_AllAvailableTchf.allocated)
         {
             rm_AllAvailableTchf.allocated = RM_TRUE;
             PM_CellMeasurement.allAvailableTCHAllocatedTime.start();

             //rm_AllAvailableTchf.startTime = tickGet();
         }
     }

     //Turn off RF transmission if need
     if (!trx && !slot)
     {
         rm_SendTuneSynthToTrx(trx,0);
         //if (rm_AmState00 != shuttingDown)
         //    rm_CellBarred = OAMrm_RA_CELL_BARRED_STATE;
         //rm_AmState00 = locked;
     }
}

void rm_ProcBtsAdminStateUnlocked(void)
{
     u8 i;

     PDEBUG__(("INFO@rm_ProcBtsAdminStateUnlocked\n"));

     //Report informational alarm
     rm_SetRmOpState(EC_RM_BTS_UNLOCKED);

     //Lock all TRXes in existence
     for (i=0; i<OAMrm_MAX_TRXS; i++ )
     { 
          //Check if ith TRX is existent
          if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
     	    {  
	         rm_ProcTrxAdminStateUnlocked(i);
	         rm_SetTrxAdminState(i,unlocked);
          }
     }

}

void rm_ProcTrxAdminStateUnlocked(u8 trx)
{
     u8 i;

     PDEBUG__(("INFO@rm_ProcTrxAdminStateUnlocked:trx=%d\n",trx));

     //Check if the requested trx existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS))
     {
	   IDEBUG__(("WARNING@rm_ProcTrxAdminStateUnlocked: invalid trx=%d\n",trx));
	   return;
     }

     //Report informational alarm
     rm_SetRmOpState(rm_TrxAdminUnlockedAlarmCode[trx]);

     //Unlock all channels on trx
     for (i=0;i<OAMrm_SLOTS_PER_TRX;i++)
     {
          rm_ProcTrxSlotAdminStateUnlocked(trx,i);
	    rm_SetTrxSlotAdminState(trx,i,unlocked);
     } 

     //Turn on RF transmission
     rm_SendTuneSynthToTrx(trx,1);
}


void rm_ProcTrxSlotAdminStateUnlocked(u8 trx, u8 slot)
{

     PDEBUG__(("INFO@rm_ProcTrxSlotAdminStateUnlocked:(%d,%d)\n",trx,slot));

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
          slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_ProcTrxSlotAdminStateUnlocked: invalid (%d,%d)\n",
		      trx,slot));
	   return;
     }

     //Report informational alarm
     rm_SetRmOpState(rm_TrxSlotAdminUnlockedAlarmCode[trx][slot]);

     //Unlock all channels under trx/slot
     rm_UnlockedAllTrxSlotChans(trx, slot);
     
     //Turn on RF transmission if needed
     if (trx || slot)
     { 
         //rm_AmState00 = unlocked;
         //OAMrm_RA_CELL_BARRED_STATEa = (Boolean) rm_CellBarred;
         //rm_SendSystemInfo2ToBcchTrx(0,2,OAMrm_RA_CELL_BARRED_STATE);        
         //rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);        
         //rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
         rm_SendTuneSynthToTrx(trx,1);
     }
}


void rm_UnlockedAllTrxSlotChans(u8 trx, u8 slot)
{
     int i;
     
     PDEBUG__(("INFO@rm_UnlockedAllTrxSlotChans:(%d,%d)\n",trx,slot));

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
          slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_UnlockedAllTrxSlotChans: invalid (%d,%d)\n",
		      trx,slot));
	   return;
     }
     
     int onBar,newFreeChan;
     onBar = 0; newFreeChan = 0;

     if (trx || slot)
     {
	 //scan through tchf table to excerise the adminState change
	 for (i=0;i<OAMrm_MAX_TCHFS;i++)
	 {
	      if (trx==rm_PhyChanTchf[i].trxNo &&
	          slot==rm_PhyChanTchf[i].slotNo)
              { 
                if ( rm_PhyChanTchf[i].usable == RM_PHYCHAN_USABLE &&
                     rm_PhyChanTchf[i].amState != unlocked )
                {
                     rm_NoOfUsableTchf++;
                     PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                     //PM_CellMeasurement.nbrOfAvailableTCHs.increment();
                }

                rm_PhyChanTchf[i].amState = unlocked;

                if (rm_PhyChanTchf[i].state == RM_PHYCHAN_FREE)
                { 
                    rm_PhyChanTchf[i].state = RM_PHYCHAN_FREE;
                    rm_PhyChanTchf[i].pMSmCtxt->state = RM_S1RM_RUNNING;
                    rm_PhyChanTchf[i].pASmCtxt->state = RM_S1RM_RUNNING;
                    //onBar = 1;
                    if (rm_PhyChanTchf[i].usable == RM_PHYCHAN_USABLE)
                    {
                        onBar = 1; 
                        newFreeChan = 1;
                    } 
                 }
               
              }
	 }

       if (onBar)
       {
           rm_SendSystemInfo2ToBcchTrx(0,2,OAMrm_RA_CELL_BARRED_STATE);        
           rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);        
           rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
           rm_VcIsBarred = RM_FALSE;
       }

       if (newFreeChan && rm_AllAvailableTchf.allocated)
       {
           rm_AllAvailableTchf.allocated = RM_FALSE;
           PM_CellMeasurement.allAvailableTCHAllocatedTime.stop();

           //time = tickGet() - rm_AllAvailableTchf.startTime;
           //PM_CellMeasurement.allAvailableTCHAllocatedTime.increment(time);
        }

     } else
     {
	  //trx==0&&slot==0; the TS is carrying BCCH and CCCH channels
       
        //Update PCH channel's amState
        rm_PhyChanBCcch[RM_PCH_IDX].amState = unlocked;

        //Update SDCCH4 channels' amState       
	  for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
        {
            if ( rm_PhyChanSdcch4[i].usable == RM_PHYCHAN_USABLE &&
                 rm_PhyChanSdcch4[i].amState != unlocked )
            { 
                 if (++rm_nbrOfAvailableSDCCHs>4)
                     printf("RM INFO@rm_UnlockedAllTrxSlotChans: nbrOfAvailableSDCCHs %d\n",
                             rm_nbrOfAvailableSDCCHs);
                 else
                     PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);

                 //PM_CellMeasurement.nbrOfAvailableSDCCHs.increment();
            }

            rm_PhyChanSdcch4[i].amState = unlocked;

            if (rm_PhyChanSdcch4[i].state == RM_PHYCHAN_FREE)
            {
                rm_PhyChanSdcch4[i].state = RM_PHYCHAN_FREE;
                rm_PhyChanSdcch4[i].pMSmCtxt->state = RM_S1RM_RUNNING;
                rm_PhyChanSdcch4[i].pASmCtxt->state = RM_S1RM_RUNNING;
                if (rm_PhyChanSdcch4[i].usable == RM_PHYCHAN_USABLE)
                    newFreeChan = 1;
            }
  	 }

       if (newFreeChan && rm_AllAvailableSdcch4.allocated)
       {
           rm_AllAvailableSdcch4.allocated = RM_FALSE;
           PM_CellMeasurement.allAvailableSDCCHAllocatedTime.stop();

           //time = tickGet() - rm_AllAvailableSdcch4.startTime;
           //PM_CellMeasurement.allAvailableSDCCHAllocatedTime.increment(time);
       }

    }
}

void rm_ProcBtsAdminStateShuttingDown(void)
{
     u8 i, childInUse,t_childInUse;
  
     PDEBUG__(("INFO@rm_ProcBtsAdminStateShuttingDown()\n"));

     //report BTS is being shutted down now
     rm_SetRmOpState(EC_RM_BTS_SHUTTINGDOWN);

     //Lock all TRXes in existence
     childInUse = 0;
     for (i=0; i<OAMrm_MAX_TRXS; i++ )
     { 
          //Check if ith TRX is existent
          if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
     	    {  
	       t_childInUse = 0;
	       rm_SetTrxAdminState(i,shuttingDown);
	       rm_ProcTrxAdminStateShuttingDown(i,&t_childInUse);
             if (t_childInUse) childInUse = 1;
          }
     }

     //Check if all existing trxes are in locked state or not
     if (!childInUse)
     {   
         //BTS goes to locked state
         rm_SetBtsAdminState(locked);

         //BTS goes to locked state
         rm_SetRmOpState(EC_RM_BTS_LOCKED);
     }
}

void rm_ProcTrxAdminStateShuttingDown(u8 trx, u8 *inUse)
{
     u8 i, j, childInUse, t_childInUse;


     PDEBUG__(("INFO@rm_ProcTrxAdminStateShuttingDown: trx=%d\n", trx));

     *inUse = 0;

     //Check if the requested trx existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) )
     {
	   IDEBUG__(("WARNING@rm_ProcTrxAdminStateShuttingDown: invalid trx=%d\n",
                      trx));
	   return;
     }

     //Report informational alarm
     rm_SetRmOpState(rm_TrxAdminShuttingDownAlarmCode[trx]);

     //Mark all active channels as shuttingDown
     childInUse = 0;
    
     for (i=0;i<OAMrm_SLOTS_PER_TRX;i++)
     {
	    t_childInUse = 0;
	    rm_SetTrxSlotAdminState(trx,i,shuttingDown);
          rm_ProcTrxSlotAdminStateShuttingDown(trx,i,&t_childInUse);
          if (t_childInUse) childInUse = 1;
     } 

     //If there is any active channel on this trx
     if (childInUse) 
     {
         *inUse = 1;
         //if (!trx) 
         //{
         //    if (OAMrm_CHN_AMSTATE(0,0) != shuttingDown)
         //    { 
         //        //Change it to shuttingDown state for consistence
         //        rm_SetTrxSlotAdminState(0,0,shuttingDown);
         //    }
         //}
     }else
     {
         //Trx goes to locked state
         rm_SetTrxAdminState(trx, locked);

         //Report informational alarm
         rm_SetRmOpState(rm_TrxAdminLockedAlarmCode[trx]);

         //Turn off RF transmission
         rm_SendTuneSynthToTrx(trx,0);
      }
}


void rm_ProcTrxSlotAdminStateShuttingDown(u8 trx, u8 slot, u8 *inUse)
{
     u8 childInUse;

     PDEBUG__(("INFO@rm_ProcTrxSlotAdminStateShuttingDown()\n"));

     *inUse = 0;
     
     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
	    slot>=OAMrm_SLOTS_PER_TRX )
     {
	    IDEBUG__(("WARNING@rm_ProcTrxSlotAdminStateShuttingDown:invalid (%d,%d)\n",
		         trx,slot));
	    return;
     }

     //Report informational alarm
     rm_SetRmOpState(rm_TrxSlotAdminShuttingDownAlarmCode[trx][slot]);

     //Mark all trx-slot channels in use as shuttingDown
     childInUse = 0;
     rm_ShuttingDownAllTrxSlotChans(trx, slot, &childInUse);

     //Check if no TCHF or SDCCH4 channels available

     if (RM_SMCTXT_NULL == rm_PreAllocSmCtxt(RM_PHYCHAN_SDCCH4) )
     { 
         if (!rm_AllAvailableSdcch4.allocated)
         {
             rm_AllAvailableSdcch4.allocated = RM_TRUE;
             PM_CellMeasurement.allAvailableSDCCHAllocatedTime.start();

             //rm_AllAvailableSdcch4.startTime = tickGet();
         }
     }

     if (RM_SMCTXT_NULL == rm_PreAllocSmCtxt(RM_PHYCHAN_TCHF_S) )
     { 
         if (!rm_AllAvailableTchf.allocated)
         {
             rm_AllAvailableTchf.allocated = RM_TRUE;
             PM_CellMeasurement.allAvailableTCHAllocatedTime.start();

             //rm_AllAvailableTchf.startTime = tickGet();
         }
     }

     if (childInUse) *inUse = 1;
     else
     {
         //(trx,slot) goes to locked state
         rm_SetTrxSlotAdminState(trx, slot,locked);

         //Report informational alarm
         rm_SetRmOpState(rm_TrxSlotAdminLockedAlarmCode[trx][slot]);

         //Turn off RF transmission if needed
         //if (!trx && !slot)
         //     rm_SendTuneSynthToTrx(trx,0);
     }

     if (!trx && !slot)
     {
         //Bar this cell to drive out all roaming mobiles in this cell
         //<TBD>MIB cellBarred indicator will not reflect this barring
         //if (*inUse)
         //    rm_AmState00 = shuttingDown;
         //else
         //    rm_AmState00 = locked;
         //rm_CellBarred = OAMrm_RA_CELL_BARRED_STATE; //Remember MIB val
         //OAMrm_RA_CELL_BARRED_STATEa = (Boolean)CNI_RIL3_CELL_BARRED_ACCESS;
         rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);        
         rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);        
         rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);
         rm_VcIsBarred = RM_TRUE;        
     }
}


void rm_SetBtsAdminState(AdministrativeState state)
{
     PDEBUG__(("INFO@rm_SetBtsAdminState:state=%d\n",state));
     OAMrm_BTS_AMSTATE = state;
     if (STATUS_OK!=
         oam_setMibIntVar(MODULE_RM,MIB_bts_administrativeState, state) )
     {
         EDEBUG__(("ERROR@rm_SetBtsAdminState: OAM setMibIntVar failed, state=%d\n",
			  state));
     }
}

void rm_SetTrxAdminState(u8 trx, AdministrativeState state)
{
     MibTag tag;

     PDEBUG__(("INFO@rm_SetTrxAdminState:trx=%d, state=%d\n", trx, state));
     
     //Check if the requested trx existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) )
     {
	   IDEBUG__(("WARNING@rm_SetTrxAdminState: invalid trx=%d\n", trx));
	   return;
     }
     
     OAMrm_TRX_AMSTATE(trx) = state;
     if (trx==0) tag = MIB_basebandAdministrativeState_0;
     else 	     tag = MIB_basebandAdministrativeState_1;

     if (STATUS_OK!=oam_setMibIntVar(MODULE_RM, tag, state) )
     {
         EDEBUG__(("ERROR@rm_SetTrxAdminState:OAM setMibIntVar failed(%x,%d)\n",
		        trx, state));
     }
}

void rm_SetTrxSlotAdminState(u8 trx, u8 slot, AdministrativeState state)
{
     MibTag tag;

     PDEBUG__(("INFO@rm_SetTrxSlotAdminState:(%d,%d,%d)\n",trx, slot,state));

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
	   slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_SetTrxSlotAdminState: invalid (%d,%d,%d)\n",
		      trx,slot,state));
	   return;
     }
    
     if (trx==0) tag=MIBT_channelAdministrativeState_0;
     else 	     tag=MIBT_channelAdministrativeState_1;

     OAMrm_CHN_AMSTATEa(trx,slot) = state;
     if (STATUS_OK!=oam_setTblEntryField(MODULE_RM,tag,slot,state))
     {
         EDEBUG__(("ERROR@rm_SetTrxSlotAdminState: OAM setTblEntryField failed (%d,%d,%d)\n",
			  trx, slot, state));
     }
}

void rm_TestPrintAmStateData(void)
{
    int i;

    printf("OAMrm_RA_CELL_BARRED_STATE:%d,%d\n",
            OAMrm_RA_CELL_BARRED_STATE,
            (((T_CNI_RIL3_CELL_BARRED_ACCESS)(rm_OamData.btsOptionsPackage.cellBarred)))
      );

    printf("OAMrm_RA_CELL_BARRED_STATEa:%d,%d\n",
            OAMrm_RA_CELL_BARRED_STATE,
            rm_OamData.btsOptionsPackage.cellBarred
      );

    printf("OAMrm_BTS_AMSTATE(%d,%d),OAMrm_TRX0_AMSTATE(%d,%d),OAMrm_CHN00_AMSTATE(%d,%d)\n",
            OAMrm_BTS_AMSTATE,rm_OamData.btsBasicPackage.bts_administrativeState,
            OAMrm_TRX_AMSTATE(0),rm_OamData.transceiverPackage[0].basebandAdministrativeState,
            OAMrm_CHN_AMSTATE(0,0), ((AdministrativeState)(((ChannelEntry*)
            (rm_OamData.transceiverPackage[0].channelTable))[0].channelAdministrativeState))
          );

    for (i=0;i<OAMrm_MAX_TCHFS;i++)
    {
         printf("%dth TCH(%d,%d): phyState %d, amState %d, MSmState %d, ASmState %d\n",
                 i,
                 rm_PhyChanTchf[i].trxNo,
                 rm_PhyChanTchf[i].slotNo,
                 rm_PhyChanTchf[i].state,
                 rm_PhyChanTchf[i].amState,
                 rm_PhyChanTchf[i].pMSmCtxt->state,
                 rm_PhyChanTchf[i].pASmCtxt->state
               );
    }

    printf("PCH(%d,%d): phyState %d, amState %d\n",
            rm_PhyChanBCcch[RM_PCH_IDX].trxNo,
            rm_PhyChanBCcch[RM_PCH_IDX].slotNo,
            rm_PhyChanBCcch[RM_PCH_IDX].state,
            rm_PhyChanBCcch[RM_PCH_IDX].amState
          );

    for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
    {
         printf("%dth SDCCH4(%d,%d): phyState %d, amState %d, MSmState %d, ASmState %d\n",
                 i,
                 rm_PhyChanSdcch4[i].trxNo,
                 rm_PhyChanSdcch4[i].slotNo,
                 rm_PhyChanSdcch4[i].state,
                 rm_PhyChanSdcch4[i].amState,
                 rm_PhyChanSdcch4[i].pMSmCtxt->state,
                 rm_PhyChanSdcch4[i].pASmCtxt->state
               );
    }
}

void rm_SetTrxSlotOpState(u8 trx, u8 slot, EnableDisable state)
{
     MibTag tag;

     PDEBUG__(("INFO@rm_SetTrxSlotOpState:(%d,%d,%d)\n",trx, slot,state));

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
	   slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_SetTrxSlotOpState: invalid (%d,%d,%d)\n",
		      trx,slot,state));
	   return;
     }
    
     if (trx==0) tag=MIBT_channelOperationalState_0;
     else 	 tag=MIBT_channelOperationalState_1;

     OAMrm_CHN_OPSTATEa(trx,slot) = state;

     if (STATUS_OK!=oam_setTblEntryField(MODULE_RM,tag,slot,state))
     {
         EDEBUG__(("ERROR@rm_SetTrxSlotOpState: OAM setTblEntryField failed (%d,%d,%d)\n",
			  trx, slot, state));
     }
}


//GP2
#endif //#if 0

void rm_ProcGrrMsgs(void)
{
   PDEBUG__(("ENTER@rm_ProcGrrMsgs()....\n"));

   if (!rm_ValidateGrrMsgs()) return;

   switch(rm_pItcRxGrrMsg->message_type)
   {
   case GRR_CB_FEATURE_CONTROL:
        rm_ProcCbSettingChange();
        break;

   case GRR_TRX_SLOT_CHANCOMB:
        rm_ProcTrxSlotCombChange();
        break;

   case GRR_TRX_SLOT_LOCK_REQ:
        rm_ProcTrxSlotLockReq();
        break;

   case GRR_TRX_SLOT_SHUTDOWN_REQ:
        rm_ProcTrxSlotShutdownReq();
        break;
   }
}

void rm_ProcCbSettingChange(void)
{
   PDEBUG__(("ENTER@rm_ProcCbSettingChange: setting(%d)\n", rm_pItcRxGrrMsg->feat_ctrl));

   //CB feature setting changed
   OAMrm_CBCH_OPSTATE = rm_pItcRxGrrMsg->feat_ctrl;
   switch(OAMrm_CBCH_OPSTATE)
   {
   case 0: //Disable CBCH channel
        if (!rm_ChanCbchActive)
            PDEBUG__(("INFO@rm_OamProcResourceConfigData: rel CBCH but it's not ON\n"));

        rm_ChanCbchActive = false; 
        rm_SendRfChanRelease(rm_pCbchChan);
        break;              

   case 1: //Enable CBCH Channel
        if (rm_ChanCbchActive)
            PDEBUG__(("INFO@rm_OamProcResourceConfigData: turn on a already on cbch\n"));

        rm_ChanCbchActive = false; 
        if ( (rm_pCbchChan->usable == RM_PHYCHAN_UNUSABLE) || 
             (OAMrm_CHN_OPSTATE(rm_pCbchChan->trxNo, rm_pCbchChan->slotNo)==opStateDisabled) )
        {
            IDEBUG__(("WARNIG@rm_OamProcResourceConfigData: CBCH usable(%d) trx(%d) slot(%d) amState(%d)\n",
                       rm_pCbchChan->usable, rm_pCbchChan->trxNo, rm_pCbchChan->slotNo, 
                       OAMrm_CHN_AMSTATE(rm_pCbchChan->trxNo, rm_pCbchChan->slotNo) ));
        } else
        {
            //block any SDCCH attempt also activate CBCH 
            rm_pCbchChan->usable = RM_PHYCHAN_USABLE;
            if (rm_pCbchChan->state == RM_PHYCHAN_FREE)
            {
                rm_pCbchChan->usable = RM_PHYCHAN_CBCH_USABLE; 
                if ( (rm_pCbchChan->pMSmCtxt->state != RM_S1RM_RUNNING) ||
                     (rm_pCbchChan->pASmCtxt->state != RM_S1RM_RUNNING)  )
                {
                     IDEBUG__(("WARNING@rm_OamProcResourceConfigData:invalid chan state(%d,%d,%d)\n",
                                rm_pCbchChan,rm_pCbchChan->pMSmCtxt->state,
                                rm_pCbchChan->pASmCtxt->state));
                     rm_pCbchChan->pMSmCtxt->state = RM_S1RM_RUNNING;
                     rm_pCbchChan->pASmCtxt->state = RM_S1RM_RUNNING;
                }
                rm_ActivateCbchChan();
            }
        }
        break;
   default:
        IDEBUG__(("WARNING@rm_OamProcResourceConfigData: invalid cbchOpState=%d\n",
                   rm_pItcRxGrrMsg->feat_ctrl));
        return;
  }
}

bool rm_ValidateGrrMsgs(void)
{
   unsigned char trx, slot, ret;

   ret=true;

   switch(rm_pItcRxGrrMsg->message_type)
   {
   case GRR_CB_FEATURE_CONTROL:
   case GRR_TRX_SLOT_LOCK_REQ:
   case GRR_TRX_SLOT_SHUTDOWN_REQ:
        break;

   case GRR_TRX_SLOT_CHANCOMB:
        if ((!rm_pItcRxGrrMsg->trx)&&(!rm_pItcRxGrrMsg->slot))
        { 
            IDEBUG__(("WARNING@rm_ValidateGrrMsgs: changed to chanComb(%d) for trx0 slot0\n",
                       rm_pItcRxGrrMsg->chan_comb));
            printf("WARNING@rm_ValidateGrrMsgs: changed to chanComb(%d) for trx0 slot0\n",
                       rm_pItcRxGrrMsg->chan_comb);
            ret=false;
        } 
        break;  
        
   default:
        ret=false;
        break;
   }

   return ret;
}

void rm_ProcTrxSlotCombChange(void)
{
   unsigned char trx, slot, comb, i;
   
   trx = rm_pItcRxGrrMsg->trx; slot = rm_pItcRxGrrMsg->slot;
   comb = rm_pItcRxGrrMsg->chan_comb;

   PDEBUG__(("ENTER@rm_ProcTrxSlotCombChange: trx(%d) slot(%d) comb(%d)\n", trx,slot,comb));

   if ( (trx>=OAMrm_MAX_TRXS) || (slot>=8) ) return;

   for (i=0;i<OAMrm_MAX_TCHFS;i++)
   {
        if ( (trx==rm_PhyChanTchf[i].trxNo) &&
	     (slot==rm_PhyChanTchf[i].slotNo)  )
        { 

            switch(comb)
            {
            case OAMrm_SLOT_COMB_1:
                 if ( (rm_PhyChanTchf[i].usable==RM_PHYCHAN_UNUSABLE) &&
                      (OAMrm_CHN_AMSTATE(trx,slot)!=locked)           &&
                      (OAMrm_CHN_OPSTATE(trx,slot)!=opStateDisabled)   )
                 {
                      ++rm_NoOfUsableTchf;
                      PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                 }
                 rm_PhyChanTchf[i].usable = RM_PHYCHAN_USABLE;

                 break;
            
            //GPRS
            case OAMrm_SLOT_COMB_13:
                 if ( (rm_PhyChanTchf[i].usable==RM_PHYCHAN_USABLE)  &&
                      (OAMrm_CHN_AMSTATE(trx,slot)!=locked)          &&
                      (OAMrm_CHN_OPSTATE(trx,slot)!=opStateDisabled)  )
                 {
                      if (--rm_NoOfUsableTchf >= 0)
                          PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                      else
                          rm_NoOfUsableTchf = 0;
                 }
                 rm_PhyChanTchf[i].usable = RM_PHYCHAN_UNUSABLE;
            
                 break;            
            
            default: 
                 IDEBUG__(("WARNING@rm_ProcTrxSlotCombChange: unexpected comb(%d) for trx(%d) slot(%d)\n",
                            comb, trx, slot));
                 break;
            }
            break;
        }
   }

   if (i>=OAMrm_MAX_TCHFS)
   {
      IDEBUG__(("WARNING@rm_ProcTrxSlotCombChange: trx(%d) slot(%d) not in tchf chan table\n",
                 trx, slot));
   }

}

void rm_ProcTrxSlotLockReq(void)
{
   u8 trx, slot, i;

   PDEBUG__(("ENTER@rm_ProcTrxSlotLockReq()...\n"));

   PDEBUG__(("INFO@rm_ProcTrxSlotAdminStateLocked:(%d,%d)\n",trx, slot));

   trx = rm_pItcRxGrrMsg->trx; slot = rm_pItcRxGrrMsg->slot;

   //Check if (trx,slot) existent
   if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
           (slot>=OAMrm_SLOTS_PER_TRX) )
   {
	 IDEBUG__(("WARNING@rm_ProcTrxSlotAdminStateLocked: invalid (%d,%d)\n",
		    trx,slot));
	 return;
   }

   //Lock channs
   if (trx||slot)
   {
       //Search for the time slot
       for (i=0;i<OAMrm_MAX_TCHFS;i++)
       {
	    if ( (trx==rm_PhyChanTchf[i].trxNo)   &&
	         (slot==rm_PhyChanTchf[i].slotNo)  )
          { 
                if ( (rm_PhyChanTchf[i].usable==RM_PHYCHAN_USABLE) &&
                     (rm_PhyChanTchf[i].amState!=locked)            )
                {
                    if (--rm_NoOfUsableTchf<0) rm_NoOfUsableTchf = 0;
                    PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                } 

	          rm_PhyChanTchf[i].amState = locked;
	          rm_LockedOneChan(&rm_PhyChanTchf[i]);	
          }
	}
   } else
   {
	  //trx=slot=0, beacon time slot
     
        //Update PCH channel's amState
        rm_PhyChanBCcch[RM_PCH_IDX].amState = locked;

        //Update SDCCH4 channels' amState       
        for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
        {
             if ( (rm_PhyChanSdcch4[i].usable==RM_PHYCHAN_USABLE) &&
                  (rm_PhyChanSdcch4[i].amState!=locked)            )
             {
                  if (--rm_nbrOfAvailableSDCCHs < 0) rm_nbrOfAvailableSDCCHs = 0;
                  PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);
             } 
             rm_PhyChanSdcch4[i].amState = locked;
             rm_LockedOneChan(&rm_PhyChanSdcch4[i]);	
        }
   }
}

void rm_ProcTrxSlotShutdownReq(void)
{
     u8 childInUse,trx,slot;

     PDEBUG__(("INFO@rm_ProcTrxSlotAdminStateShuttingDown()\n"));

     trx = rm_pItcRxGrrMsg->trx;
     slot = rm_pItcRxGrrMsg->slot;
      
     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
	    slot>=OAMrm_SLOTS_PER_TRX )
     {
	    IDEBUG__(("WARNING@rm_ProcTrxSlotAdminStateShuttingDown:invalid (%d,%d)\n",
		         trx,slot));
	    return;
     }

     //Mark all trx-slot channels in use as shuttingDown
     childInUse = 0;
     rm_ShuttingDownAllTrxSlotChans(trx, slot, &childInUse);

     if (!childInUse)
          grr_SendTrxSlotShutdownAck(MODULE_RM, grr_MsgQId, trx, slot);
}

void rm_LockedAllTrxSlotChans(u8 trx, u8 slot)
{
     u8 i;

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
	   slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_LockedAllTrxSlotChans: invalid (%d,%d)\n",
		      trx,slot));
	   return;
     }

 
     if (trx || slot)
     {
	 //scan through tchf table to excerise the adminState change
	 for (i=0;i<OAMrm_MAX_TCHFS;i++)
	 {
	      if (trx==rm_PhyChanTchf[i].trxNo &&
	          slot==rm_PhyChanTchf[i].slotNo)
              { 
                if ( rm_PhyChanTchf[i].usable  == RM_PHYCHAN_USABLE &&
                     rm_PhyChanTchf[i].amState != locked )
                {
                     if (--rm_NoOfUsableTchf >= 0)
                         PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                     else
                         rm_NoOfUsableTchf = 0;
                } 

	          rm_PhyChanTchf[i].amState = locked;
	          rm_LockedOneChan(&rm_PhyChanTchf[i]);	
              }
	 }

     } else
     {
	 //trx==0&&slot==0; the TS is carrying BCCH and CCCH channels
       
       //Update PCH channel's amState
       rm_PhyChanBCcch[RM_PCH_IDX].amState = locked;

       //Update SDCCH4 channels' amState       
	 for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
       {
            if ( rm_PhyChanSdcch4[i].usable  == RM_PHYCHAN_USABLE &&
                 rm_PhyChanSdcch4[i].amState != locked )
            {
                 if (--rm_nbrOfAvailableSDCCHs >= 0) 
                     PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);
                 else
                     rm_nbrOfAvailableSDCCHs = 0;
            } 

            rm_PhyChanSdcch4[i].amState = locked;
	      rm_LockedOneChan(&rm_PhyChanSdcch4[i]);	
       }
     }
}

void rm_LockedOneChan(rm_PhyChanDesc_t *pChan)
{
     rm_SmCtxt_t  *pMSmCtxt, *pASmCtxt;

     pMSmCtxt = pChan->pMSmCtxt; 
     pASmCtxt = pChan->pASmCtxt;

     //Deactivate this channel if needed 
     if (pMSmCtxt->state != RM_S1RM_RUNNING)
     {
         rm_SendDlRelReq(pMSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);
         pMSmCtxt->relCause = CNI_RIL3_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED;
         rm_SendRrRelInd(RM_LOCKED,pMSmCtxt->mEntryId);

         //SMS-SAPI3-Begin

         //if (pMSmCtxt->sapi != RM_SAPI0)
         //    rm_SendDlRelReq(pMSmCtxt->mOId, pMSmCtxt->sapi,
	   //		           RM_LOCAL_RELEASE);

         rm_SendDlRelReq(pMSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
         rm_SendDlRelReq(pMSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);

         //SMS-SAPI3-End
     }

     if (pASmCtxt->state != RM_S1RM_RUNNING)
     {
         //SMS-SAPI3-Begin
         rm_SendDlRelReq(pASmCtxt->aOId,RM_SAPI3,RM_LOCAL_RELEASE);

         //if (pASmCtxt->sapi != RM_SAPI0)
         //    rm_SendDlRelReq(pASmCtxt->aOId, pASmCtxt->sapi,
	   //		           RM_LOCAL_RELEASE);
	   //rm_SendRrRelInd(RM_LOCKED, pASmCtxt->aEntryId);
         //SMS-SAPI3-End
     }

     pMSmCtxt->state = RM_S1RM_RUNNING;
     pASmCtxt->state = RM_S1RM_RUNNING;
     pChan->state = RM_PHYCHAN_FREE;
     rm_SendRfChanRelease(pChan);
}

void rm_ShuttingDownAllTrxSlotChans(u8 trx, u8 slot, u8 *inUse)
{
     u8 i;

     PDEBUG__(("INFO@rm_ShuttingDownAllTrxSlotChans:(%d,%d,%d)\n",
		    trx,slot,*inUse));

     *inUse = 0;

     //Check if (trx,slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
          slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_ShuttingDownAllTrxSlotChans: invalid (%d,%d)\n",
		      trx,slot));
	   return;
     }

     //Scan through in terms of channel combination right now only 2
     if (trx || slot)
     {
	   //scan through tchf table to excerise the adminState change
	   for (i=0;i<OAMrm_MAX_TCHFS;i++)
	   {
	        if (trx==rm_PhyChanTchf[i].trxNo &&
	            slot==rm_PhyChanTchf[i].slotNo)
              {
		      if (rm_PhyChanTchf[i].state != RM_PHYCHAN_FREE)
                  {
		          //This time slot is now in use for a call
	                rm_PhyChanTchf[i].amState = shuttingDown;
	                (*inUse)++;
		      } else
		      {
                      if ( (rm_PhyChanTchf[i].usable==RM_PHYCHAN_USABLE) &&
                           (OAMrm_CHN_AMSTATE(trx,slot)!=locked)         &&
                           (OAMrm_CHN_OPSTATE(trx,slot)!=opStateDisabled) )

                      {
                           if (--rm_NoOfUsableTchf < 0) rm_NoOfUsableTchf = 0;
                           PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                      } 

                      rm_PhyChanTchf[i].pMSmCtxt->state = RM_S1RM_RUNNING;
                      rm_PhyChanTchf[i].pASmCtxt->state = RM_S1RM_RUNNING;
     	                rm_PhyChanTchf[i].amState = locked;
		      }
	        } 
	   }
         
         //Sanitory check
         if ( (*inUse) > 1)
         {
               IDEBUG__(("WARNING@ rm_ShuttingDownAllTrxSlotChans:(%d,%d,used %d)\n",
                          trx, slot, *inUse));
         }
     } else
     {
	   //trx=slot=0, beacon time slot
  
         //Update PCH channel's amState
         rm_PhyChanBCcch[RM_PCH_IDX].amState = locked; //block paging

         //Update SDCCH4 channels' amState       
	   for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
         {
 	        if (rm_PhyChanSdcch4[i].state != RM_PHYCHAN_FREE)
              {
		      //This time slot is currently use for call
                  rm_PhyChanSdcch4[i].amState = shuttingDown;
	            (*inUse)++;
	        } else
	        {
                  rm_PhyChanSdcch4[i].pMSmCtxt->state = RM_S1RM_RUNNING;
                  rm_PhyChanSdcch4[i].pASmCtxt->state = RM_S1RM_RUNNING;
                  rm_PhyChanSdcch4[i].amState = locked;
     
                  if (--rm_nbrOfAvailableSDCCHs<0) rm_nbrOfAvailableSDCCHs=0;
                  PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);
	        }
     	   }
     }
}


#endif //__RM_OAMREGTRAPVARS_CPP__


