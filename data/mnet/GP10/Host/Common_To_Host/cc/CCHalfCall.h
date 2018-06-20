#ifndef CCHalfCall_H
#define CCHalfCall_H

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
// File        : CCHalfCall.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (MM, RR)
#include "JCC/JCCL3Msg.h"

#include "JCC/JCCEvent.h"
#include "JCC/JCCState.h"
#include "JCC/JCCTimer.h"
#include "JCC/JCCLog.h"

#include "logging/vclogging.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"
#include "ril3/ril3_cc_msg.h"

#include "CC/CCTypes.h"

//BCT <xxu:09-22-00>
#include "csunew/csu_head.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

class TwoPartyCallLeg;

// *******************************************************************
// Constant definitions.
// *******************************************************************

// 4.07 Transaction Identifier
const UINT8 ORIG_OR_MASK = 0x08;
const UINT8 TERM_AND_MASK = 0x07;

const int MAX_GENERATED_EVENTS       
= (CC_MAKE_CONNECTION_REQ - CC_INTERNAL_GEN_EVENT_BASE + 1);

const int MAX_CALL_STATES 
= (THC_ST_MM_CONN_REQ + 1);

const int MAX_CALL_EXTERNAL_EVENTS    
= (THC_REMOTE_TERM_ADDRESS - CC_INTERNAL_CALL_EVENT_BASE + 1);

const int MAX_CHANN_STATES         
= (RES_ST_REL_REQ + 1);

const int MAX_CHANN_EXTERNAL_EVENTS   
= (CC_RESOURCE_ASSIGN_FAIL - CC_INTERNAL_CHANN_EVENT_BASE + 1);

//typedefs

class CCHalfCall
{
 public:

  // Following covered in CCHalfCall.cpp

  CCHalfCall();  

  virtual void                cleanup(void);
  void                        printData(JCCPrintStFn);

  JCCEvent_t                  handleInvalidEvent(void);

  JCCEvent_t                  handleMobProtocolProblem(void);

  int                         handleMMMsg(IntraL3Msg_t    *mmInMsg,
                              JCCEvent_t      currEvent);
  int                         handleRRMsg(IntraL3Msg_t    *rrInMsg,
					          JCCEvent_t      currEvent);

  int                         handleMobMsg(T_CNI_RIL3CC_MSG  *msInMsg,
                                           T_CNI_RIL3_RESULT msDecodeResult,
                                           T_CNI_RIL3_CAUSE_VALUE     causeValue,
                                           JCCEvent_t        currEvent);

  int                         handleTimeoutMsg(IntraL3Msg_t       *ccInMsg);
  int                         handleRemoteEvent(IntraL3Msg_t* h323InMsg, JCCEvent_t       remoteEvent,
						      CCRemMsgData_t   inRemMsg);

  int                         smHandleEvent(void);
  int                         smHandleFirstCallEvent(void);

  virtual bool                sendRemote(JCCEvent_t remoteEvent);

 public:
  // Remote Handlers in CCMobRemHnd.cpp

  //BCT <xxu:09-22-00> BEGIN
  JCCEvent_t		      handleBctNumberReqTimeout(void);
  JCCEvent_t                  handleGetBctNumberNack(void);
  JCCEvent_t		      handleBctUpdateHandles(void);
  JCCEvent_t		      handleMobRemoteAlerting2nd(void);
  JCCEvent_t		      handleMobRemoteChannSelected2nd(void);
  JCCEvent_t		      handleMobRemoteAnswer2nd(void);
  //BCT <xxu:09-22-00> END

  JCCEvent_t                  handleMobRemoteDisconnect(void);
  JCCEvent_t                  handleMobRemoteAbnormalRelCom(void);
  JCCEvent_t                  handleMobRemoteAlerting(void);
  JCCEvent_t                  handleMobRemoteAnswer(void);
  JCCEvent_t                  handleMobRemoteCallProgress(void);
  JCCEvent_t                  handleMobRemoteCallProgress1(void);
  JCCEvent_t                  handleMobRemoteTermAddress(void);
  JCCEvent_t                  handleMobRemoteH323ChannReady(void);

  // External Mobile Handlers in CCMobExtHnd.cpp
  JCCEvent_t                  handleMobAlerting(void);
  JCCEvent_t                  handleMobCallProgress(void);
  JCCEvent_t                  handleMobAnswer(void);
  JCCEvent_t                  handleConnectAck(void);
  JCCEvent_t                  handleMobTermAddress(void);
  JCCEvent_t                  handleMobEmergencyOrigination(void);
  JCCEvent_t                  handleDisconnectReq(void);
  JCCEvent_t                  handleReleaseReq(void);
  JCCEvent_t                  handleMobReleaseComplete(void);
   
