
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: VoipAPI.cpp													*
 *																						*
 *	Description			: API Library for the Voip task									*
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

#include <stdlib.h>
#include <string.h>
#include <vxworks.h>
#include <ticklib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>
#include "msg.h"
#include "logging/vcmodules.h"
#include "logging/vclogging.h"
#include "vipermem.h"

#include "voip/voipapi.h"
#include "voip/vblink.h"
#include "h323task.h"
#include "h323interface.h"
#include "Jcc/LudbApi.h"


extern PH323TASK_CONFIG	    pVoipTaskConfig;
extern CALL_CONTROL_BLOCK	CCBArray[MAX_ACTIVE_CALLS];



/*
 * VoipApiMessageHandler
 */
bool 
VoipApiMessageHandler(PVOIP_API_MESSAGE	VoipRequest)
{
	DBG_FUNC("VoipApiMessageHandler",H323_LAYER);
	DBG_ENTER();
	
	if (VoipRequest->MessageType == VOIP_API_CALL_EVENT) {
		DBG_TRACE("VOIP <<--- CallEvent : %s ---- CC : Txn ID %d \n",GetVoipApiCallEventName(VoipRequest->CallEvent.CallEvent),
	                    VoipRequest->TxnId);
	}
	else {
		DBG_TRACE("VOIP <<--- %s ---- CC : Txn ID %d \n",GetVoipApiMessageName(VoipRequest->MessageType),
	                    VoipRequest->TxnId);
	}


	if (!VBLinkStatus()) {
		DBG_ERROR("API Function called when ViperCell still NOT connected with ViperBase!\n");
		DBG_LEAVE();
		return FALSE;
	}
	
	/* 
	 * Perform basic validations on the request so that
	 * we can return an error from here itself
	 */
	if (VoipRequest->Magic != VOIP_API_MAGIC_VALUE) {
		DBG_ERROR("Invalid API message received!\n");
		DBG_LEAVE();
		return(FALSE);
	}


    /*
     * Check VOIP handle for all msgs for which it is expected to be valid
     */
	switch(VoipRequest->MessageType) {
        case VOIP_API_CALL_ALERTING:
        case VOIP_API_CALL_ANSWER:
        case VOIP_API_CALL_DTMF:
        case VOIP_API_CALL_PROCEEDING:
            if( (VoipRequest->VoipCallHandle > MAX_ACTIVE_CALLS) || (VoipRequest->VoipCallHandle < 0) ) {
                DBG_ERROR("Unknown call handle %d returned for msg %s\n",VoipRequest->VoipCallHandle,GetVoipApiMessageName(VoipRequest->MessageType));
                return(FALSE);
            }
    }



	switch(VoipRequest->MessageType) {
		case VOIP_API_CALL_SETUP:
		{
   			if(VoipRequest->CallSetup.CalledPartyNumber.numDigits <= 0) {
				DBG_ERROR("Invalid no specified for call setup!\n");
      			return FALSE;
			}
		}
		break;
        case VOIP_API_CALL_ALERTING:
        case VOIP_API_CALL_ANSWER:
        case VOIP_API_CALL_DTMF:
        case VOIP_API_CALL_PROCEEDING:
        {
#ifdef VOIP_DEVELOP_DEBUG
            ASSERT(!((VoipRequest->VoipCallHandle > MAX_ACTIVE_CALLS) || (VoipRequest->VoipCallHandle < 0)));
#endif
            PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[VoipRequest->VoipCallHandle];
            if ((pCCB != NULL) && (pCCB->Magic == H323TASK_MAGIC_VALUE)) {
                if (pCCB->H323CallState==H323_CALL_STATE_IDLE) {
                    DBG_ERROR("Call %#x : Received msg %s from CC when call idle\n",pCCB,GetVoipApiMessageName(VoipRequest->MessageType));
                    DumpStateTransitionHistory(pCCB);
                    return(FALSE);
                }
            }
            
        }
        break;
	}
    if (VoipRequest->MessageType==VOIP_API_CALL_RELEASE) {
        if (VoipRequest->VoipCallHandle != VOIP_NULL_CALL_HANDLE)  {
#ifdef VOIP_DEVELOP_DEBUG
            ASSERT(!((VoipRequest->VoipCallHandle > MAX_ACTIVE_CALLS) || (VoipRequest->VoipCallHandle < 0)));
#endif
			if ((VoipRequest->VoipCallHandle > MAX_ACTIVE_CALLS) || (VoipRequest->VoipCallHandle < 0))
				return (false);
        }
    }

	DBG_LEAVE();
	return(SendMessage(pVoipTaskConfig->pH323TaskInputPipe,VoipRequest));
}



