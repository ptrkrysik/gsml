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
#include <../include/parser_sim.h>
#include <../include/ttyutil.h>


static MSG_Q_ID callback_Q_Id = NULL;

#define INVALID_UID  -1


boolean config_prompt = FALSE;
char *nomemory = "\n%%No memory available";

extern parser_mode *interface_mode;

/*
 * Local structures and variables
 */
static chain_list *cl = NULL;   /* Unresolved parse chains */
static chain_list *llist = NULL;        /* Dynamic link points */

/*
 * Forward declarations
 */

static boolean word_completion(parseinfo *csb);
static boolean add_to_unresolved_chains(uint, dynamic_transition *
#ifdef  PARSER_DEBUG_LINKS
                                                                                , const char *
#endif /* PARSER_DEBUG_LINKS */
                                                                                );
static void resolve_parse_chains(void);
static boolean parser_resolve_entry_link_point(chain_link *,
                                                                                           dynamic_transition *
#ifdef  PARSER_DEBUG_LINKS
                                                                                           , const char *
#endif /* PARSER_DEBUG_LINKS */
                                                                                           );
static boolean parser_resolve_exit_link_point(chain_link *,
                                                                                          dynamic_transition *);
static boolean parser_add_link_to_list(uint,
#ifdef  PARSER_DEBUG_LINKS
                                                                           const char *,
#endif /* PARSER_DEBUG_LINKS */
                                                                           dynamic_transition *, uint);
static parsenode *get_parsenode(parseinfo *);
static void parser_exec_callback( void );
static void addSignalHandlers ( void );







static const char *generic_csb_name = "Parser CSB";
parseinfo *get_csb (const char *name)
{
    parseinfo *csb;

    if (!name) {
                name = generic_csb_name;
    }
   
    csb = (parseinfo *)PARSERcalloc(1, sizeof( parseinfo ));

    if (!csb) {
                return(NULL);
    }

    /*
     * Assignments to make purify happy
     */
    csb->tokenQ = NULL;
    csb->privQ = NULL;
    csb->priv_nvgenQ = NULL;
    csb->help_save.buf = NULL;
    csb->comment_save.buf = NULL;

    /* Initialize the parsing stuff */
    csb->line = csb->line_buffer;
    csb->break_chars = EXEC_BREAKSET;
    csb->resolvemethod = RES_NULL;
    csb->flags = 0;
    csb->mode = exec_mode;
    csb->priv = PRIV_MAX;
    csb->priv_set = PRIV_NULL;
        
        csb->uid        = -1;
        csb->MoreStatus = RC_OK;
        csb->savedFd    = -1;

        /*
         * The errno_val will only be valid if return_val is FALSE
         */
        csb->return_val = TRUE;
        csb->errno_val = 0;

        csb->configSemId = PSsemBCreate( PS_DEFAULT_BINARY_SEM_FLAG, SEM_EMPTY );
        if ( csb->configSemId == NULL ) 
                PSpanic("Cannot create configuration semaphore for CSB\n");

        csb->giveConfigSem = FALSE;

    /* Initialize tokenQ */     
    if ((csb->tokenQ = (queuetype *) 
                 PARSERmalloc(sizeof(queuetype))) == NULL) {
                free_csb(&csb);
                return(NULL);
    }
    queue_init(csb->tokenQ, 0);

    return(csb);
}



/*
 * Initialize parser variables
 */
static void parse_cmd_init (parseinfo *csb)
{

    csb->nv_command[0] = '\0';

    csb->error_index = 0;
    csb->nvgen = 0;
    csb->sense = TRUE;
    csb->set_to_default = FALSE;
    csb->saved_help_used = 0;
    save_line_init(&csb->help_save);
    csb->visible_ambig.ambig[0] = '\0';
    csb->visible_ambig.ambig_save[0] = '\0';
    csb->hidden_ambig.ambig[0] = '\0';
    csb->hidden_ambig.ambig_save[0] = '\0';
    set_multiple_funcs(csb, 0);
    csb->command_visible = TRUE;
    csb->unsupported = FALSE;
    csb->action_func = NULLFUNC;
    csb->which = 0;
    csb->func = 0;
    csb->next = NULL;
        csb->return_val = TRUE;
        csb->errno_val = 0;

    /* Clear any lingering flags */
    csb->flags &= (CONFIG_TERM | CONFIG_HTTP | COMMAND_AUTHORIZE |
                                   COMMAND_FUNCTION | HIDE_PASSWORDS | PARSE_EMPTY_STRING |
                                   CONFIG_PER_USER | CONFIG_CGI | COMMAND_COMPLETE);

    /* Zap the data area. */
    memset((void *) &(csb->csb_start_data), 0,
                   (int) ((uchar *) &csb->csb_end_data - 
                                  (uchar *) &csb->csb_start_data));
    csb->last_priv = 0;
    csb->highest_priv = 0;

    parser_ambig_debug = FALSE;
}



/*
 * get_parsenode
 * Get parsenode structure.
 */
static parsenode *get_parsenode (parseinfo *csb)
{
    parsenode *pp = NULL;

    /* pp = chunk_malloc(parsenode_chunks); */
    
    pp = ( parsenode *)PARSERmalloc(sizeof(parsenode));

    if (!pp)
                return(NULL);
    pp->next = NULL;
    pp->visible_ambig[0] = '\0';
    pp->hidden_ambig[0] = '\0';
    pp->nv_command[0] = '\0';
    return(pp);
}

static  void empty_queue (queuetype *qp)
{
    void *tmp;

    while (! QUEUEEMPTY(qp)) {
                tmp = dequeue(qp);
                PARSERfree(tmp);
    }
}

static void free_queue (queuetype **qp)
{
    if (*qp) {
                empty_queue(*qp);
                PARSERfree(*qp);
                *qp = NULL;
    }
}

/*
 * free_csb_mem
 *
 * Free the memory allocated for the various structures within
 * the csb. This includes dirty, token and line Qs and help_save,
 * comment_save fields.
 */
void free_csb_mem (parseinfo *csb)
{
    if (csb) {
                free_queue(&csb->tokenQ);
                free_queue(&csb->privQ);
                free_queue(&csb->priv_nvgenQ);
                save_line_init(&csb->help_save); /* Free help text elements */
                save_line_init(&csb->comment_save); /* Free comment text elements */
    }
}

/* Mind our Free's & Q's */     
void free_csb (parseinfo **csbp)
{
    parseinfo *csb = *csbp;

    if (csb) {
                free_csb_mem(csb);                              /* Free allocated memory hidden in csb */
                /* chunk_free(parseinfo_chunks, csb);*/
                if ( csb->configSemId ) 
                        semDelete( csb->configSemId );
                PARSERfree( csb );
    }
    *csbp = NULL;
}


static int count_help_matches (parseinfo *csb)
{
    char *cp;
    int n = 0;

    for (cp = csb->help_save.buf; cp && *cp; cp++) {
                if (*cp == '\t' || *cp == '\n'){
                        n++;
                }
    }
    return(n);
}




/*
 * Report where we found an error in the command line.
 * "error" indicates what type of error occured.
 * If 'reprint' is TRUE, then we have to reprint the line.
 */
