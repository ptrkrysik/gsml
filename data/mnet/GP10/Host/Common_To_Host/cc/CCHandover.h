#ifndef CCHandover_H
#define CCHandover_H

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
// File        : CCHandover.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "csunew/csu_head.h"
#include "JCC/JcVcToVc.h"
#include "JCC/JCCEvent.h"
#include "JCC/JCCState.h"
#include "JCC/JCCL3Msg.h"
#include "JCC/JCCLog.h"
#include "CC/smqueue.h"

// ext-HO <chenj: 05-29-01>
#include "cc/A_Handover_Common.h"
#include "voip/exchangeho.h"


// *******************************************************************
// forward declarations.
// *******************************************************************

class CCSessionHandler;

const int MAX_HO_SRC_DOWNLINK_QUEUE_LENGTH
= 5;

const int MAX_HO_SRC_STATES
= (HAND_ST_ANCH_EHO_COMPLT + 1);            // ext-HO <chenj:06-13-01>

const int MAX_HO_TRG_STATES
= MAX_HO_SRC_STATES;

const int MAX_HO_SRC_EVENTS   
= (HAND_END_HO - HAND_SRC_EVENT_BASE + 1);  // ext-HO <chenj:06-13-01>

const int MAX_HO_TRG_EVENTS   
= MAX_HO_SRC_EVENTS;


//typedefs

class CCHandover
{
 friend SmQueueMm;

 public:

  // Destructor
  // ~CCHandover() ;

  // Constructors
  CCHandover(CCSessionHandler *, int, MSG_Q_ID);  

  // Operators

  // primary behaviour methods
  JCCEvent_t                  handleInvalidEvent(void);
  JCCEvent_t                  handleNullEvent(void);
  JCCEvent_t                  handleMMLateEstCnf(void);

  JCCEvent_t                  handleInternalEvent(JCCEvent_t);

  int                         handleMMMsg(IntraL3Msg_t *mmInMsg,
                                          JCCEvent_t   currEvent);
  int                         handleRRMsg(IntraL3Msg_t *rrInMsg,
                                          JCCEvent_t   currEvent);
  int                         handleVcCcMsg(InterVcMsg_t    *vcCcInMsg);

  //ext-HO <xxu:05-22-01>
  int                         handleHoaCcMsg(InterHoaVcMsg_t    *hoaCcInMsg);
  int                         handleTimeoutMsg(IntraL3Msg_t *ccInMsg);
  void                        classmarkFill( T_AIF_MSG *AIF_Msg );
  void                        channelTypeFill( T_AIF_MSG *AIF_Msg );
  void                        currentChannelTypeFill( T_AIF_MSG *AIF_Msg );
  void                        encryptionInformationFill( T_AIF_MSG *AIF_Msg );
  void                        currentEncryptionAlgorithmFill( T_AIF_MSG *AIF_Msg );
  void                        populateAHandoverRequest(GlobalCellId_t *, GlobalCellId_t *);
  int                         selectEncryption(void);
  int                         selectChannelType(void);

  // Handover Utility functions
  void                        initData(void);
  void                        printData(JCCPrintStFn fn);

  bool                        isEqualGlobalCellId(const GlobalCellId_t *gCell1,
                                                  const GlobalCellId_t *gCell2);

  void                        sendVcMsg(JcModuleId_t origModuleId,
                                        JcModuleId_t destModuleId);

  virtual void                sendMMMsgToRemoteCC (IntraL3Msg_t *) = 0;

  void                        emptyMMQueue (void);

  // HO<xxu:02-02-00> send msg from MSC-A to MSC-B'
  void                        sendVcMsgH3(JcModuleId_t origModuleId,
                                         JcModuleId_t destModuleId);
  //RETRY<xxu:05-01-00> BEGIN
  int                         getNextHoCandidate(void);
  //RETRY<xxu:05-01-00> END

  // Ciphering <chenj:07-17-01>
  unsigned char               findGpCipheringCapability(void);

  // maintenance methods
  
  // Handle Handover cleanup
  virtual BOOL		          cleanup(BOOL force = FALSE) = 0;

  // get and set for private data members 

 private:

  // hide the assignment, and copy ctor and other defaults as needed.
  // CCHandover();  

  CCHandover (const CCHandover& rhs) ;
  
  CCHandover&  operator= (const CCHandover& rhs) ;

