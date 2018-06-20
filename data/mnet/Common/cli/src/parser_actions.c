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
#include <../include/parser_privilege.h>
/*#include <limits.h>
  /#include <clock.h> */

#define DAYS_IN_WEEK  7
#define MONTHS_IN_YEAR 12

#define CHECK_RANGE_BOUNDS()\
        if (arg->upper >= arg->lower) {\
                lower = arg->lower;\
                upper = arg->upper;\
        } else {\
                lower = arg->upper;\
                upper = arg->lower;\
        }
        
#define CHECK_RANGE_BOUNDS_ALIST()\
        if (arg->upper >= arg->lower) {\
                lower = arg->lower;\
                upper = arg->upper;\
        } else {\
                lower = arg->upper;\
                upper = arg->lower;\
        }\
        if (arg->upper2 >= arg->lower2) {\
                lower2 = arg->lower2;\
                upper2 = arg->upper2;\
        } else {\
                lower2 = arg->upper2;\
                upper2 = arg->lower2;\
        }
        
/********************************/
/*      Module Local Data       */
/********************************/

const char *const month_name[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *const day_name[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri",
                                "Sat"};
const char *const long_month_name[] = {"January", "February", "March",
                                       "April", "May", "June", "July",
                                       "August", "September", "October",
                                       "November", "December"};
const char *const long_day_name[] = {"   ", "Sunday", "Monday", "Tuesday",
                                     "Wednesday", "Thursday", "Friday",
                                     "Saturday"};

        
#ifdef IOS_PARSER

/*
 *  Local variables
 *  Uncomment this when it is necessary
 */
keyword_options permitdeny_options[] = {
        { "deny", "Specify packets to reject", FALSE },
        { "permit", "Specify packets to forward", TRUE },
        { NULL, NULL, 0 }
};

static boolean match_spaces(char *buf, int *pi);
static char *hex_to_byte(char *s, char *out);

#endif /* IOS_PARSER */

/*
 *  Forward declarations
 */
static void parser_bail_out(parseinfo *, void (*)(parseinfo *), const char *);
static void set_error_index(parseinfo *);
static boolean match_signed_dec(char *, int, int, int *, int *);

static void save_ambig_int(parseinfo *, const char *, int);
static void save_ambig_uint(parseinfo *, const char *, uint);
static void save_ambig_nstring(parseinfo *, const char *, const char *, uint);
static void save_ambig_string_flag(parseinfo *, const char *, const char *, boolean); 
static void save_help_short(parseinfo *, const char *, boolean);
static void save_help_msg(parseinfo *, const char *);
static void nvgen_action(parseinfo *, transition *, void (*)(parseinfo *));
void func_action (parseinfo *csb, transition *mine, void (*func) (parseinfo *));
/* The parser match routines use simpler C routines to match the smaller
 * pieces of tokens.  These sub-routines come in two flavors:
 *
 * XXX_match    returns (int) the number of chars it matched;
 *              use conclude_match to back over a successful match
 * XXX_check    returns (boolean) the status of the check;
 *              consumes no input
 */

/* These simple match and check routines could be made inline:
 *
 * terminator_check     Did user hit RETURN, start a comment, or request help?
 * eol_check            Did user hit RETURN or start a comment here?
 * help_check           Did user request help here?
 *                      Don't use this directly, try one of the next ones;
 * long_help_check      Did user request long help here?
 *                      i.e, is this BOL or was last char W/S?
 * short_help_check     Did user request short help here?
 *                      Opposite of long_help_check
 */

static boolean terminator_check (parseinfo *csb)
{
    return(is_terminator(csb->line[csb->line_index]));
}

boolean eol_check (parseinfo *csb)
{
    return(!csb->in_help && terminator_check(csb));
}

 boolean help_check (parseinfo *csb)
{
    return(csb->in_help && !csb->nvgen && terminator_check(csb));
}

static boolean long_help_check (parseinfo *csb)
{
    return(help_check(csb) &&
                   (csb->line_index == 0 || isspace(csb->line[csb->line_index - 1])));
}

static boolean short_help_check (parseinfo *csb)
{
    return(help_check(csb) && !long_help_check(csb));
}

static boolean priv_check (parseinfo *csb, trans_priv *priv)
{
    /*
     * Only process keywords which are visible to the user at their current
     * priv level.
     */
    if (csb->priv < priv->cur_priv) {
                return(FALSE);
    }

    if ((TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_INTERACTIVE)) {
                if (csb->flags & CONFIG_HTTP) {
                        return(FALSE);
                }
        }

    /* Modify the current keyword privilege
     * Don't allow internal commands unless they are enabled.
     */
        
    if ((TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_INTERNAL)
                /* &&   !internal_cmd_enable */ ) {
                return(FALSE);
    }
        
    /* Don't allow keywords not applicable to synalc
     */
        
    if ((TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_PSYNALC)
                /* && (cpu_type == CPU_SYNALC) */ 
                ) {
                return(FALSE);
    }
        
    if (csb->in_help) {
                /* Don't display keywords which are hidden */
                if (TRANS_PRIV_FLAGS_GET(priv->flags) &
                        (PRIV_HIDDEN | PRIV_UNSUPPORTED)) {
                        return(FALSE);
                }
    }
    /* Don't do NV generation on keywords that are obsolete */
    if (csb->nvgen &&
                !csb->priv_set &&
                (TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_NONVGEN)) {
                return(FALSE);
    }
        
    if ((TRANS_PRIV_FLAGS_GET(priv->flags) &
                 (PRIV_HIDDEN | PRIV_UNSUPPORTED)) ||
                ((TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_NOHELP) &&
                 (csb->in_help == PARSER_NO_HELP))) {
                csb->command_visible = FALSE;
    }
    if (TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_UNSUPPORTED) {
                csb->unsupported = TRUE;
    }

    return(TRUE);
}

static void keyword_flag_modify (parseinfo *csb, trans_priv *priv, uint *flags)
{
    /* If this is a duplicate keyword, as indicated by PRIV_DUPLICATE in
     * the keyword privileges, then reset KEYWORD_HELP_CHECK.  This will
     * prevent match_partial_keyword() from supplying any help for this
     * keyword.
     */

    if ((TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_DUPLICATE) ||
                (TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_NOHELP) ||
                ((TRANS_PRIV_FLAGS_GET(priv->flags) & PRIV_USER_HIDDEN) &&
                 (csb->priv == PRIV_USER) &&
                 !(csb->flags & CONFIG_HTTP))) {
                *flags &= ~KEYWORD_HELP_CHECK;
    }
}

boolean match_char (char *cp, int *offset, char c)
{
    if (*(cp + *offset) == c) {
                (*offset)++;
                return(TRUE);
    }
    return(FALSE);
}

/*
 * In effect, combines the match_whitespace ... csb->line_index += j; if
 * (j | eol_check) sequences.
 */
boolean match_whitespace2 (char *cp, int *offset, boolean in_help)
{
    if (isspace(cp[*offset])) {
                while (isspace(cp[*offset])) {
                        (*offset)++;
                }
                return(TRUE);
    }
    if ((!in_help) && is_terminator(cp[*offset])) {
                return(TRUE);
    }
    return(FALSE);
}

boolean get_name (char *buf, char *name, int *pi, int buflen)
{
    char *sp, *dp;
    int i;

    /* Copy the name into local working storage */
    sp = buf;
    dp = name;
    for (i = 0; i < buflen; i++) {
                if (*sp == ' ' || *sp == '\0') {
                        break;
                }
                *dp++ = *sp++;
    }
    *dp = '\0';
    if (i == 0)
                return(FALSE);
    if (i == buflen && (*sp != ' ' || *sp != '\0')) {
                return(FALSE);
    }

    *pi = i;
    return(TRUE);
}

/* NONE does just what its name implies */

void NONE_action (parseinfo *csb)
{
}

/* link_point is a place holder.  It should never be reached. */

void link_point_action (parseinfo *csb)
{
        printf("link_point_action(): Error\n");
}

/*
 * no_alt is used at the end of a list of alternative tokens.
 *
 * There are several actions performed in no-alt-action, depending on the
 * state of the parser.
 *
 * If performing NV generation, we return(i.e. do nothing).
 *
 * If providing help, we must check to see if more than one command has
 * provided help (i.e. the command is ambiguous.)  However, we can have
 * multiple options generated by a single command.  We keep track of all
 * this by storing, each time through no_alt, the length of the help text
 * generated so far.  If the length has increased since the last no_alt,
 * we increment csb->multiple_funcs.  Thus we count how many token chains
 * have generated help.
 *
 * If no-alt-action is called when setting keyword privileges, then
 * don't treat it as an error.  Increment csb->multiple_funcs so that
 * token chains that have to check for previously matched tokens
 * will work correctly.
 *
 * When no-alt-action is encountered in normal processing, increase
 * error_index to our current line position if it's smaller.
 * This indicates the deepest character in the command line we reached.
 * parse_cmd() checks csb->multiple_funcs first to determine if an error
 * occured.  It will be zero if no command match occurred, and greater
 * than one if the command input is ambiguous.  If no command matched,
 * error_index is deepest character we reached in the parse, and tells
 * where to put an error marker.
 */

void no_alt_action (parseinfo *csb)
{
    ambig_struct *ap;

    if (csb->command_visible) {
                ap = &csb->visible_ambig;
    } else {
                ap = &csb->hidden_ambig;
    }

    /* NV generation */
    if (csb->nvgen) {
                return;
    }

    /*
     * Priv set processing.  If csb->priv_set is not PRIV_NULL, then don't
     * treat being called as an error.  Increment csb->multiple_funcs and
     * return.
     */
    if (csb->priv_set) {
                increment_multiple_funcs(csb, "no_alt_action 1");
                return;
    }

    /* Help processing.  Check the ambiguity buffers against one another.
     * If ambig is non-null, we've matched some input and need to
     * compare against ambig_save to see if it is ambiguous.
     * This yields two cases:
     * 1. ambig_save == NULL, indicating that this is the first match
     *    and that multiple_funcs should be incremented.
     * 2. ambig_save != NULL, indicating that this is ambiguous and
     *    that multiple_funcs should be incremented again.
     */
    if (parser_ambig_debug) {
                printf("\n1 Ambig       buf '%s'", ap->ambig);
                printf("\n1 Ambig_save  buf '%s'", ap->ambig_save);
    }
        /***************************************************************************
         ***************************************************************************
         *
         *
         *  Before you touch the ambiguity code, make sure you know what you
         *  are doing, or else it will break
         *
         *  Here are some cases to case the behavior of the code
         
         en
         
         ! output should "interface  internals"
         show ip ospf int?
         
         ! ambiguous
         show ip ospf int ?
         
         conf term
         ! output should be "x25  x29  xns"
         service pad
         x?
         ! ambiguous
         x ?
         ! output should be "x25  x29"
         x2?
         ! ambiguous
         x2 ?
         ! output should be "route  routing"
         xns rout?
         ! ambiguous
         xns rout ?
         ! output should be "default-gateway  default-network"
         ip default?
         ! ambiguous
         ip default ?
         
         end
         !
         !
         ! To test the hidden keyword ambiguity stuff
         ! output should be "where"
         disable
         w?
         ! should match "where"
         w
         ! this command should also work
         who
         !
         ! output should be "snap"
         enable
         conf term
         interface Ethernet 0
         arp s?
         ! should match "arp snap"
         arp s
         ! this command should also work
         arp smds
         !
         ! broken
         slip /compressed ?
         
         *
         *
         *
         ***************************************************************************
         ***************************************************************************/
 
    if (ap->ambig[0]) {
                if (ap->ambig_save[0]) {
                        int i = strlen(ap->ambig_save);
                        boolean ambig_save_longer = FALSE;

                        if (strlen(ap->ambig) < i) {
                                ambig_save_longer = TRUE;
                                i = strlen(ap->ambig);
                        }

                        if (strncmp(ap->ambig, ap->ambig_save, i) != 0) {
                                if (parser_ambig_debug) {
                                        printf("\nAmbig case 1");
                                }
                                /*
                                 *  If there is something in the ambig_save buffer
                                 *  and it is NOT the same as the ambig_save
                                 *  buffer, so the keyword is matches or and we
                                 *  increase multiple_funcs and copy the ambig
                                 *  buffer to ambig_save buffer.
                                 */
                                increment_multiple_funcs(csb, "no_alt_action 2");

                                /* Copy everything */
                                strcpy(ap->ambig_save, ap->ambig);

                                /* Clear ambig */
                                ap->ambig[0] = '\0';

                        } else {
                                if (parser_ambig_debug) {
                                        printf("\nAmbig case 2");
                                }
                                /*
                                 *  If there is something in the ambig buffer
                                 *  and it is the same as in the ambig_save buffer,
                                 *  then we have a duplicate keyword and everything
                                 *  is ok, but we need to copy the longer string to
                                 *  ambig_save.
                                 */
                                if (!ambig_save_longer) {
                                        strcpy(ap->ambig_save, ap->ambig);
                                }
                        }
                } else {
                        if (parser_ambig_debug) {
                                printf("\nAmbig case 3");
                        }
                        /*
                         *  We didn't match anything before, so this is
                         *  the first match, and multiple_funcs has already
                         *  been set.
                         */
                        if (csb->in_help) {
                                increment_multiple_funcs(csb, "no_alt_action 3");
                        }

                        /* Copy everything */
                        strcpy(ap->ambig_save, ap->ambig);

                        /* Clear ambig */
                        ap->ambig[0] = '\0';

                }
    } else {
                if (parser_ambig_debug) {
                        printf("\nAmbig case 4");
                }
                /*
                 *  If the ambig buffer is empty,
                 *  then we are at the end of a parse chain,
                 *  so clear the ambig_save buffer
                 */
                ap->ambig_save[0] = '\0';
    }
    if (parser_ambig_debug) {
                printf("\n2 Ambig       buf '%s'", ap->ambig);
                printf("\n2 Ambig_save  buf '%s'", ap->ambig_save);
    }
    if ((parser_help_debug && csb->in_help) || parser_ambig_debug) {
                print_multiple_funcs(csb);
                printf("\nCurrent help string:\n");
                print_buf(csb->help_save, TRUE, TRUE, 0, 0);
                printf("\n");
    }
    /* Normal Error processing */
    set_error_index(csb);
}
void func_action (parseinfo *csb, transition *mine, void (*func) (parseinfo *))
{
    INVALID_NVGEN_CHECK;

    if (!csb->in_help && func) {
                (void) func(csb);
    }
    push_node(csb, mine->accept);
}
/*
 * NOP - Generally used as a label
 *
 * Differs from NONE action in that it takes it's accepting state.
 */

void NOP_action (parseinfo *csb, transition *mine)
{
    push_node(csb, mine->alternate);
    push_node(csb, mine->accept);
};

 /*
  * KEYWORD matches a unique substring of a specified string
  *
  * arg->minmatch is the minimum required number of matching characters
  *
  */
void keyword_action (parseinfo *csb, transition *mine,
                                         keyword_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, -1, 0, 0,
                (KEYWORD_WS_OK | KEYWORD_HELP_CHECK)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_mm_action (parseinfo *csb, transition *mine,
                                                keyword_mm_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, -1, 0, arg->minmatch,
                (KEYWORD_WS_OK | KEYWORD_HELP_CHECK)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_id_action (parseinfo *csb, transition *mine,
                                                keyword_id_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, arg->offset, arg->val, 0,
                (KEYWORD_WS_OK | KEYWORD_HELP_CHECK)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_orset_action (parseinfo *csb, transition *mine,
                                                   keyword_id_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, arg->offset, arg->val, 0,
                (KEYWORD_WS_OK | KEYWORD_HELP_CHECK | KEYWORD_OR_SET)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_optws_action (parseinfo *csb, transition *mine,
                                                   keyword_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, -1, 0, 0,
                (KEYWORD_WS_OK | KEYWORD_NO_WS_OK | KEYWORD_HELP_CHECK)};
    general_keyword_action(csb, mine, &argout);
}


void general_keyword_action (parseinfo *csb, transition *mine,
                                                         general_keyword_struct * const arg)
{
    int i = 0;
    uint flags;


    /* Setup to check the alternates. */
    push_node(csb, mine->alternate);

    if ((arg->flags & KEYWORD_TEST) &&
                (arg->offset != -1)) {
                if (arg->flags & KEYWORD_OR_SET) {
                        if (*CSBVAR(csb, arg->offset, uint) & arg->val) {
                                return;
                        }
                } else {
                        if (*CSBVAR(csb, arg->offset, uint)) {
                                return;
                        }
                }
    }

    /* Check for privilege to use this keyword */
    if (priv_check(csb, arg->priv) == FALSE) {
                return;
    }

    if (csb->nvgen) {
                /*
                 * Doing NV generation.  Save the transition struct and the keyword
                 * text for the NV generation function.
                 */

                if (arg->offset != -1) {
                        if (arg->flags & KEYWORD_OR_SET) {
                                *CSBVAR(csb, arg->offset, uint) |= arg->val;
                        } else {
                                *CSBVAR(csb, arg->offset, uint) = arg->val;
                        }
                }
                if (!csb->priv_set || nvgen_privilege(csb, arg->priv, arg->str)) {
                        nvgen_token(csb, mine, arg->str);
                } else {
		  printf("\n privilege check failed in keyword action");
		}
                return;
    }

    /* Make a copy of the flags word because it may originate in ROM
     * and we may need to modify the flags before matching the keyword.
     */
    flags = arg->flags;
    keyword_flag_modify(csb, arg->priv, &flags);

    /*
     * If we are at the end of the input stream and we're doing help, add the
     * long help message, which is just the concatenation of the search string
     * and the help text. We fall-through in any case.  If long help is being
     * requested, nothing matches and we return.
     * 
     * Otherwise, we are expecting to process a token. If the token matching
     * fails, due to an unexpected end of the input stream or due to a
     * different command, we will fall through the test and transition to the
     * alternates.
     *
     * The alternates are processed in any case so that we
     * exhaustively check all possible commands against the input
     * stream.  See the parser README for more information on how the
     * commands are uniquely matched.
     */
        
    /*
     * Check to see if the input matches this token.  If there is no match,
     * we skip processing this token and try the alternates.
     */
        
    i = 0;
    if (match_partial_keyword(csb, arg->str, &i, arg->help,
                                                          arg->minmatch, flags, arg->priv->cur_priv)) {
                csb->line_index += i;
                
                if (csb->priv_set) {
                        /*
                         * Modify the current keyword privilege
                         */
                        priv_push(csb, arg->priv);
                }
                csb->last_priv = arg->priv->cur_priv;
                if (csb->last_priv > csb->highest_priv) {
                        csb->highest_priv = csb->last_priv;
                }
                if (arg->offset != -1) {
                        if (arg->flags & KEYWORD_OR_SET) {
                                *CSBVAR(csb, arg->offset, uint) |= arg->val;
                        } else {
                                *CSBVAR(csb, arg->offset, uint) = arg->val;
                        }
                }
#ifdef  DEBUG_KEYWORD
                printf("\nkeyword_action: Pushing accept");
#endif  DEBUG_KEYWORD
                push_node(csb, mine->accept);
                csb->line_index -= i;
    } else {
                if (parser_ambig_debug && i) {
                        printf("\nDidn't match keyword '%s'", arg->str);
                }
#ifdef  DEBUG_KEYWORD
                printf("\nkeyword_action: match_partial_keyword returned FALSE");
#endif  DEBUG_KEYWORD
    }
}

/* EOL prints help and does func call at EOL
 *
 * A push-down stack of parseinfo structures is used to save the state of
 * the current parse.  A new parseinfo struct is used for each possible
 * command so that successive partial matching commands do not overwrite
 * data stored in the parseinfo struct of a fully matching command.
 */

void eol_action (parseinfo *csb, transition *mine, void (*func) (parseinfo *))
{
    parseinfo *savepd;
    char *short_help = "<cr>";

    if (parser_ambig_debug && !csb->nvgen) {
                printf("\nEol Action");
    }
    /*
     * If setting a command's privileges, indicate that the command was
     * successful and return.
     
    if (csb->priv_set) {
                increment_multiple_funcs(csb, "eol_action 1");
                return;
		}*/
    if (csb->nvgen) {
                if (func) {
                        (void) func(csb);
                }
    } else {
                if (csb->in_help) {
                        if (help_check(csb)) {
                                if (csb->flags & CONFIG_HTTP) {
                                        save_help_long(csb, "CR", NULL, csb->priv);
                                } else {
                                        save_help_long(csb, short_help, NULL, csb->priv);
                                }
                                save_ambig_string_flag(csb, short_help, short_help, FALSE);
                                no_alt_action(csb);
                        }
                        return;
                }
                if (eol_check(csb)) {

                        increment_multiple_funcs(csb, "eol_action 2");

                        /* savepd = chunk_malloc(parseinfo_chunks); */
                        savepd = ( parseinfo *)PARSERmalloc(sizeof(parseinfo) );
                        if (savepd == NULL) {
                                parser_bail_out(csb, func, "eol_action");
                                return;
                        }

                        /*
                         * Make a copy of the current parse data (savepd) before
                         * modifying current info. This implements a push-down
                         * stack where the top of the stack is the current
                         * (possibly invalid) parse state.  Any elements below the
                         * top are parse data from matching commands.  Only one
                         * element should be on the stack if a command matches the
                         * input.
                         */

                        save_ambig_string_flag(csb, short_help, short_help, FALSE);

                        *savepd = *csb;                         /* Struct copy */

                        csb->next = savepd;
                        savepd->action_func = func;
                } else {

                        /*
                         * This command did not accept.  Remove it from the linked list of
                         * accepting command states.  Don't discard the current error_index
                         * since it indicates that we came this far in the input before
                         * failing.
                         */
                        set_error_index(csb);
                }
                no_alt_action(csb);
    } /* if (csb->nvgen) */
}

/* EOLS - prints help and does func call at EOL.  Sets csb->which with
 * the EVAL macro so that 'val' is not constrained to be a constant.
 */

void eols_action (parseinfo *csb, transition *mine,
                                  eols_struct * const arg)
{
    csb->which = arg->subfunc;
    eol_action(csb, mine, arg->func);
}

void eolns_action (parseinfo *csb, transition *mine, void (*func) (parseinfo *))
{
    csb->which = 0;
    eol_action(csb, mine, func);
}

/*
 * EOLI - EOL for interactive command.  Sets TERMINAL_MODE flag
 * that gets saved if this command match succeeds.
 */
void eoli_action (parseinfo *csb, transition *mine,
                                  eols_struct * const arg)
{
    csb->flags |= TERMINAL_MODE;
    csb->which = arg->subfunc;
    eol_action(csb, mine, arg->func);

        /*
         * Clear TERMINAL_MODE so the next command doesn't
         * get it by mistake.
         */
    csb->flags &= ~TERMINAL_MODE;
}

void general_number_short_help_http (uint *flags, uint lower, uint upper,
                                                                         char *help, boolean doing_http)
{
    char *open_bracket = "<";
    char *close_bracket = ">";

    if (doing_http) {
                open_bracket = close_bracket = "";
    }

    switch (*flags & (NUMBER_HEX|NUMBER_OCT|NUMBER_DEC)) {
        default:
        case 0:
                printf("general_number_short_help_http: NUMHELP\n");
                *flags |= (NUMBER_HEX|NUMBER_OCT|NUMBER_DEC);
        case NUMBER_HEX | NUMBER_OCT | NUMBER_DEC:
        case NUMBER_HEX | NUMBER_OCT:
        case NUMBER_HEX | NUMBER_DEC:
        case NUMBER_OCT | NUMBER_DEC:
        case NUMBER_DEC:
                sprintf(help, "%s%u-%u%s", open_bracket, lower, upper, close_bracket);
                break;

        case NUMBER_HEX:
                switch (*flags & (HEX_ZEROX_OK|HEX_NO_ZEROX_OK)) {
                default:
                case 0:
                        /* errmsg(&msgsym(NUMHELP, PARSER), "hex"); */
                        printf("NUMBER_HEX : Error !\n");
                        *flags |= HEX_ZEROX_OK;
                case HEX_ZEROX_OK:
                case HEX_ZEROX_OK | HEX_NO_ZEROX_OK: /* why not just put this case here */
                        sprintf(help, "%s0x%x-0x%x%s",
                                        open_bracket, lower, upper, close_bracket);
                        break;
                case HEX_NO_ZEROX_OK:
                        sprintf(help, "%s%x-%x%s", open_bracket,
                                        lower, upper, close_bracket);
                        break;
                }
                break;

        case NUMBER_OCT:
                switch (*flags & (OCT_ZERO_OK|OCT_NO_ZERO_OK)) {
                default:
                case 0:
                        /* errmsg(&msgsym(NUMHELP, PARSER), "octal");*/
                        printf("NUMBER_OCT : Error!\n");
                        *flags |= OCT_ZERO_OK;
                case OCT_ZERO_OK:
                case OCT_ZERO_OK | OCT_NO_ZERO_OK: /* why not just put this case here */
                        sprintf(help, "%s0%o-0%o%s", open_bracket,
                                        lower, upper, close_bracket);
                        break;
                case OCT_NO_ZERO_OK:
                        sprintf(help, "%s%o-%o%s", open_bracket, lower, upper,
                                        close_bracket);
                        break;
                }
                break;
    }
}

/*
 * GENERAL_NUMBER matches a ranged, unsigned number, minimum one digit, and
 * assigns it to a int.  It also handles help and whitespace.  If the
 * input begins with '0x' and contains at least one hex digit, the
 * conversion is from Hex.  If the leading digit is '0', and the
 * number is considered octal. The <types> field of the number_struct
 * is a flag variable
 * indicating what types of input (hex, octal, decimal) are allowed.
 *
 * The following function takes additional structure arguments that
 * allow whitespace and help checking to be turned off.
 */

/* Scan a number and accept. Types of numbers (hex, decimal, octal) and
 * allowable range are specified via <*arg>. <*arg> also specifies via
 * another field whether whitespace after is to be tested for (required)
 * or not. Yet another field specifies whether help checks are to be
 * done.
 */

void general_number_action (parseinfo *csb, transition *mine,
                                                        number_struct * const arg)
{
    int i = 0;
    uint num = 0;
    char short_help[11];
    uint lower;
    uint upper;

    push_node(csb, mine->alternate);

    CHECK_RANGE_BOUNDS();

    if ((arg->flags & NUMBER_NV_RANGE) && csb->nvgen) {
                queuetype *t1 = csb->tokenQ;
                queuetype t2;
                uint l = strlen(csb->nv_command);

                for (i=lower; i<= upper; i++) {
                        *CSBVAR(csb, arg->offset, uint) = i;
                        sprintf(short_help, "%d", i);

                        csb->nv_command[l] = '\0';
                        csb->tokenQ = &t2;
                        queue_init(csb->tokenQ, 0);
                        nvgen_token(csb, mine, short_help);
                        parse_token(csb);
                        csb->tokenQ = t1;
                }
                return;
    }

    /*
     * We should never be called for NV generation
     */
    INVALID_NVGEN_CHECK;

    /* match the number */
    if (match_number(csb, lower, upper, &i, &num, arg->flags, arg->help)) {
#ifdef  DEBUG_NUMBER
                printf("\ngeneral_number_action: matched number %d", num);
#endif  DEBUG_NUMBER

                *CSBVAR(csb, arg->offset, uint) = num;

                csb->line_index += i;
                push_node(csb, mine->accept);
                csb->line_index -= i;

    }
}

void signed_number_action (parseinfo *csb, transition *mine,
                                                   signed_number_struct * const arg)
{
    int i = 0;
    int num = 0;
    char short_help[32];
    int lower;
    int upper;

    push_node(csb, mine->alternate);
    /*
     * We should never be called for NV generation
     */
    INVALID_NVGEN_CHECK;

    CHECK_RANGE_BOUNDS();

    sprintf(short_help,"<%d - %d>", lower, upper);

    if (help_check(csb)) {
                save_help_long(csb, short_help, arg->help, csb->priv);
                return;
    }

    /* match the number */
    i = 0;
    if (match_signed_dec(&csb->line[csb->line_index],
                                                 lower, upper, &i, &num) &&
                match_whitespace2(&csb->line[csb->line_index], &i, csb->in_help)) {

                *CSBVAR(csb, arg->offset, int) = num;

                save_ambig_int(csb, short_help, num);

                csb->line_index += i;
                push_node(csb, mine->accept);
                csb->line_index -= i;
    } else {
                help_or_error(csb, i, short_help, NO_COMMAND_COMPLETION);
    }
}

/*
 * HELP
 */

void help_action (parseinfo *csb, transition *mine,
                                  help_struct * const arg)
{
    /*
     * If doing NV generation, go to alternate (skip help output). If we're not
     * in help mode, then obviously skip to alternate. If not at the end of
     * line, we're not doing help, so go to alternate. Otherwise, this is a
     * valid help request and we add the current text to the help output
     * buffer.  But the text is only added if it exists. Transition to
     * alternate when the help string is saved to allow other tokens at this
     * level to generate help output.
     */

    if (help_check(csb)) {
                if (csb->flags & CONFIG_HTTP) {
                        save_line(csb, &csb->help_save, "<DT>%s", arg->str);
                } else {
                        save_help_msg(csb, arg->str);
                }
    }
    push_node(csb, mine->alternate);
}




/* SET takes a pointer to a variable and a constant expression to set
 * that variable to.  It is considerably more efficient than EVAL.
 *
 * Likewise, TESTVAR tests a variable for its relation with a constant
 * expression.  It, in turn, is more efficient than ASSERT.
 */

void set_action (parseinfo *csb, transition *mine,
                                 set_struct * const arg)
{
    *CSBVAR(csb, arg->offset, uint) = arg->val;
    push_node(csb, mine->accept);
}

/* Use a temporary variable in case the variable changes during the
 * parse (such as checking csb->multiple_funcs when searching for help
                  * strings).
                  */
void test_action (parseinfo *csb, transition *mine,
                                  test_struct * const arg)
{
    uint tmp = *CSBVAR(csb, arg->offset, uint);

    push_node(csb, mine->alternate);
    if (tmp == arg->val)
                push_node(csb, arg->equal);
    if (tmp != arg->val)
                push_node(csb, arg->notequal);
    if (tmp < arg->val)
                push_node(csb, arg->lt);
    if (tmp > arg->val)
                push_node(csb, arg->gt);
}

void general_string_action (parseinfo *csb, transition *mine,
                                                        general_string_struct * const arg)
{
    int i;
    boolean ws;
    const char *short_help = (arg->flags & STRING_WS_OK) ?
                "LINE" :  "WORD";

    push_node(csb, mine->alternate);
    INVALID_NVGEN_CHECK;

    if (help_check(csb)) {
                if (arg->flags & STRING_HELP_CHECK) {
                        save_help_long(csb, short_help, arg->help, csb->priv);
                }
                return;
    }

    i = copy_varstring(csb->line + csb->line_index,
                                           CSBVAR(csb, arg->offset, char),
                                           PARSEBUF, arg->flags);

    if (i) {
                ws = match_whitespace2(csb->line + csb->line_index, &i, csb->in_help);
                if (ws || (csb->flags & CONFIG_HTTP) || (arg->flags & STRING_WS_OK)) {
                        save_ambig_string(csb, short_help, CSBVAR(csb, arg->offset, char));
                        csb->line_index += i;
                        push_node(csb, mine->accept);
                        csb->line_index -= i;
                }

                if (!ws || (arg->flags & STRING_WS_OK)) {
                        if (arg->flags & STRING_HELP_CHECK) {
                                help_or_error(csb, i, short_help, NO_COMMAND_COMPLETION);
                        } else {
                                csb->line_index += i;
                                set_error_index(csb);
                                csb->line_index -= i;
                        }
                }
    }
}

/*
 * NVGEN - nvgen_action
 *
 * Call the NV generation function.
 */
void nvgen_action (parseinfo *csb, transition *mine, void (*func)(parseinfo *))
{

    if (csb->nvgen) {
      /*
       * If csb->priv_set is set, we currently don't go farther than
       * we can NV generate, so stop here!
       */

      if (csb->sense && func) {
	(void) func(csb);
      }
    } else {
      push_node(csb, mine->accept);
    }
}

/*
 * NVGENS - nvgens_action
 *
 *
 */
void nvgens_action (parseinfo *csb, transition *mine,
                                        nvgens_struct * const arg)
{
    csb->which = arg->subfunc;
    nvgen_action(csb, mine, arg->func);
}

void nvgenns_action (parseinfo *csb, transition *mine, void (*func)(parseinfo *))
{
    csb->which = 0;
    nvgen_action(csb, mine, func);
}

/*
 * nvgen_token
 *
 * Find the end of the existing command and a space if this isn't the
 * first token in the command.  Then
 * add the token to the command line.  Continue the generation by
 * transitioning to the accepting node.
 * When we return, reset the command line to its state before this token
 * was added and transition to the alternate node.
 */
void nvgen_token (parseinfo *csb, transition *mine, const char *token)
{
    char *cp;

    if (!csb->nvgen) {
                /* errmsg(&msgsym(NVGEN, PARSER) */
                printf("nvgen_token : NVGEN\n");
                return;
    }

    /*
     * The alternate transition should have already been pushed onto the stack
     * before calling nvgen_token.
     */
    for (cp = csb->nv_command; *cp; cp++); /* Null body */

    /* Add the current token and go to the accepting node */
    if (cp != csb->nv_command) {
                *cp = ' ';                                              /* Add a space unless at beginning */
                strcpy(cp+1, token);
    } else {
                strcpy(cp, token);
    }
    push_node(csb, mine->accept);
    *cp = '\0';                                                 /* Restore prior state */
    return;
}

/* NOPREFIX
 * Go to 'accept' if there isn't a 'no' prefix.
 * Consume the remainder of the input and go to 'alternate' if there
 * was a 'no' prefix.
 */

void noprefix_action (parseinfo *csb, transition *mine)
{
    int i;

    if (csb->nvgen) {
                push_node(csb, mine->accept);
                return;
    }

    if ((csb->sense == TRUE) && (csb->set_to_default == FALSE)) {
                push_node(csb, mine->accept);
    } else {
                for (i = 0; csb->line[csb->line_index + i] != '\n' &&
                         csb->line[csb->line_index + i]; i++) {
                }
                csb->line_index += i;
                push_node(csb, mine->alternate);
                csb->line_index -= i;
    }
}

/*
 * Match a decimal short unsigned int in a range
 */
boolean match_decimal (char *buf, uint low, uint high, int *pi, ushort *pvalue)
{
    ushort value = 0;
    uint i;

    buf += *pi;
    *pvalue = 0;

    i = 0;
    if (buf[i] == '+')
                i++;

    for (; isdigit(buf[i]); i++) {
                if ((value > USHRT_MAX / 10) ||
                        ((value == USHRT_MAX / 10) &&
                         ((buf[i] - '0') > USHRT_MAX % 10))) {
                        *pi += i;
                        return(FALSE);
                }
                value = 10 * value + buf[i] - '0';
                if (value > high) {
                        *pi += i;
                        return(FALSE);
                }
    }

    if (i && (value >= low)) {
                *pi += i;
                *pvalue = value;
                return(TRUE);
    }
    return(FALSE);
}

/*
 * Try to match an signed int,
 * with value between (or equal to) <low> and
 * <high>. <*pi> is incremented by the number of characters in
 * the match.
 *
 * Here's a command to test this function with
        int e0
        ip irdp
        ip irdp address 1.1.1.1 -2147483649             ! fail
        ip irdp address 1.1.1.2 -2147483648             ! succeed
        ip irdp address 1.1.1.3 -1                      ! succeed
        ip irdp address 1.1.1.4 -0                      ! succeed
        ip irdp address 1.1.1.5 -                       ! fail
        ip irdp address 1.1.1.6                 ! fail
        ip irdp address 1.1.1.7 0                       ! succeed
        ip irdp address 1.1.1.8 1                       ! succeed
        ip irdp address 1.1.1.9 2147483647              ! succeed
        ip irdp address 1.1.1.10 2147483648             ! fail
 */
static boolean match_signed_dec (char *buf, int low, int high,
                                                                 int *pi, int *pvalue)
{
    int i;
    int value = 0;
    char c;
    int sign;

    buf += *pi;
    *pvalue = 0;

    i = 0;
    if (buf[i] == '-') {
                sign = -1;
                i++;
    } else {
                sign = 1;
    }
    if (buf[i] == '+')
                i++;
        
    for (c = buf[i]; isdigit(c); c = buf[++i]) {
                if (((value*10 + (c - '0')) < value) &&
                        ((sign != -1) || ((value*10 + (int)(c - '0')) != 1<<31))) {
                        /* Too many chars, max out. */
                        *pi += i;
                        return(FALSE);
                }
                value = value * 10 + (int) (c - '0');
                if ((value*sign) > high) {
                        *pi += i;
                        return(FALSE);
                }
    }
        
    *pi += i;
    /*
     * if (number is positive and we parsed one or more numbers) or
     *    (number is negative and we parsed '-' plus one or more numbers)
     */
        if (((sign == 1) && (i > 0)) ||
                ((sign == -1) && (i > 1))) {
                
                /*
                 *  if (number is greater than or equal to the lower boundary)
                 */
                if ((value*sign) >= low) {
                        *pvalue = value*sign;
                        return(TRUE);
                }
    }
    return(FALSE);
}


/* ------------------------------------------------------------ */

boolean match_partial_keyword (parseinfo *csb, const char *keyword,
                                                           int *pi, const char *help,
                                                           int minmatch, uint flags, uint priv)
{
    int i;
    char *buf = csb->line + csb->line_index + *pi;
        int old_line_index = 0;
        char *bufptr = NULL;

        if ( ( bufptr = strchr(buf, ' ') ) == NULL )
                old_line_index = strlen(buf) - 1;
        else
                old_line_index = ( bufptr - buf ) - 1;

#ifdef  DEBUG_KEYWORD
    printf("\nmatch_partial_keyword: Entering");
#endif  DEBUG_KEYWORD
    csb->line_index += *pi;
    if (long_help_check(csb)) {
                if (flags & KEYWORD_HELP_CHECK) {
                        save_help_long(csb, keyword, help, priv);
                }
#ifdef  DEBUG_KEYWORD
                printf("\nmatch_partial_keyword: Long help");
#endif  DEBUG_KEYWORD
                csb->line_index -= *pi;
                return(FALSE);
    }
    csb->line_index -= *pi;
        
    for (i = 0; (toupper(buf[i]) == toupper(keyword[i])) && buf[i]; i++) {
                ;                                                               /* Null body */
    }
    (*pi) += i;
#ifdef  DEBUG_KEYWORD
    printf("\nmatch_partial_keyword: Matched %d letters of keyword %s", i, keyword);
#endif  DEBUG_KEYWORD
    if (i > 0) {
                if ((!minmatch || (i >= minmatch)) &&
                        (((flags & KEYWORD_WS_OK) &&
                          match_whitespace2(csb->line + csb->line_index, pi, csb->in_help))
                         || (flags & KEYWORD_NO_WS_OK && old_line_index <= i))) {

#ifdef  DEBUG_KEYWORD
                        printf("\nmatch_partial_keyword: Matched whitespace");
#endif  DEBUG_KEYWORD
                        csb->line_index += *pi;
                        if ((flags & KEYWORD_NO_WS_OK) && (short_help_check(csb))) {
                                if (flags & KEYWORD_HELP_CHECK) {
                                        save_help_short(csb, keyword, SUPPORT_COMMAND_COMPLETION);
                                }
                                csb->line_index -= *pi;
                                return(FALSE);
                        }
                        if (parser_ambig_debug) {
                                printf("\nMatched keyword '%s'", keyword);
                                printf("\ncsb->command_visible = %d", csb->command_visible);
                        }
                        save_ambig_string(csb, keyword, keyword);
                        csb->line_index -= *pi;
                        csb->flags |= KEYWORD_MATCHED;
                        return(TRUE);

                } else {
#ifdef  DEBUG_KEYWORD
                        printf("\nmatch_partial_keyword: Didn't match whitespace");
#endif  DEBUG_KEYWORD
                        if ((flags & KEYWORD_HELP_CHECK) && (flags & KEYWORD_WS_OK)) {
                                help_or_error(csb, *pi, keyword, SUPPORT_COMMAND_COMPLETION);
                        } else {
                                csb->line_index += *pi;
                                set_error_index(csb);
                                csb->line_index -= *pi;
                        }
                }
    } else {
                /* didn't match any characters, set error index */
                csb->line_index += *pi;
                set_error_index(csb);
                csb->line_index -= *pi;
    }
    return(FALSE);
}

static void string_escape (char **from, char **to, uint *i)
{
    uint count;
    uint value;
    char *pfrom = *from;
    uint pi = *i;
    
    switch (**from) {
        default:
                /* Put back backslash */
                **to = '\\';
                (*to)++;
                if (!*from) {
                        break;
                }
                /* Fall through */
        case '"':
                **to = **from;
                (*to)++;
                (*from)++;
                (*i)++;
                break;
        case 'x':
                pfrom++;
                pi++;
                count = 0;
                value = 0;
                while ((count < 2) && *pfrom && isxdigit(*pfrom)) {
                        value = (value*16) + CVHTOB(*pfrom);
                        pfrom++;
                        pi++;
                        count++;
                }
                if (count != 2) {
                        **to = '\\';
                        (*to)++;
                        **to = **from;
                        (*from)++;
                        (*i)++;
                } else {
                        **to = value;
                        *from = pfrom;
                        *i = pi;
                }
                (*to)++;
                break;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
                value = 0;
                count = 0;
                while ((count < 3) && *pfrom &&
                           (*pfrom >= '0') && (*pfrom <= '7')) {
                        value = (value*8) + (*pfrom - '0');
                        pfrom++;
                        pi++;   
                        count++;
                }
                if (count != 3) {
                        **to = '\\';
                        (*to)++;
                        **to = **from;
                        (*from)++;
                        (*i)++;
                } else {
                        **to = value;
                        *from = pfrom;
                        *i = pi;
                }
                (*to)++;
                break;
    }
}
/*
 * Copies at most <maxchar-1> non-whitespace characters from input buffer
 * to output buffer. Returns string length.  If the string is quoted,
 * then it may contain any character, including whitespace.
 */
uint copy_varstring (char *from, char *to, uint maxchar, uint flags)
{
    uint i;                                                             /* The number of characters matched */

    if ((*from == '"') &&
                (flags & STRING_QUOTED) &&
                (!(flags & STRING_WS_OK))) {

                /* Quoted string */
                from++;
                for (i = 1; i < maxchar; i++) {
                        if (*from == '\0') {
                                printf("\nWarning: Assumed end-quote for quoted string");
                                break;
                        }
                        /* If we find a quote, it must be backslash escaped to be kept.
                         * Terminate the string otherwise.
                         */
                        if (*from == '"') {
                                i++;                                    /* advance past trailing quote */
                                i = min(i, (maxchar - 1));
                                *to = '\0';
                                return(i);
                        }
                        if (*from == '\\') {
                                from++;
                                string_escape(&from, &to, &i);
                        } else {
                                *to++ = *from++;
                        }
                }
    } else {
                /* Just get the next token */
                for (i = 0; (i < maxchar) && *from; i++) {
                        if (*from == '\\') {
                                from++;
                                string_escape(&from, &to, &i);
                        } else {
                                if (!(flags & STRING_WS_OK) && isspace(*from)) {
                                        break;
                                }
                                *to++ = *from++;
                        }
                }
    }

    i = min(i, (maxchar - 1));
    *to = '\0';

    return(i);
}

void help_or_error (parseinfo *csb, int count, const char *msg,
                                        boolean do_command_completion)
{
    csb->line_index += count;
    if (help_check(csb)) {
                save_help_short(csb, msg, do_command_completion);
    } else {
                /* Not doing help.  This is an error. */
                set_error_index(csb);
    }
    csb->line_index -= count;
}

static void set_error_index (parseinfo *csb)
{
    if (csb->error_index < csb->line_index) {
                csb->error_index = csb->line_index;
    }
}

static void match_number_hex (char *buf, uint low, uint high,
                                                          int *pi, uint *pvalue, uint *flags)
{
    char c;
    int i = 0;
    uint ret = 0;

    buf += *pi;
    *pvalue = 0;
    *flags &= ~NUMBER_HEX;

#ifdef  DEBUG_NUMBER
    printf("\nmatch_number_hex: Entering");
    printf("\nmatch_number_hex: buf = '%s'", buf);
#endif  DEBUG_NUMBER
    if ((buf[0] == '0') && (toupper(buf[1]) == 'X')) {
                if (*flags & HEX_ZEROX_OK) {
                        /* saw '0x' and it is ok */
                        i += 2;
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_hex: matched '0x'");
#endif  DEBUG_NUMBER
                        ret = HEX_ZEROX_MATCHED;
                } else {
                        /* saw '0x' and it is not ok, return false */
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_hex: matched '0x' and not ok");
#endif  DEBUG_NUMBER
                        return;
                }
    } else {
                if (*flags & HEX_NO_ZEROX_OK) {
                        /* didn't see '0x' and that is ok */
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_hex: didn't match '0x'");
#endif  DEBUG_NUMBER
                        ret = HEX_ZEROX_NOT_MATCHED;
                } else {
                        /* didn't see '0x' and it is not ok, return false */
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_hex: didn't match '0x' and not ok");
#endif  DEBUG_NUMBER
                        return;
                }
    }

#ifdef  DEBUG_NUMBER
    printf("\nmatch_number_hex: trying char '%c'", buf[i]);
#endif  DEBUG_NUMBER
    /* i has already been initialized */
    for (c = toupper(buf[i]); isxdigit(c); i++, c = toupper(buf[i])) {
                if (*pvalue > ULONG_MAX / 16) {
                        /* Too many non-zero digits, max out. */
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_hex: too many chars %d", i);
#endif  DEBUG_NUMBER
                        *pi += i;
                        return;
                }
                *pvalue = *pvalue * 16 + CVHTOB(c);
                if (*pvalue > high) {
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_hex: value > high i = %d", i);
#endif  DEBUG_NUMBER
                        *pi += i;
                        return;
                }
#ifdef  DEBUG_NUMBER
                printf("\nmatch_number_hex: trying char '%c'", buf[i+1]);
#endif  DEBUG_NUMBER
    }

    if (i && (*pvalue >= low)) {
                *pi += i;
                *flags |= ret;
                *flags |= NUMBER_HEX;
#ifdef  DEBUG_NUMBER
                printf("\nmatch_number_hex: matched number 0x%x", *pvalue);
#endif  DEBUG_NUMBER
                return;
    }
#ifdef  DEBUG_NUMBER
    if (!i) {
                printf("\nmatch_number_hex: matched 0 numbers");
    } else {
                printf("\nmatch_number_hex: value < low");
    }
#endif  DEBUG_NUMBER
    return;
}

static void match_number_oct (char *buf, uint low, uint high,
                                                          int *pi, uint *pvalue, uint *flags)
{
    int i = 0;
    uint ret = 0;

    buf += *pi;
    *pvalue = 0;
    *flags &= ~NUMBER_OCT;

#ifdef  DEBUG_NUMBER
    printf("\nmatch_number_oct: Entering");
    printf("\nmatch_number_oct: buf = '%s'", buf);
#endif  DEBUG_NUMBER
    if (buf[0] == '0') {
                if (*flags & OCT_ZERO_OK) {
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_oct: matched '0'");
#endif  DEBUG_NUMBER
                        ret = OCT_ZERO_MATCHED;
                } else {
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_oct: matched '0' and not ok");
#endif  DEBUG_NUMBER
                        return;
                }
    } else {
                if (*flags & OCT_NO_ZERO_OK) {
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_oct: didn't match '0'");
#endif  DEBUG_NUMBER
                        ret = OCT_ZERO_NOT_MATCHED;
                } else {
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_oct: didn't match '0' and not ok");
#endif  DEBUG_NUMBER
                        return;
                }
    }

#ifdef  DEBUG_NUMBER
    printf("\nmatch_number_oct: trying char '%c'", buf[i]);
#endif  DEBUG_NUMBER
    for (i=0; isodigit(buf[i]); i++) {
                /* Make sure we don't have a 33-rd bit */
                if (*pvalue > ULONG_MAX / 8) {
                        *pi += i;
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_oct: too many chars %d", i);
#endif  DEBUG_NUMBER
                        return;
                }
                *pvalue = 8 * *pvalue + buf[i] - '0';
                if (*pvalue > high) {
                        *pi += i;
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_oct: value > high  i = %d", i);
#endif  DEBUG_NUMBER
                        return;
                }
#ifdef  DEBUG_NUMBER
                printf("\nmatch_number_oct: trying char '%c'", buf[i+1]);
#endif  DEBUG_NUMBER
    }
    if (i && (*pvalue >= low)) {
                *pi += i;
                *flags |= ret;
                *flags |= NUMBER_OCT;
#ifdef  DEBUG_NUMBER
                printf("\nmatch_number_oct: matched num 0%o", *pvalue);
#endif  DEBUG_NUMBER
                return;
    }
#ifdef  DEBUG_NUMBER
    if (!i) {
                printf("\nmatch_number_oct: matched 0 numbers");
    } else {
                printf("\nmatch_number_oct: value < low");
    }
#endif  DEBUG_NUMBER
    return;
}

static void match_number_dec (char *buf, uint low, uint high,
                                                          int *pi, uint *pvalue, uint *flags)
{
    int i = 0;

    buf += *pi;
    *pvalue = 0;
    *flags &= ~NUMBER_DEC;

#ifdef  DEBUG_NUMBER
    printf("\nmatch_number_dec: Entering");
    printf("\nmatch_number_dec: buf = '%s'", buf);
    printf("\nmatch_number_dec: trying char '%c'", buf[i]);
#endif  DEBUG_NUMBER
    for (i=0; isdigit(buf[i]); i++) {
                if ((*pvalue > ULONG_MAX / 10) ||
                        ((*pvalue == ULONG_MAX / 10) && ((buf[i] - '0') > ULONG_MAX % 10))) {
                        /* Too many chars, max out. */
                        *pi += i;
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_dec: too many chars %d", i);
#endif  DEBUG_NUMBER
                        return;
                }
                *pvalue = *pvalue * 10 + (uint) (buf[i] - '0');
                if (*pvalue > high) {
                        *pi += i;
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number_dec: value > high  i = %d", i);
#endif  DEBUG_NUMBER
                        return;
                }
#ifdef  DEBUG_NUMBER
                printf("\nmatch_number_dec: trying char '%c'", buf[i+1]);
#endif  DEBUG_NUMBER
    }

    if (i && (*pvalue >= low)) {
                *pi += i;
                *flags |= NUMBER_DEC;
#ifdef  DEBUG_NUMBER
                printf("\nmatch_number_dec: matched number '%d", *pvalue);
#endif  DEBUG_NUMBER
                return;
    }
#ifdef  DEBUG_NUMBER
    if (!i) {
                printf("\nmatch_number_dec: matched 0 numbers");
    } else {
                printf("\nmatch_number_dec: value < low");
    }
#endif  DEBUG_NUMBER
    return;
}

boolean match_number (parseinfo *csb, uint low, uint high, int *pi,
                                          uint *pvalue, uint flags, const char *long_help)
{
    char *buf;
    uint hex_num, oct_num, dec_num;
    int hex_pi, oct_pi, dec_pi;
    char short_help[SHORTHELP_LEN];
    boolean matched;

    buf = csb->line + csb->line_index;

    /*
     * Construct short help message, in case we need it
     */
    general_number_short_help_http(&flags, low, high, short_help,
                                                                   (csb->flags & CONFIG_HTTP));

    /* Do initial, long help check */
    csb->line_index += *pi;                             /* Needed for help_check */
    if ((flags & NUMBER_HELP_CHECK)  && help_check(csb)) {
      
                save_help_long(csb, short_help, long_help, csb->priv);
                csb->line_index -= *pi;
                return(FALSE);
    }
    csb->line_index -= *pi;

    hex_pi = oct_pi = dec_pi = *pi;

#ifdef  DEBUG_NUMBER
    printf("\nmatch_number: entering");
    printf("\nmatch_number: buf = '%s'", buf);
    printf("\nmatch_number: buf = 0x%x  pi = %d  buf[0] = '%c'", buf, *pi, buf[0]);
#endif  DEBUG_NUMBER
    if (flags & NUMBER_HEX) {
                match_number_hex(buf, low, high, &hex_pi, &hex_num, &flags);
    }
    if (flags & NUMBER_OCT) {
                match_number_oct(buf, low, high, &oct_pi, &oct_num, &flags);
    }
    if (flags & NUMBER_DEC) {
                match_number_dec(buf, low, high, &dec_pi, &dec_num, &flags);
    }

#define MATCH_NUMBER_SET(val, index, ret)\
    *pvalue = val;\
        *pi = index;\
        matched = ret

#define MAX3(n1, n2, n3)\
                        ((n1 > n2) ? ((n1 > n3) ? n1 : n3) : ((n2 > n3) ? n2 : n3))

#ifdef  DEBUG_NUMBER
                printf("\nmatch_number: Dec matched %d chars", dec_pi);
                printf("\nmatch_number: Oct matched %d chars", oct_pi);
                printf("\nmatch_number: Hex matched %d chars", hex_pi);
#endif  DEBUG_NUMBER

                 switch (flags & (NUMBER_HEX|NUMBER_OCT|NUMBER_DEC)) {
                 default:
                 case 0: /* No Matches */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: No matches");
#endif  DEBUG_NUMBER
                         MATCH_NUMBER_SET(0, MAX3(hex_pi, oct_pi, dec_pi), FALSE);
                         break;

                 case NUMBER_HEX: /* Hex Only */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched hex");
                         printf("\nmatch_number: Returning hex");
#endif  DEBUG_NUMBER
                         MATCH_NUMBER_SET(hex_num, hex_pi, TRUE);
                         break;

                 case NUMBER_OCT: /* Octal Only */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched oct");
                         printf("\nmatch_number: Returning oct");
#endif  DEBUG_NUMBER
                         MATCH_NUMBER_SET(oct_num, oct_pi, TRUE);
                         break;

                 case NUMBER_DEC: /* Decimal Only */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched decimal");
                         printf("\nmatch_number: Returning decimal");
#endif  DEBUG_NUMBER
                         MATCH_NUMBER_SET(dec_num, dec_pi, TRUE);
                         break;

                 case NUMBER_HEX | NUMBER_OCT: /* Hex and Octal */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched hex and octal");
#endif  DEBUG_NUMBER
                         if (flags & HEX_ZEROX_MATCHED) {
                                 /* This case is impossible unless whitespace is optional */
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning hex");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(hex_num, hex_pi, TRUE);
                         } else if (flags & OCT_ZERO_MATCHED) {
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning oct");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(oct_num, oct_pi, TRUE);
                         } else {
                                 /*
                                  * We matched a hex and octal, but didn't get a '0' or '0x',
                                  * so we have no idea which one it could be, so I'm going
                                  * to guess an Octal.  If anyone wants to change this,
                                  * go right ahead.
                                  */    
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning oct");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(oct_num, oct_pi, TRUE);
                         }
                         break;

                 case NUMBER_HEX | NUMBER_DEC: /* Hex and Decimal */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched hex and decimal");
#endif  DEBUG_NUMBER
                         if (flags & HEX_ZEROX_MATCHED) {
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning hex");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(hex_num, hex_pi, TRUE);
                         } else {
                                 /*
                                  * Pretty simple.
                                  */
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning decimal");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(dec_num, dec_pi, TRUE);
                         }
                         break;

                 case NUMBER_OCT | NUMBER_DEC: /* Octal and Decimal */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched octal and decimal");
#endif  DEBUG_NUMBER
                         if (flags & OCT_ZERO_MATCHED) {
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning oct");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(oct_num, oct_pi, TRUE);
                         } else {
                                 /*
                                  * Straightforward
                                  */    
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning decimal");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(dec_num, dec_pi, TRUE);
                         }
                         break;

                 case NUMBER_HEX | NUMBER_OCT | NUMBER_DEC: /* All three */
#ifdef  DEBUG_NUMBER
                         printf("\nmatch_number: Matched hex, octal, and decimal");
#endif  DEBUG_NUMBER
                         if (flags & HEX_ZEROX_MATCHED) {
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning hex");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(hex_num, hex_pi, TRUE);
                         } else if (flags & OCT_ZERO_MATCHED) {
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning oct");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(oct_num, oct_pi, TRUE);
                         } else {
#ifdef  DEBUG_NUMBER
                                 printf("\nmatch_number: Returning decimal");
#endif  DEBUG_NUMBER
                                 MATCH_NUMBER_SET(dec_num, dec_pi, TRUE);
                         }
                         break;
                 }

        if (matched) {
                if (((flags & NUMBER_WS_OK) &&
                         match_whitespace2(buf, pi, csb->in_help)) ||
                        (flags & NUMBER_NO_WS_OK)) {
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number: Matched whitespace");
#endif  DEBUG_NUMBER
                        save_ambig_uint(csb, short_help, *pvalue);
                        return(TRUE);
                } else {
                 /*
                  * We didn't match whitespace, so throw the matches
                  * out the window and return the max number of 
                  * characters matched
                  */
                        *pi = MAX3(hex_pi, oct_pi, dec_pi);
#ifdef  DEBUG_NUMBER
                        printf("\nmatch_number: Didn't match whitespace");
#endif  DEBUG_NUMBER
                }
        }
        if (flags & NUMBER_HELP_CHECK) {
                help_or_error(csb, *pi, short_help, NO_COMMAND_COMPLETION);
        } else {
                csb->line_index += *pi;
                set_error_index(csb);
                csb->line_index -= *pi;
        }
        return(FALSE);
}


void generic_addr_noflag_action (parseinfo *csb, transition *mine,
                                                                 addrparse_nf_struct * const arg)
{
    addrparse_struct out_arg = {arg->offset, arg->help, arg->type, 0};

    generic_addr_action(csb, mine, &out_arg);
}


static boolean match_general_addr (parseinfo *csb, int *pi, void *addrp,
                                                                   uint addr_type, uint flag,
                                                                   const char *long_help)
{
    char *short_help = NULL;
    match_gen_func func = NULL;
    int i;
    void *idb = NULL;

    for (i=0; addr_funcs[i].addr_type != ADDR_ILLEGAL; i++) {
                if (addr_type == addr_funcs[i].addr_type) {
                        short_help = addr_funcs[i].get_short_help(csb, flag, idb);
                        func = addr_funcs[i].func;
                        break;
                }
    }

    if (addr_funcs[i].addr_type == ADDR_ILLEGAL) {
                /* No sir, I don't like it */
                return(FALSE);
    }

    if (help_check(csb)) {
            save_help_long(csb, short_help, long_help, csb->priv);
                return(FALSE);
    }

    if ((*func)(csb, &csb->line[csb->line_index],pi,addrp,
                                csb->in_help, flag, idb)) {
                i = *pi;
                if (match_whitespace2(&csb->line[csb->line_index], pi, csb->in_help)) {
                        save_ambig_nstring(csb, short_help,
                                                           &(csb->line[csb->line_index]), i);
                        return(TRUE);
                }
    }
    ((addrtype *)addrp)->type = 0;
    ((addrtype *)addrp)->length = 0;
    if (!(flag & ADDR_NO_SHORT_HELP)) {
                help_or_error(csb, *pi, short_help, NO_COMMAND_COMPLETION);
    }
    return(FALSE);
}



void generic_addr_action (parseinfo *csb, transition *mine,
                                                  addrparse_struct * const arg)
{
    void *addrp;
    int i;
    
    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    addrp = CSBVAR(csb, arg->offset, void);

    i = 0;
    if (match_general_addr(csb, &i, addrp, arg->type, arg->flag, arg->help)) {
                csb->line_index += i;
                push_node(csb, mine->accept);
                csb->line_index -= i;

    }
}

void multiple_funcs_action (parseinfo *csb, transition *mine,
                                                        test_struct * const arg)
{
    push_node(csb, mine->alternate);
    if (test_multiple_funcs(csb, '=', 0)) {
                push_node(csb, arg->equal);
    } else {
                push_node(csb, arg->notequal);
    }
}

void save_ambig_string (parseinfo *csb, const char *help, const char *str)
{
    save_ambig_string_flag(csb, help, str, TRUE);
}

static void save_ambig_nstring (parseinfo *csb, const char *help,
                                                                const char *str, uint size)
{
    char *tmp = NULL;
    uint length;

    if (csb->flags & COMMAND_AUTHORIZE) {
                length =  strlen(str);
                if (length > size) {
                        length = size;
                }
                tmp = (char *)PARSERmalloc(length + 1);
                if (tmp) {
                        strncpy(tmp, str, length+1);
                }
    } else {
                tmp = NULL;
    }

    save_ambig_string_flag(csb, help, tmp, TRUE);
    if (tmp) {
                PARSERfree(tmp);
    }
}

static void save_ambig_int (parseinfo *csb, const char *help, int num)
{
    char str[12];

    sprintf(str, "%d", num);
    save_ambig_string_flag(csb, help, str, TRUE);
}

static void save_ambig_uint (parseinfo *csb, const char *help, uint num)
{
    char str[12];

    sprintf(str, "%u", num);
    save_ambig_string_flag(csb, help, str, TRUE);
}

static void save_ambig_string_flag (parseinfo *csb, const char *help,
                                                                        const char *ambig_string, boolean space)
{
    ambig_struct *ap;
    char *str;
    uint new_length = strlen(help) + (space ? 1 : 0);
    uint old_length;

    if (csb->command_visible) {
                ap = &csb->visible_ambig;
    } else {
                ap = &csb->hidden_ambig;
    }

    if (parser_ambig_debug) {
                printf("\nsaving string '%s' in ambig buffer '%s'", help, ap->ambig);
    }

    old_length = strlen(ap->ambig);
    str = ap->ambig + old_length;
    if ((ap->ambig != str) && (*(str-1) == (char)-1)) {
                /* Already marked as too big, so skip */
    } else {
                if ((new_length + old_length + 2) >= PARSEBUF) {
                        /* Too big, mark as too big and skip */
                        if ((old_length + 2) >= PARSEBUF) {
                                str = ap->ambig + PARSEBUF - 2;
                        }
                        *str++ = -1;
                        *str = '\0';
                } else {
                        strcat(str, help);
                        if (space) {
                                strcat(str, " ");
                        }
                }
                if (csb->flags & COMMAND_AUTHORIZE) {
                        old_length = strlen(csb->nv_command);
                        new_length = strlen(ambig_string);
                        if ((new_length + old_length + 2) < PARSEBUF) {
                                strcat(csb->nv_command, ambig_string);
                                if (space) {
                                        strcat(csb->nv_command, " ");
                                }
                        }
                }
    }
}

static void save_help_msg (parseinfo *csb, const char *help)
{
    if (csb->in_help == PARSER_HELP) {
                save_line(csb, &csb->help_save, "%s", help);
    }
}

static void save_help_short (parseinfo *csb, const char *help,
                                                         boolean do_command_completion)
{
    if ((csb->in_help == PARSER_HELP) ||
                (do_command_completion == SUPPORT_COMMAND_COMPLETION)) {
                save_line(csb, &csb->help_save, "%s\t", help);
    }
}

void save_help_long (parseinfo *csb, const char *short_help,
                                         const char *long_help, uint priv)
{
    if (csb->in_help == PARSER_HELP) {
            save_line(csb, &csb->help_save, "  %s", short_help);
            if (long_help) {
                        save_line(csb, &csb->help_save, "\t%s", long_help);
            }
                save_line(csb, &csb->help_save, "\n");
    }
}


static void parser_bail_out (parseinfo *csb, void (*func)(parseinfo *),
                                                         const char *func_name)
{
        if ( func ) 
                func(csb);
}


void priv_action (parseinfo *csb, transition *mine, priv_struct * const arg)
{
    push_node(csb, mine->alternate);
    if (priv_check(csb, arg->priv)) {
                if (csb->priv_set) {
                        priv_push(csb, arg->priv);
                }
                csb->last_priv = arg->priv->cur_priv;
                if (csb->last_priv > csb->highest_priv) {
                        csb->highest_priv = csb->last_priv;
                }
                push_node(csb, mine->accept);
    } else {
                push_node(csb, arg->fail);
    }
}

void month_action (parseinfo *csb, transition *mine,
                                   month_struct * const arg)
{
    int *var = CSBVAR(csb, arg->offset, int);
    int i;
    static const char *short_help = "MONTH";

    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (help_check(csb)) {
                save_help_long(csb, short_help, arg->help, csb->priv);
                return;
    }

    for (i=0; i < MONTHS_IN_YEAR; i++) {
                int j = 0;
                if (match_partial_keyword(csb, long_month_name[i], &j, NULL, 3, 
                                                                  KEYWORD_WS_OK, csb->priv)) {
                        csb->line_index += j;
                        *var = i + 1;
                        push_node(csb, mine->accept);
                        csb->line_index -= j;
                        break;
                }
    }
}


void day_action (parseinfo *csb, transition *mine,
                                 day_struct * const arg)
{
    int *var = CSBVAR(csb, arg->offset, int);
    int i;
    static const char *short_help = "DAY";

    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (help_check(csb)) {
                save_help_long(csb, short_help, arg->help, csb->priv);
                return;
    }

    for (i=0; i < DAYS_IN_WEEK + 1; i++) {
                int j = 0;
                if (match_partial_keyword(csb, long_day_name[i], &j, NULL, 3, 
                                                                  KEYWORD_WS_OK, csb->priv)) {
                        csb->line_index += j;
                        *var = i;
                        push_node(csb, mine->accept);
                        csb->line_index -= j;
                        break;
                }
    }
}


void timesec_action (parseinfo *csb, transition *mine, timesec_struct *arg)
{
    uint *var1 = CSBVAR(csb, arg->var1, uint);
    uint *var2 = CSBVAR(csb, arg->var2, uint);
    uint *var3;
    char *short_help;
    uint i;
    static uint flags = NUMBER_DEC | NUMBER_NO_WS_OK;

    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (arg->var3 == MAXUINT) {
                short_help = "hh:mm";
                var3 = 0;
    } else {
                short_help = "hh:mm:ss";
                var3 = CSBVAR(csb, arg->var3, uint);
    }

    if (help_check(csb)) {
                save_help_long(csb, short_help, arg->help, csb->priv);
    }

    i = 0;
    if (match_number(csb, 0, 23, &i, var1, flags, NULL) &&
                match_char(&csb->line[csb->line_index], &i, ':') &&
                match_number(csb, 0, 59, &i, var2, flags, NULL) &&
                ((arg->var3 == MAXUINT) ||
                 (match_char(&csb->line[csb->line_index], &i, ':') &&
                  match_number(csb, 0, 59, &i, var3, flags, NULL))) &&
                match_whitespace2(&csb->line[csb->line_index], &i, csb->in_help)) {
                csb->line_index += i;
                push_node(csb, mine->accept);
                csb->line_index -= i;
                return;
    }

    if (i) {
                help_or_error(csb, i, short_help, NO_COMMAND_COMPLETION);
    }
}


void comment_action (parseinfo *csb, transition *mine)
{
    push_node(csb, mine->accept);
    if (csb->nvgen && !csb->priv_set) {
      /* 
       *RAMS: commented this for now
       
       nv_write(TRUE, "!"); */
    }
}

void testexpr_action (parseinfo *csb, transition *mine, testexpr_struct * const arg)
{
    /* always push the alternate */
    push_node(csb, mine->alternate);

    if (csb->priv_set || (csb->flags & COMMAND_FUNCTION)) {
                /*
                 * Parser state may not be what test expression expects,
                 * so avoid testing the expression.
                 */
                push_node(csb, mine->accept);
                push_node(csb, arg->fal);
    } else {
                if (arg->func(csb, mine)) {
                        push_node(csb, mine->accept);
                } else {
                        push_node(csb, arg->fal);
                }
    }
}




void keyword_option_action (parseinfo *csb,
                                                        transition *mine,
                                                        keyword_option_struct * const arg)
{
    uint i;
    uint o;
    uint *var = CSBVAR(csb, arg->offset, uint);
    boolean matched;
    uint flags = KEYWORD_HELP_CHECK | KEYWORD_WS_OK;

    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (priv_check(csb, arg->priv) == FALSE) {
                return;
    }

    keyword_flag_modify(csb, arg->priv, &flags);

    o = 0;
    matched = FALSE;
    while (arg->options[o].keyword && !matched) {
                if (arg->flags & KEYWORD_TEST) {
                        if (arg->flags & KEYWORD_OR_SET) {
                                if (*var & arg->options[o].val) {
                                        o++;
                                        continue;
                                }
                        } else {
                                if (*var) {
                                        o++;
                                        continue;
                                }
                        }
                }
                i = 0;
                if (match_partial_keyword(csb, arg->options[o].keyword, &i,
                                                                  arg->options[o].help, 0, flags,
                                                                  arg->priv->cur_priv)) {
                        csb->line_index += i;
                        if (csb->priv_set) {
                                priv_push(csb, arg->priv); /* change priv level */
                        }
                        csb->last_priv = arg->priv->cur_priv;
                        if (csb->last_priv > csb->highest_priv) {
                                csb->highest_priv = csb->last_priv;
                        }
                        if (arg->offset != -1) {
                                if (arg->flags & KEYWORD_OR_SET) {
                                        *var |= arg->options[o].val;
                                } else {
                                        *var = arg->options[o].val;
                                }
                        }
                        push_node(csb, mine->accept);
                        csb->line_index -= i;
                        matched = TRUE;
                } else {
                        if (parser_ambig_debug) {
                                printf("\nDidn't match keyword '%s'", arg->options[o].keyword);
                        }
                }
                o++;
    }
}







#ifdef IOS_PARSER



/* WHITESPACE matches at least one whitespace char, maybe more
 * EOL is acceptable as whitespace.
 */

void whitespace_action (parseinfo *csb, transition *mine)
{
    int i;

    push_node(csb, mine->alternate);
    if (csb->nvgen) {
                /*
                 * Don't add whitespace here.  This function is often called between
                 * tokens which don't generate any output.
                 */
                push_node(csb, mine->accept);

                return;
    }
    i = 0;

    if (match_whitespace2(&csb->line[csb->line_index], &i, csb->in_help)) {
                csb->line_index += i;
                push_node(csb, mine->accept);
                csb->line_index -= i;
    }
    /* No point to setting error_index: if the above fails we haven't
           scanned any whitespace successfully. */
}


/* PRINT always matches and prints its string, followed by a newline */

void print_action (parseinfo *csb, transition *mine,
                                   print_struct * const arg)
{
    push_node(csb, mine->accept);
    if (csb->nvgen) {
                return;
    }

    if (arg->str) {
                printf(arg->str);
    }
}

/* Display places its message into the help buffer no matter what.
 * It's used mostly in cases where we've decided to discard the rest of
 * the input line and produce a generic help/error message.
 * THIS FUNCTION PRODUCES HELP WHEN in_help IS NOT SET.
 */

void display_action (parseinfo *csb, transition *mine,
                                         help_struct * const arg)
{
    save_help_msg(csb, arg->str);
    push_node(csb, mine->alternate);
    return;
}


/* CHARACTER matches a single character */

void char_action (parseinfo *csb, transition *mine,
                                  char_struct * const arg)
{
    char ch[2];

    push_node(csb, mine->alternate);

    /* Long help is the responsibility of the calling macro. */

    INVALID_NVGEN_CHECK;

    if (csb->line[csb->line_index] == arg->character) {

                ch[0] = arg->character;
                ch[1] = '\0';
                if (parser_ambig_debug) {
                        printf("\nMatched character '%c'", arg->character);
                        printf("\ncsb->command_visible = %d", csb->command_visible);
                }
                save_ambig_string(csb, ch, ch);

                csb->line_index++;
                push_node(csb, mine->accept);
                csb->line_index--;
    }

    /*
     * Note that short help check and error processing
     * are NOT appropriate here.
     */
}


/*
 * CHAR_NUMBER takes either a character or an integer value
 * representing a character
 */
void char_number_action (parseinfo *csb, transition *mine, 
                                                 number_struct * const arg)
{
    char short_help[32];
    int i = 0;
    uint num;
    uint lower;
    uint upper;

    push_node(csb, mine->alternate);
    INVALID_NVGEN_CHECK;

    CHECK_RANGE_BOUNDS();

    sprintf(short_help, "CHAR or <%d-%d>", lower, upper);

    if (help_check(csb)) {
                save_help_long(csb, short_help, arg->help, csb->priv);
                return;
    }

    /* Try to match the number first */
    num = 0;
    if (! match_number(csb, lower, upper, &i, &num,
                                           (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_OCT | OCT_ZERO_OK |
                                                NUMBER_DEC | NUMBER_NO_WS_OK | NUMBER_HELP_CHECK),
                                           NULL)) {

                /* No number, try a character */
                if (csb->line[csb->line_index] &&
                        !isspace(csb->line[csb->line_index])) {

                        /* We have a Non-null character, try accepting it */
                        num = (int) csb->line[csb->line_index];
                        i = 1;
                }
    }

    if (i) {
                if (match_whitespace2((csb->line + csb->line_index),
                                                          &i, csb->in_help)) {

                        *CSBVAR(csb, arg->offset, uint) = num;

                        csb->line_index += i;
                        push_node(csb, mine->accept);
                        csb->line_index -= i;
            
                        return;
                }
    }
    help_or_error(csb, i, short_help, NO_COMMAND_COMPLETION);
}

/* 
 * FUNC calls the named function if we're doing NV generation, or
 * saves the * pointer to the function and
 * the current arguments strucuture if we're
 * consuming input.
 *
 * If we're consuming, try the accepting chain, then check csb->multiple_funcs.
 * If it changed, then EOL was encountered and we leave the argument struct
 * on the parsedata list.  Otherwise, we didn't see an EOL, so remove the
 * structure from the list.  This ensures that the only structures in
 * the list are those corresponding to FUNCs along the chain leading to
 * an accepted EOL.
 *
 * If we're doing NV generation, call the function directly.  This allows the
 * function at the end of commands which are only keyword based to to
 * automatically perform NV generation without having to explicitly include an
 * NVGEN() transition in the command chain definition.
 */

void func_action (parseinfo *csb, transition *mine, void (*func) (parseinfo *))
{
    INVALID_NVGEN_CHECK;

    if (!csb->in_help && func) {
                (void) func(csb);
    }
    push_node(csb, mine->accept);
}

void test_boolean_action (parseinfo *csb, transition *mine,
                                                  test_bool_struct * const arg)
{
    push_node(csb, mine->alternate);
    if (*(arg->var)) {
                push_node(csb, arg->nonzero);
    } else {
                push_node(csb, arg->zero);
    }
}

void test_int_action (parseinfo *csb, transition *mine, 
                                          test_int_struct * const arg)
{
    push_node(csb, mine->alternate);
    if (*(arg->var)) {
                push_node(csb, arg->nonzero);
    } else {
                push_node(csb, arg->zero);
    }
}

void test_func_action (parseinfo *csb, transition *mine,
                                           test_func_struct * const arg)
{
    push_node(csb, mine->alternate);
    if (arg->func()) {
                push_node(csb, arg->nonzero);
    } else {
                push_node(csb, arg->zero);
    }
}

void keyword_nows_action (parseinfo *csb, transition *mine,
                                                  keyword_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, -1, 0, 0,
                (KEYWORD_NO_WS_OK | KEYWORD_HELP_CHECK)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_id_mm_action (parseinfo *csb, transition *mine,
                                                   keyword_id_mm_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, arg->offset, arg->val, arg->minmatch,
                (KEYWORD_WS_OK | KEYWORD_HELP_CHECK)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_trans_action (parseinfo *csb, transition *mine,
                                                   keyword_struct * const arg)
{
    if (mine && mine->accept && mine->accept->accept &&
                (mine->accept->accept->alternate != &pname(link_trans))) {
                keyword_action(csb, mine, arg);
    } else {
                push_node(csb, mine->alternate);
    }
}

void keyword_ortest_action (parseinfo *csb, transition *mine,
                                                        keyword_id_struct * const arg)
{
    general_keyword_struct argout = {
                arg->str, arg->help, arg->priv, arg->offset, arg->val, 0,
                (KEYWORD_WS_OK | KEYWORD_HELP_CHECK | KEYWORD_OR_SET | KEYWORD_TEST)};
    general_keyword_action(csb, mine, &argout);
}

void keyword_option_action (parseinfo *csb,
                                                        transition *mine,
                                                        keyword_option_struct * const arg)
{
    uint i;
    uint o;
    uint *var = CSBVAR(csb, arg->offset, uint);
    boolean matched;
    uint flags = KEYWORD_HELP_CHECK | KEYWORD_WS_OK;

    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (priv_check(csb, arg->priv) == FALSE) {
                return;
    }

    keyword_flag_modify(csb, arg->priv, &flags);

    o = 0;
    matched = FALSE;
    while (arg->options[o].keyword && !matched) {
                if (arg->flags & KEYWORD_TEST) {
                        if (arg->flags & KEYWORD_OR_SET) {
                                if (*var & arg->options[o].val) {
                                        o++;
                                        continue;
                                }
                        } else {
                                if (*var) {
                                        o++;
                                        continue;
                                }
                        }
                }
                i = 0;
                if (match_partial_keyword(csb, arg->options[o].keyword, &i,
                                                                  arg->options[o].help, 0, flags,
                                                                  arg->priv->cur_priv)) {
                        csb->line_index += i;
                        if (csb->priv_set) {
                                priv_push(csb, arg->priv); /* change priv level */
                        }
                        csb->last_priv = arg->priv->cur_priv;
                        if (csb->last_priv > csb->highest_priv) {
                                csb->highest_priv = csb->last_priv;
                        }
                        if (arg->offset != -1) {
                                if (arg->flags & KEYWORD_OR_SET) {
                                        *var |= arg->options[o].val;
                                } else {
                                        *var = arg->options[o].val;
                                }
                        }
                        push_node(csb, mine->accept);
                        csb->line_index -= i;
                        matched = TRUE;
                } else {
                        if (parser_ambig_debug) {
                                printf("\nDidn't match keyword '%s'", arg->options[o].keyword);
                        }
                }
                o++;
    }
}

void number_func_action (parseinfo *csb, transition *mine,
                                                 number_func_struct * const arg)
{
    dynamic_number_struct new_arg =
                {
                        arg->offset, 0, -1, arg->help,
                        (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_OCT | OCT_ZERO_OK |
                         NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK)
                };

    if (!csb->priv_set &&
                !(csb->flags & COMMAND_FUNCTION) &&
                arg->func) {
                arg->func(csb, &(new_arg.lower), &(new_arg.upper));
    }
    general_number_action(csb, mine, &new_arg);
}

void number_help_func_action (parseinfo *csb, transition *mine,
                                                          number_help_func_struct * const arg)
{
        dynamic_number_struct new_arg =
                {
                        arg->offset, 0, -1, "",
                        (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_OCT | OCT_ZERO_OK |
                         NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK)
                };
  
        if (!csb->priv_set &&
                !(csb->flags & COMMAND_FUNCTION) &&
                arg->func) {
                arg->func(csb, &new_arg.lower, &new_arg.upper, &new_arg.help);
        }
        general_number_action(csb, mine, &new_arg);
}

void params_action (parseinfo *csb, transition *mine,
                                        params_struct * const arg)
{
    char *cp,
        *cp_save;
    int i = 0,
        k = 0;
    uint num;
    uint lower;
    uint upper;
    uint flags = KEYWORD_WS_OK | KEYWORD_HELP_CHECK;

    /* Make sure we process any alternates */
    push_node(csb, mine->alternate);

    /* Check for privilege to use this keyword */
    if (priv_check(csb, arg->priv) == FALSE) {
                return;
    }

    if (csb->nvgen) {
                if (csb->priv_set) {
                        if (nvgen_privilege(csb, arg->priv, arg->str)) {
                                /*
                                 * The next token is a number, but we are currently
                                 * stopping at NVGEN points, which a number is, so stop!
                                 */
                        }
                        return;
                }
                for (cp = csb->nv_command; *cp; cp++) {
                        ;                                                       /* Null body */
                }
                cp_save = cp;
            
                /* Add a space if needed */
                if (cp != csb->nv_command) {
                        *cp++ = ' ';
                        *cp = 0;
                }

                /*
                 * Add the current token and call the command action routine
                 */
                strcpy(cp, arg->str);

                csb->which = arg->subfunc;
                nvgen_action(csb, mine, arg->func);

                /*
                 * Restore the original command line before processing alternates
                 */
                *cp_save = 0;
                return;
    }

    /**********  KEYWORD  **********/
    /*
     * Check to see if the input matches this token. If there is no match,
     * we skip processing this token and try the alternates.
     */
    keyword_flag_modify(csb, arg->priv, &flags);
    if (match_partial_keyword(csb, arg->str, &i, arg->keyhelp, 0,
                                                          flags, arg->priv->cur_priv)) {
                csb->line_index += i;

                if (csb->priv_set) {
                        /*
                         * Modify the current keyword privilege
                         */
                        priv_push(csb, arg->priv);
                }
                csb->last_priv = arg->priv->cur_priv;
                if (csb->last_priv > csb->highest_priv) {
                        csb->highest_priv = csb->last_priv;
                }
                /********  NVGENS   **********/
                csb->which = arg->subfunc;

                if ((csb->sense == FALSE) && (arg->flags & NO_NUMBER)) {
                        /********  NOPREFIX  *********/
                        /*
                         * We've parsed a 'no' prefix and must consume the remainder of
                         * the line, then look for EOL.
                         */
                        for (k = 0; csb->line[csb->line_index + k] != '\n' &&
                                 csb->line[csb->line_index + k]; k++) {
                                /* Null body */
                        }
                        /********  EOLS  ***********/
                        csb->line_index += k;
                        csb->which = arg->subfunc;
                        eol_action(csb, mine, arg->func);
                        csb->line_index -= k;
                        /******  End EOLS  *********/
                        /******** End NOPREFIX ********/
                } else {
                        /********  DECIMAL  **********/
                        k = 0;

                        CHECK_RANGE_BOUNDS();

                        if (match_number(csb, lower, upper, &k, &num,
                                                         arg->flags, arg->varhelp)) {

                                *CSBVAR(csb, arg->offset, uint) = num;

                                /********  EOLS  ***********/
                                csb->line_index += k;
                                csb->which = arg->subfunc;
                                eol_action(csb, mine, arg->func);
                                csb->line_index -= k;
                                /******  End EOLS  *********/
                        } else {
                                /********  NO_ALT  ***********/
                                csb->line_index += k;
                                no_alt_action(csb);
                                csb->line_index -= k;
                                /*********  End NO_ALT    **********/
                        }
                        /*********  End DECIMAL   **********/
                }

                csb->line_index -= i;
                /*********  End NVGENS    **********/
    }
}

void params_action_alist (parseinfo *csb, transition *mine,
                                                  params_struct_alist * const arg)
{
    char *cp,
        *cp_save;
    int i = 0,
        k = 0;
    uint num;
    uint lower, lower2;
    uint upper, upper2;
    uint flags = KEYWORD_WS_OK | KEYWORD_HELP_CHECK;

    /* Make sure we process any alternates */
    push_node(csb, mine->alternate);

    /* Check for privilege to use this keyword */
    if (priv_check(csb, arg->priv) == FALSE) {
                return;
    }

    if (csb->nvgen) {
                if (csb->priv_set) {
                        if (nvgen_privilege(csb, arg->priv, arg->str)) {
                                /*
                                 * The next token is a number, but we are currently
                                 * stopping at NVGEN points, which a number is, so stop!
                                 */
                        }
                        return;
                }
                for (cp = csb->nv_command; *cp; cp++) {
                        ;                                                       /* Null body */
                }
                cp_save = cp;
            
                /* Add a space if needed */
                if (cp != csb->nv_command) {
                        *cp++ = ' ';
                        *cp = 0;
                }

                /*
                 * Add the current token and call the command action routine
                 */
                strcpy(cp, arg->str);

                csb->which = arg->subfunc;
                nvgen_action(csb, mine, arg->func);

                /*
                 * Restore the original command line before processing alternates
                 */
                *cp_save = 0;
                return;
    }

    /**********  KEYWORD  **********/
    /*
     * Check to see if the input matches this token. If there is no match,
     * we skip processing this token and try the alternates.
     */
    keyword_flag_modify(csb, arg->priv, &flags);
    if (match_partial_keyword(csb, arg->str, &i, arg->keyhelp, 0,
                                                          flags, arg->priv->cur_priv)) {
                csb->line_index += i;

                if (csb->priv_set) {
                        /*
                         * Modify the current keyword privilege
                         */
                        priv_push(csb, arg->priv);
                }
                csb->last_priv = arg->priv->cur_priv;
                if (csb->last_priv > csb->highest_priv) {
                        csb->highest_priv = csb->last_priv;
                }
                /********  NVGENS   **********/
                csb->which = arg->subfunc;

                if ((csb->sense == FALSE) && (arg->flags & NO_NUMBER)) {
                        /********  NOPREFIX  *********/
                        /*
                         * We've parsed a 'no' prefix and must consume the remainder of
                         * the line, then look for EOL.
                         */
                        for (k = 0; csb->line[csb->line_index + k] != '\n' &&
                                 csb->line[csb->line_index + k]; k++) {
                                /* Null body */
                        }
                        /********  EOLS  ***********/
                        csb->line_index += k;
                        csb->which = arg->subfunc;
                        eol_action(csb, mine, arg->func);
                        csb->line_index -= k;
                        /******  End EOLS  *********/
                        /******** End NOPREFIX ********/
                } else {
                        /********  DECIMAL  **********/
                        k = 0;

                        CHECK_RANGE_BOUNDS_ALIST();

                        if (match_number(csb, lower, upper, &k, &num,
                                                         arg->flags, arg->varhelp)) {

                                *CSBVAR(csb, arg->offset, uint) = num;

                                /********  EOLS  ***********/
                                csb->line_index += k;
                                csb->which = arg->subfunc;
                                eol_action(csb, mine, arg->func);
                                csb->line_index -= k;
                                /******  End EOLS  *********/
                        } else if (match_number(csb, lower2, upper2, &k, &num,
                                                                        arg->flags & ~NUMBER_HELP_CHECK,
                                                                        NULL)) {

                                /* Convert to other numbering scheme and save */
                                *CSBVAR(csb, arg->offset, uint) = num + lower - lower2;

                                /********  EOLS  ***********/
                                csb->line_index += k;
                                csb->which = arg->subfunc;
                                eol_action(csb, mine, arg->func);
                                csb->line_index -= k;
                                /******  End EOLS  *********/
                        } else {
                                /********  NO_ALT  ***********/
                                csb->line_index += k;
                                no_alt_action(csb);
                                csb->line_index -= k;
                                /*********  End NO_ALT    **********/
                        }
                        /*********  End DECIMAL   **********/
                }

                csb->line_index -= i;
                /*********  End NVGENS    **********/
    }
}


void hexdata_action (parseinfo *csb, transition *mine,
                                         hexdata_struct * const arg)
{
    int i;
    int stored = 0;
    char short_help[] = "Hex-data";

    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (help_check(csb)) {
                save_help_long(csb, short_help, arg->help, csb->priv);
                return;
    }

    i = match_hex_digits(csb->line + csb->line_index,
                                                 CSBVAR(csb, arg->bufoffset, char),
                                                 arg->bufmax, &stored);
    if (i) {
                (void) match_whitespace2(csb->line + csb->line_index,
                                                                 &i, csb->in_help);
    }

    /*
     * The following catches all help ?'s in the middle of a stream.
     */

    csb->line_index += i;
    if (help_check(csb)) {
                save_help_short(csb, short_help, NO_COMMAND_COMPLETION);
    } else {
                if (stored > 0) {
                        /* success */
                        /* string output stored already */
                        *CSBVAR(csb, arg->numoffset, uint) = stored;
                        push_node(csb, mine->accept);
                } else {
                        set_error_index(csb);
                }
    }
    csb->line_index -= i;
}

/*
 * Like char_action, but deals with characters which might be taken
 * as designating end of line (like '#'), confusing any (prior)
 * help_check full help check. These characters can only be correctly
 * processed by not allowing alternative interpretations !!!
 */
void literal_char_action (parseinfo *csb, transition *mine,
                                                  char_struct2 * const arg)
{
    push_node(csb, mine->alternate);

    INVALID_NVGEN_CHECK;

    if (csb->line[csb->line_index] == arg->character) {
                csb->line_index++;
                push_node(csb, mine->accept);
                csb->line_index--;
    } else {
                /* Can only have help if the character is NOT found. */
                if (help_check(csb)) {
                        save_help_short(csb, arg->help, NO_COMMAND_COMPLETION);
                }
    }
}

static boolean match_spaces (char *buf, int *pi)
{
    int k;

    buf += *pi;
    for (k = 0; isspace(buf[k]); k++) {
                ;                                                               /* Null body */
    }

    *pi += k;

    return((k > 0) ? TRUE : FALSE);
}



/*
 * Converts one or two hex digits from s to a hex number, and stores it in
 * out. Returns pointer to next non-consumed char (NULL if first char is
 * not a hex digit or if it is end of string).
 */
static char *hex_to_byte (char *s, char *out)
{
    int i = 0;

    match_spaces(s, &i);
    s += i;
    *out = 0;

    if ((*s == '\0') || !isxdigit(*s)) {
                return(NULL);
    }

    *out = CVHTOB(*s);
    s++;

    if ((*s != '\0') && isxdigit(*s)) {
                *out = (*out << 4) + CVHTOB(*s);
                s++;
    }
    return(s);
}

/*
 * Recognize single or paired hex digits in buf. Convert each pair to a
 * byte, and store that byte in outbuf. maxlen is the most chars that can
 * be stored in outbuf.
 *
 * RETURNS: number of characters matched. *pchars is the number of chars
 * stored.
 *
 */
static int match_hex_digits (char *buf, char *outbuf, int maxlen, int *pchars)
{
    int i;
    char *s;
    char *ptr = buf;

    for (i = 0; i < maxlen; i++) {
                s = hex_to_byte(ptr, outbuf);
                if (s == NULL) {
                        break;
                }
                ptr = s;
                outbuf++;
    }
    *pchars = i;
    return(ptr - buf);
}

/* ALT_HELP does func call at EOL which
 * prints platform specific help.
 */

void alt_help_action (parseinfo *csb, transition *mine, void (*func) (parseinfo *))
{
        if (csb->in_help) {
                if (help_check(csb)) {
                        if (func) {
                                (void) func(csb);
                                return;
                        }
                }
        }

        no_alt_action(csb);
}

/* ALT_HELPS - does platform specific func call at EOL.  Sets csb->which with
 * the EVAL macro so that 'val' is not constrained to be a constant.
 */
void alt_helps_action (parseinfo *csb, transition *mine,
                                           eols_struct * const arg)
{
    csb->which = arg->subfunc;
    alt_help_action(csb, mine, arg->func);
}


void general_number_short_help (uint *flags, uint lower,
                                                                uint upper, char *help)
{
    general_number_short_help_http(flags, lower, upper, help, FALSE);
}

boolean match_minusone (char *buf, int *pi)
{
    if ((buf[*pi] == '-') && (buf[1 + *pi] == '1')) {
                (*pi) += 2;
                return(TRUE);
    }
    return(FALSE);
}

/*
 * Scan octal as an unsigned int.  Error if too many
 * octal digits
 */
boolean match_octal (char *buf, uint low, uint high, int *pi, ushort * pvalue)
{
    uint i = 0;
    ulong value = 0L;

    /* Fail if there is not at least one octal digit */
    *pvalue = 0L;
    buf += *pi;

    if (!isodigit(buf[i])) {
                return(FALSE);
    }

    for (; isodigit(buf[i]); i++) {
                if (value > USHRT_MAX / 8) {
                        *pi += i;
                        return(FALSE);
                }
                value = 8 * value + buf[i] - '0';
                if (value > high) {
                        *pi += i;
                        return(FALSE);
                }
    }
    if (i && (value >= (ulong) low)) {
                *pvalue = (ushort) value;
                *pi += i;
                return(TRUE);
    }
    return(FALSE);
}

/*
 * ASSUMES: no leading 0x.
 *
 */
boolean match_hexadecimal (char *buf, int *pi, ushort * pvalue)
{
    int i;
    char c;
    ushort value = 0;

    buf += *pi;
    *pvalue = 0;

    for (i = 0, c = toupper(buf[i]); isxdigit(c); i++, c = toupper(buf[i])) {
                if (value > USHRT_MAX / 16) {
                        *pi += i;
                        return(FALSE);
                }
                value = value * 16 + CVHTOB(c);
    }

    if (i > 0) {
                (*pi) += i;
                *pvalue = value;
                return(TRUE);
    }
    return(FALSE);
}


/*
 * Try to match an hexadecimal ulong, with value
 * between (or equal to) <low> and <high>. <*pi> is incremented
 * by the number of characters in the match.
 *
 * ASSUMES: no leading 0x.
 *
 */
boolean match_ulong_hexadecimal (char *buf, ulong low, ulong high,
                                                                 int *pi, ulong *pvalue)
{
    int i;
    ulong value = 0L;
    char c;

    buf += *pi;
    *pvalue = 0L;

    for (i = 0, c = toupper(buf[i]); isxdigit(c); i++, c = toupper(buf[i])) {
                if (value > ULONG_MAX / 16) {
                        /* Too many non-zero digits, max out. */
                        *pi += i;
                        return(FALSE);
                }
                value = value * 16 + CVHTOB(c);
                if (value > high) {
                        *pi += i;
                        return(FALSE);
                }
    }

    if (i && (value >= low)) {
                *pvalue = value;
                *pi += i;
                return(TRUE);
    }
    return(FALSE);
}



/*
 * Scan octal as an unsigned LONG int. Error if too many octal
 * digits.
 */
boolean match_ulong_octal (char *buf, ulong low,
                                                   ulong high, int *pi, ulong *pvalue)
{
    uint i = 0;
    ulong value = 0L;

    /* Fail if there is not at least one octal digit */
    *pvalue = 0L;
    buf += *pi;

    if (!isodigit(buf[i])) {
                return(FALSE);
    }

    for (; isodigit(buf[i]); i++) {
                /* Make sure we don't have a 33-rd bit */
                if (value > ULONG_MAX / 8) {
                        *pi += i;
                        return(FALSE);
                }
                value = 8 * value + buf[i] - '0';
                if (value > high) {
                        *pi += i;
                        return(FALSE);
                }
    }
    if (i && (value >= low)) {
                *pvalue = value;
                *pi += i;
                return(TRUE);
    }
    return(FALSE);
}

/*
 * Try to match an unsigned LONG int,
 * with value between (or equal to) <low> and
 * <high>. <*pi> is incremented by the number of characters in
 * the match.
 */
boolean match_ulong_decimal (char *buf,ulong low, ulong high,
                                                         int *pi, ulong *pvalue)
{
    int i;
    ulong value = 0L;
    char c;

    buf += *pi;
    *pvalue = 0L;

    i = 0;
    if (buf[i] == '+')
                i++;

    for (c = buf[i]; isdigit(c); c = buf[++i]) {
                if ((value > ULONG_MAX / 10)
                        || ((value == ULONG_MAX / 10) && ((c - '0') > ULONG_MAX % 10))) {
                        /* Too many chars, max out. */
                        *pi += i;
                        return(FALSE);
                }
                value = value * 10 + (ulong) (c - '0');
                if (value > high) {
                        *pi += i;
                        return(FALSE);
                }
    }

    if (i && (value >= low)) {
                *pvalue = value;
                *pi += i;
                return(TRUE);
    }
    return(FALSE);
}


#ifdef DEBUG_PARSER
void print_tokenQ (parseinfo *csb)
{
    parsenode *tos;
    for (tos = (parsenode *) csb->tokenQ->qhead; tos; tos = tos->next) {
                printf("\nNode:\n");
                printf("\tcommand '%s'\n", tos->nv_command);
                printf("\tOBJ(int,1) %d OBJ(idb,1) 0x%x '%s'\n",
                           tos->val1, tos->idb1, tos->idb1 ? tos->idb1->namestring : "");
                if (tos->val1 == PDB_OSI) {
                        printf("\tpdb1 0x%x '%s'\n", tos->pdb1,
                                   tos->pdb1 ? ((clns_pdbtype *) tos->pdb1)->name : "");
                } else {
                        printf("\tpdb1 0x%x '%s'\n", tos->pdb1,
                                   tos->pdb1 ? ((pdbtype *) tos->pdb1)->name : "");
                }
    }
}
#endif /* DEBUG_PARSER */
#endif /* IOS_PARSER */
