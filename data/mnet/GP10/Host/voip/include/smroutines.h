
/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved								*
 *																						*
 *	File				: smroutines.h													*
 *																						*
 *	Description			: State Machine functions/procedures header						*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 ****************************************************************************************
 */

#ifndef SMROUTINES_HDR_INC
#define SMROUTINES_HDR_INC

#include "ccb.h"
#include "ril3/ie_called_bcd.h"
#include "ril3/ie_calling_bcd.h"


void RunStateMachine(PCALL_CONTROL_BLOCK pCCB,PVOID Context, H323_CALL_EVENT Event);
STATUS smCallOffering(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallSetup(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smSetupFastChannels(PCALL_CONTROL_BLOCK pCCB);
STATUS smDropCall(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smAnswerCall(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallConnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallDisconnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallProceeding(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallRingBack(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallDestinationRejected(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smCallIdle(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smRecvRemoteCapabilities(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smH245TransportConnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context);
STATUS smH245Connected(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smRecvCapabilitiesAck(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smRecvCapabilitiesNak(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smChannelOffering(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smInChannelDisconnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context);
STATUS smInChannelConnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context);
STATUS smOutChannelConnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context);
STATUS smOutChannelDisconnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context);
char *GetH323CallStateName(H323_CALL_STATE CallState);
char *GetH323EventName(H323_CALL_EVENT CallEvent);
char* GetCallStateModeName(int Mode);
void UpdateStateTransitionHistory(PCALL_CONTROL_BLOCK pCCB,H323_CALL_STATE LatestState,H323_CALL_EVENT LatestEvent,H323_CALL_STATE NextState);
STATUS smFatalStateMachineViolation(PCALL_CONTROL_BLOCK pCCB,PVOID  Context);
STATUS smNonFatalStateMachineViolation(PCALL_CONTROL_BLOCK pCCB,PVOID  Context);
void SendLocalTermCapSet(PCALL_CONTROL_BLOCK pCCB);
void SendCodecInfo(PCALL_CONTROL_BLOCK pCCB);
BOOL SendProgressMessageToCC(PCALL_CONTROL_BLOCK pCCB);
char *GetVoipNumTypeStr(Q931_IE_CALL_PARTY_NO_TYPE numType);
char *GetVoipNumPlanStr(Q931_IE_CALL_PARTY_NO_PLAN numPlan);
char *GetVoipPIStr(Q931_IE_CALLING_PARTY_NO_PI PI);
char *GetVoipSIStr(Q931_IE_CALLING_PARTY_NO_SI SI);
void VoipDisplayCallParties(PCALL_CONTROL_BLOCK pCCB);
void VoipToGsmNumType(Q931_IE_CALL_PARTY_NO_TYPE voipNumType,T_CNI_RIL3_TYPE_OF_NUMBER *gsmNumType);
void VoipToGsmNumPlan(Q931_IE_CALL_PARTY_NO_PLAN voipNumPlan,T_CNI_RIL3_NUMBERING_PLAN_ID *gsmNumPlan);
void VoipToGsmPI(Q931_IE_CALLING_PARTY_NO_PI voipPI,T_CNI_RIL3_PRESENTATION_INDICATOR *gsmPI);
void VoipToGsmSI(Q931_IE_CALLING_PARTY_NO_SI voipSI,T_CNI_RIL3_SCREENING_INDICATOR *gsmSI);
void GsmToVoipNumType(T_CNI_RIL3_TYPE_OF_NUMBER gsmNumType,Q931_IE_CALL_PARTY_NO_TYPE *voipNumType);
void GsmToVoipNumPlan(T_CNI_RIL3_NUMBERING_PLAN_ID gsmNumPlan,Q931_IE_CALL_PARTY_NO_PLAN *voipNumPlan);
void GsmToVoipPI(T_CNI_RIL3_PRESENTATION_INDICATOR gsmPI,Q931_IE_CALLING_PARTY_NO_PI *voipPI);
void GsmToVoipSI(T_CNI_RIL3_SCREENING_INDICATOR gsmSI,Q931_IE_CALLING_PARTY_NO_SI *voipSI);


#endif