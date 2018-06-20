/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROCDSPMSG_CPP__
#define __GRR_PROCDSPMSG_CPP__

#include "grr\grr_head.h"

void grr_ProcDspMsg(void)
{
   JC_STATUS jcRet;
   unsigned char trx, slot;
   int msgType;

   DBG_FUNC("grr_ProcDspMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   if (!grr_ProcDspValidateMsg(&msgType)) {DBG_LEAVE();return;}

   switch(msgType)
   {
   case GRR_L1MT_PINGACK:
        //Got keep-alive rsp
        //
        grr_ProcDspPingAck();

        break;

   case GRR_L1MT_SLOTACTIVACK:
        //Got slot act ack
        //
        trx = grr_pItcRxDspMsg->buffer[3];
        slot= grr_pItcRxDspMsg->buffer[4];

        //Play sanitary check
        if ( (trx>=OAMgrr_MAX_TRXS) || (slot>=8) )
		{
            DBG_WARNING("grr_ProcDspMsg: slot Act ack to invalid trx(%d) slot(%d)\n",
                         trx, slot);
            printf("grr_ProcDspMsg: slot act ack to invalid trx(%d) slot(%d)\n",
                         trx, slot);
        }
        else
        {   
            //trx-slot op state shall stay as disabled during its configuration change
            if (OAMgrr_CHN_OPSTATE(trx,slot)==opStateEnabled)
                DBG_WARNING("grr_ProcDspMsg: opState should be disabled for trx(%d) slot(%d)\n",
                             trx, slot);
            else
            {
                OAMgrr_CHN_OPSTATEa(trx,slot) = opStateEnabled;
                grr_SetOamTrxSlotOpState(trx,slot,opStateEnabled);
            }

            //Notify RLC and RRM of the trx-slot channel comb change
            //grr_SendTrxSlotChanComb(MODULE_GRR, rlcMsgQId, trx,slot,OAMgrr_TRX_SLOT_COMB(trx,slot));
			DBG_TRACE("grr_ProcDspMsg:inform RLC_MAC of current opState (%d) for (trx=%d,slot=%d)\n",
				       opStateEnabled, trx, slot);

            if (JC_OK != (jcRet=RlcMacTSOpState(trx, slot, opStateEnabled)))
            {
                DBG_ERROR("grr_ProcDspMsg: RlcMacTSOpState(1) error (trx=%d,slot=%d,state=%d,ret=%d)\n",
					       trx,slot,opStateEnabled,jcRet);
                assert(0);
            } 

  		    DBG_TRACE("grr_ProcDspMsg:inform RLC_MAC of slot config (%d) used for (trx=%d,slot=%d)\n",
				       OAMgrr_TRX_SLOT_COMB(trx,slot), trx, slot);

            if (JC_OK != (jcRet=RlcMacConfigureTS(trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot))) )
            {
                DBG_ERROR("grr_ProcDspMsg: RlcMacConfigureTS(1) error (trx=%d, slot=%d, config=%d, ret=%d)\n",
					       trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot), jcRet);
                assert(0);
            } 

            grr_SendTrxSlotChanComb(MODULE_GRR, rm_MsgQId, trx,slot,OAMgrr_TRX_SLOT_COMB(trx,slot));
        }
        
        break;

   case GRR_L1MT_SLOTACTIVNACK:
        //Got slot act nack
        //
        trx = grr_pItcRxDspMsg->buffer[3];
        slot= grr_pItcRxDspMsg->buffer[4];

        //Play sanitary check
        if ( (trx>=OAMgrr_MAX_TRXS) || (slot>=8) )
		{
            DBG_WARNING("grr_ProcDspMsg: slot act nack to invalid trx(%d) slot(%d)\n",
                         trx, slot);
            printf("grr_ProcDspMsg: slot act nack to invalid trx(%d) slot(%d)\n",
                         trx, slot);
		}
        else
        {
            //Raise an alarm for this negative acknowledgment
            grr_SubmitAlarm(grr_TrxSlotActNackAlarmCode[trx][slot]);

            //trx-slot op state shall stay as disabled during its configuration change
            if (OAMgrr_CHN_OPSTATE(trx,slot)==opStateEnabled)
            {
                DBG_WARNING("grr_ProcDspMsg: opState should be disabled for trx(%d) slot(%d)\n",
                             trx, slot);
                OAMgrr_CHN_OPSTATEa(trx,slot) = opStateDisabled;
                grr_SetOamTrxSlotOpState(trx,slot,opStateDisabled);
            }

            //Notify RLC and RRM of the trx-slot channel comb change
            //grr_SendTrxSlotChanComb(MODULE_GRR, rlcMsgQId, trx,slot,OAMgrr_TRX_SLOT_COMB(trx,slot));
			DBG_TRACE("grr_ProcDspMsg:inform RLC_MAC of current opState (%d) for (trx=%d,slot=%d)\n",
				       opStateDisabled, trx, slot);

            if (JC_OK != (jcRet=RlcMacTSOpState(trx, slot, opStateDisabled)))
            {
                DBG_ERROR("grr_ProcDspMsg: RlcMacTSOpState(2) error (trx=%d,slot=%d,state=%d,ret=%d)\n",
					       trx,slot,opStateDisabled,jcRet);
                assert(0);
            } 

  		    DBG_TRACE("grr_ProcDspMsg:inform RLC_MAC of slot config (%d) used for (trx=%d,slot=%d)\n",
				       OAMgrr_TRX_SLOT_COMB(trx,slot), trx, slot);

            if (JC_OK != (jcRet=RlcMacConfigureTS(trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot))) )
            {
                DBG_ERROR("grr_ProcDspMsg: RlcMacConfigureTS(2) error (trx=%d, slot=%d, config=%d, ret=%d)\n",
					       trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot), jcRet);
                assert(0);
            } 

            grr_SendTrxSlotChanComb(MODULE_GRR, rm_MsgQId, trx,slot,OAMgrr_TRX_SLOT_COMB(trx,slot));
        }
        break;
   }

   DBG_LEAVE();
}

