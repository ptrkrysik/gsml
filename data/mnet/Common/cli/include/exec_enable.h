/* $Id: exec_enable.h,v 10.2 1999/03/18 01:23:09 sundars Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_enable.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_enable.h
 * Description: enable command
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_enable.h,v $
 * Revision 10.2  1999/03/18 01:23:09  sundars
 * CSCdm02577:  No Context sensitive help available for disable and enable
 * CLI comma
 *
 * Revision 10.1  1998/06/10 20:42:03  sutton
 * code merge
 *
 * Revision 1.6  1998/04/21 01:26:42  pparthas
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
 * Revision 1.3  1997/08/16  04:52:53  pparthas
 * For now, got rid of 'Privilege Levels' in 'enable' command
 *
 * Revision 1.2  1997/05/14  22:12:08  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
/****************************************************************
 * enable
 */
EOLI    (enable_endline, enable_command, CMD_ENABLE);
KEYWORD_ID(enable, enable_endline, ALTERNATE,
           OBJ(int,1), PRIV_MAX,
           "enable", "Turn on privileged commands",
           PRIV_MIN | PRIV_INTERACTIVE);

#undef  ALTERNATE
#define ALTERNATE       enable
