/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITALL_CPP__
#define __RM_INITALL_CPP__

#include "rm\rm_head.h"

STATUS l1pTrxSwap( u8 trxA, u8 trxB);

extern "C" {
	int drfIQSwapGet(void);
}

//Completely init RM
void rm_InitAll(void)
{
     int i;

     //Monitoring entrance to a func
     RDEBUG__(("ENTER@rm_InitAll\n"));

     //Reset RM internal sts
	 rm_DspCiphCap = 0;
     rm_OpState = EC_NOERROR;
     rm_AmState00 = unlocked;
     rm_VcIsBarred = RM_FALSE;
     rm_CellBarred = CNI_RIL3_CELL_NOT_BARRED;
     rm_AllAvailableTchf.allocated = RM_FALSE;
     rm_AllAvailableSdcch4.allocated = RM_FALSE;

     //CBCH assumed on SDCCH4/2
     rm_ChanCbchActive=RM_FALSE;
     rm_pCbchChan=&rm_PhyChanSdcch4[2];

     //GP2
     //
     OAMrm_CBCH_OPSTATE = false;

     rm_LiveSglCfg = RM_TRUE;
     rm_NoOfBusySdcch4=0;		  //Cur Busy SDCCH
     rm_NoOfUsableTchf=0;		  //No tchf usable
     rm_nbrOfAvailableSDCCHs=0;       //No sdcch4 usable
     rm_NoOfBusyTchf=0;			  //Cur Busy TCH/F
     rm_NoOfIdleTchf=0;			  //Cur Idle TCH/F
     rm_maxNbrOfBusyTCHs=0;
     rm_maxNbrOfBusySDCCHs=0;

     //sys_Init();  			  //Create rm msgQ

     //GP2
     //rm_RetrvConfigFromOam(); 	  //Retrv OAM data
     //if (OAMrm_BCCH_CARRIER_phTRX==1) //Swap two TRXes
     //    l1pTrxSwap(0,1);
     //rm_SetRmOpState(EC_BOOTING_UP);  //RM booting up!
     rm_WaitForGrrReadyToGo();
     //rm_OamRegTrapVars();

     //Start initialize all the RM resources from now!
     rm_InitRm();			        //Init RM itself
     rm_InitLAPDm();			  //Create L2 OIDs
     rm_InitIRT();			  //Create entryID
     csu_Init();				  //Init CSU table

     //GP2
     //for (i=0;i<OAMrm_MAX_TRXS;i++)
     //{
     //     rm_InitRcPkg(i);		  //Initialize RC0
     //     rm_InitTrxPkg(i);		  //Initialize trx
     //     rm_InitTrxWDog(i);	          //Init trx twdog
     //}
       grr_SendReadyToGoAck(MODULE_RM, grr_MsgQId);
       rm_InitCb();
}
void rm_WaitForGrrReadyToGo(void)
{
   int result, waitGrr;

   RDEBUG__(("ENTER@rm_WaitForGrrReadyToGo\n"));

   waitGrr=true;

   while(waitGrr)
   {
      //Read a msg from queue or pend on arrival of message
      result = rm_msgQReceive(rm_MsgQId, (s8 *)&rm_ItcRxMsg, 
                              sizeof(rm_ItcRxMsg_t), WAIT_FOREVER);
      if (result==ERROR)
      {   //Error in reading VxWork msg queue. Ignore msg!
          rm_SetRmOpState(EC_RM_OS_CALL_MSGQRECEIVE_FAILED);
          EDEBUG__(("ERROR@rm_WaitForGrrReadyToGo:VxWorks Q read err:%d\n",errno));
          assert(0);
      } 

      rm_pItcRxGrrMsg = (rm_ItcRxGrrMsg_t*) &rm_ItcRxMsg;

      if ( (rm_pItcRxGrrMsg->module_id==MODULE_GRR) &&
           (rm_pItcRxGrrMsg->message_type==GRR_READY_TO_GO_REQ) )
            waitGrr=false;
       else
       {
 	    if (rm_DbgDump) BYDUMP__(( (u8*)&rm_ItcRxMsg, result));
       }
   }
}        

