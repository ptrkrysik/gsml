
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: EventHandlers.cpp												*
 *																						*
 *	Description			: Functions for processing the H.323 protocol events			*
 *						  reported by the RadVision H.323 stack							*
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
#include <cm.h>
#include <stkutils.h>
#include <tickLib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>
#include <time.h>

#include "h323task.h"
#include "ccb.h"
#include "util.h"
#include "eventhandlers.h"
#include "rtptask.h"
#include "smroutines.h"
#include "voip\vblink.h"
#include "voip\voipapi.h"
#include "jcc\ludbapi.h"

extern PH323TASK_CONFIG	pVoipTaskConfig;


/*
 * NewCallHandler
 */
int CALLCONV NewCallHandler(
   IN  HAPP       hApp,
   IN  HCALL      hsCall,
   OUT LPHAPPCALL lphaCall )
{
	DBG_FUNC("NewCallHandler",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK		pCCB;
    LUDB_ID                 LudbId;

   	cmAlias Alias;
   	char CalledPartyNumber[256];

	memset(CalledPartyNumber,0,256);
   	*lphaCall = NULL; 

   	Alias.string = CalledPartyNumber; 

   	if( cmCallGetParam( hsCall, cmParamCalledPartyNumber, 0, NULL, (char*)&Alias ) < 0 ||
       Alias.length <= 0 )
   	{
      	DBG_ERROR( "Error in cmCallGetParam()\n" );
	   	if( cmCallGetParam( hsCall, cmParamDestinationAddress, 0, NULL, (char*)&Alias ) < 0 ||
	       Alias.length <= 0 ) {
      		DBG_ERROR( "Cannot retrieve called party number or destination address of incoming call\n" );
			DBG_LEAVE();
	      	return ERROR;
		}
   	}

   	DBG_TRACE( "New Incoming Call to %s\n", CalledPartyNumber );

	LudbId = 0;
	if (!ludbIsMobileRegistered(CalledPartyNumber,&LudbId))
	{
	    DBG_TRACE("Called mobile is not online now\n" );
	    LudbId = -1;
	}


   	pCCB = GetCCBFromPool();
#ifdef VOIP_DEVELOP_DEBUG
   	ASSERT(pCCB!=NULL);
#endif
   	if (pCCB == NULL)
   	{
		DBG_ERROR("No more call blocks available to handle call!\n");
		DBG_LEAVE();
		return ERROR;
   	}	


	pCCB->LanCallHandle = hsCall;
    pCCB->LudbId        = LudbId;
    pCCB->TpktHandle    = cmGetTpktChanHandle(pCCB->LanCallHandle,cmQ931TpktChannel);
    strcpy(pCCB->MobileNumber,CalledPartyNumber);


	sseCreateCall(pVoipTaskConfig->SuppStackHandle,&(pCCB->SuppCallHandle),(HSSEAPPCALL)pCCB,
			pCCB->LanCallHandle);
	sseCallImplementTransfer(pCCB->SuppCallHandle,TRUE);
	sseCallImplementForward(pCCB->SuppCallHandle);


    *lphaCall = (HAPPCALL)pCCB;

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : New incoming call \n",pCCB->MobileNumber);

	DBG_LEAVE();
    return OK;
}

	

/*
 * RASEventHandler
 */
int CALLCONV RASEventHandler(
   IN  HAPP       hApp,
   IN  cmRegState regState,
   IN  cmRegEvent regEvent,
   IN  int        regEventHandle )
{
	DBG_FUNC("RASEventHandler",H323_LAYER);
	DBG_ENTER();
    int retStatus=OK;

   	if( regEvent == -1 ) 
   	{
		DBG_LEAVE();
   		return OK;
	}


   	switch( regEvent )
   	{
      	case cmRegistrationConfirm:
        {
			DBG_ERROR("Registration confirmation received from GK\n");
        }
        break;

      	case cmUnregistrationConfirm:
        {
			DBG_ERROR("Unregistration Confirmation received from GK!\n");
        }
        break;

      	case cmRegistrationReject:
			DBG_ERROR("Registration Reject received from GK!\n");
        break;

      	case cmUnregistrationReject:
			DBG_ERROR("Unregistration Reject received from GK!\n");
		break;

      	case cmUnregistrationRequest:
			DBG_ERROR("Unregistration Request received from GK!\n");
        break;

      	case cmGatekeeperConfirm:
			DBG_ERROR("GateKeeper confirm received \n");
		break;
      	case cmGatekeeperReject:
			DBG_ERROR("GateKeeper reject received \n");
		break;
      	case cmNonStandardMessage:
			DBG_ERROR("NonStandard Message received \n");
		break;
      	default:
			DBG_ERROR("UNKNOWN RAS event received \n");
		break;
   }
	DBG_LEAVE();
   	return retStatus;
}






/*
 *	EventCallStateChanged 
 */
int CALLCONV EventCallStateChanged(
   IN HAPPCALL haCall,
   IN HCALL    hsCall,
   IN UINT32   State,
   IN UINT32   StateMode )
{
	DBG_FUNC("EventCallStateChanged",H323_LAYER);
	DBG_ENTER();
   	if( NULL == haCall)
   	{ /* to drop "over capacity" and "wrong number" calls */
      	switch( State )
      	{	
         	case cmCallStateOffering:
		        DBG_WARNING("Dropping Call since capacity exceeded or number not found!\n");
            	cmCallDrop( hsCall );
            break;

         	default:
            	cmCallClose( hsCall );
            break;
      }
	  DBG_LEAVE();
      return OK;
   	}

	PCALL_CONTROL_BLOCK	pCCB = (PCALL_CONTROL_BLOCK)haCall;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif	
	if (pCCB->Magic != H323TASK_MAGIC_VALUE) 
	{
		DBG_ERROR("Invalid CCB returned !\n");
		DBG_LEAVE();
		return ERROR;
	}

    sseCallStateChanged(pCCB->SuppCallHandle,State,StateMode);



   	DBG_TRACE("Mobile '%s' : Call 0x%x : State %s, Mode %s\n",
             pCCB->MobileNumber, pCCB, GetCallStateName( State ), GetCallStateModeName( StateMode ) );

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : State %s : Mode %s\n",pCCB->MobileNumber, 
        GetCallStateName( State ), GetCallStateModeName( StateMode ) );


   	switch( State )
   	{
      	case cmCallStateOffering:
      	{
			CALL_OFFERING_DETAILS CallOfferingDetails;
			memset(CallOfferingDetails.pszCallingPartyId,0,MAX_CALLING_PARTY_ADDRESS_SIZE );
			memset(CallOfferingDetails.pszCalledPartyId,0,MAX_CALLED_PARTY_ADDRESS_SIZE );

         	cmCallGetCallingPartyId( hsCall, CallOfferingDetails.pszCallingPartyId, MAX_CALLING_PARTY_ADDRESS_SIZE );
         	cmCallGetCalledPartyId( hsCall, CallOfferingDetails.pszCalledPartyId, MAX_CALLED_PARTY_ADDRESS_SIZE );

         	DBG_TRACE("Call Offering : CallingParty: '%s', CalledParty: '%s'\n",
                   CallOfferingDetails.pszCallingPartyId,
                   CallOfferingDetails.pszCalledPartyId);

            /*
             * FIXME::PUT IN CODE TO CHECK WHETHER THIS IS A FAST-START CALL
             *
             * pCCB->FastStart = FALSE;
			 */
			RunStateMachine(pCCB,(PVOID)&CallOfferingDetails,H323_EVENT_RECV_CALL_OFFERING);
      	}
		break;


    	case cmCallStateDialtone:
		{
				DBG_TRACE("CallState = DIALTONE\n");
                if (pCCB->FastStart) {
                    if (smSetupFastChannels(pCCB) != STATUS_SUCCESS) {
						cmCallDrop(pCCB->LanCallHandle);
					}
				}
                pCCB->TpktHandle    = cmGetTpktChanHandle(pCCB->LanCallHandle,cmQ931TpktChannel);

		}
		break;


      	case cmCallStateProceeding:
		{
			DBG_TRACE("Call proceeding\n");
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CALL_PROCEEDING);
        }
        break;

      	case cmCallStateRingBack:
		{
			DBG_TRACE("Call RingBack\n");
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CALL_RINGBACK);
		}
        break;

      	case cmCallStateConnected:
      	{


            BOOL fs;


		    fs = ((cmCallGetParam((HCALL)hsCall,cmParamConnectFastStart,0,NULL,NULL)>=0)
    		|| (cmCallGetParam((HCALL)hsCall,cmParamAlertingFastStart,0,NULL,NULL)>=0)
		    || (cmCallGetParam((HCALL)hsCall,cmParamCallProcFastStart,0,NULL,NULL)>=0));

       		if(fs) {
                DBG_TRACE("FastStart returns TRUE\n");
                pCCB->FastStart = TRUE;
            }
            else {
                DBG_TRACE("FastStart returns FALSE\n");
                pCCB->FastStart = FALSE;
            }


         	switch( StateMode )
         	{

            	case cmCallStateModeCallSetupConnected: /* Q.931 is completed; comes first */
               		DetectRemoteTerminalType(pCCB);
					RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CALL_CONNECTED);
               	break;

            	case cmCallStateModeControlConnected:   /* not always sent */
               	break;

            	case cmCallStateModeCallConnected:
					DBG_TRACE("CallStateModeCallConnected roeceived\n");
               	break;

            	default:
               	break;
         	}
      	}
		break;

      	case cmCallStateDisconnected:
      	{
			if (pCCB->CallTransferred) {
				DBG_TRACE("Transferred call-leg disconnected\n");
			}
	        int CauseValue=-1;
			unsigned char CauseCode=0;
	        cmCallGetParam(hsCall,cmParamReleaseCompleteCause,0,&CauseValue,NULL);
			CauseCode = (unsigned char)CauseValue;
			pVoipTaskConfig->CallStats.CallDisconnectStats.VoipDisconnects[CauseValue]++;
            DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Call Disconnected with Q.850 Cause %d\n",pCCB->MobileNumber,CauseCode);
			RunStateMachine(pCCB,&CauseCode,H323_EVENT_RECV_CALL_DISCONNECTED);
      	}
		break;

      	case cmCallStateIncompleteAddress:
      	case cmCallStateAdmissionReject:
	  	{
			DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Call Destination Rejected\n",pCCB->MobileNumber);
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CALL_DEST_REJECTED);
		}
         break;


      	case cmCallStateIdle:
		{
			if (pCCB->IsSuppCall) {
				DBG_TRACE("Supplementary call leg disconnected\n");
				return OK;
			}
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CALL_IDLE);
		}
        break;

      default:
         break;
   }

   DBG_LEAVE();
   return OK;
}