void report_error (parseinfo * csb, int error, boolean need_reprint)
{
    int promptlen;
    int incomplete = 0;
        
    char buffer[80];
    buffer[0]  = '\0';

    if (parser_ambig_debug) {
                need_reprint = TRUE;
    }

    if (error == PARSE_ERROR_UNKNOWN) {
                printf("\n%% Unrecognized command\n");
                parser_return(csb, S_parser_SYNTAX_UNKNOWN);
                return;
    }

    if (error == PARSE_ERROR_AMBIG) {
                printf("\n%% Ambiguous command:  \"%s\"\n", csb->line);
                parser_return(csb, S_parser_SYNTAX_AMBIGUOUS);
                return;
    }

    /* error == PARSE_ERROR_NOMATCH */
    /*
     * Prepare to designate the error.  We have to know the length of the
     * command line prompt and if configuration is from other than a
     * terminal, print the command before indicating the error location.
     */
    if (csb->flags & CONFIG_TERM) {
                promptlen = strlen(csb->prompt);
                /* Reprint if the command is too long. */
                if (promptlen + strlen(csb->line) >= SCREEN_WIDTH ||
                        need_reprint ||
                        (error & PARSE_ERROR_ALIAS)) {
                        promptlen = 0;
                        need_reprint = TRUE;
                }
    } else {
                promptlen = 0;
                need_reprint = TRUE;
    }

    if (csb->line[csb->error_index] == '\0') {
                if (csb->error_index > 0) {
                        /* NON-blank input line. Handle INCOMPLETE error similarly
                           to other errors, below. */
                        incomplete = 1;
                }
    }
    if (need_reprint) {
                int length = csb->error_index;
                int i;
                int base;

                printf("\n");

                if (SCREEN_WIDTH <= 0) {
                        base = length;
                } else {
                        base = length / SCREEN_WIDTH;
                }
                for (i = 0; csb->line[i]; i++) {
                        printf("%c", csb->line[i]); 
                        if ((SCREEN_WIDTH > 0) && ((i+1) % SCREEN_WIDTH == 0)) {
                                /* Next char starts new line. */
                                if (i / SCREEN_WIDTH == base) {
                                        /* We just printed the error line. */
                                        print_caret(length % SCREEN_WIDTH);
                                } else {
                                        /* Not the error line, print a blank line. */
                                        printf("\n"); 
                                }
                        }
                }

                if (! incomplete) {
                        printf("\n"); 

                        if (SCREEN_WIDTH <= 0) {
                                print_caret(length);
                        } else {
                                if (i / SCREEN_WIDTH == base) {
                                        /* We just printed the error line. */
                                        print_caret(length % SCREEN_WIDTH);
                                }
                        }
                }
    } else {
                if (! incomplete) {
                        /* No reprint needed, just show caret. */
                        print_caret(promptlen + csb->error_index);
                }
    }

    if (incomplete) {
                printf("\n%% Incomplete command.\n");
                parser_return(csb, S_parser_SYNTAX_INCOMPLETE);
    } else {
                printf("\n%% Invalid input detected at '^' marker.\n"); 
                parser_return(csb, S_parser_SYNTAX_INVALID);
    }
}

extern void Logout(parseinfo *csb);

/*
 * exit_exec_command
 *
 * Cleanup for an exit from the exec.  Call exit_command() to
 * drop any outstanding connections and set the exit flag.
 */

void exit_exec_command (parseinfo *csb)
{
	Logout(NULL);
		return;
        csb->exit_all = TRUE;
        if ( csb->priv == PRIV_USER ) 
                csb->priv = PRIV_NULL;
        else
        csb->priv = PRIV_MIN;
    return;
}

void set_multiple_funcs (parseinfo *csb, int value)
{
    csb->visible_ambig.multiple_funcs = value;
    csb->hidden_ambig.multiple_funcs = value;
}

void increment_multiple_funcs (parseinfo *csb, char *str)
{
    char *visible_str;
    int value;

    if (csb->command_visible) {
                csb->visible_ambig.multiple_funcs++;
                visible_str = "visible";
                value = csb->visible_ambig.multiple_funcs;
    } else {
                csb->hidden_ambig.multiple_funcs++;
                visible_str = "hidden";
                value = csb->hidden_ambig.multiple_funcs;
    }
    if (parser_help_debug || parser_ambig_debug) {
                printf("\nIncreasing %s multiple_funcs to %d in %s",
                           visible_str, value, str);
    }
        
}

int test_multiple_funcs (parseinfo *csb, char test, int value)
{
    int mf = (csb->visible_ambig.multiple_funcs +
                          csb->hidden_ambig.multiple_funcs);

    switch (test) {
    case '<': return(mf < value);
    case '>': return(mf > value);
    case '=': return(mf == value);
    default:
                printf("test_multiple_funcs: %c\n", test);
                break;
    }
    return(0);
}

int test_visible_multiple_funcs (parseinfo *csb, char test, int value)
{
    int mf = csb->visible_ambig.multiple_funcs;

    switch (test) {
    case '<': return(mf < value);
    case '>': return(mf > value);
    case '=': return(mf == value);
    default:
                /* errmsg(&msgsym(MULFUNCS, PARSER), test); */
                printf("test_multiple_visible_funcs : %c\n",test );
                break;
    }
    return(0);
}

int test_invisible_multiple_funcs (parseinfo *csb, char test, int value)
{
    int mf = csb->hidden_ambig.multiple_funcs;

    switch (test) {
    case '<': return(mf < value);
    case '>': return(mf > value);
    case '=': return(mf == value);
    default:
                printf("\n%% Unknown test in test_multiple_funcs '%c'", test);
                break;
    }
    return(0);
}

void print_multiple_funcs (parseinfo *csb)
{
    printf("\ncsb->visible_ambig.multiple_funcs = %d",
                   csb->visible_ambig.multiple_funcs);
    printf("\ncsb->hidden_ambig.multiple_funcs = %d",
                   csb->hidden_ambig.multiple_funcs);
}


static chain_link *get_link_point (uint which_chain)
{
    int i = 0;
    chain_link *cp;
    chain_list *lp;

    /* Check static link points */
    cp = &chain_gang[i++];
    while (cp && (cp->id != PARSE_LIST_END)) {
                if (cp->id == which_chain) {
                        return(cp);
                }
                cp = &chain_gang[i++];
    }
    /* Check dynamic link points */
    lp = llist;
    while (lp != NULL) {
                if (lp->cp.id == which_chain) {
                        return(&lp->cp);
                }
                lp = lp->next;
    }
    /* Didn't find anything */
    return(NULL);
}

static boolean parser_add_chains (uint which_chain,
                                                                  dynamic_transition *commands,
#ifdef  PARSER_DEBUG_LINKS
                                                                  const char *module,
#endif /* PARSER_DEBUG_LINKS */
                                                                  boolean already_on_list)
{
    chain_link *where;
        boolean status = TRUE;
        
        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }
        
    /*
     * Make sure we have a place to link this
     */
    where = get_link_point(which_chain);
    if (!where) {
                if (already_on_list) {
                        /*
                         * point is already on unresolved list, so return FALSE
                         */
                        status = (FALSE);
                        goto SEMRELEASE;

                } else {
                        /*
                         * No place to add it yet, so place on unresolved list
                         */
                        status = (add_to_unresolved_chains(which_chain, commands
#ifdef  PARSER_DEBUG_LINKS
                                                                                           , module
#endif /* PARSER_DEBUG_LINKS */
                                                                                           ));

                        goto SEMRELEASE;
                }
    }

    switch (where->type) {
    case PARSER_ENTRY_LINK_POINT:
                status = (parser_resolve_entry_link_point(where, commands
#ifdef  PARSER_DEBUG_LINKS
                                                                                                  , module
#endif /* PARSER_DEBUG_LINKS */
                                                                                                  ));
                break;
    case PARSER_EXIT_LINK_POINT:
                status = (parser_resolve_exit_link_point(where, commands));
                break;
    default:
                status = FALSE;
    }


 SEMRELEASE:

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }
        
        return(status);

}
       
