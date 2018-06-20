
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: CCB.h															*
 *																						*
 *	Description			: Hdr file containing data structure definitions for 			*
 *						  the call control block										*
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
#ifndef CCB_HDR_INC
#define CCB_HDR_INC

/*
 * VxWorks & ANSI C Headers
 */
#include <taskLib.h>
#include <ioLib.h>

/* 
 * RadVision Headers 
 */
#include <cm.h>
#include <mti.h>
#include <seli.h>
#include <pi.h>
#include <stkutils.h>

#include <sse.h>

/*
 * Application Headers
 */
#include "voip\voipapi.h"
#include "rtptask.h"
#include "logging\vcmodules.h"
#include "logging\vclogging.h"




/*
 * Common Definitions 
 */
#define H323TASK_MAGIC_VALUE				0x1234
#define STATUS_SUCCESS						1
#define STATUS_FAILURE						!STATUS_SUCCESS
#define MAX_VIPERCELL_ID_STR_LENGTH			128
#define RAD_MAX_TIMERS 						100
#define MAX_DESTINATION_ADDRESS_SIZE		100
#define MAX_VIPERCELL_NAME_STRING_LENGTH	128
#define MAX_STACK_CONFIG_BUFFER_SIZE        8092
#define MAX_IP_ADDRESS_STRING_LENGTH        32
#define VOIP_MAX_CALLING_PARTY_ID_LENGTH	255
#define VOIP_MAX_CALLED_PARTY_ID_LENGTH		255
#define VOIP_MAX_DISPLAY_INFO_LENGTH		255
#define VOIP_MAX_REMOTE_PARTY_ID_LENGTH     255
#define VOIP_MAX_ISDN_NUMBER_LENGTH         255
#define RTP_GSM_PAYLOAD_TYPE                3
#define RTP_GSM_EFR_PAYLOAD_TYPE			20
#define RTP_DYNAMIC_PAYLOAD_TYPE            96
#define RTP_STARVOX_PAYLOAD_TYPE            100



#ifndef PVOID
typedef void * PVOID;
#endif 
typedef int	STATUS;
typedef int	H323_TASK_LOCK;


/* 

/* CALL EVENTS */
typedef enum {
	H323_EVENT_RECV_CALL_OFFERING,
	H323_EVENT_RECV_CC_CALL_RELEASE,
	H323_EVENT_RECV_CC_ANSWER_CALL,
	H323_EVENT_RECV_CC_CALL_SETUP,
	H323_EVENT_RECV_CALL_PROCEEDING,
	H323_EVENT_RECV_CALL_RINGBACK,
	H323_EVENT_RECV_CALL_CONNECTED,
	H323_EVENT_RECV_CALL_DISCONNECTED,
	H323_EVENT_RECV_CALL_DEST_REJECTED,
	H323_EVENT_RECV_CALL_IDLE,
	H323_EVENT_RECV_REMOTE_CAPABILITIES,
	H323_EVENT_RECV_H245_TRANSPORT_CONNECTED,
	H323_EVENT_RECV_H245_CONNECTED,
	H323_EVENT_RECV_CAPABILITIES_ACK,
	H323_EVENT_RECV_CAPABILITIES_NAK,
	H323_EVENT_RECV_CHANNEL_OFFERING,
	H323_EVENT_RECV_IN_CHANNEL_CONNECTED,
	H323_EVENT_RECV_OUT_CHANNEL_CONNECTED,
	H323_EVENT_RECV_IN_CHANNEL_DISCONNECTED,
	H323_EVENT_RECV_OUT_CHANNEL_DISCONNECTED,
	H323_MAX_CALL_EVENTS
} H323_CALL_EVENT;

/* CALL STATES */
typedef enum {
	H323_CALL_STATE_SAME_STATE=-1,
	H323_CALL_STATE_IDLE,
	H323_CALL_STATE_WAIT_FOR_CC_ANSWER,
	H323_CALL_STATE_WAIT_FOR_PROCEEDING,
	H323_CALL_STATE_WAIT_FOR_CONNECTION,
	H323_CALL_STATE_CALL_CONNECTED,
	H323_CALL_STATE_WAIT_FOR_CAP_EXCHANGE_ACK,
	H323_CALL_STATE_WAIT_FOR_CHANNELS_CONNECTION,
	H323_CALL_STATE_CHANNELS_CONNECTED,
	H323_MAX_CALL_STATES

} H323_CALL_STATE;

