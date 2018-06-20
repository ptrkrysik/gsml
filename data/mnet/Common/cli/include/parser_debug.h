/* $Id: parser_debug.h,v 10.1 1998/06/11 22:03:31 sutton Exp $
 * $Source: /export/sbcc/cepro/os/parser/src/parser_debug.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: parser_debug.h
 * Description: Parser Debug macros
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: parser_debug.h,v $
 * Revision 10.1  1998/06/11 22:03:31  sutton
 * code merge
 *
 * Revision 1.2  1997/05/14 22:12:19  pparthas
 * Removed test_* files. Included some files for Show routines
 * and cleaned-up file structure ( not source code ;-) )
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

/* Function to fire up parser debugging */
extern void debug_parser_init(void);

/*
 * The actual debugging flags are defined in parser_debug_flags.h.
 * We include that file twice, once to define the flags themselves
 * and once to define the indices that the parser uses.
 */
#include "parser_debug_flags.h"
#define __DECLARE_DEBUG_NUMS__
#include "parser_debug_flags.h"
