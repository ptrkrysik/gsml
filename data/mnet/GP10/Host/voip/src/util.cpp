
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: Util.cpp														*
 *																						*
 *	Description			: Various utility functions required by the H.323 task			*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *						  [Code borrowed liberally from original work 					*
 *						  of Oleg Turovski]												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <cm.h>
#include <stkutils.h>
}

#include <tickLib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>
extern "C" {
#include <mti.h>
#include <seli.h>
#include <pi.h>
}
#include "logging\vcmodules.h"
#include "logging\vclogging.h"

#include "ccb.h"
#include "util.h"

extern PH323TASK_CONFIG	    pVoipTaskConfig;
extern CALL_CONTROL_BLOCK	CCBArray[MAX_ACTIVE_CALLS];

extern RADPIPE	RadPipeArray[MAX_PIPES_IN_ARRAY];
extern JCTIMER VoipTaskTimerArray[MAX_TIMERS_IN_ARRAY];
extern API_MESSAGE	VoipMessageArray[MAX_VOIP_API_MSGS_IN_ARRAY];


/*
 *  RadPipeOpen
 */
PRADPIPE 
RadPipeOpen( char* Name, int nMsgSize, int nMaxMsgs, pfnCallBackFunction pfnUserCallBack )
{
	DBG_FUNC("RadPipeOpen",H323_LAYER);
	DBG_ENTER();
   	int hPipe;
   	PRADPIPE pPipe=NULL;


   	if(piInit()==ERROR)
   	{
      DBG_ERROR( "Error in piInit()\n" );
	  DBG_LEAVE();
      return pPipe;
   	}

   	if(( hPipe = piOpen( Name, nMsgSize, nMaxMsgs ) ) &&
      0 > ( hPipe = open( Name, O_RDWR, 644 ) ) == ERROR )
   	{
      DBG_ERROR( "Error in piOpen()\n" );
	  DBG_LEAVE();
      return pPipe;
   	}

   	pPipe = (PRADPIPE)GetPipeFromPool();
   	ASSERT(pPipe != NULL);
   	if (pPipe == NULL)
   	{
		DBG_ERROR("Cannot allocate memory for pipe!\n");
		DBG_LEAVE();
		return pPipe;
   	}
   	pPipe->hPipe 					= hPipe;
   	pPipe->pfnUserCallBackFunction 	= pfnUserCallBack;
   	pPipe->nMsgSize 				= nMsgSize;
	DBG_TRACE("Pipe message size %d = %d\n",pPipe->nMsgSize,nMsgSize);
   	//pPipe->pMsg 					= (PVOID)jcMalloc(H323_LAYER,nMsgSize);

   	piCallOn( hPipe, piEvRead, (piCallback)RadPipeCallBack, pPipe );

	DBG_LEAVE();
   	return pPipe;
}



/*
 * GetPipeFromPool
 */
PRADPIPE 
GetPipeFromPool()
{
	int Count;
	PRADPIPE	pPipe=NULL;

	for(Count=0;Count<MAX_PIPES_IN_ARRAY;Count++) {
		pPipe = (PRADPIPE)&RadPipeArray[Count];
		if (pPipe->Used == FALSE) {
			pPipe->Used = TRUE;
			pPipe->Magic = H323TASK_MAGIC_VALUE;
			memset(pPipe->pMsg,0,MAX_PIPE_MSG_SIZE);
			return (pPipe);
		}
	}	
	return(NULL);
}

/*
 * ReturnPipeToPool
 */
void
ReturnPipeToPool(PRADPIPE pRadPipe)
{
	memset(pRadPipe->pMsg,0,MAX_PIPE_MSG_SIZE);
	pRadPipe->Used = FALSE;

}

/*
 * PipeArrayInit
 */
void
PipeArrayInit()
{
	int ArraySize;

	ArraySize = sizeof(RADPIPE) * MAX_PIPES_IN_ARRAY;

	memset(RadPipeArray,0,ArraySize);

}

/*
 * RadPipeWrite
 */
