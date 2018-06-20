/* $Id: exec_help.h,v 10.2 1998/12/30 18:13:02 sundars Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_help.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_help.h
 * Description: help command 
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_help.h,v $
 * Revision 10.2  1998/12/30 18:13:02  sundars
 * Fix help texts for CLI commands
 * CSCdk69292:  CLI Command Descriptions When Entering a ?-Part 2
 *
 * Revision 10.1  1998/06/10 20:42:04  sutton
 * code merge
 *
 * Revision 1.3  1997/10/09 18:14:18  pparthas
 * First Attempt to cleanup dead-code to reduce code-size; Included debug
 * scheme in place
 *
 * Revision 1.2  1997/05/14  22:12:09  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/******************************************************************
 * help
 */

EOLNS   (exec_help_eol, exec_help_command);
KEYWORD (exec_help, exec_help_eol, ALTERNATE,
         "help", "Assistance for command line interface",
         PRIV_MIN );

#undef  ALTERNATE
#define ALTERNATE       exec_help
