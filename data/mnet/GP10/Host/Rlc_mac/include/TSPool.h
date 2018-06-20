// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : TSPool.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __TSPOOL_H__
#define __TSPOOL_H__

#include "Timeslot.h"

#define INVALID_TFI 0xff

 
// *******************************************************************
// class TSPool
//
// Description
//    
// *******************************************************************
class TSPool {
public:
    TSPool();
    ~TSPool() {}
    
    Timeslot *GetTimeslotObj(int trx, int ts)
    {
        if (IsTrxTsValid(trx, ts))
            return (&sysTS[trx][ts]);        
        else
            return (0);
    }
    
    // Functions related to TAI
    unsigned char AllocateTAI(int trx, int ts, Tbf *pTbf);
    void ReleaseTAI(int trx, int ts, unsigned char tai);
    void ShowTAITable(int trx);
    void ShowTAITable(int trx, int ts);
    
    // Functions related to USF
    unsigned char AllocateUSF(int trx, int ts, ULTbf *pTbf);
    void ReleaseUSF(int trx, int ts, unsigned char usf);
    void ShowUSFTable(int trx);
    void ShowUSFTable(int trx, int ts);
    
    // Functions related to TFI assignment
    unsigned char AllocateULTFI(int trx, ULTbf *pUlTbf);
    void ReleaseULTFI(int trx, unsigned char tfi);
    void ShowULTFITable(int trx);
    ULTbf *FindULTbf(int trx, unsigned char tfi);
    unsigned char AllocateDLTFI(int trx, DLTbf *pDlTbf);
    void ReleaseDLTFI(int trx, unsigned char tfi);
    void ShowDLTFITable(int trx);
    DLTbf *FindDLTbf(int trx, unsigned char tfi);
    
    // Functions related to DL signalling queues
    JC_STATUS QueueDLSignallingMsg
    (
        int trx, 
        int ts, 
        MsgPacchReq *l1Msg    
    );
    MsgPacchReq *DequeueDLSignallingMsg
    (
        int trx, 
        int ts 
    );
    
    // Functions related to RTS counts
    int GetDataRTSCount(int trx, int ts) 
    {   
        if (IsTrxTsValid(trx, ts))
            return (sysTS[trx][ts].GetDataRTSCount());        
        else
            return (0);
    }
    
    int GetSignallingRTSCount(int trx, int ts) 
    {   
        if (IsTrxTsValid(trx, ts))
            return (sysTS[trx][ts].GetSignallingRTSCount());        
        else
            return (0);
    }
    
    void IncDataRTSCount(int trx, int ts, int inc)
    {
        if (IsTrxTsValid(trx, ts))
        {
            sysTS[trx][ts].IncDataRTSCount(inc);        
        }
    }
    
    void IncSignallingRTSCount(int trx, int ts, int inc)
    {
        if (IsTrxTsValid(trx, ts))
        {
            sysTS[trx][ts].IncSignallingRTSCount(inc);        
        }
    }
    
    void DecDataRTSCount(int trx, int ts)
    {
        if (IsTrxTsValid(trx, ts))
        {
            sysTS[trx][ts].DecDataRTSCount();        
        }
    }
    
    void DecSignallingRTSCount(int trx, int ts)
    {
        if (IsTrxTsValid(trx, ts))
        {
            sysTS[trx][ts].DecSignallingRTSCount();        
        }
    }
    
    // Channel configuration accessors
    void SetChannelComb(int trx, int ts, int chanComb)
    {
        if (IsTrxTsValid(trx, ts))
        {
            sysTS[trx][ts].SetChannelComb(chanComb);
        }
    }
    
    int GetChannelComb(int trx, int ts)
    {
        if (IsTrxTsValid(trx, ts))
        {
            return (sysTS[trx][ts].GetChannelComb());
        }
        return (INVALID_CHANNEL_COMBINATION);
    }
    
    void ShowChannelComb(int trx);
    void ShowChannelComb(int trx, int ts);
    
    // Functions related to RTI allocation.
    unsigned char GetNextRTIVal(int trx, int ts)
    {
        if (IsTrxTsValid(trx, ts))
        {
            return (sysTS[trx][ts].GetNextRTIVal());
        }
        return (INVALID_RTI);
    }

    ScheduleQueue* GetQueue(int trx, int ts, LinkDirection dir)    
    {
        if (IsTrxTsValid(trx, ts))
        {
            return (sysTS[trx][ts].GetQueue(dir));
        }
        else
        {
            return (0);
        }
    }

    ScheduleInfo* GetScheduleInfo(int trx, int ts) 
    {
        if (IsTrxTsValid(trx, ts))
        {
            return (sysTS[trx][ts].GetScheduleInfo());
        }
        else
        {
            return (0);
        }
    }

    // General Timeslot functions
    void ShowTimeslotInfo(int trx);
    void ShowTimeslotInfo(int trx, int ts);

private:

    Timeslot                sysTS[MAX_TRX][MAX_TIMESLOTS];
    ULTbf                   *sysUlTFI[MAX_TRX][MAX_TFI];
    DLTbf                   *sysDlTFI[MAX_TRX][MAX_TFI];
    
    // Private Methods
    inline bool IsTrxValid(int trx)
    {
        return ((0 <= trx) && (trx < MAX_TRX));
    }

    inline bool IsTrxTsValid(int trx, int ts)
    {
        return (((0 <= trx) && (trx < MAX_TRX)) && ((0 <= ts) && (ts < MAX_TIMESLOTS)));
    }

    inline bool IsTfiValid(unsigned char tfi)
    {
        return (tfi < MAX_TFI);
    }

};


#endif
