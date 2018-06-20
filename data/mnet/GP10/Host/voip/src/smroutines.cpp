
/****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: smroutines.cpp												*
 *																						*
 *	Description			: State Machine functions/procedures							*
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

#include <stdlib.h>
#include <string.h>
#include <cm.h>
#include <stkutils.h>
#include <tickLib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>


#include "h323task.h"
#include "ccb.h"
#include "util.h"
#include "eventhandlers.h"
#include "rtptask.h"
#include "smroutines.h"
#include "voip\voipapi.h"
#include "voip\vblink.h"
#include "voip\jetrtp.h"
#include "jcc\ludbapi.h"

extern H323_CALL_STATE_INFO H323_CALL_STATE_TABLE[H323_MAX_CALL_STATES][H323_MAX_CALL_EVENTS];
extern H323_CALL_STATE_INFO H323_FASTCALL_STATE_TABLE[H323_MAX_CALL_STATES][H323_MAX_CALL_EVENTS];
extern PH323TASK_CONFIG	pVoipTaskConfig;

#define LOCK_H323_CALL_STATE_MACHINE()	semTake(pCCB->H323CallStateMachineLock,WAIT_FOREVER)
#define UNLOCK_H323_CALL_STATE_MACHINE() semGive(pCCB->H323CallStateMachineLock)





extern CALL_CONTROL_BLOCK	CCBArray[MAX_ACTIVE_CALLS];






/*
 * RunStateMachine : This function is the heart of the entire H.323 Task
 */
void
RunStateMachine(PCALL_CONTROL_BLOCK pCCB,PVOID Context, H323_CALL_EVENT Event)
{
	DBG_FUNC("RunStateMachine",H323_PROTOCOL_LAYER);
	DBG_ENTER();

 	PH323_CALL_STATE_INFO	 CallStateInfo;
	STATUS		Status;
    H323_CALL_STATE CurrentCallState;

    CurrentCallState = pCCB->H323CallState;

	/* Use different state machines for FastStart & non-FastStart calls */
	if (pCCB->FastStart) {
		CallStateInfo = (PH323_CALL_STATE_INFO)&H323_FASTCALL_STATE_TABLE[pCCB->H323CallState][Event];
	}
	else {
		CallStateInfo = (PH323_CALL_STATE_INFO)&H323_CALL_STATE_TABLE[pCCB->H323CallState][Event];
	}


	UpdateStateTransitionHistory(pCCB,pCCB->H323CallState,Event, CallStateInfo->NextState);

	DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : State Transition : Current State %s, Event %s, Next State = %s\n",
                        pCCB->MobileNumber,
						GetH323CallStateName(pCCB->H323CallState),
						GetH323EventName(Event),
						GetH323CallStateName(CallStateInfo->NextState));
											
	if (CallStateInfo->NextState != H323_CALL_STATE_SAME_STATE) 
	{
	    pCCB->PreviousCallState = CurrentCallState;
        pCCB->CurrentEvent      = Event;
		pCCB->H323CallState     = CallStateInfo->NextState;
	}
	Status = (*CallStateInfo->smFunc)(pCCB,Context);
	
	if (Status != STATUS_SUCCESS)
	{
		DBG_ERROR("CCB 0x%x: ActionRoutine Error: Event %s in state %s",
						pCCB,GetH323EventName(Event),
					   	GetH323CallStateName(CurrentCallState));
        DumpStateTransitionHistory(pCCB);
		/* 
		 * FIXME!!!! Got to do something here for all cases.!!
		 */
		/*
		 * Cleanup the CCB in case of cmCallNew error
		 */
        if ((pCCB->CurrentEvent == H323_EVENT_RECV_CC_CALL_SETUP) && (pCCB->H323CallState == H323_CALL_STATE_WAIT_FOR_PROCEEDING)) {
			ReturnCCBToPool(pCCB);
		}

	}



	DBG_LEAVE();
}







/*
 * smCallOffering
 */
STATUS
smCallOffering(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallOffering",H323_LAYER);
	DBG_ENTER();
	BOOL Status;
	int CRV;
    cmAlias CalledPartyAlias,CallingPartyAlias;
    unsigned char callingNumber[256];
    char calledNumber[256];
	PCALL_OFFERING_DETAILS CallOfferingDetails;
	PAPI_MESSAGE	ApiMsg;

	CallOfferingDetails = (PCALL_OFFERING_DETAILS)Context;

    pCCB->CallDirection = H323_CALL_TERMINATED;
    pCCB->ControlConnected = FALSE;


   	/* 
   	 * FIXME:: conversion !!
	 */

    cmCallGetParam(pCCB->LanCallHandle,cmParamCRV,0,
    				&CRV,NULL);

	pCCB->H323CRV = (unsigned short)CRV;

	DBG_TRACE("H.323 Call Reference value %#x\n",pCCB->H323CRV);


	ApiMsg = GetFreeMsgFromPool();
	if (ApiMsg == NULL) {
		DBG_ERROR("All API messages used up from pool!\n");
		DBG_LEAVE();
		return FALSE;
	}
	PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

	pMsg->Magic 			= VOIP_API_MAGIC_VALUE;
	pMsg->LudbId 			= pCCB->LudbId;
	pMsg->VoipMobileHandle 	= ludbGetVoipMobileHandle(pCCB->LudbId);
	pMsg->VoipCallHandle 	= pCCB->Index;
    pMsg->TxnId             = pCCB->LastTxnId = VOIP_API_NULL_TXN_ID;
	pMsg->MessageType 		= VOIP_API_CALL_OFFERING;
	pMsg->H323CRV			= pCCB->H323CRV;   


   	DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Call OFFERING\n",pCCB->MobileNumber);


    memset(calledNumber,0,256);
    memset(callingNumber,0,256);
    CallingPartyAlias.string	= (char*)callingNumber;
	CallingPartyAlias.type      = cmAliasTypeE164;
	CallingPartyAlias.length	= 256;

    CalledPartyAlias.string		= calledNumber;
	CalledPartyAlias.type      	= cmAliasTypeE164;

    if (cmCallGetParam(pCCB->LanCallHandle,cmParamCallingPartyNumber,0,0,(char *)&CallingPartyAlias)>=0)
    {
		/*
		 * Check for the Cisco special hack for presentation/screening
		 * indicators
		 */
		if (callingNumber[0] > 127) {

			strncpy(pCCB->RemotePartyId,(char*)&callingNumber[sizeof(Q931_MSG_IE_CALLING_PARTY_RES_INDICATORS)],
					VOIP_MAX_REMOTE_PARTY_ID_LENGTH);

			DBG_TRACE("Calling party number retrieved as %s\n",pCCB->RemotePartyId);
			PQ931_MSG_IE_CALLING_PARTY_RES_INDICATORS res_ind;
			res_ind = (PQ931_MSG_IE_CALLING_PARTY_RES_INDICATORS)&callingNumber[0];
			pCCB->CallingPartyPI = (Q931_IE_CALLING_PARTY_NO_PI)res_ind->presentation_indicator;
			pCCB->CallingPartySI = (Q931_IE_CALLING_PARTY_NO_SI)res_ind->screening_indicator;
		
		}
		else {
			int fieldId,value;
			BOOL isString;

			strncpy(pCCB->RemotePartyId,(char*)callingNumber,VOIP_MAX_REMOTE_PARTY_ID_LENGTH);
			DBG_TRACE("Calling party number retrieved as %s\n",pCCB->RemotePartyId);

			if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.callingPartyNumber.octet3.presentationIndicator",&fieldId,&value,&isString) < 0) {
				DBG_TRACE("No Presentation/Screening indicator present in calling party number : %s,\n setting to defaults\n",pCCB->RemotePartyId);
				/*
				 * FIXME : Hardcoding PI/SI for now
				 */
			    pCCB->CallingPartyPI	= Q931_IE_CALLING_PARTY_NO_PI_ALLOWED;
			    pCCB->CallingPartySI	= Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED;
			}
			else {
				pCCB->CallingPartyPI = (Q931_IE_CALLING_PARTY_NO_PI)value;
				if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
					"setup.message.*.callingPartyNumber.octet3.screeningIndicator",&fieldId,&value,&isString) < 0) {
					DBG_TRACE("No Presentation/Screening indicator present in calling party number : %s,\n setting to defaults\n",pCCB->RemotePartyId);
					/*
					 * FIXME : Hardcoding PI/SI for now
					 */
			    	pCCB->CallingPartyPI	= Q931_IE_CALLING_PARTY_NO_PI_ALLOWED;
			    	pCCB->CallingPartySI	= Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED;
				}
				else {
					pCCB->CallingPartySI = (Q931_IE_CALLING_PARTY_NO_SI)value;
				}
			}

		}


    }
	else {
		DBG_ERROR("Mobile %s : For incoming call, failed to get Calling Party Number: %s!\n",
			pCCB->MobileNumber,callingNumber);
	}

	if (ludbIsProvCLIP(pCCB->LudbId) && (strlen(pCCB->RemotePartyId) > 0) ) {

	    if (pCCB->CallingPartyPI != Q931_IE_CALLING_PARTY_NO_PI_ALLOWED) {
            DBG_TRACE("Calling Party Presentation restricted, so not sending Clng Party Number\n");
            pMsg->CallOffering.CallingPartyNumber.ie_present = FALSE;
        }
        else {
            pMsg->CallOffering.CallingPartyNumber.ie_present = TRUE;
		    jcStringToBCD(pCCB->RemotePartyId,pMsg->CallOffering.CallingPartyNumber.digits,
			    &pMsg->CallOffering.CallingPartyNumber.numDigits);
        }
    }
    else {
        pMsg->CallOffering.CallingPartyNumber.ie_present = FALSE;
    }

	
	jcStringToBCD(pCCB->MobileNumber,pMsg->CallOffering.CalledPartyNumber.digits,
		&pMsg->CallOffering.CalledPartyNumber.numDigits);

    pMsg->CallOffering.CalledPartyNumber.ie_present 	= TRUE;


	int fieldId,value;
	BOOL isString;

	if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.callingPartyNumber.octet3.numberingPlanIdentification",&fieldId,&value,&isString) >= 0) {
		pCCB->CallingNumberingPlan = value;

	}
	else {
		DBG_WARNING("Could not retrieve calling party numbering plan\n");
		pCCB->CallingNumberingPlan = (int)Q931_IE_CALL_PARTY_NO_PLAN_ISDN;
	}

	if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.callingPartyNumber.octet3.typeOfNumber",&fieldId,&value,&isString) >= 0) {
		pCCB->CallingNumberType = value;

	}
	else {
		DBG_WARNING("Could not retrieved calling party number type\n");
		pCCB->CallingNumberType = (int)Q931_IE_CALL_PARTY_NO_TYPE_INTERNATIONAL;
	}


	if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.calledPartyNumber.octet3.numberingPlanIdentification",&fieldId,&value,&isString) >= 0) {
		pCCB->CalledNumberingPlan = value;

	}
	else {
		DBG_WARNING("Could not retrieve called party numbering plan\n");
		pCCB->CalledNumberingPlan = (int)Q931_IE_CALL_PARTY_NO_PLAN_ISDN;
	}

	if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.calledPartyNumber.octet3.typeOfNumber",&fieldId,&value,&isString) >= 0) {
		pCCB->CalledNumberType = value;

	}
	else {
		DBG_WARNING("Could not retrieved called party number type\n");
		pCCB->CalledNumberType = (int)Q931_IE_CALL_PARTY_NO_TYPE_INTERNATIONAL;
	}

	VoipToGsmNumType((Q931_IE_CALL_PARTY_NO_TYPE)pCCB->CallingNumberType,
			&pMsg->CallOffering.CallingPartyNumber.numberType);
	VoipToGsmNumPlan((Q931_IE_CALL_PARTY_NO_PLAN)pCCB->CallingNumberingPlan,
			&pMsg->CallOffering.CallingPartyNumber.numberingPlan);

	VoipToGsmSI((Q931_IE_CALLING_PARTY_NO_SI)pCCB->CallingPartySI,
			&pMsg->CallSetup.CallingPartyNumber.screeningInd);
	VoipToGsmPI((Q931_IE_CALLING_PARTY_NO_PI)pCCB->CallingPartyPI,
			&pMsg->CallSetup.CallingPartyNumber.presentationInd);


	VoipToGsmNumType((Q931_IE_CALL_PARTY_NO_TYPE)pCCB->CalledNumberType,
			&pMsg->CallOffering.CalledPartyNumber.numberType);
	VoipToGsmNumPlan((Q931_IE_CALL_PARTY_NO_PLAN)pCCB->CalledNumberingPlan,
			&pMsg->CallOffering.CalledPartyNumber.numberingPlan);

	
	DBG_TRACE("---------- For Incoming call to Mobile %s --------\n",pCCB->MobileNumber);
	VoipDisplayCallParties(pCCB);
	DBG_TRACE("----------------------------------------------------\n",pCCB->MobileNumber);



    pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallAttempts++;

	Status=VoipSendMessageToCc(pMsg);
	 
	PutMsgIntoPool(ApiMsg);
    if (Status !=  FALSE) {
        pCCB->CanDropCall = TRUE;
    }
	DBG_LEAVE();
   	return (Status);
}