bool grr_ProcDspValidateMsg(int *msgType)
{

   DBG_FUNC("grr_ValidateDspMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   //Validate the DSP message is expected:
   //

   //Check if received response to keep-alive inquiry
   if (GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_PINGACK_MSB,GRR_L1MT_PINGACK_LSB))
       *msgType = GRR_L1MT_PINGACK;
   else if (GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_SLOTACTIVACK_MSB,GRR_L1MT_SLOTACTIVACK_LSB))
       *msgType = GRR_L1MT_SLOTACTIVACK;
   else if (GRR_L1MSGCMP(GRR_L1MG_TRXMGMT,GRR_L1MT_SLOTACTIVNACK_MSB,GRR_L1MT_SLOTACTIVNACK_LSB))
       *msgType = GRR_L1MT_SLOTACTIVNACK;
   else 
       *msgType = 0;

   DBG_LEAVE();
   return ((*msgType)?true:false);
}
void grr_ProcDspPingAck(void)
{
   unsigned char trx;

   DBG_FUNC("grr_ProcDspPingAck", GRR_LAYER_DSP);
   DBG_ENTER();

   trx = grr_pItcRxDspMsg->buffer[3];

   //Check if DSP is still ticking or not
   if (grr_pItcRxDspMsg->buffer[4]==grr_TrxMgmt[trx].tdma1 &&
       grr_pItcRxDspMsg->buffer[5]==grr_TrxMgmt[trx].tdma2 &&
       grr_pItcRxDspMsg->buffer[6]==grr_TrxMgmt[trx].tdma3 &&
       grr_pItcRxDspMsg->buffer[7]==grr_TrxMgmt[trx].tdma4  )
   {
       //Not ticking any more, raise a critical alarm
       DBG_ERROR("grr_ProcDspPingAck: dsp(%d) stop ticking\n", trx);
       printf("grr_ProcDspPingAck: dsp(%d) stop ticking\n", trx);
       grr_SubmitAlarm(grr_DspNotTickingAnyMore[trx]);
       assert(0);
   } else
   {
       //Still ticking fine right now
       grr_TrxMgmt[trx].alive = true;
       grr_TrxMgmt[trx].tdma1 = grr_pItcRxDspMsg->buffer[4];
       grr_TrxMgmt[trx].tdma2 = grr_pItcRxDspMsg->buffer[5];
       grr_TrxMgmt[trx].tdma3 = grr_pItcRxDspMsg->buffer[6];
       grr_TrxMgmt[trx].tdma4 = grr_pItcRxDspMsg->buffer[7];
   }

   DBG_LEAVE();
}

#endif //__GRR_PROCDSPMSG_CPP__

