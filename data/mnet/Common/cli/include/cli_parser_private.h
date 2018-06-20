#ifndef __CLI_PARSER_PRIVATE_H__
#define __CLI_PARSER_PRIVATE_H__

#define PARSER_TASK_PRIORITY     150
#define PARSER_TASK_OPTION       0
#define PARSER_TASK_STACK_SIZE   20000
#define DEBUG_CLI_PARSER         0

#define DBG_PARSER(s) if (DEBUG_CLI_PARSER) printf(s);

#endif /* __CLI_PARSER_PRIVATE_H__ */
