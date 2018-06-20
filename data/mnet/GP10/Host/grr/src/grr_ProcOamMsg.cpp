/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROCOAMMSG_CPP__
#define __GRR_PROCOAMMSG_CPP__

#include "grr\grr_head.h"

void grr_ProcOamMsg(void)
{
   DBG_FUNC("grr_ProcOamMsg", GRR_LAYER_OAM);
   DBG_ENTER();

   //Process in terms of message type
   switch (grr_pItcRxOamMsg->msgType)
   {
   case MT_TRAP:
        //Got a trapped param change notification
        //

        //Block unauthorized change attempts
        if (!grr_ProcOamValidateMsg()) return;

        switch (MIB_TBL(grr_pItcRxOamMsg->mibTag))
        {
	  case MTBL_btsBasicPackage:
	       grr_ProcOamBtsBasicPackage();
	       break;
          case MTBL_btsOptionsPackage:
	       grr_ProcOamBtsOptionsPackage();
	       break;
	  case MTBL_firstTransceiverPackage:
	       grr_ProcOamTransceiverPackage(0);
	       break;
	  case MTBL_secondTransceiverPackage:
	       grr_ProcOamTransceiverPackage(1);
	       break;
	  case MTBL_firstRadioCarrierPackage:
	       grr_ProcOamRadioCarrierPackage(0, grr_pItcRxOamMsg->mibTag);
	       break;
	  case MTBL_secondRadioCarrierPackage:
	       grr_ProcOamRadioCarrierPackage(1, grr_pItcRxOamMsg->mibTag);
	       break;
          case MTBL_t31xx:
	       grr_ProcOamT31xxPackage();
	       break;
          case MTBL_adjacentCellPackage:
	       grr_ProcOamAdjacentCellPackage();
	       break;
          case MTBL_handoverControlPackage:
	  case MTBL_hoAveragingAdjCellParam:
	  case MTBL_hoAveragingDistParam:
	  case MTBL_hoAveragingLevParam:
	  case MTBL_hoAveragingQualParam:
	  case MTBL_hoThresholdDistParam:
	  case MTBL_hoThresholdInterfaceParam:
	  case MTBL_hoThresholdLevParam:
	  case MTBL_hoThresholdQualParam:
	  case MTBL_interferenceAveragingParam:
	       grr_ProcOamHandoverControlPackage();
	       break;
          case MTBL_powerControlPackage:
	  case MTBL_pcAveragingLev:
	  case MTBL_pcAveragingQual:
	  case MTBL_pcLowerThresholdLevParam:
	  case MTBL_pcLowerThresholdQualParam:
	  case MTBL_pcUpperThresholdLevParam:
	  case MTBL_pcUpperThresholdQualParam:
	       grr_ProcOamPowerControlPackage();
	       break;
          case MTBL_resourceConfigData:
	       grr_ProcOamResourceConfigData();
	       break;
          default:
	       //Received unknown OAM mib table MIB_TBL(tag)
      	 DBG_WARNING("grr_ProcOamMsg: unknown OAM table(%d)\n",MIB_TBL(grr_pItcRxOamMsg->mibTag));
	       break;
        }
        break;
		
   default:
	  //Received unrecognized message type
	  DBG_WARNING("grr_ProcOamMsg: unknown OAM msg type(%d)\n", grr_pItcRxOamMsg->msgType);
	  break;
   }        

   DBG_LEAVE();
}

