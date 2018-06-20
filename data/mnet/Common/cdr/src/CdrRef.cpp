// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// File        : CdrRef.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : implementation of the CdrRef class.
//
// *******************************************************************

#include "cdrRef.h"


//////////////////////////////////////////////////////////////////////
// Static Data members declarations
//////////////////////////////////////////////////////////////////////

ubyte4 CdrRef::lastRefNum_ = CDR_INVALID_REFERENCE;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Primary behaviour methods
//////////////////////////////////////////////////////////////////////

ubyte4 CdrRef::allocateRef ()
{
    if (++lastRefNum_ > CDR_MAX_REFERENCE)
        lastRefNum_ = CDR_MIN_REFERENCE;

  return (lastRefNum_);
}


ubyte4 CdrRef::getLastRef()
{
  return (lastRefNum_);
}