/*
 * smCallSetup
 */
STATUS
smCallSetup(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallSetup",H323_LAYER);
	DBG_ENTER();
	STATUS Status=STATUS_FAILURE;

	PVOIP_API_MESSAGE pMsg = (PVOIP_API_MESSAGE)Context;
   	char subscriberName[512],CalledPartyNumber[256];
   	unsigned char NumberBuffer[256];
    cmAlias Alias;
    char EmergencyNumber[32];

    cmTransportAddress qAddress={cmTransportTypeIP,0,1720};



    memset(CalledPartyNumber,0,256);
    memset(NumberBuffer,0,256);
   	jcBCDToString((char*)NumberBuffer, pMsg->CallSetup.CalledPartyNumber.digits, 
   			pMsg->CallSetup.CalledPartyNumber.numDigits );
	/*
	 * If this is an international number; add a + sign before the number
	 */
    if (pMsg->CallSetup.CalledPartyNumber.numberType==CNI_RIL3_NUMBER_TYPE_INTERNATIONAL) {
		strcpy(CalledPartyNumber,"+");
		strcat(CalledPartyNumber,(char*)NumberBuffer);
	}
	else {
		strcat(CalledPartyNumber,(char*)NumberBuffer);
	}


    memset(subscriberName,0,512);
    if (!ludbGetMobileName(pCCB->LudbId,subscriberName)) {
        /*
         * DBG_WARNING("Unable to retrieve subscriber name from ludb!!\n");
         */
        strcpy(subscriberName,"Cisco GP-10");
    }

    if (!ludbGetMobileNumber(pCCB->LudbId,pCCB->MobileNumber)) {
        DBG_ERROR("Unable to get mobile no from ludb !!!\n");
        /*strcpy(pCCB->MobileNumber,"911");*/
    }

	DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Call SETUP to %s requested\n",pCCB->MobileNumber,CalledPartyNumber);

    strncpy(pCCB->RemotePartyId,CalledPartyNumber,VOIP_MAX_REMOTE_PARTY_ID_LENGTH);

   	if(cmCallNew( pVoipTaskConfig->StackHandle, (HAPPCALL)pCCB, &pCCB->LanCallHandle) < 0)
   	{
      	DBG_ERROR("Error in cmCallNew()\n");
		pMsg->MessageType 			= VOIP_API_CALL_EVENT;
		pMsg->H323CRV				= pCCB->H323CRV;
		pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
		pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
      	Status=VoipSendMessageToCc(pMsg);
        DBG_LEAVE();
      	return Status;
   	}

    pCCB->ControlConnected = FALSE;

	sseCreateCall(pVoipTaskConfig->SuppStackHandle,&(pCCB->SuppCallHandle),(HSSEAPPCALL)pCCB,
			pCCB->LanCallHandle);
	sseCallImplementTransfer(pCCB->SuppCallHandle,TRUE);
	sseCallImplementForward(pCCB->SuppCallHandle);


    
    pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallAttempts++;

    pCCB->CallDirection = H323_CALL_ORIGINATED;

    /*
     * Setup call-signalling address 
     */
    qAddress.ip=inet_addr(pVoipTaskConfig->szGatekeeperAddress);
   
    cmCallSetParam(pCCB->LanCallHandle,cmParamDestinationIpAddress,0,sizeof(cmTransportAddress),(char*)&qAddress);
   
    cmCallSetParam(pCCB->LanCallHandle,cmParamDestCallSignalAddress,0,sizeof(cmTransportAddress),(char*)&qAddress);

    cmCallSetParam(pCCB->LanCallHandle,cmParamInformationTransferCapability,0,cmITCSpeech, 0);


    /*
     * Setup CALLED PARTY number 
     */
    memset(&Alias,0,sizeof(cmAlias));
    Alias.type      = cmAliasTypeE164;

	if (pMsg->CallSetup.IsEmergencyCall) {
		/*
		 * For emergency call we set the number to a null-terminated string of 30 zeros;
		 * which ViperBase will recognize and map to appropriate local emergency number
		 * This number is theoretically not possible in GSM
		 */
		memset(EmergencyNumber,'0',30);
		EmergencyNumber[30] = 0;
	    Alias.string    = EmergencyNumber; 
	    Alias.length    = strlen(EmergencyNumber);
    	cmCallSetParam( pCCB->LanCallHandle, cmParamCalledPartyNumber,0,0, (char*)&Alias );
	}
	else {
	    Alias.string    = CalledPartyNumber; 
	    Alias.length    = strlen(CalledPartyNumber);
    	cmCallSetParam( pCCB->LanCallHandle, cmParamCalledPartyNumber,0,0, (char*)&Alias );
	}

	GsmToVoipNumType(pMsg->CallSetup.CalledPartyNumber.numberType,
					(Q931_IE_CALL_PARTY_NO_TYPE*)&pCCB->CalledNumberType);
	GsmToVoipNumPlan(pMsg->CallSetup.CalledPartyNumber.numberingPlan,
					(Q931_IE_CALL_PARTY_NO_PLAN*)&pCCB->CalledNumberingPlan);




	/*
	 * Setup CALLING PARTY number
	 */
	memset(NumberBuffer,0,256);

	strcpy((char*)NumberBuffer,pCCB->MobileNumber);

    //CLIR <xxu:07-26-01>
    /*
     * Check for CLIR provisioning
     */
    //if (!ludbIsProvCLIR(pCCB->LudbId)) {
	//	pMsg->CallSetup.CallingPartyNumber.presentationInd = CNI_RIL3_PRESENTATION_ALLOWED;
	//}
	//else {
	//	pMsg->CallSetup.CallingPartyNumber.presentationInd = CNI_RIL3_PRESENTATION_RESTRICTED;
	//}
    if ( (pMsg->CallSetup.clir.ie_present) && (!pMsg->CallSetup.IsEmergencyCall) )
        pMsg->CallSetup.CallingPartyNumber.presentationInd = pMsg->CallSetup.clir.clirSetting;


	/*
	 * These values are not filled by CC/RIL3, so we need to hardcode it for now...
	 */
	pMsg->CallSetup.CallingPartyNumber.screeningInd = CNI_RIL3_SCREENING_NETWORK_PROVIDED;
	pMsg->CallSetup.CallingPartyNumber.numberingPlan = CNI_RIL3_NUMBERING_PLAN_ISDN_TELEPHONY;
	pMsg->CallSetup.CallingPartyNumber.numberType = CNI_RIL3_NUMBER_TYPE_INTERNATIONAL;

	GsmToVoipNumType(pMsg->CallSetup.CallingPartyNumber.numberType,
			(Q931_IE_CALL_PARTY_NO_TYPE*)&pCCB->CallingNumberType);
	GsmToVoipNumPlan(pMsg->CallSetup.CallingPartyNumber.numberingPlan,
			(Q931_IE_CALL_PARTY_NO_PLAN*)&pCCB->CallingNumberingPlan);

	GsmToVoipPI(pMsg->CallSetup.CallingPartyNumber.presentationInd,
					(Q931_IE_CALLING_PARTY_NO_PI*)&pCCB->CallingPartyPI);
	GsmToVoipSI(pMsg->CallSetup.CallingPartyNumber.screeningInd,
					(Q931_IE_CALLING_PARTY_NO_SI*)&pCCB->CallingPartySI);

	

    memset(&Alias,0,sizeof(cmAlias));
    Alias.type      = cmAliasTypeE164;
    Alias.string    = (char*)NumberBuffer; 
    Alias.length    = strlen((char*)NumberBuffer);

    cmCallSetParam( pCCB->LanCallHandle, cmParamCallingPartyNumber,1,0, (char*)&Alias );


    int count=0;
    for(count=0;count<3;count++) {
        if (pMsg->CallSetup.bearerCap.speechVersionInd[count].fieldPresent) {
            if (pMsg->CallSetup.bearerCap.speechVersionInd[count].version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1) {
                pCCB->GSMFullRateCapability=TRUE;
                DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : supports GSM FR codec \n",pCCB->MobileNumber);
            }
            if (pMsg->CallSetup.bearerCap.speechVersionInd[count].version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2) {
                pCCB->GSMEnhancedFullRateCapability=TRUE;
                DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : supports GSM EFR codec \n",pCCB->MobileNumber);
            }
        }

    }

	if ((pCCB->GSMFullRateCapability==FALSE) && (pCCB->GSMEnhancedFullRateCapability==FALSE)) {
      	DBG_ERROR("Error : No GSM Codec capabilities specified by Mobile %s while trying to make call\n",
      			pCCB->MobileNumber);
		pMsg->MessageType 			= VOIP_API_CALL_EVENT;
		pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
		pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
		pMsg->H323CRV				= pCCB->H323CRV;
      	Status=VoipSendMessageToCc(pMsg);
      	return Status;
	}

	DBG_TRACE("---------- For Outgoing call from Mobile %s --------\n",pCCB->MobileNumber);
	VoipDisplayCallParties(pCCB);
	DBG_TRACE("----------------------------------------------------\n",pCCB->MobileNumber);


    pCCB->FastStart = TRUE; /* Always try FastStart first*/

   	if(cmCallDial(pCCB->LanCallHandle) < 0 )
   	{
      	DBG_ERROR("Error in cmCallMake()\n" );
		pMsg->MessageType 			= VOIP_API_CALL_EVENT;
		pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
		pMsg->CallEvent.ReasonCode 	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
		pMsg->H323CRV				= pCCB->H323CRV;
      	Status=VoipSendMessageToCc(pMsg);
      	return Status;
   	}
   	else
   	{
		Status = STATUS_SUCCESS;
   	}	


    pCCB->CanDropCall = TRUE;

    /* DumpCCBAndRtp(); */
	DBG_LEAVE();
	return Status;
}



