/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_S9DEACTIVATINGSIGTRAFFICCHAN_CPP__
#define __RM_S9DEACTIVATINGSIGTRAFFICCHAN_CPP__

#include "rm\rm_head.h"

void rm_S9DeactivatingSigTrafficChan(void)
{
   int time;
   rm_PhyChanDesc_t *pPhyChan;


   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_S9DeactivatingSigTrafficChan\n"));
 
   if (rm_pSmCtxt->smType != RM_MAIN_SM)
   {
       //Channel Activation always associated with main SM
       IDEBUG__(("WARNING@rm_S9DeactivatingSigTrafficChan:not main SM\n"));
       return;
   }

   switch(rm_ItcRxOrigin)
   {
   case MODULE_L1:
	  //Get a MPH_INFO_INF message. Analyze and process it
        if ( rm_pItcRxL1Msg->l3_data.buffer[0]!=RM_L1MG_DEDMGMT           ||  
             rm_pItcRxL1Msg->l3_data.buffer[1]!=RM_L1MT_RFCHANRELACK_MSB  ||
             rm_pItcRxL1Msg->l3_data.buffer[2]!=RM_L1MT_RFCHANRELACK_LSB  )
	  {
	     //Got unexpected message from L1	
   	     IDEBUG__(("WARNING@rm_S9DeactivatingSigTrafficChan:L1 msg PD:%x;MT:%x,%x\n",
		       rm_pItcRxL1Msg->l3_data.buffer[0],rm_pItcRxL1Msg->l3_data.buffer[1],		
		       rm_pItcRxL1Msg->l3_data.buffer[2] ));
           return;
	  } 

	  break;
 
   case MODULE_RM: 
	  //Currently T3111 expiry messages comes here
        //PDEBUG__(("rm_S9DeactivatingSigTrafficChan: Timer Handling, timerId=%d\n",
        //rm_pItcRxRmMsg->timerId));

	  if ( rm_pItcRxRmMsg->primitive_type != RM_TIMER_IND   ||
             rm_pSmCtxt->pPhyChan->T3111.state == RM_FALSE    ||
             rm_pItcRxRmMsg->timerId != RM_T3111              )
        {
   	       IDEBUG__(("WARNING@rm_S9DeactivatingSigTrafficChan: prim:%d,timer sta=%d\n",
		            rm_pItcRxRmMsg->primitive_type,rm_pSmCtxt->pPhyChan->T3111.state ));
             return;
        }
	  break;

   default:
	  //TBD: Unexpected message orgination
        //IDEBUG__(("WARNING@rm_S9DeactivatingSigTrafficChan:wrong msg origination:%d\n",
	  //	       rm_ItcRxOrigin));
         
	  return;
        break;

   } //End of switch()

   //Free radio resources and cancel timers
   pPhyChan              = rm_pSmCtxt->pPhyChan; 
   pPhyChan->state       = RM_PHYCHAN_FREE;
   pPhyChan->rslCount    = 0;
   pPhyChan->chanBusy    = RM_FALSE;
   pPhyChan->T3101.state = RM_FALSE;
   pPhyChan->T3111.state = RM_FALSE;
   pPhyChan->T3109.state = RM_FALSE;
   rm_StopTimer( rm_ItcRxEntryId, RM_T3109 );
   rm_StopTimer( rm_ItcRxEntryId, RM_T3111 );
   rm_StopTimer( rm_ItcRxEntryId, RM_T3101 );
   pPhyChan->pMSmCtxt->state = RM_S1RM_RUNNING;
   pPhyChan->pASmCtxt->state = RM_S1RM_RUNNING;

   //Activate CBCH if its activation is pending on release of SDCCH/4 (2)
   if ( (pPhyChan==rm_pCbchChan)&&(pPhyChan->usable==RM_PHYCHAN_USABLE)&&
        (OAMrm_CBCH_OPSTATE) )
   {
       rm_pCbchChan->usable = RM_PHYCHAN_CBCH_USABLE; //block SDCCH any attempt now
       if (rm_ChanCbchActive)
           IDEBUG__(("WARNIG@rm_OamProcResourceConfigData: cbch is already on \n"));

       rm_ChanCbchActive = RM_FALSE; 
       rm_ActivateCbchChan();
   } 

   if (pPhyChan->chanType==RM_PHYCHAN_TCHF_S)      rm_PostTchfRelHandler(pPhyChan);
   else if (pPhyChan->chanType==RM_PHYCHAN_SDCCH4) rm_PostSdcch4RelHandler(pPhyChan);
   else EDEBUG__(("ERROR@rm_S9DeactivatingSigTrafficChan:chanType=%d\n",pPhyChan->chanType));

} // End of rm_S9DeactivatingSigTrafficChan()

