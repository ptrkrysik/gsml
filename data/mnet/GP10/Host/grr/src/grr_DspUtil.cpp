/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_DSPUTIL_CPP__
#define __GRR_DSPTTIL_CPP__

#include "grr\grr_head.h"

void grr_FirstRadioCtrl(void)
{
  int i;

  DBG_FUNC("grr_FirstRadioCtrl", GRR_LAYER_DSP);
  DBG_ENTER();

  for (i=0;i<OAMgrr_MAX_TRXS;i++)
  {
       switch(OAMgrr_TRX_AMSTATE(i))
       {
       case unlocked:
            grr_TuneTrxSynth(i,GRR_ON);
            break;
       case locked:
	    grr_SubmitAlarm(grr_TrxAmStateLockedAlarmCode[i]);
	    break;
       case shuttingDown:
       default: 
	    DBG_WARNING("grr_FirstRadioCtrl: trx(%d) initialization failed due to improper amState(%d)\n",
                       i, OAMgrr_TRX_AMSTATE(i));
	    printf("grr_FirstRadioCtrl: trx(%d) initialization failed due to improper amState(%d)\n",
                       i, OAMgrr_TRX_AMSTATE(i));
          if (OAMgrr_TRX_AMSTATE(i)==shuttingDown)
              grr_SubmitAlarm(grr_TrxAmStateShuttingdownAlarmCode[i]);
          else
              grr_SubmitAlarm(grr_TrxAmStateInvalidAlarmCode[i]);

	    assert(0);
          break;
      }
  }
  DBG_LEAVE();
}

void grr_RadioCtrl(unsigned char pwr)
{
   int i;

   DBG_FUNC("grr_RadioCtrl", GRR_LAYER_DSP);
   DBG_ENTER();

   DBG_TRACE("grr_RadioCtrl: set radio power(%d)\n", pwr);

   if (pwr>1)
   {
      DBG_WARNING("grr_RadioCtrl: invalid radio power setting(%d)\n", pwr);
      DBG_LEAVE();
      return;
   }

   for (i=0;i<OAMgrr_MAX_TRXS;i++)
   {
        grr_TuneTrxSynth(i,pwr);
   }

   DBG_LEAVE();
}

void grr_TuneTrxSynth(unsigned char trx, unsigned char pwr)
{
   DBG_FUNC("grr_TuneTrxSynth", GRR_LAYER_DSP);
   DBG_ENTER();

   DBG_TRACE("grr_TuneTrxSynth: set power(%d) for trx(%d)\n", pwr, trx);
   
   //Apply sanitary check
   if (trx>=OAMgrr_MAX_TRXS)
   {   
       DBG_WARNING("grr_TuneTrxSynth: invalid trx(%d)\n", trx);
       DBG_LEAVE();
   }
   
   grr_SendDspArfcn(trx, OAMgrr_BCCH_CARRIER_phTRX);

   switch(trx)
   {
   case 0:
        if (OAMgrr_BCCH_CARRIER_phTRX)
        {  //phTRX 1 as bcch carrier trx ie trx-es swapped scenario
           grr_SendDspTuneSynth(0, 0, OAMgrr_HOPPING_CTRL, pwr, OAMgrr_ARFCN(0)); 
		                //phTRX, syth, Hopping, pwrSetting, trxPackage
        } else
        {  //phTRX 0 as bcch carrier trx ie trx-es non-swapped scenario
           grr_SendDspTuneSynth(1, 0, OAMgrr_HOPPING_CTRL, pwr, OAMgrr_ARFCN(0)); 
		                //phTRX, syth, Hopping, pwrSetting, trxPackage
           grr_SendDspTuneSynth(1, 1, OAMgrr_HOPPING_CTRL, pwr, OAMgrr_ARFCN(0)); 
		                //phTRX, syth, Hopping, pwrSetting, trxPackage
        }
        break; 
   case 1:
        if (OAMgrr_BCCH_CARRIER_phTRX)
        {  //phTRX 1 as bcch carrier trx ie trx-es swapped scenario
           grr_SendDspTuneSynth(1, 0, OAMgrr_HOPPING_CTRL, pwr, OAMgrr_ARFCN(1)); 
		                //phTRX, syth, Hopping, pwrSetting, trxPackage
           grr_SendDspTuneSynth(1, 1, OAMgrr_HOPPING_CTRL, pwr, OAMgrr_ARFCN(1)); 
		                //phTRX, syth, Hopping, pwrSetting, trxPackage   
        } else
        {  //phTRX 0 as bcch carrier trx ie trx-es non-swapped scenario
           grr_SendDspTuneSynth(0, 0, OAMgrr_HOPPING_CTRL, pwr, OAMgrr_ARFCN(1)); 
		                //phTRX, syth, Hopping, pwrSetting, trxPackage
        }
        break; 
   }

   DBG_LEAVE();
}


