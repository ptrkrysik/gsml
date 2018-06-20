#ifndef __PARSER_H__
#define __PARSER_H__



enum {
  STACKMAKER_NAME =0,
  STACKMAKER_MEMBER,
};

typedef struct parsenode_ {
  struct parsenode_ *next;              /* Next stack entry */
  transition *node;                             /* The node */
  int line_index;                               /* Where we are in the input buffer */
  char nv_command[PARSEBUF];    /* The command generated to this point */
  char visible_ambig[PARSEBUF]; /* Visible command ambiguity detection */
  char hidden_ambig[PARSEBUF];  /* Hidden command Ambiguity detection */
  boolean command_visible;              /* TRUE if current keyword is visible */
  boolean unsupported;                  /* TRUE if current keyword is unsupp. */
  parser_mode *pmp;                             /* Parser mode */
  boolean sense;                                /* FALSE if 'no' parsed.  Also FALSE if 'default'
                                                                 * parsed because the off state is the default for
                                                                 * most commands. */
  boolean set_to_default;               /* TRUE if 'default' parsed */
  boolean leave_subcommand;

#ifdef BOOMERANG
  /* save string info from csb into sub-mode csb's */
  char save_txt[PARSEBUF];
#endif /* BOOMERANG */
  int interface;

} parsenode;

typedef struct callbackMsg {
        parseinfo *csb;
        tpPARSERSIM pSim;
} tCALLBACKMSG, *tpCALLBACKMSG;

/* extern chunk_type *parseinfo_chunks;*/

extern parseinfo *get_csb(const char *);
extern void free_csb(parseinfo **csb);
struct idbtype_ *findidb(parseinfo *csb);
extern void set_multiple_funcs(parseinfo *csb, int value);
extern void increment_multiple_funcs(parseinfo *csb, char *str);
extern int test_multiple_funcs(parseinfo *csb, char test, int value);
extern int test_visible_multiple_funcs(parseinfo *csb, char test, int value);
extern int test_invisible_multiple_funcs(parseinfo *csb, char test, int value);
extern void print_multiple_funcs(parseinfo *csb);
extern void report_error(parseinfo *, int, boolean);
extern tOCTET2 parser_send_response( tpCOMSIM , char *, tUOCTET4 );
extern void ParserCallbackInit( void );

/* Return values from parse_cmd() used by report_error() */
#define PARSE_ERROR_NOERR     0x0  /* Success */
#define PARSE_ERROR_AMBIG     0x1  /* Ambiguous command */
#define PARSE_ERROR_NOMATCH   0x2  /* True error; didn't match any command */
#define PARSE_ERROR_UNKNOWN   0x4  /* Didn't match anything while doing help */
#define PARSE_ERROR_ALIAS     0x8  /* Didn't match expanded alias */

#define RC_CONT      1
#define RC_MORE      2
#define RC_NO_MORE   3
#define RC_CANT_MORE 4
#define RC_WAIT      5
#define RC_BREAK     6
#define RC_CFG_BREAK 7
#define RC_TIMEOUT   8

#define MORE_BUFFER    1
#define MORE_FILE      2

#define NON_UNIQUE      1               /* Non-unique token in help */
#define PARSE_FAIL      2               /* Parse failed */

/*
 *  Values for csb->in_help
 */
#define PARSER_NO_HELP                  0
#define PARSER_HELP                     1<<0
#define PARSER_COMMAND_COMPLETION       1<<1

/*
 * Values for mode volume
 */
#define MODE_VERBOSE    1
#define MODE_SILENT     2

/*
 * Type of parser
 */
#define PARSER_CLI_DEFAULT              0
#define PARSER_CLI_TCL                  1
#define PARSER_CLI_OVERRIDE             2


#ifdef USE_PARSER_KMEM
 
#define PARSERcalloc(nelem, elemSize)     PSkcalloc((nelem), (elemSize), \
                                                  M_PARSER, M_WAITOK)
#define PARSERmalloc(elemSize)            PSkmalloc((elemSize), \
                                                  M_PARSER, M_WAITOK)
#define PARSERrealloc(elemPtr, elemSize)  PSkrealloc((elemPtr), (elemSize), \
                                                 M_PARSER, M_WAITOK)
#define PARSERfree( elemPtr )             PSkfree( (elemPtr), M_PARSER )
 
#else
 
#define PARSERcalloc(nelem, elemSize)     PScalloc((nelem), (elemSize))
#define PARSERmalloc(elemSize)            PSmalloc((elemSize))
#define PARSERrealloc(elemPtr, elemSize)  PSrealloc((elemPtr), (elemSize))
#define PARSERfree(elemPtr)               PSfree((elemPtr))
 
#endif


#define PARSER_ENTRY_LINK_POINT 1
#define PARSER_EXIT_LINK_POINT  2

