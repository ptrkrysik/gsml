/* $Id: cfg_exit.h,v 10.2 1998/12/30 18:13:01 sundars Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/cfg_exit.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: cfg_exit.h
 * Description: exit    - exit configuration mode 
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: cfg_exit.h,v $
 * Revision 10.2  1998/12/30 18:13:01  sundars
 * Fix help texts for CLI commands
 * CSCdk69292:  CLI Command Descriptions When Entering a ?-Part 2
 *
 * Revision 10.1  1998/06/10 20:41:52  sutton
 * code merge
 *
 * Revision 1.2  1997/05/14 22:12:02  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/***************************************************************
 * exit
 *
 * exits from config mode
 */

EOLS    (act_exit_configure, exit_config_command, CMD_EXIT);

KEYWORD (exit_configure, act_exit_configure, ALTERNATE, 
         "exit", "Exit configuration mode", PRIV_MIN);

#undef  ALTERNATE
#define ALTERNATE       exit_configure