boolean parser_add_commands_func (uint which_chain,
                                                                  dynamic_transition *commands
#ifdef  PARSER_DEBUG_LINKS
                                                                  , const char *module
#endif /* PARSER_DEBUG_LINKS */
                                                                  )
{
        boolean status ; 

        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }

        status = (parser_add_chains(which_chain, commands,
#ifdef  PARSER_DEBUG_LINKS
                                                                module,
#endif /* PARSER_DEBUG_LINKS */
                                                                FALSE));

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }

        return(status);
}

static boolean parser_resolve_entry_link_point (chain_link *where,
                                                                                                dynamic_transition *commands
#ifdef  PARSER_DEBUG_LINKS
                                                                                                ,const char *module
#endif /* PARSER_DEBUG_LINKS */
                                                                                                )
{
    dynamic_transition *tw;

    /*
     * Check to make sure the chain hasn't already been linked.
     */
    tw = where->link_point->accept;
    while (tw) {
                if (tw == commands) {
                        /*
                         * Parse chain already linked in, but it's not really
                         * a failure because the parse chain has been linked
                         * in, so we return TRUE.
                         */
                        return(TRUE);
                }
                if (tw->alternate == (dynamic_transition *)&pname(link_trans)) {
                        break;
                }
                tw = tw->alternate;
    }
    if (!tw || (tw->alternate != (dynamic_transition *)&pname(link_trans))) {
                printf("Parser No Link at %d\n", __LINE__);
                return(FALSE);
    }

    if (commands) {
                /*
                 * At this point last_cmd's alternate points to link_point,
                 * so we have the right place
                 */
                commands->alternate = (dynamic_transition *)tw->alternate;
#ifdef  PARSER_DEBUG_LINKS
                commands->args = module;
#endif /* PARSER_DEBUG_LINKS */
                tw->alternate = commands;
                return(TRUE);
    } else {
                printf("Parser No Link at %s:%d\n", __FILE__, __LINE__);
                return(FALSE);
    }
}

static boolean parser_resolve_exit_link_point (chain_link *where,
                                                                                           dynamic_transition *commands)
{
    if (!commands ||
                !where ||
                !commands->accept ||
                (commands->accept->alternate !=
                 (dynamic_transition *)&pname(link_trans))) {

                /*
                 * Transition is broken or already linked
                 */
                return(FALSE);
    }

    commands->accept->alternate = where->link_point;
    return(TRUE);
}

/*
 * parser_add_command_list - Add a series of parse chains
 */

boolean parser_add_command_list_func (parser_extension_request *chain
#ifdef  PARSER_DEBUG_LINKS
                                                                          , const char *module
#endif /* PARSER_DEBUG_LINKS */
                                                                          )
{
    int i;
        boolean status = TRUE;

        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }

    for (i = 0; chain[i].type != PARSE_LIST_END; i++) {
                if (!parser_add_commands(chain[i].type, chain[i].commands, module)) {
                        status = FALSE;
                        break;
                }
    }

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }
    return(status);
}


#define ADDR_GROW        3
addr_func *addr_funcs = NULL;
static uint addr_funcs_length = 0;

static void parser_add_address_set(addr_func *addr, const uint addr_type,
                                                                   short_help_func get_short_help,
                                                                   match_gen_func func);

/* NO need to protect this with semaphore since it is assumed that 
 * tParser by itself will be Single Threaded...
 */
void parser_add_address (const uint addr_type, short_help_func get_short_help,
                                                 match_gen_func func)
{
    int i;
    addr_func *tmp;

    if (addr_funcs == NULL) {
      if ((addr_funcs = (addr_func *)PARSERmalloc(ADDR_GROW*sizeof(addr_func))) == NULL) {
                        printf(nomemory);
                        return;
                }
                addr_funcs_length = ADDR_GROW;
                parser_add_address_set(&addr_funcs[0], ADDR_ILLEGAL, NULL, NULL);
    }

    i = 0;
    while ((addr_funcs[i].addr_type != ADDR_ILLEGAL) &&
                   (i < addr_funcs_length)) {
                i++;
    }

    if (addr_funcs[i].addr_type == ADDR_ILLEGAL) {
                if ((i+1) >= addr_funcs_length) {
                        if ((tmp = PARSERmalloc((ADDR_GROW + addr_funcs_length) *
                                                                        sizeof(addr_func))) == NULL) {
                                printf(nomemory);
                                return;
                        }
                        bcopy((char *)addr_funcs, (char *)tmp, 
                                  addr_funcs_length * sizeof(addr_func));
                        PARSERfree(addr_funcs);
                        addr_funcs = tmp;
                        addr_funcs_length += ADDR_GROW;
                }
                parser_add_address_set(&addr_funcs[i], addr_type,
                                                           get_short_help, func);
                parser_add_address_set(&addr_funcs[i+1], ADDR_ILLEGAL, NULL, NULL);
    } else {
                /* something's wrong */
                printf("\n%% Invalid end to address list, can't add protocol %d",
                           addr_type);
    }
}

static void parser_add_address_set (addr_func *addr, const uint addr_type,
                                                                        short_help_func get_short_help,
                                                                        match_gen_func func)
{
    addr->addr_type = addr_type;
    addr->get_short_help = get_short_help;
    addr->func = func;
}



static void resolve_parse_chains (void)
{
        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }

        { 
        chain_list *cp = cl;
        chain_list *tmp;
        chain_list *old = NULL;

        while (cp != NULL) {

                        if (!parser_add_chains(cp->cp.id, cp->cp.link_point,

#ifdef  PARSER_DEBUG_LINKS
                                                                   cp->cp.name,
#endif /* PARSER_DEBUG_LINKS */
                                                                   TRUE)) {
                        /*
                                 * Failed again, leave on list and try later
                                 */
                        old = cp;
                        cp = cp->next;
                        } else {
                                /*
                                 * Success, remove and free link
                                 */
                        if (cp == cl) {
                                        cl = cl->next;
                        }
                        tmp = cp;
                        cp = cp->next;
                        if (old) {
                                        old->next = cp;
                        }
                        PARSERfree(tmp);
                        }
        }

        }

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }

}





static boolean add_to_unresolved_chains (uint which_chain,
                                                                                 dynamic_transition *commands
#ifdef  PARSER_DEBUG_LINKS
                                                                                 , const char *module
#endif /* PARSER_DEBUG_LINKS */
                                                                                 )
{
    chain_list *cp;
        boolean status = TRUE;

        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }
    /*
     * Make sure commands isn't alreay on the list
     */
    cp = cl;
    while (cp != NULL) {
                if ((cp->cp.link_point == commands) &&
                        (cp->cp.id == which_chain)) {
                        /*
                         * It's a duplicate, but we return true because
                         * the link point has been successfully added.
                         */
                        goto SEMRELEASE;
                }
                cp = cp->next;
    }

    if ((cp = (chain_list *)PARSERmalloc(sizeof(chain_list))) == NULL) {
                printf(nomemory);
                status = FALSE;
    } else {

        cp->cp.id = which_chain;
#ifdef  PARSER_DEBUG_LINKS
        cp->cp.name = module;
#endif /* PARSER_DEBUG_LINKS */
        cp->cp.link_point = commands;
        cp->next = cl;
        cl = cp;
        }

 SEMRELEASE:

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }
    return(status);
}

