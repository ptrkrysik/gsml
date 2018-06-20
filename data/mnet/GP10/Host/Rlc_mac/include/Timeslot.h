// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : Timeslot.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __TIMESLOT_H__
#define __TIMESLOT_H__

#include "RlcMacCommon.h"
#include "TAITable.h"
#include "USFTable.h"
#include "DLSignallingMsgQueue.h"
#include "Tbf.h"
#include "ULTbf.h"
#include "DLTbf.h"
#include "ScheduleInfo.h" 
#include "ScheduleQueue.h"

#define INVALID_CHANNEL_COMBINATION 0xffffffff
#define INVALID_RTI 0xff

// *******************************************************************
// class Timeslot
//
// Description
//    
// *******************************************************************
class Timeslot {
public:
    // Constructor
    Timeslot()
    {
        dataRTSCount = 0;
        signallingRTSCount = 0;
        channelComb = INVALID_CHANNEL_COMBINATION;
        rti = 0;
    }
    
    // Destructor
    ~Timeslot() {};
     
    // Functions related to TAI
    unsigned char AllocateTAI(Tbf *pTbf)
    {
        return (sysTAI.AllocateTAI(pTbf));
    }
    
    void ReleaseTAI(unsigned char tai)
    {
        sysTAI.ReleaseTAI(tai);
    }
    
    void ShowTAITable()
    {
        sysTAI.ShowTAITable();
    }
    
    // Functions related to USF
    unsigned char AllocateUSF(ULTbf *pTbf)
    {
        return (sysUSF.AllocateUSF(pTbf));
    }
    
    void ReleaseUSF(unsigned char usf)
    {
        sysUSF.ReleaseUSF(usf);
    }
    
    void ShowUSFTable()
    {
        sysUSF.ShowUSFTable();
    }
    
    // Functions related to DL Signalling Queues
    JC_STATUS QueueDLSignallingMsg(MsgPacchReq *l1Msg)
    {
        return(sysDLSigMsgQ.QueueDLSignallingMsg(l1Msg));
    }
    
    MsgPacchReq *DequeueDLSignallingMsg()
    {
        return(sysDLSigMsgQ.DequeueDLSignallingMsg());
    }
       
    // Functions related to RTS counts
    int GetDataRTSCount() 
    {   
        return (dataRTSCount);
    }
    
    int GetSignallingRTSCount() 
    {   
        return (signallingRTSCount);
    }
    
    void IncDataRTSCount(int inc)
    {
        dataRTSCount += inc; 
        if (dataRTSCount > MAX_DATA_RTS_COUNT)
            dataRTSCount = MAX_DATA_RTS_COUNT;
    }
    
    void IncSignallingRTSCount(int inc)
    {
        signallingRTSCount += inc;
        if (signallingRTSCount > MAX_SIGNALLING_RTS_COUNT)
            signallingRTSCount = MAX_SIGNALLING_RTS_COUNT;
    }
    
    void DecDataRTSCount()
    {
        dataRTSCount--;
        if (dataRTSCount < 0)
            dataRTSCount = 0; 
    }
    
    void DecSignallingRTSCount()
    {
        signallingRTSCount--;
        if (signallingRTSCount < 0)
            signallingRTSCount = 0;
    }
     
    // Channel configuration accessors
    void SetChannelComb(int chanComb)
    {
        channelComb = chanComb;
    }
    
    int GetChannelComb()
    {
        return (channelComb);
    }
     
    // Functions related to RTI allocation.
    unsigned char GetNextRTIVal()
    {
        rti = rti++ % MAX_RTI_VAL;
        return (rti);
    }

    ScheduleQueue* GetQueue(LinkDirection dir) 
    {
        return(scheduleInfo.GetQueue(dir));
    }

    ScheduleInfo* GetScheduleInfo(void)
    {
        return (&scheduleInfo);
    }

    void ShowTimeslotInfo();
    
private:
    int                     channelComb;
    int                     dataRTSCount;
    int                     signallingRTSCount;
    TAITable                sysTAI;
    USFTable                sysUSF;
    DLSignallingMsgQueue    sysDLSigMsgQ;
    unsigned char           rti;
    ScheduleInfo            scheduleInfo;
};


#endif