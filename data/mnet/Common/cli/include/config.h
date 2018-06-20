#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SABUFSIZE 3000          /* buffer size for config from term */
#define TOKLEN 85                       /* maximum token length */
#define WORD 0                          /* gettoken should return a word */
#define LINE 1                          /* gettoken should return a line */


/*
 * flags used in csb->flags
 */
#define CONFIG_TERM                 0x00000001 /* configuring from a terminal */
#define CONFIG_ABORT            0x00000002 /* need to abort the config */
#define CONFIG_DONE                 0x00000004 /* config is done */
#define CONFIG_HTTP                 0x00000008 /* configuring from http */
#define KEYWORD_MATCHED         0x00000010 /* keyword was matched */
#define COMMAND_AUTHORIZE       0x00000020 /* expand command into nv_command */
#define COMMAND_FUNCTION        0x00000040 /* substitute function on accept */
#define HIDE_PASSWORDS          0x00000080 /* Don't print password during nvgen */
#define PARSE_EMPTY_STRING      0x00000100 /* Try to parse empty string */
#define CONFIG_PER_USER         0x00000200 /* Per-user configuration */
#define CONFIG_CGI                  0x00000400 /* configuring from http */
#define COMMAND_COMPLETE    0x00000800 /* Indicate to I/O so it can do History */
#define CONFIG_FILE         0x00001000 /* Copy Startup to Disk */
#define TERMINAL_MODE       0x00002000 /* Indicate to Callback to leave at
                                        * Terminal Mode */

#define NOMEANSZERO     1       /* if "no", assume number 0 */
#define NONULL          2       /* null args not allowed */
#define NOMEANSMINUS    3       /* if "no", assume number -1 */
#define NOMEANSNULL     4       /* if "no", assume null string */
#define NULLMEANSDONE   5       /* if NULL, quiet error return */


/*
 * Methods for resolving protocol addresses and configuration information.
 * Note that this list has grown to be used for lots more than resolving 
 * protocol addresses, however.
 */
enum RES {
    RES_NULL,                   /* Not resolved */
    RES_CONFIG,                 /* Configuration file */
    RES_MANUAL,                 /* Manually set */
    RES_NONVOL,                 /* Read from non-volatile RAM */
    RES_PRIVATE_NONVOL,         /* Read from private non-volatile RAM */
    RES_WRITING_NONVOL          /* Writing to non-volatile RAM */
};



/* 
 * RAMS : this was under ifdef of IOS_PARSER
 */

/*
 * A list of file access methods.
 * The bootstrap monitor knows about the TFTP, MOP, and FLASH codes,
 * so don't change their values.
 */
typedef enum {
    FILE_ACCESS_UNKNOWN = 0,
    FILE_ACCESS_TFTP = 1,
    FILE_ACCESS_FLASH = 3,
    FILE_ACCESS_ROM = 4,
    FILE_ACCESS_RCP = 5,
    FILE_ACCESS_FTP = 8,
} file_access_protocol;


/*
 * now define a generic named thing.
 */
typedef struct namedthingtype_ {
    struct namedthingtype_ *next;
    NAMEDTHINGMACRO
    uchar data[1];
} namedthingtype;

/*
 * RAMS : ifdef ended here
 */






/************************************************************************/
/*********** START OF NEW PARSER STRUCTURE ******************************/
/************************************************************************/


#define PARSEBUF            256 /* Input buffer length */
#define MAXTOKLEN           21  /* Maximum token length */
#define MAXPROMPTLEN    30      /* Maximum prompt; prompt truncated to fit */

/* Structure for saving blocks of character strings like comments,
 * help output, and NV generation.
 */
typedef struct save_buf_ {
  char *buf;            /* The buffer itself */
  int size;             /* Size of the malloc'ed buffer area */
  int used;             /* Number of characters stored in the buffer */
} save_buf;

/*
 *
 */
typedef struct ambig_struct_ {
    char ambig[PARSEBUF];
    char ambig_save[PARSEBUF];
    int multiple_funcs;
} ambig_struct;

/* The 'transition' struct is the central data structure of the parser.
 * A parse tree is built of transition nodes, each of which references
 * an action routine (func() in the structure).  The action routine
 * will transition to the 'accept' node if the input was matched (or
 * some other action was successful).  The 'alternate' transition is taken
 * if the action failed, and after the accepting node returns (recursion
 * is currently used to transition between nodes).  The 'args' variable
 * may be used to pass additional parameters to the action routine.  It is
 * generally defined as one of several types of structures to allow passing
 * multiple parameters.
 *
 * The parse_token() function handles transitioning between nodes.
 */

