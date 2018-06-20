#ifndef CCSessionHandler_H
#define CCSessionHandler_H

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
// File        : CCSessionHandler.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "csunew/csu_head.h"
#include "CC/CCHandover.h"
#include "CC/CallLeg.h"
#include "CC/SmsLeg.h"
// EHO <chenj:05-15-01>
#include "CC/HOCallLeg.h"

#include "JCC/JCCEvent.h"
#include "JCC/JCCL3Msg.h"
#include "JCC/JCCTimer.h"

// message analyzer
#include "CCMsgAnal.h"

#include "SmQueue.h"

//CDR <xxu:08-24-00>
#include "ril3/ie_mobile_id.h"
// *******************************************************************
// forward declarations.
// *******************************************************************


// *******************************************************************
// Constant definitions.
// *******************************************************************


//typedefs

class CCSessionHandler
{

 public:

  // Destructor
  // ~CCSessionHandler(void) ;

  // Constructors
  CCSessionHandler(CCSession_t *session);  

  // Operators

  // primary behaviour methods

  //ext-HO <xxu:06-07-01>
  void						  setReleaseCause(T_CNI_RIL3_CAUSE_VALUE cause);

  //BCT <xxu:09-22-00>
  bool						  getBctArrival(void);

  void                        sendL2Msg(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg   );
  void                        sendDLMsg(L3L2MsgType_t    msgType = L3L2_DL_EST_REQ,
                                        T_CNI_LAPDM_SAPI sapi    = SAPI_SMS       );

  void                        sendRRMsg(IntraL3PrimitiveType_t     primType ,
                                        IntraL3MsgType_t           msgType  ,
                                        IntraL3Msg_t               *rrOutMsg);
  void                        sendMMMsg(IntraL3Msg_t               *mmOutMsg);

  T_CSU_PORT_ID               mobileCsuPort(void);

  bool                        setupOrig(T_CNI_IRT_ID irtId,
                                        T_CNI_RIL3_CM_SERVICE_TYPE cmServType);

  bool                        setupTerm(VOIP_CALL_HANDLE voipCallHandle,
                                        int              cdrIndex      ,
                                        int              callRefValue  ,
                                        bool             isFirst       );

  //ext-HO <xxu:05-16-01>
  bool						  setupTermExtHo(IntraL3Msg_t* h323InMsg);
  bool                        setupTargetHandoverExtHo(void);
  void						  handleReleaseCallLegExtHo(CallLeg *leg);
  void						  handleReleaseCallLegExtH3(CallLeg *leg);
  BOOL						  hoCallVoiceConnected(void);
  BOOL						  h3CallVoiceConnected(void);
  HalfCallStates_t            hoCallState(void);



  //CH<xxu:11-23-99>
  int                         handleMobHoldRetvReq(JCCEvent_t& event, TwoPartyCallLeg* leg);

  bool                        setupTargetHandover(void);

  int                         handleMMMsg     (IntraL3Msg_t              *);
  int                         handleRRMsg     (IntraL3Msg_t              *);
  int                         handleTimeoutMsg(IntraL3Msg_t              *);
  int                         handleMobMsg    (T_CNI_RIL3MD_CCMM_MSG     *);
  int                         handleH323Msg   (IntraL3Msg_t              *);

  // ext-HO <chenj:05-15-01>
  bool                        setupOrigExtHo(void);
  bool                        setupOrigExtHo3(void);
  int                         handleHoaCcMsg (InterHoaVcMsg_t *hoaCCInMsg);

  //ext-HO <xxu:05-16-01>
  int						  handleH323MsgExtHo(IntraL3Msg_t* h323InMsg);
  int						  handleH323MsgExtH3(IntraL3Msg_t* h323InMsg);
  JCCEvent_t				  h323MapToEventExtHo(IntraL3Msg_t* h323InMsg, 
                                                  CCRemMsgData_t &remMsgData);
  JCCEvent_t				  h323MapToEventExtH3(IntraL3Msg_t* h323InMsg, 
                                                  CCRemMsgData_t &remMsgData);
  

  int                         handleVcCcMsg   (InterVcMsg_t              *);
  int                         handleVbCissMsg (CISSVblinkMsg_t          & );

  int                         smHandleEvent(JCCEvent_t);
  
  JCCEvent_t                  handleMMRelInd(void);
  JCCEvent_t                  handleMMEstCnf(void);
  JCCEvent_t                  handleMMEstInd(void);

  bool                        findHandle       (VOIP_CALL_HANDLE callHandle);
  SmsLeg                    * findSmsReference (UINT32           refNum    );

  SpeechChann_t               currSpeechCh     (void); 
  TwoPartyCallLeg           * activeLeg        (void);
  bool                        isHandoverAllowed(void);

  bool                        setIrt           (T_CNI_IRT_ID, T_CNI_L3_ID&);
  bool                        resetIrt         ();

  // maintenance methods
  void                        printData(JCCPrintStFn fn);
  void                        cleanup(T_CNI_RIL3_CAUSE_VALUE cause     = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING,
									  BOOL                   normClear = TRUE                               );

  // get and set for private data members 