//GP2
#if 0
void rm_LockParentTrxSlotIfNeeded(int trx, int slot)
{
     int i;

     PDEBUG__(("INFO@rm_LockParentTrxSlotIfNeeded: (%d,%d)\n", trx, slot));

     //Check if (trx, slot) existent
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS) ||
          slot>=OAMrm_SLOTS_PER_TRX )
     {
	   IDEBUG__(("WARNING@rm_LockParentTrxSlotIfNeeded: invalid (%d,%d)\n",
		        trx,slot));
	   return;
     }

     //Check if the TS is shutting-down pending now
     if (OAMrm_CHN_AMSTATE(trx,slot)==shuttingDown)
     {
	   //Lock this (trx,slot) if all its resident channels are locked
         if (trx||slot)
         {
             //(trx,slot) carries only one functional channel ie tch-f
             //Fire an information alarm on MIB for ViperWatch watching
             PDEBUG__(("INFO@rm_LockParentTrxSlotIfNeeded:(%d,%d) shuttingDown->locked\n",
                        trx, slot));

             rm_SetTrxSlotAdminStateIfNeeded(trx,slot,locked);
             rm_LockParentTrxIfNeeded(trx);

         } else
         {
             //(trx,slot) carrying BCCH and CCCH, equal to lock the BTS
             
	       for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
             {
 	            if (rm_PhyChanSdcch4[i].amState==shuttingDown)
                      break;
			else 
			    rm_PhyChanSdcch4[i].amState==locked;
             }
             if (i>=OAMrm_MAX_SDCCH4S)
             {
                  rm_SetTrxSlotAdminStateIfNeeded(trx,slot,locked);

                  PDEBUG__(("INFO@rm_LockParentTrxSlotIfNeeded:(%d,%d) shuttingDown->locked\n",
                             trx, slot));

                  rm_LockParentTrxIfNeeded(trx);

                  //if (OAMrm_TRX_AMSTATE(trx)==shuttingDown)
                  // {
                  //    //(trx,slot) carries only one functional channel ie tch-f
                  //    //Fire an information alarm on MIB for ViperWatch watching
                  //    PDEBUG__(("INFO@rm_LockParentTrxSlotIfNeeded:(%d,%d) shuttingDown->locked\n",
                  //               trx, slot));
                  //    rm_LockParentTrxIfNeeded(trx);
                  //} 
                  //else 
                  //{  
                  //    if (OAMrm_BTS_AMSTATE == shuttingDown)
                  //    {
			//        //All (trx=0,slot=0) resident channels are already locked,
                  //        //This is equivalent to locking the whole BTS, thus do it! 
                  //        PDEBUG__(("INFO@rm_LockParentTrxSlotIfNeeded:(%d,%d) ie whole VC shuttingDown->locked\n",
                  //                   trx, slot));
                  //        rm_SetBtsAdminStateIfNeeded(locked);
                  //        rm_ProcBtsAdminStateLocked();
                  //    }
                  //} 
		 }
     	   }
     }
}

