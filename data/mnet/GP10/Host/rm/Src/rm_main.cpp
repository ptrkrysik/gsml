/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_MAIN_CPP__
#define __RM_MAIN_CPP__

#include "rm\rm_head.h"
#include "Os\JCTask.h"

void rm_VcToVbUpNotification(void);

JCTask rmTaskObj("rmTask");

void rm_Main( void )
{
   int i;
   int result; //Result from calling a func
   u8  trx;

   //Monitoring entry to RM moule
   //RDEBUG__(("ENTER-rm_Main()\n"));
   RMDBG_ENTER(("ENTER@rm_Main()",RM_LAYER));

   //GP2
   // Let root task know that RM has entered its process loop.
   //ViperTaskEnterLoop();
   rmTaskObj.JCTaskEnterLoop();

   //InitRMsef
   rm_InitAll();
   printf("RM finished initialization\n");

   //MainLoop
   while(RM_TRUE) 
   {	
      //Confirm RM is now staying in its main-loop
      PDEBUG__(("RM_MAIN: staying in WHILE-LOOP\n"));

      //Read a msg from queue or pend on arrival of message
      result = rm_msgQReceive(rm_MsgQId, (s8 *)&rm_ItcRxMsg, 
                              sizeof(rm_ItcRxMsg_t), WAIT_FOREVER);
      if (result==ERROR)
      {   //Error in reading VxWork msg queue. Ignore msg!
          rm_SetRmOpState(EC_RM_OS_CALL_MSGQRECEIVE_FAILED);
          EDEBUG__(("ERROR@rm_Main:VxWorks Q read err:%d\n",errno));
          goto L1;
      } 
      
 	//Good receive! Dump it if needed,result=length
      //BDUMP__((RXMSG__, (u8 *)&rm_ItcRxMsg, result));

      //Handle the message in terms of origin, apply sanitory check!
      rm_ItcRxOrigin = rm_ItcRxMsg.module_id;
      if (rm_ItcRxOrigin>=MNET_MAX_MODULE_IDS)  //PR1512
      {
          IDEBUG__(("WARNING@rm_Main(): invalid moduleID=%d\n", rm_ItcRxOrigin));
          goto L1;
      }

      switch(rm_ItcRxOrigin)
      {
      //GP2
      case MODULE_GRR:
           //Received GRR message, cast it to GRR msg struct and process it
           rm_pItcRxGrrMsg = (rm_ItcRxGrrMsg_t *)&rm_ItcRxMsg;

           //If needed output the fact RM got an OAM message for trace
           PDEBUG__(("RM<==GRR@rm_Main: msgTyp(%d) trx(%d) slot(%d) ctrl(%d)\n",
                      rm_pItcRxGrrMsg->message_type,
                      rm_pItcRxGrrMsg->trx,
                      rm_pItcRxGrrMsg->slot,
                      rm_pItcRxGrrMsg->feat_ctrl));

           rm_ProcGrrMsgs();

           goto L1;

      case MODULE_OAM:
           //GP2
           goto L1;

           //Received OAM message, cast it to OAMmsg struct and process it
           rm_pItcRxOamMsg = (rm_ItcRxOamMsg_t *)&rm_ItcRxMsg;
	     rm_OamGetTrapVars();

           //If needed output the fact RM got an OAM message for trace
	     PDEBUG__(("RM<==OAM@rm_Main: msgTyp=%d,Tag=0x%x,valTyp=%d, val=%d\n",
                     rm_pItcRxOamMsg->msgType,
			   rm_pItcRxOamMsg->mibTag,
			   rm_pItcRxOamMsg->valType,
			   rm_pItcRxOamMsg->val.varVal));
           goto L1;

      case MODULE_L1:
           //Received L1 message,cast it to L1msg struct and process
	     rm_pItcRxL1Msg=(rm_ItcRxL1Msg_t *)&rm_ItcRxMsg.itcRxMdMsg;

           //If needed output the fact RM got a L1 message for trace
           PDEBUG__(("RM<==L1@rm_Main:MSG=(%x,%x,%x),SRC(%x,%x,%x)\n",
                      rm_pItcRxL1Msg->l3_data.buffer[0],
                      rm_pItcRxL1Msg->l3_data.buffer[1],
	                rm_pItcRxL1Msg->l3_data.buffer[2],
	                rm_pItcRxL1Msg->l3_data.buffer[3],
		          rm_pItcRxL1Msg->l3_data.buffer[4],
		          rm_pItcRxL1Msg->l3_data.buffer[6]));
                      
           trx = rm_pItcRxL1Msg->l3_data.buffer[3];
           //Apply sanitory check!
           if (trx>=OAMrm_MAX_TRXS)
           {
               IDEBUG__(("WARNING@rm_Main(): invalid trx=%d\n", trx));
               goto L1;
           }

           rm_TrxMgmt[trx].alive = RM_TRUE; //dsp still alive

           //Process the message from the alive trx
           switch(rm_pItcRxL1Msg->l3_data.buffer[0])
           {
           case RM_L1MG_DEDMGMT:
                //Received message is for dedicated link mgmt
	          rm_ItcRxPrim = (rm_pItcRxL1Msg->module_id<<8) |
	           	             (rm_pItcRxL1Msg->primitive_type);
		    rm_ItcRxEntryId = rm_GetIrtEntryId(rm_pItcRxL1Msg);
		    if (rm_ItcRxEntryId==CNI_IRT_NULL_ID)
		    {   
                    //the received message not linked to a valid entry ID
                    //Check if it's RTS for PAGCH
                    if (rm_pItcRxL1Msg->l3_data.buffer[4]!=0x90)
                    {
                        rm_SetRmOpState(EC_RM_RECV_CORRUPTED_DSP_MSG);
		            IDEBUG__(("WARNING@rm_Main: Null entry ID for L1 msg\n"));
                    } 
		        goto L1;
		    }
                rm_ItcRxPrimType = rm_pItcRxL1Msg->l3_data.buffer[0]; //MD
                rm_ItcRxMsgType  = (u32)( (rm_pItcRxL1Msg->l3_data.buffer[1]<<8) |
                                           rm_pItcRxL1Msg->l3_data.buffer[2]     );
		    break;                 

           case RM_L1MG_TRXMGMT:
                //GP2
                goto L1;

                //Received message is for trx management
                
                //Got PONG message for keep-alive watching
                if (RM_L1MSGCMP(RM_L1MG_TRXMGMT,RM_L1MT_PINGACK_MSB,
                                RM_L1MT_PINGACK_LSB))
                {
                    //Make sure DSP is still ticking now
                    rm_MakeSureDspIsTicking();
                    goto L1;
                }

                //Message for other TRX managment purpose
                if (rm_LiveSglCfg)
                {
                    //period of VC reboot
                    rm_ProcTrxMgmtMsg(trx);

                    //Check if configuration finished or not
                    for (i=0;i<OAMrm_MAX_TRXS;i++)
                    {
                         if ( ((OAMrm_TRX_ONOFF_MAP)&(1<<i)) && (!rm_TrxMgmt[i].done) )
                               break;
                    }
              
                    if (i>=OAMrm_MAX_TRXS)
                    {
                        //Done with initialization of DSPs, decide on/off RF
                        rm_TurnOnRadioIfNeeded();

                        //Block OnTheFly config
                        rm_LiveSglCfg = RM_FALSE;

                        //Activate CBCHchannel
                        rm_ChanCbchActive=RM_FALSE;
                        if ( OAMrm_CBCH_OPSTATE )
                        { 
                             if (rm_pCbchChan->usable != RM_PHYCHAN_USABLE )
                             {
                                 //SDCCH4 ie CBCH is not usable 
                                 rm_SetRmOpState(EC_RM_CBCH_UNUSABLE);
                                 IDEBUG__(("WARNING@rm_Main:no tchf available\n"));
                             } else
                             {   
                                 rm_pCbchChan->usable = RM_PHYCHAN_CBCH_USABLE;
                                 rm_ActivateCbchChan();
                             }
                        }
                        rm_SetRmOpState(EC_NOERROR); 
                        PDEBUG__(("INFO@rm_Main:RM INIT DONE!!!\n")); 
				//add VCup notification to VB,SMSCBC, 6/29 
				rm_VcToVbUpNotification();
                    }
                } 
                //else
                //Add logic here for on-the-fly configuration in the future
                goto L1;

           default:
                //Received unexpected L1 message
	          PDEBUG__(("INFO@rm_Main: unexpected L11 msg md=%x\n",
	                     rm_pItcRxL1Msg->l3_data.buffer[0]));
                goto L1;
           } // inner switch() 
           break;

      case MODULE_RM:
           //Received RM message,cast it to RMmsg struct and process
	     rm_pItcRxRmMsg=(rm_ItcRxRmMsg_t *)&rm_ItcRxMsg.itcRxRmMsg;   
           rm_ItcRxPrim  =(rm_pItcRxRmMsg->module_id<<8   ) |
	          		  (rm_pItcRxRmMsg->primitive_type ) ;

	     rm_ItcRxEntryId = rm_pItcRxRmMsg->entry_id;

           //Process watch-dog timer expiry here
           if (rm_pItcRxRmMsg->timerId==RM_TWDOG)  
           {   //Watchdog expired, entryId=trx
               PDEBUG__(("TRACE@rm_Main: DSP watch dog expired\n"));
               rm_ProcTrxWDogExp(rm_ItcRxEntryId);
               goto L1;
           }

           //If needed output the fact RM got a RM message for trace
           PDEBUG__(("RM<==RM@rm_Main:prim=%d,entryID=%d,timerId=%d\n",
                      rm_pItcRxRmMsg->primitive_type,
                      rm_ItcRxEntryId,
                      rm_pItcRxRmMsg->timerId));

           rm_ItcRxPrimType = rm_pItcRxRmMsg->primitive_type; //primitive type
           rm_ItcRxMsgType  = (u32)rm_pItcRxRmMsg->timerId;   //timer expired

	     break;

      default:
           //Block call signaling if bts disabled
           if (OAMrm_BTS_OPSTATE==opStateDisabled)
           {   //RM is out of service due to external or internal constraint 
               PDEBUG__(("INFO@rm_Main: BTS out of service now, rmEcCode=%d\n",
                          rm_OpState));
               goto L1;
           }
            
           //RM is now operational and go ahead to process received message  
           switch(rm_ItcRxOrigin)
           {
	     case MODULE_MD:
                //Received MD message,cast it to MDmsg struct and process
 	          rm_pItcRxMdMsg=(rm_ItcRxMdMsg_t *)&(rm_ItcRxMsg.itcRxMdMsg);   
	          rm_ItcRxPrim  =(rm_pItcRxMdMsg->module_id<<8   ) |
	         		       (rm_pItcRxMdMsg->primitive_type ) ;
		    rm_ItcRxEntryId=rm_pItcRxMdMsg->entry_id; 
                rm_SmCtxt[rm_ItcRxEntryId].sapi=rm_pItcRxMdMsg->sapi;

                //If needed output the fact RM got a L1 message for trace
 		    PDEBUG__(("RM<==MD@rm_Main:prim=%d, entryID=%d\n",
  		               rm_pItcRxMdMsg->primitive_type,rm_ItcRxEntryId));

                //Distinguish RA between RACH and TCH
                if (rm_ItcRxEntryId!=rm_RachEntryId &&
                    rm_pItcRxMdMsg->primitive_type==L23PT_DL_RANDOM_ACCESS_IND)
                    rm_ItcRxPrim = DLm_OTHRA_IND;

                // Not displaying L23PT_DL_EST_IND primatives here because they
                // are displayed later by MM and CC.  Had to make an exception for
                // paging response because neither MM or CC display it.
                if (rm_pItcRxMdMsg->primitive_type == L23PT_DL_DATA_IND)
                    PostL3RecvMsLog(rm_pItcRxMdMsg->lapdm_oid, rm_pItcRxMdMsg->sapi,
                                    &rm_pItcRxMdMsg->l3_data);
                else if ((rm_pItcRxMdMsg->primitive_type == L23PT_DL_EST_IND) &&
                         (rm_pItcRxMdMsg->l3_data.buffer[1] == 
                                           CNI_RIL3RRM_MSGID_PAGING_RESPONSE))  
                    PostL3RecvMsLog(rm_pItcRxMdMsg->lapdm_oid, rm_pItcRxMdMsg->sapi,
                                    &rm_pItcRxMdMsg->l3_data);                        
                else if (rm_pItcRxMdMsg->primitive_type == L23PT_DL_RANDOM_ACCESS_IND)
                    PostL3RandomAccess(&rm_pItcRxMdMsg->l3_data);

                rm_ItcRxPrimType = rm_pItcRxMdMsg->primitive_type; //primitive type
                rm_ItcRxMsgType  = 0;					 //not applicable

		    break;

		 case MODULE_MM:
                //Received MM message,cast it to MMmsg struct and process
	          rm_pItcRxMmMsg=(rm_ItcRxMmMsg_t *)&rm_ItcRxMsg.itcRxMmMsg;   
	          rm_ItcRxPrim  =(rm_pItcRxMmMsg->module_id<<8   ) |
	            		 (rm_pItcRxMmMsg->primitive_type ) ;
		    rm_ItcRxEntryId = rm_pItcRxMmMsg->entry_id;
                //If needed output the fact RM got a L1 message 
                PDEBUG__(("RM<==MM@rm_Main:prim=%d,msgType=%d,entryID=%d\n",
                           rm_pItcRxMmMsg->primitive_type, 
                           rm_pItcRxMmMsg->message_type,
                           rm_ItcRxEntryId));

                rm_ItcRxPrimType = rm_pItcRxMmMsg->primitive_type; //primitive type
                rm_ItcRxMsgType  = rm_pItcRxMmMsg->message_type;	 //message type

		    break;

		 case MODULE_BSSGP:
              //Received BSSGP message, cast it to BSSGP msg struct and process
	          rm_pItcRxBgMsg=(rm_ItcRxBgMsg_t *)&rm_ItcRxMsg.itcRxBgMsg;   
	          rm_ItcRxPrim  =(rm_pItcRxBgMsg->module_id<<8   ) |
	            		 (rm_pItcRxBgMsg->primitive_type ) ;
		      rm_ItcRxEntryId = rm_pItcRxBgMsg->entry_id;

              //If needed output the fact RM got a L1 message 
              PDEBUG__(("RM<==BSSGP@rm_Main:prim=%d,msgType=%d,entryID=%d\n",
                         rm_pItcRxBgMsg->primitive_type, 
                         rm_pItcRxBgMsg->message_type,
                         rm_ItcRxEntryId));

              rm_ItcRxPrimType = rm_pItcRxBgMsg->primitive_type; //primitive type
              rm_ItcRxMsgType  = rm_pItcRxBgMsg->message_type;	 //message type

		    break;

	     case MODULE_CC:
                //Received CC message,cast it to CCmsg struct and process
	          rm_pItcRxCcMsg=(rm_ItcRxCcMsg_t *)&rm_ItcRxMsg.itcRxCcMsg;   
	          rm_ItcRxPrim  =(rm_pItcRxCcMsg->module_id<<8   ) |
	           		       (rm_pItcRxCcMsg->primitive_type ) ;
  	          rm_ItcRxEntryId = rm_pItcRxCcMsg->entry_id;
                //If needed output the fact RM got a L1 message 
                PDEBUG__(("RM<==CC@rm_Main:prim=%d,msgType=%d,entryID=%d\n",
                           rm_pItcRxCcMsg->primitive_type, 
                           rm_pItcRxCcMsg->message_type,
                           rm_ItcRxEntryId));

                rm_ItcRxPrimType = rm_pItcRxCcMsg->primitive_type; //primitive type
                rm_ItcRxMsgType  = rm_pItcRxCcMsg->message_type;	 //message type

 		    break;
	
           case MODULE_SMSCBC:
                //Received CB message,cast it to CBmsg struct and process
  	          rm_pItcRxVbMsg=(rm_ItcRxVbMsg_t *)&rm_ItcRxMsg.itcRxVbMsg;
                //If needed output the fact RM got a CB message for trace

                PDEBUG__(("RM<==CB@rm_Main: got CB msg, msgTyp=%d,size=%d\n",
		 	         rm_pItcRxVbMsg->message_type,
                           rm_pItcRxVbMsg->size));

                // Check if CBCH is on
                if (!rm_ChanCbchActive)
                {
                    //SMSCBTBD: CBCH is not active. Add alarm report here later !
                    IDEBUG__(("WARNING@rm_Main: CBCH is inactive, retry later!\n"));
			  goto L1;

                }
                rm_ProcSmscbcMsg();

                rm_ItcRxPrimType = 0; 					 //not applicable
                rm_ItcRxMsgType  = rm_pItcRxVbMsg->message_type;	 //message type
 
		    goto L1;
                
		    break;

           default:
                //Received msg is from unexpected source. Ignore it!
                PDEBUG__(("INFO@rm_Main:unexpected msg origin=%d\n",
                           rm_ItcRxMsg.module_id));
                goto L1;

           } //inner switch(rm_ItcRxOrigin)

           break;

       } //outer switch(rm_ItcRxOrigin)

       //Handle it!
       rm_Despatch();
 
   L1: continue;

   } //while(RM_TRUE)

   rmTaskObj.JCTaskNormExit();
} //rm_Main()

