#ifndef __SMSMSGBUILDER_H__
#define __SMSMSGBUILDER_H__

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
// File        : SmsMsgBuilder.h
// Author(s)   : Igal Gutkin
// Create Date : 02-10-00
// Description : class specification for -  SmsMsgBuilder
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include <vxworks.h>

#include "ril3\ril3_sms_cp.h"
#include "ril3\ril3_sms_rp.h"

#include "CC\SmsCommonDef.h"

class SmsLeg;


class SmsMsgBuilder
{
	
public:
	
	// Destructor
	~SmsMsgBuilder() {};
	
	// Constructors
    SmsMsgBuilder (SmsLeg& parent) : parent_(parent) {;}
	
	// Operators
	
	// primary behaviour methods

	T_CNI_RIL3_RESULT sendCpError (T_CNI_RIL3_CP_CAUSE_VALUE);
	T_CNI_RIL3_RESULT sendCpData  ();
	T_CNI_RIL3_RESULT sendCpAck   ();

    bool sendMnSmsEstInd   (UINT32                   , UINT8* );
    bool sendMnSmsDataInd  (UINT32                   , UINT8* );
    bool sendMnSmsErrorInd (T_CNI_RIL3_RP_CAUSE_VALUE, UINT32 tempRefNum = 0);

	// maintenance methods

	// get and set for private data members 
    UINT32 getVbRefNum   ();
    void   setVbRefNum   ();

protected: // methods

private:
	bool              buildMsMsgCommon ();
    T_CNI_RIL3_RESULT sendMsMsg        ();

    bool              sendVbMsg        (SMS_VBLINK_MSGTYPE, UINT32           , 
                                        UINT8*            , UINT32 status = 0,
                                        UINT32 tempRefNum = 0                );

private: // data members

	SmsLeg&  parent_;

	// MS output message
	T_CNI_RIL3SMS_CP_MSG msOutMsg_;

	// VBLink output message
    SmsVblinkMsg_t       vbOutMsg_;
public:

};


#endif //__SMSMSGBUILDER_H__