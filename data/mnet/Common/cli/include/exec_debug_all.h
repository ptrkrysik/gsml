/* $Id: exec_debug_all.h,v 10.1 1998/06/10 20:42:02 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_debug_all.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_debug_all.h
 * Description: debug all command support
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_debug_all.h,v $
 * Revision 10.1  1998/06/10 20:42:02  sutton
 * code merge
 *
 * Revision 1.3  1998/04/03 21:21:59  pparthas
 * Changed EOLS macro to EOLI macro
 * CSCdj89287:  make the CLI callable from TCL
 *
 * Revision 1.2  1997/05/14 22:12:07  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
/******************************************************************
 * debug all
 *
 
EOLI   (debug_all_eol, debug_all_cmd, 0);
KEYWORD (debug_all, debug_all_eol, ALTERNATE,
     "all", "Enable all debugging", PRIV_OPR);
 
 
#undef  ALTERNATE
#define ALTERNATE   debug_all

*/