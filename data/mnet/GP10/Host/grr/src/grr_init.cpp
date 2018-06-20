/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_INIT_CPP__
#define __GRR_INIT_CPP__

#include "grr\grr_head.h"

int grr_InitWatchTimerExpired(int trx)
{
  DBG_FUNC("grr_InitWatchTimerExpired", GRR_LAYER_INIT);
  DBG_ENTER();
  
  DBG_ERROR("grr_InitWatchTimerExpired: DSP(%d) failed to respond during initialization\n", trx);
  printf("grr_InitWatchTimerExpired: DSP(%d) failed to respond during initialization\n", trx);

  grr_SubmitAlarm(grr_DspInitWatchTimerExpiredAlarmCode[trx]);

  DBG_LEAVE();
  assert(0);

}

void grr_InitWatchTimerStop(unsigned char trx)
{
  DBG_FUNC("grr_InitWatchTimerStop", GRR_LAYER_INIT);
  DBG_ENTER();
 
  wdCancel(grr_TrxMgmt[trx].wdog.timer_id);
  
  DBG_LEAVE();
}

void grr_InitWatchTimerStart(int trx)
{
  WDOG_ID       timer_id;
  
  DBG_FUNC("grr_InitWatchTimerStart", GRR_LAYER_INIT);
  DBG_ENTER();

  //Create watchdog to monitir init
  if ( (timer_id=wdCreate()) == NULL )
  {
        grr_SubmitAlarm(EC_GRR_OS_CALL_WDCREATE_FAILED);
        DBG_ERROR("grr_InitWatchTimerStart:failed to create initWatchTimer for trx(%d)\n", trx);
        printf("grr_InitWatchTimerStart:failed to create initWatchTimer for trx(%d)\n", trx);
        assert(0);
  }

  if ( ERROR == (wdStart(timer_id, GRR_INIT_WATCH_TIME,(FUNCPTR)grr_InitWatchTimerExpired, 
                         trx)) )
  {
        grr_SubmitAlarm(EC_GRR_OS_CALL_WDSTART_FAILED);
        DBG_ERROR("grr_InitWatchTimerStart:failed to call wdstart for trx(%d) initWatchTimer\n", trx);
        printf("grr_InitWatchTimerStart:failed to call wdstart for trx(%d) initWatchTimer\n", trx);
        assert(0);
  }
  
  grr_TrxMgmt[trx].wdog.timer_id = timer_id;

  DBG_LEAVE();
}

bool grr_InitDspOver(unsigned char trx)
{
  
  //Check if DSP initialization done
  return ( (grr_TrxMgmt[trx].state==GRR_TRXMGMT_S3_TRXCONFIG_END) ? true:false);

}