/*
 * smSetupFastChannels
 */
STATUS
smSetupFastChannels(PCALL_CONTROL_BLOCK pCCB)
{
    DBG_FUNC("smSetupFastChannels",H323_LAYER);
	DBG_ENTER();
    int FastStartStatus=0;
	STATUS Status=STATUS_FAILURE;
    cmFastStartMessage fsMessage, *t;

    
    fsMessage.partnerChannelsNum=1;


    fsMessage.partnerChannels[0].transmit.channels[0].rtp.port	= jcRtpGetPort(pCCB->RtpHandle);
    fsMessage.partnerChannels[0].transmit.channels[0].rtp.ip	= 0;
    fsMessage.partnerChannels[0].transmit.channels[0].rtcp.port	= jcRtpGetPort(pCCB->RtpHandle) + 1;
    fsMessage.partnerChannels[0].transmit.channels[0].rtcp.ip 	= 0;
    fsMessage.partnerChannels[0].transmit.channels[0].dataTypeHandle = -1;


    fsMessage.partnerChannels[0].receive.channels[0].rtp.port	= jcRtpGetPort(pCCB->RtpHandle);
    fsMessage.partnerChannels[0].receive.channels[0].rtp.ip 	= 0;
    fsMessage.partnerChannels[0].receive.channels[0].rtcp.port	= jcRtpGetPort(pCCB->RtpHandle)+1;
    fsMessage.partnerChannels[0].receive.channels[0].rtcp.ip 	= 0;
    fsMessage.partnerChannels[0].receive.channels[0].dataTypeHandle = -1;



	/*
	 * There are three possibilities here, MS supports :-
	 *  * FR only
	 *  * EFR only, or
	 *  * both FR & EFR [with neither FR nor EFR case being filtered out during smCallSetup]
	 */


	if ( (pCCB->GSMFullRateCapability == TRUE) && (pCCB->GSMEnhancedFullRateCapability == FALSE)) {
		/* Supports FR only */
    	fsMessage.partnerChannels[0].receive.channels[0].channelName  	= "gsmFullRate";
    	fsMessage.partnerChannels[0].transmit.channels[0].channelName 	= "gsmFullRate";
    	fsMessage.partnerChannels[0].transmit.altChannelNumber			= 1;
	    fsMessage.partnerChannels[0].receive.altChannelNumber			= 1;
	}
	else if ( (pCCB->GSMFullRateCapability == FALSE) && (pCCB->GSMEnhancedFullRateCapability == TRUE)) {
		/* Supports EFR only */
    	fsMessage.partnerChannels[0].receive.channels[0].channelName  	= "gsmEnhancedFullRate";
    	fsMessage.partnerChannels[0].transmit.channels[0].channelName 	= "gsmEnhancedFullRate";
    	fsMessage.partnerChannels[0].transmit.altChannelNumber			= 1;
	    fsMessage.partnerChannels[0].receive.altChannelNumber			= 1;
	}
	else if ( (pCCB->GSMFullRateCapability == TRUE) && (pCCB->GSMEnhancedFullRateCapability == TRUE)) {
		/* Supports BOTH FR & EFR  */
    	fsMessage.partnerChannels[0].receive.channels[0].channelName  	= "gsmFullRate";
    	fsMessage.partnerChannels[0].transmit.channels[0].channelName 	= "gsmFullRate";

    	fsMessage.partnerChannels[0].transmit.altChannelNumber=2;

	    fsMessage.partnerChannels[0].transmit.channels[1].rtp.port		= jcRtpGetPort(pCCB->RtpHandle);
	    fsMessage.partnerChannels[0].transmit.channels[1].rtp.ip 		= 0;
	    fsMessage.partnerChannels[0].transmit.channels[1].rtcp.port		= jcRtpGetPort(pCCB->RtpHandle)+1;
	    fsMessage.partnerChannels[0].transmit.channels[1].rtcp.ip 		= 0;
	    fsMessage.partnerChannels[0].transmit.channels[1].dataTypeHandle= -1;
	    fsMessage.partnerChannels[0].transmit.channels[1].channelName   = "gsmEnhancedFullRate";


    	fsMessage.partnerChannels[0].receive.altChannelNumber=2;

	    fsMessage.partnerChannels[0].receive.channels[1].rtp.port		= jcRtpGetPort(pCCB->RtpHandle);
	    fsMessage.partnerChannels[0].receive.channels[1].rtp.ip 		= 0;
	    fsMessage.partnerChannels[0].receive.channels[1].rtcp.port		= jcRtpGetPort(pCCB->RtpHandle)+1;
	    fsMessage.partnerChannels[0].receive.channels[1].rtcp.ip 		= 0;
	    fsMessage.partnerChannels[0].receive.channels[1].dataTypeHandle = -1;
	    fsMessage.partnerChannels[0].receive.channels[1].channelName 	= "gsmEnhancedFullRate";

	}

    fsMessage.partnerChannels[0].type = cmCapAudio;

    t = &fsMessage;


    if ((FastStartStatus=cmFastStartOpenChannels(pCCB->LanCallHandle,&fsMessage)) < 0) {
        DBG_ERROR("Error opening fast start channels; status %d, call handle %x!!!!\n",
        			FastStartStatus,pCCB);
    }
    else {
    	DBG_TRACE("FastStart channels opened successfully\n");
		if (pCCB->IsSuppCall) {
			cmAlias Alias;
			memset(&Alias,0,sizeof(Alias));
			Alias.type		= cmAliasTypeE164;
			/* cc note, use the original party, instead of the transferring party */
			Alias.string 	= (char *)pCCB->MobileNumber;
			Alias.length 	= strlen(pCCB->MobileNumber);
			/* 
			Alias.string 	= (char *)pCCB->TransferringPartyId;
			Alias.length 	= strlen(pCCB->TransferringPartyId);
			*/ 
			cmCallSetParam(pCCB->LanCallHandle,cmParamCallingPartyNumber,1,0,(char*)&Alias);

   			char CalledPartyNumber[256];

			memset(CalledPartyNumber,0,256);
   			Alias.string = CalledPartyNumber; 

		   	if( cmCallGetParam(pCCB->LanCallHandle, cmParamCalledPartyNumber, 0, NULL, (char*)&Alias ) < 0 ||
		       Alias.length <= 0 ) {
			   	if( cmCallGetParam(pCCB->LanCallHandle, cmParamDestinationAddress, 0, NULL, (char*)&Alias ) < 0 ||
			       Alias.length <= 0 ) {
					DBG_WARNING("Could not copy DestinationAddress to CalledParty Number for transferred call\n");
				}
				else {
					cmCallSetParam(pCCB->LanCallHandle,cmParamCalledPartyNumber,1,0,(char*)&Alias);
					DBG_TRACE("Copied DestinationAddress to CalledPartyNumber %s\n",CalledPartyNumber);
					/* cc note , update the remote party number*/
					{ /* remote party should be the called party number*/
					/*
					DBG_TRACE("Call transfer: local is %s, remote is \n",
						pCCB->MobileNumber,
						pCCB->RemotePartyId);
					*/
					strcpy(pCCB->RemotePartyId,CalledPartyNumber);
					}
				}

			}
			
		}
		else {
			DBG_TRACE("Regular setup\n");
		}

		/*
		 * Set the calling party number types/plan and presentation/screening indicators here
		 * this is because even if it a transferred call it will execute the code below
		 */
		if (pvtBuildByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.callingPartyNumber.octet3.presentationIndicator",pCCB->CallingPartyPI,NULL) < 0)
		{
			DBG_WARNING("Could not set calling party # presentation indicator\n");
		}

		if (pvtBuildByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"setup.message.*.callingPartyNumber.octet3.screeningIndicator",
				pCCB->CallingPartySI,NULL) < 0)
		{
			DBG_WARNING("Could not set calling party # screening indicator\n");
		}
        else {
            DBG_TRACE("Set the Calling Party Screening Indicator to %d value\n",pCCB->CallingPartySI);
        }


		if (pvtBuildByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
              "setup.message.*.callingPartyNumber.octet3.typeOfNumber",pCCB->CallingNumberType,NULL) < 0)
		{
			DBG_WARNING("Could not set calling party # number type \n");
		}

		if (pvtBuildByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
              "setup.message.*.callingPartyNumber.octet3.numberingPlanIdentification",pCCB->CallingNumberingPlan,NULL) < 0)
		{
			DBG_WARNING("Could not set calling party # numbering plan\n");
		}

		if (pvtBuildByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
              "setup.message.*.calledPartyNumber.octet3.typeOfNumber",pCCB->CalledNumberType,NULL) < 0)
		{
			DBG_WARNING("Could not set calling party # number type \n");
		}

		if (pvtBuildByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
              "setup.message.*.calledPartyNumber.octet3.numberingPlanIdentification",pCCB->CalledNumberingPlan,NULL) < 0)
		{
			DBG_WARNING("Could not set calling party # numbering plan\n");
		}


		Status = STATUS_SUCCESS;
	}

	pCCB->FastStart = FALSE;

    DBG_LEAVE();
    return (Status);

}




/*
 * smDropCall
 */
STATUS
smDropCall(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smDropCall",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;
	PVOIP_API_MESSAGE pMsg = (PVOIP_API_MESSAGE)Context;
	int Q850CauseCode;


    pCCB->CanDropCall = FALSE;

    pCCB->LocalRelease = TRUE;

    if(pMsg->CallRelease.cause >= Q850_MAX_CAUSES) {
		DBG_ERROR("Invalid disconnect cause %d specified!!\n",pMsg->CallRelease.cause);
		Q850CauseCode = Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
	}
	else {
		Q850CauseCode = pMsg->CallRelease.cause;
	}

	pVoipTaskConfig->CallStats.CallDisconnectStats.MobileDisconnects[Q850CauseCode]++;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Dropping call, cause %d : %s\n",
            pCCB->MobileNumber,Q850CauseCode,GetQ850CauseCodeName(Q850CauseCode));

	cmCallSetParam(pCCB->LanCallHandle,cmParamReleaseCompleteCause,0,Q850CauseCode,NULL);

    cmCallDrop(pCCB->LanCallHandle);


	if (pCCB->InChannelHandle != NULL)
		cmChannelDrop(pCCB->InChannelHandle);

	if (pCCB->OutChannelHandle != NULL)
		cmChannelDrop(pCCB->OutChannelHandle);

	DBG_LEAVE();
	return Status;
}


/*
 * smAnswerCall
 */
STATUS
smAnswerCall(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smAnswerCall",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

    char Display[256],CalledParty[256];

    memset(CalledParty,0,256);
    memset(Display,0,256);

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Answering incoming call\n",pCCB->MobileNumber);

    strcpy(Display,"ViperCell");
    sprintf(CalledParty,"NAME:%s,TEL:%s,",pCCB->MobileNumber,"ViperCell");

    cmCallAnswerExt(pCCB->LanCallHandle ,Display,CalledParty,strlen(CalledParty));

	DBG_LEAVE();
	return Status;
}

/*
 * smCallConnected
 */
