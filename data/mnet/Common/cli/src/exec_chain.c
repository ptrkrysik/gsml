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
/*
 * Parse chains for Exec help commands
 */
#define ALTERNATE       NONE
#include "../include/exec_help.h"
LINK_POINT(global_commands, ALTERNATE);
#undef  ALTERNATE


/*
 * Parse chains for Exec exec commands
 */
#define ALTERNATE       NONE
/*#include "../include/exec_type.h"*/
/*#include "../include/exec_copy.h"*/
/*#include "../include/exec_reload.h"*/
LINK_POINT(exec_exec_commands, ALTERNATE);
#undef  ALTERNATE


/*
 * Parse chains for Exec show commands
 */
#define ALTERNATE       NONE
/*#include "../include/exec_show_version.h" */
/*#include "../include/exec_show_debugging.h"*/
/*#include "../include/exec_show_techsupport.h"*/
LINK_POINT(exec_show_cmds, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for Exec copy commands
 */
#define ALTERNATE       NONE
/*#include "../include/exec_copy_techsupport.h"*/
LINK_POINT(exec_copy_commands, ALTERNATE);
#undef ALTERNATE


/*
 * Parse chains for copy running-configuration
 */
#define ALTERNATE       NONE
/*#include "../include/exec_copy_running_cfg.h"*/
LINK_POINT(copy_running_cfg_cmds, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for copy startup-config
 */
#define ALTERNATE       NONE
/*#include "../include/exec_copy_startup_cfg.h"*/
LINK_POINT(copy_startup_cfg_cmds, ALTERNATE);
#undef  ALTERNATE

#define ALTERNATE NONE
EOLS(exec_tftp_startup_eol, write_config, COPY_TFTP_STARTUP_CONFIG);
KEYWORD(exec_copy_tftp_startup, exec_tftp_startup_eol, ALTERNATE,
        "startup-config", "TFTP Server to Startup(NVRAM) Configuration",
        PRIV_ROOT);
LINK_POINT(copy_tftp_startup_cmds, exec_copy_tftp_startup);
#undef  ALTERNATE


#define ALTERNATE NONE
EOLS(exec_disk_startup_eol, write_config, COPY_DISK_STARTUP_CONFIG);
STRING(exec_disk_startup_string, exec_disk_startup_eol, no_alt,
       OBJ(string, 1), "Filename of existing configuration" );
KEYWORD(exec_copy_disk_startup, exec_disk_startup_string, ALTERNATE,
        "startup-config", "Disk to Startup(NVRAM) Configuration",
        PRIV_ROOT);
LINK_POINT(copy_disk_startup_cmds, exec_copy_disk_startup);
#undef  ALTERNATE

/*
 * Parse chain registration array for Exec
 */
static parser_extension_request exec_chain_init_table[] = {
    { PARSE_ADD_EXEC_CMD, &pname(exec_exec_commands) },
    { PARSE_ADD_SHOW_CMD, &pname(exec_show_cmds) },
    { PARSE_ADD_GLOBAL_CMD, &pname(global_commands) },
    { PARSE_ADD_COPY_CMD, &pname(exec_copy_commands) },
    { PARSE_ADD_COPY_RUNNING_CMD, &pname(copy_running_cfg_cmds) },
    { PARSE_ADD_COPY_STARTUP_CMD, &pname(copy_startup_cfg_cmds) },
    { PARSE_ADD_COPY_DISK_CMD, &pname(copy_disk_startup_cmds) },
    { PARSE_ADD_COPY_TFTP_CMD, &pname(copy_tftp_startup_cmds) },
    { PARSE_LIST_END, NULL }
};

/*
 * exec_parser_init - Initialize Exec parser support
 */
void exec_parser_init (void)
{
        static int exec_parser_initialized = FALSE;
  
        if (!exec_parser_initialized) {
                /* copy ... */
#if 0
        RAMS: commented this for now
                parser_add_link_point(PARSE_ADD_COPY_CMD, "copy",
                                                          &pname(exec_copy_extend_here));

                /* copy run-config ... */
                parser_add_link_point(PARSE_ADD_COPY_RUNNING_CMD,
                                                          "copy running-config *",
                                                          &pname(exec_copy_running_extend_here));
                parser_add_link_point(PARSE_ADD_COPY_RUNNING_FILE_CMD,
                                                          "copy running-config *",
                                                          &pname(exec_copy_running_file_extend_here));
                /* copy start-config ... */
                parser_add_link_point(PARSE_ADD_COPY_STARTUP_CMD,
                                                          "copy startup-config *",
                                                          &pname(exec_copy_startup_extend_here));
                parser_add_link_point(PARSE_ADD_COPY_STARTUP_FILE_CMD,
                                                          "copy startup-config *",
                                                          &pname(exec_copy_startup_file_extend_here));
#endif
                parser_add_command_list(exec_chain_init_table, "exec");

                exec_parser_initialized = TRUE;
        }
}
