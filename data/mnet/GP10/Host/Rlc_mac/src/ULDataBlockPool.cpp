// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ULDataBlockPool.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "ULDataBlockPool.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"

DbgOutput ULDataBlock::MsgDisplayObj("RLC/MAC Msg", RLC_MAC_MSG);


void ULDataBlock::DisplayDataBlock()
{
    MsgDisplayObj.Trace("UPLINK RLC DATA BLOCK\n");
    MsgDisplayObj.HexDump(streamIn.GetBitStream(),
        streamIn.GetStreamLen());
}



ULDataBlockPool::ULDataBlockPool()
{
    // Initialize head and tail pointer.
    head = tail = 0;
    
    // Allocate enough uplink blocks to handle max uplink TBF's.
    for (int i=0; i < MAX_TFI * MAX_TRX * MAX_RL_BLOCKS_PER_PDU; i++)
    {
        ULDataBlock *pUlDataBlock = new ULDataBlock();
        // Remove data blocks from the head.
        if ((head != tail) || ((head == tail) && (head!=0)))
        {   
            tail->next = pUlDataBlock;
            pUlDataBlock->next = 0;
            tail = pUlDataBlock;
        }
        else
        {
            // Handle special case where list is empty.
            head = tail = pUlDataBlock;
            pUlDataBlock->next = 0;
        }
    }
}


ULDataBlockPool::~ULDataBlockPool()
{
    while (head->next)
    {
        ULDataBlock *pUlDataBlock = head;
        head = pUlDataBlock->next;
        delete (pUlDataBlock);
    }
}


ULDataBlock *ULDataBlockPool::GetULDataBlock()
{
    DBG_FUNC("ULDataBlockPool::GetULDataBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    ULDataBlock *pUlDataBlock;
    
    // Remove data blocks from the head.
    if (head != tail)
    {   
        pUlDataBlock = head;
        head = pUlDataBlock->next;
    }
    else
    {
        pUlDataBlock = head;
        head = tail = 0;
    }
    
    DBG_LEAVE();
    return (pUlDataBlock);
}


void ULDataBlockPool::ReturnULDataBlock(ULDataBlock *pUlDataBlock)
{
    DBG_FUNC("ULDataBlockPool::ReturnULDataBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    // Remove data blocks from the head.
    if ((head != tail) || ((head == tail) && (head!=0)))
    {   
        tail->next = pUlDataBlock;
        pUlDataBlock->next = 0;
        tail = pUlDataBlock;
    }
    else
    {
        // Handle special case where list is empty.
        head = tail = pUlDataBlock;
        pUlDataBlock->next = 0;
    }
    
    // Reset the length octet pointer.
    pUlDataBlock->ulHeader.ResetCurrLengthOctet();
    
    DBG_LEAVE();
    return;
}
