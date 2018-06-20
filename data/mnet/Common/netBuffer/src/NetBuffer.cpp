// *******************************************************************
//
// (c) Copyright CISCO Systems, 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : NetBuffer.cpp
// Author(s)   : Igal Gutkin
// Create Date : 12/29/00
// Description : NetBuffer class implementation. Utilisation of the 
//               buffering using m-Block concept.
//
// *******************************************************************


#include "NetBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usrLib.h>

extern "C" void netPoolShow (NET_POOL_ID pNetPool);

#include "bufConfig.h"


//===================================================================
// Static Data members definition
//===================================================================

NetBuffer  *NetBuffer::pNetBuff_     = NULL;
unsigned    NetBuffer::poolNum_      = 0   ;


M_CL_CONFIG NetBuffer::mClBlkConfig_ =
{
/* mBlkNum     clBlkNum    memArea     memsize    */
/* -------     ----------  ----------  ---------- */
   CALC_VALUE, CALC_VALUE, CALC_VALUE, CALC_VALUE
};



//===================================================================
// Constructors & Destractor
//===================================================================

NetBuffer::NetBuffer ()
    :pNetPool_ (NULL),
     mutex     ()
{
  int           index  = 0;
  int           clDescTblNumEnt     ;

    mutex.take ();

    // allocate memory for mBlk and clBlk pools
    if ((mClBlkConfig_.memArea=(char *)malloc(mClBlkConfig_.memSize)) == NULL)
    {
        printf     ("FATAL ERROR (netPoolInit):Unable to allocate memory for block pools\n");
        printErrno (0);
        return;
    }

    // allocate memory for cluster pools
    for (index = 0; index < poolNum_; ++index)
    {
        if ((clDescTbl_[index].memArea=(char *)malloc(clDescTbl_[index].memSize)) == NULL)
        {
            printf ("FATAL ERROR (netPoolInit):Unable to allocate memory for memory cluster pools\n");
            printErrno (0);
            return;
        }
    }

    clDescTblNumEnt = NELEMENTS(clDescTbl_);

    pNetPool_ = &netPool_;

    if (netPoolInit (pNetPool_, &mClBlkConfig_, &clDescTbl_[0], clDescTblNumEnt, NULL) != OK)
    {
        printf ("FATAL ERROR (netPoolInit): Unable to initialize buffer pool.\n");
        printErrno (0);
    }

    mutex.give();
}


NetBuffer::~NetBuffer ()
{
  int index    ;

    /* delete network pool */
    if (netPoolDelete (pNetPool_) != OK)
    {
        printErrno (0);
    }

    /* release allocated memory */

    // mBlk and clBlk pools
    if (mClBlkConfig_.memArea)
    {
        free (mClBlkConfig_.memArea);
    }

    // cluster pools
    for (index = 0; index < poolNum_; ++index)
    {
        if (clDescTbl_[index].memArea)
            free (clDescTbl_[index].memArea);
    }
}


//===================================================================
// primary behavioural methods
//===================================================================

NetBuffer *NetBuffer::getInstance()
{
    return ((pNetBuff_) ? pNetBuff_ : init());
}


void NetBuffer::release ()
{
    if (pNetBuff_)
    {
        delete pNetBuff_;
        pNetBuff_ = NULL;
        poolNum_  = 0   ;
    }
}


// allocate mBlk->clBlk->cluster from the pool
// returns zero on error
unsigned NetBuffer::getBuffer  (unsigned bufSize, BUFFER_ID& pMBlk)
{
  unsigned retVal = 0;

    if (isInit ())
    {
        // M_BLK_ID netTupleGet (
        //              NET_POOL_ID pNetPool, /* pointer to the net pool    */
        //              int         bufSize , /* size of the buffer to get  */
        //              int         canWait , /* wait or dontwait           */
        //              UCHAR       type,   , /* type of data               */
        //              BOOL        bestFit ) /* TRUE/FALSE                 */

        mutex.take ();
        retVal = ((pMBlk=netTupleGet(pNetPool_,bufSize,M_DONTWAIT,MT_DATA,FALSE)) != NULL)
               ? pMBlk->pClBlk->clSize 
               : 0                    ;
        mutex.give ();

#ifdef __STDIOUT
        if (retVal)
            printf ("Buffers: allocated new block: ID %p, Pool ID %p, Ref.Count %u\n",
                    pMBlk, pMBlk->pClBlk->pNetPool, pMBlk->pClBlk->clRefCnt);
#endif //__STDIOUT
    }
    else
        pMBlk = NULL;

  return (retVal);
}


unsigned NetBuffer::getBufferSize (BUFFER_ID pSrc)
{
  unsigned retVal = 0;

    if (isValidBuffer(pSrc))
        retVal = pSrc->pClBlk->clSize;

  return (retVal);
}


bool NetBuffer::duplicateBuffer (BUFFER_ID pSrc, BUFFER_ID& pDest)
{
  bool retVal = false;

    if (isValidBuffer (pSrc))
    {
        pDest = netMblkGet(pNetPool_,M_WAIT,pDest->mBlkHdr.mType);

        // allocate new mBlock
        if (pDest != NULL)
        {
            mutex.take ();
            retVal = ((pDest=netMblkDup(pSrc, pDest)) != NULL);
            mutex.give ();

#ifdef __STDIOUT
            if (retVal)
                printf ("Buffers: allocated duplicated block: ID %p, Pool ID %p, Ref.Count %u\n",
                        pDest, pDest->pClBlk->pNetPool, pDest->pClBlk->clRefCnt);
#endif // __STDIOUT
        }
    }
    else
        pDest = NULL;

  return (retVal);
}


