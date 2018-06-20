/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SENDTUNESYNTHTOTRX_CPP__
#define __RM_SENDTUNESYNTHTOTRX_CPP__

#include "rm\rm_head.h"

void rm_SendTuneSynthToTrx(u8 trx,u8 pwr)
{
   static u8 m_pwr[2]={0,0};

   //Check if tune make sense 
   //if (m_pwr[trx] == pwr) 
   //{
   //    PDEBUG__(("INFO@rm_SendTuneSynthToTrx: nothing changed, thus fruitless tune(%d,%d)\n",
   //               trx,pwr));
   //    return;
   //}

   //Go ahead to tuen and rememebr the current pwr state
   m_pwr[trx] = pwr;

   //G2TEST

   //Monitoring entrance to a function 
   //RDEBUG__(("ENTER-rm_SendTuneSynthToTrx: Entering...\n" ));
 
   //Tune physical TRX syntheseizer according to configuration data 
   //PDEBUG__(("INFO@rm_SendTuneSynthToTrx: tune trx synth: (%d,%d)\n",trx,pwr));

   //   if (rm_TrxMgmt[trx].state==RM_TRXMGMT_SNULL ||
   //       rm_TrxMgmt[trx].state==RM_TRXMGMT_S0_BOOTSTRAP_IND)
   //    return;

   rm_SendArfcnToTrx(trx, OAMrm_BCCH_CARRIER_phTRX);

   switch(trx)
   {
   case 0:
        if (OAMrm_BCCH_CARRIER_phTRX)
        {  //phTRX 1 as bcch carrier trx ie trx-es swapped scenario
           rm_TuneSynth(0, 0, OAMrm_HOPPING_CTRL, pwr, OAMrm_ARFCN(0)); 
		            //phTRX, syth, Hopping, pwrSetting, trxPackage
        } else
        {  //phTRX 0 as bcch carrier trx ie trx-es non-swapped scenario
           rm_TuneSynth(1, 0, OAMrm_HOPPING_CTRL, pwr, OAMrm_ARFCN(0)); 
		            //phTRX, syth, Hopping, pwrSetting, trxPackage
           rm_TuneSynth(1, 1, OAMrm_HOPPING_CTRL, pwr, OAMrm_ARFCN(0)); 
		            //phTRX, syth, Hopping, pwrSetting, trxPackage
        }
        break; 
   case 1:
        if (OAMrm_BCCH_CARRIER_phTRX)
        {  //phTRX 1 as bcch carrier trx ie trx-es swapped scenario
           rm_TuneSynth(1, 0, OAMrm_HOPPING_CTRL, pwr, OAMrm_ARFCN(1)); 
		            //phTRX, syth, Hopping, pwrSetting, trxPackage
           rm_TuneSynth(1, 1, OAMrm_HOPPING_CTRL, pwr, OAMrm_ARFCN(1)); 
		            //phTRX, syth, Hopping, pwrSetting, trxPackage   
        } else
        {  //phTRX 0 as bcch carrier trx ie trx-es non-swapped scenario
           rm_TuneSynth(0, 0, OAMrm_HOPPING_CTRL, pwr, OAMrm_ARFCN(1)); 
		            //phTRX, syth, Hopping, pwrSetting, trxPackage
        }
        break; 

   default:
        EDEBUG__(("WARNING@rm_SendTuneSynthToTrx: invalid trx=%d\n",trx));
        break;
   }
}


void rm_TuneSynth(u8 phTrx, u8 synth, u8 hopping, u8 state, u16 arfcn) 
{
   rm_ItcTxMsg_t  msgToL1;     
   u16		length,setting;

   //G2TEST
   //PDEBUG__(("INFO@rm_TuneSynth: swapInd=%d, hopping=%d, tuneSetting:(%d,%d,%d,%d)\n",
   //           OAMrm_BCCH_CARRIER_phTRX, hopping, phTrx, synth, state, arfcn));
   printf("INFO@rm_TuneSynth: swapInd=%d, hopping=%d, tuneSetting:(%d,%d,%d,%d)\n",
              OAMrm_BCCH_CARRIER_phTRX, hopping, phTrx, synth, state, arfcn);

   //Fill TUNE SYNTH message's header portion
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_TUNESYNTH_MSB;	//0x12
   msgToL1.buffer[length++] = RM_L1MT_TUNESYNTH_LSB;	//0x00
   msgToL1.buffer[length++] = 0;   //Always sent ot BCCH DSP
   
   setting = 0;
   setting = ( OAMrm_phTRX_SELECT(phTrx)    | OAMrm_phTRX_SYNTH_SELECT(synth) |
               OAMrm_phTRX_HOPPING(hopping) | OAMrm_phTRX_PWR_SELECT(state)   |
               (0x3FF&arfcn) );

   msgToL1.buffer[length++] = (unsigned char)(setting>>8);
   msgToL1.buffer[length++] = (unsigned char)(setting);
   msgToL1.msgLength = length;
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer); 
}

void rm_SendArfcnToTrx(u8 trx, int swap)
{
   rm_ItcTxMsg_t  msgToL1;     
   u16		length,arfcn;
   
   switch(trx)
   {
   case 0:
        arfcn = OAMrm_ARFCN(0);
        break;
   case 1:
        arfcn = OAMrm_ARFCN(1);
        break;

   default:
        return;
   }
   
   PDEBUG__(("INFO@rm_SendArfcnToTrx: trx=%d, swap=%d, arfcn=%d\n",
              trx, swap, arfcn));

   //Fill TUNE SYNTH message's header portion
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;		//0x10
   msgToL1.buffer[length++] = RM_L1MT_ARFCN_MSB;	//0x29
   msgToL1.buffer[length++] = RM_L1MT_ARFCN_LSB;	//0x00
   msgToL1.buffer[length++] = trx;
   msgToL1.buffer[length++] = (u8)(arfcn>>8);
   msgToL1.buffer[length++] = (u8)(arfcn);
   msgToL1.msgLength = length;
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer); 
}

#endif //__RM_SENDTUNESYNTHTOTRX_CPP__