void grr_StartDspsWatch(void)
{  
   unsigned char i;

   DBG_FUNC("grr_StartDspsWatch", GRR_LAYER_DSP);
   DBG_ENTER();

  //Start keep-alive watch dogs
  for (i=0;i<OAMgrr_MAX_TRXS;i++)
  {
       grr_InitDspWdog(i);
       grr_StartDspWdog(i);
       grr_SendDspPing(i);
  }

  DBG_LEAVE();
}

void grr_InitDspWdog(unsigned char dsp)
{
  int i;
  WDOG_ID       timer_id;

  DBG_FUNC("grr_InitDspWdog", GRR_LAYER_DSP);
  DBG_ENTER();

  //Create watch dog timer for the trx
  if ( (timer_id=wdCreate()) == NULL )
  {
        grr_SubmitAlarm(EC_GRR_OS_CALL_WDCREATE_FAILED);
        DBG_ERROR("grr_InitDspWdog:failed to create dsp(%d) watch dog timer\n", dsp);
        printf("grr_InitDspWdog:failed to create dsp(%d) watch dog timer\n", dsp);
        assert(0);
  }
  grr_TrxMgmt[dsp].wdog.timer_id = timer_id;
  grr_TrxMgmt[dsp].wdog.time     = OAMgrr_WDOG_TIME;
  grr_TrxMgmt[dsp].wdog.state    = false;
 
  DBG_LEAVE();
}


//Start DSP watch dog
void grr_StartDspWdog(unsigned char dsp)
{
   DBG_FUNC("grr_StartDspWdog", GRR_LAYER_DSP);
   DBG_ENTER();
 
   if ( ERROR == (wdStart(grr_TrxMgmt[dsp].wdog.timer_id, 
                          OAMgrr_WDOG_TIME,(FUNCPTR)grr_ProcDspWdogExpired, 
                          ((dsp<<8)|GRR_WDOG) ) ) )
   {
        grr_SubmitAlarm(EC_GRR_OS_CALL_WDSTART_FAILED);
        DBG_ERROR("grr_StartDspWdog:failed to call wdstart for dsp(%d) wdog timerId(%x) errno(%d)\n",
                   dsp, grr_TrxMgmt[dsp].wdog.timer_id, errno);
        printf("grr_StartDspWdog:failed to call wdstart for dsp(%d) wdog timerId(%x) errno(%d)\n",
                   dsp, grr_TrxMgmt[dsp].wdog.timer_id, errno);

        assert(0);
   }

   DBG_LEAVE();
}


int grr_ProcDspWdogExpired(int param)
{
   STATUS ret;

   DBG_FUNC("grr_ProcDspWdogExpired", GRR_LAYER_DSP);
   DBG_ENTER();

   grr_ItcRxGrrMsg_t msg;

   msg.module_id      = MODULE_GRR;
   msg.primitive_type = GRR_DSP_WDOG_EXPIRED;
   msg.trx            = (unsigned char)(param>>8);
   msg.timer_id	      = (unsigned char)(param);

   //
   if (STATUS_OK != (ret = msgQSend(grr_MsgQId, (char*)&msg, GRR_MAX_GRRMSG_LEN,
		                    NO_WAIT, MSG_PRI_NORMAL) ) )
   {
       grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQSEND_FAILED);
       DBG_ERROR("grr_ProcDspWdogExpired: msgQSend failed Q(%x) ret(%d) errno(%d) dsp(%d) timer(%d)\n",
                  grr_MsgQId, ret, errno, (unsigned char)(param>>8), (unsigned char)(param));
       printf("grr_ProcDspWdogExpired: msgQSend failed Q(%x) ret(%d) errno(%d) dsp(%d) timer(%d)\n",
                  grr_MsgQId, ret, errno, (unsigned char)(param>>8), (unsigned char)(param));
       assert(0);
   }

   DBG_LEAVE();
}

//Acitvate the given trx and slot
void grr_ActTrxSlot(unsigned char trx, unsigned char slot)
{
   DBG_FUNC("grr_ActTrxSlot", GRR_LAYER_DSP);
   DBG_ENTER();

   //safeguard only
   if ( (trx>=OAMgrr_MAX_TRXS) || (slot>=8) )
   {   
       DBG_WARNING("grr_ActTrxSlot: invalid time slot trx(%d) slot(%d)\n",
                    trx, slot);
   } else
       //Activate the given trx and slot
       grr_SendDspSlotActivate(trx,slot);
 
   DBG_LEAVE();
} 


#endif //__GRR_DSPUTIL_CPP__
