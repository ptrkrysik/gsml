#ifndef __JCMUTEX_H__
#define __JCMUTEX_H__
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
// File        : JCMutex.h
// Author(s)   : Igal Gutkin
// Create Date : 11/06/00
// Description : JCMutex class (Mutual-Exclusion Semapore) definition
//
// *******************************************************************


#include <vxworks.h>
#include <semlib.h>

class JCMutex
{
public:
    // Constructors & destructor
             JCMutex ();
    virtual ~JCMutex ();

    // pramary methods
    bool take ();
    bool give ();

private:

// data members
private:

    SEM_ID semId_;
};

#endif //__JCMUTEX_H__