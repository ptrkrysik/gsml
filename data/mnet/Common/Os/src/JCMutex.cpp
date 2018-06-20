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
// File        : JCMutex.cpp
// Author(s)   : Igal Gutkin
// Create Date : 11/06/00
// Description : JCMutex class (Mutual-Exclusion Semapore) implementation
//
// *******************************************************************

#include "Os\JCMutex.h"


JCMutex::JCMutex ()
{
    semId_ = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
}


bool JCMutex::take ()
{
    return (semId_ && semTake (semId_, WAIT_FOREVER) == OK);
}


bool JCMutex::give ()
{
    return (semId_ && semGive (semId_) == OK);
}


JCMutex::~JCMutex ()
{
    semDelete (semId_);
}
