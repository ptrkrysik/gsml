
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: VoipApi.h														*
 *																						*
 *	Description			: The API interface for the Voice Over IP Task					*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |05/11/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */

#ifndef VOIPAPI_HDR_INC
#define VOIPAPI_HDR_INC

#include "ril3/ie_mobile_id.h"
#include "ril3/ril3irt.h"
#include "ril3/ie_called_bcd.h"
#include "ril3/ie_calling_bcd.h"
#include "ril3/ie_cc_common.h"
#include "ril3/ie_bearer_capability.h"
#include "ril3/ie_progress_indicator.h"
#include "cc/ccconfig.h"
#include "jetrtp.h"



#ifndef PVOID
#define PVOID void *
#endif

#define MAX_MOBILE_ID_LENGTH    		(CNI_RIL3_MAX_ID_DIGITS + 1)
#define MAX_MOBILE_NUM_LENGTH   		(CNI_RIL3_MAX_BCD_DIGITS + 1)
#define MAX_REGISTERED_MOBILES  		100 /* temporary, has to be 75 */
#define MAX_VOIP_API_MSGS_IN_ARRAY		20
#define MAX_PHONE_NUMBER_LENGTH  		(CNI_RIL3_MAX_BCD_DIGITS + 1)
#define MAX_ACTIVE_CALLS   				30
#define VOIP_API_MAGIC_VALUE			1234
#define MAX_VC_ID_STRING_LENGTH			64
#define MAX_VC_IP_ADDRESS_STRING_LENGTH	64
#define VOIP_NULL_CALL_HANDLE           -1
#define VOIP_NULL_MOBILE_HANDLE         -1




typedef T_CNI_RIL3_IE_MOBILE_ID					MOBILE_ID;
typedef short									LUDB_ID;
typedef T_CNI_IRT_ID							TXN_ID;
typedef int							            VOIP_MOBILE_HANDLE;
typedef	short							        VOIP_CALL_HANDLE;
typedef T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER	VOIP_CALL_DESTINATION_ADDRESS;
typedef T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER	VOIP_CALL_CALLING_PARTY_NUMBER;

typedef void ( *pfnCallBackFunction )( PVOID );


#define VOIP_API_NULL_TXN_ID					CC_MAX_CC_CALLS_DEF_VAL

#define MAX_PIPES_IN_ARRAY		5
#define MAX_PIPE_MSG_SIZE		1024

/* RAD input pipe */
typedef struct
{
   int 					hPipe;
   pfnCallBackFunction 	pfnUserCallBackFunction;
   int 					nMsgSize;
   unsigned char		pMsg[MAX_PIPE_MSG_SIZE];  
   int					Magic;	
   BOOL					Used;
}RADPIPE, *PRADPIPE;

PRADPIPE GetPipeFromPool(void);
void ReturnPipeIntoPool(PRADPIPE pRadPipe);






/*
 * Voip-Mobile call events
 */
typedef enum {
	VOIP_MOBILE_CALL_EVENT_BASE=0,
	VOIP_MOBILE_CALL_EVENT_PROCEEDING,
	VOIP_MOBILE_CALL_EVENT_RINGBACK,
	VOIP_MOBILE_CALL_EVENT_ALERTING,
	VOIP_MOBILE_CALL_EVENT_CONNECTED,
	VOIP_MOBILE_CALL_EVENT_VOICE_CONNECTED,
	VOIP_MOBILE_CALL_EVENT_DISCONNECTED,
	VOIP_MOBILE_CALL_EVENT_TRANSFER_STATUS,
	VOIP_MOBILE_CALL_EVENT_PROGRESS,
	MAX_VOIP_MOBILE_CALL_EVENTS
} VOIP_MOBILE_CALL_EVENT;



/* 
 * Voip API message types
 */
typedef enum {
	VOIP_API_CALL_OFFERING,
    VOIP_API_CALL_ALERTING,
	VOIP_API_CALL_SETUP,
	VOIP_API_CALL_RELEASE,
	VOIP_API_CALL_ANSWER,
	VOIP_API_CALL_EVENT,
	VOIP_API_CALL_DTMF,
	VOIP_API_SHUTDOWN_VOIP_TASK,
	VOIP_API_CALL_PROCEEDING,
    VOIP_API_CODEC_SELECTED,
	VOIP_API_CALL_TRANSFER,
	VOIP_API_CALL_FORWARD,
	VOIP_API_CALL_UPDATE_HANDLES,
	MAX_VOIP_API_MESSAGES
} VOIP_API_MESSAGE_TYPE;



typedef struct {

  T_CNI_RIL3_FIELD_SPEECH_VERSION_INDICATOR	speechVersionInd[3];

} VOIP_CALL_BEARER_CAP;

//CLIR <xxu:07-26-01>
typedef struct {
    bool    ie_present;
    T_CNI_RIL3_PRESENTATION_INDICATOR clirSetting;

} VOIP_CALL_CLIR_SETTING;

typedef struct {

  	VOIP_CALL_CALLING_PARTY_NUMBER	CallingPartyNumber;

	VOIP_CALL_DESTINATION_ADDRESS CalledPartyNumber;

    VOIP_CALL_BEARER_CAP bearerCap;

    bool IsEmergencyCall;

    VOIP_CALL_CLIR_SETTING  clir;  //CLIR <xxu:07-26-01>

}VOIP_API_CALL_SETUP_MSG;

typedef struct {

  	VOIP_CALL_CALLING_PARTY_NUMBER	CallingPartyNumber;

	VOIP_CALL_DESTINATION_ADDRESS 	CalledPartyNumber;

} VOIP_API_CALL_OFFERING_MSG;