typedef void (*trans_func)(parseinfo *, transition *, const void * const);
struct transition_ {
    transition *accept;
    transition *alternate;
    const trans_func func;
    const void * const args;
};

/*
 * Dynamic transitions used for adding parse chains at run time
 */
typedef struct dynamic_transition_ dynamic_transition;
struct dynamic_transition_ {
    dynamic_transition *accept;
    dynamic_transition *alternate;
    const trans_func func;
    const void *args;
};

typedef void (*command_func)(parseinfo *);

/* The parseinfo struct is used by the parser routines for
 * parameter storage between the parser and command action routines.
 * The parser and action functions must agree as to where 
 * values are stored in this struct.  Generally, this is
 * in order in which the values appear in the input stream.
 *
 * Parser internal state info is also kept here.
 */


struct parseinfo_ {

/*** Start of cleared area ***/

/*** Note that csb_start_data is defined as txt1 below! ***/

  /* Variable storage for parsed data being passed to functions */
  char txt1[PARSEBUF], txt2[PARSEBUF], txt3[PARSEBUF];  /* String data */
  char txt4[PARSEBUF], txt5[PARSEBUF], txt6[PARSEBUF];  /* String data */
  char txt7[PARSEBUF], txt8[PARSEBUF]/*, txt9[PARSEBUF]*/;      /* String data */

  uint val1, val2, val3, val4, val5;    /* Integer values */
  uint val6, val7, val8, val9, val10;
  uint val11, val12, val13, val14, val15;

  addrtype addr1, addr2, addr3, addr4, addr5, addr6, addr7, addr8;

  int interface;

  void *vptr0, *vptr1;

  void *udb1,*udb2;

  boolean leave_subcommand;

  parser_mode *pmp;

  uint end_of_data;                     /* Dummy to mark end of data area */

/*** End of cleared area ***/

  /* The below two variables are used in determining the command action
   * routine to execute.
   * 'which' is the command type code passed in the EOL structure and is
   * used by the called function to switch out to the command action routine.
   * An example is CMD_CLEAR, which causes the clear_command() function to
   * be called.
   * 'func' is a function code which determines which of several command
   * functions was requested.  An example is CLEAR_VINES_ROUTE which is
   * used by the clear_command() function to decide which action to take.
   */

  uint which;                           /* Which command type being executed */
  uint func;                            /* Command function being executed */

  boolean sense;                        /* FALSE if we parsed a 'no' prefix.  Also
                                                         * FALSE if a 'default' prefix parsed
                                                         * because the off state is the default
                                                         * for most commands.
                                                         */

  boolean set_to_default;       /* TRUE if we parsed a 'default'
                                                         * prefix
                                                         */

  int resolvemethod;            /* Config source: term, mem, or net */
  uint flags;                           /* misc flags */

  
  char *batch_buf;          /* Batch input buffer.  NULL pointer if 
                             * no batch buffer. 
                             */

  char *batch_offset;           /* Current offset into batch buffer */

  /* NV generation variables */
  boolean nvgen;                         /* We're doing NV generation */
  char nv_command[PARSEBUF]; /* The command being generated */

  /* Structures for saving comments, help output, and NV generation */
  save_buf comment_save, help_save;
  int saved_help_used;                  /* How many bytes of help we've seen */

  /* Parser internal state info */

  /* The number of commands which matched the input stream,
   * the function to call if only one matched, and a pointer to any
   * additional parseinfo structures we created.
   */
  queuetype *tokenQ;                    /* Token processing queue. This is a
                                                                 * push-down queue.  The top of the 
                                                                 * stack is the next element to
                                                                 * process.
                                                                 */
  ambig_struct visible_ambig;
  ambig_struct hidden_ambig;

  boolean command_visible;      /* TRUE if current keyword is visible */
  boolean unsupported;          /* TRUE if current keyword is unsupported */

  void (* action_func)(parseinfo *);    /* Command function to call */
  struct parseinfo_ *next;                              /* Next structure, or NULL */

  int in_help;                          /* We're processing a help request */

  uchar priv_set;                       /* If non-zero, then set keywords to
                                                         * this privilege level.
                                                         */

  uchar priv;                                   /* User's current privilege level */
  uchar highest_priv;                   /* Highest keyword priv level parsed */
  uchar last_priv;                              /* Privilege level of last keyword */
  queuetype *privQ;                             /* stack of transitions that will have
                                                                 * their privilege level changed */

  queuetype *priv_nvgenQ;               /* stack for doing NV generation */

  int exit_all;                                 /* Exit flag */

  char prompt[MAXPROMPTLEN+1];          /* Command line prompt */
  char line_buffer[PARSEBUF];           /* String being consumed */

