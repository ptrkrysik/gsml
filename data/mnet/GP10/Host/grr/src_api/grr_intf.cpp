/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_INTF_CPP_
#define __GRR_INTF_CPP__

#include "grr\grr_head.h"

//Administrative & operational state & chan config related inquiries
//
AdministrativeState grr_GetBtsAmState(void)
{
   return(OAMgrr_BTS_AMSTATE);
}

AdministrativeState grr_GetTrxAmState(int trx)
{
   return(OAMgrr_TRX_AMSTATE(trx));
}
	
AdministrativeState grr_GetTrxSlotAmState(int trx, int slot)
{
   return(OAMgrr_TRX_SLOT_AMSTATE(trx,slot));
}

EnableDisable grr_GetBtsOpState(void)
{
   return(OAMgrr_BTS_OPSTATE);
}

EnableDisable grr_GetTrxOpState(int trx)
{
   return(OAMgrr_TRX_OPSTATE(trx));
}

EnableDisable grr_GetTrxSlotOpState(int trx, int slot)
{
   return(OAMgrr_TRX_SLOT_OPSTATE(trx,slot));
}

bool grr_IsPccchExisting(int *trx, int* slot, int* comb)
{
   bool ret;
   unsigned char i, j;

   *trx = *slot = 0;
   *comb = OAMgrr_TRX_SLOT_COMB(0,0);   
   ret=false;

   for (i=0;i<OAMgrr_MAX_TRXS;i++)
   for (j=0;j<8;j++)
   { 
      if ( (OAMgrr_TRX_SLOT_COMB(i,j)==OAMgrr_SLOT_COMB_11) || 
           (OAMgrr_TRX_SLOT_COMB(i,j)==OAMgrr_SLOT_COMB_12)  )
      {
  	    *trx = i; *slot = j;
            *comb = OAMgrr_TRX_SLOT_COMB(i,j);
  	    ret = true;
            break;
      }
   }

   return ret;
}

int grr_GetTrxSlotChanComb(int trx, int slot)
{
   return(OAMgrr_TRX_SLOT_CHANCOMB(trx,slot));
}

int grr_GetTrxSlotTsc(int trx, int slot)
{
   return(OAMgrr_TRX_SLOT_TSC(trx,slot));
}

int grr_GetTrxSlotArfcn(int trx, int slot)
{
   return(OAMgrr_TRX_SLOT_ARFCN(trx,slot));
}

int grr_getBtsGprsNetworkMode(void)
{
   return (OAMgrr_GPRS_NETWORK_MODE);
}

bool grr_IsBtsAmTypeBlocked(void)
{
   return ( (OAMgrr_BTS_AMSTATE != unlocked) ||
            (OAMgrr_TRX_AMSTATE(0) != unlocked) ||
            (OAMgrr_TRX_SLOT_AMSTATE(0,0) != unlocked) );

}

bool grr_IsBtsOpTypeBlocked(void)
{
   return (OAMgrr_BTS_OPSTATE!=opStateEnabled);
}

bool grr_IsBtsBlocked(void)
{
     return(grr_IsBtsAmTypeBlocked()||grr_IsBtsOpTypeBlocked());
}

bool grr_IsBtsGprsFeatureActive(void)
{
   return (OAMgrr_GPRS_FEATURE_SETTING);
}

bool grr_IsBtsCbFeatureActive(void)
{
   return (OAMgrr_CB_FEATURE_SETTING);
}


//Update application modules i.e. RLC, RRM with current setting
//
void grr_SendTrxSlotChanComb(int src_module, MSG_Q_ID dst_q, int trx, int slot, int chanComb)
{
   DBG_FUNC("grr_SendTrxSlotChanComb", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_TRX_SLOT_CHANCOMB, trx, slot, chanComb);

   DBG_TRACE("grr_SendTrxSlotChanComb: src(%d) dst(%p) trx(%d) slot(%d) msg(%d) chanComb(%d)\n",
              src_module, dst_q, trx, slot, GRR_TRX_SLOT_CHANCOMB, chanComb);
}

void grr_SendGprsFeatureSetting(int src_module, MSG_Q_ID dst_q)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendGprsFeatureSetting", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_GPRS_FEATURE_CONTROL,0,0,OAMgrr_GPRS_FEATURE_SETTING);

   DBG_TRACE("grr_SendGprsFeatureSetting: src(%d) dst(%p) setting(%d)\n", 
              src_module, dst_q, OAMgrr_GPRS_FEATURE_SETTING);

   DBG_LEAVE();
}