/*
 * VoipSendMessageToCc
 */
BOOL
VoipSendMessageToCc(PVOIP_API_MESSAGE pMsg)
{
    DBG_FUNC("VoipSendMessageToCc",H323_LAYER);
    DBG_ENTER();
    BOOL retStatus=FALSE;


	if (pMsg->MessageType == VOIP_API_CALL_EVENT) {
		DBG_TRACE("VOIP --- CallEvent : %s ---->> CC : Txn ID %d \n",GetVoipApiCallEventName(pMsg->CallEvent.CallEvent),
	                    pMsg->TxnId);
	}
	else {
		DBG_TRACE("VOIP --- %s ---->> CC : Txn ID %d \n",GetVoipApiMessageName(pMsg->MessageType),
	                    pMsg->TxnId);
	}


    retStatus= CcVoipMessageHandler(pMsg);
    if (!retStatus) {
#ifdef VOIP_DEVELOP_DEBUG
        ASSERT(pMsg->VoipCallHandle < MAX_ACTIVE_CALLS);
#endif
		if (pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) {
			return (FALSE);
		}
        PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];
        char call_event_msg[256];
        memset(call_event_msg,0,256);
        if (pMsg->MessageType == VOIP_API_CALL_EVENT) {
            switch(pMsg->CallEvent.CallEvent) {
            	case VOIP_MOBILE_CALL_EVENT_PROCEEDING:strcpy(call_event_msg,"PROCEEDING");break;
	            case VOIP_MOBILE_CALL_EVENT_RINGBACK:strcpy(call_event_msg,"RINGBACK");break;
	            case VOIP_MOBILE_CALL_EVENT_ALERTING:strcpy(call_event_msg,"ALERTING");break;
	            case VOIP_MOBILE_CALL_EVENT_CONNECTED:strcpy(call_event_msg,"CONNECTED");break;
	            case VOIP_MOBILE_CALL_EVENT_VOICE_CONNECTED:strcpy(call_event_msg,"VOICE_CONNECTED");break;
	            case VOIP_MOBILE_CALL_EVENT_DISCONNECTED:strcpy(call_event_msg,"DISCONNECTED");break;

            }
            DBG_ERROR("Call %#x : CC returned error for VOIP_API_CALL_EVENT msg %s; going to drop call!!!!\n",
                pCCB,call_event_msg);
        }
        else {
            DBG_ERROR("CC returned error for msg %s; going to drop call!!!!\n",GetVoipApiMessageName(pMsg->MessageType));
        }
	    if(pCCB->H323CallState!=H323_CALL_STATE_IDLE) {
            pMsg->MessageType       = VOIP_API_CALL_RELEASE;
            pMsg->CallRelease.cause = Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
            VoipApiMessageHandler(pMsg);
        }
    }

    DBG_LEAVE();
    return(retStatus);
}



/*
 * VoipCallGetRTPHandle 
 */
HJCRTPSESSION 
VoipCallGetRTPHandle(VOIP_CALL_HANDLE VoipCallHandle)
{
#ifdef VOIP_DEVELOP_DEBUG
    ASSERT(!((VoipCallHandle > MAX_ACTIVE_CALLS) || (VoipCallHandle < 0)));
#endif
	if ((VoipCallHandle > MAX_ACTIVE_CALLS) || (VoipCallHandle < 0)) {
		return (NULL);
	}
    PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK )&CCBArray[VoipCallHandle];
	return(H323CallGetRTPHandle(pCCB));
}