//GP2
#if 0

//Start trx watchdog timer 
void rm_InitTrxWDog(u8 trx)
{
     WDOG_ID timerId;

     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_InitTrxWDog\n"));

     //Initialize watch dog timer for trx
     if ( (timerId=wdCreate()) == NULL )
     {
          rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
          EDEBUG__(("ERROR-rm_InitTrxWDog: wdCreate error()\n"));
     } else
     {
          rm_TrxMgmt[trx].TWDOG.id    = timerId;
          rm_TrxMgmt[trx].TWDOG.time  = OAMrm_TWDOG;
     	    rm_TrxMgmt[trx].TWDOG.state = RM_FALSE;
          //rm_StartTimer(trx, RM_TWDOG);
    }
}


//Initialize rc package
void rm_InitRcPkg(u8 rc)
{
     //Monitoring entrance to a func
     RDEBUG__(("ENTER-rm_InitRcPkg\n"));
     rm_SetRcOpState(rc, opStateDisabled);
}

//Initialize trx package
void rm_InitTrxPkg(u8 trx)
{
     //Monitoring entrance to a func
     RDEBUG__(("ENTER-rm_InitTrxPkg\n"));

     //Disable trx and its channels
     rm_SetTrxOpState(trx, opStateDisabled);   
     rm_SetAllTrxChnOpState(trx, opStateDisabled); 

     //Initialize trx management data
     RM_MEMSET(&rm_TrxMgmt[trx],sizeof(rm_TrxMgmt_t));
     rm_TrxMgmt[trx].done  = RM_FALSE;
     rm_TrxMgmt[trx].slot  = 0;
     rm_TrxMgmt[trx].tdma1 = 0xff; 
     rm_TrxMgmt[trx].tdma2 = 0xff;
     rm_TrxMgmt[trx].tdma3 = 0xff; 
     rm_TrxMgmt[trx].tdma4 = 0xff;
     rm_TrxMgmt[trx].alive = RM_TRUE;
     rm_TrxMgmt[trx].state = RM_TRXMGMT_S0_BOOTSTRAP_IND;

     //RM does not go until it gets DSP Bootstrap signal 
}

//Set operational state of one channel under trx
void rm_SetOneTrxChnOpState(u8 trx,u8 slot,EnableDisable state)
{
     MibTag tag;

     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_SetOneTrxChnOpState\n"));

     OAMrm_CHN_OPSTATEa(trx,slot) = state;
     if (trx==0) tag = MIBT_channelOperationalState_0;
     else        tag = MIBT_channelOperationalState_1;
     if (STATUS_OK!=oam_setTblEntryField(MODULE_RM,tag,slot,state))
     {
         rm_SetRmOpState(EC_RM_OAM_CALL_SETTBLENTRYFIELD_FAILED);
         EDEBUG__(("ERROR@rm_SetOneTrxChnOpState:OAM failed(%d,%d,%x,%d)\n",
                    trx,slot,tag,state));
     }
}

//Set operational state of all channels under trx
void rm_SetAllTrxChnOpState(u8 trx,EnableDisable state)
{
     int i;

     //Monitoring entrance to a func
     RDEBUG__(("ENTER-rm_SetAllTrxChnOpState\n"));

     for (i=0;i<8;i++)		
          rm_SetOneTrxChnOpState(trx,i,state);	
}   

//Set trx operational state as required
void rm_SetTrxOpState(u8 trx, EnableDisable state)
{
     MibTag tag;

     OAMrm_TRX_OPSTATE(trx) = state;

     if (trx==0)
         tag=MIB_basebandOperationalState_0;
     else
         tag=MIB_basebandOperationalState_1;
     if (STATUS_OK!=oam_setMibIntVar(MODULE_RM, tag, state))
     {
         rm_SetRmOpState(EC_RM_OAM_CALL_SETMIBINTVAR_FAILED);
         EDEBUG__(("ERROR@rm_SetTrxOpState:OAM failed (%d,%x,%d)\n",
                    trx,tag,state));
     }
}