BOOL 
RadPipeWrite( PRADPIPE pPipe, PVOID pMessage )
{
	DBG_FUNC("RadPipeWrite",H323_LAYER);
	DBG_ENTER();

	ASSERT(pPipe->Magic == H323TASK_MAGIC_VALUE);
   	if( pPipe->nMsgSize != piWrite( pPipe->hPipe, (UINT8*)pMessage, pPipe->nMsgSize ) )
   	{
      DBG_ERROR( "Error in piWrite()\n" );
	  DBG_LEAVE();
      return FALSE;
   	}

	DBG_LEAVE();
   	return TRUE;
}



/*
 * RadPipeCallBack
 */
void 
RadPipeCallBack( int hPipe, piEvents Event, BOOL Error, void* pData )
{
	DBG_FUNC("RadPipeCallBack",H323_LAYER);
	DBG_ENTER();

   	PRADPIPE pPipe;
   	int i, nPipeMsgs;
	int BytesRead;

   	pPipe = (PRADPIPE)pData;

   	if( Error || (pPipe->hPipe != hPipe))
   	{
      DBG_ERROR( "Error in RadPipeCallBack()\n" );
	  DBG_LEAVE();
      return;
   	}

   	ioctl( hPipe, FIONMSGS, (int)&nPipeMsgs );

   	for( i = 0; i < nPipeMsgs; i++ )
   	{
		BytesRead = piRead( hPipe, (UINT8*)pPipe->pMsg, pPipe->nMsgSize ) ;
      	if( pPipe->nMsgSize != BytesRead)
      	{
         	DBG_ERROR( "Error in piRead(), expected size %d, read size %d\n",pPipe->nMsgSize,BytesRead);
			DBG_LEAVE();
         	return;
      	}

      	(*pPipe->pfnUserCallBackFunction)( pPipe->pMsg );
   }
   
   DBG_LEAVE();
   return;
}


/*
 *	RadPipeClose
 */
BOOL 
RadPipeClose( PRADPIPE pPipe )
{
	DBG_FUNC("RadPipeClose",H323_LAYER);
	DBG_ENTER();

   	piClose( pPipe->hPipe );
   	piEnd();

/*
   	jcFree(H323_LAYER, pPipe->pMsg );
   	jcFree(H323_LAYER,pPipe);
*/
	ReturnPipeToPool(pPipe);

	DBG_LEAVE();
   	return TRUE;
}



/*
 * DetectRemoteTerminalType
 */
REMOTE_TERMINAL_TYPE 
DetectRemoteTerminalType(PCALL_CONTROL_BLOCK pCCB)
{
	DBG_FUNC("DetectRemoteTerminalType",H323_LAYER);
	DBG_ENTER();
   	HPVT hPVT;
   	int hRemoteInfo, hNodeID, nLen;
   	BOOL bString;
   	char ProductID[100], VersionID[100];

   	hRemoteInfo = cmCallGetRemoteEndpointInfoHandle( pCCB->LanCallHandle);
   	hPVT = cmGetValTree( pVoipTaskConfig->StackHandle );

   	hNodeID = pvtGetByPath( hPVT, hRemoteInfo, "vendor.productId", NULL, &nLen, &bString );
   	if( bString )
   	{
      	pvtGetString( hPVT, hNodeID, 100, ProductID );
      	ProductID[nLen] = 0;
		DBG_TRACE("Remote endpoint product ID : %s\n",ProductID);
   	}
   	else
      	strcpy( ProductID, "<unknown>" );

   	hNodeID = pvtGetByPath( hPVT, hRemoteInfo, "vendor.versionId", NULL, &nLen, &bString );
   	if( bString )
   	{
      	pvtGetString( hPVT, hNodeID, 100, VersionID );
      	VersionID[nLen] = 0;
   	}
   	else
      	strcpy( VersionID, "<unknown>" );

   	DBG_TRACE( "Remote Terminal Info for (call %x):\n\tProductID: '%s', VersionID: '%s'\n",
             pCCB->LanCallHandle, ProductID, VersionID );

    if ( 0 == strncmp( ProductID, "Microsoft", 9 )) {
	  	DBG_TRACE("Remote Terminal type MS-NetMeeting \n");
      	pCCB->RemoteType = REMOTE_TERMINAL_NETMEETING;
    }
    else if ( 0 == strncmp( ProductID, "IPLink", 6 ))
 	{
	  	DBG_TRACE("Remote Terminal type Dialogic-based\n");
      	pCCB->RemoteType = REMOTE_TERMINAL_NETMEETING;
   	}
   	else
      	pCCB->RemoteType = REMOTE_TERMINAL_GSM; /* at least we hope so :-) */

	DBG_LEAVE();
   	return pCCB->RemoteType;
}




