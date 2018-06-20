#ifndef CallLeg_H
#define CallLeg_H

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
// File        : CallLeg.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-10-99
// Description : interface specification for -  CallLeg
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "JCC/JCCState.h"

#include "VOIP/voipapi.h"

//BCT <xxu:07-17-00> BEGIN
#include "CC/CCHandover.h"
//BCT <xxu:07-17-00> END

#include "CC/CCHalfCall.h"

#include "CDR/CdrVoiceCall.h"


// *******************************************************************
// forward declarations.
// *******************************************************************
class CCSessionHandler;

//typedefs

typedef enum 
{
	
	CC_PROC_NULL                  = 0,
	CC_PROC_MOB_ORIG              = 1,
	CC_PROC_MOB_TERM              = 2
		
} CCProcedureType_t;

typedef enum 
{
	CISS_ST_BASE   = 0,
	CISS_ST_IDLE,
	CISS_ST_MS,
	CISS_ST_VB
} CISSStates_t;

class CallLeg
{
	
public:
	
	// Destructor
	// ~CallLeg() ;
	
	// Constructors
	CallLeg();  
	
	// Operators
	
	// primary behaviour methods
	virtual void cleanup          (T_CNI_RIL3_CAUSE_VALUE  cause   ) = 0;
	virtual int  handleTimeoutMsg (IntraL3Msg_t           *ccInMsg ) = 0;
	virtual void printData        (JCCPrintStFn            fnPtr   )    ;
    virtual void setupTerm        (VOIP_CALL_HANDLE, int, int, bool) {;};
	
	// maintenance methods
	
	// get and set for private data members 
	
protected:
	
	void                         initData(void);
	
private:
	
	// hide the assignment, and copy ctor and other defaults as needed.
	
	CallLeg (const CallLeg& rhs) ;
	
	CallLeg& operator= (const CallLeg& rhs) ;
	
	int      operator==(const CallLeg& rhs) const ;
	
protected:
	
public:
	// data members
	
	CCSessionHandler             * parent;

	T_CNI_LAPDM_OID              oid();
	T_CNI_IRT_ID                 entryId();
	short                        ludbIndex();
	
	
	//CDR <xxu:08-22-00>
	CCdrVoiceCall				 ccCdr;
	bool						 ccCdrCallState;
    JCCTimer					 *ccCdrTimer;

	// CDR information
	int                          cdrIndex;
	
	int                          callRefValue;
	
	short                        callLegNum;
	
	bool                         idle;
	
	int                          callIndex;
	MSG_Q_ID                     msgQId;
	
};

class TwoPartyCallLeg : public CallLeg
{
	
public:
	
	// Destructor
	// ~TwoPartyCallLeg() ;
	
	// Constructors
	TwoPartyCallLeg(CCSessionHandler *session, int callId, MSG_Q_ID qid, short legNum);  
	
	// Operators
	
	// primary behaviour methods
	virtual void                 cleanup  (T_CNI_RIL3_CAUSE_VALUE cause);
	virtual void                 printData(JCCPrintStFn fnPtr);
	void                         setupOrig(T_CNI_RIL3_SI_TI            ti = 0,
                                           T_CNI_RIL3_CM_SERVICE_TYPE  serviceRequested = CNI_RIL3_CM_SERVICE_TYPE_MO_CALL_OR_PACKET_MODE);
	void                         setupTerm(VOIP_CALL_HANDLE voipCallHandle,
                                           int              cdrIndex      ,
                                           int              callRefValue  ,
                                           bool             callWaiting   );
	void                         handleReleaseCall(void);
	virtual int                  handleTimeoutMsg(IntraL3Msg_t       *ccInMsg);
	
    bool                         isActive(void);
	// maintenance methods
	
	// get and set for private data members 
	//BCT <xxu:07-12-00> BEGIN
	T_CNI_RIL3_SI_TI             getBctTi();
	bool                         getBctInvoke();
	TwoPartyCallLeg		     *getBctLeg();
	T_CNI_IRT_ID                 getEntryId();
	CCHandover		     *getHandoverSession();
	CCAnchorHandover	     *getAnchorHandoverSession();
	HJCRTPSESSION                getHoRtpSession();
	//BCT <xxu:07-12-00> END
	
	
protected:
	
	void                         initData(void);
	
private:
	
	// hide the assignment, and copy ctor and other defaults as needed.
	TwoPartyCallLeg();  
	
	TwoPartyCallLeg (const TwoPartyCallLeg& rhs) ;
	
	TwoPartyCallLeg&  operator= (const TwoPartyCallLeg& rhs) ;
	
	int               operator==(const TwoPartyCallLeg& rhs) const ;
	
private:

public:
	
	//CDR <xxu:08-22-00>
	//CCdrVoiceCall				 ccCdr;
	//bool						 ccCdrCallState;
    //JCCTimer					 *ccCdrTimer;

	// data members
	bool                         waitingCallLeg;

	CCProcedureType_t            procType;
	
	// Only one of the two half calls below is valid depending on the type of call
	void                         * halfCallStorage;
        CCOrigHalfCall               * origSide;
        CCTermHalfCall               * termSide;
        CCHalfCall                   * msSide;
	
	// H323 Channel state
	ResStates_t                  h245ChannState;
	
	// Connection State - Connection Manager Functionality
	bool                         voiceConnected;
	
	// H323 side call state - only need to store the Call Handle 
	VOIP_CALL_HANDLE             h323CallHandle;

	//BCT <xxu:09-21-00>
 	VOIP_CALL_HANDLE             oH323CallHandle; 
 	VOIP_CALL_HANDLE             nH323CallHandle; 
 	bool						 bctArrival_;
 
	bool                         disconnectSentToVoip;

	bool                         isEmergencyCall;

	//PR1378 <xxu:08-16-00>
	JCCTimer					 *tclTimer;

};

class CISSCallLeg : public CallLeg
{
	
public:
	
	// Destructor
	~CISSCallLeg();
	
	// Constructors
	CISSCallLeg(CCSessionHandler *session, int callId, MSG_Q_ID qid, short callLegNum);  
	
	// Operators
	
	// primary behaviour methods
	handleMsMsg(T_CNI_RIL3MD_CCMM_MSG  *msInMsg);
	handleVbMsg(CISSVblinkMsg_t &netInMsg);
	
	// maintenance methods
	virtual void cleanup   (T_CNI_RIL3_CAUSE_VALUE cause);
	virtual void printData (JCCPrintStFn fnPtr);
	
	virtual int  handleTimeoutMsg(IntraL3Msg_t  *ccInMsg);
	
	
	// get and set for private data members 
	
protected:
	void                         initData(void);
	
private:
	
	// hide the assignment, and copy ctor and other defaults as needed.
	CISSCallLeg();  
	
	CISSCallLeg (const CISSCallLeg& rhs) ;
	
	CISSCallLeg&  operator= (const CISSCallLeg& rhs) ;
	
	int operator==(const CISSCallLeg& rhs) const ;
	
private:
	
	// ciss state
	CISSStates_t      cissState_;
};


#endif                        // CallLeg_H