/*
 * EventCallInfo
 */
int CALLCONV EventCallInfo(
   IN HAPPCALL haCall,
   IN HCALL    hsCall,
   IN char*    Display,
   IN char*    User,
   IN int      UserSize )
{
	DBG_FUNC("EventCallInfo",H323_LAYER);
	DBG_ENTER();
   	
   	DBG_TRACE("Display = %s, User = %s\n",
             Display ? Display : "<no>", User ? User : "<no>" );

	DBG_LEAVE();
   	return OK;
}




/*
 * EventCallCapabilities
 */
int CALLCONV EventCallCapabilities(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmCapStruct*        capabilities[] )
{
	DBG_FUNC("EventCallCapabilities",H323_LAYER);
	DBG_ENTER();

   	int i;
    REMOTE_CAPABILITIES_DETAILS RemoteCapabilities;

   	PCALL_CONTROL_BLOCK pCCB= (PCALL_CONTROL_BLOCK)haCall;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB value returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

#ifdef DEBUG_CAPABILITIES
   	DBG_WARNING("Capabilities reported by remote:\n" );
   	for( i = 0; capabilities[i]; i++ )
      CapabilityPrint( capabilities[i] );

	DBG_WARNING("End of Remote capabilities\n");
#endif /* #ifdef DEBUG_CAPABILITIES */

    RemoteCapabilities.GSMCodecCapability           = FALSE;
    RemoteCapabilities.GSMFullRateCapability        = FALSE;
    RemoteCapabilities.GSMEnhancedFullRateCapability= FALSE;

  	/*
  	 * looking for GSM audio cap
	 */

   	for( i = 0; capabilities[i]; i++ )
   	{
       	if (capabilities[i]->type == cmCapAudio) 
       	{
            if (strcmp( capabilities[i]->name, "gsmFullRate" ) == 0) {
                RemoteCapabilities.GSMCodecCapability=TRUE;
                RemoteCapabilities.GSMFullRateCapability = TRUE;
			    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : H.323 V2 GSM FR Codec capability reported by remote endpoint\n",
			        pCCB->MobileNumber);
           	    break;
            }
            if (strcmp( capabilities[i]->name, "gsmEnhancedFullRate" ) == 0) {
                RemoteCapabilities.GSMCodecCapability=TRUE;
                RemoteCapabilities.GSMEnhancedFullRateCapability = TRUE;
			    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : H.323 V2 GSM EFR Codec capability reported by remote endpoint\n",
			        pCCB->MobileNumber);
           	    break;
            }

       	}
   	}
	
	if (!RemoteCapabilities.GSMCodecCapability) {
		DBG_WARNING("GSM Capability NOT found!!\n");
	    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : No H.323 V2 GSM Codecs capability reported by remote endpoint\n",
	            pCCB->MobileNumber);
	}

    
    

	RunStateMachine(pCCB,(PVOID)&RemoteCapabilities,H323_EVENT_RECV_REMOTE_CAPABILITIES);

	DBG_LEAVE();
   	return OK;
}


