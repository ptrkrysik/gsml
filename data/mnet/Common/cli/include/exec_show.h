/* $Id: exec_show.h,v 10.1 1998/06/10 20:42:06 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_show.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_show.h
 * Description: show command
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_show.h,v $
 * Revision 10.1  1998/06/10 20:42:06  sutton
 * code merge
 *
 * Revision 1.2  1997/05/14 22:12:10  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/******************************************************************/
EOLNS   (exec_show_help_eol, show_help_command);
PRIV_TEST(exec_show_help, no_alt, no_alt,
          exec_show_commands, PRIV_MIN);

KEYWORD (exec_show, exec_show_help, ALTERNATE,
         "show", "Show running system information", PRIV_MIN);

#undef  ALTERNATE
#define ALTERNATE       exec_show
