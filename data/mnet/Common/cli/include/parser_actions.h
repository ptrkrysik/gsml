#ifndef __PARSER_ACTIONS_H__
#define __PARSER_ACTIONS_H__
 
/******************************************************************
 *                      Macros
 ******************************************************************/

#define SHORTHELP_LEN           32

#define isodigit(c)             (((c) >= '0') && ((c) <= '7'))
#define is_terminator(c)        (((c)=='\0') || ((c)=='!') || ((c)==';'))
#define CVHTOB(c)       ((int) c - \
                         (((c >= '0') && (c <= '9')) ? \
                          '0' : \
                          (((c >= 'a') && (c <= 'f')) ? \
                                 'a'-10 : \
                                 'A'-10)))

#define INVALID_NVGEN_CHECK\
    if (csb->nvgen) {\
        if (!csb->priv_set) {\
            printf("ERROR:INVALID_NVGEN_CHECK\n");\
        }\
        return;\
    }


/******************************************************************
 *                      Defines
 ******************************************************************/

/* Command completion flags */
#define NO_COMMAND_COMPLETION           0
#define SUPPORT_COMMAND_COMPLETION      1

#define ADDR_PARSE(n)   0xf000+n

#define ADDR_NAME_LOOKUP        0x00000001
#define ADDR_LIL_ENDIAN         0x00000002
#define ADDR_HOST_ADDR_ONLY     0x00000004
#define ADDR_HOST_NET_ONLY      0x00000008
#define ADDR_PROTO_ADDR         0x00000010
#define ADDR_NO_SHORT_HELP      0x80000000
#define ADDR_ALL                        0xffffffff

/* Flag values for number_struct types field. */

#define NUMBER_HEX              0x00000001 /* hexadecimal number             */
#define NUMBER_OCT              0x00000002 /* octal number                   */
#define NUMBER_DEC              0x00000004 /* decimal number                 */
#define NO_NUMBER               0x00000008 /* don't parse number if !sense   */
#define HEX_ZEROX_OK            0x00000010 /* hex number can have '0x000'    */
#define HEX_NO_ZEROX_OK         0x00000020 /* hex number can have no '0x000' */
#define HEX_ZEROX_MATCHED       0x00000040 /* hex number matched '0x000'     */
#define HEX_ZEROX_NOT_MATCHED   0x00000080 /* hex number didn't match '0x000'*/
#define OCT_ZERO_OK             0x00000100 /* oct number can have '0'        */
#define OCT_NO_ZERO_OK          0x00000200 /* oct number can have no '0'     */
#define OCT_ZERO_MATCHED        0x00000400 /* oct number matched '0'         */
#define OCT_ZERO_NOT_MATCHED    0x00000800 /* oct number didn't match '0'    */

#define SIGNED_NUMBER           0x00001000 /* parse signed number            */

#define NUMBER_WS_OK            0x00100000 /* number can have whitespace     */
#define NUMBER_NO_WS_OK         0x00200000 /* number can have no whitespace  */
#define KEYWORD_WS_OK           0x00400000 /* keyword can have whitespace    */
#define KEYWORD_NO_WS_OK        0x00800000 /* keyword can have no whitespace */

#define NUMBER_HELP_CHECK       0x01000000 /* give help                      */
#define KEYWORD_HELP_CHECK      0x02000000 /* give help                      */

#define KEYWORD_OR_SET          0x04000000 /* OR value instead of set        */
#define KEYWORD_TEST            0x08000000 /* Test value before parsing      */
#define NUMBER_NV_RANGE         0x10000000 /* Do NV of number range          */

#define STRING_WS_OK            NUMBER_WS_OK /* Whitespace allowed in string */
#define STRING_HELP_CHECK       NUMBER_HELP_CHECK /* Provide help for string */
#define STRING_QUOTED           0x20000000 /* Quotes aren't included         */

/*
 * Command privileges.  The privilege levels can be 1-255 (stored
 * as uchar).  Privilege level 0 is reserved
 * to indicate that normal processing of keywords should
 * be done.  If 'priv_set' is non-zero, then the keywords which match
 * should copy 'priv_set' into their 'priv' variable.
 *
 */

/*
 * Privilege level is stored in a unsigned integer broken
 * down like the following.
 *
 * bits 0-3     current privilege level
 * bits 4-7     original privilege level when privilege level has been changed
 * bits 8       indicates current privilege level has changed
 * bits 9-22    unused
 * bits 23              indicate distilled configuration
 * bits 24-31   privilege modification flags
 *
 */

