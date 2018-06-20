/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
//#ifndef __RM_TESTPRINTOAMRMCONFIGDATA_CPP__
//#define __RM_TESTPRINTOAMRMCONFIGDATA_CPP__

#include "rm\rm_head.h"


void rm_TestPrintOamRmConfigData(void)
{
int i,j;

printf("OAMrm_T3L01: %x, %x\n",
        OAMrm_T3L01,
        ((int)(grr_OamData.grrConfigData.rm_t3L01)));
        //0xFF
printf("OAMrm_BS_CC_CHANS: %x, %x\n",
        OAMrm_BS_CC_CHANS,
        ((unsigned char)(grr_OamData.grrConfigData.rm_bsCcChans)));
        //0x01
printf("OAMrm_BSCCCHSDCCH_COMB: %x, %x\n",
        OAMrm_BSCCCHSDCCH_COMB,
        ((unsigned char)(grr_OamData.grrConfigData.rm_bsCcchSdcchComb)));   
        //0x01 
printf("OAMrm_SYSINFO_ONOFF_MAP: %x, %x\n",
        OAMrm_SYSINFO_ONOFF_MAP,
        ((unsigned short)(grr_OamData.grrConfigData.rm_sysInfoOnOffMap)));     
        //0x3E
printf("OAMrm_AIRINTERFACE: %x, %x\n",
        OAMrm_AIRINTERFACE,
	((unsigned char)(grr_OamData.grrConfigData.rm_airInterface)));         
        //0x02
printf("OAMrm_BTS_TSC: %x, %x\n",
        OAMrm_BTS_TSC,
        ((unsigned char)(grr_OamData.grrConfigData.rm_viperCellTsc)));         
        //0x05
printf("OAMrm_SLOTS_PER_TRX: %x, %x\n",
        OAMrm_SLOTS_PER_TRX,
        ((unsigned char)(grr_OamData.grrConfigData.rm_maxSlotPerTrx)));        
        //0x08
printf("OAMrm_MAX_TRXS: %x, %x\n",
        OAMrm_MAX_TRXS,
        ((unsigned char)(grr_OamData.grrConfigData.rm_maxTrxs)));              
        //0x02
printf("OAMrm_MAX_TCHFS: %x, %x\n",
        OAMrm_MAX_TCHFS,
        ((unsigned char)(grr_OamData.grrConfigData.rm_maxTchfs)));             
        //0x0f
printf("OAMrm_MAX_SDCCH4S: %x, %x\n",
        OAMrm_MAX_SDCCH4S,
        ((unsigned char)(grr_OamData.grrConfigData.rm_maxSdcch4s)));           
        //0x04
printf("OAMrm_TRX_ONOFF_MAP: %x, %x\n",
        OAMrm_TRX_ONOFF_MAP,
        ((unsigned char)(grr_OamData.grrConfigData.rm_trxOnOffMap)));          
        //0x01
printf("OAMrm_NETWORKIFCONFIG: %x, %x\n",
        OAMrm_NETWORKIFCONFIG,
        ((unsigned char)(grr_OamData.grrConfigData.rm_networkIfConfig)));      
        //0x00
printf("OAMrm_BCCH_TRX: %x, %x\n",
        OAMrm_BCCH_TRX,
        ((unsigned char)(grr_OamData.grrConfigData.rm_bcchTrx)));              
        //0x00
printf("OAMrm_PREALPHA_TRX: %x, %x\n",
        OAMrm_PREALPHA_TRX,
        ((unsigned char)(grr_OamData.grrConfigData.rm_preAlphaTrx)));          
        //0x00
printf("OAMrm_CCCH_SDCCH_COMB: %x, %x\n",
        OAMrm_CCCH_SDCCH_COMB,
        ((unsigned char)(grr_OamData.grrConfigData.rm_ccchBcchComb)));         
        //0x01
printf("OAMrm_CCCH_CONF: %x, %x\n",
        OAMrm_CCCH_CONF,
	((unsigned char)(grr_OamData.grrConfigData.rm_ccchConf)));             
        //0x01 
        //FS:T_CNI_RIL3_CCCH_CONF:CNI_RIL3_CCCH_CONF_ONE_CCCH_COMBINED_SDCCHS=1
for (i=0;i<2;i++)
for (j=0;j<8;j++)
{
printf("OAMrm_NIM(%d,%d): %x, %x\n", i,j,
        OAMrm_NIM(i,j),
	((unsigned char)(grr_OamData.grrConfigData.rm_nim[i][j])));
        //rm_nim[2][8]: all elements<-0xFF
}

for (i=0;i<2;i++)
for (j=0;j<4;j++)
{
//printf("OAMrm_TRX_SYNTH_SELECT(%d,%d): %x, %x\n",i,j,
//        OAMrm_TRX_SYNTH_SELECT(i,j),
//        ((unsigned char)(j)));
//        //N.A. Y: ULSYNTH_A/B(0/1), DLSYNTH_A/B(2/3)
printf("OAMrm_TRX_SYNTH_SETTINGMSB(%d,%d): %x, %x\n",i,j,
        OAMrm_TRX_SYNTH_SETTINGMSB(i,j),
        ((unsigned char)(grr_OamData.grrConfigData.rm_trxSynthSetting[i][j]>>8)));
        //rm_trxSynthSetting[2][4] all elements<-0x00
printf("OAMrm_TRX_SYNTH_SETTINGLSB(%d,%d): %x, %x\n",i,j,
        OAMrm_TRX_SYNTH_SETTINGLSB(i,j),
        ((unsigned char)(grr_OamData.grrConfigData.rm_trxSynthSetting[i][j])));
       //defined above
}
 
for (i=0;i<2;i++)
{
printf("OAMrm_TRX_ULFPGADELAY_MSB(%d): %x, %x\n",i,
        OAMrm_TRX_ULFPGADELAY_MSB(i),
  	((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[i][0]>>8)));
        //rm_trxFpgaDelay[2][2] all elements<-0x00
printf("OAMrm_TRX_ULFPGADELAY_LSB(%d): %x, %x\n",i,
        OAMrm_TRX_ULFPGADELAY_LSB(i),
        ((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[i][0])));
        //defined above
//printf("OAMrm_TRX_DLFPGADELAY_MSB(%d): %x, %x\n",i,
//        OAMrm_TRX_DLFPGADELAY_MSB(i),
//  	((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[i][1]>>8)));
        //defined above
//printf(OAMrm_TRX_DLFPGADELAY_MSB(%d): %x, %x\n",i,
//       OAMrm_TRX_DLFPGADELAY_MSB(i),
//        ((unsigned char)(grr_OamData.grrConfigData.rm_trxFpgaDelay[i][1])));
//        //defined above
}

for (i=0;i<2;i++)
{
printf("OAMrm_UPLINK_SGAINCTRL_SETTING(%d): %x, %x\n",i,
        OAMrm_UPLINK_SGAINCTRL_SETTING(i),
        ((unsigned short)(grr_OamData.grrConfigData.rm_SGainCtrlSetting[i][0])));
        //rm_SGainCtrlSetting[2][2]:[X][0]<-0x380
printf("OAMrm_DOWNLINK_SGAINCTRL_SETTING(%d): %x, %x\n",i,
        OAMrm_DOWNLINK_SGAINCTRL_SETTING(i),
        ((unsigned char)(grr_OamData.grrConfigData.rm_SGainCtrlSetting[i][1])));
        //rm_SGainCtrlSetting[2][2]:[X][1]<-0xC0
}

printf("OAMrm_SYNCH_CHANINFO: %x, %x\n",
        OAMrm_SYNCH_CHANINFO,
        ((int)(grr_OamData.grrConfigData.rm_synchChanInfo)));
        //0x00
}

//#endif /* __RM_TESTPRINTOAMRMCONFIGDATA_CPP__ */