STATUS
smCallConnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallConnected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;
    char id[255];

	/*
     * Cleanup the RTP counters
	 */
	RtpResetSessionCounters(pCCB->RtpHandle);

	memset(id,0,255);
    cmCallGetDisplayInfo(pCCB->LanCallHandle,id,255);
	strcpy(pCCB->DisplayInfo,id);

    pCCB->LocalRelease = FALSE;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Q.931 connected on call\n",pCCB->MobileNumber);

    if ( (pCCB->CallDirection == H323_CALL_ORIGINATED) && (!pCCB->FastStart) ){

		/*
		 * In this case we will assume for now that the remote endpoint supports
		 * only the codec that our endpoint supports and carry on, and in case
		 * we find out through H.245 that the remote does not, we will drop the 
		 * call : DineshN
		 */
		 if (pCCB->GSMEnhancedFullRateCapability)
		 {
			if (pVoipTaskConfig->DefaultCodecGsmFullRate) {
				DBG_TRACE("Remote is Non-FastStart endpoint, defaulting to FullRate codec\n");
				pCCB->RemoteGSMFullRateCapability=TRUE;
				pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
			}
			else {
				DBG_TRACE("Remote is Non-FastStart endpoint, defaulting to EnhancedFullRate codec\n");
				pCCB->RemoteGSMEnhancedFullRateCapability=TRUE;
				pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_EFR;
			}
		 }
		 else if (pCCB->GSMFullRateCapability) {
			pCCB->RemoteGSMFullRateCapability=TRUE;
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
		 }
		 else {
#ifdef VOIP_DEVELOP_DEBUG
			ASSERT(0);
#endif
			DBG_ERROR("Local Mobile %s reported invalid codecs\n",pCCB->MobileNumber);
			return STATUS_FAILURE;
		 }

		SendCodecInfo(pCCB);
	}

	pCCB->Q931Connected = TRUE;

    if ((pCCB->CallDirection == H323_CALL_ORIGINATED) && (pCCB->OutChannelConnected)){
		PAPI_MESSAGE	ApiMsg;
		ApiMsg = GetFreeMsgFromPool();
		if (ApiMsg == NULL) {
			DBG_ERROR("All API messages used up from pool!\n");
			DBG_LEAVE();
			return FALSE;
		}
		PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

	    pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
		pMsg->H323CRV				= pCCB->H323CRV;
	    pMsg->LudbId 				= pCCB->LudbId;
	    pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	    pMsg->VoipCallHandle 		= pCCB->Index;
	    pMsg->TxnId					= pCCB->LastTxnId;
	    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_CONNECTED;
		DBG_TRACE("Mobile %s: Sending Call Connected to CC\n",pCCB->MobileNumber);
		if (!pCCB->IsSuppCall)
	    Status=VoipSendMessageToCc(pMsg);
		PutMsgIntoPool(ApiMsg);

		if (pCCB->FastStart) {
			/* Lets try out this hack for now */
            if (!pCCB->ControlConnected) {
			    cmCallConnectControl(pCCB->LanCallHandle);
                pCCB->ControlConnected = TRUE;
            }
		}

    }

	DBG_LEAVE();
	return Status;
}



/*
 * smCallDisconnected
 */
STATUS
smCallDisconnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallDisconnected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;



	if (pCCB->CallTransferred)	{
		DBG_LEAVE();
		return (Status);
	}

	PAPI_MESSAGE	ApiMsg;
	ApiMsg = GetFreeMsgFromPool();
	if (ApiMsg == NULL) {
		DBG_ERROR("All API messages used up from pool!\n");
		DBG_LEAVE();
		return FALSE;
	}
	PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

    pCCB->CanDropCall = FALSE;
	
	/*
	 * Depending upon the state we are in, we
	 * may need to perform additional cleanup functions
	 */
	if(pCCB->H323CallState==H323_CALL_STATE_WAIT_FOR_CC_ANSWER)
	{
        DBG_TRACE("Call offering followed by disconnect, sending null txn id\n");
	    pCCB->LastTxnId = VOIP_API_NULL_TXN_ID;
	} 
    else {
        DBG_TRACE("Regular disconnect\n");
    }

    unsigned char *DisconnectReason = (unsigned char *)Context;


    /*
     * If this is a transfer attempt which failed,
     * we need to inform CC to swap back to the orignal
     * VOIP call handle
     */
    if (pCCB->IsSuppCall)  {
        PCALL_CONTROL_BLOCK pOrigCCB = &CCBArray[pCCB->OldIndex];
        /*
         * If the original call-leg is still up, we swap the handles
         */
        if ((pOrigCCB->CallTransferred) && (pOrigCCB->H323CallState == H323_CALL_STATE_CHANNELS_CONNECTED)) {

	        pMsg->Magic 				        = VOIP_API_MAGIC_VALUE;
	        pMsg->H323CRV				        = pCCB->H323CRV;
	        pMsg->LudbId 				        = pCCB->LudbId;
	        pMsg->VoipMobileHandle 		        = ludbGetVoipMobileHandle(pCCB->LudbId);
	        pMsg->VoipCallHandle 		        = pCCB->Index;
	        pMsg->TxnId					        = pCCB->LastTxnId;
	        pMsg->MessageType 			        = VOIP_API_CALL_UPDATE_HANDLES;
   	        pMsg->CallUpdateHandles.NewHandle 	= pCCB->OldIndex;
            pMsg->CallUpdateHandles.ReconnectOnTransferFailure = TRUE;
	        Status=VoipSendMessageToCc(pMsg);
	        PutMsgIntoPool(ApiMsg);
	        pCCB->Q931Connected = FALSE;
            DBG_TRACE("Mobile %s : Transfer attempt callleg disconnected, reverting to old Callleg\n",pCCB->MobileNumber);
            pCCB->IsSuppCall = FALSE;
            pOrigCCB->CallTransferred = FALSE;
            DBG_LEAVE();
            return(Status);
        }

    }

    pMsg->TxnId                 = pCCB->LastTxnId;
	pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	pMsg->H323CRV				= pCCB->H323CRV;
	pMsg->LudbId 				= pCCB->LudbId;
	pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	pMsg->VoipCallHandle 		= pCCB->Index;
	pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
	pMsg->CallEvent.ReasonCode 	= (unsigned char)*DisconnectReason;

    /*
     * CC does NOT need this message if it is locally initiated drop
     */
    if (pCCB->LocalRelease != TRUE) {
	    Status=VoipSendMessageToCc(pMsg);
    }
	 
	PutMsgIntoPool(ApiMsg);

	pCCB->Q931Connected = FALSE;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Call disconnected\n",pCCB->MobileNumber);

	DBG_LEAVE();
	return(Status);
}




/*
 * smCallProceeding
 */
STATUS
smCallProceeding(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallProceeding",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;
	int CRV;

    cmCallGetParam(pCCB->LanCallHandle,cmParamCRV,0,
    				&CRV,NULL);

	pCCB->H323CRV = (unsigned short)CRV;
	DBG_CALL_TRACE((unsigned long)pCCB,"H.32\3 Call Reference value %#x\n",pCCB->H323CRV);
	DBG_TRACE("H.323 Call Reference value %#x\n",pCCB->H323CRV);

	PAPI_MESSAGE	ApiMsg;
	ApiMsg = GetFreeMsgFromPool();
	if (ApiMsg == NULL) {
		DBG_ERROR("All API messages used up from pool!\n");
		DBG_LEAVE();
		return FALSE;
	}
	PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

	pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	pMsg->H323CRV				= pCCB->H323CRV;
	pMsg->LudbId 				= pCCB->LudbId;
	pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	pMsg->VoipCallHandle 		= pCCB->Index;
	pMsg->TxnId					= pCCB->LastTxnId;
	pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_PROCEEDING;
	if (!pCCB->IsSuppCall)
	Status=VoipSendMessageToCc(pMsg);
	 
	PutMsgIntoPool(ApiMsg);

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : <-- Q.931 PROCEEDING received\n",pCCB->MobileNumber);

	DBG_LEAVE();
	return Status;
}






/*
 * smCallRingBack
 */
STATUS
smCallRingBack(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallRingBack",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;
	int fieldId,value;
	BOOL isString;
	PAPI_MESSAGE	ApiMsg;


	ApiMsg = GetFreeMsgFromPool();
	if (ApiMsg == NULL) {
		DBG_ERROR("All API messages used up from pool!\n");
		DBG_LEAVE();
		return FALSE;
	}
	PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

    pCCB->InBandInfo = FALSE;

    /*
     * Check the alerting msg to see if progress IE is present along with description;
     * and if the description indicates inband-information; we gotta be ready...
     */
	if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				"alerting.message.*.progressIndicator.octet4.ext",&fieldId,&value,&isString) < 0)
	{
		DBG_TRACE("Could not get Progress Indicator in Alerting msg : octet4.ext bit is not set\n");
	}
    else {
	    /* octet4.progressDescription */
	    if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				    "alerting.message.*.progressIndicator.octet4.progressDescription",&fieldId,&value,&isString) < 0)
	    {
		    DBG_WARNING("Could not get Progress Indicator description in Alerting msg\n");
	    }
        else {
            if (value = Q931_IE_PROGRESS_DESC_INBAND_INFO) {
                DBG_TRACE("Received in-band information indication via Progress IE\n");
                pCCB->InBandInfo = TRUE;
            }
        }
    }

    if (pCCB->InBandInfo) {
        pMsg->CallEvent.EventInfo.AlertingProgressIE.ie_present         = TRUE;
        pMsg->CallEvent.EventInfo.AlertingProgressIE.codingStandard     = CNI_RIL3_GSMPLMN_CODING;
        pMsg->CallEvent.EventInfo.AlertingProgressIE.location           = CNI_RIL3_LOCATION_PUBLIC_NETWORK_LOCAL_USER;
        pMsg->CallEvent.EventInfo.AlertingProgressIE.progressDesp       = CNI_RIL3_PROGRESS_IN_BAND_INFO_AVAILABLE;
    }
    else {
        pMsg->CallEvent.EventInfo.AlertingProgressIE.ie_present = FALSE;
    }

	pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	pMsg->H323CRV				= pCCB->H323CRV;
	pMsg->LudbId 				= pCCB->LudbId;
	pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	pMsg->VoipCallHandle 		= pCCB->Index;
	pMsg->TxnId					= pCCB->LastTxnId;
	pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_ALERTING;
	Status=VoipSendMessageToCc(pMsg);
	 
	PutMsgIntoPool(ApiMsg);

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : <-- Q.931 ALERTING received\n",pCCB->MobileNumber);

	DBG_LEAVE();
	return Status;
}




/*
 * smDestinationRejected
 */
STATUS
smCallDestinationRejected(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallDestinationRejected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

	PAPI_MESSAGE	ApiMsg;
	ApiMsg = GetFreeMsgFromPool();
	if (ApiMsg == NULL) {
		DBG_ERROR("All API messages used up from pool!\n");
		DBG_LEAVE();
		return FALSE;
	}
	PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

	pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	pMsg->H323CRV				= pCCB->H323CRV;
	pMsg->LudbId 				= pCCB->LudbId;
	pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	pMsg->VoipCallHandle 		= pCCB->Index;
	pMsg->TxnId					= pCCB->LastTxnId;
	pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_DISCONNECTED;
	pMsg->CallEvent.ReasonCode	= Q850_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
	Status=VoipSendMessageToCc(pMsg);
	 
	PutMsgIntoPool(ApiMsg);

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Call Destination REJECTED\n",pCCB->MobileNumber);

	DBG_LEAVE();
	return Status;
}


/*
 * smCallIdle
 */