#define MAX_STATE_TRANSITION_HISTORY	10

/*
 * Q.850 Disconnect Cause Codes
 */
typedef enum {

	Q850_CAUSE_UNALLOCATED_NUM								= 1,
	Q850_CAUSE_NO_ROUTE_TO_NET								= 2,
	Q850_CAUSE_NO_ROUTE_TO_DEST								= 3,
	Q850_CAUSE_SEND_SPECIAL_TONE							= 4,
	Q850_CAUSE_MISDIALED_TRUNK_PREFIX						= 5,
	Q850_CAUSE_CHANNEL_UNACCEPTABLE							= 6,
	Q850_CAUSE_CALL_AWARDED_IN_EST_CHANNEL					= 7,
	Q850_CAUSE_PREEMPTION									= 8,
	Q850_CAUSE_PREEMTION_CKT_RESVD							= 9,
	Q850_CAUSE_NORMAL_CLEARING								= 16,
	Q850_CAUSE_USER_BUSY									= 17,
	Q850_CAUSE_NO_USER_RESPONDING							= 18,
	Q850_CAUSE_NO_ANSWER									= 19,
	Q850_CAUSE_SUBS_ABSENT									= 20,
	Q850_CAUSE_CALL_REJECTED								= 21,
	Q850_CAUSE_NUM_CHANGED									= 22,
	Q850_CAUSE_NON_SELECTED_USER_CLEARING					= 26,
	Q850_CAUSE_DEST_OUT_OF_ORDER							= 27,
	Q850_CAUSE_INVALID_NUM_FORMAT							= 28,
	Q850_CAUSE_FACILITY_REJECTED							= 29,
	Q850_CAUSE_STATUS_ENQUIRY_RESPONSE						= 30,
	Q850_CAUSE_NORMAL_UNSPECIFIED							= 31,
	Q850_CAUSE_NO_CIRCUIT_AVAILABLE							= 34,
	Q850_CAUSE_NET_OUT_OF_ORDER								= 38,
	Q850_CAUSE_PFMC_OUT_OF_SERVICE							= 39,
	Q850_CAUSE_PFMC_OPERATIONAL								= 40,
	Q850_CAUSE_TEMP_FAILURE									= 41,
	Q850_CAUSE_SWITCH_CONGESTION							= 42,
	Q850_CAUSE_ACCESS_INFO_DISCARDED						= 43,
	Q850_CAUSE_REQUESTED_CKT_UNAVAILABLE					= 44,
	Q850_CAUSE_PRECEDENCE_CALL_BLOCKED						= 46,
	Q850_CAUSE_RESOURCE_UNAVAIL_UNSPECIFIED					= 47,
	Q850_CAUSE_QOS_UNAVAILABLE								= 49,
	Q850_CAUSE_REQ_FACILITY_NOT_SUBSCRIBED					= 50,
	Q850_CAUSE_CUG_OUTCALL_BARRED							= 53,
	Q850_CAUSE_CUG_INCALL_BARRED							= 55,
	Q850_CAUSE_BEARER_CAP_NOT_AUTH							= 57,
	Q850_CAUSE_BEARER_CAP_NOT_AVAIL							= 58,
	Q850_CAUSE_INCONSISTENCY_ACCESS_INFO					= 62,
	Q850_CAUSE_SERVICE_NOT_AVAILABLE						= 63,
	Q850_CAUSE_BEARER_CAP_NOT_IMPLEMENTED					= 65,
	Q850_CAUSE_CHANNEL_TYPE_NOT_IMPLEMENTED					= 66,
	Q850_CAUSE_REQ_FACILITY_NOT_IMPLEMENTED					= 69,
	Q850_CAUSE_RESTRICTED_CAP_AVAILABLE						= 70,
	Q850_CAUSE_SERVICE_NOT_IMPL_UNSPECIFIED					= 79,
	Q850_CAUSE_INVALID_CRV									= 81,
	Q850_CAUSE_IDENTIFIED_CHANNEL_NOT_EXIST					= 82,
	Q850_CAUSE_SUSPENDED_CALL_EXISTS						= 83,
	Q850_CAUSE_CALL_ID_IN_USE								= 84,
	Q850_CAUSE_NO_CALL_SUSPENDED							= 85,
	Q850_CAUSE_REQ_CALL_ID_CLEARED							= 86,
	Q850_CAUSE_USER_NOT_CUG_MEMBER							= 87,
	Q850_CAUSE_INCOMPATIBLE_DEST							= 88,
	Q850_CAUSE_NON_EXISTENT_CUG								= 90,
	Q850_CAUSE_INVALID_TRANSIT_NETWORK						= 91,
	Q850_CAUSE_INVALID_MSG									= 95,
	Q850_CAUSE_MANDATORY_IE_MISSING							= 96,
	Q850_CAUSE_MSG_TYPE_NON_EXISTENT						= 97,
	Q850_CAUSE_MSG_NOT_COMPAT_STATE_NOT_IMPL				= 98,
	Q850_CAUSE_IE_PARAM_NON_EXISTENT						= 99,
	Q850_CAUSE_INVALID_IE_CONTENTS							= 100,
	Q850_CAUSE_MSG_NOT_COMPAT_STATE							= 101,
	Q850_CAUSE_TIMER_EXPIRY_RECOVER							= 102,
	Q850_CAUSE_PARAM_NOT_IMPL_PASSED_ON						= 103,
	Q850_CAUSE_MSG_PARAM_UNRECOGNIZED_DISCARDED				= 110,
	Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED					= 111,
	Q850_CAUSE_INTERWORKING_UNSPECIFIED						= 127,
	Q850_MAX_CAUSES											= 128

} Q850_DISCONNECT_CAUSE;