//Set rc operational state as required
void rm_SetRcOpState(u8 rc, EnableDisable state)
{
     MibTag tag;

     OAMrm_RC_OPSTATE(rc) = state;

     if (rc==0)
         tag=MIB_carrier_operationalState_0;
     else
         tag=MIB_carrier_operationalState_1;
     if (STATUS_OK!=oam_setMibIntVar(MODULE_RM, tag, state))
     {    
         rm_SetRmOpState(EC_RM_OAM_CALL_SETMIBINTVAR_FAILED);
         EDEBUG__(("ERROR@rm_SetRcOpState:OAM failed (%d,%x,%d)\n",
                    rc,tag,state));
     }
}

//Process L1 trx mgmt message 
void rm_ProcTrxMgmtMsg(u8 trx)
{
   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_ProcTrxMgmtMsg\n"));

   //Process in terms of trx state
   switch ( rm_TrxMgmt[trx].state )
   {
   case RM_TRXMGMT_S0_BOOTSTRAP_IND:
        //Await dsp bootstrap signal
        if (!RM_L1MSGCMP(RM_L1MG_TRXMGMT,RM_L1MT_BOOTSTRAPCNF_MSB,RM_L1MT_BOOTSTRAPCNF_LSB))
        {   //Got unexpected message
            IDEBUG__(("WARNING@rm_ProcTrxMgmtMsg:exp bootInd not(%d,%d,%d,%d)\n",
                       rm_pItcRxL1Msg->l3_data.buffer[0],
                       rm_pItcRxL1Msg->l3_data.buffer[1],
	               rm_pItcRxL1Msg->l3_data.buffer[2],
	               rm_pItcRxL1Msg->l3_data.buffer[3]));
            return;
        }

		rm_DspCiphCap = rm_pItcRxL1Msg->l3_data.buffer[4];

        rm_StartTimer(trx, RM_TWDOG);

        //Auto-detection of RF board
          //1. AisValue = drfTypeGet( )  -- Chetan: please tell us the return values (e.g., 0=GSM900, 1=DCS1800, 2=PCS1900?)
          //This is done in OAM already.

          //2. IQSwapValue = drfIQSwapGet( )  (the 4th character is capital i)  --  This function returns a True/False value
          //Decide if I-Q swap needed
          OAMrm_I_Q_SWAP = drfIQSwapGet();

          //3a. FreqHopDsp0Value = drfFreqHopGet(0)  -- This function returns a True/False value
          //3b. FreqHopDsp1Value = drfFreqHopGet(1)  -- This function returns a True/False value
          //Decide if hopping cable. If not, turn off the hopping feature. Assumed not capable now.
        OAMrm_HOPPING_CTRLa = 0;
        
        //Got expected message i.e. bootstrap, config trx
        rm_SendTrxConfigToTrx(trx);
        rm_TrxMgmt[trx].state=RM_TRXMGMT_S1_TRXCONFIG_ACK;
        ConfigTxPwrValues(trx);
        break;

   case RM_TRXMGMT_S1_TRXCONFIG_ACK:
        //Wait for trx configure ack forever. No nack defined !
        if (!RM_L1MSGCMP(RM_L1MG_TRXMGMT,RM_L1MT_TRXCONFIGACK_MSB,
                      RM_L1MT_TRXCONFIGACK_LSB))
        {   //Got unexpected message
            IDEBUG__(("WARNING@rm_ProcTrxMgmtMsg:exp trxAck not(%d,%d,%d,%d)\n",
                       rm_pItcRxL1Msg->l3_data.buffer[0],
                       rm_pItcRxL1Msg->l3_data.buffer[1],
	                 rm_pItcRxL1Msg->l3_data.buffer[2],
	                 rm_pItcRxL1Msg->l3_data.buffer[3]));
            return;
        }
   
        //Init trx mgmt structure and start init its TSs!
        rm_TrxMgmt[trx].state=RM_TRXMGMT_S2_SLOTACTIV_ACK;
        rm_TrxMgmt[trx].slot=0; //start slot act from ts0
        rm_ActTimeSlot(trx,0);  //Activat 1st slot of trx
        break;

   case RM_TRXMGMT_S2_SLOTACTIV_ACK:
        //Apply sanitory Check here
        if (rm_pItcRxL1Msg->l3_data.buffer[4] != rm_TrxMgmt[trx].slot ||
            rm_pItcRxL1Msg->l3_data.buffer[4] >= OAMrm_SLOTS_PER_TRX   )
        {
            EDEBUG__(("ERROR@rm_ProcTrxMgmtMsg: encounter mess-up (%d,%d)\n",
                       rm_TrxMgmt[trx].slot,rm_pItcRxL1Msg->l3_data.buffer[4]));
            return;
        }
 
        //Wait for slot activati ack
        if (RM_L1MSGCMP(RM_L1MG_TRXMGMT,RM_L1MT_SLOTACTIVACK_MSB,
                      RM_L1MT_SLOTACTIVACK_LSB))
        {   
            PDEBUG__(("INFO@rm_ProcTrxMgmtMsg:slotAct ack to(%d,%d)\n",
                       rm_pItcRxL1Msg->l3_data.buffer[3],
                       rm_pItcRxL1Msg->l3_data.buffer[4]));

            //Got positive ack, (trx,slot) goes into enabled state
            rm_SetOneTrxChnOpState(trx,rm_pItcRxL1Msg->l3_data.buffer[4],opStateEnabled);
            rm_ProcSlotActRsp(trx,rm_pItcRxL1Msg->l3_data.buffer[4],RM_PHYCHAN_USABLE);

        } else if (RM_L1MSGCMP(RM_L1MG_TRXMGMT,RM_L1MT_SLOTACTIVNACK_MSB,
                   RM_L1MT_SLOTACTIVNACK_LSB))
        {   
            IDEBUG__(("INFO-rm_ProcTrxMgmtMsg:slotAct nack to(%d,%d)\n",
                       rm_pItcRxL1Msg->l3_data.buffer[3],
                       rm_pItcRxL1Msg->l3_data.buffer[4]));

            //Got negative ack, (trx,slot) goes into disabled state
            rm_SetOneTrxChnOpState(trx, rm_pItcRxL1Msg->l3_data.buffer[4],opStateDisabled);
            rm_ProcSlotActRsp(trx,rm_pItcRxL1Msg->l3_data.buffer[4],RM_PHYCHAN_UNUSABLE);

        } else
        {   
            //Got unexpected message
            IDEBUG__(("WARNING@rm_ProcTrxMgmtMsg:exp tsAck not(%d,%d,%d,%d)\n",
                       rm_pItcRxL1Msg->l3_data.buffer[0],
                       rm_pItcRxL1Msg->l3_data.buffer[1],
	                 rm_pItcRxL1Msg->l3_data.buffer[2],
	                 rm_pItcRxL1Msg->l3_data.buffer[3]));
            return;
        }

        //Check if all time slots of this trxs are initialized or not!
        if (rm_pItcRxL1Msg->l3_data.buffer[4] == OAMrm_SLOTS_PER_TRX-1)
        {  
            rm_PassOpParamsToTrx(trx);
            rm_TrxMgmt[trx].done =RM_TRUE;
            rm_TrxMgmt[trx].state=RM_TRXMGMT_S3_TRXCONFIG_END;

            //Now trx goes into enabled state if it is needed
            if (!trx && (OAMrm_CHN_OPSTATE(trx,0)==opStateDisabled) )
                 rm_SetTrxOpState(trx,opStateDisabled);
            else
                 rm_SetTrxOpState(trx,opStateEnabled);
        } else
        {
            //Activate next time slot of this trx
            rm_TrxMgmt[trx].slot++;
            rm_ActTimeSlot(trx,rm_TrxMgmt[trx].slot);
        } 
        break;

   case RM_TRXMGMT_S3_TRXCONFIG_END:
        //trx is in normal op. state
        break;

   default:
        //Unrecognized trx state
        IDEBUG__(("WARNING@rm_ProcTrxMgmtMsg: invalid trx SM state=%d\n",
                   rm_TrxMgmt[trx].state));
        break;
   }
}

