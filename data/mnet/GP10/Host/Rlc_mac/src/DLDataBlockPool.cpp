// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : DLDataBlockPool.cpp
// Author(s)   : Tim Olson
// Create Date : 1/5/2001
// Description : 
//
// *******************************************************************


#include "DLDataBlockPool.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"

DbgOutput DLDataBlock::MsgDisplayObj("RLC/MAC Msg", RLC_MAC_MSG);


void DLDataBlock::DisplayDataBlock()
{
    MsgDisplayObj.Trace("DOWNLINK RLC DATA BLOCK\n");
    MsgDisplayObj.HexDump(streamOut, streamOutLen);
}

DLDataBlockPool::DLDataBlockPool()
{
    // Initialize head and tail pointer.
    head = tail = 0;
    
    // Allocate enough uplink blocks to handle max downlink TBF's.
    for (int i=0; i < MAX_TFI * MAX_TRX * MAX_RL_BLOCKS_PER_PDU; i++)
    {
        DLDataBlock *pDlDataBlock = new DLDataBlock();
        // Remove data blocks from the head.
        if ((head != tail) || ((head == tail) && (head!=0)))
        {   
            tail->next = pDlDataBlock;
            pDlDataBlock->next = 0;
            tail = pDlDataBlock;
        }
        else
        {
            // Handle special case where list is empty.
            head = tail = pDlDataBlock;
            pDlDataBlock->next = 0;
        }
    }
}


DLDataBlockPool::~DLDataBlockPool()
{
    while (head->next)
    {
        DLDataBlock *pDlDataBlock = head;
        head = pDlDataBlock->next;
        delete (pDlDataBlock);
    }
}


DLDataBlock *DLDataBlockPool::GetDLDataBlock()
{
    DBG_FUNC("DLDataBlockPool::GetDLDataBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    DLDataBlock *pDlDataBlock;
    
    // Remove data blocks from the head.
    if (head != tail)
    {   
        pDlDataBlock = head;
        head = pDlDataBlock->next;
    }
    else
    {
        pDlDataBlock = head;
        head = tail = 0;
    }
    
    DBG_LEAVE();
    return (pDlDataBlock);
}


void DLDataBlockPool::ReturnDLDataBlock(DLDataBlock *pDlDataBlock)
{
    DBG_FUNC("DLDataBlockPool::ReturnDLDataBlock", RLC_MAC_RLC_LINK);
    DBG_ENTER();
    
    // Remove data blocks from the head.
    if ((head != tail) || ((head == tail) && (head!=0)))
    {   
        tail->next = pDlDataBlock;
        pDlDataBlock->next = 0;
        tail = pDlDataBlock;
    }
    else
    {
        // Handle special case where list is empty.
        head = tail = pDlDataBlock;
        pDlDataBlock->next = 0;
    }
    
    // Reset the length octet pointer.
    pDlDataBlock->dlHeader.ResetCurrLengthOctet();
    
    DBG_LEAVE();
    return;
}