void grr_SendCbFeatureSetting(int src_module, MSG_Q_ID dst_q)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendCbFeatureSetting", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_CB_FEATURE_CONTROL, 0, 0, OAMgrr_CB_FEATURE_SETTING);

   DBG_TRACE("grr_SendCbFeatureSetting: src(%d) dst(%p) setting(%d)\n", 
              src_module, dst_q, OAMgrr_CB_FEATURE_SETTING);

   DBG_LEAVE();
}

void grr_SendReadyToGoReq(int src_module, MSG_Q_ID dst_q)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendReadyToGoReq", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_READY_TO_GO_REQ, 0, 0, 0);

   DBG_TRACE("grr_SendReadyToGoReq: src(%d) dst(%p)\n", src_module, dst_q);

   DBG_LEAVE();

}

void grr_SendReadyToGoAck(int src_module, MSG_Q_ID dst_q)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendReadyToGoAck", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_READY_TO_GO_ACK, 0, 0, 0);

   DBG_TRACE("grr_SendReadyToGoAck: src(%d) dst(%p)\n", src_module, dst_q);

   DBG_LEAVE();
}

//Administrative State related operations: requests and acknowledgements
//
//void grr_SendBtsLockReq	(int src_module, MSG_Q_ID dst_q);
//void grr_SendBtsUnlockReq	(int src_module, MSG_Q_ID dst_q);
//void grr_SendBtsShutdownReq	(int src_module, MSG_Q_ID dst_q);
//void grr_SendBtsLockAck	(int src_module, MSG_Q_ID dst_q);
//void grr_SendBtsUnlockAck	(int src_module, MSG_Q_ID dst_q);
//void grr_SendBtsShutdownAck	(int src_module, MSG_Q_ID dst_q);

//void grr_SendTrxLockReq	(int src_module, MSG_Q_ID dst_q, int trx);
//void grr_SendTrxUnlockReq	(int src_module, MSG_Q_ID dst_q, int trx);
//void grr_SendTrxShutdownReq	(int src_module, MSG_Q_ID dst_q, int trx);
//void grr_SendTrxLockAck	(int src_module, MSG_Q_ID dst_q, int trx);
//void grr_SendTrxUnlockAck	(int src_module, MSG_Q_ID dst_q, int trx);
//void grr_SendTrxShutdownAck	(int src_module, MSG_Q_ID dst_q, int trx);

void grr_SendTrxSlotLockReq (int src_module, MSG_Q_ID dst_q, int trx, int slot)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendTrxSlotLockReq", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_TRX_SLOT_LOCK_REQ, trx, slot, 0);

   DBG_TRACE("grr_SendTrxSlotLockReq: src(%d) dst(%d) trx(%d) slot(%d) msg(%d)\n",
              src_module, dst_q, trx, slot, GRR_TRX_SLOT_LOCK_REQ);

   DBG_LEAVE();
}

void grr_SendTrxSlotShutdownReq(int src_module, MSG_Q_ID dst_q, int trx, int slot)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendTrxSlotShutdownReq", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_TRX_SLOT_SHUTDOWN_REQ, trx, slot, 0);

   DBG_TRACE("grr_SendTrxSlotShutdownReq: src(%d) dst(%d) trx(%d) slot(%d) msg(%d)\n",
              src_module, dst_q, trx, slot, GRR_TRX_SLOT_SHUTDOWN_REQ);

   DBG_LEAVE();
}

void grr_SendTrxSlotLockAck(int src_module, MSG_Q_ID dst_q, int trx, int slot)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendTrxSlotLockAck", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_TRX_SLOT_LOCK_ACK, trx, slot, 0);

   DBG_TRACE("grr_SendTrxSlotLockAck: src(%d) dst(%d) trx(%d) slot(%d) msg(%d)\n",
              src_module, dst_q, trx, slot, GRR_TRX_SLOT_LOCK_ACK);

   DBG_LEAVE();
}

void grr_SendTrxSlotShutdownAck	(int src_module, MSG_Q_ID dst_q, int trx, int slot)
{
   STATUS	ret;
   grr_ItcMsg_t msg;

   DBG_FUNC("grr_SendTrxSlotShutdownAck", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_TRX_SLOT_SHUTDOWN_ACK, trx, slot, 0);

   DBG_TRACE("grr_SendTrxSlotShutdownAck: src(%d) dst(%d) trx(%d) slot(%d) msg(%d)\n",
              src_module, dst_q, trx, slot, GRR_TRX_SLOT_SHUTDOWN_ACK);

   DBG_LEAVE();

}

//void grr_SendTrxSlotUnlockAck(int src_module, MSG_Q_ID dst_q, int trx, int slot);
//void grr_SendTrxSlotUnlockReq(int src_module, MSG_Q_ID dst_q, int trx, int slot);