//Process L1 ack & nack message to slot activ
u8 rm_ProcSlotActRsp(u8 trx,u8 slot,u8 ackVal)
{
   int j;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_ProcSlotActRsp\n"));
 
   //Got an expected ts Ack 
   switch(OAMrm_TRX_SLOT_COMB(trx,slot))   
   {
   case OAMrm_SLOT_COMB_1:
        if (ackVal==RM_PHYCHAN_USABLE)
        {
            //PMADDED:nbrOfAvailableTCHs
            if (OAMrm_CHN_AMSTATE(trx,slot) != locked)
            {
                rm_NoOfUsableTchf++;

                PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                //PM_CellMeasurement.nbrOfAvailableTCHs.increment();
            }
        }

        //Enable func channels 
        rm_EnDisableRmTchf(trx,slot,ackVal);

        break;

   case OAMrm_SLOT_COMB_5:
        //Sanitory check
        if ( trx || slot )
        {
            rm_SetRmOpState(EC_RM_CHAN_COMB_5_MISPLACED);
            //comb 5 should only appear on trx=0 and slot=0 in the design of RM
            EDEBUG__(("ERROR@rm_ProcSlotActRsp: chan comb 5 comes on (%d,%d)\n",
                       trx,slot));
            return (RM_TRUE);
        }

        //Now PCH ready to go !

        rm_PhyChanBCcch[RM_PCH_IDX].usable  = ackVal;
        if (ackVal==RM_PHYCHAN_USABLE)
        {
            rm_PhyChanBCcch[RM_PCH_IDX].trxNo   =  trx;
            rm_PhyChanBCcch[RM_PCH_IDX].slotNo  =  slot;
            rm_PhyChanBCcch[RM_PCH_IDX].state   =  RM_PHYCHAN_FREE;
        }

        //Make 4 sdcch4 channels usable 
        for (j=0;j<OAMrm_MAX_SDCCH4S;j++)
        {
	       rm_PhyChanSdcch4[j].usable=ackVal;

             if (ackVal==RM_PHYCHAN_USABLE && (OAMrm_CHN_AMSTATE(trx,slot) != locked) )
             {
                 //PMADDED:nbrOfAvailableSDCCHs
                 
                 if (++rm_nbrOfAvailableSDCCHs>4)
                     printf("RR INFO@rm_ProcSlotActRsp: nbrOfAvailableSDCCHs %d\n",
                             rm_nbrOfAvailableSDCCHs);
                 else
                     PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);
                     //PM_CellMeasurement.nbrOfAvailableSDCCHs.increment();
             }
        }
        break;

   default:
        //Unsupported channel combination
        rm_SetRmOpState(EC_RM_CHAN_COMB_UNSUPPORTED);
        IDEBUG__(("WARNING@rm_ProcSlotActRsp:unsupported channel comb '%d'\n",
                   OAMrm_TRX_SLOT_COMB(trx,slot) ));
        break;
   }
}