boolean parser_add_link_point_func (uint which_chain,
#ifdef  PARSER_DEBUG_LINKS
                                                                        const char *module,
#endif /* PARSER_DEBUG_LINKS */
                                                                        transition *lp)
{
    dynamic_transition *dp = (dynamic_transition *)lp;

    return(parser_add_link_to_list(which_chain,
#ifdef  PARSER_DEBUG_LINKS
                                                                   module,
#endif /* PARSER_DEBUG_LINKS */
                                                                   dp, PARSER_ENTRY_LINK_POINT));
}

boolean parser_add_link_exit_func (uint which_chain,
#ifdef  PARSER_DEBUG_LINKS
                                                                   const char *module,
#endif /* PARSER_DEBUG_LINKS */
                                                                   transition *lp)
{
    dynamic_transition *dp = (dynamic_transition *)lp;

    return(parser_add_link_to_list(which_chain,
#ifdef  PARSER_DEBUG_LINKS
                                                                   module,
#endif /* PARSER_DEBUG_LINKS */
                                                                   dp, PARSER_EXIT_LINK_POINT));
}

static boolean parser_add_link_to_list (uint which_chain,
#ifdef  PARSER_DEBUG_LINKS
                                                                                const char *module,
#endif /* PARSER_DEBUG_LINKS */
                                                                                dynamic_transition *lp, uint type)
{
    chain_list *new_lp;
        boolean status = TRUE;

        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }

    if ((new_lp = (chain_list *)PARSERmalloc(sizeof(chain_list))) == NULL) {
                printf(nomemory);
                status = FALSE;
                goto SEMRELEASE;
    }

    new_lp->cp.id = which_chain;
#ifdef  PARSER_DEBUG_LINKS
    new_lp->cp.name = module;
#endif /* PARSER_DEBUG_LINKS */
    new_lp->cp.link_point = lp;
    new_lp->cp.type = type;
    new_lp->next = llist;

    llist = new_lp;

 SEMRELEASE:

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }

    return(status);
}




/*****************************************************************
 *
 *   Func: parser_exec_callback
 *
 *   Desc: Task entry point for Callback tasks
 *
 *   Inputs:  None
 *
 *   Result:  None
 *
 *   Algorithm:
 *             Parser identifies callback function for a command
 *             Sends the callback function pointer information to 
 *             the callback task, so that it can process other 
 *             commands from other I/O sources. This function waits
 *             for intimation from parser and executes the callback
 *             function synchronously
 *
 *****************************************************************/
void parser_exec_callback( void ) 
{
        tINT          outFd, inFd, errFd;
        tUOCTET4      moduleId = 1;  
        tOCTET2       rc = RC_OK;
        tpCOMSIM      pComSim = NULL;  
        tpCALLBACKMSG pCallback = NULL ;
        tpPARSERSIM   pSim = NULL;

        parseinfo     *csb = NULL;
        char          *buffer = NULL;  
        parser_mode   *prevMode = NULL ;
        boolean           bErrorFlag = FALSE;
        char             *pCwd = NULL;
        
        addSignalHandlers();

        /* Get the original FDs */
        inFd  = ioTaskStdGet( 0, STD_IN );
        outFd  = ioTaskStdGet( 0, STD_OUT );
        errFd  = ioTaskStdGet( 0, STD_ERR );


        FOREVER { 

                pCallback = NULL;
                pSim =  NULL;
                csb =  NULL;
                bErrorFlag = FALSE;

                /* Reset the errno of the callback task */
                errnoOfTaskSet( 0, OK );

                /* Reset the Signal Flags */
                /*
                 * RAMS: commented this for now
                 
                 setTaskSignalMask(0,0); */

                /* Wait till a command needs to be executed as 
                 * determined by the tParser 
                 */
                if ( msgQReceive( callback_Q_Id, (char *)&pCallback, 
                                  sizeof( tCALLBACKMSG), WAIT_FOREVER )
                     == ERROR ) {
                  printf("tCallback:Cannot receive from Message Q\n");
                        continue;
                }
        
                /* Something wrong if we get NULL callback message */
                if ( pCallback == NULL ) {
                        printf("tCallback:NULL Message obtained from Message Queue\n");
                        continue;
                }

                /* Get the CSB & SIM associated with the callback message */
                csb = pCallback->csb;
                pSim = pCallback->pSim;
                pComSim = &(pSim->comsim);

                if ( !csb || !pSim ) {
                        printf("NULL Pointer obtained for CSB / SIM \n");
                        bErrorFlag = TRUE;
                        goto cleanup;
                }

                /*
                 * If this is an interactive command,
                 * the command callback is responsible
                 * for doing any special tty processing
                 * that is needed
                 */
                if ( csb->flags & TERMINAL_MODE ) {
                        /*
                         * Reset the terminal characteristics to 'TERMINAL' so that
                 * the callback functions need not do echoing etc 
                 */
                        if ( !( csb->flags & CONFIG_CGI ) ) {
                                ioSetTerminalMode( csb->inFd, IO_LINE_MODE );
                        } 

                        /*
                         * Disable tMore processing.  We just
                         * redirect the output back to the
                         * original fd and let the parser_input
                         * layer cleanup the tMore task.
                         */
                        if (csb->savedFd >= 0) {
                                csb->outFd = csb->savedFd;
                        }
                }
                /*
                 * RAMS: commented this for now
                 
                 taskSetTtyAttr(csb->inFd, SECONDARY_TASK, TRUE); */

                /* Redirect the I/O based on the I/O Source */
                ioTaskStdSet(0, STD_IN, csb->inFd );
                ioTaskStdSet(0, STD_OUT, csb->outFd );
                ioTaskStdSet(0, STD_ERR, csb->outFd );

#if 0
                /* RAMS: no access control check now */
                /* Do not do access control check if the system is loading
         * since during intialization all tasks execute in 'root'
         * user context 
         */
                if ( !systemloading && ( setUid( csb->uid ) == FALSE ) ) {
                        printf("Invalid UID obtained %d\n", (int)csb->uid);
                        csb->uid = INVALID_UID;
                        bErrorFlag = TRUE;
                        goto cleanup;
                }
#endif

                moduleId  = pComSim->origid;
                rc = RC_OK;

                /* Make note of previous mode we are in */
                prevMode  = csb->mode;

                /* pIndex is a pointer to the previous cookie for 'more' */
                csb->pIndex = pSim->index;
                csb->maxRecordCount = pSim->maxCount;
                /*
                  RAMS : commented this for now
                if (( pCwd = getTaskCWD_k(csb->ioTaskId, &pFreeFunc)) == NULL) {
                        printf("Cannot get Current Working Directory\n");
                } else { 
                        setCWD( pCwd );
                        pFreeFunc( pCwd );
                }
                */

                /* Call the 'callback function' */
                csb->action_func(csb);
        
                if (csb->mode == exec_mode) {
                        /* Clear all flags except some fixed ones */
                        csb->flags &=  ( CONFIG_TERM | KEYWORD_MATCHED | CONFIG_PER_USER 
                                                         | CONFIG_CGI | COMMAND_COMPLETE );
                }

                /* See if we had something from callback, via 'write_buffer' */
        /* buffer = read_buffer();*/
                fflush(stderr);
                fflush(stdout);
        
                /* Reset the CSB Line */
                if (csb->in_help == PARSER_NO_HELP) {
                *(csb->line) = '\0';
                        /* Indicate to I/O Task that the command was complete 
                     * for history mechanism 
                         */
                        csb->flags |= COMMAND_COMPLETE;
                }
        
        cleanup:

                /* Send response back to the I/O Source ( i.e. proxy tParser )
         * indicating the command execution is complete 
         */
                if ( ( rc = parser_send_response( pComSim, buffer, csb->MoreStatus ) ) 
                        != RC_OK ) {
                        printf("tCallback:Error occured while Posting in tParser\n");
        }
                

                /* Do cleanup */
                if ( pSim && pComSim ) 
                        mFREESIM( pComSim );

                PARSERfree( pCallback );
        
                /* The I/O Task will be waiting on configuration semaphore
         * for configuration commands; If the callback function 
         * asked us to do the job ( via csb->giveConfigSem ), lets
         * give the semaphore.
         * Also, release the configuration semaphore, during error
         * conditions.
         */
        if ( csb->giveConfigSem || bErrorFlag) { 
                        if ( prevMode == config_mode ||  
                             csb->mode == config_mode ||
                             prevMode == interface_mode ||
                             csb->mode == interface_mode) {
                                if ( PSsemGive( csb->configSemId ) == ERROR )
                                        PSpanic("Cannot Give configuration semaphore\n");
                        }
                }

                /* Reset the I/O redirection */
                ioTaskStdSet(0, STD_IN, inFd);
                ioTaskStdSet(0, STD_OUT, outFd); 
                ioTaskStdSet(0, STD_ERR, errFd);
                /*
                 * RAMS: commented this for now
                 
                 taskSetTtyAttr(csb->inFd, SECONDARY_TASK, FALSE); */
        }

        return;
}

