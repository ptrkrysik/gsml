
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: H323Interface.cpp												*
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
#include <stdlib.h>
#include <string.h>
#include <cm.h>
#include <stkutils.h>

#include <tickLib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>
#include <seli.h>

#include "Os/JCTask.h"
#include "Os/JCModule.h"
#include "GP10OsTune.h"
#include "logging\vcmodules.h"
#include "logging\vclogging.h"
#include "vipermem.h"

#include "voip\voipapi.h"
#include "h323task.h"
#include "rtptask.h"
#include "util.h"
#include "eventhandlers.h"
#include "smroutines.h"
#include "h323interface.h"
#include "mnetmoduleid.h"
#include "Jcc/LudbApi.h"


extern PH323TASK_CONFIG	pVoipTaskConfig;
extern CALL_CONTROL_BLOCK	CCBArray[MAX_ACTIVE_CALLS];

extern JCTask VOIPTaskObj;






/*
 * H323MessageHandler
 */
void H323MessageHandler(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("H323MessageHandler",H323_LAYER);
	DBG_ENTER();
	BOOL	FreeMsg=TRUE;

	
	
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pMsg->Magic == VOIP_API_MAGIC_VALUE);
#endif
	if (pMsg->Magic != VOIP_API_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid msg received!!\n");
		DBG_LEAVE();
		return;
	}

   	switch( pMsg->MessageType )
   	{
      	case VOIP_API_SHUTDOWN_VOIP_TASK:
		{
         	InterfaceShutdownH323Task();
		}
        break;

      	case VOIP_API_CALL_SETUP:
		{
			InterfaceCallSetup(pMsg);
		}
        break;

      	case VOIP_API_CALL_ANSWER:
		{
			InterfaceCallAnswer(pMsg);
		}
        break;

      	case VOIP_API_CALL_RELEASE:
		{
			InterfaceCallRelease(pMsg);
		}
      	break;

		case VOIP_API_CALL_DTMF:
		{
			InterfaceSendDtmf(pMsg);
		}
		break;

        case VOIP_API_CALL_ALERTING:
        {
            InterfaceCallAlerting(pMsg);
        }
        break;
        case VOIP_API_CALL_PROCEEDING:
        {
            InterfaceCallProceeding(pMsg);
        }
        break;
		case VOIP_API_CALL_TRANSFER:
		{
			InterfaceCallTransfer(pMsg);
		}
		break;
      	default:
         	DBG_ERROR("Unknown message type %d on input pipe!\n", pMsg->MessageType );
        break;
   	}

	DBG_LEAVE();
   	return;
}








/*
 * H323CallGetRtpHandle
 */
HJCRTPSESSION 
H323CallGetRTPHandle(PCALL_CONTROL_BLOCK pCCB)
{
	DBG_FUNC("H323CallGetRTPHandle",H323_LAYER);
	DBG_ENTER();

#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB value returned!\n");
		DBG_LEAVE();
		return NULL;
	}
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->RtpHandle!=NULL);
#endif
	DBG_LEAVE();
	return(pCCB->RtpHandle);

}



/*
 * InterfaceCallSetup
 */
void
InterfaceCallSetup(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceCallSetup",H323_LAYER);
	DBG_ENTER();
    BOOL Status=FALSE;

	/*
	 * FIXME: Lock for glare avoidance
	 */
	PCALL_CONTROL_BLOCK	pCCB;
	pCCB = GetCCBFromPool();
    if (pCCB == NULL) {
        DBG_ERROR("Trying to place more calls than the no of CCBs available!!!\n");
	    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
	    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
	    pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
		pMsg->H323CRV				= pCCB->H323CRV;
        Status=VoipSendMessageToCc( pMsg);
    }
    else {
#ifdef VOIP_DEVELOP_DEBUG
	    ASSERT(pCCB!=NULL);
#endif
	    if (pCCB == NULL)
	    {
		    DBG_ERROR("No more call blocks available to handle call!\n");
	    }	
	    pCCB->LudbId    = pMsg->LudbId;
        pCCB->LastTxnId = pMsg->TxnId;
	    RunStateMachine(pCCB,(PVOID)pMsg,H323_EVENT_RECV_CC_CALL_SETUP);
    }
	DBG_LEAVE();
	return;
}


/* 
 * InterfaceCallAnswer
 */