//extern int	ta_HoldSdcch;
//int ta_HoldFacch = 0;
//int taHoldFacchOn(void);
//int taHoldFacchOff(void);

//int taHoldFacchOn(void)
//{
//    ta_HoldSdcch = 1;
//    ta_HoldFacch = 1;
//    printf("[RM] HoldFacch Mode is now turned on.\n");
//}

//int taHoldFacchOff(void)
//{
//    ta_HoldSdcch = 0;
//    ta_HoldFacch = 0;
//    printf("[RM] HoldFacch Mode is now turned off.\n");
//}

void rm_TurnOnRadioIfNeeded(void)
{
//G2
return;
#if 0

     int i,j;

     PDEBUG__(("INFO@rm_TurnOnRadioIfNeeded()\n"));
    
     //Check if there is any usable traffic channels
     if (!rm_NoOfUsableTchf)
     {  
         //No tchfs available,nonsense!
         rm_SetRmOpState(EC_RM_NOTCHFS_USABLE);
         IDEBUG__(("WARNING@rm_Main:no tchf available\n"));
     }
 
     //Check if pic unit is working properly
     //put here later!

     //Logic for safety check to solve any inconsistency
     if ( OAMrm_BTS_AMSTATE != unlocked      ||
          OAMrm_TRX_AMSTATE(0) != unlocked   ||
          OAMrm_CHN_AMSTATE(0,0) != unlocked )
     {
         rm_SetBtsAdminState(locked);
         rm_ProcBtsAdminStateLocked();
     } else
     {  
         //Bts is unlocked, handle TRX admin states
         for (i=0;i<OAMrm_MAX_TRXS;i++)
         {
              if (OAMrm_TRX_AMSTATE(i)!=unlocked)
              {
	            rm_SetTrxAdminState(i,locked);
	            rm_ProcTrxAdminStateLocked(i);
              } else
              {
                  //TRX is unlocked, handle TS admin states
                  for (j=0;j<OAMrm_SLOTS_PER_TRX;j++)
                  {
                       if (OAMrm_CHN_AMSTATE(i,j) != unlocked)
                       {
	                     rm_SetTrxSlotAdminState(i,j,locked);
                           rm_ProcTrxSlotAdminStateLocked(i,j);
                       }
                  }
              }
         }
     } 

     //Turn on TRX power if needed
     for (i=0;i<OAMrm_MAX_TRXS;i++)
     {
          if (OAMrm_TRX_AMSTATE(i)==unlocked && 
              OAMrm_TRX_OPSTATE(i)==opStateEnabled)
          { 
              //Turn on TRX radio transmission
              rm_SendTuneSynthToTrx(i,1);
          } else
          {
              //No tchfs available,nonsense!
              rm_SetRmOpState(rm_TrxRadioOff[i]);
              IDEBUG__(("WARNING@rm_TurnOnRadioIfNeeded:trx '%d' radio off\n", i));
          }
     }

     for (i=0;i<OAMrm_MAX_TRXS;i++)
     {
          if (OAMrm_TRX_OPSTATE(i) == opStateEnabled)
              break;
     }
     if (i>=OAMrm_MAX_TRXS)
     {
          //No trx is enabled,block whole VC
          rm_SetRmOpState(EC_RM_NO_TRX_ENABLED);
          IDEBUG__(("WARNING@rm_TurnOnRadioIfNeeded:no trx is enabled\n"));
     }
#endif //#if 0
}