STATUS
smCallIdle(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smCallIdle",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

	/* FIXME:: should we be doing this ??!! */
	if (pCCB->LanCallHandle) 
	{
		cmCallClose(pCCB->LanCallHandle);
		sseCallClose(pCCB->SuppCallHandle);
		ReturnCCBToPool(pCCB);
	}
	else 
	{
		
	}
	DBG_LEAVE();
	return Status;
}



/*
 * smRecvRemoteCapabilities
 */
STATUS
smRecvRemoteCapabilities(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smRecvRemoteCapabilities",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_FAILURE;
	PREMOTE_CAPABILITIES_DETAILS RemoteCapabilities= (PREMOTE_CAPABILITIES_DETAILS)Context;


	if (pCCB->FastStart) {
		if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_EFR) 
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_EFR_PAYLOAD_TYPE);
		else 
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_PAYLOAD_TYPE);
		Status = STATUS_SUCCESS;
		DBG_LEAVE();
		return Status;
	}

	/* First check whether remote endpoint has any GSM capability */
	if (RemoteCapabilities->GSMCodecCapability) {
		if ( (RemoteCapabilities->GSMEnhancedFullRateCapability) && 
			 (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_EFR) ) {
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_EFR_PAYLOAD_TYPE);
			Status = STATUS_SUCCESS;
		}
		else if ((RemoteCapabilities->GSMFullRateCapability) && 
				 (pCCB->CodecUsed==VOIP_CALL_CODEC_GSM_FR)) {
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_PAYLOAD_TYPE);
			Status = STATUS_SUCCESS;
		}
		else {
			DBG_ERROR("Capabilities mismatch\n");
		}
	}
	else {
			DBG_ERROR("Capabilities mismatch\n");
	}


	DBG_LEAVE();
	return (Status);
}



/*
 * smH245TransportConnected
 */
STATUS
smH245TransportConnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smH245TransportConnected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : <-- H.245 Transport connected\n",pCCB->MobileNumber);

    SendLocalTermCapSet(pCCB);

	DBG_LEAVE();
	return Status;
}


/*
 * smH245Connected
 */
STATUS
smH245Connected(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smH245Connected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_FAILURE;
   	/* possible to open logical channel now */

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : <-- H.245 connected\n",pCCB->MobileNumber);

	if (pCCB->FastStart) {
		/* In this case we dont really need to open channels
		   since they would already be open */
		Status = STATUS_SUCCESS;
		DBG_LEAVE();
		return Status;
	}


   	cmChannelNew(pCCB->LanCallHandle, (HAPPCHAN)pCCB, &pCCB->OutChannelHandle);

   	DBG_TRACE("Opening outgoing channel %x ; CCB 0x%x\n", pCCB->OutChannelHandle, pCCB);
 
  	cmChannelSetRTCPAddress( pCCB->OutChannelHandle, 0, jcRtpGetPort( pCCB->RtpHandle ) + 1 );
	
   	cmChannelSetDynamicRTPPayloadType( pCCB->OutChannelHandle, 96 );

   	if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_FR) 
   	{
		if( cmChannelOpen( pCCB->OutChannelHandle, "gsmFullRate", NULL, NULL, 0 ) < 0 )
		{
	    	DBG_ERROR("Error in cmChannelOpen()\n" );
	    }
	    else 
	    {
			Status = STATUS_SUCCESS;
		}
    }
    else if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_EFR) 
    {
    	if( cmChannelOpen( pCCB->OutChannelHandle, "gsmEnhancedFullRate", NULL, NULL, 0 ) < 0 )
       	{
     		DBG_ERROR("Error in cmChannelOpen()\n" );
	    }
	    else 
	    {
			Status = STATUS_SUCCESS;
        }
	}

	DBG_LEAVE();
	return Status;
}


/*
 * smRecvCapabilitiesAck
 */
STATUS
smRecvCapabilitiesAck(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smRecvCapabilitiesAck",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;
	DBG_TRACE("Capabilities accepted by remote terminal\n");

	DBG_LEAVE();
	return Status;
}


/*
 * smRecvCapabilitiesNak
 */
STATUS
smRecvCapabilitiesNak(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smRecvCapabilitiesNak",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

	DBG_ERROR("Capabilities REJECTED by remote terminal; dropping call\n");
    if (pCCB->CanDropCall) {
      cmCallDropParam(pCCB->LanCallHandle, cmReasonTypeDestinationRejection);
    }

	DBG_LEAVE();
	return Status;
}


/*
 * smChannelOffering
 */
STATUS
smChannelOffering(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smChannelOffering",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;


    cmChannelSetAddress(pCCB->InChannelHandle, 0, jcRtpGetPort( pCCB->RtpHandle ) );
	
    cmChannelSetRTCPAddress( pCCB->InChannelHandle, 0, jcRtpGetPort( pCCB->RtpHandle ) + 1 );
	
    cmChannelAnswer(pCCB->InChannelHandle);
	
	DBG_LEAVE();
	return Status;

}

/*
 * smInChannelDisconnected
 */
STATUS
smInChannelDisconnected(PCALL_CONTROL_BLOCK pCCB,PVOID Context)
{
	DBG_FUNC("smInChannelDisconnected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

    cmChannelClose(pCCB->InChannelHandle);

	pCCB->InChannelHandle = NULL;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : IN Channel 0x%x DISCONNECTED\n",pCCB->MobileNumber,pCCB->InChannelHandle);

	DBG_LEAVE();
	return Status;


}


/*
 * smInChannelConnected
 */
STATUS
smInChannelConnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context)
{
	DBG_FUNC("smInChannelConnected",H323_LAYER);
	DBG_ENTER();
	STATUS  Status = STATUS_SUCCESS;


    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : IN Channel 0x%x CONNECTED\n",pCCB->MobileNumber,pCCB->InChannelHandle);
		
    /*
     * If this is a fast-start call, and we received early indication
     * of media via progress ie with description of in-band info,
     * we should be ready to accept a media stream now
     *
     */
	if ((pCCB->FastStart) && (pCCB->InBandInfo)) {
		if (pCCB->RemoteGSMEnhancedFullRateCapability && pCCB->GSMEnhancedFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_EFR;
		    SendCodecInfo(pCCB);    
		}
		else if (pCCB->RemoteGSMFullRateCapability && pCCB->GSMFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
		    SendCodecInfo(pCCB);    
		}

	    PAPI_MESSAGE	ApiMsg;
	    ApiMsg = GetFreeMsgFromPool();
	    if (ApiMsg == NULL) {
		    DBG_ERROR("All API messages used up from pool!\n");
		    DBG_LEAVE();
		    return FALSE;
	    }
	    PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

	    pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	    pMsg->H323CRV				= pCCB->H323CRV;
	    pMsg->LudbId 				= pCCB->LudbId;
	    pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	    pMsg->VoipCallHandle 		= pCCB->Index;
	    pMsg->TxnId					= pCCB->LastTxnId;
	    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_VOICE_CONNECTED;
	    DBG_TRACE("Mobile %s: Sending Voice Connected to CC\n",pCCB->MobileNumber);
	    Status=VoipSendMessageToCc(pMsg);

        DBG_TRACE("Inchannel connected, setting up voice path early 'cos in-band information indicated\n");
        PutMsgIntoPool(ApiMsg);

	}
    else {
	    DBG_TRACE("In channel connected\n");
    }
	DBG_LEAVE();
	return Status;
}


/*
 * smOutChannelConnected
 */