/*
 * GetCallStateName
 */
char* 
GetCallStateName( int State )
{

   	static char* Names[] =
   	{ 
	   "Dialtone", 
	   "Proceeding", 
	   "RingBack", 
	   "Connected", 
	   "Disconnected", 
	   "Idle", 
	   "Offering",
	   "Transfering", 
	   "AdmissionConfirm", 
	   "AdmissionReject", 
	   "IncompleteAddress" 
   	};

   	if( State >= 0 && State < 11)
   	{
      	return Names[State];
	}
   	else
   	{
      	return "<unknown>";
	}
}

/*
 *	GetCallStateModeName
 */
char* 
GetCallStateModeName(int Mode)
{

   	static char* Names[] =
    { 
    	"DisconnectedBusy", 
    	"DisconnectedNormal", 
    	"DisconnectedReject", 
    	"DisconnectedUnreachable",
      	"DisconnectedUnknown", 
      	"DisconnectedLocal", 
      	"ConnectedControl", 
      	"ConnectedCallSetup",
      	"ConnectedCall", 
      	"ConnectedConference", 
      	"OfferingCreate", 
      	"OfferingInvite",
      	"OfferingJoin", 
      	"OfferingCapabilityNegotiation", 
      	"OfferingCallIndependentSupplService" 
    };

   	if( Mode >= 0 && Mode < 15)
   	{
      	return Names[Mode];
	}
   	else
	{
      	return "<unknown>";
	}
}



/*
 *	GetRASEventName
 */
char* 
GetRASEventName( int Event )
{
   	static char* Names[] =
    { 
    	"GatekeeperConfirm", 
    	"GatekeeperReject", 
    	"RegistrationConfirm", 
    	"RegistrationReject",
      	"UnregistrationRequest", 
      	"UnregistrationConfirm", 
      	"UnregistrationReject", 
      	"NonStandardMessage" 
    };

   	if( Event >= 0 && Event < 8 )
	{
      	return Names[Event];
	}
   	else
	{
      	return "<unknown>";
	}
}



/*
 * GetChannelStateName
 */
char* 
GetChannelStateName( int State )
{
   	static char* Names[] =
    { 
    	"Dialtone", 
    	"RingBack", 
    	"Connected", 
    	"Disconnected", 
    	"Idle", 
    	"Offering" 
    };

   	if( State >= 0 && State < 6 )
   	{
      	return Names[State];
	}
   	else
	{
      	return "<unknown>";
	}
}



/*
 * GetChannelStateModeName
 */
char* 
GetChannelStateModeName(int Mode)
{

   	static char* Names[] =
    { 
    	"On", 
    	"Off", 
    	"DisconnectedLocal", 
    	"DisconnectedRemote", 
    	"DisconnectedMasterSlaveConflict",
      	"Duplex", 
      	"DisconnectedReasonUnknown", 
      	"DisconnectedReasonReopen",
      	"DisconnectedReasonReservationFailure" 
    };

   	if( Mode >= 0 && Mode < 9 )
   	{
      	return Names[Mode];
	}
   	else
	{
      	return "<unknown>";
	}
}





/*
 * GetControlStateName
 */
char* 
GetControlStateName(int State)
{
   	static char* Names[] =
    { 
    	"Connected", 
    	"Conference", 
    	"TransportConnected", 
    	"TransportDisconnected", 
    	"FastStart" 
    };

   	if( State >= 0 && State < 5 )
   	{
      	return Names[State];
	}
   	else
	{
      	return "<unknown>";
	}
}



/*
 *	CapabilityPrint
 */
