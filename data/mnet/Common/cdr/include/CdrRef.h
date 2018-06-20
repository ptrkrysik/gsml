#ifndef __CDRREF_H__
#define __CDRREF_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : cdrRef.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : interface for the CdrRef class
//
// *******************************************************************

#include "CdrCommon.h"

class CdrRef
{
public:
           CdrRef() {;}
  virtual ~CdrRef() {;}

  static ubyte4 allocateRef ();
  static ubyte4 getLastRef  ();

// data members
private:
  static ubyte4 lastRefNum_;
};


#endif // __CDRREF_H__