  int               operator==(const CCHandover& rhs) const ;

 protected:
  CCSessionHandler            * parent;
  MSG_Q_ID                    msgQId;

  bool                        hoSrcLinkLost_;

  IntraL3Msg_t                mmOutMsg;
  IntraL3Msg_t                rrOutMsg;
 
  InterVcMsg_t                * vcInMsg;   //HO<xxu:02-07-00>

  
 
  InterVcMsg_t                vcOutMsg;

  IntraL3Msg_t                * mmInMsg;
  IntraL3Msg_t                * rrInMsg;


  // Handover Rtp data
  // BCT <xxu:07-14-00> BEGIN
  // HJCRTPSESSION               hoRtpSession_;
  // BCT <xxu:07-14-00> END

  UINT32                      hoVcIpAddress_;
  JcSubId_t                   hoVcCallIndex_;

  // Rtp data
  UINT16                      otherHoIncomingRtpPort_;

  JCCEvent_t                  currEvent;

 protected:   //HO<xxu:01-26-00> Handto3rd Rtp data
  HJCRTPSESSION               h3RtpSession_;
  UINT32                      h3VcIpAddress_;
  JcSubId_t                   h3VcCallIndex_;
  UINT16                      otherH3IncomingRtpPort_;
  bool                        h3SrcLinkLost_;

  //RETRY<xxu:04-21-00> BEGIN
  VBLINK_API_CELL_LOCATION_REQUEST_MSG cellLocationRequest;
  IntraL3HandoverRequired_t   handReqd;
  UINT16				candIndex_;
  UINT16				hoRetry_;
  UINT16				hoNextCandidate_;
  //RETRY<xxu:04-21-00> END

 public:      //HO<xxu:01-26-00>
  GlobalCellId_t              h3GlobalCellId_;
  HandoverTypes_t             hoType_;
  
  SmQueueMm                   mmQueue_;

  //ext-HO <xxu:05-30-01>
  InterHoaVcMsg_t                          *hoaInMsg; 
  InterHoaVcMsg_t                           hoaOutMsg;
  T_AIF_MSG                                 aifMsg;
  JcSubId_t                                 hoAgId_;
  JcSubId_t                                 h3AgId_;
  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER     hoNumber_;
  int                                       hoEndCause_;
  IntraL3CipherModeCommand_t                currEncrypt_;
  A_Ie_Encryption_Information_t             reqEncrypt_;
  A_Ie_Chosen_Encrypt_Algorithm_t           optCurrEncrypt_;
  SpeechChann_t                             currSpeechCh_;
  A_Ie_Channel_Type_t                       reqChannelType_;
  A_Ie_Current_Channel_Type1_t              optCurrChannelType_;
  L3RMChannelType_t                         currChannel_;
  A_Ie_Classmark_Information2_t             currClassmark_;
  int                                       callIndex;

 private:

  int                         smHandleEvent();
    
 public:
  // data members

  // BCT <xxu:07-14-00> BEGIN
  HJCRTPSESSION               hoRtpSession_;
  // BCT <xxu:07-14-00> END

  HandoverStates_t            hoState_;

  GlobalCellId_t              hoGlobalCellId_;
};

class CCAnchorHandover : public CCHandover
{

 public:

  // Destructor
  // ~CCAnchorHandover() ;

  // Constructors
  CCAnchorHandover(CCSessionHandler *session,
                   int              callId, 
                   MSG_Q_ID         qid);  

  // Operators

  // primary behaviour methods
  void                        addToDownlinkQueue (T_CNI_LAPDM_L3MessageUnit        *msEncodedMsg);
  void                        emptyDownlinkQueue (void);
  void                        addToRRQueue (IntraL3Msg_t      *rrMsg);
  void                        emptyRRQueue (void);

  void                        sendL2MsgToTargetCC(T_CNI_LAPDM_L3MessageUnit        *msEncodedMsg);
  void                        sendRRMsgToTargetCC(IntraL3Msg_t      *rrMsg);

  // ext-HO <chenj:06-12-01>
  void                        sendL2MsgToTargetPLMN(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg);
  void                        sendRRMsgToTargetPLMN(IntraL3Msg_t      *rrMsg);
  void                        sendMMMsgToRemotePLMN (IntraL3Msg_t *);

  T_CSU_PORT_ID               mobileCsuPort(void);

