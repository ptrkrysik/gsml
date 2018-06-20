// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacTask.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************
#ifndef __RLCMACTASK_H__
#define __RLCMACTASK_H__

#include <msgQLib.h>
#include "Os/JCTask.h"
#include "GP10Err.h"
#include "Rlc_mac/RlcMacIntf.h"
#include "ril3/msg_channel_request.h"
#include "MacHeader.h"
#include "msgL1.h"
#include "TbfPool.h"
#include "TSPool.h"
#include "ULDataBlockPool.h"
#include "MsgBase.h"
#include "MsgPacketResourceRequest.h"
#include "ScheduleAllocationInfo.h"
#include "MsgPacketTBFRelease.h"
#include "MsgPacketDownlinkAckNack.h"

typedef enum
{
    RESOURCES_UNAVAILABLE,
    RESOURCES_ALLOCATED,
    TBF_ACTIVE
} DL_ALLOC_RES;

class RlcMacTask {
public:

    static int InitRlcMac(void);
    static int RlcMacLoop(void);
        
    // OAM functions
    JC_STATUS DecodeOamAdminMsgInd(RlcMacAdminStateChangeType *msgReq);
    JC_STATUS DecodeOamOpMsgInd(RlcMacOpStateChangeType *msgReq);
    
    // RM MD message handling
    JC_STATUS ProcessRmMdMsgInd(T_CNI_RIL3MD_RRM_MSG *msgReq);
    JC_STATUS ProcessChanReq(T_CNI_RIL3MD_RRM_MSG *msgReq);
    
    // MAC allocation
    JC_STATUS ULOnePhaseAllocation(T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST *chanReq, T_CNI_RIL3MD_RRM_MSG *msgReq);
    JC_STATUS ULCCCHImmediateAssignment(ULTbf *pUlTbf, 
        T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST *chanReq, T_CNI_RIL3MD_RRM_MSG *msgReq);
    
    JC_STATUS ULSingleBlockAllocation
    (
        T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST *chanReq,
        T_CNI_RIL3MD_RRM_MSG *msgReq
    );
    JC_STATUS SendPacketAccessReject(int trx, int ts, IeBase *pIe,
        MS_ID_TYPE idType);
    JC_STATUS SendPacketUplinkAssignment(int trx, int ts, ULTbf *pUlTbf, IeBase *pIe,
        MS_ID_TYPE idType); 
    JC_STATUS SendPacketTBFRelease(DLTbf *pDlTbf, ULTbf *pUlTbf,
        MsgPacketTBFRelease::TBF_RELEASE_CAUSE cause); 
    
        
    JC_STATUS AllocateULResources
    (
        MsgPacketResourceRequest &msg, 
        int trx, 
        int ts 
    ); 
    JC_STATUS SendPacketDownlinkAssignment(DLTbf *pDlTbf, ULTbf *pUlTbf, bool controlAck); 
    JC_STATUS SendPacketDownlinkAssignment(DLTbf *pDlTbf, int trx, int ts, bool controlAck);
    DL_ALLOC_RES AllocateDLResources(BSSGP_DL_UNITDATA_MSG *pUnitDataMsg); 
    JC_STATUS CheckDLMsgQueues(); 
    JC_STATUS DLCCCHImmediateAssignment(DLTbf *pDlTbf);
    
        
    // Layer 1 message handling
    JC_STATUS DecodeL1MsgInd(RlcMacL1MsgIndType *msgReq);
    JC_STATUS ProcessPhPdtchInd(MsgPdtchInd *l1PdtchInd);
    JC_STATUS ProcessPhPacchInd(MsgPacchInd *l1PacchInd);
    JC_STATUS ProcessPhRTSInd(MsgReadyToSend *pL1RTSInd);
    bool SendDownlinkDummyControlBlock(int trx, int ts);
    bool SendDownlinkDataBlock(int trx, int ts);
    void SendULActivationMsg(ULTbf *pUlTbf);
    void SendULReconfigurationMsg(ULTbf *pUlTbf);
    void SendULDeactivationMsg(ULTbf *pUlTbf);
    void SendDLActivationMsg(DLTbf *pDlTbf);
    void SendDLReconfigurationMsg(DLTbf *pDlTbf);
    void SendDLDeactivationMsg(DLTbf *pDlTbf);
    void SendSingleBlockAssignAlertMsg(int trx, int ts, unsigned int frameNum);
    void SendDLAssignAlertMsg(int trx, int ts, unsigned int frameNum, char tag);
    void SendDLAckNackInfoMsg(int trx, DLTbf *pDlTbf, MsgPacketDownlinkAckNack *pMsg);
    
