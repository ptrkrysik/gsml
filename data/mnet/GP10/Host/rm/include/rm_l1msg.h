/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_L1MSG_H__ 
#define __RM_L1MSG_H__

#include "rm/rm_const.h"
//#include "rm/rm_types.h"


typedef struct {
   u8		msgGroup;
   u8		filler;
   u16 		msgType;

} rm_L1TxMsgHeader_t;

typedef struct {
   rm_L1TxMsgHeader_t	 header;
   u8			 trxNumber;
   u8		 	 airIfStd;
   u8 		 	 bsic;
} rm_L1TrxConfig_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u8		 	trxNumber;
   u8			slotNumber;
   u8			chanComb;
} rm_L1SlotActiv_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u8		 	trxNumber;
   u8		 	sysInfoType;
   u8 		 	l3Info[RM_L1MAX_SYSINFO_LEN];
} rm_L1SacchFill_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u8		 	trxNumber;
   u8		 	uplinkSGainCtrlSetting;
} rm_L1SetUGain_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u16		 	trxNumber;
   u8		 	downlinkSGainCtrlSetting;
} rm_L1SetDGain_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u16		 	trxNumber;
   u8		 	synthSelect;
   u8		        synchSettingMSB;
   u8			synchSettingLSB;
} rm_L1TuneSynth_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u16		 	trxNumber;
   u8		 	ulFpgaDelayMSB;
   u8			ulFpgaDelayLSB;
   u8		 	dlFpgaDelayMSB;
   u8		 	dlFpgaDelayLSB;
} rm_L1ConfigFpgaDelay_t;

typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u8			trxNumber;
   u8 			absoluteFrameNo[RM_L1SYNCH_CHANINFO_LEN];
} rm_L1Synch_t;
  
typedef struct {
   rm_L1TxMsgHeader_t 	header;
   u8		 	trxNumber;
   u8		 	chanNumberMSB;
   u8		 	chanNumberLSB;
} rm_L1BcchInfo_t;

typedef union
{
   rm_L1TxMsgHeader_t		header;
   rm_L1TrxConfig_t		rm_L1TrxConfig;
   rm_L1SlotActiv_t		rm_L1SlotActiv;
   rm_L1SacchFill_t		rm_L1SacchFill;
   rm_L1SetUGain_t		rm_L1SetUGain;
   rm_L1SetDGain_t		rm_L1SetDGain;
   rm_L1TuneSynth_t		rm_L1TuneSynth;
   rm_L1ConfigFpgaDelay_t 	rm_L1ConfigFpgaDelay;
   rm_L1Synch_t			rm_L1Synch;
   rm_L1BcchInfo_t		rm_L1BcchInfo;
} rm_L1TxMsg_t;

#if 0
typedef union   //TBD
{
   rm_L1RxMsgHeader_t	header;
   rm_L1TrxConfigAck_t	rm_L1TrxConfigAck;  <TBD>
   rm_L1SlotActivAck_t	rm_L1SlotActivAck;  <TBD>
   rm_L1SlotActivNAck_t	rm_L1SlotActivNAck; <TBD>
   rm_L1SynchAck_t	rm_L1SynchAck;      <TBD>

} rm_L1RxMsg_t

#endif 

typedef struct {
	u16 msgLength;
	u8  buffer[RM_L1MAX_SYSINFO_LEN];
} rm_L1SysInfo_t;

typedef struct {
	u8	synthSelect;	/* UL Synth A/B, DL Synth A/B */
	u8	synthSettingMSB;
	u8	synthSettingLSB;
	u8	filler;
} rm_L1TrxSynthSetting_t;

typedef struct {
	u8	ulFpgaDelayMSB;
	u8	ulFpgaDelayLSB;
	u8	dlFpgaDelayMSB;
	u8	dlFpgaDelayLSB;
} rm_L1TrxFpgaDelay_t;

#endif /* __RM_L1MSG_H__ */
