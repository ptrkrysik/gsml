#ifndef HOCallLeg_H
#define HOCallLeg_H


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
// File        : HOCallLeg.h
// Author(s)   : Joe Chen
// Create Date : 05-15-01
// Description : class specification for -  HOCallLeg for external HO
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************


// Parent Class
#include "CC/CallLeg.h"

#include "CC/HOHalfCall.h"

class HOCallLeg : public CallLeg
{
	
public:
	
	// Destructor
	// ~HOCallLeg() ;
	
	// Constructors
	HOCallLeg(CCSessionHandler *session, int callId, MSG_Q_ID qid, short legNum);  
	
	// Operators
	
	// primary behaviour methods
	virtual void                 cleanup  (T_CNI_RIL3_CAUSE_VALUE cause);
	virtual void                 printData(JCCPrintStFn fnPtr);
	void                         setupOrigExtHo(void);
	void                         disconnectOrigExtHo(void);
	void                         setupTermExtHo(IntraL3Msg_t* h323InMsg);
	void                         handleReleaseCall(void);
	virtual int                  handleTimeoutMsg(IntraL3Msg_t       *ccInMsg);

	
	//ext-HO <xxu:06-07-01>
	JcSubId_t					  hoAgId(void);
    int							  callIndexEHO(void);
	BOOL						  authTerm(void);
	BOOL						  cancelHoTimerIfNeeded(void);
    T_CNI_RIL3_SPEECH_VERSION     speechVersion(void);
	
protected:
	
	void						initData(void);
	
private:
	
	// hide the assignment, and copy ctor and other defaults as needed.
	HOCallLeg();  
	
	HOCallLeg (const HOCallLeg& rhs) ;
	
	HOCallLeg&  operator= (const HOCallLeg& rhs) ;
	
	int               operator==(const HOCallLeg& rhs) const ;
	
private:

public:
	
	// data members
	CCProcedureType_t			procType;
	
	// Only one of the two half calls below is valid depending on the type of call
    void						* halfExtHoCallStorage;
    HOMoHalfCall				* origSide;
    HOMtHalfCall				* termSide;
    HOHalfCall					* msSide;

	
	// Connection State - Connection Manager Functionality
	bool						voiceConnected;
	
	// H323 side call state - only need to store the Call Handle 
	VOIP_CALL_HANDLE             h323CallHandle;

	bool                         disconnectSentToVoip;

	JCCTimer					 *tclTimer;

};


#endif //HOCallLeg_h