/*
 * Q.931 Progress Description Types
 */
typedef enum {

	Q931_IE_PROGRESS_DESC_NOT_END_TO_END_ISDN	= 0x01,
	Q931_IE_PROGRESS_DESC_DEST_NOT_ISDN			= 0x02,
	Q931_IE_PROGRESS_DESC_ORIG_NOT_ISDN			= 0x03,
	Q931_IE_PROGRESS_DESC_RETURN_TO_ISDN		= 0x04,
	Q931_IE_PROGRESS_DESC_INTERWK_SVC_CHANGE	= 0x05,
	Q931_IE_PROGRESS_DESC_INBAND_INFO			= 0x08

} Q931_IE_PROGRESS_DESC;


typedef enum {
    H323_CALL_ORIGINATED,
    H323_CALL_TERMINATED
}H323_CALL_DIRECTION;


typedef enum {
	REMOTE_TERMINAL_GSM,
	REMOTE_TERMINAL_NETMEETING,
	REMOTE_TERMINAL_CISCO,
	MAX_REMOTE_TERMINAL_TYPES
}REMOTE_TERMINAL_TYPE;


typedef void ( *pfnCommonCallBack )( PVOID );

#define MAX_TIMERS_IN_ARRAY			10


typedef struct _JCTIMER
{
   pfnCommonCallBack 	pfnCallBack;
   PVOID 				pData;
   BOOL 				bMultiple;
   time_t 				tmInterval;
   BOOL					Used;
   int					Magic;
}JCTIMER, *PJCTIMER;




/*
 * Number type
 */
typedef enum {

	Q931_IE_CALL_PARTY_NO_TYPE_UNKNOWN          = 0x00,
	Q931_IE_CALL_PARTY_NO_TYPE_INTERNATIONAL    = 0x01,
	Q931_IE_CALL_PARTY_NO_TYPE_NATIONAL         = 0x02,
	Q931_IE_CALL_PARTY_NO_TYPE_NETWK_SPECIFIC   = 0x03,
	Q931_IE_CALL_PARTY_NO_TYPE_SUBS_NO          = 0x04,
	Q931_IE_CALL_PARTY_NO_TYPE_ABBR_NO          = 0x06,
	Q931_IE_CALL_PARTY_NO_TYPE_RESERVED         = 0x07

}Q931_IE_CALL_PARTY_NO_TYPE;


/*
 * Numbering plan
 */
typedef enum {

	Q931_IE_CALL_PARTY_NO_PLAN_UNKNOWN          = 0x00,
	Q931_IE_CALL_PARTY_NO_PLAN_ISDN             = 0x01,
	Q931_IE_CALL_PARTY_NO_PLAN_DATA             = 0x03,
	Q931_IE_CALL_PARTY_NO_PLAN_TELEX            = 0x04,
	Q931_IE_CALL_PARTY_NO_PLAN_NATIONAL         = 0x08,
	Q931_IE_CALL_PARTY_NO_PLAN_PRIVATE          = 0x09,
	Q931_IE_CALL_PARTY_NO_PLAN_RESERVED         = 0x0f

} Q931_IE_CALL_PARTY_NO_PLAN;



