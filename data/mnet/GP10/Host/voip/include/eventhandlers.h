
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: EventHandlers.H												*
 *																						*
 *	Description			: Hdr file containing data structure and function prototypes	*
 *						  definitions for the file EventHandlers.cpp					*
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

#ifndef EVENTHANDLERS_HDR_INC
#define EVENTHANDLERS_HDR_INC

#include <stdlib.h>
#include <cm.h>




/*
 * Function Prototypes
 */
int CALLCONV NewCallHandler(
   IN  HAPP       hApp,
   IN  HCALL      hsCall,
   OUT LPHAPPCALL lphaCall);
int CALLCONV RASEventHandler(
   IN  HAPP       hApp,
   IN  cmRegState regState,
   IN  cmRegEvent regEvent,
   IN  int        regEventHandle);
int CALLCONV EventCallStateChanged(
   IN HAPPCALL haCall,
   IN HCALL    hsCall,
   IN UINT32   State,
   IN UINT32   StateMode);
int CALLCONV EventCallInfo(
   IN HAPPCALL haCall,
   IN HCALL    hsCall,
   IN char*    Display,
   IN char*    User,
   IN int      UserSize);
int CALLCONV EventCallCapabilities(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmCapStruct*        capabilities[]);
int CALLCONV EventCallCapabilitiesExt(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmCapStruct***      capabilities[]);
int CALLCONV EventCallNewChannel(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      HCHAN               hsChan,
      OUT     LPHAPPCHAN          lphaChan);
int CALLCONV EventCallCapabilitiesResponse(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              status);
int CALLCONV EventCallMasterSlaveStatus(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              status);
int CALLCONV EventCallRoundTripDelay(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      INT32               delay);
int CALLCONV EventCallUserInput(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      INT32               userInputId);
int CALLCONV EventCallRequestMode(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmReqModeStatus     status,
      IN      INT32               nodeId);
int CALLCONV EventCallMiscStatus(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmMiscStatus        status);
int CALLCONV EventCallControlStateChanged(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              State,
      IN      UINT32              StateMode);
int CALLCONV EventCallMasterSlave(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              terminalType,
      IN      UINT32              statusDeterminationNumber);
int CALLCONV EventCallFacility(
		IN      HAPPCALL    haCall,
		IN      HCALL       hsCall,
		IN      int         handle,
		OUT IN	BOOL		*proceed);
int CALLCONV EventCallFastStartSetup(
		IN HAPPCALL haCall, 
		IN HCALL hsCall,
		IN cmFastStartMessage *fsMessage);
int CALLCONV suppEventForwardActivated(
	IN HSSEAPPCALL	hSSEaCall,
	IN HSSECALL		hSSECall
	);
int CALLCONV suppEventForwardDeactivated(
	IN HSSEAPPCALL	hSSEaCall,
	IN HSSECALL		hSSECall
	);
int CALLCONV suppEventCallReroute(
	IN HSSEAPPCALL	hSSEaCallPri,
	IN HSSECALL		hSSECallPri,
	OUT HSSECALL 	*hSSECallSec);
int CALLCONV suppEventCallTransfer(
		IN	HSSEAPPCALL	    hSSEaCallPri,
		IN      HSSECALL	    hSSECallPri,
		OUT	HSSECALL*	    hSSECallSec);
int CALLCONV 
EventCallH450SupplementaryService(
			IN	HAPPCALL	  haCall, 
			IN	HCALL		  hsCall,
			IN	cmCallQ931MsgType msgType,
			IN	int		  nodeId,
			IN	int		  size);
int CALLCONV EventChannelSetAddress(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              ip,
      IN      UINT16              port);
int CALLCONV EventChannelStateChanged(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              State,
      IN      UINT32              StateMode);
int CALLCONV EventChannelNewRate(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              rate);
int CALLCONV EventChannelMaxSkew(
      IN      HAPPCHAN            haChan1,
      IN      HCHAN               hsChan1,
      IN      HAPPCHAN            haChan2,
      IN      HCHAN               hsChan2,
      IN      UINT32              skew);
int CALLCONV EventChannelParameters(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      char*               channelName,
      IN      HAPPCHAN            haChanSameSession,
      IN      HCHAN               hsChanSameSession,
      IN      HAPPCHAN            haChanAssociated,
      IN      HCHAN               hsChanAssociated,
      IN      UINT32              rate);
int CALLCONV EventChannelVideoFastUpdatePicture(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan);
int CALLCONV EventChannelVideoFastUpdateGOB(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      int                 firstGOB,
      IN      int                 numberOfGOBs);
int CALLCONV EventChannelVideoFastUpdateMB(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      int                 firstGOB,
      IN      int                 firstMB,
      IN      int                 numberOfMBs);
int CALLCONV EventChannelHandle(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      int                 dataTypeHandle,
      IN      cmCapDataType       dataType);
int CALLCONV EventChannelRequestCloseStatus(
      IN      HAPPCHAN              haChan,
      IN      HCHAN                 hsChan,
      IN      cmRequestCloseStatus  status);
int CALLCONV EventChannelTSTO(
      IN      HAPPCHAN              haChan,
      IN      HCHAN                 hsChan,
      IN      INT8                  isCommand,
      IN      INT8                  tradeoffValue);
int CALLCONV EventChannelMediaLoopStatus(
      IN      HAPPCHAN              haChan,
      IN      HCHAN                 hsChan,
      IN      cmMediaLoopStatus     status);
int CALLCONV EventChannelReplace(
      IN    HAPPCHAN         haChan,
      IN    HCHAN            hsChan,
      IN    HAPPCHAN         haReplacedChannel,
      IN    HCHAN            hsReplacedChannel);
int CALLCONV EventChannelFlowControlToZero(
      IN    HAPPCHAN         haChan,
      IN    HCHAN            hsChan);
int CALLCONV EventHookSend(
      IN    HPROTCONN        hConn,
      IN    int              nodeId,
      IN    BOOL             error);
int CALLCONV EventHookRecv(
      IN    HPROTCONN        hConn,
      IN    int              nodeId,
      IN    BOOL             error);

int CALLCONV EventChannelSetRTCPAddress(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              ip,
      IN      UINT16              port );
int CALLCONV EventChannelRTPDynamicPayloadType(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      INT8                dynamicPayloadType );
int CALLCONV EventChannelGetRTCPAddress(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32*             ip,
      IN      UINT16*             port );

BOOL CALLCONV 
EventHookSendTo(
        IN      HPROTCONN           hConn,
		IN      int                 nodeId,
        IN      int                 nodeIdTo,
		IN      BOOL                error
		);

BOOL CALLCONV EventHookRecvFrom(
        IN      HPROTCONN           hConn,
		IN      int                 nodeId,
        IN      int                 nodeIdFrom,
        IN      BOOL                multicast,
		IN      BOOL                error
	        );

#endif /* #ifndef EVENTHANDLERS_HDR_INC */
