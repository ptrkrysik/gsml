#include <vxworks.h>
#include <stdioLib.h>
#include <tyLib.h>
#include <taskLib.h>
#include <errnoLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN */
#include <timers.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <msgQLib.h>
#include <sigLib.h> 

 
#include <../include/cli_parser_private.h>
#include <../include/pstypes.h>
#include <../include/psvxif.h>
#include <../include/imsg.h>
#include <../include/imc.h>
#include <../include/address.h>
#include <../include/parsertypes.h>
#include <../include/config.h>
#include <../include/parser_sim.h>
#include <../include/parser.h>
#include <../include/parser_defs_exec.h>
#include <../include/parser_input.h>
#include <../include/parser_sim.h>
#include <../include/parser_util.h>
#include <../include/parser_commands.h>
#include <../include/nv.h>
#include <../include/parser_errno.h>
#include <../include/parser_ModNum.h>
#include <../include/defs.h>
#include <../include/parser_init.h>
#include <../include/macros.h>
#include <../include/parser_actions.h>
/*#include <parser/parser_defs_config.h>
  #include "exception.h" */

enum {
  TASK_DETAIL,
  TASK_SUMMARY,
  TASK_GEN
};

const char parser_help_default_keyword[] = "Set a command to its defaults";
const char parser_help_no_keyword[] = "Negate a command or set its defaults";
const char parser_default_keyword[] = "default";
const char parser_no_keyword[] = "no";


/* Global parser items */
transition pname(NONE) = {NULL, NULL, (trans_func) NONE_action, NULL};
transition pname(no_alt) = {NULL, NULL, (trans_func) no_alt_action, NULL};
transition pname(link_point) = {NULL, NULL, (trans_func) link_point_action, NULL};
transition pname(link_trans) = {NULL, NULL, (trans_func) NONE_action, NULL};

/*  Function for emitting comments about last config and NV update times */
static void write_config_times (parseinfo *csb)
{
    if (csb->nvgen && !csb->priv_set) {
      /* 
       *RAMS: commented this for now
       
       nv_write(TRUE, "!"); */
    }

}

/***************************************************
 * Interface mode commands 
 */

#ifdef a0

#define  ALTERNATE  no_alt
#include "os/ip/src/cfg_interface_exit.h"
#include "os/ip/src/cfg_int_ip.h"
#undef ALTERNATE

#endif
 

LINK_TRANS(top_configure_interface_extend_here, no_alt);
NO_OR_DEFAULT(config_interface_no, top_configure_interface_extend_here, 
              PRIV_MIN | PRIV_NONVGEN);
HELP(top_configure_interface, config_interface_no, 
     "Configure Interface commands:\n");

/*********************************************************
 * Clear commands.
 */
LINK_TRANS(exec_clear_extend_here, no_alt);

#undef      ALTERNATE
#define         ALTERNATE       exec_clear_extend_here
/*#include "../include/exec_clear_interface.h" */
NOP     (exec_clear_commands, ALTERNATE, NONE);


/*********************************************************
 * Debug commands.
 * Linkage to here is from exec_commands/debug.h
 */
LINK_TRANS(exec_debug_extend_here, no_alt);

#undef          ALTERNATE
#define         ALTERNATE       exec_debug_extend_here
#include        "../include/exec_debug_all.h"
NOP     (exec_debug_commands, ALTERNATE, NONE);


/*********************************************************
 * Show commands.
 * Linkage to here is from exec_commands/show.h
 */
LINK_TRANS(exec_show_extend_here, no_alt);

#undef          ALTERNATE
#define         ALTERNATE       exec_show_extend_here
/*#include      "../include/exec_show_configuration.h" */
NOP     (exec_show_commands, ALTERNATE, NONE);

/*********************************************************
 *      Global commands that exist at all levels
 *      These always terminate at 'no_alt' and therefore
 *      must be referenced from the terminal states in
 *      each of the command sets.  
 *
 *      None of the global commands
 *      should have a 'no' prefix format.
 */

LINK_TRANS      (global_extend_here, NONE);
TESTVAR (globalcmd_start_no, NONE, global_extend_here,
         NONE, NONE, NONE, sense, FALSE);
TESTVAR (globalcmd_start, globalcmd_start_no, NONE,
         NONE, NONE, no_alt, nvgen, FALSE);

