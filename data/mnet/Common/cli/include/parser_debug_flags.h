#include <../include/debug_macros.h>

/* Now define the actual flags and the array that points to them */
DEBUG_ARRDECL(parser_debug_arr)

DEBUG_FLAG(parser_ambig_debug,DEBUG_PARSER_AMBIG,"Parser ambiguity")
DEBUG_FLAG(parser_help_debug,DEBUG_PARSER_HELP,"Help message")
DEBUG_FLAG(parser_mode_debug,DEBUG_PARSER_MODE,"Parser mode")
DEBUG_FLAG(parser_alias_debug,DEBUG_PARSER_ALIAS,"Parser aliases")
DEBUG_FLAG(parser_priv_debug,DEBUG_PARSER_PRIV,"Privilege level")
DEBUG_FLAG(parser_http_debug,DEBUG_PARSER_HTTP,"Parser HTTP processing")

DEBUG_ARRDONE