void grr_ProcOamBtsBasicPackage(void)
{
   unsigned char i;

   DBG_FUNC("grr_ProcOamBtsBasicPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamBtsBasicPackage: changed mib param tag(%x)\n",
              grr_pItcRxOamMsg->mibTag);

   //Retrieve BtsBasicPackage
   OAMgrr_BTS_OPSTATEold = OAMgrr_BTS_OPSTATE;
   OAMgrr_BTS_AMSTATEold = OAMgrr_BTS_AMSTATE;
   grr_GetOamBtsBasicPackage();

   //Check if BTS operational state is changed or not
   if (grr_pItcRxOamMsg->mibTag==MIB_bts_operationalState)
   {
       grr_ProcOamBtsOpStateChange();
       return;
   }

   //Check if got BTS administrative state changed notifica
   if (grr_pItcRxOamMsg->mibTag==MIB_bts_administrativeState)
   {
       grr_ProcOamBtsAmStateChange();
       return;
   }

   //Blindly update DSPs with new parameters
   i = 0;
   while (i<OAMgrr_MAX_TRXS)
   { 
      //Update both DSPs and MS
      grr_PassOamMsSystemInformation2(i,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation3(i,GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation4(i,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation5(i,GRR_L1SI_TYPE_5,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation6(i,GRR_L1SI_TYPE_6,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation13(i,GRR_L1SI_TYPE_13,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamDspBtsPackage(i);
      grr_SendDspTrxConfig(i);

      i++;
   }
  
   DBG_LEAVE();
}

void grr_ProcOamBtsOpStateChange(void)
{
     int i;
	 JC_STATUS  jcRet;

     DBG_FUNC("grr_ProcOamBtsOpStateChange", GRR_LAYER_PROC);
     DBG_ENTER();

     DBG_TRACE("TRACE@grr_ProcOamBtsOpStateChange: new btsOpState(%d) old btsOpState(%d)\n",
                grr_pItcRxOamMsg->val.varVal, OAMgrr_BTS_OPSTATEold);

     switch(grr_pItcRxOamMsg->val.varVal)
     {
     case opStateEnabled:
          //Turn on radio
          if (!grr_IsBtsAmTypeBlocked()) grr_RadioCtrl(GRR_ON);

		  DBG_TRACE("grr_ProcOamBtsOpStateChange: inform RLC_MAC of bts opState enabled\n");

		  if (JC_OK != (jcRet=RlcMacGPOpState(opStateEnabled)))
  		      DBG_ERROR("grr_ProcOamBtsOpStateChange: RlcMacGPOpState() err(%d)\n", jcRet);
          
          break;

     case opStateDisabled:
          //Turn off radio
          grr_RadioCtrl(GRR_OFF);

		  DBG_TRACE("grr_ProcOamBtsOpStateChange: inform RLC_MAC of bts opState disabled\n");

 		  if (JC_OK != (jcRet=RlcMacGPOpState(opStateDisabled)))
  		      DBG_ERROR("grr_ProcOamBtsOpStateChange: RlcMacGPOpState() err(%d)\n", jcRet);

          break;

     default:
          DBG_ERROR("grr_ProcOamBtsOpStateChange: invalid new btsOpState Value(%d) old btsOpState\n",
	               grr_pItcRxOamMsg->val.varVal, OAMgrr_BTS_OPSTATEold);
          printf("grr_ProcOamBtsOpStateChange: invalid new btsOpState Value(%d) old btsOpState\n",
	               grr_pItcRxOamMsg->val.varVal, OAMgrr_BTS_OPSTATEold);
          assert(0);
          break;
     }

     OAMgrr_BTS_OPSTATEa = (EnableDisable_t) grr_pItcRxOamMsg->val.varVal;

     DBG_LEAVE();
}

void grr_ProcOamBtsAmStateChange(void)
{
   DBG_FUNC("grr_ProcOamBtsAmStateChange", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamBtsAmStateChange: get new btsAmState(%d) oldBtsAmState(%d)\n",
              grr_pItcRxOamMsg->val.varVal, OAMgrr_BTS_AMSTATEold);

   //In terms of amState change notif
   switch(grr_pItcRxOamMsg->val.varVal)
   {
   case locked:
        grr_ProcOamBtsAmStateLocked();
	break;

   case unlocked:
        grr_ProcOamBtsAmStateUnlocked();
        break;

   case shuttingDown:
	  grr_ProcOamBtsAmStateShuttingdown();
        break;

   default:
        DBG_ERROR("grr_ProcOamBtsAmStateChange: invalid new btsAmState(%d) old btsAmState(%d)\n",
                   grr_pItcRxOamMsg->val.varVal, OAMgrr_BTS_AMSTATEold);
        printf("grr_ProcOamBtsAmStateChange: invalid new btsAmState(%d) old btsAmState(%d)\n",
                   grr_pItcRxOamMsg->val.varVal, OAMgrr_BTS_AMSTATEold);
        assert(0);
        break;
   }

   DBG_LEAVE();
}

void grr_ProcOamBtsAmStateLocked(void)
{
   unsigned char i;
 
   DBG_FUNC("grr_ProcOamBtsAmStateLocked", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamBtsAmStateLocked: btsAmState goes locked\n");

   //Report informational alarm
   grr_SubmitAlarm(EC_GRR_BTS_LOCKED);

   //Lock all TRXes in existence
   for (i=0; i<OAMgrr_MAX_TRXS; i++ )
   { 
      //Check if ith TRX is existent
      grr_SetOamTrxAmState(i,locked);
      grr_ProcOamTrxAmStateLocked(i);
   }
   
   DBG_LEAVE();
}

void grr_ProcOamTrxAmStateLocked(unsigned char trx)
{
   unsigned char i;
 
   DBG_FUNC("grr_ProcOamTrxAmStateLocked", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrxAmStateLocked: trx(%d) amState goes locked\n", trx);

   //Check if trx existent
   if (trx>=OAMgrr_MAX_TRXS)
   {
	 DBG_WARNING("grr_ProcOamTrxAmStateLocked: invalid trx(%d)\n",trx);
	 return;
   }

   //Report informational alarm
   grr_SubmitAlarm(grr_TrxAmStateLockedAlarmCode[trx]);

   //Turn off radio trans
   grr_TuneTrxSynth(trx,0);

   //lock all TSs
   for (i=0;i<8;i++)
   {
       grr_SetOamTrxSlotAmState(trx,i,locked);
       grr_ProcOamTrxSlotAmStateLocked(trx,i);
   } 

   DBG_LEAVE();
}

void grr_ProcOamTrxSlotAmStateLocked(unsigned char trx, unsigned char slot)
{
   JC_STATUS jcRet;

   DBG_FUNC("grr_ProcOamTrxSlotAmStateLocked", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrxSlotAmStateLocked: trx(%d) Ts(%d) amState goes locked\n", trx,slot);

   //Check if trx & slot existent
   if ( (trx>=OAMgrr_MAX_TRXS)||(slot>=8) )
   {
	 DBG_WARNING("grr_ProcOamTrxSlotAmStateLocked: invalid trx(%d) slot(%d)\n",trx,slot);
	 return;
   }

   //Report informational alarm
   grr_SubmitAlarm(grr_TrxSlotAmStateLockedAlarmCode[trx][slot]);

   //Send lock request to RLC, RRM
   //grr_SendTrxSlotLockReq(MODULE_GRR, rlcMsgQId, trx, slot);
   if (JC_OK != (jcRet=RlcMacTSAdminState(trx,slot,locked)))
   {
       DBG_ERROR("grr_ProcOamTrxSlotAmStateLocked: RlcMacTSAdminState error (state=%d), ret=%d)(%d)\n",
		         locked,jcRet);
       assert(0);
   }
   grr_SendTrxSlotLockReq(MODULE_GRR, grr_MsgQId, trx, slot);

   DBG_LEAVE();
}

void grr_ProcOamBtsAmStateUnlocked(void)
{
   unsigned char i;

   DBG_FUNC("grr_ProcOamBtsAmStateUnlocked", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamBtsAmStateUnlocked: btsAmState goes unlocked\n");

   //Report informational alarm
   grr_SubmitAlarm(EC_GRR_BTS_UNLOCKED);

   //Unlock all TRXes in existence
   for (i=0; i<OAMgrr_MAX_TRXS; i++ )
   { 
      //Check if ith TRX is existent
      grr_SetOamTrxAmState(i,unlocked);
      grr_ProcOamTrxAmStateUnlocked(i);
   }
   
   DBG_LEAVE();
}

void grr_ProcOamTrxAmStateUnlocked(unsigned char trx)
{
   unsigned char i;
 
   DBG_FUNC("grr_ProcOamTrxAmStateUnlocked", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrxAmStateUnlocked: trx(%d) amState goes unlocked\n", trx);

   //Check if trx existent
   if (trx>=OAMgrr_MAX_TRXS)
   {
	 DBG_WARNING("grr_ProcOamTrxAmStateUnlocked: invalid trx(%d)\n",trx);
	 return;
   }

   //Report informational alarm
   grr_SubmitAlarm(grr_TrxAmStateUnlockedAlarmCode[trx]);

   //unlock all TSs
   for (i=0;i<8;i++)
   {
       grr_SetOamTrxSlotAmState(trx,i,unlocked);
       grr_ProcOamTrxSlotAmStateUnlocked(trx,i);
   } 

   //Turn on radio transmission if should be
   if (OAMgrr_BTS_OPSTATE==opStateEnabled) grr_TuneTrxSynth(trx,1);

   DBG_LEAVE();
}


void grr_ProcOamTrxSlotAmStateUnlocked(unsigned char trx, unsigned char slot)
{
   JC_STATUS jcRet;

   DBG_FUNC("grr_ProcOamTrxSlotAmStateUnlocked", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrxSlotAmStateUnlocked: trx(%d) Ts(%d) amState goes unlocked\n", trx,slot);

   //Check if trx & slot existent
   if ( (trx>=OAMgrr_MAX_TRXS)||(slot>=8) )
   {
	 DBG_WARNING("grr_ProcOamTrxSlotAmStateUnlocked: invalid trx(%d) or slot(%d)\n",trx,slot);
	 return;
   }

   //Report informational alarm
   grr_SubmitAlarm(grr_TrxSlotAmStateUnlockedAlarmCode[trx][slot]);

   //Send unlock request to RLC, RRM
   //grr_SendTrxSlotUnlockReq(MODULE_GRR, rlcMsgQId, trx, slot);
   //grr_SendTrxSlotUnlockReq(MODULE_GRR, rm_MsgQId, trx, slot);
   if (JC_OK != (jcRet=RlcMacTSAdminState(trx,slot,unlocked)))
   {
       DBG_ERROR("grr_ProcOamTrxSlotAmStateUnlocked: RlcMacTSAdminState error (state=%d, ret=%d)\n",
		          unlocked,jcRet);
       assert(0);
   }


   DBG_LEAVE();
}

void grr_ProcOamBtsAmStateShuttingdown(void)
{
   unsigned char i;
 
   DBG_FUNC("grr_ProcOamBtsAmStateShuttingdown", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamBtsAmStateShuttingdown: btsAmState being shuttingdown\n");

   //Report informational alarm
   grr_SubmitAlarm(EC_GRR_BTS_SHUTTINGDOWN);

   //Shutdown all TRXes in existence
   for (i=0; i<OAMgrr_MAX_TRXS; i++ )
   { 
      //Check if ith TRX is existent
      grr_SetOamTrxAmState(i,shuttingDown);
      grr_ProcOamTrxAmStateShuttingdown(i);
   }
   
   DBG_LEAVE();     
}

void grr_ProcOamTrxAmStateShuttingdown(unsigned char trx)
{
   unsigned char i;
 
   DBG_FUNC("grr_ProcOamTrxAmStateShuttingdown", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrxAmStateShuttingdown: trx(%d) amState being shutdown\n", trx);

   //Check if trx existent
   if (trx>=OAMgrr_MAX_TRXS)
   {
	 DBG_WARNING("grr_ProcOamTrxAmStateShuttingdown: invalid trx(%d)\n",trx);
	 return;
   }

   //Report informational alarm
   grr_SubmitAlarm(grr_TrxAmStateShuttingdownAlarmCode[trx]);

   //Shutdown all TSs
   for (i=0;i<8;i++)
   {
       grr_SetOamTrxSlotAmState(trx,i,shuttingDown);
       grr_ProcOamTrxSlotAmStateShuttingdown(trx,i);
   } 

   DBG_LEAVE();
}

void grr_ProcOamTrxSlotAmStateShuttingdown(unsigned char trx, unsigned char slot)
{
   JC_STATUS jcRet;

   DBG_FUNC("grr_ProcOamTrxSlotAmStateShuttingdown", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrxSlotAmStateShuttingdown: trx(%d) slot(%d) amState being shutdown\n", trx,slot);

   //Check if trx & slot existent
   if ( (trx>=OAMgrr_MAX_TRXS)||(slot>=8) )
   {
	 DBG_WARNING("grr_ProcOamTrxSlotAmStateShuttingdown: invalid trx(%d) slot(%d)\n",trx,slot);
	 return;
   }

   //Report informational alarm
   grr_SubmitAlarm(grr_TrxSlotAmStateShuttingdownAlarmCode[trx][slot]);

   //Send shutdown request to RLC, RRM
   //grr_SendTrxSlotShutdownReq(MODULE_GRR, rlcMsgQId, trx, slot);
   if (JC_OK != (jcRet=RlcMacTSAdminState(trx,slot,shuttingDown)))
   {
       DBG_ERROR("grr_ProcOamTrxSlotAmStateLocked: RlcMacTSAdminState error (state=%d, ret=%d)\n",
		          shuttingDown,jcRet);
       assert(0);
   }

   grr_SendTrxSlotShutdownReq(MODULE_GRR, rm_MsgQId, trx, slot);

   DBG_LEAVE();
}

void grr_ProcOamBtsOptionsPackage(void)
{
   unsigned char i;
	
   DBG_FUNC("grr_ProcOamBtsOptionsPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamBtsOptionsPackage: changed mib param tag(%x)\n",
              grr_pItcRxOamMsg->mibTag);

   //Retrieve BtsOptionPackage
   grr_GetOamBtsOptionPackage();
	
   //Blindly update DSPs with new parameters
   i = 0;
   while (i<OAMgrr_MAX_TRXS)
   { 
      //Update both DSPs and MS
      grr_PassOamMsSystemInformation2(i,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation3(i,GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation4(i,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation5(i,GRR_L1SI_TYPE_5,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation6(i,GRR_L1SI_TYPE_6,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamMsSystemInformation13(i,GRR_L1SI_TYPE_13,OAMgrr_RA_CELL_BARRED_STATE);
      grr_PassOamDspBtsPackage(i);
      i++;
   }
	
   DBG_LEAVE();
}

void grr_ProcOamTransceiverPackage(unsigned char trx)
{
   unsigned char i, slot;
   JC_STATUS	 jcRet;

   DBG_FUNC("grr_ProcOamTransceiverPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTransceiverPackage: trx(%d) changed mib param tag(%x)\n",
              trx, grr_pItcRxOamMsg->mibTag);

   if (trx>=OAMgrr_MAX_TRXS)
   {
       DBG_WARNING("grr_ProcOamTransceiverPackage: changed mib param tag(%d) but invalid trx(%d)\n",
                    grr_pItcRxOamMsg->mibTag, trx);
       return;
   }

   //Retrieve changed bts transceiver package
   if (trx == 0)
   {
      OAMgrr_TRX0_AMSTATEold = OAMgrr_TRX_AMSTATE(0);
      OAMgrr_TRX0SLOT0_AMSTATEold = OAMgrr_TRX_SLOT_AMSTATE(0,0);
      grr_GetOamBtsFirstTrxPackage();
   }  else
      grr_GetOamBtsSecondTrxPackage();
      
   switch(grr_pItcRxOamMsg->mibTag)
   {
   case MIB_basebandAdministrativeState_0:
        //Notify of Trx-0 amState change
        //
        grr_ProcOamTrx0AmStateChange();
        break;

   case MIB_basebandAdministrativeState_1:
	  //Notify of other trxs adminState change
        //
        switch(grr_pItcRxOamMsg->val.varVal)
        {
        case locked:
             grr_ProcOamTrxAmStateLocked(1);
             break;

        case unlocked:
 	       grr_ProcOamTrxAmStateUnlocked(1);
		 break;

        case shuttingDown:
             grr_ProcOamTrxAmStateShuttingdown(1);
             break;

        default:
             DBG_WARNING("grr_ProcOamTransceiverPackage:trx1,invalid amStateValue:%d\n",
				  grr_pItcRxOamMsg->mibTag);
             break;
	  }
        break;

   case MIBT_channelAdministrativeState_0:
   case MIBT_channelAdministrativeState_1:
        //Got trx slot amState change notif
        //
        slot  = grr_pItcRxOamMsg->val.varTbl.idx;

        if ((trx>=OAMgrr_MAX_TRXS) || (slot>=8))
        {
           DBG_WARNING("grr_ProcOamTransceiverPackage: amState changed for invalid trx(%d) slot(%d)\n",
                        trx, slot);
           return;
        }
 
        if ( (trx==0) && (slot==0) ) grr_ProcOamTrx0Slot0AmStateChange();
        else 
        {  
           switch(grr_pItcRxOamMsg->val.varTbl.value)
           {
           case locked:
                grr_ProcOamTrxSlotAmStateLocked(trx, slot);
                break;
           case unlocked:
                grr_ProcOamTrxSlotAmStateUnlocked(trx, slot);
	          break;
           case shuttingDown:
                grr_ProcOamTrxSlotAmStateShuttingdown(trx, slot);
                break;
           default:
                DBG_WARNING("grr_ProcOamTransceiverPackage:trx(%d) slot(%d) invalid amState(%d)\n",
                             trx,slot,grr_pItcRxOamMsg->val.varTbl.value);
                break;
	     }
        }
        break;

   case MIBT_channelCombination_0:
   case MIBT_channelCombination_1:
        //Change trx slot channel combination
        //
        unsigned char slot;
 
        slot = grr_pItcRxOamMsg->val.varTbl.idx;

        //trx-slot is change qualified earlier, thus going ahead to realize the change
        OAMgrr_CHN_OPSTATEa(trx,slot) = opStateDisabled;
        grr_SetOamTrxSlotOpState(trx,slot,opStateDisabled);

  	    if (JC_OK != (jcRet=RlcMacTSOpState(trx, slot, opStateDisabled)))
		{
			DBG_ERROR("grr_ProcOamTransceiverPackage: RlcMacTSOpState error (trx=%d,slot=%d,state=%d,ret=%d)\n",
				        trx,slot,opStateDisabled,jcRet);
		}

        grr_SendDspSlotActivate(trx, slot);
     
        break;
        
    default:
        //Handle other trapped notification here
        DBG_WARNING("grr_ProcOamTransceiverPackage: change for mib param tag(%d) unsupported\n",
                     grr_pItcRxOamMsg->val.varVal);

        break;
    }

    DBG_LEAVE();
}

void grr_ProcOamTrx0AmStateChange(void)
{
   DBG_FUNC("grr_ProcOamTrx0AmStateChange", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrx0AmStateChange: new amState(%d) old amState(%d)\n",
              grr_pItcRxOamMsg->val.varVal, OAMgrr_TRX0_AMSTATEold);

   if ( (grr_pItcRxOamMsg->val.varVal==locked)      ||
        (grr_pItcRxOamMsg->val.varVal==unlocked)    ||
        (grr_pItcRxOamMsg->val.varVal==shuttingDown) )
   {
         grr_SetOamBtsAmState((AdministrativeState_t)(grr_pItcRxOamMsg->val.varVal));
         grr_ProcOamBtsAmStateChange();
   } else
   {
         DBG_ERROR("grr_ProcOamTrx0AmStateChange: invalid new amState(%d) old amState(%d)\n",
                      grr_pItcRxOamMsg->val.varVal,OAMgrr_TRX0_AMSTATEold); 
         printf("grr_ProcOamTrx0AmStateChange: invalid new amState(%d) old amState(%d)\n",
                      grr_pItcRxOamMsg->val.varVal,OAMgrr_TRX0_AMSTATEold); 
         assert(0);
   }

   OAMgrr_TRX0_AMSTATEold = grr_pItcRxOamMsg->val.varVal;

   DBG_LEAVE();
}

void grr_ProcOamTrx0Slot0AmStateChange(void)
{
   DBG_FUNC("grr_ProcOamTrx0Slot0AmStateChange", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamTrx0Slot0AmStateChange: new amState(%d) old amState(%d)\n",
              grr_pItcRxOamMsg->val.varTbl.value, OAMgrr_TRX0SLOT0_AMSTATEold);

   if ( (grr_pItcRxOamMsg->val.varTbl.value==locked)      ||
        (grr_pItcRxOamMsg->val.varTbl.value==unlocked)    ||
        (grr_pItcRxOamMsg->val.varTbl.value==shuttingDown) )
   {
         grr_SetOamBtsAmState((AdministrativeState_t)(grr_pItcRxOamMsg->val.varTbl.value));
   } else
   {
         DBG_WARNING("grr_ProcOamTrx0Slot0AmStateChange: invalid new amState(%d), old amState(%d)\n",
                      grr_pItcRxOamMsg->val.varTbl.value,OAMgrr_TRX0_AMSTATEold);
         printf("grr_ProcOamTrx0Slot0AmStateChange: invalid new amState(%d), old amState(%d)\n",
                      grr_pItcRxOamMsg->val.varTbl.value,OAMgrr_TRX0_AMSTATEold);
         assert(0);
   } 

   switch(grr_pItcRxOamMsg->val.varTbl.value)
   {
   case locked:
        grr_ProcOamBtsAmStateLocked();
        break;

   case unlocked:
        grr_ProcOamBtsAmStateUnlocked();
        break;

   case shuttingDown:
        grr_ProcOamBtsAmStateShuttingdown();
        break;
   }

   OAMgrr_TRX0SLOT0_AMSTATEold = grr_pItcRxOamMsg->val.varTbl.value;

   DBG_LEAVE();
}

void grr_ProcOamRadioCarrierPackage(unsigned char rc, MibTag tag)
{
   unsigned char pwr,trx;

   DBG_FUNC("grr_ProcOamRadioCarrierPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamRadioCarrierPackage: radio carrier(%d), changed mib param tag(%d)\n",
              rc, tag);

   if(rc>=OAMgrr_MAX_TRXS)
   {
      DBG_WARNING("ProcOamRadioCarrierPackage: received invalid rc(%d)\n", rc);
      return;
   }
	
   //Retrieve rc package
   if (rc==0) grr_GetOamBtsFirstRadioCarrierPackage();
   else       grr_GetOamBtsSecondRadioCarrierPackage();

   if (OAMgrr_TRX_RC(0) == rc) trx = 0;
   else				 trx = 1;

   switch (tag)
   {
   case MIBT_carrierFrequency_0:
   case MIBT_carrierFrequency_1:
        //Update DSP with new frequencies
        //
        if (grr_IsBtsAmTypeBlocked()||grr_IsBtsOpTypeBlocked()) pwr = 0;
        else 						                pwr = 1;

        grr_TuneTrxSynth(trx, pwr);

        break;

   case MIB_txPwrMaxReduction_0:
   case MIB_txPwrMaxReduction_1:
        //radio carriers' Power reduction changed
        //
        grr_PassOamDspPwrRedStep(trx);
	
	  break;
        
   default:
        // do nothing for now.
        DBG_WARNING("grr_ProcOamRadioCarrierPackage: received invalid mib param tag(%d)\n",tag);
        break;
   }	

   DBG_LEAVE();
}

void grr_ProcOamT31xxPackage(void)
{
   DBG_FUNC("grr_ProcOamT31xxPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   DBG_TRACE("grr_ProcOamT31xxPackage: do nothing but retrieve the new setting\n");
     
   grr_GetOamT31xxTimerStructure();

   DBG_LEAVE();
}

void grr_ProcOamAdjacentCellPackage(void)
{
   unsigned char i,j,ncellNo;
   
   DBG_FUNC("grr_ProcOamAdjacentCellPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   switch(MIB_ITM(grr_pItcRxOamMsg->mibTag))
   {
   case LEAF_adjCell_reselectionTable:
        DBG_TRACE("grr_ProcOamAdjacentCellPackage: reselction table updated\n");
        grr_PassOamMsSystemInformation2(0,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
        break;

   case LEAF_adjCell_handoverTable:
        DBG_TRACE("grr_ProcOamAdjacentCellPackage: handover table updated\n");
        i = 0;
        while ( i<OAMgrr_MAX_TRXS )
        { 
            //Pass updated information to both mobile and DSP
            grr_PassOamMsSystemInformation5(i,GRR_L1SI_TYPE_5,OAMgrr_RA_CELL_BARRED_STATE);	    

            ncellNo=grr_GetOamTotalAvailableHoNCells();
            for (j=0;j<OAMgrr_HO_ADJCELL_NO_MAX;j++)
                 grr_PassOamDspHoNeighCell(i,j,ncellNo);
            i++;
        }
        break;

   default:
        //Received unrecognized message tag
        DBG_WARNING("grr_ProcOamAdjacentCellPackage: unexpected OAM mib Table tag(%x)\n",
			MIB_ITM(grr_pItcRxOamMsg->mibTag) );
        break;
   }

   DBG_LEAVE();
}

void grr_ProcOamHandoverControlPackage(void)
{
   int i;

   DBG_FUNC("grr_ProcOamHandoverControlPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   //Retrive changed ho ctrl package
   grr_GetOamHandoverControlPackage();
	
   //Update DSP with new HandoverControlPackage
   i = 0;
   while ( i<OAMgrr_MAX_TRXS )
   { 
	 //Pass updated info to DSPs
       grr_PassOamDspHoServCell(i);
	 i++;
   }

   DBG_LEAVE();
}

void grr_ProcOamPowerControlPackage(void)
{ 
   int i;

   DBG_FUNC("grr_ProcOamPowerControlPackage", GRR_LAYER_PROC);
   DBG_ENTER();

   //Retrive changed pwr ctrl pkg
   grr_GetOamPowerControlPackage();
	
   //Update DSP with new power control package
   i = 0;
   while ( i<OAMgrr_MAX_TRXS )
   { 
       grr_PassOamDspPwrCtrl(i);
	 i++;
   }

   DBG_LEAVE();
}


void grr_ProcOamResourceConfigData(void)
{
   int i;
   int opState;

   DBG_FUNC("grr_ProcOamResourceConfigData", GRR_LAYER_PROC);
   DBG_ENTER();
	
   //Retrive Rrm data 
   opState = OAMgrr_CB_FEATURE_SETTING;
   grr_GetOamRrmData();

   //Process mib rrm config change
   switch (grr_pItcRxOamMsg->mibTag)
   {
   case MIB_rm_nim_0_1:
        //CB feature control
        //
        if (grr_pItcRxOamMsg->val.varVal>1)
        {
            DBG_WARNING("grr_ProcOamResourceConfigData: invalid cb config val(%d)\n",
                         grr_pItcRxOamMsg->val.varVal);
            printf("grr_ProcOamResourceConfigData: invalid cb config val(%d)\n",
                         grr_pItcRxOamMsg->val.varVal);
            assert(0);

        } else
        {
            grr_PassOamMsSystemInformation4(0,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);
            grr_SendCbCfgSetting(MODULE_GRR, rm_MsgQId);
        }
        
        break;

   case MIB_rm_nim_0_3: //Cell Reselect Offset
   case MIB_rm_nim_0_4: //Temporary Offset
   case MIB_rm_nim_0_5: //Penalty Offset
   case MIB_rm_nim_0_6: //Cell Bar Qualify
 //case MIB_rm_nim_0_7: //auth/cipher feature control
        //Update mobile with the new config params
        // 
        grr_PassOamMsSystemInformation3(0, GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
        grr_PassOamMsSystemInformation4(0, GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);

        break;

   case MIB_rm_nim_1_1:
        //Overloaded cell barring. FS needed
        //

        break;

        grr_PassOamMsSystemInformation2(0,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
        grr_PassOamMsSystemInformation3(0,GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
        grr_PassOamMsSystemInformation4(0,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);

        break;
         
   default:
        DBG_WARNING("grr_ProcOamResourceConfigData: invalid changed mib param tag(%d)\n",
	               grr_pItcRxOamMsg->mibTag);
        break;
   } 

   DBG_LEAVE();
}

bool grr_ProcOamValidateMsg(void)
{
   bool ret;
   unsigned char trx;

   DBG_FUNC("grr_ProcOamValidateMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   ret = true;

   switch (grr_pItcRxOamMsg->mibTag)
   {
   case MIBT_channelCombination_0:
   case MIBT_channelCombination_1:

        //Attempt to change trx0 slot0 channel comb
        //

        if ( (grr_pItcRxOamMsg->val.varTbl.idx==0) &&
             (grr_pItcRxOamMsg->mibTag==MIBT_channelCombination_0) )
        {
            //Not allowed to change trx0slot0's channel combination
            DBG_WARNING("grr_ProcOamValidateMsg: apply chan comb(%d) to trx(0) slot(0)\n",
                         OAMgrr_TRX_SLOT_COMB(0,0));
            printf("grr_ProcOamValidateMsg: apply chan comb(%d) to trx(0) slot(0)\n",
                         OAMgrr_TRX_SLOT_COMB(0,0));
            grr_SubmitAlarm(grr_TrxSlotChanCombChangeAlarmCode[0][0]);
            ret = false;
        } else
        { 
            //Before change channel slot combination, the trx-slot must be locked
            //
            if (grr_pItcRxOamMsg->mibTag==MIBT_channelCombination_0) trx = 0;
 	      else							               trx = 1;

            if (OAMgrr_CHN_AMSTATE(trx,grr_pItcRxOamMsg->val.varTbl.idx) != locked)
            {
                //GP Manager shall block this change unless amState is locked
                //
                DBG_WARNING("grr_ProcOamValidateMsg: lock trx(%d) slot(%d) before change chan comb\n",
                             trx, grr_pItcRxOamMsg->val.varTbl.idx);
                printf("grr_ProcOamValidateMsg: lock trx(%d) slot(%d) before change chan comb\n",
                             trx, grr_pItcRxOamMsg->val.varTbl.idx);
                ret = false;
            }
        }
  
        break;

   default:
        break;
   }

   DBG_LEAVE();
   return ret;
}

#endif //__GRR_PROCOAMMSG_CPP__