void
InterfaceCallAnswer(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceCallAnswer",H323_LAYER);
	DBG_ENTER();
    BOOL Status=FALSE;

#ifdef VOIP_DEVELOP_DEBUG
    ASSERT(!((pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) || (pMsg->VoipCallHandle < 0)));
#endif
	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];

	DBG_TRACE("CallAnswer message received for CCB %x\n",pCCB);
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic !=  H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Unknown CCB returned by CC!\n");
		pMsg->MessageType 			= VOIP_API_CALL_EVENT;
		pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
		pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
		pMsg->H323CRV				= pCCB->H323CRV;
	   	Status=VoipSendMessageToCc( pMsg);
	}
	else
	{
		pCCB->LastTxnId = pMsg->TxnId;
		RunStateMachine(pCCB,(PVOID)pMsg,H323_EVENT_RECV_CC_ANSWER_CALL);
	}
	DBG_LEAVE();
	return;
}




/* 
 * InterfaceCallAlerting
 */
void
InterfaceCallAlerting(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceCallAlerting",H323_LAYER);
	DBG_ENTER();
    BOOL Status=FALSE;

#ifdef VOIP_DEVELOP_DEBUG
    ASSERT(!((pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) || (pMsg->VoipCallHandle < 0)));
#endif
	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];

	DBG_TRACE("CallAlerting message received for CCB %x\n",pCCB);
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic !=  H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Unknown CCB returned by CC!\n");
		pMsg->MessageType 			= VOIP_API_CALL_EVENT;
		pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
		pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
		pMsg->H323CRV				= pCCB->H323CRV;
	   	Status=VoipSendMessageToCc( pMsg);
	}
	else
	{
        DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : --> Q.931 ALERTING being sent\n",pCCB->MobileNumber);
		pCCB->LastTxnId = pMsg->TxnId;
		cmCallAccept(pCCB->LanCallHandle);
	}
	DBG_LEAVE();
	return;
}



/*
 * InterfaceCallRelease
 */
void
InterfaceCallRelease(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceCallRelease",H323_LAYER);
	DBG_ENTER();
    PCALL_CONTROL_BLOCK pCCB ;
    BOOL Status=FALSE;

    if (pMsg->VoipCallHandle != VOIP_NULL_CALL_HANDLE) {
#ifdef VOIP_DEVELOP_DEBUG
        ASSERT(!((pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) || (pMsg->VoipCallHandle < 0)));
#endif
	    pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];
	    DBG_TRACE("CallRelease message received for CCB %x\n",pCCB);
#ifdef VOIP_DEVELOP_DEBUG
	    ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	    if (pCCB->Magic !=  H323TASK_MAGIC_VALUE)
	    {
		    DBG_ERROR("Unknown CCB returned by CC!\n");
		    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
		    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
		    pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
			pMsg->H323CRV				= pCCB->H323CRV;
	   	    Status=VoipSendMessageToCc( pMsg);
	    }
	    else
	    {
            /*
             * Check for glare avoidance
             */
            if (pCCB->H323CallState==H323_CALL_STATE_IDLE) {
                DBG_ERROR("Received CC Call Release for an IDLE CCB!!\n");
                //DumpStateTransitionHistory(pCCB);
            }
            else {
		        pCCB->LastTxnId = pMsg->TxnId;
                DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : --> Q.931 RELEASE being sent\n",pCCB->MobileNumber);
		        RunStateMachine(pCCB,(PVOID)pMsg,H323_EVENT_RECV_CC_CALL_RELEASE);
            }
	    }
    }
    else {
	    int Count;

	    for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	    {
		    pCCB = &CCBArray[Count];
            if (pCCB->LastTxnId != pMsg->TxnId) continue;
            else break;
        }
        if (pCCB->LastTxnId != pMsg->TxnId) {
            DBG_ERROR("CC requested call release with NULL VOIP handle, cant find CCB !!!\n");
        }
        else {
            DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : --> Q.931 RELEASE being sent\n",pCCB->MobileNumber);
		    RunStateMachine(pCCB,(PVOID)pMsg,H323_EVENT_RECV_CC_CALL_RELEASE);
        }

    }
	DBG_LEAVE();
	return;
}




/*
 * InterfaceSendDtmf
 */
