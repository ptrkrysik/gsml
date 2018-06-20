#ifndef _PARSER_DEFS_H
#define _PARSER_DEFS_H

#ifdef NONE
#undef NONE
#endif

#define pname(x) PARSER_ ## x
#define pdecl(x) extern transition pname(x)
#define lpdecl(x) extern dynamic_transition pname(x);

#define CSBOFF(elem) ((int) &((parseinfo*)NULL)->elem)
#define CSBVAR(csb, offset, type)  ((type *) ((u_char*) csb + offset))

extern transition PARSER_NONE;

pdecl(no_alt);
pdecl(link_point);
pdecl(link_trans);

/*
 * List structure for parser extension
 */
typedef const struct {
    uint type;                  /* Type code */
    dynamic_transition *commands;       /* Transition pointer */
} parser_extension_request;

/*
 * Extending the parser.
 * Some brave soul should turn this into an enum and rearrange things
 * intelligently. 
 */
enum {
    PARSE_LIST_END=0,                   /* End of list - not a real code */
    PARSE_ADD_EXEC_CMD,
    PARSE_ADD_SHOW_CMD,
    PARSE_ADD_CLEAR_CMD,
    PARSE_ADD_DEBUG_CMD,
    PARSE_ADD_TERM_CMD,                 /* "term xxx" commands, not saved */
    PARSE_ADD_CFG_TOP_CMD,              /* top level config commands */
    PARSE_ADD_GLOBAL_CMD,               /* global commands */
    PARSE_ADD_PING_CMD,                 /* exec ping commands */
    PARSE_ADD_COPY_CMD,                 /* copy commands */
    PARSE_ADD_COPYFILE_CMD,             /* copy file commands */
    PARSE_ADD_CFG_LAST_CMD,             /* top level config commands at end */
    PARSE_ADD_CFG_CDP,                  /* Cisco Discovery Protocol */
    PARSE_ADD_WRITE_CMD,                /* to add dynamic write commands */
    PARSE_ADD_CFG_FILE_CMD,             /* to add config file command */
    PARSE_ADD_COPY_RUNNING_CMD,         /* extending copy running-config */
    PARSE_ADD_COPY_STARTUP_CMD,         /* extending copy startup-config */
    PARSE_ADD_COPY_FLASH_CMD,           /* extending copy flash */
    PARSE_ADD_COPY_DISK_CMD,            /* extending copy disk cmd */
    PARSE_ADD_COPY_TFTP_CMD,            /* extending copy tftp cmd */
    PARSE_ADD_COPY_RUNNING_FILE_CMD,    /* extending copy running-config */
    PARSE_ADD_COPY_STARTUP_FILE_CMD,    /* extending copy startup-config */
    PARSE_ADD_COPY_FLASH_FILE_CMD,      /* extending copy flash */
    PARSE_ADD_COPY_RCP_FILE_CMD,        /* extending copy rcp */
    PARSE_ADD_COPY_TFTP_FILE_CMD,       /* extending copy tftp cmd */
    PARSE_ADD_ERASE_CMD,                /* to add erase file cmd */
    PARSE_ADD_CFG_LOG_CMD,              /* syslog history config cmd */
    PARSE_ADD_SHOW_LOG_CMD,             /* syslog history show cmd */
    PARSE_ADD_GLOBAL_EXIT,              /* Global command exit */
    PARSE_ADD_SHOW_TECHSUPPORT_CMD,     /* show tech-support commands */
    PARSE_ADD_SHOW_TECHSUPPORT_EXIT,    /* show tech-support exit */
    PARSE_ADD_SHOW_REV_CMD,             /* show revision commands */
    PARSE_ADD_CFG_INTERFACE_TOP_CMD,    /* Config Interface commands */
    PARSE_ADD_CFG_CELL_CMD,             /* config cell mode commands */
    PARSE_ADD_CFG_ADJCELL_CMD,          /* config adjacent cell mode commands */
    PARSE_ADD_CFG_PROTO_CMD,            /* config protocol mode commands */
    PARSE_ADD_SHOW_STATS_CMD,       /* show statistics */
    PARSE_ADD_CLEAR_STATS_CMD,      /* clear statistics */
    PARSE_ADD_CFG_DNS_BOOM_CLIENT_TOP_CMD,/* Config dns-boomerang client cmds */
    PARSE_ADD_CFG_DNS_BOOM_SERVER_TOP_CMD,/* Config dns-boomerang server cmds */
    PARSE_ADD_CFG_GP10_CELL,
    PARSE_ADD_CFG_GP10_ADJCELL,
    PARSE_ADD_CFG_GP10_PROTO,
    PARSE_ADD_CFG_GP10_CELL_TRX,
    PARSE_ADD_CFG_GP10_CELL_BTS,
    PARSE_ADD_CFG_GP10_CELL_CCCH,
	PARSE_ADD_CFG_GMC_CAPS,				/*  Config GMC CAPS Paramaters  */
	PARSE_ADD_CFG_GMC_CCS,				/*  Config GMC CCS Paramaters  */
	PARSE_ADD_CFG_GMC_SYNAXIS,			/*  Config GMC SYNAXIS Paramaters  */
	PARSE_ADD_EXEC_SMSCB_SCHEDULE,		/*  Exec commands for SMS-CB Schedule Messages  */
	PARSE_ADD_EXEC_SMSCB_CHANGE,		/*  Exec commands for SMS-CB Change Messages  */
	PARSE_ADD_CFG_GMC_ITP,				/*  Config GMC ITP Paramaters  */
	PARSE_ADD_CFG_GMC_ITP_PATH			/*  Config GMC ITP Path Paramaters  */

};

#ifdef  PARSER_DEBUG_LINKS
#define parser_add_commands(def, trans, str)\
        parser_add_commands_func(def, trans, str)
#define parser_add_command_list(list, str)\
        parser_add_command_list_func(list, str)
#define parser_add_link_point(def, str, trans)\
        parser_add_link_point_func(def, str, trans)
#define parser_add_link_exit(def, str, trans)\
        parser_add_link_exit_func(def, str, trans)

extern boolean parser_add_commands_func(uint, dynamic_transition *, const char *);
extern boolean parser_add_command_list_func(parser_extension_request *,
                                            const char *);
extern boolean parser_add_link_point_func(uint, const char *, transition *);
extern boolean parser_add_link_exit_func(uint, const char *, transition *);
#else   /* PARSER_DEBUG_LINKS */
#define parser_add_commands(def, trans, str)\
        parser_add_commands_func(def, trans)
#define parser_add_command_list(list, str)\
        parser_add_command_list_func(list)
#define parser_add_link_point(def, str, trans)\
        parser_add_link_point_func(def, trans)
#define parser_add_link_exit(def, str, trans)\
        parser_add_link_exit_func(def, trans)

extern boolean parser_add_commands_func(uint, dynamic_transition *);
extern boolean parser_add_command_list_func(parser_extension_request *);
extern boolean parser_add_link_point_func(uint, transition *);
extern boolean parser_add_link_exit_func(uint, transition *);
#endif  /* PARSER_DEBUG_LINKS */

/*
 * True during reading of the configuration from NVRAM
 */
extern boolean systemloading; 

#endif /* _PARSER_DEFS_H */