/*****************************************************************
 *
 *   Func: parser_send_response
 *
 *   Desc: Send a SIM to I/O Source indicating command is complete
 *
 *   Inputs:
 *         pSim     : Input SIM from parser
 *         buffer   : Output buffer, if any. NULL, if nothing
 *                    The I/O source will free this buffer
 *         status   : Passed as SIM Status to I/O Source
 *
 *   Result:  RC_OK  : If SIM Posted to I/O Source
 *            RC_ERR : Otherwise
 *
 *   Algorithm:
 *
 *****************************************************************/
tOCTET2 parser_send_response( tpCOMSIM pSim, char *buffer, tUOCTET4 status )
{
        tpCOMSIM pComSim = NULL;
        tOCTET2 rc = RC_OK;

        pComSim = NULL;
    pComSim = mGETSIM( sizeof( tCOMSIM ) );

    if (pComSim == NULL)
        return (RC_ERR);

    mFILL_COM_SIM( pComSim, PARSER_MODULE, pSim->origid,\
                                  IO_CMD_COMPLETE_IND, pSim->requestid, buffer, 0, status );
        
    mPOST_SIM_NIND( pComSim, rc );
        return ( rc );
}




static void addSignalHandlers ( void )
{ 
  /*
   *RAMS: Commented this for now
   
        signal(SIGINT, defaultSignalHandler);
        signal(SIGKILL, defaultSignalHandler); */
}


#ifdef IOS_PARSER 
/*
 * Recurse through instead of using a stack
 */
void recurse_node (parseinfo *csb, transition *node)
{
    queuetype *t1 = csb->tokenQ;
    queuetype t2;

    queue_init(&t2, 0);
    csb->tokenQ = &t2;
    push_node(csb, node);
    parse_token(csb);
    csb->tokenQ = t1;
}

/*
 * parse_checklevels
 * check previous parser levels to see if there is a command match
 * "up there" somewhere.  This used to be part of parse_cmd(), but
 * I moved it out to save stack space when it's not used.
 */

static void parse_checklevels (parseinfo *csb, int save_line_index)
{
#define MAX_MODE_STACK  5
    typedef struct parser_mode_stack_ {
        parser_mode *mode;
        void *var;
        boolean leave_subcommand;
        uint error_index;
    } parser_mode_stack;

    /*
     * If the parse at the secondary level fails, we save the secondary state
     * and reset the system for parsing the at the primary level.  If the
     * primary parse also fails, we report the error for the best match.
     * The state is set for the best match (i.e. longest error_index).
     * This prevents an error in an interface command from allowing
     * the broken command's subcommands to mistakenly operate on
     * a previous interface.  Similarly for other commands which have
     * subcommands.
     *
     * We save (and clear) the IDB pointer so that we can get out of
     * subinterface state.  Ditto the CDB pointer.
     */
    
    parser_mode_stack mstack[MAX_MODE_STACK];
    parser_mode *alt_mode;
    int i = 0;
    
    alt_mode = get_alt_mode(csb->mode);
    while (alt_mode) {
        
                if (i < MAX_MODE_STACK) {
                        mstack[i].mode = csb->mode;
                        mstack[i].var = mode_save_vars(csb);
                        mstack[i].leave_subcommand = csb->leave_subcommand;
                        mstack[i].error_index = csb->error_index;
                        i++;
                }
        
                csb->mode = alt_mode;
                set_multiple_funcs(csb, 0);
                csb->error_index = 0;
                csb->line_index = save_line_index;
                csb->sense = TRUE;
                csb->set_to_default = FALSE;
                /*              zero_csb_strings(csb); */
                csb->leave_subcommand = FALSE;
        
                push_node(csb, get_mode_top(csb->mode));
                parse_token(csb);
        
                if (test_multiple_funcs(csb, '=', 0) || (csb->next == NULL)) {
                        alt_mode = get_alt_mode(csb->mode);
                } else {
                        alt_mode = NULL;
                }
    }
    
    if (test_multiple_funcs(csb, '=', 0) || (csb->next == NULL)) {
                while ((i > 0) && (!csb->leave_subcommand) ) {
                        i--;
                        csb->error_index = mstack[i].error_index;
                        csb->leave_subcommand = mstack[i].leave_subcommand;
                        csb->mode = mstack[i].mode;
                        mode_reset_vars(csb, mstack[i].var);
                }
    }
}

void show_parser_unresolved (parseinfo *csb)
{
    chain_list *cp = cl;

    printf("\nUnresolved parse chains:");
    while (cp != NULL) {
                printf("\n  %-3d", cp->cp.id);
#ifdef  PARSER_DEBUG_LINKS
                printf(" %s", cp->cp.name);
#endif /* PARSER_DEBUG_LINKS */
                cp = cp->next;
    }
}

#ifdef  PARSER_DEBUG_LINKS
void show_parser_links (parseinfo *csb)
{
    chain_link *cp;
    dynamic_transition *tp;
    int i = 0;
    chain_list *lp;
    static const char *show_links_header =
                "\n  Name                             ID   Addr      Type";

        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }

    if (!GETOBJ(string,1)[0]) {
                printf("\nCurrent parser link points:");
                printf(show_links_header);

                cp = &chain_gang[i++];
                while (cp && cp->name != NULL) {
                        printf("\n  %30s %-4d   0x%6x %-2d",
                                   cp->name, cp->id, cp->link_point, cp->type);
                        cp = &chain_gang[i++];
                }
                printf("\nAdded at run time:");
                printf(show_links_header);
                lp = llist;
                while (lp != NULL) {
                        printf("\n  %30s %-4d   0x%6x %-2d",
                                   lp->cp.name, lp->cp.id, lp->cp.link_point, lp->cp.type);
                        lp = lp->next;
                }
                printf("\n");
                goto SEMRELEASE;
    }       

    cp = get_link_point_by_name(GETOBJ(string,1));
    if (!cp) {
                printf("\n%% Unknown parser link point '%s'", GETOBJ(string,1));
                goto SEMRELEASE;
    }

    if (!cp->link_point ||
                !cp->link_point->accept ||
                cp->link_point->accept == (dynamic_transition *)&pname(link_trans)) {
                printf("\nNo links for link point '%s'", GETOBJ(string,1));
                goto SEMRELEASE;
    }

    printf("\nCurrent links for link point '%s':", GETOBJ(string,1));
    tp = cp->link_point->accept;
    while (tp && (tp != (dynamic_transition *)&pname(link_trans))) {
                if (tp->args) {
                        printf("\n  %s", tp->args);
                }
                tp = tp->alternate;
    }
    printf("\n");


 SEMRELEASE:

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }
}