#define PRIV_MIN        0x0
#define PRIV_NULL       0x0             /* PRIV level means nothing. */
#define PRIV_USER       0x1             /* No special privileges */
#define PRIV_OPR        0xf             /* Operator */
#define PRIV_CONF       0xf             /* Allowed to do configuration */
#define PRIV_ROOT       0xf             /* The Supreme Administrator */
#define PRIV_MAX        0xf
#define PRIV_INTERACTIVE 0x00400000     /* Interactive command */
#define PRIV_CHANGED    0x00000100      /* Privilege level has changed */
#define PRIV_DISTILLED  0x00800000      /* Configuration to be distilled */
#define PRIV_INTERNAL   0x01000000      /* Command "for internal use only" */
#define PRIV_UNSUPPORTED 0x02000000     /* Hidden unsupported command */
#define PRIV_USER_HIDDEN 0x04000000     /* Hidden command from disabled user */
#define PRIV_SUBIF      0x08000000      /* Allowed on subinterfaces */
#define PRIV_HIDDEN     0x10000000      /* Hidden commands.  Subtree rooted at
                                         * this keyword is not searched. */
#define PRIV_DUPLICATE  0x20000000      /* Keyword is a duplicate for
                                         * split parse chains */
#define PRIV_NONVGEN    0x40000000      /* Keywords that do not NVGEN */
#define PRIV_NOHELP     0x80000000      /* Keywords that do not provide help */
#define DEFAULT_PRIV    PRIV_USER       /* Default user privilege level */

#define PRIV_PSYNALC    0x00200000      /* Keywords that are not used by synalc */

#define TRANS_PRIV_PRIV_MASK    0xf
#define TRANS_PRIV_FLAGS_OFFSET 9
#define TRANS_PRIV_FLAGS_GET(priv) ((priv) << TRANS_PRIV_FLAGS_OFFSET)
#define TRANS_PRIV_FLAGS_SET(priv) ((priv) >> TRANS_PRIV_FLAGS_OFFSET)

/*
 *
 */
#define RMATCH_IP               0x00100000
#define RMATCH_OSI              0x00200000
#define RMATCH_NOVELL           0x00400000
#define RMATCH_NONE             0x80000000

#define RMATCH_ARGS             0x01000000
#define RMATCH_NO_ARGS          0x02000000
#define RMATCH_TAG_REQUIRED     0x04000000
#define RMATCH_NO_TAG           0x08000000
#define RMATCH_NO_KEY_HELP      0x10000000

/******************************************************************
 *                      Data Structures
 ******************************************************************/


typedef struct trans_priv_ {
     uint cur_priv : 4;
     uint def_priv : 4;
     uint PACKED(flags : 23);
     uint PACKED(priv_changed : 1);
#ifdef OBJECT_4K
     uint pad;   /* see CSCdi32916, before eliminating this pad */
#endif
} trans_priv;

typedef const struct general_keyword_struct_ {
    const char *str;    /* The string to match */
    const char *help;   /* Help string */
    trans_priv *priv;   /* Privilege and flags of this keyword */
    int offset;         /* CSB offset to store value if keyword matches */
    uint val;           /* Value to store */
    int minmatch;       /* min number of chars required to match */
    uint flags;         /* white space and help flags */
} general_keyword_struct;

typedef const struct keyword_struct_ {
    const char *str;
    const char *help;
    trans_priv *priv;
} keyword_struct;

typedef const struct keyword_mm_struct_ {
    const char *str;
    const char *help;
    trans_priv *priv;
    int   minmatch;
} keyword_mm_struct;

typedef const struct keyword_id_struct_ {
    const char *str;    /* The keyword */
    const char *help;   /* Help string for this keyword */
    trans_priv *priv;   /* Privilege of this keyword */
    int   offset;       /* CSB offset to store value if keyword matches */
    uint  val;          /* Value to store */
} keyword_id_struct;

typedef struct dynamic_number_struct_ {
    int offset;         /* CSB offset to store parsed number */
    uint lower;         /* Lower limit we'll accept */
    uint upper;         /* Upper limit */
    const char *help;   /* The long help string */
    uint flags;         /* flags: hex, octal, decimal allowed,
                           whitespace allowed, to help or not to help   */
} dynamic_number_struct;

typedef const struct dynamic_number_struct_ number_struct;

typedef const struct signed_number_struct_ {
    int offset;         /* CSB offset to store parsed number */
    signed int lower;   /* Lower limit we'll accept */
    signed int upper;   /* Upper limit */
    const char *help;   /* The long help string */
} signed_number_struct;

typedef void (*parser_number_func)(parseinfo *, uint *, uint *);





