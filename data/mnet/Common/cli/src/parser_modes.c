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
#include <../include/parser_modes.h>



/*
 * Local structure definitions
 */
struct parser_mode_ {
    struct parser_mode_ *next;
    const char *name;
    const char *prompt;
    const char *help;
    const char *alt_mode;
    mode_save_var_func save_vars;
    mode_reset_var_func reset_vars;
    mode_http_init_func http_init;
    uint flags;
    transition *top;

#ifdef INCLUDE_PARSER_ALIAS
    queuetype *aliases;
    queuetype *def_aliases;
#endif
    boolean priv_changed;
};

static queuetype parser_modes_queue;
static queuetype *parser_modes = &parser_modes_queue;
static boolean parser_modes_inited = FALSE;


/*
 * parser_modes_init
 * Initialize well known parser modes and add default aliases
 */
void parser_modes_init (void)
{
    if (!parser_modes_inited) {
                parser_modes_inited = TRUE;
                parser_modes = &parser_modes_queue;
                queue_init(parser_modes, 0);

                parser_add_main_modes();
    }
}


/*
 * create_new_mode
 * Allocate and fill in new mode structure
 */
static parser_mode *create_new_mode (const char *name, const char *prompt,
                                                                         const char *help, 
                                                                         boolean do_aliases, boolean do_privileges,
                                                                         const char *alt_mode,
                                                                         mode_save_var_func save_vars,
                                                                         mode_reset_var_func reset_vars,
                                                                         transition *top,
                                                                         mode_http_init_func http_init)
{
    parser_mode *new_mode;

    if (!parser_modes_inited) {
                parser_modes_init();
    }

    new_mode = (parser_mode *)PARSERmalloc(sizeof(parser_mode) );
    if (new_mode == NULL) {
                return(NULL);
    }

    new_mode->name = name;
    new_mode->prompt = prompt;
    new_mode->help = help;
    new_mode->alt_mode = alt_mode;
    new_mode->save_vars = save_vars;
    new_mode->reset_vars = reset_vars;
    new_mode->flags = PARSER_MODE_NONE;
    new_mode->next = NULL;

    if (do_aliases) {
                new_mode->flags |= PARSER_MODE_ALIASES;
    }
    if (do_privileges) {
                new_mode->flags |= PARSER_MODE_PRIVILEGE;
    }
    new_mode->top = top;

#ifdef INCLUDE_PARSER_ALIAS
    new_mode->aliases = (queuetype *)PARSERmalloc(sizeof(queuetype) );

    if (new_mode->aliases) {
                queue_init(new_mode->aliases, 0);
    }
    new_mode->def_aliases = (queuetype *)PARSERmalloc(sizeof(queuetype)); 
    if (new_mode->def_aliases) {
                queue_init(new_mode->def_aliases, 0);
    }
#endif

    new_mode->priv_changed = FALSE;
    new_mode->http_init = http_init;

    if (parser_mode_debug) {
                printf("\nAdding parser mode: '%s'  0x%x", name, (uint)new_mode);
    }

    return(new_mode);
}


/*
 * parser_add_mode
 * Create and add new parser mode
 *      name - mode name, used in MODE macro for matching, so
 *             can't contain whitespace
 *      prompt - mode prompt extension, ie. (config-if), except
 *               without the parentheses
 *      help - help strings describing mode, used as MODE macro help
 *      alt_mode - alternate mode to try if parse fails
 *      save_vars - function to save csb vars when trying alt mode
 *      reset_vars - function to reset csb vars when alt mode fails
 *      do_aliases - are aliases allowed in this mode?
 *      do_privileges - can privilege levels be changed in this mode?
 *      top - top of parse chain for parsing
 */
parser_mode *parser_add_mode (const char *name, const char *prompt,
                                                          const char *help,
                                                          boolean do_aliases, boolean do_privileges,
                                                          const char *alt_mode,
                                                          mode_save_var_func save_vars,
                                                          mode_reset_var_func reset_vars,
                                                          transition *top,
                                                          mode_http_init_func http_init)
{
    parser_mode *mode;
        
    mode = create_new_mode(name, prompt, help, do_aliases, do_privileges,
                                                   alt_mode, save_vars, reset_vars, top, http_init);
    if (mode) {
                enqueue(parser_modes, mode);
    }
    return(mode);
}       