  void                        setMmTermSetup     (CallLeg *leg) {setupMmTermLeg = leg          ;}
  void                        resetMmTermSetup   ()             {setupMmTermLeg = NULL         ;}
  CallLeg                   * getMmTermSetupLeg  ()             {return(setupMmTermLeg)        ;}
  bool                        isMmSetupTermActive()             {return(setupMmTermLeg != NULL);}

  void                        setSapi3           () {sapi3Exist_ = true ;}
  void                        resetSapi3         () {sapi3Exist_ = false;}
  bool                        getSapi3Status     () {return sapi3Exist_ ;}

  bool                        isMmConnected      () {return (entryId         != (T_CNI_IRT_ID)JCC_UNDEFINED || 
                                                             handoverSession == anchorHandoverSession       ||
                                                             handoverSession == targetHandoverSession         );}

  bool                        isSmsHoAllowed     ();

  void                        flashSapi3Queue    ();
 protected:

 private:


  // Internal Utility functions
  void                        handleReleaseCallLeg(CallLeg *leg);

  void                        initData(void);
  bool                        routeFromVoipToCallLeg(IntraL3Msg_t         *h323InMsg     ,
                                                     TwoPartyCallLeg      *&whichLeg     );
  bool                        routeFromVoipToSmsLeg (IntraL3Msg_t         * h323InMsg    ,
                                                     SmsLeg               *&whichLeg     );
  bool                        routeFromMsToCallLeg  (T_CNI_RIL3CC_MSG     *msInDecodedMsg,
                                                     TwoPartyCallLeg      *&whichLeg     );
  bool                        routeFromMsToSmsLeg   (T_CNI_RIL3_CP_HEADER& msInDecodedHdr,
                                                     SmsLeg               *&whichLeg     );

  CallLeg                    *getCallLegForNewTermination(bool &);
  CallLeg                    *getSmsLegForNewTermination (bool &);

  bool                        forwardPdu (T_CNI_LAPDM_L3MessageUnit       *msEncodedMsg  );


  // hide the assignment, and copy ctor and other defaults as needed.
  CCSessionHandler();  

  CCSessionHandler (const CCSessionHandler& rhs) ;
  
  CCSessionHandler&  operator= (const CCSessionHandler& rhs) ;

  int               operator==(const CCSessionHandler& rhs) const ;

 public:
 //CDR <xxu:08-18-00>
  TwoPartyCallLeg             * callLeg1;
  TwoPartyCallLeg             * callLeg2;
  TwoPartyCallLeg             * callLeg3;

  // ext-HO <chenj:05-15-01>
  HOCallLeg                   * hoCallLeg;
  HOCallLeg                   * h3CallLeg;

  T_CNI_RIL3_IE_MOBILE_ID       mobileId;

 private:
  CCMsgAnalyzer               *ma;
  CCSession_t                 * parent;
  
  //CDR <xxu:08-18-00>
  //TwoPartyCallLeg             * callLeg1;
  //TwoPartyCallLeg             * callLeg2;
  //TwoPartyCallLeg             * callLeg3;

  SmsLeg                      * smsLeg1; // MT SMS-PP
  SmsLeg                      * smsLeg2; // M0 SMS-PP
  // Igal: smsLeg3 Currently is NOT IN USE!!!
  SmsLeg                      * smsLeg3; // MT SMS-PP

  CISSCallLeg                 * cissCallLeg;

  T_CNI_RIL3_CAUSE_VALUE      releaseCause;

  IntraL3Msg_t                * mmInMsg;
  IntraL3Msg_t                * rrInMsg;

  bool                        isEmergencyCall;

  CallLeg                     * setupMmTermLeg;
  bool                        sapi3Exist_     ;
  SmQueue                     sapi3Queue_     ;
 
 public:
  // data members
  
  //BCT <xxu:07-06-00> BEGIN
  bool						  bctInvoke_;
  T_CNI_RIL3_SI_TI            bctTi_;
  TwoPartyCallLeg			  *bctLeg_;
  TwoPartyCallLeg			  *bctMainLeg_;
  VOIP_CALL_CALLING_PARTY_NUMBER  CallingPartyNumber;
  VOIP_CALL_DESTINATION_ADDRESS   CalledPartyNumber;
  VOIP_CALL_DESTINATION_ADDRESS   XltCalledPartyNumber;

  //BCT <xxu:07-06-00> END

  JCCTimer                    * sessionTimer;

  CCHandover                  * handoverSession;

  CCAnchorHandover            * anchorHandoverSession;
  CCTargetHandover            * targetHandoverSession;

  //ext-HO <xxu:05-16-01>
  CCTargetHandoverExtHo       * targetHandoverSessionExtHo;
  VOIP_CALL_HANDLE              hoCallHandle;  // used for hand-in only
  T_CNI_RIL3_IE_MS_CLASSMARK_2  classmark2Info;

  T_CNI_LAPDM_OID             oid;
  T_CNI_IRT_ID                entryId;

  short                       ludbIndex;

  bool                        isOtherLegHeld(short         legNum);

  // to test status message
  friend                      
    void                      sendStatusMsg (int sessionNum      ,
                                             int callLeg         );
};



#endif                                       // CCSessionHandler_H