/*
 *	EventCallCapabilitiesExt
 */
int CALLCONV EventCallCapabilitiesExt(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmCapStruct***      capabilities[] )
{
	DBG_FUNC("EventCallCapabilitiesExt",H323_LAYER);
	DBG_ENTER();

   	int i, j, k;
   	cmCapStruct* pCaps;
#ifdef DEBUG_CAPABILITIES
   	DBG_TRACE("Capability Descriptors:\n" );

   	for( i = 0; capabilities[i]; i++ )
   	{
      DBG_TRACE("Simultaneous Capabilities:\n" );

      for( j = 0; capabilities[i][j]; j++ )
      {
         DBG_TRACE("Alternative Capabilities:\n" );

         for( k = 0; capabilities[i][j][k]; k++ )
         {
            CapabilityPrint( pCaps = capabilities[i][j][k] );
            
         }

      }

   	}

#endif /* #ifdef DEBUG_CAPABILITIES */

	DBG_LEAVE();
   	return OK;
}



/*
 *	EventCallNewChannel
 */
int CALLCONV EventCallNewChannel(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      HCHAN               hsChan,
      OUT     LPHAPPCHAN          lphaChan )
{
	DBG_FUNC("EventCallNewChannel",H323_LAYER);
	DBG_ENTER();
   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haCall;
	BOOL origin;
	char direction[128];


#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}
    
    cmChannelGetOrigin(hsChan,&origin);

	if (origin) {
		strcpy(direction,"outgoing");
	}
	else {
		strcpy(direction,"incoming");
	}

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : New %s channel 0x%x\n",pCCB->MobileNumber,direction,hsChan);
   	DBG_TRACE("New %s channel 0x%x for '%s'\n", direction,hsChan,
             pCCB->MobileNumber);

   	*lphaChan = (HAPPCHAN)pCCB;
	DBG_LEAVE();
   	return OK;
}





/*
 *	EventCallCapabilitiesResponse
 */
int CALLCONV EventCallCapabilitiesResponse(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              status )
{
	DBG_FUNC("EventCallCapabilitiesResponse",H323_LAYER);
	DBG_ENTER();
   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haCall;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}


   	switch(status )
   	{
      	case cmCapAccept:
			DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : H.245 TermCapSet Accepted by remote endpoint\n",pCCB->MobileNumber);
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CAPABILITIES_ACK);
        break;

      	case cmCapReject:
			DBG_WARNING("H.245 TermCapSet REJECTED by remote endpoint\n");
			DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : H.245 TermCapSet REJECTED by remote endpoint\n",pCCB->MobileNumber);
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CAPABILITIES_NAK);
        break;

      	default:
			DBG_WARNING("Unknown Capabilities response by remote station\n");
        break;
   }

   
   DBG_LEAVE();
   return OK;
}



/*
 *	EventCallMasterSlaveStatus
 */
int CALLCONV EventCallMasterSlaveStatus(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              status )
{
	DBG_FUNC("EventCallMasterSlaveStatus",H323_LAYER);
	DBG_ENTER();
   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haCall;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

   	char* psz;

   	switch( status )
   	{
      	case cmMSMaster:
         	psz = "Master";
        break;

      	case cmMSSlave:
         	psz = "Slave";
        break;

      	case cmMSError:
         	psz = "Error!";
        break;

      	default:
         	psz = "Unknown!";
        break;
   }

   	DBG_TRACE("Master-Slave determination: Remote station is %s\n", psz );
    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Remote station is %s for the call\n",pCCB->MobileNumber,psz);
	DBG_LEAVE();
   	return OK;
}



