// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// File        : CdrRefSem.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : implementation of the CdrRefSem class.
//
// *******************************************************************

#include "CdrRefSem.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Primary behaviour methods
//////////////////////////////////////////////////////////////////////

ubyte4 CdrRefSem::allocateRef ()
{
  ubyte4 newRef;

    mutex_.take ();

    newRef = CdrRef::allocateRef();

    mutex_.give ();

  return (newRef);
}