STATUS
smOutChannelConnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context)
{
	DBG_FUNC("smOutChannelConnected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

	DBG_TRACE("Out channel connected\n");
    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : OUT Channel 0x%x CONNECTED\n",pCCB->MobileNumber,pCCB->OutChannelHandle);

    /* Increment call counters only if this is not a supplementary call */
	if (!pCCB->IsSuppCall) {
	    if (pCCB->CallDirection == H323_CALL_ORIGINATED) {
	        pVoipTaskConfig->CallStats.CallCompletionStats.OutgoingCallsConnected++;
	    }
	    else {
	        pVoipTaskConfig->CallStats.CallCompletionStats.IncomingCallsConnected++;
	    }
	}


    /*
     * If the call is fast-start and there was no indication
     * of in-band information via progress IE in alerting,
     * we fix the preferred-negotiated codec first.
     * otherwise in case of in-band info we already have 
     * setup the voice-patch in the in-channel connection above
     */
	if ((pCCB->FastStart) && (!pCCB->InBandInfo)) {
		if (pCCB->RemoteGSMEnhancedFullRateCapability && pCCB->GSMEnhancedFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_EFR;
		    SendCodecInfo(pCCB);    
		}
		else if (pCCB->RemoteGSMFullRateCapability && pCCB->GSMFullRateCapability) {
			pCCB->CodecUsed = VOIP_CALL_CODEC_GSM_FR;
		    SendCodecInfo(pCCB);    
		}
		if (pCCB->CallDirection == H323_CALL_ORIGINATED) {
            if (!pCCB->ControlConnected) {
			    cmCallConnectControl(pCCB->LanCallHandle);
                pCCB->ControlConnected = TRUE;
            }
		}
	}

	PAPI_MESSAGE	ApiMsg;
	ApiMsg = GetFreeMsgFromPool();
	if (ApiMsg == NULL) {
	    DBG_ERROR("All API messages used up from pool!\n");
	    DBG_LEAVE();
	    return FALSE;
	}
	PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;


    /*
     * again, no need to indicate voice-path setup if the
     * in-band-info indicator has already caused us
     * to do so earlier...
     */
    if (!pCCB->InBandInfo) {

	    pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	    pMsg->H323CRV				= pCCB->H323CRV;
	    pMsg->LudbId 				= pCCB->LudbId;
	    pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	    pMsg->VoipCallHandle 		= pCCB->Index;
	    pMsg->TxnId					= pCCB->LastTxnId;
	    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_VOICE_CONNECTED;
	    DBG_TRACE("Mobile %s: Sending Voice Connected to CC\n",pCCB->MobileNumber);
	    Status=VoipSendMessageToCc(pMsg);

	    pCCB->OutChannelConnected = TRUE;
    }

    if (pCCB->IsSuppCall) {
        PutMsgIntoPool(ApiMsg);
		DBG_LEAVE();
		return (Status);
    }

    /*
     * CC needs this event only if we are originating the call
     */
    if ((pCCB->CallDirection == H323_CALL_ORIGINATED) && (pCCB->Q931Connected)){
	    pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
		pMsg->H323CRV				= pCCB->H323CRV;
	    pMsg->LudbId 				= pCCB->LudbId;
	    pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	    pMsg->VoipCallHandle 		= pCCB->Index;
	    pMsg->TxnId					= pCCB->LastTxnId;
	    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_CONNECTED;
		DBG_TRACE("Mobile %s: Sending Call Connected to CC\n",pCCB->MobileNumber);
	    Status=VoipSendMessageToCc(pMsg);
    }
	PutMsgIntoPool(ApiMsg);



	DBG_LEAVE();
	return Status;
}


/*
 * smOutChannelDisconnected
 */
STATUS
smOutChannelDisconnected(PCALL_CONTROL_BLOCK pCCB,PVOID  Context)
{
	DBG_FUNC("smOutChannelDisconnected",H323_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : OUT Channel 0x%x DISCONNECTED\n",pCCB->MobileNumber,pCCB->OutChannelHandle);

	DBG_TRACE("Out Channel Disconnected");

	pCCB->OutChannelConnected = FALSE;

	pCCB->OutChannelHandle = NULL;

	DBG_LEAVE();
	return Status;
}



/*
 * VoipDropAllCalls
 */
void
VoipDropAllCalls()
{
    DBG_FUNC("VoipDropAllCalls",H323_LAYER);
    DBG_ENTER();
    int Count;
    PCALL_CONTROL_BLOCK pCCB;

    for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)  {
        pCCB = &CCBArray[Count];
	    if ( (pCCB->H323CallState!=H323_CALL_STATE_IDLE) && (pCCB->LanCallHandle != NULL) )
	    {
            if (pCCB->CanDropCall == TRUE)
	            cmCallDrop(pCCB->LanCallHandle);

           	if (pCCB->InChannelHandle != NULL)
		        cmChannelDrop(pCCB->InChannelHandle);

            if (pCCB->OutChannelHandle != NULL)
	            cmChannelDrop(pCCB->OutChannelHandle);

            pCCB->LocalRelease = TRUE;
	    }
    }
    DBG_LEAVE();
}




#define BASE_OFFSET				0
#define STATE_STR_OFFSET		BASE_OFFSET
#define STATE_STR_WIDTH			58
#define EVENT_STR_OFFSET		STATE_STR_OFFSET+STATE_STR_WIDTH
#define EVENT_STR_WIDTH			52
#define NEXT_STATE_STR_OFFSET	EVENT_STR_OFFSET+EVENT_STR_WIDTH
#define NEXT_STATE_STR_WIDTH	68
#define NULL_VALUE_OFFSET		NEXT_STATE_STR_OFFSET+NEXT_STATE_STR_WIDTH

STATUS
smFatalStateMachineViolation(PCALL_CONTROL_BLOCK pCCB,PVOID  Context)
{
	DBG_FUNC("smFatalStateMachineViolation",H323_PROTOCOL_LAYER);
	DBG_ENTER();
	STATUS Status =  STATUS_SUCCESS;

	DBG_ERROR("Call %#x : State Machine Violation : Current State : %s : Event %s : Next State %s\n",
	            pCCB,GetH323CallStateName(pCCB->PreviousCallState),
	            GetH323EventName(pCCB->CurrentEvent),GetH323CallStateName(pCCB->H323CallState));
    DBG_ERROR("State transition history:-\n");
	DumpStateTransitionHistory(pCCB);
 	/*
 	 * ASSERT(0);
     */

	DBG_LEAVE();
	return Status;
}


STATUS
smNonFatalStateMachineViolation(PCALL_CONTROL_BLOCK pCCB,PVOID  Context)
{
	DBG_FUNC("smNonFatalStateMachineViolation",H323_PROTOCOL_LAYER);
	DBG_ENTER();
	STATUS Status = STATUS_SUCCESS;

	/*
	DBG_WARNING("Non Fatal State Machine Violation: -\n");
	DumpStateTransitionHistory(pCCB);
	*/
	DBG_LEAVE();
	return Status;
}










/*
 * GetH323CallStateName
 */
char *GetH323CallStateName(H323_CALL_STATE CallState)
{

   	static char* StateNames[] =
    { 
		"H323_CALL_STATE_IDLE",
		"H323_CALL_STATE_WAIT_FOR_CC_ANSWER",
		"H323_CALL_STATE_WAIT_FOR_PROCEEDING",
		"H323_CALL_STATE_WAIT_FOR_CONNECTION",
		"H323_CALL_STATE_CALL_CONNECTED",
		"H323_CALL_STATE_WAIT_FOR_CAP_EXCHANGE_ACK",
		"H323_CALL_STATE_WAIT_FOR_CHANNELS_CONNECTION",
		"H323_CALL_STATE_CHANNELS_CONNECTED",
    };
	static char* UnknownStateString = "UNKNOWN STATE";
	static char* SameStateString = "H323_CALL_STATE_SAME_STATE";


   	if( CallState >= 0 && CallState < H323_MAX_CALL_STATES)
   	{
      	return StateNames[CallState];
	}
	else if (CallState == H323_CALL_STATE_SAME_STATE)
	{
		return SameStateString;
	}

	return UnknownStateString;
}


/*
 * GetH323EventName
 */
char *GetH323EventName(H323_CALL_EVENT CallEvent)
{

   	static char* EventNames[] =
    { 
		"H323_EVENT_RECV_CALL_OFFERING",
		"H323_EVENT_RECV_CC_CALL_RELEASE",
		"H323_EVENT_RECV_CC_ANSWER_CALL",
		"H323_EVENT_RECV_CC_CALL_SETUP",
		"H323_EVENT_RECV_CALL_PROCEEDING",
		"H323_EVENT_RECV_CALL_RINGBACK",
		"H323_EVENT_RECV_CALL_CONNECTED",
		"H323_EVENT_RECV_CALL_DISCONNECTED",
		"H323_EVENT_RECV_CALL_DEST_REJECTED",
		"H323_EVENT_RECV_CALL_IDLE",
		"H323_EVENT_RECV_REMOTE_CAPABILITIES",
		"H323_EVENT_RECV_H245_TRANSPORT_CONNECTED",
		"H323_EVENT_RECV_H245_CONNECTED",
		"H323_EVENT_RECV_CAPABILITIES_ACK",
		"H323_EVENT_RECV_CAPABILITIES_NAK",
		"H323_EVENT_RECV_CHANNEL_OFFERING",
		"H323_EVENT_RECV_IN_CHANNEL_CONNECTED",
		"H323_EVENT_RECV_OUT_CHANNEL_CONNECTED",
		"H323_EVENT_RECV_IN_CHANNEL_DISCONNECTED",
		"H323_EVENT_RECV_OUT_CHANNEL_DISCONNECTED",
    };
	static char* UnknownEventString = "UNKNOWN EVENT";

   	if( CallEvent >= 0 && CallEvent < H323_MAX_CALL_EVENTS )
   	{
      	return EventNames[CallEvent];
	}

	return UnknownEventString;
}




/*
 * UpdateStateTransitionHistory
 */
void
UpdateStateTransitionHistory(PCALL_CONTROL_BLOCK pCCB,H323_CALL_STATE LatestState,H323_CALL_EVENT LatestEvent,H323_CALL_STATE NextState)
{
	int Index;
	pCCB->StateTransitionHistory.PairIndex++;

	if (pCCB->StateTransitionHistory.PairIndex >= MAX_STATE_TRANSITION_HISTORY) 
	{ 
		pCCB->StateTransitionHistory.PairIndex = Index = 0;
	}
    else 
        Index = pCCB->StateTransitionHistory.PairIndex;

	pCCB->StateTransitionHistory.CallStates[Index] = LatestState;
	pCCB->StateTransitionHistory.NextCallStates[Index] = NextState;
	pCCB->StateTransitionHistory.CallEvents[Index] = LatestEvent;
}



/*
 * DumpStateTransitionHistory
 */
void
DumpStateTransitionHistory(PCALL_CONTROL_BLOCK pCCB)
{
	DBG_FUNC("DumpStateTransitionHistory",H323_PROTOCOL_LAYER);
	DBG_ENTER();

	int Count,Index;
	char TempString[256];
 	Index = pCCB->StateTransitionHistory.PairIndex;

    DBG_ERROR("State machine transition history dump for CCB 0x%x\n",pCCB);

	for(Count=0;Count<MAX_STATE_TRANSITION_HISTORY;Count++)
	{
		if (Index < MAX_STATE_TRANSITION_HISTORY)
		{
			memset(TempString,32,256);
			sprintf(&TempString[STATE_STR_OFFSET],"State %s",GetH323CallStateName(pCCB->StateTransitionHistory.CallStates[Index]));
			sprintf(&TempString[EVENT_STR_OFFSET],": Event %s",GetH323EventName(pCCB->StateTransitionHistory.CallEvents[Index]));
			sprintf(&TempString[NEXT_STATE_STR_OFFSET]," --> State %s",GetH323CallStateName(pCCB->StateTransitionHistory.NextCallStates[Index]));
			for(int Count=0;Count<256;Count++)
				if (TempString[Count] == 0) TempString[Count] = 32;
			TempString[NULL_VALUE_OFFSET] = 0;
			DBG_ERROR("%s\n",TempString);
		}

        Index++;
        /*
         * Roll back
         */
		if (Index >= MAX_STATE_TRANSITION_HISTORY)
		{
		 	Index = 0;
		}
	}

	DBG_LEAVE();
	return;
 }




/*
 * SendLocalTermCapSet
 */
void
SendLocalTermCapSet(PCALL_CONTROL_BLOCK pCCB)
{
    DBG_FUNC("SendLocalTermCapSet",H323_LAYER);
    DBG_ENTER();
	BOOL SpecifyFullRateCodec=FALSE,SpecifyEnhancedFullRateCodec=FALSE;

    if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_FR) {
		SpecifyFullRateCodec = TRUE;
	}
	else {
		SpecifyEnhancedFullRateCodec = TRUE;
	}

    char nameToRemove[100];

    memset(nameToRemove,0,100);

    HPVT hVal=cmGetValTree(pVoipTaskConfig->StackHandle);
	HPST h245Syn=cmGetSynTreeByRootName(pVoipTaskConfig->StackHandle,"h245");
	int nodeId=pvtGetNodeIdByPath(hVal,cmGetH245ConfigurationHandle(pVoipTaskConfig->StackHandle),"capabilities.terminalCapabilitySet");

    if ((SpecifyFullRateCodec) && (SpecifyEnhancedFullRateCodec)) {
        cmCallSendCapability(pCCB->LanCallHandle, nodeId);
        return;
    }

    if (SpecifyFullRateCodec != TRUE) {
        strcpy(nameToRemove,"gsmFullRate");
    }
    else if (SpecifyEnhancedFullRateCodec != TRUE) {
        strcpy(nameToRemove,"gsmEnhancedFullRate");
    }
    else {
        DBG_ERROR("Invalid case specified whether neither FullRate nor EnhancedFullRateCodec is to be specified\n");
#ifdef VOIP_DEVELOP_DEBUG
        ASSERT(0);
#endif
        return;
    }

	int newNodeId=pvtAddRootByPath(hVal,h245Syn, "request.terminalCapabilitySet", 0, NULL);
	pvtSetTree(hVal,newNodeId,hVal,nodeId);
	{
	    int capTab=pvtGetNodeIdByPath(hVal,newNodeId,"capabilityTable");
	    int capDesc=pvtGetNodeIdByPath(hVal,newNodeId,"capabilityDescriptors");
	    int child=pvtChild(hVal,capTab);
	    int pChild;
	    while(child>=0)
	    {
			int fieldId;
			char name[100];
			int mediaNodeId=pvtGetByPath(hVal,child,"capability.*.*",(int *)&fieldId,NULL,NULL);
			pstGetFieldName(h245Syn,fieldId,sizeof(name),name);
			pChild=child;
			child=pvtBrother(hVal,child);
			if (!strcmp(name,"application"))
			{
			    pvtGetByPath(hVal,mediaNodeId,"*",(int *)&fieldId,NULL,NULL);
			    pstGetFieldName(h245Syn,fieldId,sizeof(name),name);
			}
			if (!strcmp(name,nameToRemove))
			{
			    int indexToRemove;
			    int next=pvtNext(hVal,capDesc,capDesc);
			    int pNext;
			    int synNodeId1,synNodeId2;
			    int value;
			    pvtGet(hVal,pvtGetNodeIdByPath(hVal,capDesc,"*.simultaneousCapabilities.*.*"),NULL,&synNodeId1,NULL,NULL);
			    pvtGetByPath(hVal,pChild,"capabilityTableEntryNumber",NULL,&indexToRemove,NULL);
			    
			    while(next>0)
			    {
				pNext=next;
				next=pvtNext(hVal,capDesc,next);    
				pvtGet(hVal,pNext,NULL,&synNodeId2,&value,NULL);
				if (value==indexToRemove && synNodeId1==synNodeId2)
				{
				    int parent=pvtParent(hVal,pNext);
				    int grandParent=pvtParent(hVal,parent);
				    pvtDelete(hVal,(pvtNumChilds(hVal,parent)==1)?parent:pNext);
				    if (pvtNumChilds(hVal,grandParent)==0)
					pvtDelete(hVal,pvtParent(hVal,grandParent));
				}
			    }
			    pvtDelete(hVal,pChild);
			}
	    }

	}
	cmCallSendCapability(pCCB->LanCallHandle, newNodeId);       
	pvtDelete(hVal,newNodeId);
    DBG_LEAVE();
 }



