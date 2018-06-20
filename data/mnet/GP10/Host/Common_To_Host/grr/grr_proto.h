/*
********************************************************************
**
** (c); Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PROTO_H__
#define __GRR_PROTO_H__

#include "grr_head.h"

//Functions defined in grr_TestUtil.cpp
//

void grr_PrintGsmOamData(int selection=0);
void grr_PrintCellAllocationTable(Table *tbl);
void grr_PrintPlmnPermittedTable(Table *tbl);
void grr_PrintNotAllowedAccessClassTable(Table *tbl);
void grr_PrintChannelTable(Table *tbl);
void grr_PrintCarrierFrequencyTable(Table *tbl);
void grr_PrintAdjCellHandoverTable(Table *tbl);
void grr_PrintAdjCellReselectionTable(Table *tbl);
void grr_PrintBtsBasicPackage(BtsBasicPackage *btsBasicPackage);
void grr_PrintBtsOptionsPackage(BtsOptionsPackage *btsOptionsPackage);
void grr_PrintTransceiverPackage(TransceiverPackage *transceiverPackage);
void grr_PrintFirstTransceiverPackage(FirstTransceiverPackage *firstTransceiverPackage);
void grr_PrintSecondTransceiverPackage(SecondTransceiverPackage *secondTransceiverPackage);
void grr_PrintRadioCarrierPackage(RadioCarrierPackage *radioCarrierPackage);
void grr_PrintFirstRadioCarrierPackage(FirstRadioCarrierPackage *firstRadioCarrierPackage);
void grr_PrintSecondRadioCarrierPackage(SecondRadioCarrierPackage *secondRadioCarrierPackage);
void grr_PrintT31xx(T31xx *t31xx);
void grr_PrintAdjacentCellPackage(AdjacentCellPackage *adjacentCellPackage);
void grr_PrintHandoverControlPackage(HandoverControlPackage *handoverControlPackage);
void grr_PrintPowerControlPackage(PowerControlPackage *powerControlPackage);

void grr_PrintIntBtsBasicPackage(void);
void grr_PrintIntBtsBcchConfigurationPackage(void);
void grr_PrintIntBtsCellAllocationTable(void);
void grr_PrintIntBtsOptionsPackage(void);
void grr_PrintIntFirstTransceiverPackage(void);
void grr_PrintIntSecondTransceiverPackage(void);
void grr_PrintIntT31xxPackage(void);
void grr_PrintIntFirstTrxRadioCarrierPackage(void);
void grr_PrintIntSecondTrxRadioCarrierPackage(void);
void grr_PrintIntAdjacentCellHandoverPackage(void);
void grr_PrintIntAdjacentCellReselectPackage(void);
void grr_PrintIntGrrConfigData(void);

void grr_SetDbgDump(void);
void grr_ResetDbgDump(void);
void grr_SetGprsCtrl(void);
void grr_ResetGprsCtrl(void);
void grr_GprsSetting(void);


//Functions defined in grr_Task.cpp
//
void grr_CreateQ(void);
void grr_Main(void);
int SysCommand_GRR(T_SYS_CMD action);

//Functions defined in grr_SysInfo.cpp
//
void grr_PopulateSi2(T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PopulateSi3(T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PopulateSi4(T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PopulateSi5(void);
void grr_PopulateSi6(void);
void grr_PopulateSi13(void);

//Functions defined in grr_SetOamMib.cpp
//
void grr_SetOamBtsAmState(AdministrativeState state);
void grr_SetOamTrxAmState(unsigned char trx, AdministrativeState state);
void grr_SetOamTrxSlotAmState(unsigned char trx, unsigned char slot, AdministrativeState state);
void grr_SetOamTrxOpState(unsigned char trx, EnableDisable state);
void grr_SetOamAllTrxSlotOpState(unsigned char trx, EnableDisable state);
void grr_SetOamTrxSlotOpState(unsigned char trx, unsigned char slot, EnableDisable state);
void grr_SetOamTrxRcOpState(unsigned char trx, EnableDisable state);

//Functions defined in grr_SendDspMsg.cpp
//
void grr_SendDspSynInf(unsigned char trx);
void grr_SendDspArfcn(unsigned char trx, int swap);
void grr_SendDspPing(int dsp);
void grr_SendDspSlotActivate(unsigned char trx, unsigned char slot);
void grr_SendDspTrxConfig(unsigned char trx);
void grr_SendDspTuneSynth( unsigned char phTrx, unsigned char synth, unsigned char hopping,
                           unsigned char state, unsigned short arfcn); 

//Functions defined in grr_ProcRrmMsg.cpp
//
void grr_ProcRrmMsg(void);
bool grr_ProcRrmValidateMsg(void);
void grr_ProcRrmTrxSlotShutdownAck(unsigned char trx, unsigned char slot);
void grr_ProcRrmTryShutdownTrx(unsigned char trx);
void grr_ProcRrmTryShutdownBts(void);

//Functions defined in grr_ProcRlcMsg.cpp
//
void grr_ProcRlcMsg(void);
bool grr_ProcRlcValidateMsg(void);
void grr_ProcRlcTrxSlotShutdownAck(unsigned char trx, unsigned char slot);
void grr_ProcRlcTryShutdownTrx(unsigned char trx);
void grr_ProcRlcTryShutdownBts(void);

//Functions defined in grr_ProcOamMsg.cpp
//
void grr_ProcOamMsg(void);
void grr_ProcOamBtsBasicPackage(void);
void grr_ProcOamBtsOpStateChange(void);
void grr_ProcOamBtsAmStateChange(void);
void grr_ProcOamBtsAmStateLocked(void);
void grr_ProcOamTrxAmStateLocked(unsigned char trx);
void grr_ProcOamTrxSlotAmStateLocked(unsigned char trx, unsigned char slot);
void grr_ProcOamBtsAmStateUnlocked(void);
void grr_ProcOamTrxAmStateUnlocked(unsigned char trx);
void grr_ProcOamTrxSlotAmStateUnlocked(unsigned char trx, unsigned char slot);
void grr_ProcOamBtsAmStateShuttingdown(void);
void grr_ProcOamTrxAmStateShuttingdown(unsigned char trx);
void grr_ProcOamTrxSlotAmStateShuttingdown(unsigned char trx, unsigned char slot);
void grr_ProcOamBtsOptionsPackage(void);
void grr_ProcOamTransceiverPackage(unsigned char trx);
void grr_ProcOamTrx0AmStateChange(void);
void grr_ProcOamTrx0Slot0AmStateChange(void);
void grr_ProcOamRadioCarrierPackage(unsigned char rc, MibTag tag);
void grr_ProcOamT31xxPackage(void);
void grr_ProcOamAdjacentCellPackage(void);
void grr_ProcOamHandoverControlPackage(void);
void grr_ProcOamPowerControlPackage(void);
void grr_ProcOamResourceConfigData(void);
bool grr_ProcOamValidateMsg(void);

//Functions defined in grr_ProcGrrMsg.cpp
//
void grr_ProcGrrMsg(void);
bool grr_ProcGrrValidateMsg(void);

//Functions defined in grr_ProcDspMsg.cpp
//
void grr_ProcDspMsg(void);
bool grr_ProcDspValidateMsg(int *msgType);
void grr_ProcDspPingAck(void);

//Functions defined in grr_PassOamMib.cpp
//
void grr_PassOamDspBtsPackage(unsigned char trx);
void grr_PassOamDspPwrCtrl(unsigned char trx);
void grr_PassOamDspHoServCell(unsigned char trx);
void grr_PassOamDspHoNeighCell(unsigned char trx, unsigned char cell, unsigned char noCells);
void grr_PassOamDspPwrRedStep(unsigned char trx);
void grr_PassOamMsSystemInformation2(unsigned char trx, unsigned char SI,T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PassOamMsSystemInformation3(unsigned char trx,unsigned char SI,T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PassOamMsSystemInformation4(unsigned char trx,unsigned char SI, T_CNI_RIL3_CELL_BARRED_ACCESS barState);;
void grr_PassOamMsSystemInformation5(unsigned char trx, unsigned char SI, T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PassOamMsSystemInformation6(unsigned char trx, unsigned char SI, T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PassOamMsSystemInformation13(unsigned char trx, unsigned char SI, T_CNI_RIL3_CELL_BARRED_ACCESS barState);
void grr_PassOamMsParams(unsigned char trx);
void grr_PassOamDspParams(unsigned char trx);
void grr_PassOamParams(unsigned char trx);

//Functions defined in grr_OamUtil.cpp
//
void grr_RegOamTrap(void);
void grr_FixOamData(void);
unsigned char grr_GetOamTotalAvailableHoNCells(void);


//Functions defined in grr_init.cpp
//
int grr_InitWatchTimerExpired(int trx);
void grr_InitWatchTimerStop(unsigned char trx);
void grr_InitWatchTimerStart(int trx);
bool grr_InitDspOver(unsigned char trx);
void grr_InitDspMsgProc(unsigned char trx);
void grr_InitTrxMgmtData(unsigned char trx);
void grr_InitWdogTimer(unsigned char trx);
void grr_InitOamTrxPkg(unsigned char trx);
void grr_InitDsp(void);
void grr_InitWaitForRlcRrmGoAck(void);
void grr_InitOam(void);
void grr_Init(void);
bool grr_GotAllDspsUpSignals(char *dsp);
void grr_InitWaitForDspsUpSignal(void);

//Functions defined in grr_GetOamMib.cpp
//
void grr_GetOamBtsBasicPackage(void);
void grr_GetOamBtsOptionPackage(void);
void grr_GetOamBtsFirstTrxPackage(void);
void grr_GetOamBtsSecondTrxPackage(void);
void grr_GetOamBtsFirstRadioCarrierPackage(void);
void grr_GetOamBtsSecondRadioCarrierPackage(void);
void grr_GetOamT31xxTimerStructure(void);
void grr_GetOamAdjacentCellPackage(void);
void grr_GetOamHandoverControlPackage(void);
void grr_GetOamPowerControlPackage(void);
void grr_GetOamGsmData(void);
void grr_GetOamRrmData(void);
void grr_GetOamData(void);

//Functions defined in grr_DspUtil.cpp
//
void grr_RadioCtrl(unsigned char pwr);
void grr_TuneTrxSynth(unsigned char trx, unsigned char pwr);
void grr_StartDspsWatch(void);
void grr_InitDspWdog(unsigned char dsp);
void grr_StartDspWdog(unsigned char dsp);
int grr_ProcDspWdogExpired(int param);
void grr_ActTrxSlot(unsigned char trx, unsigned char slot);
void grr_StartDspWdog(unsigned char dsp);
void grr_FirstRadioCtrl(void);

//Functions defined in grr_glob.cpp
//
void grr_SubmitAlarm(Alarm_errorCode_t code);
void grr_SendMsg(int src_module, MSG_Q_ID dst_q, grr_MessageType_t msgType, int trx, int slot, int ctrl);
void grr_SendMsgFromL1ToGrr(unsigned short length, unsigned char* buf);

//Functions defined in grr_ProcUtil.cpp
//
void grr_ProcUtilTryShutdownTrx(unsigned char trx);
void grr_ProcUtilTryShutdownBts(void);

//Functions defined in grr_RadioCtrl.cpp
//
void ConfigTxPwrValues(unsigned char trx);
void grr_RadioMonitor();

//Functions defined in other modules
//
void sendDsp (unsigned char *buffer, int len);
STATUS l1pTrxSwap(unsigned char trxA, unsigned char trxB);
extern "C" {
	int drfIQSwapGet(void);
}

#endif //__GRR_PROTO_H__