/*
 * GetVoipApiMessageName
 */
char 
*GetVoipApiMessageName(VOIP_API_MESSAGE_TYPE MessageType)
{
	static char *ApiNames[] = {
		"VOIP_API_CALL_OFFERING",
	    "VOIP_API_CALL_ALERTING",
		"VOIP_API_CALL_SETUP",
		"VOIP_API_CALL_RELEASE",
		"VOIP_API_CALL_ANSWER",
		"VOIP_API_CALL_EVENT",
		"VOIP_API_CALL_DTMF",
		"VOIP_API_SHUTDOWN_VOIP_TASK",
		"VOIP_API_CALL_PROCEEDING",
	    "VOIP_API_CODEC_SELECTED",
		"VOIP_API_CALL_TRANSFER",
		"VOIP_API_CALL_FORWARD",
		"VOIP_API_UPDATE_HANDLES"
	};
	if ((MessageType >= 0) && (MessageType < MAX_VOIP_API_MESSAGES)) {
		return (ApiNames[MessageType]);
	}
}


/*
 * GetVoipApiCallEventName
 */
char *
GetVoipApiCallEventName(VOIP_MOBILE_CALL_EVENT CallEvent)
{
	static char *CallEventNames[] = {
		"VOIP_MOBILE_CALL_EVENT_BASE",
		"VOIP_MOBILE_CALL_EVENT_PROCEEDING",
		"VOIP_MOBILE_CALL_EVENT_RINGBACK",
		"VOIP_MOBILE_CALL_EVENT_ALERTING",
		"VOIP_MOBILE_CALL_EVENT_CONNECTED",
		"VOIP_MOBILE_CALL_EVENT_VOICE_CONNECTED",
		"VOIP_MOBILE_CALL_EVENT_DISCONNECTED",
		"VOIP_MOBILE_CALL_EVENT_TRANSFER_STATUS",
        "VOIP_MOBILE_CALL_EVENT_PROGRESS"
	};
	if ((CallEvent >= 0) && (CallEvent < MAX_VOIP_MOBILE_CALL_EVENTS)) {
		return (CallEventNames[CallEvent]);
	}

}

/*
 * VoipStackSetDefaultCodec
 */
int
VoipStackSetDefaultCodec(int a)
{
	if (!a) {
		pVoipTaskConfig->DefaultCodecGsmFullRate = TRUE;
		printf("Default Codec set to GSM Full Rate for non-fastStart endpoints\n");
	}
	else {
		pVoipTaskConfig->DefaultCodecGsmFullRate = FALSE;
		printf("Default Codec set to GSM Enhanced Full Rate for non-fastStart endpoints\n");
	}
	return (0);
}

/*
 * VoipStackLogEnableModule
 */
int 
VoipStackLogEnableModule(char *moduleName)
{
    return(msAdd(moduleName));
}

/*
 * VoipStackLogDisableModule
 */
int
VoipStackLogDisableModule(char *moduleName)
{
    return(msDelete(moduleName));
}


/*
 * VoipStackLogDisableAll
 */
int
VoipStackLogDisableAll()
{
    return(msDeleteAll());
}


/*
 * VoipStackLogGetDebugLevel
 */
int
VoipStackLogGetDebugLevel()
{
    return(msGetDebugLevel());
}

/*
 * VoipStackLogSetLogFile
 */
void
VoipStackLogSetLogFile(char *logFile)
{
    msLogFile(logFile);
    return;
}


/*
 * VoipStackLogSetDebugLevel
 */
int
VoipStackLogSetDebugLevel(int debugLevel)
{
    return(msSetDebugLevel(debugLevel));
}



/*
 * VoipStackLogAddSink
 */
int
VoipStackLogAddSink(char *sinkName)
{
    return(msSinkAdd(sinkName));
}


/*
 * VoipStackLogRemoveSink
 */
int
VoipStackLogRemoveSink(char *sinkName)
{
    return(msSinkDelete(sinkName));
}



