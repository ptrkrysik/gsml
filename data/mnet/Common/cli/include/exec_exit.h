/* $Id: exec_exit.h,v 10.1 1998/06/10 20:42:04 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_exit.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_exit.h
 * Description: exit, quit - exit from the EXEC
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_exit.h,v $
 * Revision 10.1  1998/06/10 20:42:04  sutton
 * code merge
 *
 * Revision 1.3  1998/03/18 04:34:54  sutton
 * Initial release of CLI/TCL integration.
 * Still missing -- sysconf commit, sysconf abort,
 *                  and interrupt character processing.
 * Also changed the output pager (More processing) to
 * be much simpler and universal for all commands.
 * CSCdj89287:  make the CLI callable from TCL
 *
 * Revision 1.2  1997/05/14 22:12:09  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
/******************************************************************
 * exit, quit - exit from the EXEC
 */
extern void Logout(parseinfo *csb);
EOLI    (exec_quit_eol, exit_exec_command, CMD_QUIT);
EOLI    (exec_exit_eol, Logout, CMD_EXIT);
KEYWORD (exec_quit, exec_quit_eol, ALTERNATE,
         "quit", "Exit from the EXEC", PRIV_MIN  | PRIV_HIDDEN | PRIV_INTERACTIVE);
KEYWORD (exec_exit, exec_exit_eol, exec_quit,
         "exit", "Exit from the EXEC", PRIV_MIN | PRIV_INTERACTIVE);

#undef  ALTERNATE
#define ALTERNATE       exec_exit