//Mark the tchf chan in rm as usable
u8 rm_EnDisableRmTchf(u8 trx, u8 slot, u8 ackVal)
{
     int i;

     //Monitoring entrance to a function
     RDEBUG__(("ENTER@rm_EnDisableRmTchf\n"));

     for (i=0;i<OAMrm_MAX_TCHFS;i++)
     {
          if (rm_PhyChanTchf[i].trxNumber==trx &&
             (rm_PhyChanTchf[i].chanNumberMSB&0x07)==slot)
          {  
              //found corresponding rm tchf channel
 	        rm_PhyChanTchf[i].usable = ackVal;
              break;
          }
     }
 
     if (i>=OAMrm_MAX_TCHFS)
     {  
         //trx-slot pair not valid
         EDEBUG__(("ERROR@rm_EnDisableRmTchf:no tchf linked to(%d,%d)\n",
                   trx,slot));
     }

     return RM_TRUE;
} 

//Acitvate the given trx and slot
void rm_ActTimeSlot(u8 trx,u8 slot)
{
   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_ActTimeSlot\n"));

   //safeguard only
   if (trx>=OAMrm_MAX_TRXS||slot>=8)
   {   
       IDEBUG__(("WARNING@rm_ActTimeSlot: invalid trx=%d,slot=%d\n",
                  trx,slot));
       return;
   }
  
   //Activate the given trx and slot
   rm_SendSlotActivateToTrx(trx,slot);
} 