void	grr_SendCbCfgSetting(int src_module, MSG_Q_ID dst_q)
{
   DBG_FUNC("grr_SendCbCfgSetting", GRR_LAYER_INTF);
   DBG_ENTER();

   grr_SendMsg(src_module, dst_q, GRR_CB_FEATURE_CONTROL, 0, 0, OAMgrr_CB_FEATURE_SETTING);

   DBG_TRACE("grr_SendCbCfgSetting: src(%d) dst(%d) msg(%d) setting(%d)\n",
              src_module, dst_q, GRR_CB_FEATURE_CONTROL, OAMgrr_CB_FEATURE_SETTING);

   DBG_LEAVE();
}


//Messaging related to radio resource scheduling
//
//void grr_ReportRadioResUsage	(int src_module, MSG_Q_ID dst_q);
//void grr_GetRadioResRelCandiate	(int *trx, int *slot);

//void grr_OverloadedCellBarOff(void)
//{
//   DBG_FUNC("grr_OverloadedCellBarOff", GRR_LAYER_INTF);
//   DBG_ENTER();

//   if ( (OAMgrr_BTS_AMSTATE==unlocked) &&
//        (OAMgrr_TRX_AMSTATE(0)==unlocked) &&
//        (OAMgrr_CHN_AMSTATE(0,0)==unlocked) )
//   {
//        grr_PassOamMsSystemInformation2(0,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
//        grr_PassOamMsSystemInformation3(0,GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
//        grr_PassOamMsSystemInformation4(0,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);
//   }
//
//   DBG_LEAVE();
//}

T_CNI_RIL3_ACCESS_TECHNOLOGY_TYPE grr_GetRadioAccessTechnology(void)
{
    T_CNI_RIL3_ACCESS_TECHNOLOGY_TYPE sys = CNI_RIL3_ACCESS_TECHNOLOGY_TYPE_UNKNOWN;

    DBG_FUNC("grr_GetRadioAccessTechnology", GRR_LAYER_INTF);
    DBG_ENTER();

    switch(OAMgrr_GSMDCS_INDICATOR)
    {
    case gsm: //0
	   sys = CNI_RIL3_ACCESS_TECHNOLOGY_TYPE_GSM_P;		
         break;
    case extendedgsm: //1
	   sys = CNI_RIL3_ACCESS_TECHNOLOGY_TYPE_GSM_E;		
         break;
    case dcs: //2
	   sys = CNI_RIL3_ACCESS_TECHNOLOGY_TYPE_GSM_1800;	
         break;
    case pcs1900: //3
   	   sys = CNI_RIL3_ACCESS_TECHNOLOGY_TYPE_GSM_1900;		
         break;
    //case gsmr: //4
    //     sys = CNI_RIL3_ACCESS_TECHNOLOGY_TYPE_GSM_R;
    //     break;
    }
    DBG_LEAVE();
    return sys;
}


short grr_DecideCipherAlgo( 
      T_CNI_RIL3_IE_CIPHER_MODE_SETTING *setting,
      T_CNI_RIL3_IE_MS_CLASSMARK_2 *classmark2)
{
     short algo = 0;

     DBG_FUNC("grr_DecideCipherAlgo", GRR_LAYER_INTF);
     DBG_ENTER();

     setting->ciphering = CNI_RIl3_CIPHER_START_CIPHERING;

	 DBG_TRACE("grr_DecideCipherAlgo: dspCiphCap(%x)\n", grr_DspCiphCap);

     if ( !classmark2->a51 && GRR_DSPA51(grr_DspCiphCap) )
     {
          algo = 1;
          setting->algorithm = CNI_RIL3_CIPHER_ALGORITHM_A51;
     } else if (classmark2->a52 && GRR_DSPA52(grr_DspCiphCap) )
     {
          algo = 2;
          setting->algorithm = CNI_RIL3_CIPHER_ALGORITHM_A52;
     } else
     {
          algo = 0;
          setting->ciphering =CNI_RIl3_CIPHER_NO_CIPHERING; 
     }

	 DBG_LEAVE();
     return (algo);
}