void
InterfaceSendDtmf(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceSendDtmf",H323_LAYER);
	DBG_ENTER();
	char userInput[10];
	PCALL_CONTROL_BLOCK pCCB;

	memset(userInput,0,10);

#ifdef VOIP_DEVELOP_DEBUG
    ASSERT(!((pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) || (pMsg->VoipCallHandle < 0)));
#endif
	pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	switch(pMsg->CallDtmf.Digit)
	{
		case DTMF_DIGIT_0:strcpy(userInput,"0");break;
		case DTMF_DIGIT_1:strcpy(userInput,"1");break;
		case DTMF_DIGIT_2:strcpy(userInput,"2");break;
		case DTMF_DIGIT_3:strcpy(userInput,"3");break;
		case DTMF_DIGIT_4:strcpy(userInput,"4");break;
		case DTMF_DIGIT_5:strcpy(userInput,"5");break;
		case DTMF_DIGIT_6:strcpy(userInput,"6");break;
		case DTMF_DIGIT_7:strcpy(userInput,"7");break;
		case DTMF_DIGIT_8:strcpy(userInput,"8");break;
		case DTMF_DIGIT_9:strcpy(userInput,"9");break;
		case DTMF_DIGIT_HASH:strcpy(userInput,"#");break;
		case DTMF_DIGIT_STAR:strcpy(userInput,"*");break;
		default:DBG_ERROR("Unknown DTMF digit %d specified\n",pMsg->CallDtmf.Digit);break;
	}
    pCCB->LastTxnId = pMsg->TxnId;

	DBG_TRACE("Sending DTMF digit %s\n",userInput);
    cmUserInputData userData={userInput,strlen(userInput)};
    cmCallSendUserInput(pCCB->LanCallHandle,cmUserInputBuildAlphanumeric(pVoipTaskConfig->StackHandle, &userData));

	DBG_LEAVE();
}



/*
 * InterfaceShutdownH323Task
 */
void InterfaceShutdownH323Task()
{
	DBG_FUNC("InterfaceShutdownH323Task",H323_LAYER);
	DBG_ENTER();

	DBG_TRACE("Shutting down H323 task!\n");

	SignalRtpTaskShutdown();

    cmUnregister(pVoipTaskConfig->StackHandle);
    cmEnd(pVoipTaskConfig->StackHandle);

    RadPipeClose(pVoipTaskConfig->pH323TaskInputPipe);

	DBG_LEAVE();
	exit(1);	 
}



/*
 * InterfaceCallProceeding
 */
void
InterfaceCallProceeding(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceCallProceeding",H323_LAYER);
	DBG_ENTER();
    BOOL Status=FALSE;

#ifdef VOIP_DEVELOP_DEBUG
    ASSERT(!((pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) || (pMsg->VoipCallHandle < 0)));
#endif
	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic !=  H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Unknown CCB returned by CC!\n");
	}
	else
	{
		pCCB->LastTxnId = pMsg->TxnId;
        int count;
        for(count=0;count<3;count++) {
            if (pMsg->CallProceeding.bearerCap.speechVersionInd[count].fieldPresent) {
                if (pMsg->CallProceeding.bearerCap.speechVersionInd[count].version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1) {
                    pCCB->GSMFullRateCapability=TRUE;
                }
                if (pMsg->CallProceeding.bearerCap.speechVersionInd[count].version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2) {
                    pCCB->GSMEnhancedFullRateCapability=TRUE;
                }
            }
        }

        DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : --> Q.931 PROCEEDING being sent\n",pCCB->MobileNumber);
        if (cmCallSendCallProceeding(pCCB->LanCallHandle) != TRUE) {
            DBG_ERROR("Error sending call proceeding on mobile %s\n",pCCB->MobileNumber);
        }
	}

	if (pCCB->FastStart) {
	    cmTransportAddress rtp;
	    cmTransportAddress rtcp;

    	rtp.port	= jcRtpGetPort(pCCB->RtpHandle)+1;
    	rtp.ip 		= 0;
    	rtcp.port 	= jcRtpGetPort(pCCB->RtpHandle);
    	rtcp.ip 	= 0;


		if (pCCB->RemoteGSMEnhancedFullRateCapability && pCCB->GSMEnhancedFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_EFR;
    		cmFastStartChannelsAckIndex(pCCB->LanCallHandle,pCCB->remoteGsmEfrTxIndex,&rtp,&rtcp);
    		cmFastStartChannelsAckIndex(pCCB->LanCallHandle,pCCB->remoteGsmEfrRxIndex,&rtp,&rtcp);
	    	cmFastStartChannelsReady(pCCB->LanCallHandle);
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_EFR_PAYLOAD_TYPE);
		    SendCodecInfo(pCCB);    
		}
		else if (pCCB->RemoteGSMFullRateCapability && pCCB->GSMFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
    		cmFastStartChannelsAckIndex(pCCB->LanCallHandle,pCCB->remoteGsmFrTxIndex,&rtp,&rtcp);
    		cmFastStartChannelsAckIndex(pCCB->LanCallHandle,pCCB->remoteGsmFrRxIndex,&rtp,&rtcp);
	    	cmFastStartChannelsReady(pCCB->LanCallHandle);
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_PAYLOAD_TYPE);
		    SendCodecInfo(pCCB);    
		}
		else {
			DBG_ERROR("Capabilities mismatch!!\n");
	        RunStateMachine(pCCB,(PVOID)pMsg,H323_EVENT_RECV_CC_CALL_RELEASE);
		}
	}
	else {
		if (pCCB->GSMEnhancedFullRateCapability) {
			if (pVoipTaskConfig->DefaultCodecGsmFullRate) {
				DBG_TRACE("Remote is Non-FastStart endpoint, defaulting to FullRate codec\n");
				pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
    			jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_PAYLOAD_TYPE);
			}
			else {
				DBG_TRACE("Remote is Non-FastStart endpoint, defaulting to EnhancedFullRate codec\n");
				pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_EFR;
    			jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_EFR_PAYLOAD_TYPE);
			}
		    SendCodecInfo(pCCB);    
		}
		else if (pCCB->GSMFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_PAYLOAD_TYPE);
		    SendCodecInfo(pCCB);    
		}
		else {
			DBG_ERROR("Neither FullRate NOR EnhancedFullRate codec supported by mobile %s\n",pCCB->MobileNumber);
#ifdef VOIP_DEVELOP_DEBUG
			ASSERT(0);
#endif
		}
	}


    DBG_LEAVE();
}