void grr_InitDspMsgProc(unsigned char trx)
{
   unsigned char slot;

   DBG_FUNC("grr_InitDspMsgProc", GRR_LAYER_INIT);
   DBG_ENTER();
 
   //Process in terms of trx state
   slot  = grr_pItcRxDspMsg->buffer[4];

   switch ( grr_TrxMgmt[trx].state )
   {
   case GRR_TRXMGMT_S0_BOOTSTRAP_IND:
        //
        //Waiting for dsp bootstrap indication
        if (!GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_BOOTSTRAPCNF_MSB,GRR_L1MT_BOOTSTRAPCNF_LSB))
        {  
            //Got unexpected message
            DBG_WARNING("grr_InitDspMsgProc: expecting DspBootstrapInd but recved msg(%d,%d,%d,%d)\n",
                         grr_pItcRxDspMsg->buffer[0],
                         grr_pItcRxDspMsg->buffer[1],
	                 grr_pItcRxDspMsg->buffer[2],
	                 grr_pItcRxDspMsg->buffer[3]);
            return;
        }

        //Process the received msg
        grr_SendDspTrxConfig(trx);
        ConfigTxPwrValues(trx);
        grr_SetOamTrxOpState(trx,opStateDisabled);
        grr_SetOamAllTrxSlotOpState(trx, opStateDisabled);
        grr_TrxMgmt[trx].state=GRR_TRXMGMT_S1_TRXCONFIG_ACK;
        break;

   case GRR_TRXMGMT_S1_TRXCONFIG_ACK:
        //
        //Waiting for trx config ack 
        if (!GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_TRXCONFIGACK_MSB,
                          GRR_L1MT_TRXCONFIGACK_LSB))
        {   
            //Got unexpected message
            DBG_WARNING("grr_InitDspMsgProc: expecting trxConigAck but recved msg(%d,%d,%d,%d)\n",
                         grr_pItcRxDspMsg->buffer[0],
                         grr_pItcRxDspMsg->buffer[1],
	                 grr_pItcRxDspMsg->buffer[2],
	                 grr_pItcRxDspMsg->buffer[3]);
            return;
        }
   
        //Trx config acked, then go init its TSs
        grr_SetOamTrxOpState(trx, opStateEnabled);
        grr_TrxMgmt[trx].state=GRR_TRXMGMT_S2_SLOTACTIV_ACK;
        grr_TrxMgmt[trx].slot=0;
        grr_ActTrxSlot(trx,grr_TrxMgmt[trx].slot); 
        break;

   case GRR_TRXMGMT_S2_SLOTACTIV_ACK:
        //
        //Apply sanitory Check here
        if ( (grr_pItcRxDspMsg->buffer[4] != grr_TrxMgmt[trx].slot) ||
             (grr_pItcRxDspMsg->buffer[4] >= 8)			    )
        {
            DBG_WARNING("grr_InitDspMsgProc: expecting trx(%d) slotAck for slot(%d) not slot(%d)\n",
                         trx, grr_TrxMgmt[trx].slot, grr_pItcRxDspMsg->buffer[4]);
            return;
        }
 
        //check if got the expected msg
        if (GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_SLOTACTIVACK_MSB,
                         GRR_L1MT_SLOTACTIVACK_LSB))
        {   
            DBG_TRACE("grr_InitDspMsgProc: slotAct ack to trx(%d) slot(%d)\n",
                       grr_pItcRxDspMsg->buffer[3],
                       grr_pItcRxDspMsg->buffer[4]);

            //Got positive ack, (trx,slot) goes into enabled state
            grr_SetOamTrxSlotOpState(trx,grr_pItcRxDspMsg->buffer[4],opStateEnabled);


        } else if (GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_SLOTACTIVNACK_MSB,
                   GRR_L1MT_SLOTACTIVNACK_LSB))
        {   
            DBG_WARNING("grr_InitDspMsgProc: slotAct nack to trx(%d) slot(%d)\n",
                         grr_pItcRxDspMsg->buffer[3],
                         grr_pItcRxDspMsg->buffer[4]);

            //Got negative ack, (trx,slot) goes into disabled state
            grr_SubmitAlarm(grr_TrxSlotActNackAlarmCode[trx][slot]);
            grr_SetOamTrxSlotOpState(trx, grr_pItcRxDspMsg->buffer[4], opStateDisabled);

        } else
        {   
            //Got unexpected message
            DBG_WARNING("grr_InitDspMsgProc: expecting slotAck/Nack to trx(%d) slot(%d) but recved msg(%d,%d,%d,%d)\n",
                         trx,
                         grr_TrxMgmt[trx].slot,
                         grr_pItcRxDspMsg->buffer[0],
                         grr_pItcRxDspMsg->buffer[1],
	                 grr_pItcRxDspMsg->buffer[2],
	                 grr_pItcRxDspMsg->buffer[3]);
            return;
        }

        //Check if all TSs of trx are initialized
        if (grr_pItcRxDspMsg->buffer[4]==7)
        {  
            grr_TrxMgmt[trx].state = GRR_TRXMGMT_S3_TRXCONFIG_END;
        }
        else 
        {
            //Activate next time slot of this trx
            grr_TrxMgmt[trx].slot++;
            grr_ActTrxSlot(trx, grr_TrxMgmt[trx].slot);
        } 
        break;

   case GRR_TRXMGMT_S3_TRXCONFIG_END:
   default:
        //Unrecognized trx state
        DBG_WARNING("grr_InitDspMsgProc: invalid state(%d) for trx(%d)\n",
                     grr_TrxMgmt[trx].state, trx);
        break;
   }
}

void grr_InitTrxMgmtData(unsigned char trx)
{
  DBG_FUNC("grr_InitTrxMgmtData", GRR_LAYER_INIT);
  DBG_ENTER();

  grr_TrxMgmt[trx].state	= 0;
  grr_TrxMgmt[trx].slot       = 0;
  grr_TrxMgmt[trx].tdma1      = 0;
  grr_TrxMgmt[trx].tdma2      = 0;
  grr_TrxMgmt[trx].tdma3      = 0;
  grr_TrxMgmt[trx].tdma4      = 0;
  grr_TrxMgmt[trx].done       = false;
  grr_TrxMgmt[trx].alive      = true;

  DBG_LEAVE();
}

