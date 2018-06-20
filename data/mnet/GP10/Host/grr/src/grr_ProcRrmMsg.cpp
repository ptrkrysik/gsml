/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROCRRMMSG_CPP__
#define __GRR_PROCRRMMSG_CPP__

#include "grr\grr_head.h"

void grr_ProcRrmMsg(void)
{
   unsigned char trx, slot;

   DBG_FUNC("grr_ProcRrmMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   trx = grr_pItcRxRrmMsg->trx; slot = grr_pItcRxRrmMsg->slot;

   DBG_TRACE("grr_ProcRrmMsg: received Rrm msg(%d) trx(%d) slot(%d)\n",
              grr_pItcRxRrmMsg->message_type, trx, slot);

   //Validate received RRM message
   if (!grr_ProcRrmValidateMsg()) {DBG_LEAVE(); return;}

   //Only expecting individual slot shutting down acknowledge
   switch (grr_pItcRxRrmMsg->message_type)
   {
   case GRR_TRX_SLOT_SHUTDOWN_ACK:
        //Received trx slot shutdown ack
        // 
        grr_ProcRrmTrxSlotShutdownAck(trx,slot);

        break;

   default:
	  break;
   }

   DBG_LEAVE();
}

bool grr_ProcRrmValidateMsg(void)
{
   unsigned char trx, slot;

   DBG_FUNC("grr_ProcRrmValidateMsg", GRR_LAYER_PROC);
   DBG_ENTER();


   //Validate the RRM message is expected:
   //

   //Right now only expecting TS shuttingdow acknowledgement
   if (grr_pItcRxRrmMsg->message_type != GRR_TRX_SLOT_SHUTDOWN_ACK)
   {
       DBG_LEAVE();
       return false;
   }

   trx = grr_pItcRxRrmMsg->trx; slot = grr_pItcRxRrmMsg->slot;
   
   if  ( (trx>=OAMgrr_MAX_TRXS) || (slot>=8) )
   {
         DBG_WARNING("grr_ProcRrmValidateMsg: recved slot shutdown ack for invalid trx(%d) slot(%d)\n",
                      trx, slot);
         DBG_LEAVE();
         return false;
   }

   //Check if (trx,slot) is still pending on shutdown ack
   //
   if (OAMgrr_TRX_SLOT_AMSTATE(trx,slot) != shuttingDown) {DBG_LEAVE(); return false;}

   //Check if (trx,slot) is configured for circuit-switched use
   //
   if ( (OAMgrr_TRX_SLOT_COMB(trx,slot)!=OAMgrr_TRX_SLOT_COMB_5) &&
        (OAMgrr_TRX_SLOT_COMB(trx,slot)!=OAMgrr_TRX_SLOT_COMB_1)  )
   {
         DBG_WARNING("grr_ProcRrmValidateMsg: recved trx(%d) slot(%d) shutdown ack chanComb(%d) n.a.\n", 
                      trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot));
         printf("grr_ProcRrmValidateMsg: recved trx(%d) slot(%d) shutdown ack chanComb(%d) n.a.\n", 
                      trx, slot, OAMgrr_TRX_SLOT_COMB(trx,slot));
         DBG_LEAVE();
         return false;
   }

   DBG_LEAVE();
   return true;
}
    

void grr_ProcRrmTrxSlotShutdownAck(unsigned char trx, unsigned char slot)
{
   DBG_FUNC("grr_ProcRrmTrxSlotShutdownAck", GRR_LAYER_PROC);
   DBG_ENTER();

   //Apply sanitary check
   if ( (trx>=OAMgrr_MAX_TRXS)||(slot>=8))
   {
       DBG_WARNING("grr_ProcRrmTrxSlotShutdownAck: invalid trx(%d) or slot(%d)\n",
                    trx, slot);
       DBG_LEAVE();
       return;
   }
   
   OAMgrr_CHN_AMSTATEa(trx, slot) = locked;
   grr_SetOamTrxSlotAmState(trx,slot,locked);
   grr_SubmitAlarm(grr_TrxSlotAmStateLockedAlarmCode[trx][slot]);

   if (OAMgrr_TRX_AMSTATE(trx)==shuttingDown)
       grr_ProcRrmTryShutdownTrx(trx);

   DBG_LEAVE();
}

void grr_ProcRrmTryShutdownTrx(unsigned char trx)
{
   DBG_FUNC("grr_ProcRrmTryShutdownTrx", GRR_LAYER_PROC);
   DBG_ENTER();

   grr_ProcUtilTryShutdownTrx(trx);

   DBG_LEAVE();
}

void grr_ProcRrmTryShutdownBts(void)
{
   DBG_FUNC("grr_ProcRrmTryShutdownBts", GRR_LAYER_PROC);
   DBG_ENTER();

   grr_ProcUtilTryShutdownBts();

   DBG_LEAVE();
}

#endif //__GRR_PROCRRMMSG_CPP__