/*
 *	EventCallRoundTripDelay
 */
int CALLCONV EventCallRoundTripDelay(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      INT32               delay )
{
	DBG_FUNC("EventCallRoundTripDelay",H323_LAYER);
	DBG_ENTER();
   	DBG_TRACE( "Round Trip Delay: %d\n", delay );
	DBG_LEAVE();
   	return OK;
}





/*
 *	EventCallUserInput
 */
int CALLCONV EventCallUserInput(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      INT32               userInputId )
{
	DBG_FUNC("EventCallUserInput",H323_LAYER);
	DBG_ENTER();

	DBG_ERROR("Received USER INPUT !!!\n");

	DBG_LEAVE();
   	return OK;
}



/*
 *	EventCallRequestMode
 */
int CALLCONV EventCallRequestMode(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmReqModeStatus     status,
      IN      INT32               nodeId )
{
	DBG_FUNC("EventCallRequestMode",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}





/*
 *	EventCallMiscStatus
 */
int CALLCONV EventCallMiscStatus(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      cmMiscStatus        status )
{
	DBG_FUNC("EventCallMiscStatus",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}





/*
 * EventCallControlStateChanged
 */
int CALLCONV EventCallControlStateChanged(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              State,
      IN      UINT32              StateMode )
{
	DBG_FUNC("EventCallControlStateChanged",H323_LAYER);
	DBG_ENTER();

   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haCall;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

   	DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : Control State %s\n",pCCB->MobileNumber,GetControlStateName( State ) );

   	switch( State )
   	{
/*
  cmControlStateConnected,
  cmControlStateConference,    
  cmControlStateTransportConnected,
  cmControlStateTransportDisconnected,
*/
		case cmControlStateFastStart:
		{
			DBG_TRACE("Fast Setup successful\n");
			pCCB->FastStart = TRUE;
		}
		break;


      	case cmControlStateTransportConnected: 
		{
			DBG_TRACE("H.245 Transport connected\n");
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_H245_TRANSPORT_CONNECTED);
		}
        break;
      	case cmControlStateConnected:         
		{
			DBG_TRACE("H.245 Connected\n");
			RunStateMachine(pCCB,NULL,H323_EVENT_RECV_H245_CONNECTED);
		}
      	break;

      default:
         break;
   	}
	DBG_LEAVE();
   	return OK;
}






/*
 *	EventCallMasterSlave
 */
int CALLCONV EventCallMasterSlave(
      IN      HAPPCALL            haCall,
      IN      HCALL               hsCall,
      IN      UINT32              terminalType,
      IN      UINT32              statusDeterminationNumber )
{
	DBG_FUNC("EventCallMasterSlave",H323_LAYER);
	DBG_ENTER();
   	DBG_TRACE("Remote terminalType = %d, SDN = %d\n",
             terminalType, statusDeterminationNumber );
	DBG_LEAVE();
   	return OK;
}





/*
 * EventCallFacility
 */
int CALLCONV 
EventCallFacility(
		IN      HAPPCALL    haCall,
		IN      HCALL       hsCall,
		IN      int         handle,
		OUT IN	BOOL		*proceed)
{
    DBG_FUNC("EventCallFacility",H323_LAYER);
	DBG_ENTER();
    INT32 valueAlt=0;
    INT32 valueAlias=0;
	int nodeId;

    *proceed = TRUE;
    

    nodeId = pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),handle,"message.*.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.*.alternativeAddress",NULL,&valueAlt,NULL);
    DBG_TRACE("Facility : node id %d, Alternate %d\n",nodeId,valueAlt);

    nodeId = pvtGetByPath(cmGetValTree(pVoipTaskConfig->StackHandle),handle,"message.*.userUser.h323-UserInformation.h323-uu-pdu.h323-message-body.*.alternativeAliasAddress",NULL,&valueAlias,NULL);
    DBG_TRACE("Facility : node id %d,Alias %d\n",nodeId,valueAlias);

	DBG_LEAVE();
    return 0;
}


/*
 *  EventCallFastStartSetup
 */
int CALLCONV
EventCallFastStartSetup(HAPPCALL haCall, HCALL hsCall,cmFastStartMessage *fsMessage)
{
    DBG_FUNC("EventCallFastStartSetup",H323_LAYER);
	DBG_ENTER();

    cmTransportAddress rtp;
    cmTransportAddress rtcp;
    int i,j,k;

   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haCall;

#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

    pCCB->FastStart = TRUE;

	

    for (i=0;i<fsMessage->partnerChannelsNum;i++) {
        if (fsMessage->partnerChannels[i].type != cmCapAudio) continue;
        else {
			/* Search for gsmFullRate first */
            for (j=0;j<fsMessage->partnerChannels[i].transmit.altChannelNumber;j++) {
                if (strcmp(fsMessage->partnerChannels[i].transmit.channels[j].channelName,"gsmFullRate")==0) {
					pCCB->remoteGsmFrTxIndex = fsMessage->partnerChannels[i].transmit.channels[j].index;
                    DBG_TRACE("GSM Full Rate codec transmit capability found\n");
                }
            }
            for (k=0;k<fsMessage->partnerChannels[i].receive.altChannelNumber;k++) {
                if (strcmp(fsMessage->partnerChannels[i].receive.channels[k].channelName,"gsmFullRate")==0) {
					pCCB->remoteGsmFrRxIndex = fsMessage->partnerChannels[i].receive.channels[k].index;
                    DBG_TRACE("GSM Full Rate codec receive capability found\n");
					pCCB->RemoteGSMFullRateCapability=TRUE;
                }
            }


			/* Search for gsmEnhancedFullRate first */
            for (j=0;j<fsMessage->partnerChannels[i].transmit.altChannelNumber;j++) {
                if (strcmp(fsMessage->partnerChannels[i].transmit.channels[j].channelName,"gsmEnhancedFullRate")==0) {
					pCCB->remoteGsmEfrTxIndex = fsMessage->partnerChannels[i].transmit.channels[j].index;
                    DBG_TRACE("GSM Enhanced Full Rate codec transmit capability found\n");
                }
            }
            for (k=0;k<fsMessage->partnerChannels[i].receive.altChannelNumber;k++) {
                if (strcmp(fsMessage->partnerChannels[i].receive.channels[k].channelName,"gsmEnhancedFullRate")==0) {
					pCCB->remoteGsmEfrRxIndex = fsMessage->partnerChannels[i].receive.channels[k].index;
                    DBG_TRACE("GSM Enhanced Full Rate codec receive capability found\n");
					pCCB->RemoteGSMEnhancedFullRateCapability=TRUE;
                }
            }
        }
    }  

    DBG_LEAVE();
    return TRUE;
}