//GRR always call this func to submit alarms
void grr_SubmitAlarm(Alarm_errorCode_t code)
{    
  STATUS ret;
  JC_STATUS	jcRet;

  DBG_FUNC("grr_SubmitAlarm", GRR_LAYER_ALARM);
  DBG_ENTER();

  DBG_TRACE("grr_SubmitAlarm: submit alarm code(%d)\n", code);  

  if (code != EC_NOERROR)
  {
     if (JC_OK!=alarm_raise(MODULE_GRR, MIB_errorCode_GRR, code))
     {
         //This is very critical, should abort GRR task here !
         DBG_ERROR("grr_SubmitAlarm: alarm_raise failed to raise alarm(%d)\n", code);
         printf("grr_SubmitAlarm: alarm_raise failed to raise alarm(%d)\n", code);
         assert(0);
     }
  } else
  {
     if (JC_OK!=alarm_clear(MODULE_GRR, MIB_errorCode_GRR, code))
     {
         //This is very critical, should abort GRR task here <TBD>!!!
         DBG_ERROR("grr_SubmitAlarm: alarm_clear failed to clear an alarm\n");
         printf("grr_SubmitAlarm: alarm_clear failed to clear an alarm\n");
         assert(0);
     }
  }

  //Remember it if it is a critical alarm
  if (code < EC_GRR_INFO_BASE) 
  {
      //btsOpState goes disabled for a critical alarm
	  if (code == EC_NOERROR)
	  {
		  if (OAMgrr_BTS_OPSTATE == opStateDisabled)
		  {
			  DBG_TRACE("grr_SubmitAlarm: inform RLC_MAC of bts op enabled (op %d, alarm %d)\n",
				         OAMgrr_BTS_OPSTATE, code);  
		      if (JC_OK != (jcRet=RlcMacGPOpState(opStateEnabled)))
  		          DBG_ERROR("grr_SubmitAlarm: RlcMacGPOpState() err(%d)\n", jcRet);
		  }
		  OAMgrr_BTS_OPSTATEa = opStateEnabled;
	  } else
	  {
	      if (OAMgrr_BTS_OPSTATE == opStateEnabled)
		  {
			  DBG_TRACE("grr_SubmitAlarm: inform RLC_MAC of bts op disabled (op %d, alarm %d)\n",
				         OAMgrr_BTS_OPSTATE, code);  
		      if (JC_OK != (jcRet=RlcMacGPOpState(opStateDisabled)))
  		          DBG_ERROR("grr_SubmitAlarm: RlcMacGPOpState() err(%d)\n", jcRet);
		  }
          OAMgrr_BTS_OPSTATEa = opStateDisabled;
	  } 
  }

  DBG_LEAVE();
}

void grr_SendMsg(int src_module, MSG_Q_ID dst_q, grr_MessageType_t msgType,
                 int trx, int slot, int ctrl)
{
   STATUS       ret;
   grr_ItcMsg_t msg;
   

   DBG_FUNC("grr_SendMsg", GRR_LAYER_INTF);
   DBG_ENTER();

   msg.module_id     = (unsigned char) src_module;
   msg.message_type  = msgType;
   msg.trx	         = (unsigned char) trx;
   msg.slot	         = (unsigned char) slot;
   msg.feat_ctrl     = (unsigned char) ctrl; 

   //
   if (STATUS_OK != (ret = msgQSend(dst_q, (char*)&msg,
                                    sizeof(grr_ItcMsg_t), NO_WAIT, MSG_PRI_NORMAL) ) )
   {
       grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQSEND_FAILED);
       DBG_ERROR(" grr_SendMsg: msgQSend failed Q(%x) ret(%d) errno(%d)\n",
                  dst_q, ret, errno);
       printf(" grr_SendMsg: msgQSend failed Q(%x) ret(%d) errno(%d)\n",
                  dst_q, ret, errno);
       assert(0);
   }

   DBG_LEAVE();

}

void grr_SendMsgFromL1ToGrr(unsigned short length, unsigned char* buf)
{
     STATUS       	ret;
     grr_ItcMsg_t 	msg;     

     DBG_FUNC("grr_SendMsgFromL1ToGrr", GRR_LAYER_INTF);
     DBG_ENTER();

     //Check if buf[3] i.e. trx and length valid
     if ( (buf[3]>=OAMgrr_MAX_TRXS) || (length<4) )
     {   
         DBG_WARNING("grr_SendMsgFromL1ToGrr: invalid trx(%d) or length(%d)\n",
			    buf[3], length);
         return;
     }

     msg.module_id = MODULE_L1;
     memcpy(msg.buffer, buf, length);

     if (grr_DbgDump) DBG_HEXDUMP(buf, length);
	   
     if (STATUS_OK != (ret = msgQSend(grr_MsgQId, (char*) &msg, sizeof(grr_ItcMsg_t),
			                    WAIT_FOREVER, MSG_PRI_NORMAL) ) )
     {
         grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQSEND_FAILED);
	   DBG_ERROR("grr_SendMsgFromL1ToGrr: msgQSend failed, Queue(%p) ret(%d) errno(%d)\n",
			  grr_MsgQId, ret, errno);
	   printf("grr_SendMsgFromL1ToGrr: msgQSend failed, Queue(%p) ret(%d) errno(%d)\n",
			  grr_MsgQId, ret, errno);
         assert(0);
     }
     
     DBG_LEAVE();
}

#endif  //__GRR_INTF_CPP__