typedef const struct keyword_id_mm_struct_ {
    const char *str;    /* The keyword */
    const char *help;   /* Help string for this keyword */
    trans_priv *priv;   /* Privilege of this keyword */
    int   offset;       /* CSB offset to store value if keyword matches */
    uint  val;          /* Value to store */
    int   minmatch;     /* minimum number of char to match */
} keyword_id_mm_struct;

typedef const struct keyword_options_ {
    const char *keyword;
    const char *help;
    uint val;
} keyword_options;

extern keyword_options permitdeny_options[];

typedef const struct keyword_option_struct_ {
    keyword_options *options;
    uint offset;
    trans_priv *priv;
    uint flags;
} keyword_option_struct;


#ifdef IOS_PARSER

typedef const struct number_func_struct_ {
    int offset;                 /* CSB offset to store parsed number */
    parser_number_func func;    /* Function to call to set range */
    const char *help;           /* The long help string */
} number_func_struct;

typedef void (*parser_number_help_func)(parseinfo *, uint *, uint *,const char **);
typedef const struct number_help_func_struct_ {
    int offset;                    /* CSB offset to store parsed number */
    parser_number_help_func func;  /* Function to set range and help string */
} number_help_func_struct;


#define PARAMS_STRUCT_COMMON \
    const char *str;    /* Keyword to match */ \
    int offset;         /* CSB offset to store parsed number */ \
    uint lower;         /* Lower range of parsed number */ \
    uint upper;         /* Upper range of parsed number */ \
    void *func;         /* func to call if parse is successful */ \
    int subfunc;        /* Subfunction code */ \
    const char *keyhelp;/* Keyword help string */ \
    const char *varhelp;/* Variable help string */ \
    trans_priv *priv;   /* Privilege of the keyword */ \
    uint flags;         /* Look for trailing whitespace, no whitespace, etc */

typedef const struct params_struct_ {
    PARAMS_STRUCT_COMMON
} params_struct;

typedef const struct params_struct_alist {
    PARAMS_STRUCT_COMMON
    uint lower2;        /* Lower range of alternate parsed number */
    uint upper2;        /* Upper range of alternate parsed number */
} params_struct_alist;

typedef const struct print_struct_ {
    const char * str;
} print_struct;
 
typedef const struct char_struct_ {
  const character;
} char_struct;

typedef const struct string_struct_ {
    int offset;         /* CSB offset to store parsed string */
    const char *help;   /* Help message for this string */
} string_struct;

typedef const struct test_bool_struct_ {
    boolean *var;               /* Pointer to the variable to be tested */
    transition *zero;           /* Go here if variable == NULL */
    transition *nonzero;        /* Go here if variable != NULL */
} test_bool_struct;

typedef const struct test_int_struct_ {
    int *var;                   /* Pointer to the variable to be tested */
    transition *zero;           /* Go here if variable == NULL */
    transition *nonzero;        /* Go here if variable != NULL */
} test_int_struct;

typedef const struct test_func_struct_ {
    boolean (* func)(void);
    transition *zero;           /* Go here if variable == NULL */
    transition *nonzero;        /* Go here if variable != NULL */
} test_func_struct;

typedef const struct test_line_struct_ {
    uint type;                  /* bit field of valid line types */
    transition *zero;           /* Go here if line range is valid */
    transition *nonzero;        /* Go here if line range is invalid */
} test_line_struct;

typedef const struct hexdata_struct_ {
    int bufoffset;              /* CSB offset to store parsed hex bytes */
    int numoffset;              /* CSB offset for number of bytes stored */
    const char *help;   /* Help string for this address */
    uint bufmax;                /* number of bytes in output buffer */
} hexdata_struct;

typedef const struct char_struct2_ {
    const char character;
    const char *help;
} char_struct2;

/* TCP & UDP port information */

typedef const struct portinfo_ {
    ushort port;                        /* port number */
    const char *str;                    /* port name */
    const char *help;                   /* help string */
    uchar minmatch;                     /* min match */
} portinfo;

typedef const struct portparse_struct_ {
  int offset;                           /* CSB offset to store parsed port */
} portparse_struct;

/* New protocol name/address parsing scheme
 *
 * The specification of which protocols are valid for parsing is done
 * using a string, interpreted as a series of two-character codes.
 * My convension is that protocol groups are named with two capital
 * letters, and individual protocols are named with a capital and
 * a lowercase letter.  However, this convension is nowhere enforced
 * or relied on by code.  It serves mearly to help me keep these
 * names unique.
 *
 * Any changes to this list should be reflected in the link_names
 * table in actions.c that maps codes (both individual and group)
 * to names and LINK_... macros.
 */