void 
CapabilityPrint(cmCapStruct* pCaps )
{
	DBG_FUNC("CapabilityPrint",H323_LAYER);
	DBG_ENTER();

   	static char* pszDirection[] = 
   	{ 
   		"UNKNOWN", 
   		"RECEIVE", 
   		"TRANSMIT", 
   		"BIDIRECTIONAL" 
   	};
   	static char* pszDataType[] = 
   	{ 
   		"UNKNOWN", 
   		"AUDIO", 
   		"VIDEO", 
   		"DATA", 
   		"NON-STANDARD" 
   	};

    if  ( (pCaps->direction >= 0) && (pCaps->direction < 4) &&
          (pCaps->type >= 0) && (pCaps->type < 5) )
   	DBG_TRACE("Capability Name: %s, Direction: %s, Data Type: %s\n",
             pCaps->name, pszDirection[pCaps->direction], pszDataType[pCaps->type] );

	DBG_LEAVE();
   	return;
}




/*
 * jcULongPack
 */
void 
jcULongPack( ULONG uLong, UCHAR** ppszBuffer, int* pnSize )
{
   UCHAR* pszBuffer = *ppszBuffer;

   pszBuffer[0] = (UCHAR)( ( uLong >> 0  ) & 0xFF );
   pszBuffer[1] = (UCHAR)( ( uLong >> 8  ) & 0xFF );
   pszBuffer[2] = (UCHAR)( ( uLong >> 16 ) & 0xFF );
   pszBuffer[3] = (UCHAR)( ( uLong >> 24 ) & 0xFF );

   *ppszBuffer += 4;
   *pnSize += 4;

   return;
}

/*
 * jcULongUnpack
 */
ULONG 
jcULongUnpack( UCHAR** ppszBuffer )
{
   ULONG uLong;
   UCHAR* pszBuffer = *ppszBuffer;

   uLong = ( (ULONG)pszBuffer[0] << 0  ) |
           ( (ULONG)pszBuffer[1] << 8  ) |
           ( (ULONG)pszBuffer[2] << 16 ) |
           ( (ULONG)pszBuffer[3] << 24 );

   *ppszBuffer += 4;

   return uLong;
}

/*
 * jcUShortPack
 */
void 
jcUShortPack( USHORT uShort, UCHAR** ppszBuffer, int* pnSize )
{
   UCHAR* pszBuffer = *ppszBuffer;

   pszBuffer[0] = (UCHAR)( ( uShort >> 0  ) & 0xFF );
   pszBuffer[1] = (UCHAR)( ( uShort >> 8  ) & 0xFF );

   *ppszBuffer += 2;
   *pnSize += 2;

   return;
}

/*
 * jcUShortUnpack
 */
USHORT 
jcUShortUnpack( UCHAR** ppszBuffer )
{
   USHORT uShort;
   UCHAR* pszBuffer = *ppszBuffer;

   uShort = ( (USHORT)pszBuffer[0] << 0  ) |
            ( (USHORT)pszBuffer[1] << 8  );

   *ppszBuffer += 2;

   return uShort;
}

/*
 * jcBufferPack
 */
void 
jcBufferPack( PVOID pData, USHORT nDataSize, UCHAR** ppszBuffer, int* pnSize )
{
   if( nDataSize == 0 ) return;

   memcpy( *ppszBuffer, pData, nDataSize );

   *ppszBuffer += nDataSize;
   *pnSize += nDataSize;

   return;
}

/*
 * jcBufferUnpack
 */
PVOID 
jcBufferUnpack( USHORT nDataSize, UCHAR** ppszBuffer )
{
   PVOID pData;

   if( nDataSize == 0 ) return NULL;

   pData = *ppszBuffer;
   *ppszBuffer += nDataSize;

   return pData;
}



/*
 * jcIpToString
 */
char* 
jcIpToString( UINT32 addr, UINT16 port )
{
   static char buf[22];
   BYTE* ip=(BYTE*)&addr; 
   sprintf( buf, "%d.%d.%d.%d:%d", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3], port );
   return buf;
}




/*
 * jcStringPack
 */
void 
jcStringPack( char* pszString, UCHAR** ppszBuffer, int* pnSize )
{
   size_t nSize;

   nSize = strlen( pszString ) + 1;
   strcpy((char*)*ppszBuffer, pszString );

   *ppszBuffer += nSize;
   *pnSize += nSize;

   return;
}

/*
 * jcStringUnpack
 */
char* 
jcStringUnpack( UCHAR** ppszBuffer )
{
   char* pszString = (char*)*ppszBuffer;

   *ppszBuffer += strlen( pszString ) + 1;

   return pszString;
}


/*
 * TimerCreate
 */
