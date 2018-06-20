/* $Id: exec_undebug.h,v 10.1 1998/06/10 20:42:17 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_undebug.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_undebug.h
 * Description: undebug command support
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_undebug.h,v $
 * Revision 10.1  1998/06/10 20:42:17  sutton
 * code merge
 *
 * Revision 1.1  1998/04/21 01:57:17  pparthas
 * CLI Changes
 * CSCdj68847:  CLI changes
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/******************************************************************/
KEYWORD_ID(exec_undebug, exec_debug_commands, ALTERNATE,
                   sense, FALSE,
                   "undebug", "Disable debugging functions (see also 'debug')",
                   PRIV_OPR);

#undef  ALTERNATE
#define ALTERNATE   exec_undebug