/*
 * Presentation Indicator
 */
typedef enum {

	Q931_IE_CALLING_PARTY_NO_PI_ALLOWED          = 0x00,
	Q931_IE_CALLING_PARTY_NO_PI_RESTRICTED       = 0x01,
	Q931_IE_CALLING_PARTY_NO_PI_NOT_AVAIL        = 0x02,
	Q931_IE_CALLING_PARTY_NO_PI_RESERVED         = 0x03

} Q931_IE_CALLING_PARTY_NO_PI;


/*
 * Screening Indicator
 */
typedef enum {

	Q931_IE_CALLING_PARTY_NO_SI_UP_NOT_SCREENED     = 0x00,
	Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_PASSED  = 0x01,
	Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_FAILED  = 0x02,
	Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED    = 0x03

} Q931_IE_CALLING_PARTY_NO_SI;


typedef struct {

    UINT8           octet2_bit:1;
    UINT8           presentation_indicator:2;
    UINT8           reserved:3;
    UINT8           screening_indicator:2;

} Q931_MSG_IE_CALLING_PARTY_RES_INDICATORS,*PQ931_MSG_IE_CALLING_PARTY_RES_INDICATORS;






typedef struct {
  H323_CALL_STATE	CallStates[MAX_STATE_TRANSITION_HISTORY];
  H323_CALL_STATE	NextCallStates[MAX_STATE_TRANSITION_HISTORY];
  H323_CALL_EVENT	CallEvents[MAX_STATE_TRANSITION_HISTORY];
  int				PairIndex;
} STATE_TRANSITION_HISTORY;


#define VOIP_NONSTD_TERMCAPS_CODEC_SUPPORTED		100
#define VOIP_NONSTD_TERMCAPS_CODEC_NOT_SUPPORTED	0


typedef struct {
	int				Magic;
	int				gsmFullRate;
	int				gsmEnhancedFullRate;
} VOIP_NONSTD_TERMCAPS;


typedef enum {
    VOIP_CALL_CODEC_GSM_FR,
    VOIP_CALL_CODEC_GSM_EFR
}  VOIP_CALL_SELECTED_CODEC;


typedef struct {

    unsigned long           MobileDisconnects[Q850_MAX_CAUSES];
    unsigned long           VoipDisconnects[Q850_MAX_CAUSES];
} VOIP_CALL_DISCONNECT_STATS;

typedef struct {

    unsigned long               IncomingCallAttempts;
    unsigned long               IncomingCallsConnected;
    unsigned long               OutgoingCallAttempts;
    unsigned long               OutgoingCallsConnected;

} VOIP_CALL_COMPLETION_STATS;

typedef struct {

    VOIP_CALL_COMPLETION_STATS      CallCompletionStats;

    VOIP_CALL_DISCONNECT_STATS      CallDisconnectStats;

} VOIP_CALL_STATISTICS;