PJCTIMER 
TimerCreate(UINT32 tmInterval, pfnCommonCallBack pfnCallBack, PVOID pData, BOOL bMultiple )
{
   PJCTIMER pTimer;


   	pTimer = (PJCTIMER)AllocTimerFromPool();
	ASSERT(pTimer!=NULL);
   	pTimer->pData       = pData;
   	pTimer->pfnCallBack = pfnCallBack;
   	pTimer->bMultiple   = bMultiple;
	pTimer->tmInterval  = tmInterval;
   	if( 0 > mtimerSet(pVoipTaskConfig->hsTimer, (LPMTIMEREVENTHANDLER)VxWorksTimerCallBack,
                      (PVOID)pTimer, 1000 * tmInterval ) )
   	{
      ReturnTimerToPool(pTimer );
      return NULL;
   	}

   	return pTimer;
}



/*
 * AllocTimerFromPool
 */
PJCTIMER 
AllocTimerFromPool()
{
	int Count;
	PJCTIMER pTimer=NULL;

	for(Count=0;Count<MAX_TIMERS_IN_ARRAY;Count++) {
		pTimer = &VoipTaskTimerArray[Count];
		if (pTimer->Used == FALSE) {
			pTimer->Used = TRUE;
			pTimer->Magic = H323TASK_MAGIC_VALUE;
			return(pTimer);
		}
	}
	return NULL;
}

/*
 * ReturnTimerToPool
 */
void
ReturnTimerToPool(PJCTIMER pTimer)
{
	pTimer->Used = FALSE;
}

/*
 * TimerArrayInit
 */
void
TimerArrayInit()
{
	int ArraySize;

	ArraySize = sizeof(JCTIMER) * MAX_TIMERS_IN_ARRAY;

	memset(VoipTaskTimerArray,0,ArraySize);

}

/*
 * VxWorksTimerCallBack
 */
void 
VxWorksTimerCallBack( PVOID pData )
{
   PJCTIMER pTimer = (PJCTIMER)pData;

   (*pTimer->pfnCallBack)( pTimer->pData );

   if( FALSE == pTimer->bMultiple )
      TimerDelete( pTimer );

   return;
}


/*
 * TimerDelete
 */
BOOL 
TimerDelete( PJCTIMER pTimer )
{
   if( NULL == pTimer )
      return FALSE;

   mtimerResetByValue( pVoipTaskConfig->hsTimer, (LPMTIMEREVENTHANDLER)VxWorksTimerCallBack, (PVOID)pTimer );

   ReturnTimerToPool(pTimer);

   return TRUE;
}



/*
 * DumpCCBAndRtp
 */