  JCCEvent_t                  handleInternalEvent(JCCEvent_t);

  int                         handleMMMsg(IntraL3Msg_t *mmInMsg,
                                          JCCEvent_t   currEvent);
  int                         handleRRMsg(IntraL3Msg_t *rrInMsg,
                                          JCCEvent_t   currEvent);
  int                         handleVcCcMsg(InterVcMsg_t    *vcCcInMsg);
  int                         handleTimeoutMsg(IntraL3Msg_t *ccInMsg);

  // used for getting the vipercell address of a neighbouring cell.
  int                         handleVbCcMsg(IntraL3Msg_t      *vbCcInMsg);

  virtual void                sendMMMsgToRemoteCC (IntraL3Msg_t *);
  
  // Handover Src Handlers in CCHoSrcHnd.cpp
  void			      releaseMobRes();
  void			      releaseMobAndConnRes();
  
  JCCEvent_t                  handleAnchPostHoRls();

  //HO<xxu:02-23-00>
  JCCEvent_t                  handleAnchPostH3Rls();
  JCCEvent_t                  cleanHoCallWiCause(InterVcMsgType_t msg, JcFailureCause_t cause);
  JCCEvent_t                  cleanH3CallWiCause(InterVcMsgType_t msg, JcFailureCause_t cause);

  JCCEvent_t                  handleAnchPostHoMobEvent();
  
  JCCEvent_t                  handleAnchDuringHoRls();
  
  JCCEvent_t                  handleAnchHoReqd();
  
  JCCEvent_t                  handleVcAddressRsp();
  JCCEvent_t                  handleAnchHoFail();
  
    // Handover Target To Anchor Handlers in CCHoSrcHnd.cpp
  JCCEvent_t                  handleTrgToAnchPerfHoAck();
  JCCEvent_t                  handleTrgToAnchPerfHoNack();
  JCCEvent_t                  handleTrgToAnchCompltHo();
  
  JCCEvent_t                  handleAnchThoTimerExpiry();
  JCCEvent_t                  handleAnchT103TimerExpiry();

  //RETRY<xxu:04-21-00> BEGIN
  JCCEvent_t			handleAnchTvbTimerExpiry();
  //RETRY<xxu:04-21-00> END
  
  // Handback Target to Anchor Handlers in CCHoSrcHnd.cpp
  JCCEvent_t                  handleTrgToAnchPerfHbReq();
  JCCEvent_t                  handleTrgToAnchAbortHb();
  
  JCCEvent_t                  handleAnchT104TimerExpiry();
  
  // Handback Target Handlers in CCHoSrcHnd.cpp
  JCCEvent_t                  handleAnchHoReqAck();
  JCCEvent_t                  handleAnchHoReqNack();
  JCCEvent_t                  handleAnchHoComplt();

  //HO<xxu:01-27-00> Sendback HoReqNack to target cell with failure cause
  JCCEvent_t                  handleAnchHoReqNackWiCos(JcFailureCause_t cause);
  JCCEvent_t			handleAnchAbortWiCos(JcFailureCause_t cause);

  // ext-HO <chenj:05-15-01>
  int                         handleHoaCcMsg(InterHoaVcMsg_t *hoaCcInMsg);
  JCCEvent_t                  handleExtPerformHOAck(void);
  JCCEvent_t                  handleExtPerformHONack(void);
  JCCEvent_t                  handleTrgToAnchEndHandover(void);
  JCCEvent_t                  handleAnchPostExtHoRls();
  JCCEvent_t                  cleanExtHoCallWiCause(InterEHOMsgType_t msg, JcFailureCause_t cause);
  JCCEvent_t                  ExtPerformHO(void);
  JCCEvent_t                  handleAnchExtHoTimerExpiry(void);
  JCCEvent_t                  handleAnchPostExtHoMobEvent(void);
  JCCEvent_t                  handleAnchPostExtHoHandbackEvent(void);
  JCCEvent_t                  sendHandbackAck(void);
  JCCEvent_t                  sendHandbackNack(void);
  JCCEvent_t                  handleAnchExtHoReqAck(void);
  JCCEvent_t                  handleAnchExtHOT104TimerExpiry();
  JCCEvent_t                  handleAnchExtHoComplt();
  JCCEvent_t                  handleAnchExtHoFail(void);
  JCCEvent_t                  handleAnchExtHandbackFail(void);
  JCCEvent_t                  do_Handback_scenario(void);
  JCCEvent_t                  do_Handover_To_Third_PLMN_scenario(void);
  JCCEvent_t                  do_Handback_To_Third_MNET_scenario(void);
  JCCEvent_t                  do_MNET_Handback_scenario(void);
  JCCEvent_t                  do_MNET_Handback_To_Third_MNET_scenario(void);
  JCCEvent_t                  do_MNET_Handback_To_External_PLMN_scenario(void);
  void                        initExtHandbackData(void);
  void                        initExtHandmsc3Data(void);
  void                        swapH3Data(void);