void rm_LockParentTrxIfNeeded(int trx)
{
     PDEBUG__(("INFO@rm_LockParentTrxIfNeeded: trx=%d\n",trx));

     //Check if trx is existent or not
     if ( !( (OAMrm_TRX_ONOFF_MAP)&(1<<trx) ) || (trx>=OAMrm_MAX_TRXS))
     {
	   IDEBUG__(("WARNING@rm_LockParentTrxIfNeeded: invalid trx=%d\n",
		        trx));
	   return;
     }

     if (OAMrm_TRX_AMSTATE(trx)==shuttingDown)
     {
	   //Lock this trx if all the channels are already locked now
          int i;
 
	   for (i=0;i<OAMrm_SLOTS_PER_TRX;i++)
         {
		  if (OAMrm_CHN_AMSTATE(trx, i)==shuttingDown)
			break;
		  else 
		  {
                  rm_SetTrxSlotAdminStateIfNeeded(trx,i,locked);
		  }
         }
	   
	   if (i>=OAMrm_SLOTS_PER_TRX)
         {
             rm_SetTrxAdminStateIfNeeded(trx,locked);

		 //All trx-resident time slots already locked now,lock trx
             //Fire an information alarm on MIB for ViperWatch watching
             PDEBUG__(("INFO@rm_LockParentTrxIfNeeded: trx '%d' shuttingDown->locked\n",
                       trx));
             rm_LockParentBtsIfNeeded();
         }
     } 
}

void rm_LockParentBtsIfNeeded(void)
{
     PDEBUG__(("INFO@rm_LockParentBtsIfNeeded\n"));
     
     //Check if BTS is shutting down pending first
     if (OAMrm_BTS_AMSTATE==shuttingDown)
     {
         //Lock BTS if all its trxes are already locked
	   int i;
         for (i=0; i<OAMrm_MAX_TRXS; i++ )
         { 
             //Check if ith TRX is existent
             if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
     	       {  
	            if (OAMrm_TRX_AMSTATE(i)==shuttingDown)
			    break;
			else
			{
                      rm_SetTrxAdminStateIfNeeded(i,locked);
			}
             }
         }   
         if (i>=OAMrm_MAX_TRXS)
         {
		 //all trxes have been locked, thus locking BTS
             rm_SetBtsAdminStateIfNeeded(locked);
             rm_SendTuneSynthToTrx(0,0);
             rm_SendTuneSynthToTrx(1,0);
	   }
     }
}

void rm_SetTrxSlotAdminStateIfNeeded(int trx, int slot, AdministrativeState state)
{
     PDEBUG__(("INFO@rm_SetTrxSlotAdminStateIfNeeded: (%d,%d,%d)\n", trx,slot,state));

     if (OAMrm_CHN_AMSTATE(trx,slot) != state)
     {
         switch(state)
         {
         case locked:
              rm_SetRmOpState(rm_TrxSlotAdminLockedAlarmCode[trx][slot]);
              break;
         case unlocked:
              rm_SetRmOpState(rm_TrxSlotAdminUnlockedAlarmCode[trx][slot]);
              break;
         case shuttingDown:
              rm_SetRmOpState(rm_TrxSlotAdminShuttingDownAlarmCode[trx][slot]);
              break;
         default:
              IDEBUG__(("WARNING@rm_SetTrxSlotAdminStateIfNeeded: (%d,%d,%d)\n", trx,slot,state));
              break;
         }
      }
      OAMrm_CHN_AMSTATEa(trx, slot) = state;
      rm_SetTrxSlotAdminState(trx, slot, state);
}

void rm_SetBtsAdminStateIfNeeded(AdministrativeState state)
{
     PDEBUG__(("INFO@rm_SetBtsAdminStateIfNeeded: state=%d\n",state));

     if ( OAMrm_BTS_AMSTATE != state )
     {
          if (state == locked)
              rm_SetRmOpState(EC_RM_BTS_LOCKED);
          else if (state == unlocked)
              rm_SetRmOpState(EC_RM_BTS_UNLOCKED);
          else if (state == shuttingDown)
              rm_SetRmOpState(EC_RM_BTS_SHUTTINGDOWN);
          else 
              IDEBUG__(("WARNING@rm_SetBtsAdminStateIfNeeded: invalid state=%d\n",state));
     }
     OAMrm_BTS_AMSTATE = state;
     rm_SetBtsAdminState(state);
}