/*
 * suppEventForwardActivated
 */
int CALLCONV 
suppEventForwardActivated(
	IN HSSEAPPCALL	hSSEaCall,
	IN HSSECALL		hSSECall
	)
{
	DBG_FUNC("suppEventForwardActivated",H323_LAYER);
	DBG_ENTER();
	DBG_TRACE("Forwarding activated\n");
	DBG_LEAVE();
	return (0);
}


/*
 * suppEventForwardDeactivated
 */
int CALLCONV
suppEventForwardDeactivated(
	IN HSSEAPPCALL	hSSEaCall,
	IN HSSECALL		hSSECall
	)
{
	DBG_FUNC("suppEventForwardDeactivated",H323_LAYER);
	DBG_ENTER();
	DBG_TRACE("Forwarding De-activated\n");
	DBG_LEAVE();
	return (0);
}


/*
 * suppEventCallReroute
 */
int CALLCONV
suppEventCallReroute(
	IN HSSEAPPCALL	hSSEaCallPri,
	IN HSSECALL		hSSECallPri,
	OUT HSSECALL 	*hSSECallSec)
{

	DBG_FUNC("suppEventCallReroute",H323_LAYER);
	DBG_ENTER();
	DBG_TRACE("Call rerouting started\n");
/*
	cmCallNew(hApp,(HAPPCALL)&theCall[fr],&(theCall[fr].hsCall));
	sseCreateCall(hSSEApp,&(theCall[fr].hSSECall),(HSSEAPPCALL)&theCall[fr],
			  theCall[fr].hsCall);
	
	sseCallImplementTransfer(theCall[fr].hSSECall,TRUE);
	sseCallImplementForward(theCall[fr].hSSECall);
	*hSSECallSec=theCall[fr].hSSECall;
*/
	DBG_LEAVE();
	return (0);
}



/*
 * suppEventCallTransfer
 */
int CALLCONV 
suppEventCallTransfer(
		IN	HSSEAPPCALL	    hSSEaCallPri,
		IN  HSSECALL	    hSSECallPri,
		OUT	HSSECALL*	    hSSECallSec)
{

	DBG_FUNC("suppEventCallTransfer",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK pOrigCCB,pCCB;
	STATUS Status;

	pOrigCCB = (PCALL_CONTROL_BLOCK)hSSEaCallPri;
	if (pOrigCCB->Magic != H323TASK_MAGIC_VALUE) {
		DBG_ERROR("Unknown handle returned by callback!\n");
		return ERROR;
	}


	DBG_TRACE("Call of mobile %s being transferred\n",pOrigCCB->MobileNumber);

	pCCB = GetCCBFromPool();
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB != NULL);
#endif
	if (pCCB == NULL) {
		DBG_ERROR("No more call blocks available in pool\n");
		DBG_LEAVE();
		return ERROR;
	}

	pCCB->IsSuppCall 			= TRUE;
	pOrigCCB->CallTransferred 	= TRUE;

   	if(cmCallNew(pVoipTaskConfig->StackHandle,(HAPPCALL)pCCB,&pCCB->LanCallHandle) < 0) {
		DBG_ERROR("Call capacity exhausted in RadVision stack\n");
		DBG_LEAVE();
		return ERROR;
	}
	sseCreateCall(pVoipTaskConfig->SuppStackHandle,&(pCCB->SuppCallHandle),(HSSEAPPCALL)pCCB,
			pCCB->LanCallHandle);
	sseCallImplementTransfer(pCCB->SuppCallHandle,TRUE);
	sseCallImplementForward(pCCB->SuppCallHandle);

	pCCB->H323CRV		= pOrigCCB->H323CRV;
	pCCB->LudbId		= pOrigCCB->LudbId;
	pCCB->LastTxnId		= pOrigCCB->LastTxnId;
	pCCB->CodecUsed		= pOrigCCB->CodecUsed;
	pCCB->OldIndex		= pOrigCCB->Index;

	pCCB->CallingNumberType		= pOrigCCB->CallingNumberType;
	pCCB->CallingNumberingPlan	= pOrigCCB->CallingNumberingPlan;
	pCCB->CallingPartyPI		= pOrigCCB->CallingPartyPI;
	pCCB->CallingPartySI		= pOrigCCB->CallingPartySI;



	pCCB->GSMFullRateCapability = pOrigCCB->GSMFullRateCapability;
	pCCB->GSMEnhancedFullRateCapability = pOrigCCB->GSMEnhancedFullRateCapability;

	pCCB->FastStart 	= TRUE;
	strcpy(pCCB->MobileNumber,pOrigCCB->MobileNumber);
	/* cc note */
	/*   don't cat the -TRDSE, since it will be used to setup fast channel
	strcat(pCCB->MobileNumber,"-TRDSE");
	*/

	strcpy(pCCB->TransferringPartyId,pOrigCCB->RemotePartyId);

	*hSSECallSec = pCCB->SuppCallHandle;

	pCCB->H323CallState = H323_CALL_STATE_WAIT_FOR_PROCEEDING;


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
	pMsg->VoipCallHandle 		= pCCB->OldIndex;
	pMsg->TxnId					= pCCB->LastTxnId;
	pMsg->MessageType 			= VOIP_API_CALL_UPDATE_HANDLES;
   	pMsg->CallUpdateHandles.NewHandle 	= pCCB->Index;
    pMsg->CallUpdateHandles.ReconnectOnTransferFailure = FALSE;
	DBG_TRACE("Mobile %s: Sending Update Handles to CC\n",pCCB->MobileNumber);
	Status=VoipSendMessageToCc(pMsg);

	DBG_LEAVE();
	return (0);
}





