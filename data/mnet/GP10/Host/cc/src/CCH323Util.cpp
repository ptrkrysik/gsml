// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CCH23Util.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 02-01-98
// Description : CC (VOIP)H323 side utilities  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "CC/CCH323Util.h"

#include "logging/VCLOGGING.h"

#include "JCC/JCCLog.h"

#include "taskLib.h"

#include "CC/CallLeg.h"

//ext-HO <chenj:06-06-11>
#include "CC/HOCallLeg.h"

//CDR <xxu:08-21-00> BEGIN
#include "JCC/LUDBapi.h"
#include "CDR/CdrVoiceCall.h"
#include "CDR/CdrSSA.h"
extern BtsBasicPackage        ccBtsBasicPackage;

void gCaptureSetupCdr( IntraL3Msg_t      *h323InMsg,
                       CdrRecCauseTerm_t cdrCause,
                       T_CNI_RIL3_CAUSE_VALUE ccCause );

bool
    sendH323DisconnectMsg(
              IntraL3Msg_t *h323InMsg,
              CdrRecCauseTerm_t cdrCause,
              T_CNI_RIL3_CAUSE_VALUE ccCause )
    {
      DBG_FUNC("sendH323DisconnectMsg", CC_LAYER);
      DBG_ENTER();
    
      VOIP_API_MESSAGE voipOutMsg;
    
      bool retValue;
    
      voipOutMsg.Magic = VOIP_API_MAGIC_VALUE;
      
      voipOutMsg.MessageType = VOIP_API_CALL_RELEASE;
    
      voipOutMsg.CallRelease.cause = ccCause;
      
      DBG_TRACE("CC->H323 LOG: Sending Disconnect Msg reason(%d) h323CallHandle(%d) ludbId(%d)\n",
                 ccCause,h323InMsg->call_handle,
                 h323InMsg->entry_id);
    
      voipOutMsg.VoipCallHandle = h323InMsg->call_handle;
    
      voipOutMsg.LudbId = h323InMsg->entry_id;
    
      // Eventhough the Transaction Id is not used in this case, fill in this 
      // parameter with the NULL value to make it valid.
      voipOutMsg.TxnId = VOIP_NULL_CALL_HANDLE;
    
      retValue = VoipApiMessageHandler(&voipOutMsg);
    
      // Capture CDR data for all MTC landed in GP
      gCaptureSetupCdr(h323InMsg, cdrCause, ccCause);
    
      DBG_LEAVE();
      return (retValue);
    }
    //CDR <xxu:08-21-00> END
    
    bool
    sendH323Msg(TwoPartyCallLeg         *parent,
                JCCEvent_t      remoteEvent, 
                CCRemMsgData_t  remMsgData)
    {
      DBG_FUNC("sendH323Msg", CC_LAYER);
      DBG_ENTER();
    
      VOIP_API_MESSAGE voipOutMsg;
    
      bool retValue;
    
      voipOutMsg.Magic = VOIP_API_MAGIC_VALUE;
      
      voipOutMsg.TxnId = parent->callIndex;
    
      voipOutMsg.LudbId = parent->ludbIndex();
    
      voipOutMsg.VoipCallHandle = parent->h323CallHandle;
    
      switch (remoteEvent)
        {
        case HC_REMOTE_DTMF_REQ:
          voipOutMsg.MessageType = VOIP_API_CALL_DTMF;
          voipOutMsg.CallDtmf.Digit = (VOIP_DTMF_DIGIT) (DTMF_DIGIT_0 + remMsgData.dtmfDigit);
          voipOutMsg.CallDtmf.Duration = 15;
          break;
              
        case THC_REMOTE_TERM_ADDRESS:
    
          voipOutMsg.MessageType = VOIP_API_CALL_SETUP;
          voipOutMsg.CallSetup = remMsgData.setup;
    
          //          DBG_TRACE("Call Log: called party number sent to Voip:\n");
          //          DBG_TRACE("(numDigits=%d), (%d), (%d), (%d), (%d), (%d)\n",
          //                            voipOutMsg.CallSetup.CalledPartyNumber.numDigits,
          //                            voipOutMsg.CallSetup.CalledPartyNumber.digits[0],
          //                            voipOutMsg.CallSetup.CalledPartyNumber.digits[1],
          //                            voipOutMsg.CallSetup.CalledPartyNumber.digits[2],
          //                            voipOutMsg.CallSetup.CalledPartyNumber.digits[3],
          //                            voipOutMsg.CallSetup.CalledPartyNumber.digits[4]
          //                            );
    
          break;
          
        case HC_REMOTE_CALL_PROGRESS:
          voipOutMsg.MessageType = VOIP_API_CALL_PROCEEDING;
          voipOutMsg.CallProceeding = remMsgData.callProceeding;
          break;
    
        case OHC_REMOTE_ALERTING:
          voipOutMsg.MessageType = VOIP_API_CALL_ALERTING;
          break;
          
        case OHC_REMOTE_ANSWER:
          voipOutMsg.MessageType = VOIP_API_CALL_ANSWER;
          break;
          
        case HC_REMOTE_DISCONNECT_REQ:
        case HC_REMOTE_RELEASE_COMPLETE: 
          // To avoid sending the release a second time to H323
    
          if (! (parent->disconnectSentToVoip))
            {
              parent->disconnectSentToVoip = true;
              voipOutMsg.MessageType = VOIP_API_CALL_RELEASE;
              voipOutMsg.CallRelease = remMsgData.callRelease;
              JCCLog1("CC->Voip: Disconnect (reason = %d)\n",
                      voipOutMsg.CallRelease.cause);
              DBG_TRACE("CC->Voip: Disconnect (reason = %d)\n",
                        voipOutMsg.CallRelease.cause);
    
              // The following is needed when the Voip side starts handling 
              // Disconnect right after a setup!!
            }
          else
            {
              // Do Nothing
              DBG_LEAVE();
              return (true);
            }
          break;
    
        //BCT <xxu:07-11-00> BEGIN
        case OHC_REMOTE_BCT_INVOKE: 
             voipOutMsg.MessageType  = VOIP_API_CALL_TRANSFER;
             voipOutMsg.CallTransfer = remMsgData.callTransfer;
             break;
        //BCT <xxu:07-11-00> END
    
        default:
          DBG_ERROR("CC->H323 ERROR: Unexpected (Remote Event = %d) from GSM CC.\n",  
                     remoteEvent);
          DBG_LEAVE();
          return (false);
         
        }
    
      DBG_TRACE("CC->H323 LOG: Sending Msg(%d) TxnId(%d) h323CallHandle(%d) ludbId(%d) reEv(%d) callLeg(%d) entryId(%d)\n",
                 voipOutMsg.MessageType,
                 voipOutMsg.TxnId,
                 voipOutMsg.VoipCallHandle,
                 voipOutMsg.LudbId,
  		 remoteEvent,
  		 parent->callLegNum,
  		 parent->entryId() );
     
      retValue = VoipApiMessageHandler(&voipOutMsg);
    
      DBG_LEAVE();
      return (retValue);
    }

    // <ext-HO <chenj:06-11-01>
    bool
    sendH323MsgExtHo(HOCallLeg         *parent,
                     JCCEvent_t        remoteEvent, 
                     CCRemMsgData_t    remMsgData,
                     unsigned short    callLegId)
    {
      DBG_FUNC("sendH323Msg", CC_LAYER);
      DBG_ENTER();
    
      VOIP_API_MESSAGE voipOutMsg;
    
      bool retValue;
    
      voipOutMsg.Magic = VOIP_API_MAGIC_VALUE;
      
      voipOutMsg.TxnId = ( (callLegId << 8) | 
                           (parent->callIndex & 0xFF) );
    
      voipOutMsg.LudbId = parent->ludbIndex(); 
    
      voipOutMsg.VoipCallHandle = parent->h323CallHandle;
    
      switch (remoteEvent)
        {
        case HC_REMOTE_DTMF_REQ:
          voipOutMsg.MessageType = VOIP_API_CALL_DTMF;
          voipOutMsg.CallDtmf.Digit = (VOIP_DTMF_DIGIT) (DTMF_DIGIT_0 + remMsgData.dtmfDigit);
          voipOutMsg.CallDtmf.Duration = 15;
          break;
              
        case THC_REMOTE_TERM_ADDRESS:
    
          voipOutMsg.MessageType = VOIP_API_CALL_SETUP;
          voipOutMsg.CallSetup = remMsgData.setup;
    
          break;
          
        case HC_REMOTE_CALL_PROGRESS:
          voipOutMsg.MessageType = VOIP_API_CALL_PROCEEDING;
          voipOutMsg.CallProceeding = remMsgData.callProceeding;
          break;
    
        case OHC_REMOTE_ALERTING:
          voipOutMsg.MessageType = VOIP_API_CALL_ALERTING;
          break;
          
        case OHC_REMOTE_ANSWER:
          voipOutMsg.MessageType = VOIP_API_CALL_ANSWER;
          break;
          
        case HC_REMOTE_DISCONNECT_REQ:
        case HC_REMOTE_RELEASE_COMPLETE: 
          // To avoid sending the release a second time to H323
    
          if (! (parent->disconnectSentToVoip))
            {
              parent->disconnectSentToVoip = true;
              voipOutMsg.MessageType = VOIP_API_CALL_RELEASE;
              voipOutMsg.CallRelease = remMsgData.callRelease;
              JCCLog1("CC->Voip: Disconnect (reason = %d)\n",
                      voipOutMsg.CallRelease.cause);
              DBG_TRACE("CC->Voip: Disconnect (reason = %d)\n",
                        voipOutMsg.CallRelease.cause);
    
              // The following is needed when the Voip side starts handling 
              // Disconnect right after a setup!!
            }
          else
            {
              // Do Nothing
              DBG_LEAVE();
              return (true);
            }
          break;
    
        //BCT <xxu:07-11-00> BEGIN
        case OHC_REMOTE_BCT_INVOKE: 
             voipOutMsg.MessageType  = VOIP_API_CALL_TRANSFER;
             voipOutMsg.CallTransfer = remMsgData.callTransfer;
             break;
        //BCT <xxu:07-11-00> END
    
        default:
          DBG_ERROR("CC->H323 ERROR: Unexpected (Remote Event = %d) from GSM CC.\n",  
                     remoteEvent);
          DBG_LEAVE();
          return (false);
         
        }
    
      DBG_TRACE("CC->H323 LOG: Sending Msg(%d) TxnId(%d) h323CallHandle(%d) ludbId(%d) reEv(%d) callLeg(%d) entryId(%d)\n",
                 voipOutMsg.MessageType,
                 voipOutMsg.TxnId,
                 voipOutMsg.VoipCallHandle,
                 voipOutMsg.LudbId,
  		 remoteEvent,
  		 parent->callLegNum,
  		 parent->entryId() );
     
      retValue = VoipApiMessageHandler(&voipOutMsg);
    
      DBG_LEAVE();
      return (retValue);
    }

    
    //CDR <xxu:08-23-00> BEGIN
    void gCaptureSetupCdr(
              IntraL3Msg_t *h323InMsg,
              CdrRecCauseTerm_t cdrCause,
              T_CNI_RIL3_CAUSE_VALUE ccCause )
    {
      CCdrVoiceCall  ccCdr;
    
      T_SUBSC_IE_ISDN *msisdn;
      short  ludbIndex = h323InMsg->entry_id;
    
      DBG_FUNC("msgHandlerInit", CC_LAYER);
      DBG_ENTER();
    
      // Capture setup cdr data
      if (ludbGetSubscInfoExists(ludbIndex))
      {
          ccCdr.setServedIMSI(ludbGetMobileIdPtr(ludbIndex));
          msisdn = ludbGetMSISDNPtr(ludbIndex);
          msisdn->num_plan[0] = 0x11;
          ccCdr.setServedMSISDN(msisdn);
          //ccCdr.setServedMSISDN(ludbGetMSISDNPtr(ludbIndex));
      }
    
      ccCdr.seisure_NormalTermination(0);
    
      ccCdr.setCallingPartyNumber( &h323InMsg->l3_data.voipMsg.setup.CallingPartyNumber );
      //ccCdr.setCalledPartyNumber( &h323InMsg->l3_data.voipMsg.setup.CalledPartyNumber );
    
      if ( ludbIsProvCW(ludbIndex) && ludbIsActiveCW(ludbIndex) && 
           (ccCause== CNI_RIL3_CAUSE_USER_BUSY) )
      {
            CCdrSSA  cwCdr;
            if (ludbGetSubscInfoExists(ludbIndex))
            {
                cwCdr.setServedIMSI(ludbGetMobileIdPtr(ludbIndex));
                msisdn = ludbGetMSISDNPtr(ludbIndex);
                msisdn->num_plan[0] = 0x11;
                cwCdr.setServedMSISDN(msisdn);
                //cwCdr.setServedMSISDN(ludbGetMSISDNPtr(ludbIndex));
            }
            cwCdr.setCorrelatedEventRefNumber(ccCdr.getEventRefNumber());
            cwCdr.generateRecord(CDR_SS_ACTION_INVOCATION, SUPP_SERV_CW, CNI_RIL3_CAUSE_USER_BUSY);
      }
    
      //Load up with Location area info
      T_CNI_RIL3_IE_LOCATION_AREA_ID lai;
      T_CNI_RIL3_IE_CELL_ID ci;
    
      ci.ie_present = true;
      ci.value = ((short)(ccBtsBasicPackage.bts_ci));
    
      lai.ie_present = true;
      lai.mcc[0] = ((char*)&(ccBtsBasicPackage.bts_mcc))[0];
      lai.mcc[1] = ((char*)&(ccBtsBasicPackage.bts_mcc))[1];
      lai.mcc[2] = ((char*)&(ccBtsBasicPackage.bts_mcc))[2];
      lai.mnc[0] = ((char*)&(ccBtsBasicPackage.bts_mnc))[0];
      lai.mnc[1] = ((char*)&(ccBtsBasicPackage.bts_mnc))[1];
      lai.lac    = ((short)(ccBtsBasicPackage.bts_lac));
    
      DBG_TRACE("mcc(%d,%d,%d) mnc(%d,%d) lac(%d) ci(%d)\n",
    		 lai.mcc[0],lai.mcc[1],lai.mcc[2],lai.mnc[0],lai.mnc[1],lai.lac,ci.value);
    
      DBG_TRACE("MCC(%d,%d,%d) MNC(%d,%d)\n",
    		((char*)&(ccBtsBasicPackage.bts_mcc))[0],
    		((char*)&(ccBtsBasicPackage.bts_mcc))[1],
    		((char*)&(ccBtsBasicPackage.bts_mcc))[2],
    		((char*)&(ccBtsBasicPackage.bts_mnc))[0],
    		((char*)&(ccBtsBasicPackage.bts_mnc))[1]);
    
      ccCdr.setLocationAreaCID(&lai, &ci);
    
      // Generate last CDR record
      ccCdr.release(ccCause, cdrCause);
    }
    //CDR <xxu:08-23-00> END


    //ext-HO <xxu:06-01-01>
    bool
    sendH323DisconnectMsgExtHo(VOIP_CALL_HANDLE callHandle, T_CNI_RIL3_CAUSE_VALUE ccCause)
	{
      DBG_FUNC("sendH323DisconnectMsgExtHo", CC_HO_LAYER);
      DBG_ENTER();
    
      VOIP_API_MESSAGE voipOutMsg;
    
      bool retValue;

      voipOutMsg.Magic = VOIP_API_MAGIC_VALUE;
      voipOutMsg.MessageType = VOIP_API_CALL_RELEASE;
    
      voipOutMsg.CallRelease.cause = ccCause;
      voipOutMsg.VoipCallHandle = callHandle;  
    
      voipOutMsg.LudbId = -1;
    
	  voipOutMsg.TxnId = VOIP_NULL_CALL_HANDLE;

      retValue = VoipApiMessageHandler(&voipOutMsg);

      DBG_TRACE("{\nMNEThoTRACE(sendH323DisconnectMsgExtHo): [==>VOIP DISCONNECT HANDOVER VOICE PATH] (hoCallHandle(%x), cause(%d)\n}\n",
		         callHandle, ccCause);
    
      DBG_LEAVE();
      return (retValue);
	}