//Initialize trx package
void grr_InitOamTrxPkg(unsigned char trx)
{
  int length;

  DBG_FUNC("grr_InitOamTrxPkg", GRR_LAYER_INIT);
  DBG_ENTER();
  
  //Reset operational state of all TRX-owned resources
  grr_SetOamTrxOpState(trx, opStateDisabled);
  grr_SetOamAllTrxSlotOpState(trx, opStateDisabled);    
  grr_SetOamTrxRcOpState(trx, opStateDisabled);

  //Starting initializing trx and set its SM state to S0
  DBG_TRACE("GRR starting the initialization of trx(%d)\n", trx);
  printf("GRR starting the initialization of trx(%d)\n", trx);

  //Initialize trx init management data
  grr_InitTrxMgmtData(trx);

  //Start init watch timer
  grr_InitWatchTimerStart(trx);

  //Configure trx and wait for its ack
  grr_SendDspTrxConfig(trx);
  ConfigTxPwrValues(trx);
  grr_TrxMgmt[trx].state=GRR_TRXMGMT_S1_TRXCONFIG_ACK;

  //Init Loop
  while (true)
  {
       // Retrieve a message from Q
       if ( (length = msgQReceive(grr_MsgQId,(char*)&grr_ItcRxMsg, sizeof(grr_ItcRxMsg_t), WAIT_FOREVER))
             == ERROR )
       {
             //failed in calling vxWorks receive call. Underlying issue. Abort the task!
             grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQRECEIVE_FAILED);
             DBG_ERROR("grr_InitOamTrxPkg(%d): failed to read VxWorks GRR Queue.\n",trx);
             printf("grr_InitOamTrxPkg(%d): failed to read VxWorks GRR msg Queue\n",trx);
             DBG_LEAVE();
             assert(0);
       } else 
       {
             //Good receive! If needed dump out for debug purpose
             if (grr_DbgDump) DBG_HEXDUMP(( unsigned char *)&grr_ItcRxMsg, length);

             //Handle the message in terms of its origin, do sanitory check too!
             grr_ItcRxOrigin = grr_ItcRxMsg.module_id;

             if (grr_ItcRxOrigin == MODULE_L1)
             {
                 //Received L1 message
                 grr_pItcRxDspMsg=(grr_ItcRxDspMsg_t *)&grr_ItcRxMsg.itcRxDspMsg;

                 //If needed, output trace for debugging purpose
                 DBG_TRACE("grr_InitOamTrxPkg(%d): received L1 MSG(%x,%x,%x) SRC(%x,%x,%x)\n",
                            trx,
                            grr_pItcRxDspMsg->buffer[0],
                            grr_pItcRxDspMsg->buffer[1],
	                      grr_pItcRxDspMsg->buffer[2],
	                      grr_pItcRxDspMsg->buffer[3],
		                grr_pItcRxDspMsg->buffer[4],
		                grr_pItcRxDspMsg->buffer[5]);
                   
                 //Carry on DSP init
                 grr_InitDspMsgProc(trx);

                 if ( grr_InitDspOver(trx) ) break;
       
              } else
              {           
                 DBG_WARNING("grr_InitOamTrxPkg(%d): unexpected module origin(%d)\n",
				      trx, grr_ItcRxOrigin);
              }
        }
  }

  grr_InitWatchTimerStop(trx);

  DBG_TRACE("GRR finished the initialization of trx(%d)\n", trx);
  printf("GRR finished the initialization of trx(%d)\n", trx);

  DBG_LEAVE();
}

void grr_InitDsp(void)
{
  int length;

  DBG_FUNC("grr_InitDsp", GRR_LAYER_INIT);
  DBG_ENTER();

  DBG_TRACE("GRR starting DSP initializatoin (swap=%d)\n",
             OAMgrr_TRX_SWAP_SETTING);
  printf("GRR starting DSP initializatoin (swap=%d)\n",
             OAMgrr_TRX_SWAP_SETTING);

  if (OAMgrr_TRX_SWAP_SETTING==1)   //Swap two TRXs
      l1pTrxSwap(0,1);

  grr_InitWaitForDspsUpSignal();

  //Decide if I-Q swap needed
  OAMgrr_IQ_SWAP = drfIQSwapGet();

  //3a. FreqHopDsp0Value = drfFreqHopGet(0)  -- This function returns a True/False value
  //3b. FreqHopDsp1Value = drfFreqHopGet(1)  -- This function returns a True/False value
  //Decide if hopping cable. If not, turn off the hopping feature. Assumed not capable now.
  OAMgrr_HOPPING_CTRLa = 0;

  //Init all trxs one by one
  for (int i=0;i<OAMgrr_MAX_TRXS;i++)
  {
       grr_InitOamTrxPkg(i);
       grr_PassOamParams(i);
       grr_SendDspSynInf(i);
  }

  DBG_TRACE("GRR finished DSP initializatoin (swap=%d)\n",
             OAMgrr_TRX_SWAP_SETTING);
  printf("GRR finished DSP initializatoin (swap=%d)\n",
             OAMgrr_TRX_SWAP_SETTING);

  DBG_LEAVE();
}