#ifdef  PARSER_DEBUG_LINKS
/*
 * Get link point by name
 */
static chain_link *get_link_point_by_name (const char *name)
{
    int i = 0;
    chain_link *cp;
    chain_list *lp;

    /* Check static link points */
    cp = &chain_gang[i++];
    while (cp && cp->name != NULL) {
                if (strcmp(cp->name, name) == 0) {
                        return(cp);
                }
                cp = &chain_gang[i++];
    }
    /* Check dynamic link points */
    lp = llist;
    while (lp != NULL) {
                if (strcmp(lp->cp.name, name) == 0) {
                        return(&lp->cp);
                }
                lp = lp->next;
    }
    /* Didn't find anything */
    return(NULL);
}
#endif  /* PARSER_DEBUG_LINKS */

/*
 * parser_link_point_exists
 * Return TRUE if someone has added the named entry link point.
 */

boolean parser_link_point_exists (uint which_chain)
{
    chain_list *lp;
        boolean status = FALSE;

        if ( PSsemTake(parse_chain_sem, WAIT_FOREVER) == ERROR ) { 
                PSpanic("Cannot take parse chain semaphore\n");
        }

    for (lp = llist; lp; lp = lp->next) {
                if ((which_chain == lp->cp.id) &&
                        (PARSER_ENTRY_LINK_POINT == lp->cp.type))
                        status = (TRUE);
                break;
    }

        if ( PSsemGive(parse_chain_sem) == ERROR ) { 
                PSpanic("Cannot give parse chain semaphore\n");
        }
    return(status);
}
    
#endif  /* PARSER_DEBUG_LINKS */


/*
 * exit any configuration mode session block might be in.
 */
void exit_config_modes (parseinfo *csb)
{
    if ((csb->mode == config_mode) || test_mode_config_subcommand(csb)) {
        exit_config_command(csb);
    }
}


#endif /* IOS_PARSER */


#define BEL "\007"

/*
 * word_completion
 *
 * Complete a keyword by comparing the current input line and the
 * contents of the help buffer.  If there is only one match in the help
 * buffer, then verify that the keyword and the match are the same.
 */
static boolean word_completion (parseinfo *csb)
{
    char *cp, *sp;
    char *hp, *jp, *ojp, *kp;
    int help_matches = 0;

        cp = sp = hp = jp = ojp = kp = NULL;

    if ((help_matches = count_help_matches(csb)) == 0) {
                /* No matches, seeya */
                return(FALSE);
    }

    /* Find the start of the last word in the input */
    for (cp = csb->line; *cp; cp++)
                ;                                                               /* Null body */ 

    while (cp != csb->line && *(cp-1) != ' ') {
                cp--;
    }

    /* Find first help match */
    for (hp = csb->help_save.buf;
                 hp && *hp && *hp != '\t' && *hp != '\n';
                 hp++) {
                ;                                                               /* Null body */
    }
    /* See if we can find anything shorter */
    jp = hp;
    while (jp && *jp) {
                /* move past white space to next help */
                for (; jp && (*jp == '\t' || *jp == '\n'); jp++) {
                        ;                                                       /* Null body */
                }
                if (!jp || !*jp) {
                        /* end of help */
                        continue;
                }
                /* save pointer to beginning of current help */
                ojp = jp;
                /* find length of match */
                for (kp = csb->help_save.buf;
                         jp && *jp && *jp != '\t' && *jp != '\n' &&
                         (tolower(*kp) == tolower(*jp));
                         jp++, kp++) {
                        ;                                                       /* Null body */
                }
                /* See if matched length is shorter */
                if ((hp - csb->help_save.buf) > (jp - ojp)) {
                        hp = csb->help_save.buf + (jp - ojp);
                }
                /* Move to next white space */
                for (; jp && *jp && *jp != '\t' && *jp != '\n'; jp++) {
                        ;                                                       /* Null body */
                }
    }

    /* move to end of last word in the input */
    for (sp = csb->help_save.buf;
                 *cp && *sp && (sp != hp) && (*sp != '\t') && (*sp != '\n');
                 cp++,sp++) {

                /* Check that the input and the matched word are the same. */
                if (tolower(*cp) != tolower(*sp)) {
                        printf("word_completion:Input / Output Did not Match\n");
                        return(FALSE);
                }
    }
    /* We may have been attempting a match at the end of a line.
     * In this case, we could copy an invalid help string onto the
     * end of the current line (e.g. adding "<cr>" to the line.)
     *
     * This proccedure of using the help mechanism is totally wrong.
     * Instead, a separate flag should be used for command completion
     * and only the keyword action routine should add text to the help
     *  buffer.  The other action routines should not add anything to
     * the help buffer in this case.
     *
     * Until the above gets done, do not add help text which begins with
     * a blank.
     */
    if (*sp == ' ') {
                return(FALSE);
    }

    while (*sp && (sp != hp) && (*sp != '\t') && (*sp != '\n')) {
                /* copy the rest of the matched word into the input buffer */
                *cp++ = *sp++;
    }
    if (help_matches != 1) {
                *cp = '\0';
                return(FALSE);
    }
    *cp++ = ' ';                                                /* Add trailing space */
    *cp = '\0';
    return(TRUE);
}

void push_node (parseinfo *csb, transition *node)
{
    parsenode *pp;

    if (node->func == (trans_func) NONE_action) {
                /*
                 * Nothing to do for NONE transitions, so return
                 */
                return;
    }

    /* If we fail to obtain memory, we will not push this node onto
     * the queue and this transition will never happen.  We cannot
     * propagate a return code all the way back up the stack and even
     * if we did, we'd probably have to just prune this portion of the
     * parse tree and try to keep going.  Doing it here seems the best
     * overall solution.
     */
    if ((pp = get_parsenode(csb)) == NULL) {
                return;
    }
    pp->node = node;
    pp->line_index = csb->line_index;
    if (csb->nvgen || (csb->flags & COMMAND_AUTHORIZE)) {
                strcpy(pp->nv_command, csb->nv_command);
    }
    pp->pmp = csb->pmp;
    strcpy(pp->visible_ambig, csb->visible_ambig.ambig);
    strcpy(pp->hidden_ambig, csb->hidden_ambig.ambig);
    pp->command_visible = csb->command_visible;
    pp->unsupported = csb->unsupported;
    pp->sense = csb->sense;
    pp->set_to_default = csb->set_to_default;
    requeue(csb->tokenQ, pp);
}