static int dropTrdCallTimer=5;

/*
 * InterfaceCallTransfer
 */
void
InterfaceCallTransfer(PVOIP_API_MESSAGE pMsg)
{
	DBG_FUNC("InterfaceCallTransfer",H323_LAYER);
	DBG_ENTER();
    BOOL Status=FALSE;
	char NumberBuffer[256],DestinationNumber[256];

#ifdef VOIP_DEVELOP_DEBUG
    ASSERT(!((pMsg->VoipCallHandle > MAX_ACTIVE_CALLS) || (pMsg->VoipCallHandle < 0)));
#endif
	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)&CCBArray[pMsg->VoipCallHandle];
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic !=  H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Unknown CCB returned by CC!\n");
	}
	else
	{
	    memset(NumberBuffer,0,256);
	    memset(DestinationNumber,0,256);
	   	jcBCDToString(NumberBuffer, pMsg->CallTransfer.CalledPartyNumber.digits, 
	   			pMsg->CallSetup.CalledPartyNumber.numDigits );
	    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Transferring active call to %s\n",pCCB->MobileNumber,NumberBuffer);
		DBG_TRACE("Mobile %s : Transferring active call to %s\n",pCCB->MobileNumber,NumberBuffer);

		sprintf(DestinationNumber,"TEL:%s,TA:%s:1720",NumberBuffer,pVoipTaskConfig->szGatekeeperAddress);
		/*sprintf(DestinationNumber,"TEL:%s,TA:127.0.0.1:1720",NumberBuffer);*/
		DBG_TRACE("Transfer destination address given as %s\n",DestinationNumber);
		sseCallTransfer(pCCB->SuppCallHandle,NULL,DestinationNumber);

	}

	
	DBG_LEAVE();
	return;
}




/*
 * SendMessage
 */
bool SendMessage(PRADPIPE pInputPipe,PVOIP_API_MESSAGE pMsg)
{
	return (RadPipeWrite( pInputPipe, pMsg));
}


/*
 * SendMessageToRtp
 */
bool SendMessageToRtp(PRADPIPE pInputPipe,RtpInputMessage_t *pMsg)
{
	return (RadPipeWrite(pInputPipe,pMsg));
}








/*
 * VoipShowAllCalls
 */
