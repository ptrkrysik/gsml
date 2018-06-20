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
#include <../include/parser_defs_config.h>

/*
 * Parse chains for Command exec commands
 */
#define ALTERNATE       NONE
#include "../include/exec_disable.h"
#include "../include/exec_enable.h"
#include "../include/exec_configure.h"
/*#include "../include/exec_write.h" */
LINK_POINT(command_exec_commands, ALTERNATE);
#undef  ALTERNATE

#define ALTERNATE NONE
/*#include "../include/exec_erase.h" */
LINK_POINT(erase_commands, ALTERNATE);
#undef ALTERNATE

/*
 * Parse chain registration array for Command
 */
static parser_extension_request command_chain_init_table[] = {
    { PARSE_ADD_EXEC_CMD, &pname(command_exec_commands) },
    { PARSE_ADD_ERASE_CMD, &pname(erase_commands) },
    { PARSE_LIST_END, NULL }
};


/*
 * command_parser_init - Initialize Command parser support
 */
void command_parser_init (void)
{
    parser_add_command_list(command_chain_init_table, "command");
    return;
}