#define PMATCH_EXCLUDE          "E:"    /* Exclude following protocols */
#define PMATCH_INCLUDE          "I:"    /* Include following protocols */

#define PMATCH_ALL              "AL"    /* All protocols */
#define PMATCH_DGRAM            "DG"    /* All datagram protocols */

#define PMATCH_CDP              "Cd"
#define PMATCH_IP               "Ip"


typedef const struct protoname_struct_ {
  int variable;                 /* CSB offset to store parsed link type */
  const char *match_spec;       /* Specification of acceptable protocols */
} protoname_struct;

typedef const struct protoaddr_struct_ {
    int num_offset;
    int addr_offset;
    const char *match_spec;
} protoaddr_struct;


#endif /* IOS_PARSER */





typedef const struct help_struct_ {
  const char *str;
  transition *eol;
} help_struct;

typedef const struct general_string_struct_ {
    int offset;         /* CSB offset to store parsed string */
    const char *help;   /* Help message for this string */
    uint flags;         /* Flags affecting how string is parsed */
} general_string_struct;

typedef const struct set_struct_ {
    int offset;         /* CSB offset to store value */
    uint val;           /* Value to store */
} set_struct;

typedef const struct test_struct_ {
    int offset;         /* CSB offset to numeric variable to test */
    uint        val;    /* The value to test it against */
    transition *equal;  /* Go here if == */
    transition *notequal;       /* Go here if != */
    transition *lt;             /* Go here if < */
    transition *gt;             /* Go here if > */
} test_struct;

typedef const struct eols_struct_ {
        void    *func;
        int     subfunc;
} eols_struct;

typedef const struct nvgens_struct_ {
    void *func;
    int subfunc;
} nvgens_struct;

typedef const struct addrparse_struct_ {
    int offset;         /* CSB offset to store parsed address */
    const char *help;   /* Help string for this address */
    uint type;          /* Type of addres to parse */
    uint flag;          /*  How to parse (e.g. addr only, addr or hostname) */
} addrparse_struct;


/* Same thing as an addrparse_struct, except without the flag */
typedef const struct addrparse_nf_struct_ {
    int offset;         /* CSB offset to store parsed address */
    const char *help;   /* Help string for this address */
    uint type;          /* Type of addres to parse */
} addrparse_nf_struct;


typedef const struct ipaddrparse_struct_ {
    int offset;         /* CSB offset to store parsed address */
    const char *help;   /* Help string for this address */
    int host;
    int flag;
} ipaddrparse_struct;

typedef const struct ipmask_struct_ {
    int mask;           /* CSB offset to store parsed mask */
    const char *help;   /* Help string */
} ipmask_struct;

typedef const struct ipaddrmask_struct_ {
    int addr;           /* CSB offset to store parsed address */
    int mask;           /* CSB offset to store parsed mask */
    const char *help;   /* Help string */
} ipaddrmask_struct;


/* Month of the year */

typedef const struct month_struct_ {
    int offset;         /* CSB offset */
    const char *help;
} month_struct;

/* Day of the week */

typedef const struct day_struct_ {
    int offset;         /* CSB offset */
    const char *help;
} day_struct;

typedef const struct timesec_struct_ {
    int var1;
    int var2;
    int var3;
    const char *help;
} timesec_struct;

typedef boolean (* testexpr_func)(parseinfo *, transition *);
typedef const struct testexpr_struct_ {
    transition *fal;
    const testexpr_func func;
} testexpr_struct;

typedef struct addr_func_ {
    uint addr_type;
    short_help_func get_short_help;
    match_gen_func func;
} addr_func;

typedef struct priv_struct_ {
    trans_priv *priv;
    transition *fail;
} priv_struct;

typedef const struct mode_struct_ {
    int offset;
    uint flags;
} mode_struct;


extern addr_func *addr_funcs;

/******************************************************************
 *              Function prototypes
 ******************************************************************/