/*
 * Parse tokens from the top of the token stack.  We check our
 * runtime against the saved runtime and allow other processes
 * access to the CPU every 250ms.  This prevents the starvation
 * of other processes during NVGEN and other CPU intensive operations.
 *
 * While there are tokens on the stack, we pop one off and call the
 * token specific processing routine.  The token specific processing
 * routine will make sure the top of the stack contains the next
 * node that should be processed (either by adding one or more nodes to the
 * stack, or not adding anything to the stack.
 */
void parse_token (parseinfo *csb)
{
    parsenode *tos;                                             /* Top of the stack */

    while (! QUEUEEMPTY(csb->tokenQ)) {

                tos = (parsenode *)dequeue(csb->tokenQ);
		
                if (tos) {
                        if (tos->node) {
                                csb->line_index = tos->line_index;
                                csb->pmp = tos->pmp;
                                strcpy(csb->visible_ambig.ambig, tos->visible_ambig);
                                strcpy(csb->hidden_ambig.ambig, tos->hidden_ambig);
                                csb->command_visible = tos->command_visible;
                                csb->unsupported = tos->unsupported;
                                csb->sense = tos->sense;
                                csb->set_to_default = tos->set_to_default;
                                if (csb->nvgen || (csb->flags & COMMAND_AUTHORIZE)) {
                                        strcpy(csb->nv_command, tos->nv_command);
				}
                                if (tos->node->func) {
				  tos->node->func(csb, tos->node, tos->node->args);
                                }
                        }
                        tos->next = NULL;
                        PARSERfree(tos);
                }
    }
}

/*
 * Parse a command line.
 */
int parse_cmd (parseinfo *csb, tpPARSERSIM pSim)
{
  int error = PARSE_ERROR_NOERR;                /* 0=ERROR_NOERR; ERROR_AMBIG;
                                                 * ERROR_NOMATCH; ERROR_UNKNOWN;
                                                 * ERROR_ALIAS;
                                                 */
  int save_line_index;
  static boolean first_time_through = TRUE;
  boolean alias_was_expanded = FALSE;
  boolean execCallback = FALSE;
  int inFd, outFd, errFd;
  tpCALLBACKMSG pCallback=NULL;
  tpCOMSIM pComSim = NULL;


  inFd  = ioTaskStdGet( 0, STD_IN );
  outFd  = ioTaskStdGet( 0, STD_OUT );
  errFd  = ioTaskStdGet( 0, STD_ERR );

  ioTaskStdSet(0, STD_IN, csb->inFd );
  ioTaskStdSet(0, STD_OUT, csb->outFd );
  ioTaskStdSet(0, STD_ERR, csb->outFd );

  csb->MoreStatus = RC_OK;
  csb->giveConfigSem = TRUE;


  if (first_time_through) {
    resolve_parse_chains();
    parser_modes_init();
    first_time_through = FALSE;
  }
  
  parse_cmd_init(csb);
  
  /*
   * Check for empty lines.
   * Strip leading whitespace.
   * Save comments if in config mode.
   */
  for (csb->line_index = 0; csb->line[csb->line_index] == ' ' ||
         csb->line[csb->line_index] == '\t'; csb->line_index++)
    ;                                                           /* Null body */
  
  /* Ignore empty lines which aren't help requests */
  if ((csb->line[csb->line_index] == '\0') &&
      !csb->in_help &&
      !(csb->flags & PARSE_EMPTY_STRING)) {
    *csb->line = '\0';
    error = PARSE_ERROR_NOERR;
    goto SEMRELEASE;
    
  }
  if ((csb->line[csb->line_index] == '!' ||
       csb->line[csb->line_index] == '#' ||
       csb->line[csb->line_index] == ';')) {
    /* XXX Need a better algorithm for saving comments. */
    save_line(csb, &csb->comment_save, "%s\n", csb->line);
    *csb->line = '\0';
    /* return(PARSE_ERROR_NOERR);*/     /* comments are always successful */
    error = PARSE_ERROR_NOERR;
    goto SEMRELEASE;
  }
  
  /* We do not currently support aliases. Need to 
   * include parser_alias.c in Makefile to make this
   * work
   if (expand_alias(csb)) {
   alias_was_expanded = TRUE;
   }
  */

  save_line_index = csb->line_index;
  
  push_node(csb, get_mode_top(csb->mode));
  parse_token(csb);
  
  /* If setting privilege levels on commands, csb->priv_set will
   * be non-zero, and csb->multiple_funcs will be > 1.  This is
   * not an error.
   */
  if (csb->priv_set) {
    error = PARSE_ERROR_NOERR;
    goto SEMRELEASE;
  }
  
  /* Doing help processing.
   * If there was an ambiguous command match,
   * print the ambiguous message, otherwise print the help message.
   * If no help was provided, then tell the user that the command
   * was unrecognized.
   */
  if (csb->in_help) {
    /*
     *  If the number of visible matches is > 1, or
     *  the number of visible matches == 0 and the
     *  number of invisible matches > 1, then the
     *  command is ambiguous
     */
    if (test_visible_multiple_funcs(csb, '>', 1) ||
        (test_visible_multiple_funcs(csb, '=', 0) &&
         test_invisible_multiple_funcs(csb, '>', 1))) {
      DBG_PARSER("\n Ambiguous command");
      error = PARSE_ERROR_AMBIG;
    } else {
      
      /* There are two choices here:
       * 1. The user asked for help.
       * 2. The user requested keyword completion.
       */
      if (csb->lastchar == '?') {
        if (csb->help_save.used > 0) {
          
          csb->action_func = parser_help;
          /*
           * Have a callback task run the command.
           */
          pCallback = ( tpCALLBACKMSG)
            PARSERmalloc(sizeof(tCALLBACKMSG));
          pCallback->csb = csb;
          pCallback->pSim = pSim;
          if ((csb->mode != config_mode))  /* ||
                                              (csb->mode != interface_mode)) */
            {
                      csb->giveConfigSem = FALSE;
            }
          if ( msgQSend(callback_Q_Id, ( char *)&pCallback,
                        sizeof(tpCALLBACKMSG), 
                        WAIT_FOREVER, MSG_PRI_NORMAL) == ERROR )
            {
              printf("Cannot send message to "
                     "callback queue\n");
              error = PARSE_ERROR_NOERR;
            }
          execCallback = TRUE;
          
        } else {
          error = PARSE_ERROR_UNKNOWN;
        }
      } else {                                  /* csb->lastchar == '\t' */
        if ((word_completion(csb) == FALSE)) {
          printf("\007");
        }
      }
    }
  } else {
    
    /* If not doing help processing, then check whether the
     * parse uniquely identified a command.  If not, report
     * the error (if no command was matched) or report that
     * the command was ambiguous (if more than one command matched).
     */
    
    /* No commands matched.  Report the error */
    if (test_multiple_funcs(csb, '=', 0) || (csb->next == NULL)) {
      if (alias_was_expanded) {
        error = PARSE_ERROR_NOMATCH | PARSE_ERROR_ALIAS;
      } else {
	/* DBG_PARSER("\nNO MATCH OF COMMANDS"); */
        error = PARSE_ERROR_NOMATCH;
      }
    }
    
    /*
     *  If the number of visible matches is > 1, or
     *  the number of visible matches == 0 and the
     *  number of invisible matches > 1, then the
     *  command is ambiguous.
     *
     * Multiple functions were identified.  This was an
     * ambiguous command.  Report the error.
     */
    
    if (test_visible_multiple_funcs(csb, '>', 1)) {
      error = PARSE_ERROR_AMBIG;
    } else {
      if ((test_visible_multiple_funcs(csb, '=', 0) &&
           test_invisible_multiple_funcs(csb, '>', 1))) {
        if ((csb->line[csb->error_index] == '\0') &&
            (csb->error_index > 0) && !csb->next) {
          error = PARSE_ERROR_NOMATCH;
        } else {
          error = PARSE_ERROR_AMBIG;
        }
      }
    }
    /* If not doing help processing and one function selected, 
     * then we've successfully parsed a command.  Restore its
     * state and call the command function.
     */
    if (test_visible_multiple_funcs(csb, '=', 1) ||
        (test_visible_multiple_funcs(csb, '=', 0) &&
         test_invisible_multiple_funcs(csb, '=', 1))) {
      boolean do_hidden_funcs = TRUE;
      
      /*
       * If test_visible_multiple_funcs == 1, then
       * don't execute hidden functions
       */

      if (test_visible_multiple_funcs(csb, '=', 1)) {
        do_hidden_funcs = FALSE;
      }
      
      while (csb->next != NULL) {
        parseinfo *thiscsb;
        
                                /* Don't free the csb itself.  Must copy its contents
                                 * since csb was passed to us and the caller will need to
                                 * continue to use this particular structure.
                                 */
        thiscsb = csb->next;
        *csb = *csb->next;              /* Struct copy */
                                /* chunk_free(parseinfo_chunks, thiscsb); */
        PARSERfree( thiscsb );
        thiscsb = NULL;
        
        if (csb->command_visible ||
            (!csb->command_visible && do_hidden_funcs)) {
          
          if (csb->flags & COMMAND_FUNCTION) {
            if (csb->command_function) {
              csb->command_function(csb);
            }
          } else {
            if (csb->action_func)  {
              
              /*
               * Have a callback task run the command.
               */
              pCallback = ( tpCALLBACKMSG)
                PARSERmalloc(sizeof(tCALLBACKMSG));
              pCallback->csb = csb;
              pCallback->pSim = pSim;
              csb->giveConfigSem = FALSE;
              if ( msgQSend(callback_Q_Id, ( char *)&pCallback,
                            sizeof(tpCALLBACKMSG), 
                            WAIT_FOREVER, MSG_PRI_NORMAL)
                   == ERROR ) {
                printf("Cannot send message to "
                       "callback queue\n");
                error = PARSE_ERROR_NOERR;
              }
              execCallback = TRUE;
            }
          }
        }
      }
    }
    
  }
  
  /* Free any lingering saved parseinfo structures, if needed. */
  {
    parseinfo *thiscsb = csb->next;
    while (thiscsb != NULL) {
      parseinfo *nextpd = thiscsb->next;
      /* chunk_free(parseinfo_chunks, thiscsb); */
      PARSERfree( thiscsb );
      thiscsb = nextpd;
    }
    csb->next = NULL;
  }
  
  /* Reset prompt_lead if needed.  This has to be done here because
   * a primary config command may cause the parser to switch from
   * a subcommand set.  Only here do we know that this switch has
   * occurred and only by examining the top of the active parse tree.
   * This also has to be done for handling configuration from
   * memory or net so that we aren't left with the prompt
   * indicating the wrong command level.
   */
  if ( ( execCallback == FALSE ) && (csb->mode == exec_mode)) {
    /* Clear all flags except some fixed ones */
    csb->flags &=  ( CONFIG_TERM | KEYWORD_MATCHED | CONFIG_PER_USER 
                     | CONFIG_CGI | COMMAND_COMPLETE );
  }
  
  /* The only errors that we can report are parse errors.  The commands 
   * are expected to report their internal errors.
   */
  
 SEMRELEASE:
  
  if ( execCallback == FALSE ) { 
    
    /* Indicate to I/O Task that the command was complete 
     * for history mechanism 
     */
    if ( csb->in_help == PARSER_NO_HELP ) 
      csb->flags |= COMMAND_COMPLETE;
    
    pComSim = &(pSim->comsim);
    
    if ( error != PARSE_ERROR_NOERR && !(csb->flags & CONFIG_HTTP ) ) {
      report_error(csb,error , FALSE);
    }
    if (csb->in_help == PARSER_NO_HELP) {
      *(csb->line) = '\0';
    }
    parser_send_response(pComSim, NULL, csb->MoreStatus );
    mFREESIM(pComSim);
    
    if ( csb->giveConfigSem && 
         ((csb->mode == config_mode) || (csb->mode == interface_mode))) 
      {
        if ( PSsemGive( csb->configSemId ) == ERROR ) 
          {
            PSpanic("Cannot Give configuration semaphore\n");
          } 
      }
    
  }
  
  ioTaskStdSet(0, STD_IN, inFd);
  ioTaskStdSet(0, STD_OUT, outFd);
  ioTaskStdSet(0, STD_ERR, errFd);
  
  return(error);
}


