/* $Id: exec_debug.h,v 10.1 1998/06/10 20:42:01 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_debug.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_debug.h
 * Description: debug  command support
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_debug.h,v $
 * Revision 10.1  1998/06/10 20:42:01  sutton
 * code merge
 *
 * Revision 1.3  1998/04/21 01:26:41  pparthas
 * CLI Related changes / Undo log / Errno propagation
 * CSCdj68847:  CLI changes
 *
 * Revision 1.2  1997/05/14 22:12:06  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

EOLNS   (exec_debug_help_eol, debug_help_command);
PRIV_TEST(exec_debug_help, exec_debug_help_eol, NONE,
          exec_debug_commands, PRIV_USER | PRIV_HIDDEN);

KEYWORD_ID(exec_debug_false, exec_debug_help, no_alt,
           sense, FALSE,
           "debug", "Disable debugging functions",
           PRIV_OPR);
KEYWORD (exec_debug_no, exec_debug_false, ALTERNATE,
         "no", "Disable debugging functions", PRIV_OPR);

KEYWORD_ID(exec_debug_true, exec_debug_help, exec_debug_no,
           sense, TRUE,
           "debug", "Debugging functions", PRIV_OPR);

#undef  ALTERNATE
#define ALTERNATE       exec_debug_true
