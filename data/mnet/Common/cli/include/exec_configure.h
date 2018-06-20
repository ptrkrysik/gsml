
/* $Id: exec_configure.h,v 10.1 1998/06/10 20:41:59 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/exec_configure.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: exec_configure.h
 * Description: configure commands
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: exec_configure.h,v $
 * Revision 10.1  1998/06/10 20:41:59  sutton
 * code merge
 *
 * Revision 1.4  1997/10/11 00:51:07  pparthas
 * Reverted to configure terminal , except that 'terminal' is now hidden
 *
 * Revision 1.3  1997/10/09  18:14:15  pparthas
 * First Attempt to cleanup dead-code to reduce code-size; Included debug
 * scheme in place
 *
 * Revision 1.2  1997/05/14  22:12:05  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */


/*************************************************************n
 * configure 
 *
 */

EOLS    (exec_config_term_eol, manual_configure, PARSER_CONF_TERM);
KEYWORD (exec_config_term, exec_config_term_eol, exec_config_term_eol,
         "terminal", "Configure from Terminal", PRIV_ROOT | PRIV_HIDDEN);
KEYWORD (exec_configure, exec_config_term, ALTERNATE,
         "configure", "Enter configuration mode", PRIV_ROOT);

#undef  ALTERNATE
#define ALTERNATE       exec_configure