/*
 *  bad_parser_subcommand
 *  Common code fragment for handling a bad subcommands with the new parser
 */
void bad_parser_subcommand (parseinfo *csb, uint var)
{
    printf("Bad Parser Sub-command : %s\n", 
                   (csb->nvgen)?csb->nv_command:csb->line);
}


/*****************************************************************
 *
 *   Func: ParserCallbackInit
 *
 *   Desc: Initialize all parser callback tasks
 *
 *   Inputs:  None
 *
 *   Result:  None
 *
 *   Algorithm:
 *
 *****************************************************************/
extern int ParserCallBackTask[3];
void ParserCallbackInit( void )
{
#define MAX_CALLBACK_TASKS 3

        tINT index;
        char tname[32];

        /* Create the Message Queue. The parser (tParser) communicates
         * with the callback tasks (tCallback) via this message queue
         */
        callback_Q_Id = msgQCreate( 1, sizeof(tpCALLBACKMSG), MSG_Q_FIFO);
        
        if ( callback_Q_Id == NULL ) {
                PSpanic("Cannot create callback message queue\n");
        }
        
        /* Spawn all the callback tasks */
        for ( index = 0; index < MAX_CALLBACK_TASKS; index++) {
          sprintf(tname, "tCallback%d", index); 
           ParserCallBackTask[index]=taskSpawn(tname,PARSER_TASK_PRIORITY, PARSER_TASK_OPTION, PARSER_TASK_STACK_SIZE,
                         (FUNCPTR)parser_exec_callback, 0,0,0,0,0,0,0,0,0,0); 
            if(ParserCallBackTask[index] == ERROR ) {
            PSpanic("Cannot spawn callback tasks\n");
          }
          
        }
}
void ParserCallbackRestart(int index){
       
        char tname[32];
        /* Create the Message Queue. The parser (tParser) communicates
         * with the callback tasks (tCallback) via this message queue
         */
        callback_Q_Id = msgQCreate( 1, sizeof(tpCALLBACKMSG), MSG_Q_FIFO);
        
        if ( callback_Q_Id == NULL ) {
                PSpanic("Cannot create callback message queue\n");
        }
        /* Spawn all the callback tasks */
          sprintf(tname, "tCallback%d", index); 
           ParserCallBackTask[index]=taskSpawn(tname,PARSER_TASK_PRIORITY, PARSER_TASK_OPTION, PARSER_TASK_STACK_SIZE,
                         (FUNCPTR)parser_exec_callback, 0,0,0,0,0,0,0,0,0,0); 
            if(ParserCallBackTask[index] == ERROR ) {
            PSpanic("Cannot spawn callback tasks\n");
          }
          
}