/*
 * SendCodecInfo
 */
void
SendCodecInfo(PCALL_CONTROL_BLOCK pCCB)
{
    DBG_FUNC("SendCodecInfo",H323_LAYER);
    DBG_ENTER();

	if (!pCCB->CodecInfoSent) {
		PAPI_MESSAGE	ApiMsg;
		ApiMsg = GetFreeMsgFromPool();
		if (ApiMsg == NULL) {
			DBG_ERROR("All API messages used up from pool!\n");
			DBG_LEAVE();
			return;
		}
		PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

		pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
		pMsg->H323CRV				= pCCB->H323CRV;
		pMsg->LudbId 				= pCCB->LudbId;
		pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
		pMsg->VoipCallHandle 		= pCCB->Index;
		pMsg->TxnId					= pCCB->LastTxnId;
		pMsg->MessageType 			= VOIP_API_CODEC_SELECTED;



	    if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_FR) {
	        pMsg->CodecSelected.Codec = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
	        DBG_TRACE("GSM Full rate codec to be used for call CCB= %x\n",pCCB);
	    }
	    else if (pCCB->CodecUsed == VOIP_CALL_CODEC_GSM_EFR) {
	        pMsg->CodecSelected.Codec = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2;
	        DBG_TRACE("GSM Enhanced Full rate codec to be used for call CCB= %x\n",pCCB);
	    }
	    else {
	        DBG_ERROR("Invalid codec specified\n");
#ifdef VOIP_DEVELOP_DEBUG
	        ASSERT(0);
#endif
	    }


		/*
		 * Send the message now
	     */
		BOOL Status=VoipSendMessageToCc(pMsg);
		PutMsgIntoPool(ApiMsg);
		pCCB->CodecInfoSent = TRUE;
	}

    DBG_LEAVE();
    return;
}



/*
 * SendProgressMessageToCC
 */
BOOL
SendProgressMessageToCC(PCALL_CONTROL_BLOCK pCCB)
{
    DBG_FUNC("SendProgressMessageToCC",H323_LAYER);
    DBG_ENTER();
    BOOL status = FALSE, valueRetrieved=FALSE;
  	int fieldId,value;
	BOOL isString;
	
    if (pCCB->Magic == H323TASK_MAGIC_VALUE) {
		PAPI_MESSAGE	ApiMsg;
		ApiMsg = GetFreeMsgFromPool();
		if (ApiMsg == NULL) {
			DBG_ERROR("All API messages used up from pool!\n");
			DBG_LEAVE();
			return(FALSE);
		}
		PVOIP_API_MESSAGE	pMsg = (PVOIP_API_MESSAGE)&ApiMsg->VoipMsg;

	    if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				    "progress.message.*.progressIndicator.octet4.ext",&fieldId,&value,&isString) < 0)
	    {
		    DBG_TRACE("Could not get Progress Indicator in Progress msg : octet4.ext bit is not set\n");
	    }
        else {
	        /* octet4.progressDescription */
	        if (pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),cmGetProperty((HPROTOCOL)pCCB->LanCallHandle),
				        "progress.message.*.progressIndicator.octet4.progressDescription",&fieldId,&value,&isString) < 0)
	        {
		        DBG_WARNING("Could not get Progress Indicator description in Progress msg\n");
	        }
            else {
                pMsg->CallEvent.EventInfo.AlertingProgressIE.ie_present         = TRUE;
                pMsg->CallEvent.EventInfo.AlertingProgressIE.codingStandard     = CNI_RIL3_GSMPLMN_CODING;
                pMsg->CallEvent.EventInfo.AlertingProgressIE.location           = CNI_RIL3_LOCATION_PUBLIC_NETWORK_LOCAL_USER;
                pMsg->CallEvent.EventInfo.AlertingProgressIE.progressDesp       = (T_CNI_RIL3_PROGRESS_DESCRIPTION)value;
                DBG_TRACE("Progress Indicator retrieved from Progress msg, value %#x\n",value);
                valueRetrieved = TRUE;
            }
        }


	    pMsg->Magic 				= VOIP_API_MAGIC_VALUE;
	    pMsg->H323CRV				= pCCB->H323CRV;
	    pMsg->LudbId 				= pCCB->LudbId;
	    pMsg->VoipMobileHandle 		= ludbGetVoipMobileHandle(pCCB->LudbId);
	    pMsg->VoipCallHandle 		= pCCB->Index;
	    pMsg->TxnId					= pCCB->LastTxnId;
	    pMsg->MessageType 			= VOIP_API_CALL_EVENT;
   	    pMsg->CallEvent.CallEvent 	= VOIP_MOBILE_CALL_EVENT_PROGRESS;
        if (!valueRetrieved) {
            /*
             * We default to in-band information, which is the most likely suspect
             */
            DBG_TRACE("Defaulting to in-band information value in progressIE in progress msg\n");
            pMsg->CallEvent.EventInfo.AlertingProgressIE.ie_present         = TRUE;
            pMsg->CallEvent.EventInfo.AlertingProgressIE.codingStandard     = CNI_RIL3_GSMPLMN_CODING;
            pMsg->CallEvent.EventInfo.AlertingProgressIE.location           = CNI_RIL3_LOCATION_PUBLIC_NETWORK_LOCAL_USER;
            pMsg->CallEvent.EventInfo.AlertingProgressIE.progressDesp       = CNI_RIL3_PROGRESS_IN_BAND_INFO_AVAILABLE;
        }
	    DBG_TRACE("Mobile %s: Sending Progress Message to CC\n",pCCB->MobileNumber);
	    status=VoipSendMessageToCc(pMsg);
		PutMsgIntoPool(ApiMsg);
    }

    DBG_LEAVE();
    return (status);
}


/*
 * VoipDisplayCallParties
 */
void
VoipDisplayCallParties(PCALL_CONTROL_BLOCK pCCB)
{
	DBG_FUNC("VoipDisplayCallParties",H323_LAYER);
	DBG_ENTER();

    if (pCCB->CallDirection == H323_CALL_TERMINATED) {
		DBG_TRACE("Calling party %s : Plan %s, Type %s, PI %s, SI %s\n",
					pCCB->RemotePartyId,
					GetVoipNumPlanStr((Q931_IE_CALL_PARTY_NO_PLAN)pCCB->CallingNumberingPlan),
					GetVoipNumTypeStr((Q931_IE_CALL_PARTY_NO_TYPE)pCCB->CallingNumberType),
					GetVoipPIStr((Q931_IE_CALLING_PARTY_NO_PI)pCCB->CallingPartyPI),
					GetVoipSIStr((Q931_IE_CALLING_PARTY_NO_SI)pCCB->CallingPartySI)
					);
		DBG_TRACE("Called party %s : Plan %s, Type %s\n",
					pCCB->MobileNumber,
					GetVoipNumPlanStr((Q931_IE_CALL_PARTY_NO_PLAN)pCCB->CalledNumberingPlan),
					GetVoipNumTypeStr((Q931_IE_CALL_PARTY_NO_TYPE)pCCB->CalledNumberType));
	}
	else {
		DBG_TRACE("Calling party %s : Plan %s, Type %s, PI %s, SI %s\n",
					pCCB->MobileNumber,
					GetVoipNumPlanStr((Q931_IE_CALL_PARTY_NO_PLAN)pCCB->CallingNumberingPlan),
					GetVoipNumTypeStr((Q931_IE_CALL_PARTY_NO_TYPE)pCCB->CallingNumberType),
					GetVoipPIStr((Q931_IE_CALLING_PARTY_NO_PI)pCCB->CallingPartyPI),
					GetVoipSIStr((Q931_IE_CALLING_PARTY_NO_SI)pCCB->CallingPartySI)
					);
		DBG_TRACE("Called party %s : Plan %s, Type %s\n",
					pCCB->RemotePartyId,
					GetVoipNumPlanStr((Q931_IE_CALL_PARTY_NO_PLAN)pCCB->CalledNumberingPlan),
					GetVoipNumTypeStr((Q931_IE_CALL_PARTY_NO_TYPE)pCCB->CalledNumberType));
	}

	DBG_LEAVE();
}









/*
 * GetVoipNumTypeStr
 */