/*
 * EventCallH450SupplementaryService
 */
int CALLCONV 
EventCallH450SupplementaryService(
			IN	HAPPCALL	  haCall, 
			IN	HCALL		  hsCall,
			IN	cmCallQ931MsgType msgType,
			IN	int		  nodeId,
			IN	int		  size)
{
	DBG_FUNC("EventCallH450SupplementaryService",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haCall;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

    sseCallH450SupplServ(pCCB->SuppCallHandle,msgType,nodeId,size);
	DBG_LEAVE();
    return 0;
}




/*
 *
 * EventChannelSetRTCPAddress
 */
int CALLCONV EventChannelSetRTCPAddress(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              ip,
      IN      UINT16              port )
{
    DBG_FUNC("jcEvChannelSetRTCPAddress",H323_LAYER);
    DBG_ENTER();
    PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haChan;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : RTCP address %s for channel 0x%x\n", 
            pCCB->MobileNumber,jcIpToString( ip, port ), hsChan );
   
    jcRtcpSetRemoteAddress( jcRtpGetRTCPSession( pCCB->RtpHandle ), ip, port );
   	
    DBG_LEAVE();
    return OK;
}


/*
 * EventChannelRTPDynamicPayloadType
 */
int CALLCONV EventChannelRTPDynamicPayloadType(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      INT8                dynamicPayloadType )
{
    DBG_FUNC("EventChannelRTPDynamicPayloadType",H323_LAYER);
    DBG_ENTER();
    PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haChan;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}
    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : RTP payload type value %d for channel 0x%x\n",
            pCCB->MobileNumber,(int)dynamicPayloadType, hsChan );
    DBG_LEAVE();
    return OK;
}


/*
 * EventChannelGetRTCPAddress
 */
int CALLCONV EventChannelGetRTCPAddress(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32*             ip,
      IN      UINT16*             port )
{
   DBG_FUNC("EventChannelGetRTCPAddress",H323_LAYER);
   DBG_ENTER();
   DBG_LEAVE();
   return OK;
}



/*
 * EventChannelSetAddress
 */
int CALLCONV EventChannelSetAddress(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              ip,
      IN      UINT16              port )
{
	DBG_FUNC("EventChannelSetAddress",H323_LAYER);
	DBG_ENTER();

   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haChan;
#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}

    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : RTP address %s for channel 0x%x\n", 
            pCCB->MobileNumber,jcIpToString( ip, port ), hsChan );
   	jcRtpSetRemoteAddress( pCCB->RtpHandle, ip, port );

	DBG_LEAVE();
   	return OK;
}




/*
 *	EventChannelStateChanged
 */
int CALLCONV EventChannelStateChanged(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              State,
      IN      UINT32              StateMode )
{
	DBG_FUNC("EventChannelStateChanged",H323_LAYER);
	DBG_ENTER();


   	DBG_TRACE("Channel State Changed() for %x channel: State '%s', StateMode '%s'\n", hsChan,
             GetChannelStateName( State ), GetChannelStateModeName( StateMode ) );

   	switch( State )
   	{

		case cmChannelStateDisconnected:
		{
		   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haChan;
#ifdef VOIP_DEVELOP_DEBUG
			ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
			if (pCCB->Magic != H323TASK_MAGIC_VALUE)
			{
				DBG_ERROR("Invalid CCB returned!\n");
				DBG_LEAVE();
				return ERROR;
			}

			if (pCCB->InChannelHandle == hsChan)
			{
			    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : IN Channel 0x%x disconnected\n",pCCB->MobileNumber,hsChan);
				RunStateMachine(pCCB,NULL,H323_EVENT_RECV_IN_CHANNEL_DISCONNECTED);
			}
			else if (pCCB->OutChannelHandle == hsChan)
			{
			    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : OUT Channel 0x%x disconnected\n",pCCB->MobileNumber,hsChan);
				RunStateMachine(pCCB,NULL,H323_EVENT_RECV_OUT_CHANNEL_DISCONNECTED);
			}
			else if (pCCB->NonAudioChannelHandle == hsChan)
			{
				DBG_TRACE("Non audio channel %x disconnected\n");
			}
			else 
			{
				DBG_ERROR("Unknown channel disconnected!!\n");
			}
		}
		break;
		case cmChannelStateOffering:
		{
		   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haChan;
#ifdef VOIP_DEVELOP_DEBUG
			ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
			if (pCCB->Magic != H323TASK_MAGIC_VALUE)
			{
				DBG_ERROR("Invalid CCB returned!\n");
				DBG_LEAVE();
				return ERROR;
			}
			if (pCCB->NonAudioChannelHandle == hsChan)
			{
				DBG_TRACE("Non audio channel %x disconnected\n");
			}
			else
			{
       			DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : 0x%x Channel offering\n",pCCB->MobileNumber,hsChan);
				DBG_TRACE("%x Channel offering \n",hsChan);
				pCCB->InChannelHandle = hsChan;
				RunStateMachine(pCCB,NULL,H323_EVENT_RECV_CHANNEL_OFFERING);
			}
		}
        break;

      	case cmChannelStateConnected:
		{
		   	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)haChan;
#ifdef VOIP_DEVELOP_DEBUG
			ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
			if (pCCB->Magic != H323TASK_MAGIC_VALUE)
			{
				DBG_ERROR("Invalid CCB returned!\n");
				DBG_LEAVE();
				return ERROR;
			}
			if (hsChan == pCCB->NonAudioChannelHandle)
			break;

			BOOL origin;
    		cmChannelGetOrigin(hsChan,&origin);
    		if (origin)
			{
       			DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : OUT Channel 0x%x Connected\n",pCCB->MobileNumber,hsChan);
				DBG_TRACE("OutChannel %x connected\n",hsChan);
				RunStateMachine(pCCB,NULL,H323_EVENT_RECV_OUT_CHANNEL_CONNECTED);
			}
			else
			{
       			DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : IN Channel 0x%x Connected\n",pCCB->MobileNumber,hsChan);
				DBG_TRACE("InChannel %x connected\n",hsChan);
				RunStateMachine(pCCB,NULL,H323_EVENT_RECV_IN_CHANNEL_CONNECTED);
			}
		}
        break;

    	case cmChannelStateDialtone:
    	case cmChannelStateRingBack:
		{
		}
		break;
      	case cmChannelStateIdle:
		{	
			cmChannelClose(hsChan);
		}
        break;

      	default:
		{
			DBG_WARNING("Unknown channel state change reported!\n");
		}
        break;
   	}
    DBG_LEAVE();
   return OK;
}

