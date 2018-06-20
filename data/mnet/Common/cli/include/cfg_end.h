/* $Id: cfg_end.h,v 10.2 1998/12/30 18:13:01 sundars Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/cfg_end.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: cfg_end.h
 * Description: end  - exit configuration mode
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: cfg_end.h,v $
 * Revision 10.2  1998/12/30 18:13:01  sundars
 * Fix help texts for CLI commands
 * CSCdk69292:  CLI Command Descriptions When Entering a ?-Part 2
 *
 * Revision 10.1  1998/06/10 20:41:51  sutton
 * code merge
 *
 * Revision 1.4  1998/04/21 01:26:41  pparthas
 * CLI Related changes / Undo log / Errno propagation
 * CSCdj68847:  CLI changes
 *
 * Revision 1.3  1997/08/04 19:28:27  pparthas
 * Made 'end' from config mode a hidden command
 *
 * Revision 1.2  1997/05/14  22:12:01  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/***************************************************************
 * end
 *
 * exits from config mode
 */

EOLS    (act_end_configure, exit_config_command, CMD_END);

KEYWORD (end_configure, act_end_configure, ALTERNATE, "end",
                 "Exit configuration mode", PRIV_MIN);

#undef  ALTERNATE
#define ALTERNATE       end_configure