//Pass trx all needed parameters
void rm_PassOpParamsToTrx(u8 trx)
{
     u8 i, ncellNo;

     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_PassOpParamsToTrx '%d'\n",trx));

     //Pass system information messages
     rm_SendAllSystemInfosToBcchTrx(trx);
     rm_SendSacchFillingSI5(trx, RM_L1SYSINFO_TYPE_5);	    	    
     rm_SendSacchFillingSI6(trx, RM_L1SYSINFO_TYPE_6);	    	    
     rm_OamSendPcToDsp(trx);
     rm_OamSendHoSCellToDsp(trx);
     ncellNo=rm_OamGetTotalAvailableHoNCells();

     for (i=0;i<OAMrm_HO_ADJCELL_NO_MAX;i++)
 	    rm_OamSendHoNCellToDsp(trx,i,ncellNo);

     rm_OamSendPwrRedStepToDsp(trx);
     rm_SendSynchInfoToBcchTrx(trx);
     rm_SendBtsPackageToTrx(trx);
}

void rm_SendBtsPackageToTrx(u8 trx)
{
    s16        length;            //Length of the message to send to L1
    rm_ItcTxMsg_t   msgToL1;      //Store an encoded message sent to L1
    s16 tmp;
        
    //Monitoring entrance to a function
    RDEBUG__(("ENTER@rm_SendBtsPackageToTrx: Entering...\n" ));
	
    //clear the memory first
    memset(&msgToL1, 0, sizeof(rm_ItcTxMsg_t));
    length = 0;
	
    //Encode BtsPackage message
    msgToL1.buffer[length++] = (unsigned char ) OAM_MANAGE_MSG;          // message discriminator 
    msgToL1.buffer[length++] = (unsigned char ) (OAM_BTS_PACKAGE >>8);  // high byte of msg type
    msgToL1.buffer[length++] = (unsigned char ) OAM_BTS_PACKAGE;        // low byte of msg type
    msgToL1.buffer[length++] = trx;                                      // trx number

    //Parameters contained in this message
    msgToL1.buffer[length++] = OAMrm_RADIO_LINK_TIMEOUT;

    //PR1219
    msgToL1.buffer[length++] = OAMrm_BS_PA_MFRMS;

    //PR1381 BEGIN
    if ( ((int)OAMrm_MS_UPLINK_DTX_STATE==1) ||
         ((int)OAMrm_MS_UPLINK_DTX_STATE==0)  ) 
          msgToL1.buffer[length] |= 1; //DTX ul ON
    //PR1381 END

    if (trx && OAMrm_MS_DNLINK_DTX_STATE)
         msgToL1.buffer[length] |= 2;        //DTX dl ON
    length++;

    msgToL1.msgLength = length;

    //Hex Dump the message if __TDUMP__ defined 
    //TDUMP__((TXMSG__, msgToL1.buffer, length));

    //Send message to DSP
    api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
    
} //rm_SendBtsPackageToTrx()