void grr_InitWaitForRlcRrmGoAck(void)
{
  int length;

  DBG_FUNC("grr_InitWaitForRlcRrmGoAck", GRR_LAYER_INIT);
  DBG_ENTER();

  bool waitForRlcGoAck=true;
  bool waitForRrmGoAck=true;
  
  while(waitForRlcGoAck||waitForRrmGoAck)
  {  
       int length;

       // Retrieve a message from Q
       if ( (length = msgQReceive(grr_MsgQId,(char*)&grr_ItcRxMsg, sizeof(grr_ItcRxMsg_t), WAIT_FOREVER))
             == ERROR )
       {
             //failed in calling vxWorks msgQReceive(). Abort the task!
             grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQRECEIVE_FAILED);
             DBG_ERROR("grr_InitWaitForRlcRrmGoAck: failed to read VxWorks GRR Queue.\n");
             printf("grr_InitWaitForRlcRrmGoAck: failed to read VxWorks GRR msg Queue\n");
             DBG_LEAVE();
             assert(0);
       } else 
       {
             //Good receive! If needed dump for debug purpose
             if (grr_DbgDump) DBG_HEXDUMP((unsigned char*)&grr_ItcRxMsg, length);

             //Handle the message in terms of its origin, do sanitory check too!
             grr_ItcRxOrigin = grr_ItcRxMsg.module_id;

             if (grr_ItcRxOrigin == MODULE_RLCMAC)
             {
                 grr_pItcRxRlcMsg=(grr_ItcRxRlcMsg_t *)&grr_ItcRxMsg.itcRxRlcMsg;

                 //If needed, output trace for debugging purpose
                 DBG_TRACE("grr_InitWaitForRlcRrmGoAck: received a RLC msg (%d)\n", 
                            grr_pItcRxRlcMsg->message_type);
                 if (grr_pItcRxRlcMsg->message_type == GRR_READY_TO_GO_ACK)
                     waitForRlcGoAck = false;

             } else if (grr_ItcRxOrigin == MODULE_RM)
             {
                 grr_pItcRxRrmMsg=(grr_ItcRxRrmMsg_t *)&grr_ItcRxMsg.itcRxRrmMsg;
                 DBG_TRACE("grr_InitWaitForRlcRrmGoAck: received a RRM msg (%d)\n", 
                            grr_pItcRxRrmMsg->message_type);

                 if (grr_pItcRxRrmMsg->message_type == GRR_READY_TO_GO_ACK)
                     waitForRrmGoAck = false;
             } else
             {           
                 DBG_WARNING("grr_InitWaitForRlcRrmGoAck: unexpected module origin(%d)\n",
                              grr_ItcRxOrigin);
             }
        }
  }
  DBG_LEAVE();
}

void grr_InitOam(void)
{
  DBG_FUNC("grr_InitOam", GRR_LAYER_INIT);
  DBG_ENTER();

  memset(&grr_OamData, 0, sizeof(grr_OamData));

  grr_GetOamData();
  grr_FixOamData();
  grr_RegOamTrap();

  DBG_LEAVE();
}

bool grr_GotAllDspsUpSignals(char *dsp)
{
   int i;

   DBG_FUNC("grr_GotAllDspsUpSignals", GRR_LAYER_INIT);
   DBG_ENTER();

   for (i=0;i<OAMgrr_MAX_TRXS;i++)
       if (!dsp[i]) break;

   DBG_LEAVE();
   return ((i>=OAMgrr_MAX_TRXS)?true:false);
}