typedef struct {

    VOIP_CALL_BEARER_CAP bearerCap;

}VOIP_API_CALL_PROCEEDING_MSG;

typedef struct {

    T_CNI_RIL3_SPEECH_VERSION   Codec;

} VOIP_API_CODEC_SELECTED_MSG;

typedef struct {

 /* 
  * FIXME : Additional info to be included if required;
  * otherwise right now this is an empty structure
  * since all required info is already present in msg header
  */
}VOIP_API_CALL_ANSWER_MSG;

/*
 */
typedef struct {

}VOIP_API_CALL_ALERTING_MSG;


typedef struct {

  unsigned char cause;

}VOIP_API_CALL_RELEASE_MSG;

typedef enum {

	DTMF_DIGIT_0,
	DTMF_DIGIT_1,
	DTMF_DIGIT_2,
	DTMF_DIGIT_3,
	DTMF_DIGIT_4,
	DTMF_DIGIT_5,
	DTMF_DIGIT_6,
	DTMF_DIGIT_7,
	DTMF_DIGIT_8,
	DTMF_DIGIT_9,
	DTMF_DIGIT_STAR,
	DTMF_DIGIT_HASH,
	MAX_DTMF_DIGITS

} VOIP_DTMF_DIGIT;

typedef struct {

   VOIP_DTMF_DIGIT	Digit;  /*
   							 * ASCII CODE for the digit to be played;
   							 * to be enumerated later on in this file
   							 */
   unsigned long	Duration; /* described in milliseconds */	

} VOIP_API_CALL_DTMF_MSG;


typedef struct {

	VOIP_MOBILE_CALL_EVENT	CallEvent;

	unsigned char			ReasonCode;

	union {

		T_CNI_RIL3_IE_PROGRESS_INDICATOR	AlertingProgressIE;
		
	} EventInfo;


} VOIP_API_CALL_EVENT_MSG;


typedef struct {

   	VOIP_CALL_CALLING_PARTY_NUMBER	CallingPartyNumber;

	VOIP_CALL_DESTINATION_ADDRESS 	CalledPartyNumber;

} VOIP_CALL_TRANSFER_MSG;


typedef struct {

	VOIP_CALL_DESTINATION_ADDRESS 	ForwardedToNumber;

} VOIP_CALL_FORWARD_MSG;

typedef struct {

	VOIP_CALL_HANDLE				NewHandle;

	BOOL							ReconnectOnTransferFailure;

} VOIP_CALL_UPDATE_HANDLES_MSG;


/*
 * The main VOIP API Message structure definition
 */
typedef struct {

   VOIP_MOBILE_HANDLE		VoipMobileHandle;
   VOIP_CALL_HANDLE			VoipCallHandle;	
   LUDB_ID					LudbId;
   TXN_ID					TxnId;
   unsigned short			H323CRV;

   VOIP_API_MESSAGE_TYPE	MessageType;		
	
   int						Magic;	/* Always set to VOIP_API_MAGIC value for msg integrity validation*/

   union {
	VOIP_API_CALL_SETUP_MSG						CallSetup;
    VOIP_API_CALL_ALERTING_MSG                  CallAlerting;
	VOIP_API_CALL_ANSWER_MSG					CallAnswer;
	VOIP_API_CALL_EVENT_MSG						CallEvent;
	VOIP_API_CALL_OFFERING_MSG					CallOffering;
	VOIP_API_CALL_RELEASE_MSG					CallRelease;
	VOIP_API_CALL_DTMF_MSG						CallDtmf;
	VOIP_API_CALL_PROCEEDING_MSG				CallProceeding;
    VOIP_API_CODEC_SELECTED_MSG                 CodecSelected;
	VOIP_CALL_TRANSFER_MSG						CallTransfer;
	VOIP_CALL_FORWARD_MSG						CallForward;
	VOIP_CALL_UPDATE_HANDLES_MSG				CallUpdateHandles;
   };

} VOIP_API_MESSAGE, *PVOIP_API_MESSAGE;





/**************** Function prototypes ****************************/



/*
 *  To be implemented by CC
 *  The message types that would be handled by CC would include:-
 *  VOIP_API_CALL_OFFERING
 *	VOIP_API_CALL_EVENT
 *
 * 
 */
bool CcVoipMessageHandler(VOIP_API_MESSAGE *VoipMessage);
HJCRTPSESSION VoipCallGetRTPHandle(VOIP_CALL_HANDLE VoipCallHandle); /* macro to get RTP handle */
BOOL VoipSendMessageToCc(PVOIP_API_MESSAGE pMsg);


/*
 *	To be implmeneted by the VOIP task
 *	VOIP_API_CALL_SETUP
 *	VOIP_API_CALL_ANSWER
 *	VOIP_API_CALL_SEND_DTMF
 */
bool VoipApiMessageHandler(VOIP_API_MESSAGE	*VoipMessage);

/* 
 * To be implemented by CSU 
 * void RTPEventHandler( HJCRTPSESSION RtpHandle, VOIP_CALL_HANDLE VoipCallHandle, TXN_ID	TxnId);
 * void	RTPFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,
 *                              int packetLength,rtpParam RtpParam,T_CNI_IRT_ID MobileHandle);
 */



char *GetVoipApiMessageName(VOIP_API_MESSAGE_TYPE MessageType);
char *GetVoipApiCallEventName(VOIP_MOBILE_CALL_EVENT CallEvent);



#endif /* VOIPAPI_HDR_INC */



