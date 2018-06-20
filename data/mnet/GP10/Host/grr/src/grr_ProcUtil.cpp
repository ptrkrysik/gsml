/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROCUTIL_CPP__
#define __GRR_PROCUTIL_CPP__

#include "grr\grr_head.h"

void grr_ProcUtilTryShutdownTrx(unsigned char trx)
{
   unsigned char i;

   DBG_FUNC("grr_ProcUtilTryShutdownTrx", GRR_LAYER_PROC);
   DBG_ENTER();

   //Apply sanitary check
   if (trx>=OAMgrr_MAX_TRXS)
   {
       DBG_WARNING("grr_ProcUtilTryShutdownTrx: invalid trx(%d)\n", trx);
   } else 
   {
       //Scan through all its slots to see if all of them locked or not
       for (i=0;i<8;i++)
       {
            if ( OAMgrr_CHN_AMSTATE(trx, i) == shuttingDown)  break;
            else
            {
                 if (OAMgrr_CHN_AMSTATE(trx,i) != locked)
                 {
                     DBG_WARNING("grr_ProcUtilTryShutdownTrx: amState(%d) impossible for trx(%d) slot(%d)\n",
                                  OAMgrr_CHN_AMSTATE(trx,i),trx,i);
                     OAMgrr_CHN_AMSTATEa(trx,i) = locked;
                     grr_SetOamTrxSlotAmState(trx,i,locked);
                     grr_SubmitAlarm(grr_TrxSlotAmStateLockedAlarmCode[trx][i]);
                 }
            }
       } 
 
       if (i>=8)
       {
           //Lock the whole trx
           //
           OAMgrr_TRX_AMSTATEa(trx) = locked;
           grr_SubmitAlarm(grr_TrxAmStateLockedAlarmCode[trx]);
           grr_TuneTrxSynth(trx,0);

           if (OAMgrr_BTS_AMSTATE==shuttingDown)
               grr_ProcRlcTryShutdownBts();
       }
   } 

   DBG_LEAVE();
}

void grr_ProcUtilTryShutdownBts(void)
{
   int i,j;

   DBG_FUNC("grr_ProcUtilTryShutdownBts", GRR_LAYER_PROC);
   DBG_ENTER();

   //Scan through all TRXs to see if all of them locked or not
   for (i=0;i<OAMgrr_MAX_TRXS;i++)
   {
        if (OAMgrr_TRX_AMSTATE(i)==shuttingDown)  break;
        else
        {
            if (OAMgrr_TRX_AMSTATE(i) != locked)   
            {
                DBG_WARNING("grr_ProcUtilTryShutdownBts: trx(%d) amState(%d) impossible\n",
                             i, OAMgrr_TRX_AMSTATE(i));
                OAMgrr_TRX_AMSTATEa(i) = locked;
                grr_SetOamTrxAmState(i,locked);
                grr_SubmitAlarm(grr_TrxAmStateLockedAlarmCode[i]);
                for (j=0;j<8;j++)
                {
                     if (OAMgrr_CHN_AMSTATE(i,j) != locked)
                     {
                         OAMgrr_CHN_AMSTATEa(i,j) = locked;
                         grr_SetOamTrxSlotAmState(i,j,locked);
                         grr_SubmitAlarm(grr_TrxSlotAmStateLockedAlarmCode[i][j]);
                     }
                }
            }
        }
   }

   if (i>=OAMgrr_MAX_TRXS)
   {
       //Lock BTS
       OAMgrr_BTS_AMSTATEa = locked;
       grr_SetOamBtsAmState(locked);
       grr_SubmitAlarm(EC_GRR_BTS_LOCKED);
   }

   DBG_LEAVE();
}

#endif //__GRR_PROCUTIL_CPP__