void
DumpCCBAndRtp()
{
	DBG_FUNC("DumpCCBAndRtp",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK	pCCB;
	int Count;

	for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	{
		pCCB = &CCBArray[Count];
		DBG_ERROR("CCB = %x: RTP = %x\n",pCCB,pCCB->RtpHandle);
	}
    DBG_LEAVE();
	return;
}


/*
 * jcBCDToString
 */
char* 
jcBCDToString( char* String, unsigned char* Digits, short nDigits )
{
   static char Translation[16] = { '0', '1', '2', '3', '4', '5', '6', '7',  '8', '9',
                                   '*', '#', 'a', 'b', 'c', 'd' };

   for( short i = 0; i < nDigits; i++ )
      String[i] = Translation[Digits[i]];

   String[nDigits] = '\0';

   return String;
}


/*
 * jcStringToBCD
 */
void
jcStringToBCD(char *string,unsigned char *digits, short *nDigits)
{
	static char Translation[16] = {
				'0', '1', '2', '3', '4', '5', '6', '7',  '8', '9',
                '*', '#', 'a', 'b', 'c', 'd' };
	int Count,index;
	for(index=0;index<strlen(string);index++) {
		for(Count=0;Count<16;Count++) {
			if (Translation[Count] == string[index]) {
				digits[index] = Count;
			}
		}
	}
	*nDigits = strlen(string);
	return;
}



/*
 * GetFreeMsgFromPool
 */
PAPI_MESSAGE 
GetFreeMsgFromPool()
{
	PAPI_MESSAGE pRetValue=NULL;
	int Count;
	for(Count=0;Count<MAX_VOIP_API_MSGS_IN_ARRAY;Count++) {
		if (VoipMessageArray[Count].Used == FALSE) {
			 pRetValue = &VoipMessageArray[Count];
			 pRetValue->Used = TRUE;
             pRetValue->VoipMsg.TxnId = VOIP_API_NULL_TXN_ID;
			 break;
		}
	}
	return (pRetValue);
}


/*
 * PutMsgIntoPool
 */
void
PutMsgIntoPool(PAPI_MESSAGE pMsg)
{
	memset(pMsg,0,sizeof(API_MESSAGE));
	pMsg->Used = FALSE;
}





/*
 * ExtractTelString
 */
char *
ExtractTelString(char *buffer)
{
    static char telString[100];
    char *tempPtr;

    memset(telString,0,100);
    strcpy(telString,"0000");

    if (buffer != NULL) {
        if ((tempPtr=strstr(buffer,"TEL:"))!=NULL) {
              tempPtr += 4;
              int remainderLen = strlen(tempPtr);
              int count;
              for(count=0;count<remainderLen;count++) {
                if (tempPtr[count] != ',') {
                    telString[count] = tempPtr[count];
                }
                else break;
              }
        }
    }
    return(telString);

}


/*
 * GetQ850CauseCodeName
 */
char *
GetQ850CauseCodeName(unsigned char cause_value)
{
	DBG_FUNC("GetQ850CauseCodeName",H323_LAYER);
	DBG_ENTER();
	static char cause_value_str[100];
	memset(cause_value_str,0,100);
	switch(cause_value) {
		case Q850_CAUSE_UNALLOCATED_NUM:{
			strncpy(cause_value_str,"Unallocated (unassigned) number",100);
		}
		break;
		case Q850_CAUSE_NO_ROUTE_TO_NET:{
			strncpy(cause_value_str,"No route to specified transit network",100);
		}
		break;
		case Q850_CAUSE_NO_ROUTE_TO_DEST:{
			strncpy(cause_value_str,"No route to destination",100);
		}
		break;
		case Q850_CAUSE_SEND_SPECIAL_TONE:{
			strncpy(cause_value_str,"Send special information tone",100);
		}
		break;
		case Q850_CAUSE_MISDIALED_TRUNK_PREFIX:{
			strncpy(cause_value_str,"Misdialled trunk prefix",100);
		}
		break;
		case Q850_CAUSE_CHANNEL_UNACCEPTABLE:{
			strncpy(cause_value_str,"Channel unacceptable",100);
		}
		break;
		case Q850_CAUSE_CALL_AWARDED_IN_EST_CHANNEL:{
			strncpy(cause_value_str,"Call awarded and being delivered in an established channel",100);
		}
		break;
		case Q850_CAUSE_PREEMPTION:{
			strncpy(cause_value_str,"Preemption",100);
		}
		break;
		case Q850_CAUSE_PREEMTION_CKT_RESVD:{
			strncpy(cause_value_str,"Preemption - circuit reserved for reuse",100);
		}
		break;
		case Q850_CAUSE_NORMAL_CLEARING:{
			strncpy(cause_value_str,"Normal call clearing",100);
		}
		break;
		case Q850_CAUSE_USER_BUSY:{
			strncpy(cause_value_str,"User busy",100);
		}
		break;
		case Q850_CAUSE_NO_USER_RESPONDING:{
			strncpy(cause_value_str,"No user responding",100);
		}
		break;
		case Q850_CAUSE_NO_ANSWER:{
			strncpy(cause_value_str,"No answer from user (user alerted)",100);
		}
		break;
		case Q850_CAUSE_SUBS_ABSENT:{
			strncpy(cause_value_str,"Subscriber absent",100);
		}
		break;
		case Q850_CAUSE_CALL_REJECTED:{
			strncpy(cause_value_str,"Call rejected",100);
		}
		break;
		case Q850_CAUSE_NUM_CHANGED:{
			strncpy(cause_value_str,"Number changed",100);
		}
		break;
		case Q850_CAUSE_NON_SELECTED_USER_CLEARING:{
			strncpy(cause_value_str,"Non-selected user clearing",100);
		}
		break;
		case Q850_CAUSE_DEST_OUT_OF_ORDER:{
			strncpy(cause_value_str,"Destination out of order",100);
		}
		break;
		case Q850_CAUSE_INVALID_NUM_FORMAT:{
			strncpy(cause_value_str,"Invalid number format (address incomplete)",100);
		}
		break;
		case Q850_CAUSE_FACILITY_REJECTED:{
			strncpy(cause_value_str,"Facility rejected",100);
		}
		break;
		case Q850_CAUSE_STATUS_ENQUIRY_RESPONSE:{
			strncpy(cause_value_str,"Response to STATUS ENQUIRY",100);
		}
		break;
		case Q850_CAUSE_NORMAL_UNSPECIFIED:{
			strncpy(cause_value_str,"Normal, unspecified",100);
		}
		case Q850_CAUSE_NO_CIRCUIT_AVAILABLE:{
			strncpy(cause_value_str,"No ciruit/channel available",100);
		}
		break;
		case Q850_CAUSE_NET_OUT_OF_ORDER:{
			strncpy(cause_value_str,"Network out of order",100);
		}
		break;
		case Q850_CAUSE_PFMC_OUT_OF_SERVICE:{
			strncpy(cause_value_str,"Permanent frame mode connection out of service",100);
		}
		break;
		case Q850_CAUSE_PFMC_OPERATIONAL:{
			strncpy(cause_value_str,"Permanent frame mode connection operational",100);
		}
		break;
		case Q850_CAUSE_TEMP_FAILURE:{
			strncpy(cause_value_str,"Temporary failure",100);
		}
		break;
		case Q850_CAUSE_SWITCH_CONGESTION:{
			strncpy(cause_value_str,"Switching equipment congestion",100);
		}
		break;
		case Q850_CAUSE_ACCESS_INFO_DISCARDED:{
			strncpy(cause_value_str,"Access information discarded",100);
		}
		break;
		case Q850_CAUSE_REQUESTED_CKT_UNAVAILABLE:{
			strncpy(cause_value_str,"Requested circuit/channel not available",100);
		}
		break;
		case Q850_CAUSE_PRECEDENCE_CALL_BLOCKED:{
			strncpy(cause_value_str,"Precedence call blocked",100);
		}
		break;
		case Q850_CAUSE_RESOURCE_UNAVAIL_UNSPECIFIED:{
			strncpy(cause_value_str,"Resource unavailable, unspecified",100);
		}
		break;
		case Q850_CAUSE_QOS_UNAVAILABLE:{
			strncpy(cause_value_str,"Quality of service unavailable",100);
		}
		break;
		case Q850_CAUSE_REQ_FACILITY_NOT_SUBSCRIBED:{
			strncpy(cause_value_str,"Requested facility not subscribed",100);
		}
		break;
		case Q850_CAUSE_CUG_OUTCALL_BARRED:{
			strncpy(cause_value_str,"Outgoing calls barred within CUG",100);
		}
		break;
		case Q850_CAUSE_CUG_INCALL_BARRED:{
			strncpy(cause_value_str,"Incoming calls barred within CUG",100);
		}
		break;
		case Q850_CAUSE_BEARER_CAP_NOT_AUTH:{
			strncpy(cause_value_str,"Bearer capability not authorized",100);
		}
		break;
		case Q850_CAUSE_BEARER_CAP_NOT_AVAIL:{
			strncpy(cause_value_str,"Bearer capability not presently available",100);
		}
		break;
		case Q850_CAUSE_INCONSISTENCY_ACCESS_INFO:{
			strncpy(cause_value_str,"Inconsistency in designated outgoing access information and subscriber class",100);
		}
		break;
		case Q850_CAUSE_SERVICE_NOT_AVAILABLE:{
			strncpy(cause_value_str,"Service or option not available, unspecified",100);
		}
		break;
		case Q850_CAUSE_BEARER_CAP_NOT_IMPLEMENTED:{
			strncpy(cause_value_str,"Bearer capability not implemented",100);
		}
		break;
		case Q850_CAUSE_CHANNEL_TYPE_NOT_IMPLEMENTED:{
			strncpy(cause_value_str,"Channel type not implemented",100);
		}
		break;
		case Q850_CAUSE_REQ_FACILITY_NOT_IMPLEMENTED:{
			strncpy(cause_value_str,"Requested facility not implemented",100);
		}
		break;
		case Q850_CAUSE_RESTRICTED_CAP_AVAILABLE:{
			strncpy(cause_value_str,"Only restricted digital information bearer capability is available",100);
		}
		break;
		case Q850_CAUSE_SERVICE_NOT_IMPL_UNSPECIFIED:{
			strncpy(cause_value_str,"Service or option not implemented, unspecified",100);
		}
		break;
		case Q850_CAUSE_INVALID_CRV:{
			strncpy(cause_value_str,"Invalid call reference value",100);
		}
		break;
		case Q850_CAUSE_IDENTIFIED_CHANNEL_NOT_EXIST:{
			strncpy(cause_value_str,"Identified channel does not exist",100);
		}
		break;
		case Q850_CAUSE_SUSPENDED_CALL_EXISTS:{
			strncpy(cause_value_str,"A suspended call exists, but this call identity does not",100);
		}
		break;
		case Q850_CAUSE_CALL_ID_IN_USE:{
			strncpy(cause_value_str,"Call identity in use",100);
		}
		break;
		case Q850_CAUSE_NO_CALL_SUSPENDED:{
			strncpy(cause_value_str,"No call suspended",100);
		}
		break;
		case Q850_CAUSE_REQ_CALL_ID_CLEARED:{
			strncpy(cause_value_str,"Call having requested call identity has been cleared",100);
		}
		break;
		case Q850_CAUSE_USER_NOT_CUG_MEMBER:{
			strncpy(cause_value_str,"User not member of CUG",100);
		}
		break;
		case Q850_CAUSE_INCOMPATIBLE_DEST:{
			strncpy(cause_value_str,"Incompatible destination",100);
		}
		break;
		case Q850_CAUSE_NON_EXISTENT_CUG:{
			strncpy(cause_value_str,"Non-existent CUG",100);
		}
		break;
		case Q850_CAUSE_INVALID_TRANSIT_NETWORK:{
			strncpy(cause_value_str,"Invalid transit network selection",100);
		}
		break;
		case Q850_CAUSE_INVALID_MSG:{
			strncpy(cause_value_str,"Invalid message, unspecified",100);
		}
		break;
		case Q850_CAUSE_MANDATORY_IE_MISSING:{
			strncpy(cause_value_str,"Mandatory Information Element is missing",100);
		}
		break;
		case Q850_CAUSE_MSG_TYPE_NON_EXISTENT:{
			strncpy(cause_value_str,"Message type non-existent or not implemented",100);
		}
		break;
		case Q850_CAUSE_MSG_NOT_COMPAT_STATE_NOT_IMPL:{
			strncpy(cause_value_str,"Message not compatible with call state or message type non-existent or not implemented",100);
		}
		break;
		case Q850_CAUSE_IE_PARAM_NON_EXISTENT:{
			strncpy(cause_value_str,"Information Element/parameter non-existent or not implemented",100);
		}
		break;
		case Q850_CAUSE_INVALID_IE_CONTENTS:{
			strncpy(cause_value_str,"Invalid Information Element contents",100);
		}
		break;
		case Q850_CAUSE_MSG_NOT_COMPAT_STATE:{
			strncpy(cause_value_str,"Message not compatible with call state",100);
		}
		break;
		case Q850_CAUSE_TIMER_EXPIRY_RECOVER:{
			strncpy(cause_value_str,"Recover on timer expiry",100);
		}
		break;
		case Q850_CAUSE_PARAM_NOT_IMPL_PASSED_ON:{
			strncpy(cause_value_str,"Parameter non-existent or not implemented, passed on",100);
		}
		break;
		case Q850_CAUSE_MSG_PARAM_UNRECOGNIZED_DISCARDED:{
			strncpy(cause_value_str,"Message with unrecognized parameter, discarded",100);
		}
		break;
		case Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED:{
			strncpy(cause_value_str,"Protocol error, unspecified",100);
		}
		break;
		case Q850_CAUSE_INTERWORKING_UNSPECIFIED:{
			strncpy(cause_value_str,"Interworking, unspecified",100);
		}
		break;
		default: {
			strncpy(cause_value_str,"UNKNOWN cause value",100);
		}
		break;
	}
	DBG_LEAVE();
	return(cause_value_str);
}