void rm_MakeSureDspIsTicking(void)
{

//G2
#if 0
     u8 trx;

     trx = rm_pItcRxL1Msg->l3_data.buffer[3];

     //Check if DSP is still ticking or not
     if (rm_pItcRxL1Msg->l3_data.buffer[4]==rm_TrxMgmt[trx].tdma1 &&
         rm_pItcRxL1Msg->l3_data.buffer[5]==rm_TrxMgmt[trx].tdma2 &&
         rm_pItcRxL1Msg->l3_data.buffer[6]==rm_TrxMgmt[trx].tdma3 &&
         rm_pItcRxL1Msg->l3_data.buffer[7]==rm_TrxMgmt[trx].tdma4  )
     {
         //Not ticking any more, raise a critical alarm
         rm_SetRmOpState(rm_DspNotTickingAnyMore[trx]);
     } else
     {
         //Still ticking fine right now
         rm_TrxMgmt[trx].tdma1 = rm_pItcRxL1Msg->l3_data.buffer[4];
         rm_TrxMgmt[trx].tdma2 = rm_pItcRxL1Msg->l3_data.buffer[5];
         rm_TrxMgmt[trx].tdma3 = rm_pItcRxL1Msg->l3_data.buffer[6];
         rm_TrxMgmt[trx].tdma4 = rm_pItcRxL1Msg->l3_data.buffer[7];
     }
#endif //#if 0
}

