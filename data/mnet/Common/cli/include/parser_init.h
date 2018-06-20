#ifndef __PARSER_INIT_H
#define __PARSER_INIT_H



#define MAXCSB 32

/* 
 * Externals
 */
extern parser_mode *exec_mode;
extern parser_mode *config_mode;
 
extern void test_parser_init(void);
extern void parser_pipe_init(void);
extern void show_os_chain_init(void);
extern void parser_modes_init(void);
extern void command_parser_init(void);
extern void     exec_parser_init(void);
extern void     parse_pager_init(void);
extern void parser_sema_init(void);
extern void parser_task_init(void);
extern void parser_iowork_init(void);
extern void parser_configure(void);
extern parseinfo* get_csb(const char*);
extern void BuildPrompt(parseinfo *, tt_soc*);

extern void console_main(void);
extern void parser_init(void);
extern void getPrompt( parseinfo*, char*, char* );
extern void processCommand( tpPARSERSIM );

extern boolean config_prompt;
extern int consoleFd;
extern void parser_check();

extern tBSEMAPHORE configSemId;
#endif __PARSER_INIT_H