/*********************************************************
 *      configure commands from here down
 */

#undef          ALTERNATE
#define         ALTERNATE       globalcmd_start

#include        "../include/cfg_exit.h"
#include        "../include/cfg_end.h"
NVCOMMENT(after_line_comment, ALTERNATE);
LINK_TRANS(cfg_extend_last_here, after_line_comment);
#undef  ALTERNATE
#define ALTERNATE       cfg_extend_last_here
/*#include    "../include/cfg_exception.h" */
/*#include    "../include/cfg_term.h" */
/*#include    "../include/cfg_trusthost.h" */
/*#include    "../include/cfg_exec_timeout.h" */
NVCOMMENT(cfg_extend_comment, ALTERNATE );
LINK_TRANS(cfg_extend_here, cfg_extend_comment);
#undef  ALTERNATE
#define ALTERNATE       cfg_extend_here


EVAL    (config_times, ALTERNATE, write_config_times(csb));
NVCOMMENT(first_comment, config_times);
NO_OR_DEFAULT(config_no, first_comment, PRIV_MIN | PRIV_NONVGEN);
HELP    (top_configure, config_no, "Configure commands:\n");


/*********************************************************
 * Exec Commands
 *
 * If we're enabled, then include the priv_commands, otherwise, go
 * to the global command set.
 *
 */

LINK_TRANS(exec_extend_here, globalcmd_start);

#undef  ALTERNATE
#define ALTERNATE       exec_extend_here
/*#include    "../include/exec_ping.h" */
#include    "../include/exec_undebug.h"
#include    "../include/exec_debug.h"
#include      "../include/exec_exit.h"
#include    "../include/exec_show.h"
/*#include    "../include/exec_term.h" */
/*#include    "../include/exec_tclsh.h" */
#include    "../include/exec_clear.h"
/*#include    "../include/exec_traceroute.h"*/
HELP    (top, ALTERNATE, "Exec commands:\n");

#ifdef  PARSER_DEBUG_LINKS
#define LINK(which,str,where,link)\
    {which,str,((dynamic_transition *)&pname(where)),link},
#else   /* PARSER_DEBUG_LINKS */
#define LINK(which,str,where,link)\
    {which,((dynamic_transition *)&pname(where)),link},
#endif  /* PARSER_DEBUG_LINKS */

chain_link chain_gang[] = {
    LINK(PARSE_ADD_EXEC_CMD, "exec", exec_extend_here,
          PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_SHOW_CMD, "show", exec_show_extend_here,
      PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_DEBUG_CMD, "debug", exec_debug_extend_here,
      PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_CLEAR_CMD, "clear", exec_clear_extend_here,
      PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_CFG_TOP_CMD, "configuration", cfg_extend_here,
      PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_CFG_INTERFACE_TOP_CMD, "interface", 
      top_configure_interface_extend_here,
      PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_CFG_LAST_CMD, "end of top level configuration commands",
          cfg_extend_last_here, PARSER_ENTRY_LINK_POINT)
    LINK(PARSE_ADD_GLOBAL_CMD, "global", global_extend_here,
      PARSER_ENTRY_LINK_POINT)


    { PARSE_LIST_END,
#ifdef  PARSER_DEBUG_LINKS
          NULL,
#endif  /* PARSER_DEBUG_LINKS */
          NULL, 0}
};





/*
 * chain.c common mode information
 */

parser_mode *exec_mode;
parser_mode *config_mode;
parser_mode *interface_mode;

void parser_add_main_modes (void)
{
    exec_mode = parser_add_mode("exec", NULL, "Exec mode",
                                TRUE, TRUE, NULL, NULL, NULL,
                                &pname(top),
                                NULL );

    config_mode = parser_add_mode("configure", "config",
                                  "Global configuration mode",
                                  TRUE, TRUE, NULL, NULL, NULL,
                                  &pname(top_configure),
                                  NULL);

    interface_mode = parser_add_mode("interface", "config-if",
                                     "Interface configuration mode",
                                     TRUE, TRUE,
                                     NULL, /* "configure", */
                                     NULL, /* interface_save_var, */
                                     NULL, /* interface_reset_var, */
                                     &pname(top_configure_interface),
                                     NULL);
}


void show_test_mnet(parseinfo *csb)
{
  printf("\n show command is integrated\n");
}