typedef struct _CALL_CONTROL_BLOCK{

	HCALL						LanCallHandle;
	HSSECALL					SuppCallHandle;
	HPROTCONN					TpktHandle;
	BOOL						ControlConnected;

	unsigned short				H323CRV;


	H323_CALL_STATE				H323CallState;
	H323_CALL_STATE				PreviousCallState;
    H323_CALL_EVENT             CurrentEvent;

    LUDB_ID                     LudbId;

    H323_CALL_DIRECTION         CallDirection;
	BOOL						Q931Connected;

	BOOL						IsSuppCall;
	BOOL						CallTransferred;
	short						OldIndex;
	BOOL						InBandInfo;

    BOOL                        LocalRelease;
    short                       Index;

	TXN_ID						LastTxnId;

	HJCRTPSESSION				RtpHandle;
	REMOTE_TERMINAL_TYPE		RemoteType;
    BOOL                        FastStart;
	HCHAN 						OutChannelHandle;
	HCHAN						InChannelHandle;
	HCHAN						NonAudioChannelHandle;
	BOOL						InChannelConnected;
	BOOL						OutChannelConnected;
    BOOL                        CanDropCall;

	int							CallingNumberType;
	int							CallingNumberingPlan;
	int							CalledNumberType;
	int							CalledNumberingPlan;
	int							CallingPartyPI;
	int							CallingPartySI;

	BOOL						CodecInfoSent;
    BOOL                        GSMFullRateCapability;
    BOOL                        GSMEnhancedFullRateCapability;
    BOOL                        RemoteGSMFullRateCapability;
    BOOL                        RemoteGSMEnhancedFullRateCapability;
    VOIP_CALL_SELECTED_CODEC    CodecUsed;

	int							remoteGsmFrTxIndex;
	int							remoteGsmFrRxIndex;
	int							remoteGsmEfrTxIndex;
	int							remoteGsmEfrRxIndex;

	BOOL						Used;
	int							Magic;	
    char                        MobileNumber[VOIP_MAX_ISDN_NUMBER_LENGTH];
    char                        RemotePartyId[VOIP_MAX_REMOTE_PARTY_ID_LENGTH];
    char                        TransferringPartyId[VOIP_MAX_REMOTE_PARTY_ID_LENGTH];
	char						DisplayInfo[VOIP_MAX_DISPLAY_INFO_LENGTH];
	STATE_TRANSITION_HISTORY	StateTransitionHistory;

} CALL_CONTROL_BLOCK, *PCALL_CONTROL_BLOCK;



typedef struct {
	HAPP						StackHandle;
	HSSEAPP 					SuppStackHandle;
	HSSAPP 						SuppAppHandle;


	PRADPIPE 					pH323TaskInputPipe;
	PRADPIPE					pRtpInputPipe;
    char                        szGatekeeperAddress[MAX_IP_ADDRESS_STRING_LENGTH];

	int							Magic;
	int							RtpTaskHandle;

   	HSTIMER 					hsTimer;

	BOOL						DefaultCodecGsmFullRate;


    /*
     * Statistics fields
     */

    VOIP_CALL_STATISTICS        CallStats;

    unsigned long               GKPort;
    unsigned long               GKIPAddress;
    unsigned long               Q931ResponseTimeOut;
    unsigned long               Q931ConnectTimeOut;
    unsigned long               Q931CallSignalingPort;
    unsigned long               t35CountryCode;
    unsigned char               gkIpAddrOctets[4];

} H323TASK_CONFIG,*PH323TASK_CONFIG;






typedef STATUS (*SM_FUNCTION)(PCALL_CONTROL_BLOCK pCCB,PVOID Context);


typedef struct {
  SM_FUNCTION	smFunc;	
  H323_CALL_STATE NextState;
} H323_CALL_STATE_INFO, *PH323_CALL_STATE_INFO;

typedef struct {
  VOIP_API_MESSAGE		VoipMsg;
  BOOL					Used;	
}API_MESSAGE,*PAPI_MESSAGE;


#define MAX_CALLING_PARTY_ADDRESS_SIZE	64
#define MAX_CALLED_PARTY_ADDRESS_SIZE	64

typedef struct 
{
	char pszCallingPartyId[MAX_CALLING_PARTY_ADDRESS_SIZE];
	
	char pszCalledPartyId[MAX_CALLED_PARTY_ADDRESS_SIZE];

} CALL_OFFERING_DETAILS, *PCALL_OFFERING_DETAILS;

typedef struct 
{
	BOOL	GSMCodecCapability;

    BOOL    GSMFullRateCapability;

    BOOL    GSMEnhancedFullRateCapability;

} REMOTE_CAPABILITIES_DETAILS, *PREMOTE_CAPABILITIES_DETAILS;


#define WAIT_FOREVER_FOR_H323_EVENTS();		while(1) \
{ \
seliSelect(); \
}

#define WAIT_FOREVER_FOR_RTP_EVENTS();  WAIT_FOREVER_FOR_H323_EVENTS();




void InitializeCallControlBlocks(void);
void InitializeCCB(PCALL_CONTROL_BLOCK pCCB,short Index);





PCALL_CONTROL_BLOCK GetCCBFromPool(void);
void ReturnCCBToPool(PCALL_CONTROL_BLOCK pCCB);
PCALL_CONTROL_BLOCK GetCCBByTpktHandle(HPROTCONN TpktHandle);

void VoipDropAllCalls(void);

void DumpStateTransitionHistory(PCALL_CONTROL_BLOCK pCCB);

#endif /* #ifndef CCB_HDR_INC */