  // maintenance methods
  void                        initData();
  
  void                        initHandbackData();

  //HO<xxu:01-27-00>
  void				initHandmsc3Data();
    
  // Handle Handover cleanup
  virtual BOOL                cleanup(BOOL force = FALSE);

  // get and set for private data members 

 //BCT <xxu:09-23-00>
 public:
  bool                        hoSimplexConnected_;
  bool                        h3SimplexConnected_;

  // ext-HO <chenj:06-29-01>
  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  targetMscNumber_;

 private:

  // hide the assignment, and copy ctor and other defaults as needed.
  // CCAnchorHandover();  

  CCAnchorHandover (const CCAnchorHandover& rhs) ;
  
  CCAnchorHandover&  operator= (const CCAnchorHandover& rhs) ;

  int               operator==(const CCAnchorHandover& rhs) const ;

 private:
  
  int                         smHandleEvent();

  // ext-HO <chenj:05-11-01>
  void                        sendEndHandover(void);

  // For building data for remote events
  CCRemMsgData_t              remMsgData;

  int                         msMsgsQueued_;
  T_CNI_LAPDM_L3MessageUnit   downlinkQueue_[MAX_HO_SRC_DOWNLINK_QUEUE_LENGTH];              

  bool                        rrMsgsQueued_;
  IntraL3Msg_t                savedRRMsg_;

  //BCT <xxu:09-23-00>
  //bool                      hoSimplexConnected_;

  //HO<xxu:01-26-00>
  //bool                        h3SimplexConnected_;

};

class CCTargetHandover : public CCHandover
{

 public:

  // Destructor
  // ~CCTargetHandover() ;

  // Constructors
  CCTargetHandover(CCSessionHandler *session,
                   int              callId, 
                   MSG_Q_ID         qid);  

  // Operators

  // primary behaviour methods

  void                        sendRRMsgToAnchorCC(IntraL3Msg_t               *rrMsg);
  void                        sendMSMsgToAnchorCC(T_CNI_RIL3MD_CCMM_MSG      *msInMsg);
  virtual void                sendMMMsgToRemoteCC      (IntraL3Msg_t *);

  JCCEvent_t                  handleInternalEvent(JCCEvent_t);

  int                         handleMMMsg(IntraL3Msg_t *mmInMsg,
                                          JCCEvent_t   currEvent);
  int                         handleRRMsg(IntraL3Msg_t *rrInMsg,
                                          JCCEvent_t   currEvent);
  int                         handleVcCcMsg(InterVcMsg_t    *vcCcInMsg);
  int                         handleTimeoutMsg(IntraL3Msg_t *ccInMsg);

  // Handover Anchor To Target Handlers in CCHoTrgHnd.cpp
  JCCEvent_t                  handleAnchToTrgPerfHoReq();
  JCCEvent_t                  handleAnchToTrgAbortHo();
  
  JCCEvent_t                  handleTrgT204TimerExpiry();
  
  // Handover Target Handlers in CCHoTrgHnd.cpp
  JCCEvent_t                  handleTrgHoReqAck();
  JCCEvent_t                  handleTrgHoReqNack();
  JCCEvent_t                  handleTrgHoComplt();
  
    // Handback Src Handlers in CCHoTrgHnd.cpp
  JCCEvent_t                  handleTrgHoReqd();
  JCCEvent_t                  handleTrgHoFail();
  
  // Handback Anchor to Target Handlers in CCHoTrgHnd.cpp
  JCCEvent_t                  handleAnchToTrgPerfHbAck();
  JCCEvent_t                  handleAnchToTrgPerfHbNack();
  JCCEvent_t                  handleAnchToTrgCompltHb();
  
