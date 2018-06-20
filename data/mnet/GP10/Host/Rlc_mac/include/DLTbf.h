// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLTbf.h
// Author(s)   : Tim Olson
// Create Date : 12/1/2000
// Description : 
//
// *******************************************************************

#ifndef __DLTBF_H__
#define __DLTBF_H__

#include "Tbf.h"
#include "bssgp/bssgp_api.h"
#include "DLRlcLink.h"


// *******************************************************************
// class DLTbf
//
// Description
//    
// *******************************************************************
class DLTbf : public Tbf {
public:

    const int ControlAckTagArrDepth = 5 * 8;  // Max is 5 * num timeslots
    
    // Constructor
    DLTbf() : Tbf() {};
    // Destructor
    ~DLTbf() {}
     
    JC_STATUS EstablishDLTbf(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg, bool ctrkAck);
    JC_STATUS ReestablishDLTbf(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg, bool ctrkAck);
    void UpdateTSSpecificData();
    JC_STATUS ReleaseDLTbf(bool deactivateDsp);
    JC_STATUS InternalReleaseDLTbf();
    JC_STATUS AbnormalReleaseDLTbf();
    
    unsigned char GetControlAckTag() { return(ctlAckTag[currAckTag]); }
    void ClearControlAckTag() 
    { 
        for (int i=0; i<ControlAckTagArrDepth; i++) 
            ctlAckTag[i] = CTL_ACK_TAG_NOT_APPLICABLE;
        currAckTag=0; 
    }
    unsigned char SetControlAckTag() 
    { 
        currAckTag = (++currAckTag) % currNumAckTags;
        ctlAckTag[currAckTag] = (++sysCtlAckCount != CTL_ACK_TAG_NOT_APPLICABLE ? sysCtlAckCount : ++sysCtlAckCount);
        return (ctlAckTag[currAckTag]);
    }
    
    bool CheckControlAckTag(unsigned char tag)
    {
        for (int i=0; i<currNumAckTags; i++) 
            if (ctlAckTag[i] == tag) return (TRUE);
        return (FALSE);    
    }
    
    void ShowTBFInfo();

    /*
    *  NOTE: In the following accessors, convert class values to ensure Class 1 gets value=1, Class 2
    *  gets 2, etc., as described in GSM 03.60, Section 15. The Scheduler assumes this to index arrays.
    */
    int GetDelayClass(void) const 
    {
        return(qosParams.delay_class-CNI_RIL3_QOS_DELAY_CLASS_1+1); 
    }
    int GetPrecedenceClass(void) const
    {
        return(qosParams.precedence_class-CNI_RIL3_QOS_PRECEDENCE_CLASS_HIGH_PRIORITY+1); 
    }
    int GetPeakThroughputClass(void) const
    {
        return(qosParams.peak_throughput-CNI_RIL3_QOS_PEAK_THROUGHPUT_1_KBPS+1);
    }
    int GetMultislotClass(void) const;

    T_RIL3_IE_QOS_PROFILE_ID                    qosProfile;
    T_CNI_RIL3_IE_QOS                           qosParams;
    T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY    raCapability;
    T_CNI_RIL3_IE_DRX_PARAMETER                 drxParams;
    T_CNI_RIL3_IE_MOBILE_ID                     msId;
    RIL3_IE_PRIORITY                            priority;
    DLRlcLink                                   rlcLink;
    bool                                        controlAck;

private:

    // Control message acknowledgement tags.
    unsigned char                   ctlAckTag[ControlAckTagArrDepth];
    int                             currAckTag;
    int                             currNumAckTags;
};


#endif