//PR 1323 SMSCBC, shedule message, 6/29
bool JcMsgSendToSmscbc(InterTxCbcCbMsg_t *msg);

void rm_VcToVbUpNotification(void)
{
     InterTxCbcCbMsg_t vcUp;
     long int vbIpAddress, vcIpAddress;

     RMDBG_ENTER(("rm_VcToVbUpNotification", RM_LAYER));
     // DBG_ENTER();

     if (STATUS_OK!=oam_getMibIntVar(MIB_h323_GKIPAddress, &vbIpAddress))
     {
         EDEBUG__(("RM_MAIN: VB IP address not available\n"));
         return;
     } 

     vcIpAddress = inet_addr(GetViperCellIpAddress());

     vcUp.msgType = INTER_SMSCBC_VC_UP;

     //vcUp.origVcId = (short) vcId;
     vcUp.origVcAddress = vcIpAddress;
     vcUp.origModuleId = MODULE_RM;
     vcUp.origSubId = 0;

     //vcUp.destVcId = 0;
     vcUp.destVcAddress = vbIpAddress;
     vcUp.destModuleId = MODULE_SMSCBC;
     vcUp.destSubId = 0;
     vcUp.message_id = 0;
     JcMsgSendToSmscbc(&vcUp);
}	

void rm_InitCb(void)
{
     PDEBUG__(("ENTER@rm_InitCb(): cb control setting=(%d)\n", OAMrm_CBCH_OPSTATE));

     OAMrm_CBCH_OPSTATE = OAMrm_CB_FEATURE_SETTING;
     if ( (OAMrm_CBCH_OPSTATE) && (rm_pCbchChan->usable==RM_PHYCHAN_USABLE) )
     { 
         rm_pCbchChan->usable = RM_PHYCHAN_CBCH_USABLE;
         rm_ActivateCbchChan();
         rm_WaitForCbchChanActAck();
     }
         
     rm_ChanCbchActive = true;
     rm_BroadcastNilCbMsg();
     rm_VcToVbUpNotification();
}