    JC_STATUS SendPacchMsg
    (
        int trx, 
        int ts, 
        DLMacHeader *pDlMsgHdr,
        DownlinkMsgBase *pDlMsg,
        unsigned char ctlAckTag,
        char burstType
    ); 
    JC_STATUS SendPdtchMsg
    (
        int trx, 
        int ts, 
        unsigned char ctlAckTag,
        char burstType,
        DLDataBlock *pDlDataBlock
    ); 
    
    // RLC/MAC message handling
    JC_STATUS ProcessULRlcDataBlock
    (
        MsgPdtchInd *l1PdtchInd, 
        ULDataBlock *pUlDataBlock
    );
    JC_STATUS DecodeULRlcControlBlock
    (
        MsgPacchInd *l1PacchInd,
        ULMacHeader &rlcMacMsgHeader, 
        BitStreamIn &l1MsgBitsIn
    );
    JC_STATUS DecodeULRlcControlBlockAccessBurst
    (
        MsgPacchAbInd *l1PacchAbInd
    );
    
    
    // Uplink resource requests
    bool ULSingleBlockRequest(int *trx, int *ts);
    bool ULResourceReallocRequest(ULTbf *pUlTbf);
    bool ULResourceReallocRequest(ULTbf *pUlTbf, DLTbf *pDlTbf);
    bool ULResourceRequest(ULTbf *pUlTbf);
    bool ULResourceRequest(ULTbf *pUlTbf, DLTbf *pDlTbf);
    bool ULReleaseResourceRequest(ULTbf *pUlTbf);
    
    // Downlink resource requests
    bool DLSingleTSResourceRequest(DLTbf *pDlTbf);
    bool DLResourceReallocRequest(DLTbf *pDlTbf);
    bool DLResourceReallocRequest(DLTbf *pDlTbf, ULTbf *pUlTbf);
    bool DLResourceRequest(DLTbf *pDlTbf);
    bool DLResourceRequest(DLTbf *pDlTbf, ULTbf *pUlTbf);
    bool DLReleaseResourceRequest(DLTbf *pDlTbf);

    // Scheduling requests
    bool TSSelectionRequest(int *trx, int *ts, DLTbf *pDlTbf);
    bool TSSelectionRequest(int *trx, int *ts, ULTbf *pUlTbf);
    bool PollingTsCheck(int trx, int ts, DLTbf *pDlTbf);
    bool ScheduleRLCBlock(int trx, int ts, DLTbf **ppDlTbf);
    bool ScheduleRLCBlock(int trx, int ts, ULTbf **ppUlTbf);
    bool ScheduleRLCBlock(int trx, int ts, DLTbf **ppDlTbf, ULTbf **ppUlTbf);

    // Schedule Helpers
    bool AllocateUser (ScheduleAllocationInfo *schedAllocInfo);
    bool AllocateUserIfBetterQoS (ScheduleAllocationInfo *schedAllocInfo);
    bool AllocateOneSlot (ScheduleAllocationInfo *schedAllocInfo); 
    bool FindBestSlot (ScheduleAllocationInfo *schedAllocInfo); 
    bool DeallocateUser (Tbf* tbf, LinkDirection dir);
    bool GetLeastUtilizedSlot(Tbf* tbf, int *trxArg, int *tsArg);
    bool GetNaturalSlot(Tbf* tbf, int *trxArg, int *tsArg) ;
    int GetBtsGprsConfig(int trx, bool* tsMap, float* capUL, float* capDL);
    void UpdateAllowedSlots(ScheduleAllocationInfo *schedAllocInfo);
     
    TbfPool             tbfPool;
    TSPool              tsPool;
    static RlcMacTask   *theRlcMacTask;

private:

    static bool isCreated;
    JCTask RlcMacTaskObj;

};


#endif