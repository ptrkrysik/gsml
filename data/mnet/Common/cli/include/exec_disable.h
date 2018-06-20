/* $Id: exec_disable.h,v 10.2 1999/03/18 01:23:08 sundars Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_disable.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_disable.h
 * Description: disable command support
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_disable.h,v $
 * Revision 10.2  1999/03/18 01:23:08  sundars
 * CSCdm02577:  No Context sensitive help available for disable and enable
 * CLI comma
 *
 * Revision 10.1  1998/06/10 20:42:02  sutton
 * code merge
 *
 * Revision 1.6  1998/04/21 01:26:41  pparthas
 * CLI Related changes / Undo log / Errno propagation
 * CSCdj68847:  CLI changes
 *
 * Revision 1.5  1998/03/18 04:34:53  sutton
 * Initial release of CLI/TCL integration.
 * Still missing -- sysconf commit, sysconf abort,
 *                  and interrupt character processing.
 * Also changed the output pager (More processing) to
 * be much simpler and universal for all commands.
 * CSCdj89287:  make the CLI callable from TCL
 *
 * Revision 1.4  1997/10/09 18:14:17  pparthas
 * First Attempt to cleanup dead-code to reduce code-size; Included debug
 * scheme in place
 *
 * Revision 1.3  1997/09/27  04:30:57  pparthas
 * Removed levels for 'disable' command
 *
 * Revision 1.2  1997/05/14  22:12:07  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
EOLI    (exec_disable_endline, enable_command, CMD_DISABLE);
KEYWORD_ID(exec_disable, exec_disable_endline, ALTERNATE,
           OBJ(int,1), DEFAULT_PRIV,
           "disable", "Turn off privileged commands",
           PRIV_ROOT);

#undef  ALTERNATE
#define ALTERNATE       exec_disable
