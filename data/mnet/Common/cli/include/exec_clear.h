/* $Id: exec_clear.h,v 10.2 1999/02/12 03:19:54 sundars Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_clear.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File:  exec_clear.h
 * Description: clear - Reset Functions
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_clear.h,v $
 * Revision 10.2  1999/02/12 03:19:54  sundars
 * create a customer debug build; remove lots of junk printfs from
 * customer builds;disable wccp when we suspend a task due to
 * panic/exception etc.
 * CSCdk92101:  Make cli as the default on development
 *
 * Revision 10.1  1998/06/10 20:41:58  sutton
 * code merge
 *
 * Revision 1.3  1997/12/05 18:17:43  pparthas
 * Unhide the 'clear' command
 * CSCdj39802:  A command to clear serial interfaces
 *
 * Revision 1.2  1997/05/14 22:12:04  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
/******************************************************************/
#ifdef MNET_GP10
EOLNS    (exec_clear_help_eol, clear_help_command);
PRIV_TEST(exec_clear_help, exec_clear_help_eol, NONE,
                  exec_clear_commands, PRIV_CONF | PRIV_HIDDEN );

KEYWORD (exec_clear, exec_clear_help, ALTERNATE,
         "clear", "Reset functions", PRIV_CONF | PRIV_HIDDEN);

#undef  ALTERNATE
#define ALTERNATE       exec_clear

#endif