/*
 * get_mode_tmp
 * Create temporary mode
 */
parser_mode *get_mode_tmp (const char *name, const char *prompt,
                                                   const char *help,
                                                   boolean do_aliases, boolean do_privileges,
                                                   transition *top)
{
    return(create_new_mode(name, prompt, help, do_aliases, do_privileges,
                                                   NULL, NULL, NULL, top, NULL));
}


/*
 * free_mode
 *
 */
void free_mode (parser_mode *mode)
{
#ifdef INCLUDE_PARSER_ALIAS
        free_aliases(mode->aliases);
        free_aliases(mode->def_aliases);
#endif
        PARSERfree(mode);
        
}



/*
 * get_mode_byname
 * Find mode with name
 */
parser_mode *get_mode_byname (const char *name, uint volume)
{
    parser_mode *mode;
        
    if (!parser_modes_inited) {
                parser_modes_init();
    }
        
    mode = (parser_mode *)parser_modes->qhead;
    while (mode) {
                if (strcmp(name, mode->name) == 0) {
                        break;
                }
                mode = mode->next;
    }
    if (parser_mode_debug) {
                printf("\nLook up of parser mode '%s' %s",
                           name, mode ? "succeeded" : "failed");
        }
    if (!mode && (volume == MODE_VERBOSE)) {
                printf("\n%% Unable to find %s mode", name);
    }
        
    return(mode);
}


/*
 * test_mode_config_subcommand
 * Return TRUE if mode is submode of config mode
 */
boolean test_mode_config_subcommand (parseinfo *csb)
{
    if (csb && csb->mode && csb->mode->alt_mode) {
                return(TRUE);
    }
    return(FALSE);
}

parser_mode *get_alt_mode (parser_mode *mode)
{
    if (mode && mode->alt_mode) {
                return(get_mode_byname(mode->alt_mode, MODE_SILENT));
    } else {
                return(NULL);
    }
}


/*
 * get_mode_prompt
 * Return prompt extension for this mode
 */
const char *get_mode_prompt (parseinfo *csb)
{
    if (csb && csb->mode) {
                return(csb->mode->prompt);
    } else {
                return("UNKNOWN-MODE");
    }
}


/*
 * get_mode_top
 * Return top of parse chain for this mode
 */
transition *get_mode_top (parser_mode *mode)
{
    if (mode) {
                return(mode->top);
    } else {
                return(NULL);
    }
}


/*
 * get_mode_nv
 * Return top of parse chain for NV generation for this mode
 */
transition *get_mode_nv (parseinfo *csb)
{
    if (csb && csb->mode) {
                return(csb->mode->top);
    } else {
                return(NULL);
    }
}

void *mode_save_vars (parseinfo *csb)
{
    if (csb && csb->mode &&
                csb->mode->save_vars && csb->mode->reset_vars) {
                return(csb->mode->save_vars(csb));
    } else {
                return(NULL);
    }
}

/*
 * set_mode_byname
 * Set current mode to name
 */
boolean set_mode_byname (parser_mode **mode, const char *name, uint volume)
{
    parser_mode *new_mode;

    new_mode = get_mode_byname(name, volume);
    if (new_mode) {
                *mode = new_mode;
                return(TRUE);
    } else {
                return(FALSE);
    }
}

#ifdef IOS_PARSER

void mode_reset_vars (parseinfo *csb, void *var)
{
    if (csb && csb->mode && csb->mode->reset_vars) {
                csb->mode->reset_vars(csb, var);
    }
}

void set_priv_changed_in_mode (parser_mode *mode, boolean val)
{
    mode->priv_changed = val;
}

boolean mode_http_init (parseinfo *csb)
{
    if (csb && csb->mode && csb->mode->http_init) {
                return(csb->mode->http_init(csb));
    } else {
                return(TRUE);
    }

}