void rm_SetTrxAdminStateIfNeeded(int trx, AdministrativeState state)
{
     int i;

     PDEBUG__(("INFO@rm_SetTrxAdminStateIfNeeded: trx=%d, state=%d\n",trx,state));
     if (OAMrm_TRX_AMSTATE(trx) != state)
     {
         switch(state)
         {
         case locked:
	        rm_SetRmOpState(rm_TrxAdminLockedAlarmCode[trx]);
              break;
         case unlocked:
	        rm_SetRmOpState(rm_TrxAdminUnlockedAlarmCode[trx]);
              break;
         case shuttingDown:
	        rm_SetRmOpState(rm_TrxAdminShuttingDownAlarmCode[trx]);
              break;
         default:
              IDEBUG__(("WARNING@rm_SetTrxAdminStateIfNeeded: trx=%d, invalid state=%d\n",
                         trx, state));
              break;
         }
     }
     OAMrm_TRX_AMSTATE(i)=locked;
     rm_SetTrxAdminState(i,locked);
}                
#endif //#if 0


//GP2
void rm_PostTchfRelHandler(rm_PhyChanDesc_t *pPhyChan)
{
   unsigned char trx, slot;

   trx = pPhyChan->trxNo; slot = pPhyChan->slotNo;

   switch(pPhyChan->amState)
   {
   case shuttingDown:
        pPhyChan->amState = locked;
        if (trx||slot)
        {
             if (--rm_NoOfUsableTchf < 0) rm_NoOfUsableTchf = 0;
             PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);

             grr_SendTrxSlotShutdownAck(MODULE_RM, grr_MsgQId, trx, slot);
        }

        break;

   case unlocked:
        rm_NoOfIdleTchf++;
        PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.addRecord(rm_NoOfIdleTchf);
        if (rm_AllAvailableTchf.allocated)
        {
            rm_AllAvailableTchf.allocated = RM_FALSE;
            PM_CellMeasurement.allAvailableTCHAllocatedTime.stop();
        }

        //Unbar the cell if needed
        grr_OverloadedCellBarOff();

        break;

   case locked:
        break;

   default:
        IDEBUG__(("WARNING@rm_PostTchfRelHandler: invalid amState(%d) for trx(%d)slot(%d)\n",
                   pPhyChan->amState, trx, slot));
        break;
   }
}

//GP2
void rm_PostSdcch4RelHandler(rm_PhyChanDesc_t *pPhyChan)
{
   unsigned char trx, slot, i;

   trx = pPhyChan->trxNo; slot = pPhyChan->slotNo;

   switch(pPhyChan->amState)
   {
   case shuttingDown:
        if (--rm_nbrOfAvailableSDCCHs < 0) rm_nbrOfAvailableSDCCHs = 0;
        PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);

        //Check if the TS is shutting-down pending now
        if (OAMrm_CHN_AMSTATE(trx,slot) == shuttingDown)
        {
            //
            for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
            {
 	         if (rm_PhyChanSdcch4[i].amState==shuttingDown) break;
 	         else             rm_PhyChanSdcch4[i].amState==locked;
            }
         
            if (i>=OAMrm_MAX_SDCCH4S)
                grr_SendTrxSlotShutdownAck(MODULE_RM, grr_MsgQId, trx, slot);
        }
       
        break;

   case unlocked:
        if (rm_AllAvailableSdcch4.allocated)
        {
            rm_AllAvailableSdcch4.allocated = RM_FALSE;
            PM_CellMeasurement.allAvailableSDCCHAllocatedTime.stop();
        }

        break;

   case locked:
        break;

   default:
        IDEBUG__(("WARNING@rm_PostSdcch4RelHandler: invalid amState(%d) for trx(%d)slot(%d)chan(%d)\n",
                   pPhyChan->amState, trx, slot, pPhyChan->subChanNumber));
        break;
   }
}

#endif //__RM_S9DEACTIVATINGSIGTRAFFICCHAN_CPP__