void
VoipShowAllCalls()
{
	DBG_FUNC("VoipShowAllCalls",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK pCCB;
	int Count,TotalCalls=0;
	BOOL Show=FALSE;

	for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	{
		pCCB = &CCBArray[Count];
		if (pCCB->H323CallState != H323_CALL_STATE_IDLE) {
            TotalCalls++;
            DisplayCallInfo(pCCB,Count,TotalCalls);
			Show=TRUE;
		}
	}
	if (Show==TRUE) {
		printf("--------------------------------------------------------------------------\n");
		printf("Total : %d : All other VOIP Call Control Blocks IDLE/Unused\n",TotalCalls);
		printf("--------------------------------------------------------------------------\n");
	}
	else {
		printf("No Calls active\n");
	}
	DBG_LEAVE();
	return;
}


/*
 * VoipShowCall
 */
void
VoipShowCall(PCALL_CONTROL_BLOCK pCCB)
{
	DBG_FUNC("VoipShowCall",H323_LAYER);
	DBG_ENTER();
	char DisplayBuffer[8000];
	memset(DisplayBuffer,0,8000);

	if (pCCB->Magic != H323TASK_MAGIC_VALUE) {
		printf("Invalid CCB specified\n");
		DBG_ERROR("Invalid CCB specified\n");
		DBG_LEAVE();
		return;
	}
	if (pCCB->H323CallState != H323_CALL_STATE_IDLE) {
	    PCALL_CONTROL_BLOCK NewCCB;
        int Count;
	    for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	    {
		    NewCCB = &CCBArray[Count];
            if (NewCCB==pCCB) break;
        }
        if (NewCCB==pCCB) {
           DisplayCallInfo(pCCB,Count,0);
			printf("--------------------------------------------------------------------------\n");
        }
	}
	else {	
		printf("CCB not in use");
	}
	DBG_LEAVE();
}






/*
 * DisplayCallInfo
 */
void
DisplayCallInfo(PCALL_CONTROL_BLOCK pCCB,int index, int displayIndex)
{

	char stateBuffer[128];
	int stripLen;

	printf("------------------------- Call Block Index %02d---------------------------\n",pCCB->Index);
	printf(" Call Control Block        : %#x\n",pCCB);
	printf(" Call Direction            :");
	if (pCCB->CallDirection == H323_CALL_TERMINATED) {
		printf(" Locally Terminated\n");
	}
	else {
		printf(" Locally Originated\n");
	}

	memset(stateBuffer,0,128);
	strncpy(stateBuffer,GetH323CallStateName(pCCB->H323CallState),128);
	stripLen = strlen("H323_CALL_STATE_");
	memset(stateBuffer,0,stripLen);
	printf(" Call State                : %s\n",&stateBuffer[stripLen]);
	printf(" Local MS-ISDN Number      : %s\n",pCCB->MobileNumber);
	if (pCCB->IsSuppCall) {
		printf(" Remote Party Number       : %s; transferred from %s\n",pCCB->RemotePartyId,
											pCCB->TransferringPartyId);
	}
	else {
		printf(" Remote Party Number       : %s\n",pCCB->RemotePartyId);
	}
	printf(" Negotiated Codec          :");
	if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_FR) {
		printf(" GSM Full Rate\n");
	}
	else {
		printf(" GSM Enhanced Full Rate\n");
	}

	printf(" H.323 Calling Method      :");
	if (pCCB->FastStart) {
		printf(" FastSetup\n");
	}
	else {
		printf(" Slow Start\n");
	}

	printf(" H.323 Call Reference Val  : %#x\n",pCCB->H323CRV);
	printf(" Associated RTP Handle     : %#x\n",pCCB->RtpHandle);
	printf(" RTP remote IP-Port        : %s\n",
					RtpGetRemoteTsapString(pCCB->RtpHandle));
	printf(" RTP local IP-Port         : %s\n",
					RtpGetLocalTsapString(pCCB->RtpHandle));
	printf(" RTP octets TX             : %8ld\n",
					RtpGetSessionBytesSent(pCCB->RtpHandle));
	printf(" RTP octets RX             : %8ld\n",
					RtpGetSessionBytesRecvd(pCCB->RtpHandle));


}









/*
 * VoipShowCallCounters
 */
void 
VoipShowCallCounters()
{
    double IncomingPercentage,OutgoingPercentage;


    IncomingPercentage = (double)pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallsConnected/pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallAttempts;
    OutgoingPercentage = (double)pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallsConnected/pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallAttempts;

    IncomingPercentage = IncomingPercentage*100;
    OutgoingPercentage = OutgoingPercentage*100;

    printf("\n*************************** VOIP CALL COUNTERS *********************\n");
    printf("Incoming call attempts : %ld ; Incoming calls connected %ld; Success rate %00.4f\n",
        pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallAttempts,pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallsConnected,IncomingPercentage);
    printf("Outgoing call attempts : %ld ; Outgoing calls connected %ld; Success rate %00.4f\n",
        pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallAttempts,pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallsConnected,OutgoingPercentage);
    printf("********************************************************************\n");
}


/*
 * VoipClearCallStats
 */