void rm_WaitForCbchChanActAck(void)
{
   int waitAck, result;

   PDEBUG__(("ENTER@rm_WaitForCbchChanActAck: cb control setting=(%d)\n", OAMrm_CBCH_OPSTATE));

   waitAck=true;

   while(waitAck)
   {
      //Read a msg from queue or pend on arrival of message
      if (ERROR == (result=rm_msgQReceive(rm_MsgQId, (s8 *)&rm_ItcRxMsg, 
                                          sizeof(rm_ItcRxMsg_t), WAIT_FOREVER)))
      {  
          //Error in reading VxWork msg queue. Ignore msg!
          rm_SetRmOpState(EC_RM_OS_CALL_MSGQRECEIVE_FAILED);
          EDEBUG__(("ERROR@rm_WaitForCbchChanActAck:VxWorks Q. read errno(%d)\n", errno));
          printf("ERROR@rm_WaitForCbchChanActAck:VxWorks Q. read errno(%d)\n", errno);
          assert(0);
      } 

      rm_pItcRxL1Msg = (rm_ItcRxL1Msg_t*) &rm_ItcRxMsg;

      if ( (rm_pItcRxL1Msg->module_id==MODULE_L1) &&
           (RM_L1MSGCMP(RM_L1MG_DEDMGMT,RM_L1MT_CHANACTIVACK_MSB,RM_L1MT_CHANACTIVACK_LSB)) )
            waitAck=false;
       else
            if (rm_DbgDump) BYDUMP__(( (u8*)&rm_ItcRxMsg, result));
   }
}

#endif //__RM_MAIN_CPP__