  //CAUSE<xxu:03-21-00> BEGIN
  JCCEvent_t                  handleMobReleaseCompleteUtil(void);
  //CAUSE<xxu:03-21-00> END

  JCCEvent_t                  handleMobStatusInfo(void);
  JCCEvent_t                  handleMobStatusEnquiry(void);

  JCCEvent_t                  handleMobNoReply(void);

  // Internal/Generated Event Handlers in CCMobIntHnd.cpp
  JCCEvent_t                  handleNullEvent(void);

  JCCEvent_t                  handleInvalidChannEvent(void);
  JCCEvent_t                  handleChannAssignReq(void);
  JCCEvent_t                  handleChannAssignComplt(void);
  JCCEvent_t                  handleChannAssignFail(void);

  JCCEvent_t                  handleConnReq(void);

  //CH<xxu:Mod 12-13-99>
  JCCEvent_t                  handleConnComplt(void);
  JCCEvent_t                  handleConnFail(void);

  JCCEvent_t                  handleDisconnectTimeout(void);
  JCCEvent_t                  handleReleaseTimeout(void);

  JCCEvent_t                  handleMobResRelease(void);

  JCCEvent_t                  handleMMRelInd(void);
  JCCEvent_t                  handleMMEstCnf(void);
  JCCEvent_t                  handleMMEstRej(void);

  // DTMF Handlers
  JCCEvent_t                  handleStartDtmf(void);
  JCCEvent_t                  handleStopDtmf(void);

  //CH<xxu:11-10-99> Call Hold & Retrieve & Swap handlers
  JCCEvent_t                  handleHoldAct();
  JCCEvent_t                  handleHoldRej();
  JCCEvent_t                  handleRetvAct();
  JCCEvent_t                  handleRetvRej();
  JCCEvent_t                  handleHoldReq();
  JCCEvent_t                  handleRetvReq(); //CH<xxu:Mod 12-13-99>
  JCCEvent_t                  handleSwapTimerExpiry();

  // Chann Selection Handler
  JCCEvent_t                  handleMobRemoteChannSelected(void);


 private:
  // Message Builders in CCBldMSMsg.cpp
  void                        buildMSMsgCommon(void);

  T_CNI_RIL3_RESULT           buildMSMsg(T_CNI_LAPDM_L3MessageUnit	
					 *msEncodedMsg);

  T_CNI_RIL3_RESULT           buildStartDtmfAck(T_CNI_LAPDM_L3MessageUnit	
                                                *msEncodedMsg);

  T_CNI_RIL3_RESULT           buildStartDtmfRej(T_CNI_LAPDM_L3MessageUnit	
                                                *msEncodedMsg);

  T_CNI_RIL3_RESULT           buildStopDtmfAck(T_CNI_LAPDM_L3MessageUnit	
                                               *msEncodedMsg);

  T_CNI_RIL3_RESULT           buildAlerting(T_CNI_LAPDM_L3MessageUnit
					                          *msEncodedMsg);
  T_CNI_RIL3_RESULT 
                           buildProgress(T_CNI_LAPDM_L3MessageUnit     *msEncodedMsg);

