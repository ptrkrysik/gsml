// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ULDataBlockPool.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __ULDATABLOCKPOOL_H__
#define __ULDATABLOCKPOOL_H__

#include "bitstream.h"
#include "MacHeader.h"
#include "ieChannelCodingCommand.h"

// Max RLC PDU size is 1560.  Minus header for CS-1 18 bytes may be sent.
// For safety add a few extra and 100 should be good.
#define MAX_RL_BLOCKS_PER_PDU 	100 
// *******************************************************************
// class ULDataBlock
//
// Description
//    
// *******************************************************************
class ULDataBlock {
public:
	ULDataBlock(): next(0) {}
	~ULDataBlock() {}

	friend class ULDataBlockPool;
    
    int GetDataBlockLength() { return (chanCoding.GetRLCDataBlockSize()); }
    void DisplayDataBlock();

	BitStreamIn	streamIn;
	ULMacHeader ulHeader;
    IeChannelCodingCommand  chanCoding;
private:
	ULDataBlock *next;
    static DbgOutput MsgDisplayObj;
};


// *******************************************************************
// class ULDataBlockPool
//
// Description
//    
// *******************************************************************
class ULDataBlockPool {
public:
    // Constructor
    ULDataBlockPool();
    // Destructor
    ~ULDataBlockPool();
    
    ULDataBlock *GetULDataBlock();
    void ReturnULDataBlock(ULDataBlock *pUlDataBlock);
    
private:
	ULDataBlock *head, *tail;
};


#endif