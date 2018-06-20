#ifndef __NETBUFFER_H__
#define __NETBUFFER_H__
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
// File        : NetBuffer.h
// Author(s)   : Igal Gutkin
// Create Date : 12/29/00
// Description : NetBuffer class definiton.
//
// *******************************************************************

#include <vxworks.h>
#include <netBufLib.h>

#include "Os\JCMutex.h"

#define CALC_VALUE          0
#define BUFFER_ID           M_BLK_ID

// shell accessible info method
extern "C" void netBufPrintInfo (void);

class NetBuffer
{
public:


    static 
    NetBuffer *getInstance();

    static
    bool       isInit     ()                { return (pNetBuff_ != NULL); }

    static
    void       release    ();

    static
    void       printInfo  ();

    // Check the Buffer
    bool       isValidBuffer (BUFFER_ID pMblk) { return (pNetBuff_ && pMblk      &&
                                                         pMblk->pClBlk->clRefCnt &&
                                                         pMblk->pClBlk->pNetPool == pNetPool_); }
    // Allocate new buffer
    // Returns the size of the allocated buffer.
    // On error returns zero and pMBlk == NULL
    // Allocated buffer size could be not the same size as requested!
    unsigned   getBuffer  (unsigned  bufSize, BUFFER_ID& pMBlk);

    // Returns total size of the associated memory buffer or zero on invalid buffer
    unsigned   getBufferSize (BUFFER_ID pMBlk);

    // Release buffer
    bool       freeBuffer (BUFFER_ID pMblk  );

    // Create new Buffer ID without replicating the data itself
    bool       duplicateBuffer (BUFFER_ID pSrc, BUFFER_ID& pDest);

    // Return a pointer to the data and the actual data size
    char      *getDataPtr (BUFFER_ID pMblk, unsigned& dataLen);

    // Provide the size of the data stored in the buffer
    // Return false on invalid buffer ID
    bool       getDataSize(BUFFER_ID pMblk, unsigned& dataLen);

    // Set the size of the data stored in the buffer
    // Return false on invalid buffer ID or data size exceeding buffer length
    bool       setDataSize(BUFFER_ID pMblk, unsigned  dataLen);

    // Copy data from the memory cluster to the user buffer
    bool       getData    (BUFFER_ID pMblk, unsigned& dataLen, char *pBuff, unsigned maxLen);

    // Copy data from the user buffer to the memory cluster
    bool       setData    (BUFFER_ID pMblk, unsigned  dataLen, char *pBuff);


protected:

    // Class Constractors & Destractor
    NetBuffer ();

    virtual
   ~NetBuffer ();

private  :
    static
    NetBuffer *init ();



// Data members
private:

    static NetBuffer   *pNetBuff_    ;

    static M_CL_CONFIG  mClBlkConfig_;
    static CL_DESC      clDescTbl_   [];

    static unsigned     poolNum_     ;

           NET_POOL     netPool_     ;
           NET_POOL_ID  pNetPool_    ;

           JCMutex      mutex        ;
}; 



#endif //__NETBUFFER_H__