  T_CNI_RIL3_RESULT           buildCallProceeding(T_CNI_LAPDM_L3MessageUnit	
						  *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildConnect(T_CNI_LAPDM_L3MessageUnit	
					   *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildConnectAck(T_CNI_LAPDM_L3MessageUnit	
					      *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildDisconnect(T_CNI_LAPDM_L3MessageUnit	
					      *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildReleaseComplete(T_CNI_LAPDM_L3MessageUnit	
						   *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildReleaseReq(T_CNI_LAPDM_L3MessageUnit	
					      *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildSetup(T_CNI_LAPDM_L3MessageUnit	
					 *msEncodedMsg);
  T_CNI_RIL3_RESULT           buildStatusInfo(T_CNI_LAPDM_L3MessageUnit	
                                              *msEncodedMsg);

  //CH<xxu:11-10-99>
  T_CNI_RIL3_RESULT 		buildHoldAck(T_CNI_LAPDM_L3MessageUnit
					     *msEncodedMsg);
  T_CNI_RIL3_RESULT 		buildHoldRej(T_CNI_LAPDM_L3MessageUnit
					     *msEncodedMsg);
  T_CNI_RIL3_RESULT 		buildRetvAck(T_CNI_LAPDM_L3MessageUnit
					     *msEncodedMsg);
  T_CNI_RIL3_RESULT 		buildRetvRej(T_CNI_LAPDM_L3MessageUnit
					     *msEncodedMsg);

  // Other Class and data members start

 protected:

  virtual void                initData(void);

  // Save MS message data for use
  //T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER
  //  cpn;
  //T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER  
  //  callingPartyNum;

  short                       releaseCounter;
  
  // For building data for remote events
  CCRemMsgData_t              remMsgData;

  T_CNI_RIL3_RESULT           msOutResult;
  T_CNI_RIL3CC_MSG            msOutMsg;

  IntraL3Msg_t                * mmInMsg;
  IntraL3Msg_t                * rrInMsg;
  IntraL3Msg_t				  * h323InMsg;

 public:

  // Save MS message data for use
  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER
    cpn;
  T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER  
    callingPartyNum;

  //CAUSE<xxu:03-21-00> moved here from private
  T_CNI_RIL3_CAUSE_VALUE    voipReleaseCause_;

  //BCT <xxu:09-23-00> BEGIN
  AnnStates_t		    annState_;
  T_CSU_PORT_ID		    annSrcPort_;
  T_CSU_PORT_ID		    annSnkPort_;
  //BCT <xxu:08-23-00> END

  //CH<xxu:11-10-99>
  ChStates_t		      chState_;
  T_CNI_RIL3_CAUSE_VALUE      chCause_;

  JCCTimer                    *hcTimer;  //CH<xxu:12-06-99>

  HalfCallStates_t            callState_ ;

  ResStates_t                 channState_;
  
  SpeechChann_t               currSpeechCh_; 

  T_CNI_RIL3_SI_TI            ti;

  T_CNI_RIL3_IE_CAUSE         rlsCause;

  DbgOutput                   Dbg;

  T_CNI_RIL3_FIELD_SPEECH_VERSION_INDICATOR  bearCap1speechVersionInd_[3];

 protected:
  T_CNI_RIL3CC_MSG	      *msDecodedMsg;

  TwoPartyCallLeg             *parent;

  JCCEvent_t                  currEvent;

  bool                        isEmergencyCall;
 private:
  T_CNI_RIL3_RESULT	      msDecodeResult;

  T_CNI_RIL3_IE_BEARER_CAPABILITY 
    bearerCap;


  T_CNI_RIL3_IE_KEYPAD_FACILITY 
    currKeypadFacility_;

  //CAUSE<xxu:03-21-00> moved to public
  //MOBILE_CALL_DISCONNECT_CAUSE
  //T_CNI_RIL3_CAUSE_VALUE
  //  voipReleaseCause_;

  bool alertingSent_;
  bool connectSent_;
  bool progressSent_;

  ULONG setupTime_;

 private:

  // hide the assignment, and copy ctor and other defaults as needed.

  CCHalfCall (const CCHalfCall& rhs) ;
  
  CCHalfCall&  operator= (const CCHalfCall& rhs) ;

  int               operator==(const CCHalfCall& rhs) const ;

};

class CCOrigHalfCall : public CCHalfCall
{
 public:
  //PR1378 <xxu:08-16-00>
  CCOrigHalfCall(TwoPartyCallLeg *callLeg, JCCTimer *tclTimer);
  void                        setupNewCall(T_CNI_RIL3_SI_TI            ti = 0,
                                           T_CNI_RIL3_CM_SERVICE_TYPE  serviceRequested = CNI_RIL3_CM_SERVICE_TYPE_MO_CALL_OR_PACKET_MODE);

 protected:
  virtual void                initData(void);

 private:

  // hide the assignment, and copy ctor and other defaults as needed.
  CCOrigHalfCall();  

  CCOrigHalfCall (const CCOrigHalfCall& rhs) ;
  
  CCOrigHalfCall&  operator= (const CCOrigHalfCall& rhs) ;

  int               operator==(const CCOrigHalfCall& rhs) const ;

};

class CCTermHalfCall : public CCHalfCall
{
 public:
  //PR1378 <xxu:08-16-00>
  CCTermHalfCall(TwoPartyCallLeg *callLeg, JCCTimer *tclTimer);

 protected:
  virtual void                initData(void);

 private:

  // hide the assignment, and copy ctor and other defaults as needed.
  CCTermHalfCall();  

  CCTermHalfCall (const CCTermHalfCall& rhs) ;
  
  CCTermHalfCall&  operator= (const CCTermHalfCall& rhs) ;

  int               operator==(const CCTermHalfCall& rhs) const ;
}; 

// pointer to member type
typedef JCCEvent_t (CCHalfCall::* HalfCallMsgHandler_t) (); 

extern HalfCallMsgHandler_t channStateHandler[MAX_CHANN_STATES][MAX_CHANN_EXTERNAL_EVENTS];
extern HalfCallMsgHandler_t generatedEventHandler[MAX_GENERATED_EVENTS];
extern HalfCallMsgHandler_t callStateHandler[MAX_CALL_STATES][MAX_CALL_EXTERNAL_EVENTS];

#endif                                       // CCHalfCall_H