  char *line;                                           /* Pointer to string being consumed */
  int line_index;                                       /* Current token ptr into line */
  int error_index;                                      /* Position of error in input line */
  char *break_chars;                            /* Texti break character set */
  char lastchar;                                        /* last char read by readline() */
  parser_mode *mode;                            /* Current parser mode */
  command_func command_function;        /* Substitute function on accept */

  /* Microweb Specific Additions */
  ushort MoreStatus;                /* Who does 'more' etc. ? */
  SEM_ID configSemId;               /* Semaphore to hold for configuration */
  boolean giveConfigSem;            /* Easy access to release configuration 
                                       semaphore */
  
  int inFd;                                                     /* Input 'fd'  */
  int outFd;                        /* Output 'fd'  */
  int savedFd;                                          /* saved outFd when output is redirected */
  void *pIndex;                     /* Used for 'more' kind of processing, 
                                       cookie */
  long maxRecordCount;              /* Used for 'more'; Maximum record count */
  int moduleId;                     /* Who owns this CSB ? */
  

  int uid;                         /* UID of the I/O Module */
  
  int ioTaskId;                     /* Task Id of the I/O Module */
  timer_t io_timer;                                     /* io input timeout */
  boolean return_val;                           /* return value for command */
  int errno_val;                                        /* error value for command */
  void *userDataBlock[2];
  /*  struct parser_session *session;    parser session, if one exists */

};

/* Start and end of variable area (for clearing purposes) */

#define csb_start_data txt1                     /* First address to clear */
#define csb_end_data end_of_data        /* Last address + 1 to clear */

#define PFLAG_in_help   in_help         /* Help flag */
#define PFLAG_nvgen     nvgen           /* NV gen flag */
#define PFLAG_sense     sense           /* sense flag */
#define PFLAG_which     which           /* which flag */

#define PTXT_0                  nv_command
#define PTXT_1                  txt1
#define PTXT_2                  txt2
#define PTXT_3                  txt3
#define PTXT_4                  txt4
#define PTXT_5                  txt5
#define PTXT_6                  txt6
#define PTXT_7                  txt7
#define PTXT_8                  txt8
#define PTXT_9                  txt9
#define PTXT_prompt             prompt
#define PTXT_line               line
#define PTXT_break_chars        break_chars

#define PVPTR_0         vptr0
#define PVPTR_1         vptr1


#define PCHAR_lastchar          lastchar

#define PTYPE_int(num) val ## num       /* Integer */
#define PTYPE_char(num)  PCHAR ## num
#define PTYPE_string(num) PTXT_ ## num  /* String */
#define PTYPE_paddr(num) addr ## num    /* Protocol address ptr */
#define PTYPE_hwaddr(num) hwaddr ## num /* Hardware address ptr */
#define PTYPE_maskedAddr(num) maskedAddr ## num /* masked address ptr */
#define PTYPE_flag(num) PFLAG_ ## num   /* Parser flags */
#define PTYPE_vptr(num) PVPTR_ ## num   /* Parser void* */
#define PUDB_current1 userDataBlock[0]
#define PUDB_current2 userDataBlock[1]
#define PUDB_1 udb1
#define PUDB_2 udb2
#define PTYPE_udb(num) PUDB_ ## num	/* User Defined data block ptr */


#define PPTR_int
#define PPTR_string
#define PPTR_paddr &
#define PPTR_hwaddr &
#define PPTR_maskedAddr &
#define PPTR_flag
#define PPTR_vptr
#define PPTR_udb

#define GETOBJ(type,num) (PPTR_ ## type ## (csb->PTYPE_ ## type ## ( ## num ## )))
#define SETOBJ(type,num) csb->PTYPE_ ## type ## ( ## num ## )
#define ISOBJ(type,num) ((int) GETOBJ(type,num) != 0)

/************************************************************************/
/*********** END OF NEW PARSER STRUCTURE ********************************/
/************************************************************************/

/***********************************************************************
 *
 *                      Externs and Prototypes
 *
 ***********************************************************************/

/*
 * parser.c
 */
typedef boolean (*match_gen_func)(parseinfo *, char *, int *, void *,
                                  int, uint, void *);
typedef char * (*short_help_func)(parseinfo *, uint, void *);

extern void parser_add_address(const uint, short_help_func, match_gen_func);
extern void exit_config_submode(parseinfo *);
extern void exit_config_modes(parseinfo *);
extern void parse_configure (char *, int , int , uchar );
extern void wait_for_system_load( void );
extern boolean yes_or_no(const char *prompt, boolean def, boolean usedef);
#endif __CONFIG_H__

