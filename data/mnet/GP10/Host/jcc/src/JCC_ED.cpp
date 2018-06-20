// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : JCC_ED.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include <vxworks.h>
#include <stdio.h>

#include "JCC_ED.h"
#include "lapdm/lapdm_l3intf.h"
#include "lapdm/lapdm_config.h"
#include "RIL3/ril3irt.h"
#include "CC/CCconfig.h"

// *******************************************************************
// Local function prototypes
// *******************************************************************
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeAlerting     (T_CNI_RIL3CC_MSG_ALERTING       &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeCallConfirmed(T_CNI_RIL3CC_MSG_CALL_CONFIRMED &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeConnect      (T_CNI_RIL3CC_MSG_CONNECT        &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeConnectAck   (T_CNI_RIL3CC_MSG_CONNECT_ACK    &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeSetup        (T_CNI_RIL3CC_MSG_SETUP		     &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeDisconnect   (T_CNI_RIL3CC_MSG_DISCONNECT     &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeRelease      (T_CNI_RIL3CC_MSG_RELEASE		 &);
T_CNI_RIL3_RESULT JCC_RIL3SMS_DecodeCpData      (T_CNI_RIL3SMS_MSG_CP_DATA       &);
T_CNI_RIL3_RESULT JCC_RIL3SMS_DecodeCpAck       (T_CNI_RIL3SMS_MSG_CP_ACK        &);
T_CNI_RIL3_RESULT JCC_RIL3SMS_DecodeCpError     (T_CNI_RIL3SMS_MSG_CP_ERROR      &);
T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeReleaseComplete      (T_CNI_RIL3CC_MSG_RELEASE_COMPLETE        &);
T_CNI_RIL3_RESULT JCC_RIL3MM_DecodeLocationUpdateRequest(T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REQUEST &);
T_CNI_RIL3_RESULT JCC_RIL3MM_DecodeCMServiceRequest     (T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST      &);


T_CNI_RIL3_IE_MOBILE_ID testIMSI[CC_MAX_CC_CALLS_DEF_VAL];

short testIMSIindex;

short origSoftMobDialedNumIndex;

T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER testDialedNum[CC_MAX_CC_CALLS_DEF_VAL];

T_CNI_IRT_ID softMobEntryId[CC_MAX_CC_CALLS_DEF_VAL] 
= {200, 201, 202, 203, 204, 205, 206};

T_CNI_LAPDM_OID softMobLapdmOid[CC_MAX_CC_CALLS_DEF_VAL]
= {200, 201, 202, 203, 204, 205, 206};

int termSoftMobIndex = 1,
    origSoftMobIndex = 0;
 
// The only message that needs any data right now is the Called Party Number


T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeAlerting(
	T_CNI_RIL3CC_MSG_ALERTING &alerting
					) 
{
		return CNI_RIL3_RESULT_SUCCESS;
}

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeCallConfirmed(
	T_CNI_RIL3CC_MSG_CALL_CONFIRMED &callConfirmed
					) {
		return CNI_RIL3_RESULT_SUCCESS;
}

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeConnect(
	T_CNI_RIL3CC_MSG_CONNECT &connect
					) {
		return CNI_RIL3_RESULT_SUCCESS;
}	

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeConnectAck(
	T_CNI_RIL3CC_MSG_CONNECT_ACK &connectAck
					) {
		return CNI_RIL3_RESULT_SUCCESS;
}

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeSetup(
	T_CNI_RIL3CC_MSG_SETUP		&setup 
					) {

  setup.calledBCD = testDialedNum[origSoftMobDialedNumIndex];

  return CNI_RIL3_RESULT_SUCCESS;
}

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeDisconnect(
	T_CNI_RIL3CC_MSG_DISCONNECT		&disconnect
					) {
		return CNI_RIL3_RESULT_SUCCESS;
}	

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeRelease(
	T_CNI_RIL3CC_MSG_RELEASE		&release
					) {
		return CNI_RIL3_RESULT_SUCCESS;	
}

T_CNI_RIL3_RESULT JCC_RIL3CC_DecodeReleaseComplete(
	T_CNI_RIL3CC_MSG_RELEASE_COMPLETE		&releaseComplete
					) {
		return CNI_RIL3_RESULT_SUCCESS;	
}

T_CNI_RIL3_RESULT JCC_RIL3MM_DecodeLocationUpdateRequest(
	T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REQUEST &locationUpdateRequest
					) {
  // mandatory IEs

  //T_CNI_RIL3_IE_LOCATION_UPDATE_TYPE		
  locationUpdateRequest.locationUpdateType.locationUpdateType = CNI_RIL3_LOCATION_UPDATE_TYPE_NORMAL;

  // 10.5.3.5
  // T_CNI_RIL3_IE_CIPHER_KEY				
  locationUpdateRequest.cipherKey.keySeq = CNI_RIL3_NO_KEY_AVAILABLE;

  // 10.5.1.2
  // T_CNI_RIL3_IE_LOCATION_AREA_ID			
  locationUpdateRequest.locationId.mcc[0] = 2 ;
  locationUpdateRequest.locationId.mcc[1] = 1 ;
  locationUpdateRequest.locationId.mcc[2] = 2 ;

  locationUpdateRequest.locationId.mnc[0] = 1;
  locationUpdateRequest.locationId.mnc[1] = 3;

  locationUpdateRequest.locationId.lac = 5 ;

  // 10.5.1.3
  // T_CNI_RIL3_IE_MS_CLASSMARK_1			
  locationUpdateRequest.classmark1.revLevel = CNI_RIL3_REV_LEVEL_PHASE1;
  locationUpdateRequest.classmark1.esInd = CNI_RIL3_EARLY_CLASSMARK_SENDING_NOT_IMPLEMENTED;
  locationUpdateRequest.classmark1.a51 = CNI_RIL3_A51_AVAILABLE;
  locationUpdateRequest.classmark1.rfPowerCap = CNI_RIL3_GSM_CLASS_1;

  // 10.5.1.5
  // T_CNI_RIL3_IE_MOBILE_ID					
  locationUpdateRequest.mobileId = testIMSI[testIMSIindex];

  // 10.5.1.4
		
  return CNI_RIL3_RESULT_SUCCESS;	
}

T_CNI_RIL3_RESULT JCC_RIL3MM_DecodeCMServiceRequest(
	T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST &cmServiceRequest
					) {
  // mandatory IEs
  
  // T_CNI_RIL3_IE_CM_SERVICE_TYPE	
  cmServiceRequest.cmServiceType.serviceType = 
    CNI_RIL3_CM_SERVICE_TYPE_MO_CALL_OR_PACKET_MODE;
  // 10.5.3.3

  // T_CNI_RIL3_IE_CIPHER_KEY		
  cmServiceRequest.cipherKey.keySeq = CNI_RIL3_NO_KEY_AVAILABLE;
  // 10.5.1.2

  // T_CNI_RIL3_IE_MS_CLASSMARK_2	
  cmServiceRequest.classmark2.revLevel = CNI_RIL3_REV_LEVEL_PHASE1;
  cmServiceRequest.classmark2.esInd = CNI_RIL3_EARLY_CLASSMARK_SENDING_NOT_IMPLEMENTED;
  cmServiceRequest.classmark2.a51 = CNI_RIL3_A51_AVAILABLE;
  cmServiceRequest.classmark2.rfPowerCap = CNI_RIL3_GSM_CLASS_1;
  cmServiceRequest.classmark2.psCap = CNI_RIL3_PS_CAPABILITY_NOT_PRESENT;
  cmServiceRequest.classmark2.ssScreening = CNI_RIL3_SS_SCREENING_PHASE1_DEFAULT;
  cmServiceRequest.classmark2.smCap = CNI_RIL3_SM_MT_PTP_NOT_SUPPORTED;
  cmServiceRequest.classmark2.fcFreqCap = CNI_RIL3_GSM_EXTENTION_BAND_G1_NOT_SUPPORTED;
  cmServiceRequest.classmark2.classmark3 = CNI_RIL3_CLASSMARK3_INFO_NOTAVAILABLE;

  /* Classmark3 not available
  cmServiceRequest.classmark2.cmsp = 0;
  cmServiceRequest.classmark2.a53 = 0;
  cmServiceRequest.classmark2.a52 = 0;
  */
  // 10.5.1.6

  // T_CNI_RIL3_IE_MOBILE_ID			
  cmServiceRequest.mobileId = testIMSI[testIMSIindex];
  // 10.5.1.4
	
  return CNI_RIL3_RESULT_SUCCESS;	
}


// SMS-PP CP-messages
T_CNI_RIL3_RESULT JCC_RIL3SMS_DecodeCpData  (T_CNI_RIL3SMS_MSG_CP_DATA &cp_data  ) 
{
	return CNI_RIL3_RESULT_SUCCESS;
}


T_CNI_RIL3_RESULT JCC_RIL3SMS_DecodeCpAck   (T_CNI_RIL3SMS_MSG_CP_ACK  &cp_ack   )
{
	return CNI_RIL3_RESULT_SUCCESS;
}


T_CNI_RIL3_RESULT JCC_RIL3SMS_DecodeCpError (T_CNI_RIL3SMS_MSG_CP_ERROR &cp_error)
{
	return CNI_RIL3_RESULT_SUCCESS;
}


T_CNI_RIL3_RESULT 
JCC_RIL3CC_Decode(
        bool isSoftMob,
	T_CNI_LAPDM_L3MessageUnit *l3MsgUnit, T_CNI_RIL3CC_MSG *ccMsg) 
{
	if (isSoftMob == false)
          return (CNI_RIL3CC_Decode(l3MsgUnit,ccMsg));

	unsigned char *buffer = l3MsgUnit->buffer;
	int length = l3MsgUnit->msgLength;

	if (length < 2) return CNI_RIL3_INCORRECT_LENGTH;

	// zap the output ccMsg	
	CNI_RIL3_ZAPMSG(ccMsg, sizeof(T_CNI_RIL3CC_MSG));

	// get skip indicator
	ccMsg->header.si_ti = 
		(T_CNI_RIL3_SI_TI) (buffer[0] >> 4);

	// get message type
	ccMsg->header.message_type = 
		(T_CNI_RIL3_MESSAGE_TYPE) (buffer[1] & 0x3f);

	switch (ccMsg->header.message_type)
	{
	case CNI_RIL3CC_MSGID_ALERTING:
		return JCC_RIL3CC_DecodeAlerting(
					ccMsg->alerting
					);	
	case CNI_RIL3CC_MSGID_CALL_CONFIRMED:
		return JCC_RIL3CC_DecodeCallConfirmed(
					ccMsg->callConfirmed
					);	
	case CNI_RIL3CC_MSGID_CONNECT:
		return JCC_RIL3CC_DecodeConnect(
					ccMsg->connect
					);	
	case CNI_RIL3CC_MSGID_CONNECT_ACKNOWLEDGE:
		return JCC_RIL3CC_DecodeConnectAck(
					ccMsg->connectAck
					);	
	case CNI_RIL3CC_MSGID_EMERGENCY_SETUP:
		return CNI_RIL3_RESULT_SUCCESS;	
	case CNI_RIL3CC_MSGID_SETUP:
		return JCC_RIL3CC_DecodeSetup(
					ccMsg->setup
					);	
	case CNI_RIL3CC_MSGID_MODIFY:
	case CNI_RIL3CC_MSGID_MODIFY_COMPLETE:
	case CNI_RIL3CC_MSGID_MODIFY_REJECT:
		return CNI_RIL3_RESULT_SUCCESS;	
	case CNI_RIL3CC_MSGID_DISCONNECT:
		return JCC_RIL3CC_DecodeDisconnect(
					ccMsg->disconnect
					);		
	case CNI_RIL3CC_MSGID_RELEASE:
		return JCC_RIL3CC_DecodeRelease(
					ccMsg->release
					);			
	case CNI_RIL3CC_MSGID_RELEASE_COMPLETE:
		return JCC_RIL3CC_DecodeReleaseComplete(
					ccMsg->releaseComplete
					);		
	case CNI_RIL3CC_MSGID_CONGESTION_CONTROL:
	case CNI_RIL3CC_MSGID_NOTIFY:
	case CNI_RIL3CC_MSGID_STATUS:
	case CNI_RIL3CC_MSGID_STATUS_ENQUIRY:
	default:
		return CNI_RIL3_UNKNOWN_MESSAGE_TYPE;
	}
  return CNI_RIL3_RESULT_SUCCESS;
}


T_CNI_RIL3_RESULT 
JCC_RIL3MM_Decode(
        bool isSoftMob,
	T_CNI_LAPDM_L3MessageUnit *l3MsgUnit, T_CNI_RIL3MM_MSG *mmMsg) 
{
	if (isSoftMob == false)
          return (CNI_RIL3MM_Decode(l3MsgUnit,mmMsg));

	unsigned char *buffer = l3MsgUnit->buffer;
	int length = l3MsgUnit->msgLength;

	if (length < 2) return CNI_RIL3_INCORRECT_LENGTH;

	// zap the output mmMsg	
	CNI_RIL3_ZAPMSG(mmMsg, sizeof(T_CNI_RIL3MM_MSG));

	// get message type
	mmMsg->header.message_type = 
		(T_CNI_RIL3_MESSAGE_TYPE) (buffer[1] & 0x3f);

	switch (mmMsg->header.message_type)
	{
	case CNI_RIL3MM_MSGID_IMSI_DETACH_INDICATION:
	case CNI_RIL3MM_MSGID_LOCATION_UPDATING_REQUEST:
		return JCC_RIL3MM_DecodeLocationUpdateRequest(
					mmMsg->locationUpdateRequest
					);
	case CNI_RIL3MM_MSGID_AUTHENTICATION_RESPONSE:
	case CNI_RIL3MM_MSGID_IDENTITY_RESPONSE:
	case CNI_RIL3MM_MSGID_TMSI_REALLOCATION_COMPLETE:
		return CNI_RIL3_RESULT_SUCCESS;	

	case CNI_RIL3MM_MSGID_CM_SERVICE_ABORT:
		return CNI_RIL3_RESULT_SUCCESS;	

	case CNI_RIL3MM_MSGID_CM_SERVICE_REQUEST:
		return JCC_RIL3MM_DecodeCMServiceRequest(
					mmMsg->cmServiceRequest
					);
	case CNI_RIL3MM_MSGID_CM_REESTABLISHMENT_REQUEST:
	case CNI_RIL3MM_MSGID_MM_NULL:
	default:
		return CNI_RIL3_UNKNOWN_MESSAGE_TYPE;
	}
  return CNI_RIL3_RESULT_SUCCESS;
}


T_CNI_RIL3_RESULT JCC_RIL3SMS_Decode (bool                       isSoftMob,
									  T_CNI_LAPDM_L3MessageUnit *l3MsgUnit, 
									  T_CNI_RIL3SMS_CP_MSG      *smsMsg   ) 
{
  T_CNI_RIL3_RESULT retVal;

	if (isSoftMob == false)
    {
		retVal = CNI_RIL3SMS_CP_Decode (l3MsgUnit, smsMsg); //Real Mode
    }
    else
    {
	    // Test mode
	    unsigned char *buffer = l3MsgUnit->buffer   ;
	    int            length = l3MsgUnit->msgLength;

	    if (length < 2) 
		    retVal = CNI_RIL3_INCORRECT_LENGTH;
        else
        {
    	    // zap the output ccMsg	
        	CNI_RIL3_ZAPMSG (smsMsg, sizeof(T_CNI_RIL3SMS_CP_MSG));

	        // get skip indicator
	        smsMsg->header.si_ti = (T_CNI_RIL3_SI_TI) (buffer[0] >> 4);

	        // get message type
	        smsMsg->header.message_type = (T_CNI_RIL3_CP_MESSAGE_TYPE) (buffer[1] & 0x3f);

	        switch (smsMsg->header.message_type)
	        {
	        case CNI_RIL3SMS_MSGID_CP_DATA :
        		retVal = JCC_RIL3SMS_DecodeCpData  (smsMsg->cp_data );
                break;

	        case CNI_RIL3SMS_MSGID_CP_ACK  :
		        retVal = JCC_RIL3SMS_DecodeCpAck   (smsMsg->cp_ack  );
                break;

    	    case CNI_RIL3SMS_MSGID_CP_ERROR:
	    	    retVal = JCC_RIL3SMS_DecodeCpError (smsMsg->cp_error);
                break;

	        default:
		        retVal = CNI_RIL3_UNKNOWN_MESSAGE_TYPE;
                break;
            }
        }
    }

  return (retVal);
}


