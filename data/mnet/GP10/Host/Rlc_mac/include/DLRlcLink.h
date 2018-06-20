// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLRlcLink.h
// Author(s)   : Tim Olson
// Create Date : 12/1/2000
// Description : 
//
// *******************************************************************


#ifndef __DLRLCLINK_H__
#define __DLRLCLINK_H__

#include "Os/JCCTimer.h"
#include "RlcMacCommon.h"
#include "DLDataBlockPool.h"
#include "bssgp/bssgp_api.h"
#include "ieChannelCodingCommand.h"
#include "MsgPacketDownlinkAckNack.h"
#include "MsgPacketControlAcknowledgement.h"

#define XMT_WINDOW_SIZE_K   64

// Forward declaration.
class DLTbf;

#define T3191_VALUE     GetMibValueInt(MIB_t3191)*10  // in 100's of ms
#define T3193_VALUE     GetMibValueInt(MIB_t3193)     // in 100's of ms
#define T3195_VALUE     GetMibValueInt(MIB_t3195)*10  // in 100's of ms

// *******************************************************************
// class DLRlcLink
//
// Description
//    
// *******************************************************************
class DLRlcLink {
public:

    typedef enum
    {
        ACKNOWLEDGED_RLC_MODE,
        UNACKNOWLEDGED_RLC_MODE
    } RLC_MODE;

    typedef enum
    {
        NACKED,
        PENDING_ACK,
        ACKED,
        INVALID
    } ACKNOWLEDGE_STATUS;

    // Constructor
    DLRlcLink();
    // Destructor
    ~DLRlcLink() {}
     
    JC_STATUS EstablishRLCLink(DLTbf *pDlTbf, RLC_MODE mode);
    JC_STATUS ReestablishRLCLink(DLTbf *pDlTbf, RLC_MODE mode);
    void UpdateTSSpecificData();
    JC_STATUS AbnormalRelease();
    DLDataBlock *GetDLRlcBlock(unsigned char &ctrlAckTag, char &burstType, int reqTrx, int reqTs);
    bool SetPollingBit(DLDataBlock *pDlDataBlock, unsigned char &ctrlAckTag, char &burstType, int reqTrx, int reqTs); 
    DLDataBlock *BuildDLRlcBlock(); 
    bool SegmentLLCPdu(DLDataBlock *pDlDataBlock, int &bytesCopied);
    bool ReallocateDLResources();
    DLDataBlock *RetransmitNackedDLRlcBlock();
    DLDataBlock *RetransmitPendingAckDLRlcBlock();
    void ReceiveDLAckNack(MsgPacketDownlinkAckNack *pCtrlMsg);
    void ReceivePacketControlAck(MsgPacketControlAcknowledgement *pCtrlMsg, unsigned char ta);
    void ReceivePacketControlNack();
    JC_STATUS SendPacketPowerControlTimingAdvance(unsigned char ta);
    JC_STATUS SendPacketPollingRequest();
    bool GetNextLLCPdu();
    bool MoreLLCPdusAvailable(BSSGP_DL_UNITDATA_MSG **pDlUnitData);
    void ProcessT3195Expiry();
    void ProcessT3191Expiry();
    void ProcessT3193Expiry();
    void ProcessActivityTimerExpiry();
    void ReleaseDataBlocks();
    void SetCurrentCodingRate(IeChannelCodingCommand::CHANNEL_CODING_COMMAND rate) 
    { chanCoding.SetChannelCodingCommand(rate); }
    IeChannelCodingCommand::CHANNEL_CODING_COMMAND GetCurrentCodingRate() 
    { return(chanCoding.GetChannelCodingCommand()); }
    void ShowRlcLinkInfo();
    bool IsT3193Running() { return (isT3193Running); }
    void CancelT3193() { pT3193->cancelTimer(); isT3193Running = FALSE; }
    
    static int T3195TimerExpiry(int tbfPointer);
    static int T3191TimerExpiry(int tbfPointer);
    static int T3193TimerExpiry(int tbfPointer);
    static int ActivityTimerExpiry(int tbfPointer);
    static DLDataBlockPool sysDlDataBlockPool;
    
private:
    RLC_MODE                rlcMode;            // ACK or non ACK mode
    int                     k;                  // Window size
    int                     vS;                 // Send state variable
    int                     vCS;                // Control send state variable
    DLDataBlock             *pDlBlocks[RLC_MOD_VAL];
    int                     lastPendingAckXmit;
    bool                    lastBlockSentOnPollTs;
    int                     lastDataBlock;
    IeChannelCodingCommand  chanCoding;
    int                     n3105;
    int                     numBlocksSincePoll;
    int                     dlAssignCnt;
    bool                    pollOnActivation;
    
    // Only valid for acknowledged mode.
    int                 vA;                 // Acknowledged state variable
    ACKNOWLEDGE_STATUS  vB[RLC_MOD_VAL];    // Acknowledged state array
    DLTbf               *pAssociatedTbf;
    
    // Next PDU for segmentation.
    char *              llcPdu;
    int                 llcPduLen;
    int                 currPduByte;
    
    int                 N3105_MAX;
    static int          dlAssignCntMax;
    JCCTimer            *pT3195;
    JCCTimer            *pT3191;
    JCCTimer            *pT3193;
    JCCTimer            *pActivityTimer;
    bool                isT3193Running;
};


#endif