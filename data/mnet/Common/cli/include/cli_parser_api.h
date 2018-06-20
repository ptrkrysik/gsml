#ifndef __CLI_PARSER_API_H__
#define __CLI_PARSER_API_H__

/* exported by cli_parser_init.c */
extern void CliParserInit();
extern void CliIOInit (int);

/* exported by debug.cpp */

extern void debug_init();

/* exported by command_chain.cpp */
extern void command_parser_init();

/* exported by exec_chain.cpp */
extern void exec_parser_init();

#endif /* __CLI_PARSER_API_H__ */