void grr_InitWaitForDspsUpSignal(void)
{
  int length;
  char dsp[OAMgrr_MAX_TRXS];

  DBG_FUNC("grr_InitWaitForDspsUpSignal", GRR_LAYER_INIT);
  DBG_ENTER();

  memset(dsp, 0, OAMgrr_MAX_TRXS);

  while(!grr_GotAllDspsUpSignals(dsp))
  {  
       int length;

       // Retrieve a message from Q
       if ( (length = msgQReceive(grr_MsgQId,(char*)&grr_ItcRxMsg, sizeof(grr_ItcRxMsg_t), WAIT_FOREVER))
             == ERROR )
       {
             //failed in calling vxWorks msgQReceive(). Abort the task!
             grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQRECEIVE_FAILED);
             DBG_ERROR("grr_InitWaitForDspsUpSignal: failed to read VxWorks GRR Queue.\n");
             printf("grr_InitWaitForDspsUpSignal: failed to read VxWorks GRR msg Queue\n");
             DBG_LEAVE();
             assert(0);
       } else 
       {
             //Good receive! If needed dump for debug purpose
             if (grr_DbgDump) DBG_HEXDUMP((unsigned char*)&grr_ItcRxMsg, length);

             //Handle the message in terms of its origin, do sanitory check too!
             grr_ItcRxOrigin = grr_ItcRxMsg.module_id;

             if (grr_ItcRxOrigin != MODULE_L1)
             {
                 DBG_WARNING("grr_InitWaitForDspsUpSignal: unexpected module origin(%d)\n",
                              grr_ItcRxOrigin);
             } else
             {
                 grr_pItcRxDspMsg=(grr_ItcRxDspMsg_t *)&grr_ItcRxMsg.itcRxDspMsg;

                 //If needed, output trace for debugging purpose
                 DBG_TRACE("grr_InitWaitForDspsUpSignal: recv DSP msg(%x,%x,%x)) src(%x,%x,%x)\n", 
                            grr_pItcRxDspMsg->buffer[0],
                            grr_pItcRxDspMsg->buffer[1],
                            grr_pItcRxDspMsg->buffer[2],
                            grr_pItcRxDspMsg->buffer[3],
                            grr_pItcRxDspMsg->buffer[4],
                            grr_pItcRxDspMsg->buffer[5]);
            
                 if (GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_BOOTSTRAPCNF_MSB,GRR_L1MT_BOOTSTRAPCNF_LSB))
                 {
                     if ( (grr_pItcRxDspMsg->buffer[3]<OAMgrr_MAX_TRXS) && !(dsp[grr_pItcRxDspMsg->buffer[3]]))
					 {
 		                  dsp[grr_pItcRxDspMsg->buffer[3]] = 1;
						  grr_DspCiphCap = grr_pItcRxDspMsg->buffer[4];

					 } else
                          DBG_WARNING("grr_InitWaitForDspsUpSignal:dsp(%d) and its bootstrap status(%d)\n",
                        	           grr_pItcRxDspMsg->buffer[3], dsp[grr_pItcRxDspMsg->buffer[3]]);
                 } else
                 {
                     DBG_WARNING("grr_InitWaitForDspsUpSignal: unexpected msg(%x,%x,%x) src(%x,%x,%x)\n",
                            grr_pItcRxDspMsg->buffer[0],
                            grr_pItcRxDspMsg->buffer[1],
                            grr_pItcRxDspMsg->buffer[2],
                            grr_pItcRxDspMsg->buffer[3],
                            grr_pItcRxDspMsg->buffer[4],
                            grr_pItcRxDspMsg->buffer[5]);
                 }

             }
        }
  }
  DBG_LEAVE();
}

void grr_Init(void)
{
  JC_STATUS jcRet;
  unsigned char i;

  DBG_FUNC("grr_Init", GRR_LAYER_INIT);
  DBG_ENTER();

  //Starting initialization.

  //Block other GP modules
  grr_SubmitAlarm(EC_GRR_BOOTING_UP);

  grr_InitOam();
  grr_InitDsp();
  
  //Signify to allow RRM and RLC go
  //grr_SendReadyToGoReq(MODULE_GRR, rlcMsgQId);
  if (JC_OK != (jcRet=RlcMacGrrReadyToGo()) )
  {
     DBG_ERROR("grr_Init: RlcMacGrrReadyToGo ret code(%d)\n", jcRet);
     assert(0);
  }

  grr_SendReadyToGoReq(MODULE_GRR, rm_MsgQId);

  //Wait for RRM & RLC go cnf
  grr_InitWaitForRlcRrmGoAck();

  //Turn on radio if needed
  grr_FirstRadioCtrl();

  //Unblock other GP modules
  grr_SubmitAlarm(EC_NOERROR);     	  

  //Initialization is done.

  DBG_LEAVE();
}

#endif //__GRR_INIT_CPP__
