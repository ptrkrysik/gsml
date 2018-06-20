// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ULRlcLink.h
// Author(s)   : Tim Olson
// Create Date : 12/1/2000
// Description : 
//
// *******************************************************************


#ifndef __ULRLCLINK_H__
#define __ULRLCLINK_H__

#include "Os/JCCTimer.h"
#include "RlcMacCommon.h"
#include "ULDataBlockPool.h"
#include "ieChannelCodingCommand.h"
#include "MsgPacketControlAcknowledgement.h"

#define RCV_WINDOW_SIZE_K               64
#define MAX_RLC_BLOCKS_BETWEEN_ACKS     (RCV_WINDOW_SIZE_K/2)
#define LAST_BLOCK_UNKNOWN              -1

#define T3169_VALUE     GetMibValueInt(MIB_t3191)*10     // in 100's of ms

// Forward declaration.
class ULTbf;

// *******************************************************************
// class ULRlcLink
//
// Description
//    
// *******************************************************************
class ULRlcLink {
public:

    typedef enum
    {
        ACKNOWLEDGED_RLC_MODE,
        UNACKNOWLEDGED_RLC_MODE
    } RLC_MODE;
    
    typedef enum
    {
        RECEIVED,
        INVALID
    } RECEIVE_STATUS;

    // Constructor
    ULRlcLink();
    // Destructor
    ~ULRlcLink() {}
     
    JC_STATUS EstablishRLCLink(ULTbf *pUlTbf, RLC_MODE mode);
    void UpdateTSSpecificData();
    JC_STATUS AbnormalRelease();
    JC_STATUS ReceiveULDataBlock(ULDataBlock *pUlDataBlock);
    bool UpdateRcvStateVariables(ULDataBlock *pUlDataBlock);
    void SendULAckNack();
    void ReassembleULPDU();
    bool IsPDUAssemblyNeeded(ULDataBlock *pUlDataBlock);
    void ReleaseDataBlocks(int startBlock, int endBlock);
    bool ScanDataBlocks(int currDataBlock);
    void SendBSSGPUnitData(unsigned char *pBuf, int len);
    void ReceivePacketControlAck(MsgPacketControlAcknowledgement *pCtrlMsg);
    void ReceivePacketControlNack();
    void ProcessT3169Expiry();
    void ProcessActivityTimerExpiry();
    bool IncrementN3101();
    void SetCurrentCodingRate(IeChannelCodingCommand::CHANNEL_CODING_COMMAND rate) 
    { chanCoding.SetChannelCodingCommand(rate); }
    IeChannelCodingCommand::CHANNEL_CODING_COMMAND GetCurrentCodingRate() 
    { return(chanCoding.GetChannelCodingCommand()); }
    
    void ShowRlcLinkInfo();
    
    static ULDataBlockPool sysUlDataBlockPool;
    static int T3169TimerExpiry(int tbfPointer);
    static int ActivityTimerExpiry(int tbfPointer);
    
private:
    RLC_MODE            rlcMode;            // ACK or non ACK mode
    int                 k;                  // Window size
    int                 vQ;                 // Receive window state
    RECEIVE_STATUS      vN[RLC_MOD_VAL];    // Receive state array
    int                 vR;                 // Receive state variable
    int                 n3103;
    int                 n3101;
    int                 n3101FirstBlkBuffer;
    IeChannelCodingCommand chanCoding;
    
    ULDataBlock         *pUlBlocks[RLC_MOD_VAL];
    int                 firstDataBlock;
    int                 lastDataBlock;
    int                 numRLCBlocksSinceLastAck;
    bool                endOfTBF;
    ULTbf               *pAssociatedTbf;
    
    static int          N3103_MAX;
    static int          N3101_MAX;
    JCCTimer            *pT3169;
    JCCTimer            *pActivityTimer;
};


#endif