  JCCEvent_t                  handleTrgTshoTimerExpiry();
  JCCEvent_t                  handleTrgT203TimerExpiry();
  
  // Post Handover Mobile Events
  JCCEvent_t                  handleTrgPostHoRls();
  
  // maintenance methods
  void                        initHandbackData();
  
  void                        printData(JCCPrintStFn fn);

  // Handle Handover cleanup
  virtual BOOL                cleanup(BOOL force = FALSE);

  // get and set for private data members 

 private:

  // hide the assignment, and copy ctor and other defaults as needed.
  // CCTargetHandover();  

  CCTargetHandover (const CCTargetHandover& rhs) ;
  
  CCTargetHandover&  operator= (const CCTargetHandover& rhs) ;

  int               operator==(const CCTargetHandover& rhs) const ;

 private:
  int                         smHandleEvent();
      
  void                        initData(void);

  bool                        voiceConnected;

  SpeechChann_t               currSpeechCh_;
  L3RMChannelType_t			  currChannel_;    //ext-HO <xxu:06-08-01>

};


//ext-HO <xxu:06-07-01>
class CCTargetHandoverExtHo : public CCHandover
{

 public:

  // Destructor
  // ~CCTargetHandoverExtHo() ;

  // Constructors
  CCTargetHandoverExtHo(CCSessionHandler *session,
                        int              callId, 
                        MSG_Q_ID         qid);  

  // Operators

  // Primary behaviour methods
  int						  handleHoaCcMsg(InterHoaVcMsg_t *hoaCcInMsg);
  int                         handleVcCcMsg(InterVcMsg_t    *vcCcInMsg);
  int                         handleVbCcMsg (IntraL3Msg_t *vbCcInMsg);
  int					      handleRRMsg(IntraL3Msg_t *rrMsg, JCCEvent_t hoEvent);
  int						  handleMMMsg(IntraL3Msg_t *mmMsg, JCCEvent_t   hoEvent);
  int						  handleTimeoutMsg (IntraL3Msg_t *ccInMsg);
  void				          sendMSMsgToAnchorCC(T_CNI_RIL3MD_CCMM_MSG *msInMsg);
  virtual void				  sendMMMsgToRemoteCC(IntraL3Msg_t *mmInMsg);
  
  //External Handover common event handlers
  JCCEvent_t						  handleAnchToTrgPostHoHoaEvent(void);
  JCCEvent_t						  handleTrgToAnchPostHoMobEvent(void);
  JCCEvent_t						  handleNullEvent(void);

  JCCEvent_t						  handleAnchToTrgEndHandover(void);
  JCCEvent_t				  handleTrgToAnchAbortHb(void);
  JCCEvent_t				  handleVcAddressRsp(void);
  JCCEvent_t				  handleAnchPostHoRls(void);
  JCCEvent_t				  handleTrgMmRelInd(void);

  JCCEvent_t				  handleAnchTvbTimerExpiry(void);
  JCCEvent_t				  handleTrgT202TimerExpiry(void);  //interBSS handover under MSC-B
  JCCEvent_t				  handleTrgT204TimerExpiry(void);
  JCCEvent_t				  handleTrgT211TimerExpiry(void);  //formerly Tsho timer
 
  //External Handover handin case handlers
  JCCEvent_t				  handleAnchToTrgPerfHoReq(void);
  JCCEvent_t                  handleTrgHoReqAck(void);
  JCCEvent_t                  handleTrgHoReqNack(void);
  JCCEvent_t                  handleTrgHoAccess(void);
  JCCEvent_t                  handleTrgHoComplt(void);
  
  //External Handover postHandin--handback anchor GP->PLMN case handlers
  JCCEvent_t				  handleTrgHoReqd(void);
//JCCEvent_t				  handleAnchToTrgPerfHbAck(void);
//JCCEvent_t				  handleAnchToTrgPerfHbNack(void);
  
  JCCEvent_t				  handleTrgHoFail(void);
  JCCEvent_t				  handleAnchToTrgCompltHb(void);
  
  //External Handover postHandin--handover anchor GP->another GP case handlers
  JCCEvent_t				  handleTrgToAnchPerfHoAck(void);
  JCCEvent_t				  handleTrgToAnchPerfHoNack(void);
  JCCEvent_t				  handleTrgToAnchCompltHo(void);
  JCCEvent_t				  handleAnchHoFail(void);
    
