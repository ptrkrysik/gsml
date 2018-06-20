#ifndef __CDRREFSEM_H__
#define __CDRREFSEM_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : CdrRefSem.h
// Author(s)   : Igal Gutkin
// Create Date : 11/08/00
// Description : Interface for the CdrRefSem class 
//               Provides semaphore protection in the multitask environment
// *******************************************************************

#include "Os\JCMutex.h"

#include "CdrCommon.h"

#include "CdrRef.h"

class CdrRefSem : public CdrRef
{
public:
             CdrRefSem () : CdrRef (), mutex_() {}
    virtual ~CdrRefSem () {}

    ubyte4   allocateRef ();

// data members
private:
    // Preserves generation of unique CDR ref numbers
    JCMutex mutex_;
};


#endif // __CDRREFSEM_H__