void 
VoipClearCallStats()
{
    memset(&pVoipTaskConfig->CallStats,0,sizeof(pVoipTaskConfig->CallStats));

    printf("\nVoip Call Statistics cleared\n");

}


/*
 * VoipShowCallStats
 */
void
VoipShowCallStats()
{

    double IncomingPercentage,OutgoingPercentage;
	int count;
	unsigned long remoteDiscs;

    if (pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallAttempts != 0) {
        IncomingPercentage = (double)pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallsConnected/pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallAttempts;
        IncomingPercentage = IncomingPercentage*100;
    }
    else {
        IncomingPercentage = 0;
    }
    if (pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallAttempts != 0) {
        OutgoingPercentage = (double)pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallsConnected/pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallAttempts;
        OutgoingPercentage = OutgoingPercentage*100;
    }
    else {
        OutgoingPercentage = 0;
    }

    printf("\n*************************** VOIP CALL STATS ************************\n");
    printf("--------------------------- VOIP CALL COUNTERS ---------------------\n");
    printf("Incoming call attempts : %ld ; Incoming calls connected %ld; Success rate %00.4f\n",
        pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallAttempts,pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallsConnected,IncomingPercentage);
    printf("Outgoing call attempts : %ld ; Outgoing calls connected %ld; Success rate %00.4f\n",
        pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallAttempts,pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallsConnected,OutgoingPercentage);

    printf("------------------------ LOCALLY INITIATED DISCONNECTS -----------------------\n");
	for(count=0;count<Q850_MAX_CAUSES;count++) {
		if (pVoipTaskConfig->CallStats.CallDisconnectStats.MobileDisconnects[count] > 0) {
			printf("%d   : %s\n",pVoipTaskConfig->CallStats.CallDisconnectStats.MobileDisconnects[count],
				GetQ850CauseCodeName(count));
		}
	}


    printf("------------------------- DISCONNECTS FROM REMOTE ENDPOINT -------------------\n");
	/*
	 * Keep in mind that every disconnect, whether locally initiated or
	 * from the remote side triggers the VOIP call state disconnected state change,
	 * where we increment the counter; so for an accurate picture of remote discs
	 * we need to subtract local from total
	 */
	for(count=0;count<Q850_MAX_CAUSES;count++) {
		remoteDiscs = pVoipTaskConfig->CallStats.CallDisconnectStats.VoipDisconnects[count] - 
						pVoipTaskConfig->CallStats.CallDisconnectStats.MobileDisconnects[count];

		if (remoteDiscs > 0) {
			printf("%d   : %s\n",remoteDiscs,
				GetQ850CauseCodeName(count));
		}
	}

    printf("********************************************************************\n");



}



#ifndef LOOPBACK_MODE

/*
 * SysCommand_Voip
 */
int SysCommand_Voip(T_SYS_CMD action)
{
    DBG_FUNC("SysCommand_VOIP",H323_LAYER);
    DBG_ENTER();
    switch(action){
        case SYS_SHUTDOWN:
        {
           	STATUS Status = STATUS_FAILURE;
            printf("VOIP TASK shutdown notification received\n");
            DBG_WARNING("VOIP TASK shutdown notification received\n");
	        PAPI_MESSAGE	ApiMsg;
	        ApiMsg = GetFreeMsgFromPool();
	        if (ApiMsg == NULL) {
		        DBG_ERROR("All API messages used up from pool!\n");
		        DBG_LEAVE();
		        return FALSE;
	        }
	        PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;
	        pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	        pMsg->MessageType 			= VOIP_API_SHUTDOWN_VOIP_TASK;
	        Status = VoipApiMessageHandler(pMsg);
	 
	        PutMsgIntoPool(ApiMsg);
        }
            break;
        case SYS_START:
        {
			printf("Initializing JetRtp<-->MotFec Interface first\n");
		    InitializeJetRtp();

            printf("Spawning VOIP Task now\n");


	        if (VOIPTaskObj.JCTaskSpawn( H323_TASK_PRIORITY, H323_TASK_OPTION, H323_TASK_STACK_SIZE,
			(FUNCPTR) InitializeH323Task, 0,0,0,0, 0, 0, 0, 0, 0, 0,MODULE_H323,JC_CRITICAL_TASK) == ERROR){  
		        printf("Unable to spawn VOIP task!!\n");
	        }
	        else {
		        printf("Spawned VOIPTask successfully!\n");
	        }



            break;
        }
    }
    DBG_LEAVE();
    return 0;

}
#endif /* #ifndef LOOPBACK_MODE */