/*
 * Uncomment this define to add parser link point names for debugging
 */
/*#define       PARSER_DEBUG_LINKS*/

typedef struct chain_link_ {
    uint id;
#ifdef  PARSER_DEBUG_LINKS
    const char *name;
#endif  /* PARSER_DEBUG_LINKS */
    dynamic_transition *link_point;
    uint type;
} chain_link;

typedef struct chain_list_ {
    struct chain_list_ *next;
    chain_link cp;
} chain_list;

extern chain_link chain_gang[];
extern SEM_ID parse_chain_sem;

typedef void * (*mode_save_var_func)(parseinfo *);
typedef void (*mode_reset_var_func)(parseinfo *, void *);
typedef boolean (*mode_http_init_func)(parseinfo *);


extern parser_mode *parser_add_mode(const char *, const char *, const char *,
                                    boolean, boolean, const char *,
                                    mode_save_var_func, mode_reset_var_func,
                                    transition *,
                                    mode_http_init_func);

extern parser_mode *get_mode_tmp(const char *, const char *, const char *,
                                 boolean, boolean,
                                 transition *);
extern void free_mode(parser_mode *);
extern parser_mode *get_alt_mode(parser_mode *);
extern void *mode_save_vars(parseinfo *);
extern void mode_reset_vars(parseinfo *, void *);
extern boolean priv_changed_in_mode(parser_mode *);
extern void set_priv_changed_in_mode(parser_mode *, boolean);
extern boolean mode_http_init(parseinfo *);
extern int print_debug(char *, ... );

/*
 * The big well known parser modes
 */
extern parser_mode *exec_mode;
extern parser_mode *config_mode;

/*extern BOOT_PARAMS sysBootParams;*/

/*
 * Some extern declarations for variables
 * defined in parser.c / parser_init.c
 */
extern SEM_ID systemloadingSemId;
extern int terminalLength;
extern int parserNullFd;
extern FILE* parserNullFp;

/*
 * Miscellaneous routine declarations.
 */
void parse_token(parseinfo *);
void push_node(parseinfo *, transition *);
int parse_cmd(parseinfo *, tpPARSERSIM);
boolean batch_input(parseinfo *, char *, int, char *, char *);
void print_caret(int);
void save_line_init(save_buf*);
void save_line(parseinfo *, save_buf*, const char*, ...);
int print_buf(save_buf, boolean, boolean, int *, long);
extern void bad_parser_subcommand(parseinfo *, uint);
char *last_config_time_string(void);
char *last_write_mem_time_string(void);
void note_write_mem_time(void);
void free_csb_mem(parseinfo *);
extern int cliShell_sync(void);

/*
 * Parser Mode functions
 */
extern void parser_modes_init(void);
extern parser_mode *get_mode_byname(const char *, uint);
extern boolean set_mode_byname(parser_mode **, const char *, uint);
extern queuetype *get_mode_aliases(parser_mode *);
extern queuetype *get_mode_defaliases(parser_mode *);
extern const char *get_mode_prompt(parseinfo *);
extern boolean test_mode_config_subcommand(parseinfo *);
extern transition *get_mode_nv(parseinfo *);
extern transition *get_mode_top(parser_mode *);
extern const char *get_mode_name(parser_mode *);
extern void display_mode_aliases(parseinfo *, parser_mode *);
extern void parser_add_main_modes(void);

/*
 *  Global parser debug variables
 */
extern boolean parser_help_debug;
extern boolean parser_ambig_debug;
extern boolean parser_mode_debug;
extern boolean parser_priv_debug;

/*
 * Should we have a prompt in config mode.
 */
extern boolean config_prompt;
extern char *nomemory;

extern int doprnt (char *, tt_soc *, const char *, va_list , boolean );
extern void printVersion( void );
extern int getTftpInput( char *pTftpServer, char *pFileName );

extern boolean queueempty_inline (queuetype *q);
extern void enqueue(queuetype *, void *);
extern void unqueue(queuetype *, void *);
extern void queue_init(queuetype *, int);
extern void *dequeue(queuetype *);
extern void requeue(queuetype *, void *);
extern void *peekqueuehead (queuetype* q);
extern int queryqueuedepth(queuetype *);
extern void insqueue(queuetype *, void *, void *);
extern ushort ipcrc (ushort *p, int count);
extern boolean name_verify( char *);
#ifdef IOS_PARSER
/*
 *  Global parser debug variables
 */
extern boolean parser_alias_debug;
extern boolean parser_http_debug;

extern boolean parser_link_point_exists(uint);

void recurse_node(parseinfo *, transition *);
extern void *p_dequeue(queuetype *);
extern void p_enqueue (queuetype *, void *);
extern void p_unqueue (queuetype *, void *);
extern void *remqueue(queuetype *, void *, void *);
#endif


#endif  /* __PARSER_H_ */