//Set all rm channels from a trx to given state
void rm_SetAllRmTrxChnOpState(u8 trx, u8 state )
{
     int i,j;

     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_SetAllRmTrxChnOpState '%d'\n",trx));

     //Change usable state of any of all tchf channels of trx
     for (i=0; i<8; i++)
     for (j=0; j<OAMrm_MAX_TCHFS; j++)
     {    //Scan through all tchf channel table
          if (rm_PhyChanTchf[j].trxNumber==trx &&
              (rm_PhyChanTchf[j].chanNumberMSB&0x07)==i)
          {    //Found one rm channel belong to trx
               if (rm_PhyChanTchf[i].usable!=state &&
                   state==RM_PHYCHAN_UNUSABLE)
               {
                   rm_PhyChanTchf[i].usable==state;
                   rm_NoOfUsableTchf--;
               } 
               break;
          }
     }
   
     //Change usable state of any of all sdcch4 channels of trx
     if (!trx)
     {
         for (i=0;i<4;i++)
              rm_PhyChanSdcch4[i].usable==state;
     }
}

#endif //#if 0
//GP2

//Process trx watchdog expiry
void rm_ProcTrxWDogExp(u8 trx)
{
     int oneTrx,i;

     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_ProcTrxWDogExp\n"));
 
     //Check if trx is still alive
     if (rm_TrxMgmt[trx].alive)
     {   
         //trx is still alive. Ping it!
         rm_TrxMgmt[trx].alive = RM_FALSE;
         rm_SendPingToTrx(trx);
         rm_StartTimer(trx,RM_TWDOG);
         return;
     } else
     {
         //trx not responding, raise an alarm!
         rm_SetRmOpState(rm_DspNotResponding[trx]);
         EDEBUG__(("ERROR@rm_ProcTrxWDog: trx '%d' not responding\n",trx));
     }
         
     return;

     //Autorecovery logic from trx failed to be laid here in the future 
}

//Set RM's internal and OAM operation state
void rm_SetRmOpState(Alarm_errorCode_t code)
{    //0 opStateEnabled, otherwise report an alarm

     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_SetRmOpState\n"));

     PDEBUG__(("INFO@rm_SetRmOpState: alarmCode=%d\n",code));

     if (code != EC_NOERROR)
     {
         if (JC_OK!=alarm_raise(MODULE_RM, MIB_errorCode_RM, code))
         {
             //This is very critical, should abort RM task here <TBD>!!!
             EDEBUG__(("ERROR@rm_SetRmOpState: \"alarm_raise(%d, %d)\" failed\n",
                 MODULE_RM,code));
         }
     } else {
         if (JC_OK!=alarm_clear(MODULE_RM, MIB_errorCode_RM, EC_NOERROR))
         {
             //This is very critical, should abort RM task here <TBD>!!!
             EDEBUG__(("ERROR@rm_SetRmOpState: \"alarm_clear(%d, %d)\" failed\n",
                 MODULE_RM,code));
         }
     }

     //Track own alarm
     rm_OpState = code;

     if (code < EC_RM_INFO_BASE) 
     {
         //Sync with btsOpState which goes disabled for a critical code
         OAMrm_BTS_OPSTATE = opStateDisabled;
     } 
}

//Send ping message to trx
void rm_SendPingToTrx(u8 trx)
{
     u16		  length;	 //length of sent message
     rm_ItcTxMsg_t  msgToL1;   //holder of sent message
    
     //Monitoring entrance to a function
     RDEBUG__(("ENTER-rm_SendPingToTrx '%d'\n",trx));

     //Form ping message
     length = 0;
     msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;
     msgToL1.buffer[length++] = RM_L1MT_PING_MSB;
     msgToL1.buffer[length++] = RM_L1MT_PING_LSB;
     msgToL1.buffer[length++] = trx;

     msgToL1.msgLength = length;
 
     api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
}

#endif //__RM_INITALL_CPP__
