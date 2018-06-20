// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ULTbf.h
// Author(s)   : Tim Olson
// Create Date : 12/1/2000
// Description : 
//
// *******************************************************************

#ifndef __ULTBF_H__
#define __ULTBF_H__

#include "Tbf.h"
#include "ULRlcLink.h"
#include "ieChannelRequestDescription.h"
#include "ieMSRadioAccessCapability.h"
#include "ieAccessType.h"


// *******************************************************************
// class ULTbf
//
// Description
//    
// *******************************************************************
class ULTbf : public Tbf {
public:

    // Constructor
    ULTbf() : Tbf() {};
    // Destructor
    ~ULTbf() {};   

    JC_STATUS EstablishULTbf(
        IeAccessType                      *accessType,
        IeTLLI                            *tlli, 
        IeChannelRequestDescription       *chanReqDesc,
        IeMSRadioAccessCapability         *radioAccessCapability
    );
    JC_STATUS ReallocateTbf(
        IeAccessType                      *accessType,
        IeChannelRequestDescription       *chanReqDesc,
        IeMSRadioAccessCapability         *radioAccessCapability
    );
    JC_STATUS ReleaseULTbf();
    JC_STATUS AbnormalReleaseULTbf();

    unsigned char GetControlAckTag() { return(ctlAckTag); }
    void ClearControlAckTag() { ctlAckTag = CTL_ACK_TAG_NOT_APPLICABLE; }
    unsigned char SetControlAckTag() 
    { 
        ctlAckTag = (++sysCtlAckCount != CTL_ACK_TAG_NOT_APPLICABLE ? sysCtlAckCount : ++sysCtlAckCount);
        return (ctlAckTag);
    }

    void ShowTBFInfo();

    int GetDelayClass(void) const  
    {
        return(chanReq.radioPriority+1);         // input is zero-based; Scheduler uses 1-based values
    }
    int GetPrecedenceClass(void) const
    {
        return(1);                               // MS does not send Precedence Class; use constant=1
    }
    int GetPeakThroughputClass(void) const
    {
        return(chanReq.peakThroughputClass);     // input is already one-based (for uplink only)
    }
    int GetMultislotClass(void);

    // Allocated resources
    unsigned char                   usf[MAX_TIMESLOTS];       

    // ULTbf data members   
    IeAccessType                      accType;
    IeChannelRequestDescription       chanReq;
    IeMSRadioAccessCapability         raCapability;                     
    ULRlcLink                         rlcLink;
    
private:
    // Control message acknowledgement tags.
    unsigned char                   ctlAckTag;
    
};


#endif