/*
 *	EventChannelNewRate
 */
int CALLCONV EventChannelNewRate(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      UINT32              rate )
{
	DBG_FUNC("EventChannelNewRate",H323_LAYER);
	DBG_ENTER();
	DBG_TRACE( "Rate = %d\n", rate );
	DBG_LEAVE();
   	return OK;
}



/*
 *	EventChannelMaxSkew
 */
int CALLCONV EventChannelMaxSkew(
      IN      HAPPCHAN            haChan1,
      IN      HCHAN               hsChan1,
      IN      HAPPCHAN            haChan2,
      IN      HCHAN               hsChan2,
      IN      UINT32              skew )
{
	DBG_FUNC("EventChannelMaxSkew",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}


	

 

/*
 * EventChannelParameters
 */
int CALLCONV EventChannelParameters(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      char*               channelName,
      IN      HAPPCHAN            haChanSameSession,
      IN      HCHAN               hsChanSameSession,
      IN      HAPPCHAN            haChanAssociated,
      IN      HCHAN               hsChanAssociated,
      IN      UINT32              rate )
{
	DBG_FUNC("EventChannelParameters",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK	pCCB = (PCALL_CONTROL_BLOCK)haChan;


#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}


   	DBG_TRACE( "Parameters for %x channel. Name: '%s', Rate: %d\n",
             hsChan, channelName, rate );

	if (pCCB->FastStart) {

	    if (strcmp(channelName, "gsmFullRate" ) == 0) {
		    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s :  GSM FullRate Codec selected by remote endpoint\n",
		        pCCB->MobileNumber);
			pCCB->RemoteGSMFullRateCapability=TRUE;
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_PAYLOAD_TYPE);
	    }
	    if (strcmp(channelName, "gsmEnhancedFullRate" ) == 0) {
		    DBG_CALL_TRACE((unsigned long)pCCB,"Mobile %s : GSM Enhanced FullRate Codec selected by remote endpoint\n",
		        pCCB->MobileNumber);
				pCCB->RemoteGSMEnhancedFullRateCapability=TRUE;
    		jcRtpSetPayloadType(pCCB->RtpHandle,(BYTE)RTP_GSM_EFR_PAYLOAD_TYPE);
		}

	}

	DBG_LEAVE();
   	return OK;
}


/*
 * EventChannelVideoFastUpdatePicture
 */
