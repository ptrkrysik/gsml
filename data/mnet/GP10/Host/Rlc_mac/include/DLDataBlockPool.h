// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLDataBlockPool.h
// Author(s)   : Tim Olson
// Create Date : 1/5/2001
// Description : 
//
// *******************************************************************


#ifndef __DLDATABLOCKPOOL_H__
#define __DLDATABLOCKPOOL_H__

#include "bitstream.h"
#include "MacHeader.h"
#include "ieChannelCodingCommand.h"

// Max RLC PDU size is 1560.  Minus header for CS-1 18 bytes may be sent.
// For safety add a few extra and 100 should be good.
#define MAX_RL_BLOCKS_PER_PDU 	100 
// *******************************************************************
// class DLDataBlock
//
// Description
//    
// *******************************************************************
class DLDataBlock {
public:
	DLDataBlock(): next(0), streamOutLen(0), dlHeaderLen(0) {}
	~DLDataBlock() {}
    void DisplayDataBlock();

	friend class DLDataBlockPool;
    
	unsigned char	streamOut[MAX_RLC_DATA_MSG];
    int             streamOutLen;
	DLMacHeader     dlHeader;
    int             dlHeaderLen;
    IeChannelCodingCommand  chanCoding;
private:
	DLDataBlock *next;
    static DbgOutput MsgDisplayObj;
};


// *******************************************************************
// class DLDataBlockPool
//
// Description
//    
// *******************************************************************
class DLDataBlockPool {
public:
    // Constructor
    DLDataBlockPool();
    // Destructor
    ~DLDataBlockPool();
    
    DLDataBlock *GetDLDataBlock();
    void ReturnDLDataBlock(DLDataBlock *pDlDataBlock);
    
private:
	DLDataBlock *head, *tail;
};


#endif