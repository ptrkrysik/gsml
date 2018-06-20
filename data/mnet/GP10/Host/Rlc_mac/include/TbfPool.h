// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : TbfPool.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __TBFPOOL_H__
#define __TBFPOOL_H__

#include "RlcMacCommon.h"
#include "ULTbf.h"
#include "DLTbf.h"
#include "ieGlobalTFI.h"
#include "ieTFI.h"


// *******************************************************************
// class TbfPool
//
// Description
//    
// *******************************************************************
class TbfPool {
public:
    TbfPool() {}
    ~TbfPool() {}
    
    // Functions related to administrative and op state changes
    void AbnormalReleaseBts();
    void AbnormalReleaseTrx(int trx);
    void AbnormalReleaseTs(int trx, int ts);
     
    // Functions related to uplink TBF
    ULTbf *EstablishULTbf(
        IeAccessType                      *accessType,
        IeTLLI                            *tlli, 
        IeChannelRequestDescription       *chanReqDesc,
        IeMSRadioAccessCapability         *radioAccessCapability
    );
    ULTbf *FindULTbf(IeGlobalTFI &globalTfi);
    ULTbf *FindULTbf(IeTFI &tfi);
    ULTbf *FindULTbf(unsigned long tlli);
    ULTbf *FindULTbf(unsigned char controlAckTag);
    
    // Functions related to downlink TBF
    DLTbf *EstablishDLTbf(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg, bool ctrlAck);
    DLTbf *FindDLTbf(IeGlobalTFI &globalTfi);
    DLTbf *FindDLTbf(IeTFI &tfi);
    DLTbf *FindDLTbf(unsigned long tlli);
    DLTbf *FindDLTbf(unsigned char controlAckTag);
    
    bool AnyActiveTBF(int trx, int ts);
    
    void ShowULAllocatedTBF();
    void ShowDLAllocatedTBF();
    void ShowULFreeTBF();
    void ShowDLFreeTBF();
    void ShowULAllTBF();
    void ShowDLAllTBF();
    
private:

    ULTbf       sysULTbf[MAX_TRX * MAX_TFI];
    DLTbf       sysDLTbf[MAX_TRX * MAX_TFI];
    
    inline bool IsTrxValid(int trx)
    {
        return (0 <= trx < MAX_TRX);
    }
};


#endif
