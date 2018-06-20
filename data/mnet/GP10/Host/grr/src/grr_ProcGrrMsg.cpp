/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROCGRRMSG_CPP__
#define __GRR_PROCGRRMSG_CPP__

#include "grr\grr_head.h"


void grr_ProcGrrMsg(void)
{
   unsigned char trx;

   DBG_FUNC("grr_ProcGrrMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   if (!grr_ProcGrrValidateMsg())  return;

   trx = grr_pItcRxGrrMsg->trx;

   //Check if trx is still alive
   if (grr_TrxMgmt[trx].alive)
   {   
       //trx is still alive. Ping it!
       grr_TrxMgmt[trx].alive = false;
       grr_SendDspPing(trx);
       grr_StartDspWdog(trx);
   } else
   {
       //trx not responding, raise an alarm and abort this task!
       grr_SubmitAlarm(grr_DspNotResponding[trx]);
       DBG_ERROR("grr_ProcGrrMsg: dsp(%d) not responding\n", trx);
       printf("grr_ProcGrrMsg: dsp(%d) not responding\n", trx);
       assert(0);
   }
   DBG_LEAVE();         
}

bool grr_ProcGrrValidateMsg(void)
{
   bool ret;

   DBG_FUNC("grr_ProcGrrValidateMsg", GRR_LAYER_PROC);
   DBG_ENTER();

   ret = true;

   if ( (grr_pItcRxGrrMsg->trx>=OAMgrr_MAX_TRXS)||
        (grr_pItcRxGrrMsg->timer_id != GRR_WDOG) )
   {
        DBG_WARNING("grr_ProcGrrValidateMsg: invalid trx(%d) or wdog id(%x)\n",
                     grr_pItcRxGrrMsg->trx,
			   grr_pItcRxGrrMsg->timer_id);
        ret = false;
   }

   DBG_LEAVE();
   return ret;
}

#endif //__GRR_PROCGRRMSG_CPP__