char *
GetVoipNumTypeStr(Q931_IE_CALL_PARTY_NO_TYPE numType)
{
	static char NumTypeStr[128];
	switch(numType) {
		case Q931_IE_CALL_PARTY_NO_TYPE_UNKNOWN:{
			strncpy(NumTypeStr,"UNKNOWN",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_INTERNATIONAL:{
			strncpy(NumTypeStr,"INTERNATIONAL",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_NATIONAL:{
			strncpy(NumTypeStr,"NATIONAL",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_NETWK_SPECIFIC:{
			strncpy(NumTypeStr,"NETWORK SPECIFIC",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_SUBS_NO:{
			strncpy(NumTypeStr,"SUBSCRIBER",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_ABBR_NO:{
			strncpy(NumTypeStr,"ABBRIEVATED NUMBER",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_RESERVED:{
			strncpy(NumTypeStr,"RESERVED",128);
		}
		break;
		default:{
			strncpy(NumTypeStr,"UNKNOWN NUMBER TYPE",128);
		}
		break;
	}
	return (NumTypeStr);
}


/*
 * GetVoipNumPlanStr
 */					
char *
GetVoipNumPlanStr(Q931_IE_CALL_PARTY_NO_PLAN numPlan)
{
	static char NumPlanStr[128];
	switch(numPlan) {
		case Q931_IE_CALL_PARTY_NO_PLAN_UNKNOWN:{
			strncpy(NumPlanStr,"UNKNOWN",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_PLAN_ISDN:{
			strncpy(NumPlanStr,"ISDN",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_PLAN_DATA:{
			strncpy(NumPlanStr,"DATA",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_PLAN_TELEX:{
			strncpy(NumPlanStr,"TELEX",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_PLAN_NATIONAL:{
			strncpy(NumPlanStr,"NATIONAL",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_PLAN_PRIVATE:{
			strncpy(NumPlanStr,"PRIVATE",128);
		}
		break;
		case Q931_IE_CALL_PARTY_NO_PLAN_RESERVED:{
			strncpy(NumPlanStr,"RESERVED",128);
		}
		break;
		default:{
			strncpy(NumPlanStr,"UNKNOWN NUMBERING PLAN",128);
		}
		break;
	}
	return (NumPlanStr);
}

/*
 * GetVoipPIStr
 */
char *
GetVoipPIStr(Q931_IE_CALLING_PARTY_NO_PI PI)
{
	static char presentationIndicatorStr[128];
	switch (PI) {
		case Q931_IE_CALLING_PARTY_NO_PI_ALLOWED:{
			strncpy(presentationIndicatorStr,"PRESENTATION ALLOWED",128);
		}
		break;
		case Q931_IE_CALLING_PARTY_NO_PI_RESTRICTED:{
			strncpy(presentationIndicatorStr,"PRESENTATION RESTRICTED",128);
		}
		break;
		case Q931_IE_CALLING_PARTY_NO_PI_NOT_AVAIL:{
			strncpy(presentationIndicatorStr,"NOT AVAILABLE",128);
		}
		break;
		case Q931_IE_CALLING_PARTY_NO_PI_RESERVED:{
			strncpy(presentationIndicatorStr,"PRESENTATION RESERVED",128);
		}
		break;
		default:{
			strncpy(presentationIndicatorStr,"UNKNOWN PRESENTATION INDICATOR",128);
		}
		break;
	}
	return (presentationIndicatorStr);
}

/*
 * GetVoipSIStr
 */
char *
GetVoipSIStr(Q931_IE_CALLING_PARTY_NO_SI SI)
{
	static char screeningIndicatorStr[128];
	switch (SI) {
		case Q931_IE_CALLING_PARTY_NO_SI_UP_NOT_SCREENED:{
			strncpy(screeningIndicatorStr,"USER PROVIDED, NOT SCREENED",128);
		}
		break;
		case Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_PASSED:{
			strncpy(screeningIndicatorStr,"USER PROVIDED,PASSED",128);
		}
		break;
		case Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_FAILED:{
			strncpy(screeningIndicatorStr,"USER PROVIDED,VERIFIED,FAILED",128);
		}
		break;
		case Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED:{
			strncpy(screeningIndicatorStr,"NETWORK PROVIDED",128);
		}
		break;
		default:{
			strncpy(screeningIndicatorStr,"UNKNOWN SCREENING INDICATOR",128);
		}
		break;
	}
	return (screeningIndicatorStr);
}






/*
 * VoipToGsmNumType
 */
void
VoipToGsmNumType(Q931_IE_CALL_PARTY_NO_TYPE voipNumType,T_CNI_RIL3_TYPE_OF_NUMBER *gsmNumType)
{
	switch (voipNumType) {
		case Q931_IE_CALL_PARTY_NO_TYPE_INTERNATIONAL:
			*gsmNumType = CNI_RIL3_NUMBER_TYPE_INTERNATIONAL;
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_NATIONAL:
			*gsmNumType = CNI_RIL3_NUMBER_TYPE_NATIONAL;
		break;
		case Q931_IE_CALL_PARTY_NO_TYPE_NETWK_SPECIFIC:
			*gsmNumType = CNI_RIL3_NUMBER_TYPE_NETWORK_SPECIFIC;
		break;
		/*
		 * No mappings for these guys
		 */
		case Q931_IE_CALL_PARTY_NO_TYPE_SUBS_NO:
		case Q931_IE_CALL_PARTY_NO_TYPE_ABBR_NO:
		case Q931_IE_CALL_PARTY_NO_TYPE_RESERVED:
		default:
		case Q931_IE_CALL_PARTY_NO_TYPE_UNKNOWN:
			*gsmNumType = CNI_RIL3_NUMBER_TYPE_UNKNOWN;
		break;
	}
	return;
}


/*
 * VoipToGsmNumPlan
 */
void
VoipToGsmNumPlan(Q931_IE_CALL_PARTY_NO_PLAN voipNumPlan,T_CNI_RIL3_NUMBERING_PLAN_ID *gsmNumPlan)
{
	switch(voipNumPlan) {

		case Q931_IE_CALL_PARTY_NO_PLAN_ISDN:
			*gsmNumPlan = CNI_RIL3_NUMBERING_PLAN_ISDN_TELEPHONY;
		break;

		case Q931_IE_CALL_PARTY_NO_PLAN_DATA:
			*gsmNumPlan = CNI_RIL3_NUMBERING_PLAN_DATA;
		break;

		case Q931_IE_CALL_PARTY_NO_PLAN_TELEX:
			*gsmNumPlan = CNI_RIL3_NUMBERING_PLAN_TELEX;
		break;

		case Q931_IE_CALL_PARTY_NO_PLAN_NATIONAL:
			*gsmNumPlan = CNI_RIL3_NUMBERING_PLAN_NATIONAL;
		break;

		case Q931_IE_CALL_PARTY_NO_PLAN_PRIVATE:
			*gsmNumPlan = CNI_RIL3_NUMBERING_PLAN_PRIVATE;
		break;

		case Q931_IE_CALL_PARTY_NO_PLAN_RESERVED:
		case Q931_IE_CALL_PARTY_NO_PLAN_UNKNOWN:
		default:
			*gsmNumPlan = CNI_RIL3_NUMBERING_PLAN_UNKNOWN;
		break;
	}
	return;
}


/*
 * VoipToGsmPI
 */
void
VoipToGsmPI(Q931_IE_CALLING_PARTY_NO_PI voipPI,T_CNI_RIL3_PRESENTATION_INDICATOR *gsmPI)
{
	switch(voipPI) {
		case Q931_IE_CALLING_PARTY_NO_PI_ALLOWED:
			*gsmPI = CNI_RIL3_PRESENTATION_ALLOWED;
		break;

		case Q931_IE_CALLING_PARTY_NO_PI_RESTRICTED:
			*gsmPI = CNI_RIL3_PRESENTATION_RESTRICTED;
		break;

		case Q931_IE_CALLING_PARTY_NO_PI_NOT_AVAIL:
		case Q931_IE_CALLING_PARTY_NO_PI_RESERVED:
		default:
			*gsmPI = CNI_RIL3_PRESENTATION_NUMBER_NOT_AVAILABLE;
		break;
	}
	return;
}


/*
 * VoipToGsmSI
 */
void
VoipToGsmSI(Q931_IE_CALLING_PARTY_NO_SI voipSI,T_CNI_RIL3_SCREENING_INDICATOR *gsmSI)
{
	switch(voipSI) {
		default:
		case Q931_IE_CALLING_PARTY_NO_SI_UP_NOT_SCREENED:
			*gsmSI = CNI_RIL3_SCREENING_USER_PROVIDED_NOT_SCREENED;
		break;
		case Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_PASSED:
			*gsmSI = CNI_RIL3_SCREENING_USER_PROVIDED_PASSED;
		break;
		case Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_FAILED:
			*gsmSI = CNI_RIL3_SCREENING_USER_PROVIDED_FAILED;
		break;
		case Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED:
			*gsmSI = CNI_RIL3_SCREENING_NETWORK_PROVIDED;
		break;
	}
	return;
}



/*
 * GsmToVoipNumType
 */
void
GsmToVoipNumType(T_CNI_RIL3_TYPE_OF_NUMBER gsmNumType,Q931_IE_CALL_PARTY_NO_TYPE *voipNumType)
{


	switch (gsmNumType) {
		case CNI_RIL3_NUMBER_TYPE_INTERNATIONAL:
			*voipNumType = Q931_IE_CALL_PARTY_NO_TYPE_INTERNATIONAL;
		break;
		case CNI_RIL3_NUMBER_TYPE_NATIONAL:
			*voipNumType = Q931_IE_CALL_PARTY_NO_TYPE_NATIONAL;
		break;
		case CNI_RIL3_NUMBER_TYPE_NETWORK_SPECIFIC:
			*voipNumType = Q931_IE_CALL_PARTY_NO_TYPE_NETWK_SPECIFIC;
		break;
		case CNI_RIL3_NUMBER_TYPE_DEDICATED_ACCESS:
		case CNI_RIL3_NUMBER_TYPE_UNKNOWN:
		default:
			*voipNumType = 	Q931_IE_CALL_PARTY_NO_TYPE_UNKNOWN;
		break;
	}
	return;
}


/*
 * GsmToVoipNumPlan
 */
void
GsmToVoipNumPlan(T_CNI_RIL3_NUMBERING_PLAN_ID gsmNumPlan,Q931_IE_CALL_PARTY_NO_PLAN *voipNumPlan)
{
	switch(gsmNumPlan){
		case CNI_RIL3_NUMBERING_PLAN_ISDN_TELEPHONY:
			*voipNumPlan = Q931_IE_CALL_PARTY_NO_PLAN_ISDN;
		break;

		case CNI_RIL3_NUMBERING_PLAN_DATA:
			*voipNumPlan = Q931_IE_CALL_PARTY_NO_PLAN_DATA;
		break;

		case CNI_RIL3_NUMBERING_PLAN_TELEX:
			*voipNumPlan = Q931_IE_CALL_PARTY_NO_PLAN_TELEX;
		break;

		case CNI_RIL3_NUMBERING_PLAN_NATIONAL:
			*voipNumPlan = Q931_IE_CALL_PARTY_NO_PLAN_NATIONAL;
		break;

		case CNI_RIL3_NUMBERING_PLAN_PRIVATE:
			*voipNumPlan = Q931_IE_CALL_PARTY_NO_PLAN_PRIVATE;
		break;

		case CNI_RIL3_NUMBERING_PLAN_UNKNOWN:
		default:
			*voipNumPlan = Q931_IE_CALL_PARTY_NO_PLAN_UNKNOWN;
		break;

	}
	return;
}



/*
 * GsmToVoipPI
 */
void
GsmToVoipPI(T_CNI_RIL3_PRESENTATION_INDICATOR gsmPI,Q931_IE_CALLING_PARTY_NO_PI *voipPI)
{
	switch(gsmPI) {
		case CNI_RIL3_PRESENTATION_ALLOWED:
			*voipPI = Q931_IE_CALLING_PARTY_NO_PI_ALLOWED;
		break;
		case CNI_RIL3_PRESENTATION_RESTRICTED:
			*voipPI = Q931_IE_CALLING_PARTY_NO_PI_RESTRICTED;
		break;
		case CNI_RIL3_PRESENTATION_NUMBER_NOT_AVAILABLE:
		default:
			*voipPI = Q931_IE_CALLING_PARTY_NO_PI_NOT_AVAIL;
		break;
	}
	return;
}

/*
 * GsmToVoipSI
 */
void
GsmToVoipSI(T_CNI_RIL3_SCREENING_INDICATOR gsmSI,Q931_IE_CALLING_PARTY_NO_SI *voipSI)
{

    /*
     * Since we provide the number from the LUDB, we have to hardcode
     * the screening Indicator to network provided
     */

	*voipSI = Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED;
    return;


	switch(gsmSI) {

		case CNI_RIL3_SCREENING_USER_PROVIDED_NOT_SCREENED:
			*voipSI = Q931_IE_CALLING_PARTY_NO_SI_UP_NOT_SCREENED;
		break;
		case CNI_RIL3_SCREENING_USER_PROVIDED_PASSED:
			*voipSI = Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_PASSED;
		break;
		case CNI_RIL3_SCREENING_USER_PROVIDED_FAILED:
			*voipSI = Q931_IE_CALLING_PARTY_NO_SI_UP_VERIFIED_FAILED;
		break;
		case CNI_RIL3_SCREENING_NETWORK_PROVIDED:
			*voipSI = Q931_IE_CALLING_PARTY_NO_SI_NETWORK_PROVIDED;
		break;
	}
	return;

}

