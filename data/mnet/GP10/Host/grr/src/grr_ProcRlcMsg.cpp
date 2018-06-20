/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROCRLCMSG_CPP__
#define __GRR_PROCRLCMSG_CPP__

#include "grr\grr_head.h"

void grr_ProcRlcMsg(void)
{
   unsigned char trx, slot;

   DBG_FUNC("grr_ProcRlcMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   trx = grr_pItcRxRlcMsg->trx; slot = grr_pItcRxRlcMsg->slot;

   DBG_TRACE("grr_ProcRlcMsg: received Rlc msg(%d) for trx(%d) slot(%d)\n",
              grr_pItcRxRlcMsg->message_type, trx, slot);

   //Validate received RRM message
   if (!grr_ProcRlcValidateMsg()) {DBG_LEAVE(); return;}

   //Only expecting trx slot shutdown ack
   switch (grr_pItcRxRlcMsg->message_type)
   {
   case GRR_TRX_SLOT_SHUTDOWN_ACK:
        //Received trx slot shutdown ack
        // 
        grr_ProcRlcTrxSlotShutdownAck(trx,slot);
     
        break;
   
   default:
        break;
   }

   DBG_LEAVE();
}

bool grr_ProcRlcValidateMsg(void)
{
   unsigned char trx, slot;

   DBG_FUNC("grr_ProcRlcRlcValidateMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   //Validate the received RLC message before processing it
   //

   //Right now only expecting trx slot shutdown acknowledgement
   if (grr_pItcRxRlcMsg->message_type != GRR_TRX_SLOT_SHUTDOWN_ACK)
   {
       DBG_LEAVE();
       return false;
   }

   //Apply sanitary check
   trx = grr_pItcRxRlcMsg->trx; slot  = grr_pItcRxRlcMsg->slot;
   
   if  ( (trx>=OAMgrr_MAX_TRXS) || (slot>=8) )
   {
         DBG_WARNING("grr_ProcRlcValidateMsg: recved slot shutdown ack for invalid trx(%d) slot(%d)\n",
                      trx, slot);
         DBG_LEAVE();
         return false;
   }

   //Check if the (trx,slot) already left pending on shutdown ack now
   //
   if (OAMgrr_TRX_SLOT_AMSTATE(trx,slot) != shuttingDown) {DBG_LEAVE();return false;}
   
   //Check if the shutdown ack makes sense to current chancomb
   //
   if (OAMgrr_TRX_SLOT_COMB(trx,slot)!=OAMgrr_TRX_SLOT_COMB_13)
   {
       DBG_WARNING("grr_ProcRlcValidateMsg: received trx(%d) slot(%d) shutdown ack chanComb(%d) n.a.\n", 
                    trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot));
       printf("grr_ProcRlcValidateMsg: received trx(%d) slot(%d) shutdown ack chanComb(%d) n.a.\n", 
                    trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot));

       DBG_LEAVE();
       return false;
   }

   DBG_LEAVE();
   return true;
}

void grr_ProcRlcTrxSlotShutdownAck(unsigned char trx, unsigned char slot)
{
   DBG_FUNC("grr_ProcRlcTrxSlotShutdownAck", GRR_LAYER_PROC);
   DBG_ENTER();

   //Apply sanitary check
   if ( (trx>=OAMgrr_MAX_TRXS)||(slot>=8))
   {
       DBG_WARNING("grr_ProcRlcTrxSlotShutdownAck: invalid trx(%d) or slot(%d)\n",
                    trx, slot);
       DBG_LEAVE();
       return;
   }
   
   OAMgrr_CHN_AMSTATEa(trx, slot) = locked;
   grr_SetOamTrxSlotAmState(trx,slot,locked);
   grr_SubmitAlarm(grr_TrxSlotAmStateLockedAlarmCode[trx][slot]);

   if (OAMgrr_TRX_AMSTATE(trx)==shuttingDown)
       grr_ProcRlcTryShutdownTrx(trx);

   DBG_LEAVE();
}

void grr_ProcRlcTryShutdownTrx(unsigned char trx)
{
   DBG_FUNC("grr_ProcRlcTryShutdownTrx", GRR_LAYER_PROC);
   DBG_ENTER();

   grr_ProcUtilTryShutdownTrx(trx);

   DBG_LEAVE();
}

void grr_ProcRlcTryShutdownBts(void)
{
   DBG_FUNC("grr_ProcRlcTryShutdownBts", GRR_LAYER_PROC);
   DBG_ENTER();

   grr_ProcUtilTryShutdownBts();

   DBG_LEAVE();
}

#endif //__GRR_PROCRLCMSG_CPP__