extern void nvgen_token(parseinfo *, transition *, const char *);
boolean help_check(parseinfo *);
void help_or_error(parseinfo *, int, const char *, boolean);
boolean match_whitespace2(char *, int *, boolean);
boolean match_number(parseinfo *, uint, uint, int *, uint *, uint, const char *);
boolean match_decimal(char *, uint, uint, int *, ushort *);
boolean match_hexadecimal(char *, int *, ushort *);
boolean match_ulong_decimal(char *, ulong, ulong, int *, ulong *);
boolean match_ulong_octal(char *, ulong, ulong, int *, ulong *);
boolean match_ulong_hexadecimal(char *, ulong, ulong, int *, ulong *);
boolean match_char(char *, int *, char);
boolean get_name(char *buf, char *name, int *pi, int buflen);
void save_ambig_string(parseinfo *, const char *, const char *);
void save_help_long(parseinfo *, const char *, const char *, uint);
uint copy_varstring(char *, char *, uint, uint);
void multiple_funcs_action(parseinfo *, transition *, test_struct * const);
void display_action(parseinfo *, transition *, help_struct * const);
void NONE_action(parseinfo *);
void no_alt_action(parseinfo *);
void link_point_action(parseinfo *);
void NOP_action(parseinfo *, transition *);
void general_number_short_help_http(uint *, uint, uint, char *, boolean);
void general_keyword_action(parseinfo *, transition *, general_keyword_struct * const);
void keyword_action(parseinfo *, transition *, keyword_struct * const);
void keyword_mm_action(parseinfo *, transition *, keyword_mm_struct * const);
void keyword_id_action(parseinfo *, transition *, keyword_id_struct * const);
void keyword_orset_action(parseinfo *, transition *, keyword_id_struct * const);
void keyword_ortest_action(parseinfo *, transition *, keyword_id_struct * const);
void keyword_optws_action(parseinfo *, transition *, keyword_struct * const);
void eol_action(parseinfo *, transition *, void (*)(parseinfo *));
void eols_action(parseinfo *, transition *, eols_struct * const);
void eoli_action(parseinfo *, transition *, eols_struct * const);
void eolns_action(parseinfo *, transition *, void (*)(parseinfo *));
void help_action(parseinfo *, transition *, help_struct * const);
void func_action(parseinfo *, transition *, void (*)(parseinfo *));
void set_action(parseinfo *, transition *, set_struct * const);
void test_action(parseinfo *, transition *, test_struct * const);
void ipmask_action (parseinfo *, transition *, ipmask_struct * const);
void ipaddrmask_action (parseinfo *, transition *, ipaddrmask_struct * const);
void day_action(parseinfo *, transition *, day_struct * const);
void month_action(parseinfo *, transition *, month_struct * const);
void timesec_action(parseinfo *, transition *, timesec_struct * const);
void nvgens_action(parseinfo *, transition *, nvgens_struct * const);
void nvgenns_action(parseinfo *, transition *, void (*)(parseinfo *));
void noprefix_action(parseinfo *, transition *);
void general_number_action(parseinfo *, transition *, number_struct * const);
void signed_number_action(parseinfo *, transition *, signed_number_struct * const);
void generic_addr_action(parseinfo *, transition *, addrparse_struct * const);
void generic_addr_noflag_action(parseinfo *, transition *, addrparse_nf_struct * const);
boolean eol_check(parseinfo *);
void priv_action(parseinfo *, transition *, priv_struct * const);
void mode_action(parseinfo *, transition *, mode_struct *const);
void comment_action(parseinfo *, transition *);
void testexpr_action(parseinfo *, transition *, testexpr_struct * const);
void general_string_action(parseinfo *, transition *, general_string_struct * const);
boolean match_partial_keyword(parseinfo *, const char *, int *pi,
                              const char *, int, uint, uint);
void keyword_option_action(parseinfo *, transition *, keyword_option_struct * const);

#ifdef IOS_PARSER 

boolean match_octal(char *, uint, uint, int *, ushort *);
boolean match_minusone(char *, int *);
void general_number_short_help(uint *, uint, uint, char *);
void whitespace_action(parseinfo *, transition *);
void print_action(parseinfo *, transition *, print_struct * const);
void char_action(parseinfo *, transition *, char_struct * const);
void char_number_action(parseinfo *, transition *, number_struct * const);
void test_boolean_action(parseinfo *, transition *, test_bool_struct * const);
void test_int_action(parseinfo *, transition *, test_int_struct * const);
void test_func_action(parseinfo *, transition *, test_func_struct * const);
void number_func_action(parseinfo *, transition *, number_func_struct * const);
void number_help_func_action(parseinfo *, transition *, number_help_func_struct * const);
void hexdata_action(parseinfo *, transition *, hexdata_struct * const);
void keyword_id_mm_action(parseinfo *, transition *, keyword_id_mm_struct * const);
void alt_help_action(parseinfo *, transition *, void (*)(parseinfo *));
void alt_helps_action(parseinfo *, transition *, eols_struct * const);
void params_action(parseinfo *, transition *, params_struct * const);
void params_action_alist(parseinfo *, transition *, params_struct_alist * const);
void keyword_nows_action(parseinfo *, transition *, keyword_struct * const);

void keyword_trans_action(parseinfo *, transition *, keyword_struct *const);

#endif  /* IOS_PARSER */
#endif  /* __PARSER_ACTIONS_H__ */






