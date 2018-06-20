
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: H323Interface.H												*
 *																						*
 *	Description			: API Interface for the H323 task								*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#ifndef H323INTERFACE_HDR_INC
#define H323INTERFACE_HDR_INC


#define MOBILE_DISPLAY_INDEX_OFFSET             1
#define MOBILE_DISPLAY_INDEX_WIDTH              8
#define MOBILE_DISPLAY_MCB_OFFSET               MOBILE_DISPLAY_INDEX_OFFSET+MOBILE_DISPLAY_INDEX_WIDTH
#define MOBILE_DISPLAY_MCB_WIDTH                10
#define MOBILE_DISPLAY_IMSI_OFFSET              MOBILE_DISPLAY_MCB_OFFSET+MOBILE_DISPLAY_MCB_WIDTH
#define MOBILE_DISPLAY_IMSI_WIDTH               17
#define MOBILE_DISPLAY_STATE_OFFSET             MOBILE_DISPLAY_IMSI_OFFSET+MOBILE_DISPLAY_IMSI_WIDTH
#define MOBILE_DISPLAY_STATE_WIDTH              25
#define MOBILE_DISPLAY_NUMBER_OFFSET            MOBILE_DISPLAY_STATE_OFFSET+MOBILE_DISPLAY_STATE_WIDTH
#define MOBILE_DISPLAY_NUMBER_WIDTH             15
#define MOBILE_DISPLAY_LUDBID_OFFSET            MOBILE_DISPLAY_NUMBER_OFFSET+MOBILE_DISPLAY_NUMBER_WIDTH
#define MOBILE_DISPLAY_LUDBID_WIDTH             10
#define MOBILE_DISPLAY_DISPLAY_LENGTH           MOBILE_DISPLAY_LUDBID_OFFSET+MOBILE_DISPLAY_LUDBID_WIDTH+50

#define CALL_DISPLAY_INDEX_OFFSET               1
#define CALL_DISPLAY_INDEX_WIDTH                7
#define CALL_DISPLAY_CCB_OFFSET                 CALL_DISPLAY_INDEX_OFFSET+CALL_DISPLAY_INDEX_WIDTH
#define CALL_DISPLAY_CCB_WIDTH                  10
#define CALL_DISPLAY_CALLSTATE_OFFSET           CALL_DISPLAY_CCB_OFFSET+CALL_DISPLAY_CCB_WIDTH
#define CALL_DISPLAY_CALLSTATE_WIDTH            35
#define CALL_DISPLAY_REMOTE_PARTY_OFFSET        CALL_DISPLAY_CALLSTATE_OFFSET+CALL_DISPLAY_CALLSTATE_WIDTH
#define CALL_DISPLAY_REMOTE_PARTY_WIDTH         20
#define CALL_DISPLAY_RTP_HANDLE_OFFSET          CALL_DISPLAY_REMOTE_PARTY_OFFSET+CALL_DISPLAY_REMOTE_PARTY_WIDTH
#define CALL_DISPLAY_RTP_HANDLE_WIDTH           11
#define CALL_DISPLAY_DISPLAY_LENGTH             CALL_DISPLAY_RTP_HANDLE_OFFSET+CALL_DISPLAY_RTP_HANDLE_WIDTH+50

#define ASCII_STAR_CHAR                         42






void H323MessageHandler(PVOIP_API_MESSAGE pMsg);
HJCRTPSESSION H323CallGetRTPHandle(PCALL_CONTROL_BLOCK pCCB);
void InterfaceCallSetup(PVOIP_API_MESSAGE pMsg);
void InterfaceCallAnswer(PVOIP_API_MESSAGE pMsg);
void InterfaceCallAlerting(PVOIP_API_MESSAGE pMsg);
void InterfaceCallRelease(PVOIP_API_MESSAGE pMsg);
void InterfaceShutdownH323Task(void);
void InterfaceSendDtmf(PVOIP_API_MESSAGE pMsg);
void InterfaceCallTransfer(PVOIP_API_MESSAGE pMsg);
bool SendMessage(PRADPIPE pInputPipe,PVOIP_API_MESSAGE pMsg);
bool SendMessageToRtp(PRADPIPE pInputPipe,RtpInputMessage_t *pMsg);

void VoipShowAllCalls(void);
void VoipShowCall(PCALL_CONTROL_BLOCK pCCB);

#ifndef LOOPBACK_MODE
int SysCommand_Voip(T_SYS_CMD action);
#endif

#ifdef LOOPBACK_MODE
RTP_READ_STATUS RTPEventHandler( HJCRTPSESSION hRTP, VOIP_CALL_HANDLE CallHandle, TXN_ID TxnId);
#endif
void InterfaceCallProceeding(PVOIP_API_MESSAGE pMsg);

void DisplayCallInfo(PCALL_CONTROL_BLOCK pCCB,int index,int displayIndex);


#endif /* #ifndef H323INTERFACE_HDR_INC */