int CALLCONV EventChannelVideoFastUpdatePicture(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan )
{
	DBG_FUNC("EventChannelVideoFastUpdatePicture",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelVideoFastUpdateGOB
 */
int CALLCONV EventChannelVideoFastUpdateGOB(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      int                 firstGOB,
      IN      int                 numberOfGOBs )
{
	DBG_FUNC("EventChannelVideoFastUpdateGOB",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelVideoFastUpdateMB
 */
int CALLCONV EventChannelVideoFastUpdateMB(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      int                 firstGOB,
      IN      int                 firstMB,
      IN      int                 numberOfMBs )
{
	DBG_FUNC("EventChannelVideoFastUpdateMB",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelHandle
 */
int CALLCONV EventChannelHandle(
      IN      HAPPCHAN            haChan,
      IN      HCHAN               hsChan,
      IN      int                 dataTypeHandle,
      IN      cmCapDataType       dataType )
{
	DBG_FUNC("EventChannelHandle",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK	pCCB = (PCALL_CONTROL_BLOCK)haChan;
	BOOL origin;

	cmChannelGetOrigin(hsChan,&origin);


#ifdef VOIP_DEVELOP_DEBUG
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
#endif
	if (pCCB->Magic != H323TASK_MAGIC_VALUE)
	{
		DBG_ERROR("Invalid CCB returned!\n");
		DBG_LEAVE();
		return ERROR;
	}
   	if( dataType != cmCapAudio )
   	{
		DBG_WARNING("Dropping non-audio channel %x\n",hsChan);
      	cmChannelDrop( hsChan );
		pCCB->NonAudioChannelHandle = hsChan;
		DBG_LEAVE();
      	return OK;
   	}
	else {
		if (origin) {
			pCCB->OutChannelHandle = hsChan;
		}
		else {
			pCCB->InChannelHandle = hsChan;
		}
	}

	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelRequestCloseStatus
 */
int CALLCONV EventChannelRequestCloseStatus(
      IN      HAPPCHAN              haChan,
      IN      HCHAN                 hsChan,
      IN      cmRequestCloseStatus  status )
{
	DBG_FUNC("EventChannelRequestCloseStatus",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}

/*
 *
 */
int CALLCONV EventChannelTSTO(
      IN      HAPPCHAN              haChan,
      IN      HCHAN                 hsChan,
      IN      INT8                  isCommand,
      IN      INT8                  tradeoffValue )
{
	DBG_FUNC("EventChannelTSTO",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelMediaLoopStatus
 */
int CALLCONV EventChannelMediaLoopStatus(
      IN      HAPPCHAN              haChan,
      IN      HCHAN                 hsChan,
      IN      cmMediaLoopStatus     status )
{
	DBG_FUNC("EventChannelMediaLoopStatus",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelReplace
 */
int CALLCONV EventChannelReplace(
      IN    HAPPCHAN         haChan,
      IN    HCHAN            hsChan,
      IN    HAPPCHAN         haReplacedChannel,
      IN    HCHAN            hsReplacedChannel )
{
	DBG_FUNC("EventChannelReplace",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}



/*
 * EventChannelFlowControlToZero
 */
int CALLCONV EventChannelFlowControlToZero(
      IN    HAPPCHAN         haChan,
      IN    HCHAN            hsChan )
{
	DBG_FUNC("EventChannelFlowControlToZero",H323_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
   	return OK;
}


/*
 * EventHookSendTo
 */
BOOL CALLCONV 
EventHookSendTo(
        IN      HPROTCONN           hConn,
		IN      int                 nodeId,
        IN      int                 nodeIdTo,
		IN      BOOL                error
		)
{
	DBG_FUNC("EventHookSendTo",H323_PROTOCOL_LAYER);
    DBG_ENTER();
    char messageName[256],protocolName[256];

    memset(messageName,0,256);
    memset(protocolName,0,256);

    strcpy(messageName,cmGetProtocolMessageName(pVoipTaskConfig->StackHandle,nodeId));
    strcpy(protocolName,cmProtocolGetProtocolName(cmProtocolGetProtocol(pVoipTaskConfig->StackHandle,nodeId)));

    DBG_TRACE("SENTTO message --> %s on %s \n",
    					messageName,
                        protocolName
                        );

	DBG_LEAVE();
    return OK;
}


/*
 * EventHookSend
 */
int CALLCONV EventHookSend(
      IN    HPROTCONN        hConn,
      IN    int              nodeId,
      IN    BOOL             error )
{
	DBG_FUNC("EventHookSend",H323_PROTOCOL_LAYER);
	DBG_ENTER();
    char messageName[256],protocolName[256];

    memset(messageName,0,256);
    memset(protocolName,0,256);

    strcpy(messageName,cmGetProtocolMessageName( pVoipTaskConfig->StackHandle, nodeId ));
    strcpy(protocolName,cmProtocolGetProtocolName( cmProtocolGetProtocol( pVoipTaskConfig->StackHandle, nodeId ) ) );

   	DBG_TRACE( "Message sent -> %s on %s\n",
             messageName,
             protocolName);

	DBG_LEAVE();
   	return OK;
}




/*
 * EventHookRecv
 */
int CALLCONV EventHookRecv(
      IN    HPROTCONN        hConn,
      IN    int              nodeId,
      IN    BOOL             error )
{
	DBG_FUNC("EventHookRecv",H323_PROTOCOL_LAYER);
	DBG_ENTER();
    char messageName[256],protocolName[256];
    PCALL_CONTROL_BLOCK pCCB;

    memset(messageName,0,256);
    memset(protocolName,0,256);

	strcpy(messageName,cmGetProtocolMessageName(pVoipTaskConfig->StackHandle,nodeId));
	strcpy(protocolName,cmProtocolGetProtocolName(cmProtocolGetProtocol(pVoipTaskConfig->StackHandle,nodeId)));

   	DBG_TRACE("Message recv <- %s on %s\n", messageName,protocolName);

	if (!strcmp("progress",messageName)){
        pCCB = GetCCBByTpktHandle((HPROTCONN)hConn);
        if (pCCB != NULL) {
            if (!SendProgressMessageToCC(pCCB)) {
                DBG_ERROR("Could not send Progress message to CC!!\n");
            }
        }
        else {
            DBG_ERROR("Progress message received but could not find call handle for TPK handle %#x\n",hConn);
        }
	}

    DBG_LEAVE();
   	return OK;
}

 
 
BOOL CALLCONV EventHookRecvFrom(
        IN      HPROTCONN           hConn,
		IN      int                 nodeId,
        IN      int                 nodeIdFrom,
        IN      BOOL                multicast,
		IN      BOOL                error
	        )
{
	DBG_FUNC("EventHookRecvFrom",H323_PROTOCOL_LAYER);
    DBG_ENTER();

    char messageName[256],protocolName[256];

    memset(messageName,0,256);
    memset(protocolName,0,256);

	strcpy(messageName,cmGetProtocolMessageName(pVoipTaskConfig->StackHandle,nodeId));
	strcpy(protocolName,cmProtocolGetProtocolName(cmProtocolGetProtocol(pVoipTaskConfig->StackHandle,nodeId)));

   	DBG_TRACE("Message recv <- %s on %s\n", messageName,protocolName);
	DBG_LEAVE();
    return 0;
}