/*
 * VoipHelp
 */
int
VoipHelp()
{
    printf("\n*****************************************************************************");
    printf("\n*                VOIP Task Help                                             *");
    printf("\n*****************************************************************************");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackSetDefaultCodec(x)     : x non-zero sets codec to EFR; otherwise*");
    printf("\n*                                    the default codec is FullRate          *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipShowAllCalls()              : Shows all calls currently active within*");
    printf("\n*                                    the VOIP task                          *");
    printf("\n*  VoipShowCall(CCB)               : Shows the state of the call having     *");
    printf("\n*                                    the control block CCB                  *");
    printf("\n*  VoipShowCallCounters()          : Shows the VOIP call counters           *");     
    printf("\n*                                                                           *");     
    printf("\n*  VoipShowCallStats()             : Displays VOIP call statistics          *");     
    printf("\n*                                                                           *");     
    printf("\n*  VoipClearCallStats()            : Clears the VOIP call statistics        *");     
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogEnableModule(Name)  : Enables stack logging of module        *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogDisableModule(Name) : Disables stack logging of module       *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogDisableAll()        : Disables all stack logging             *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogGetDebugLevel()     : Shows the current debug level of stack *");
    printf("\n*                                    logging                                *");
    printf("\n*  VoipStackLogSetDebugLevel(x)    : Sets the debug level of stack logs to x*");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogSetLogFile(File)    : Sets the output stack log file         *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogAddSink(Name)       : Adds log output device to log          *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipStackLogRemoveSink(Name)    : Removes log output device              *");
    printf("\n*                                                                           *");     
    printf("\n*  VoipOptionsHelp()               : Displays help on options for CLID &    *");     
    printf("\n*                                    Stack Logging                          *");     
    printf("\n*                                                                           *");     
    printf("\n*  VoipHelp()                      : Shows this screen                      *");     
    printf("\n*                                                                           *");     
    printf("\n*****************************************************************************\n");




   return(0);
}


/*
 * VoipOptionsHelp
 */
int
VoipOptionsHelp()
{
    printf("\n*****************************************************************************");
    printf("\n*                   VOIP STACK TRACING/LOGGING HELP                         *");     
    printf("\n*****************************************************************************");
    printf("\n*                                                                           *");     
    printf("\n*------------------- Stack Logging options ---------------------------------*");     
    printf("\n*  Sink options        : 'terminal','file','logger'                         *");     
    printf("\n*                                                                           *");     
    printf("\n*  Debug levels :-                                                          *");     
    printf("\n*    0 : All debugging turned off; default value                            *");     
    printf("\n*    1 : Filter WITHOUT checks and trees                                    *");     
    printf("\n*    2 : Filter WITHOUT checks                                              *");     
    printf("\n*    3 : Filter WITH checks and trees                                       *");     
    printf("\n*    4 : Dont filter, display all messages                                  *");     
    printf("\n*                                                                           *");     
    printf("\n*  Module Names :-                                                          *");     
    printf("\n*    'PERERR','CMERR','PDLERR','VT','RA','CMAPI','CMAPICB','CM','TPKTCHAN', *");     
    printf("\n*    'UDPCHAN','CHANNELS','PDL','PER','Q931','PDLPRINT','PDLPRNERR','LI',   *");     
    printf("\n*    'PDLPRNWRN','PDLERROR','PDLTIMER','PDLSM','PDLMISC','PDLSRC','SELI',   *");     
    printf("\n*    'PDLFNERR','PDLAPI','PDLCONF','PDLMTASK','PDLAPI','PDLCOMM','PDLCHAN', *");     
    printf("\n*    'LOOPBACK','CONF','TIMER','LIINFO'                                     *");     
    printf("\n*                                                                           *");     
    printf("\n*                                                                           *");     
    printf("\n*****************************************************************************\n");

}

#ifdef LOOPBACK_MODE

#include "voipapitest.cpp"

#endif /* #ifdef LOOPBACK_MODE */
