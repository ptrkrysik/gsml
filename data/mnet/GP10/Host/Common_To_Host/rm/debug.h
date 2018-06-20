/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "stdio.h"

// Constants used in debug.cpp
#define ROW_LEN		   10
#define MAREA__		   0
#define RXMSG__		   1
#define TXMSG__		   2

#define PDEBUG__(X)	   rm_Dbg.Trace X
#define EDEBUG__(X)	   rm_Dbg.Error X
#define IDEBUG__(X)	   rm_Dbg.Warning X
#define DDEBUG__(X)	   rm_Dbg.Warning X
#define UDEBUG__(X)	   rm_Dbg.Warning X
#define RMDBG_ENTER(X)	   rm_Dbg.Func X
#define BYDUMP__(X)    	   rm_Dbg.HexDump X

// DUMP->db_BDump to dump out data in hex form

#define RDEBUG__(X)        rm_Dbg.Trace X
#define TDEBUG__(X)	   
#define BDUMP__(X)
#define EDUMP__(X)
#define UDUMP__(X)	
#define TDUMP__(X)	
void    db_BDump ( unsigned char, unsigned char *, unsigned short );

#endif //__DEBUG_H__