  //External Handover postHandin-->handback nonanchor GP->PLMN case handlers
  JCCEvent_t				  handleTrgToAnchPerfHbReq(void);
  JCCEvent_t				  handleAnchToTrgPerfHbAck(void);
  JCCEvent_t				  handleAnchToTrgPerfHbNack(void);
  JCCEvent_t				  handleTrgToAnchHbFail(void);     //revert to old cell case
  JCCEvent_t				  handleTrgToAnchHoAccess(void);
//JCCEvent_t				  handleAnchToTrgCompltHb(void);
    
  //External Handover postHandin-->handback nonanchor GP->anchor GP case handlers
  JCCEvent_t				  handleAnchHoReqAck(void);
  JCCEvent_t				  handleAnchHoReqNack(void);
  JCCEvent_t				  handleAnchHoComplt(void);
  JCCEvent_t				  handleTrgToAnchHoFail(void);
    
  //External Handover postHandin-->handover non-anchor GP->3rd non-anchor GP case handlers
  //JCCEvent_t				  handleTrgToAnchPerfHoAck(void);
  //JCCEvent_t				  handleTrgToAnchPerfHoNack(void);
  //JCCEvent_t				  handleTrgToAnchAbortHb(void);
  //JCCEvent_t				  handleTrgToAnchCompltHo(void);
  //JCCEvent_t				  handleTrgToAnchHbFail(void);  
  
  void                        printData(JCCPrintStFn fn);
  
  // Handle Handover cleanup
  virtual BOOL                cleanup(BOOL force = FALSE);

  public:
  bool                        hoSimplexConnected_;
  bool                        h3SimplexConnected_;

 private:

  // hide the assignment, and copy ctor and other defaults as needed.
  // CCTargetHandoverExtHo();  

  CCTargetHandoverExtHo (const CCTargetHandoverExtHo& rhs) ;
  
  CCTargetHandoverExtHo&  operator= (const CCTargetHandoverExtHo& rhs) ;

  int               operator==(const CCTargetHandoverExtHo& rhs) const ;

 private:
  int                         smHandleEvent();
  
  void                        sendL2MsgToGpBCc(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg);
  bool                        sendL2MsgToGpACc (T_CNI_LAPDM_L3MessageUnit *msEncodedMsg);
  void                        addToDownlinkQueue (T_CNI_LAPDM_L3MessageUnit *msEncodedMsg);
  void                        emptyDownlinkQueue (void);
  void                        addToRRQueue (IntraL3Msg_t *rrMsg);
  void                        emptyRRQueue (void);

  //External Handover initialization handlers
  void                        initData(void);
  int						  ccReleaseCall(void);
  int						  ccReleaseOwn(void);
  void						  ccReleaseGpHo(void);
  void						  ccReleaseGpHb(void);
  void						  ccReleaseGpH3(void);
  void						  ccReleaseEhHb(void);
  void						  ccReleaseGpEhHb(void);

  int                         msMsgsQueued_;
  bool                        rrMsgsQueued_;
  IntraL3Msg_t                savedRRMsg_;

  bool                        voiceConnected;
  T_CNI_LAPDM_L3MessageUnit   downlinkQueue_[MAX_HO_SRC_DOWNLINK_QUEUE_LENGTH];              
  
};

// pointer to member type
typedef JCCEvent_t (CCAnchorHandover::* AnchorHandoverMsgHandler_t) (); 
typedef JCCEvent_t (CCTargetHandover::* TargetHandoverMsgHandler_t) (); 

extern AnchorHandoverMsgHandler_t hoSrcHandler[MAX_HO_SRC_STATES][MAX_HO_SRC_EVENTS];
extern TargetHandoverMsgHandler_t hoTrgHandler[MAX_HO_TRG_STATES][MAX_HO_TRG_EVENTS];

//ext-HO <xxu:06-07-01>
typedef JCCEvent_t (CCTargetHandoverExtHo::* ExtHoTargetHandoverMsgHandler_t) (); 
extern ExtHoTargetHandoverMsgHandler_t extHoTrgHandler[MAX_HO_TRG_STATES+MAX_HO_SRC_STATES][MAX_HO_TRG_EVENTS+MAX_HO_SRC_EVENTS];

#endif                                       // CCHandover_H
