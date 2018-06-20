#ifndef __DBGFUNC_H__
#define __DBGFUNC_H__
// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : dbgfunc.h
// Author(s)   : Igal Gutkin
// Create Date : 8/22/01
// Description :  
//
// *******************************************************************

#include "defs.h"

extern "C"
{
  // print hex dump to the buffer
  void FormatHexDump (unsigned char *pSrc, int length, char *pDest);
  void PlainHexDump  (unsigned char *pSrc, int length, char *pDest);

  // stdio direct debug output
  void dbg_print     (char *format, ...);
  void HexDump       (unsigned char *pSrc, int length);

  // Set debug checkpoint
  void DbgCheckPoint (char * pFile = NULL, int line = 0);
}

#define SET_CHECKPOINT DbgCheckPoint(__FILE__,__LINE__)



#endif //__DBGFUNC_H__