boolean priv_changed_in_mode (parser_mode *mode)
{
    return(mode->priv_changed);
}


/*
 * show_parser_modes
 *
 */
void show_parser_modes (parseinfo *csb)
{
    parser_mode *mode;
        
    mode = parser_modes->qhead;
    if (mode) {
                printf("\nParser modes:");
                printf("\nName                Prompt              Top       "
                           "Alias   Privilege\n");
                while (mode) {
                        printf("%20s%20s0x%8x%8s%8s\n",
                                   mode->name, mode->prompt, (uint)mode->top,
                                   ((mode->flags & PARSER_MODE_ALIASES) ? "TRUE" : "FALSE"),
                                   ((mode->flags & PARSER_MODE_PRIVILEGE) ? "TRUE" : "FALSE"));
                        mode = mode->next;
                }
    } else {
                printf("\nNo parser modes");
    }
}





/*
 * get_mode_aliases
 * Return pointer to aliases in this mode
 */
queuetype *get_mode_aliases (parser_mode *mode)
{
#ifdef INCLUDE_PARSER_ALIAS     
    if (mode) {
                return(mode->aliases);
    } else {
                return(NULL);
    }
#else
        return( NULL ) ;
#endif

}


/*
 * get_mode_defaliases
 * Return pointer to default aliases in this mode
 */
queuetype *get_mode_defaliases (parser_mode *mode)
{
#ifdef INCLUDE_PARSER_ALIAS     
    if (mode) {
                return(mode->def_aliases);
    } else {
                return(NULL);
    }
#else
        return( NULL );
#endif
}


/*
 * get_mode_name
 * Return name of mode
 */
const char *get_mode_name (parser_mode *mode)
{
    return(mode->name);
}



/*
 * match_mode
 * Parser function for matching the MODE macro
 */
uint match_mode (parseinfo *csb, int *pi,
                                 parser_mode **matched_mode, uint flags)
{
    parser_mode *mode;
    uint num_matches = 0;
    int i;
    int matched_pi = *pi;
    char *word;

    mode = parser_modes->qhead;
    while (mode) {
                if (mode->flags & flags) {
                        i = *pi;
                        if (match_partial_keyword(csb, mode->name, &i, mode->help,
                                                                          0, KEYWORD_WS_OK | KEYWORD_HELP_CHECK,
                                                                          csb->priv)) {
                                *matched_mode = mode;
                                matched_pi = i;
                                num_matches++;

                                word = csb->line + csb->line_index;
                                if (strncmp(mode->name, word, strlen(mode->name)) == 0) {
                                        /* This was strncasecmp 
                                         * if (strncasecmp(mode->name, word,
                                         * strlen(mode->name)) == 0) { 
                                         */
                                        *pi = matched_pi;
                                        return(1);
                                }
                        }
                }
                mode = mode->next;
    }
    if (num_matches > 1) {
                *matched_mode = NULL;
                return(0);
    }
    *pi = matched_pi;
    return(num_matches);
}


/*
 * nv_mode
 * Do NV generation for MODE macro
 */
void nv_mode (parseinfo *csb, transition *mine, mode_struct * const arg)
{
    parser_mode *mode;

    mode = parser_modes->qhead;
    while (mode) {
                if (mode->flags & arg->flags) {
                        *CSBVAR(csb, arg->offset, parser_mode *) = mode;
                        nvgen_token(csb, mine, mode->name);
                }
                mode = mode->next;
    }
}

#ifdef INCLUDE_PARSER_ALIAS


/*
 * display_mode_aliases
 * Show mode aliases
 *      if (mp != NULL)
 *          show mp mode aliases
 *      else
 *          show all mode aliases
 */
void display_mode_aliases (parseinfo *csb, parser_mode *mp)
{
    parser_mode *mode;

    mode = parser_modes->qhead;
    while (mode) {
                if (!mp || (mp == mode)) {
                        show_mode_aliases(csb, mode->aliases, mode->help); 
                }
                mode = mode->next;
    }
}

#endif


#endif