bool NetBuffer::freeBuffer (BUFFER_ID pMblk)
{
  bool retVal = false;

    if (isValidBuffer(pMblk))
    {
#ifdef __STDIOUT
    printf ("Buffers: releasing buffer ID %p, Pool ID %p, Ref.Count %u\n",
            pMblk, pMblk->pClBlk->pNetPool, pMblk->pClBlk->clRefCnt);
#endif // __STDIOUT

        mutex.take ();
        netMblkClFree (pMblk);
        mutex.give ();
        retVal = true;
    }

  return (retVal);
}


// Return pointer to the data and the data length
// Returns NULL on error
char *NetBuffer::getDataPtr (BUFFER_ID pMblk, unsigned& dataLen)
{
  char * pRetVal = NULL;

    if (isValidBuffer (pMblk))
    {
        dataLen = pMblk->mBlkHdr.mLen ; 
        pRetVal = pMblk->mBlkHdr.mData;
    }
    else
        dataLen = 0;

  return (pRetVal);
}


bool NetBuffer::getDataSize (BUFFER_ID pMblk, unsigned& dataLen)
{
  bool retVal = false;

    if (isValidBuffer (pMblk))
    {
        dataLen = pMblk->mBlkHdr.mLen ; 
        retVal  = true;
    }
    else
        dataLen = 0;

  return (retVal);
}


bool NetBuffer::setDataSize (BUFFER_ID pMblk, unsigned dataLen)
{
  bool retVal = false;

    if (isValidBuffer(pMblk))
    {
#ifdef __STDIOUT
        printf ("Buffers: set size: Block ID %p, Pool ID %p, Cluster size %u, Data size %u\n",
                pMblk, pMblk->pClBlk->pNetPool, pMblk->pClBlk->clSize, dataLen);
#endif //__STDIOUT

        pMblk->mBlkHdr.mLen = dataLen; 
        retVal  = true;
    }

  return (retVal);
}


// Copy data from the user buffer to the memory cluster
// If the data length > than the cluster size, returns false. Data is not copied
bool NetBuffer::setData (BUFFER_ID pMblk, unsigned dataLen, char *pBuff)
{
  bool retVal = false;

    if (isValidBuffer (pMblk)           &&
        dataLen <= pMblk->pClBlk->clSize   )
    {
#ifdef __STDIOUT
        printf ("Buffers: set size: Block ID %p, Pool ID %p, Cluster size %u Data size %u\n",
                pMblk, pMblk->pClBlk->pNetPool, pMblk->pClBlk->clSize, dataLen);
#endif //__STDIOUT

        memcpy (pMblk->mBlkHdr.mData, pBuff, dataLen);
        pMblk->mBlkHdr.mLen = dataLen                ;
        retVal  = true                               ;
    }

  return (retVal);
}


// Copy data from the memory cluster to the user buffer
// If the user buffer is smaller than the actual size of the received data, 
// only part of the data is copied, the actual size of the data is returned using dataLen
// and returns false.
bool NetBuffer::getData (BUFFER_ID pMblk, unsigned& dataLen, char *pBuff, unsigned maxLen)
{
  bool     retVal = false;
  unsigned tmpLen ;

    if (isValidBuffer (pMblk))
    {
        if (retVal = (pMblk->mBlkHdr.mLen <= maxLen))
        {
            dataLen = netMblkToBufCopy (pMblk, pBuff, NULL);
        }
        else
        {
            memcpy (pBuff, pMblk->mBlkHdr.mData, maxLen);
            dataLen = pMblk->mBlkHdr.mLen               ;
        }
    }

  return (retVal);
}


void NetBuffer::printInfo  ()
{
    if (isInit ())
    {
        netPoolShow (getInstance()->pNetPool_);
    }
    else
        printf ("NetBuffer subsystem is not initialized\n");
}

//===================================================================
// private methods
//===================================================================

NetBuffer *NetBuffer::init ()
{
  int           index  = 0;
  unsigned      uNum   = 0;

    if (!pNetBuff_)
    {
        poolNum_ = NELEMENTS(clDescTbl_);

        // Calculate the total number of required clBlks
        for (index = 0; index < poolNum_; ++index)
        {
            uNum += clDescTbl_[index].clNum;
        }

        mClBlkConfig_.clBlkNum = uNum;

        uNum = NETBUFF_M_BLK_NUM;

        // allocated number of mBlks >= number of clBlks
        mClBlkConfig_.mBlkNum  = (uNum >= mClBlkConfig_.clBlkNum) ? uNum : mClBlkConfig_.clBlkNum;

        // calculate pool size according to the required number of mBlks and clBlks
        mClBlkConfig_.memSize = (mClBlkConfig_.mBlkNum  * (M_BLK_SZ + sizeof(long)))
            + mClBlkConfig_.clBlkNum * CL_BLK_SZ;

#ifdef __STDIOUT
        printf ("Buffers: allocate %u mBlocks, %u Cluster Blocks (ratio %.2f), %u pools\n", 
                mClBlkConfig_.mBlkNum, mClBlkConfig_.clBlkNum, 
                ((double)(mClBlkConfig_.mBlkNum))/mClBlkConfig_.clBlkNum, poolNum_);
#endif //__STDIOUT

        // calculate memory size for the pool of clusters
        for (index = 0; index < poolNum_; ++index)
        {
            clDescTbl_[index].memSize = (clDescTbl_[index].clNum * (clDescTbl_[index].clSize + sizeof(long)));
        }

        pNetBuff_ = new NetBuffer;
    }

  return (pNetBuff_);
}


//===================================================================
// Shell accessible methods
//===================================================================

void netBufPrintInfo (void)
{
    NetBuffer::printInfo ();
}

