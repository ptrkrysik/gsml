
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: Util.h														*
 *																						*
 *	Description			: Hdr file for utilities program file							*
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

#ifndef UTIL_HDR_INC
#define UTIL_HDR_INC

#include "ccb.h"




#define ArraySize( Array ) ( sizeof( Array ) / sizeof( Array[0] ) )


PRADPIPE RadPipeOpen( char* Name, int nMsgSize, int nMaxMsgs, pfnCallBackFunction pfnUserCallBack);
BOOL RadPipeWrite( PRADPIPE pPipe, PVOID pMessage);
void RadPipeCallBack( int hPipe, piEvents Event, BOOL Error, void* pData);
BOOL RadPipeClose( PRADPIPE pPipe);
REMOTE_TERMINAL_TYPE DetectRemoteTerminalType(PCALL_CONTROL_BLOCK pCCB);
char* GetCallStateName( int State);
char* GetCallStateModeName(int Mode);
char* GetRASEventName( int Event);
char* GetChannelStateName( int State);
char* GetChannelStateModeName(int Mode);
char* GetControlStateName(int State);
void CapabilityPrint(cmCapStruct* pCaps);
void jcULongPack( ULONG uLong, UCHAR** ppszBuffer, int* pnSize);
ULONG jcULongUnpack( UCHAR** ppszBuffer);
void jcUShortPack( USHORT uShort, UCHAR** ppszBuffer, int* pnSize);
USHORT jcUShortUnpack( UCHAR** ppszBuffer);
void jcBufferPack( PVOID pData, USHORT nDataSize, UCHAR** ppszBuffer, int* pnSize );
PVOID jcBufferUnpack( USHORT nDataSize, UCHAR** ppszBuffer );
char* jcIpToString( UINT32 addr, UINT16 port );
void jcStringPack( char* pszString, UCHAR** ppszBuffer, int* pnSize );
char* jcStringUnpack( UCHAR** ppszBuffer );
PJCTIMER TimerCreate(UINT32 tmInterval, pfnCommonCallBack pfnCallBack, PVOID pData, BOOL bMultiple );
void VxWorksTimerCallBack( PVOID pData );
BOOL TimerDelete( PJCTIMER pTimer );
void DumpCCBAndRtp(void);
char* jcBCDToString( char* String, unsigned char* Digits, short nDigits );
PAPI_MESSAGE GetFreeMsgFromPool(void);
void PutMsgIntoPool(PAPI_MESSAGE pMsg);

PJCTIMER AllocTimerFromPool(void);
void ReturnTimerToPool(PJCTIMER pTimer);
void TimerArrayInit(void);
void PipeArrayInit(void);
void jcStringToBCD(char *string,unsigned char *digits, short *nDigits);
char *ExtractTelString(char *buffer);
char *GetQ850CauseCodeName(unsigned char cause_value);

#endif