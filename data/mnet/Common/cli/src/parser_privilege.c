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


#ifdef IOS_PARSER


boolean parser_priv_debug = FALSE;
static void set_privilege_level(parser_mode *, uint, uint,
                                                                char *, boolean, boolean, char *);


/*
 * priv_command
 *
 * Handle parsing of privilege global config command. Call
 * set_privilege_level since we need a new csb, so call with
 * needed arguments extracted from csb
 */
void priv_command (parseinfo *csb)
{
    if (csb->nvgen && !priv_changed_in_mode(csb->pmp)) {
                return;
    }
    set_privilege_level(csb->pmp, GETOBJ(int,1), GETOBJ(int,2),
                                                GETOBJ(string,1), csb->sense, csb->nvgen,
                                                csb->nv_command);
}

/*
 * set_privilege_level
 *
 * Handle parsing and NV generation of privilege global config command.
 * If doing NV generation
 *      traverse parse tree of current node
 *      NV generate commands added to priv_nvgenQ
 * else
 *      parser command with parse tree of current node
 *      change priv level of transitions added to privQ
 */
static void set_privilege_level (parser_mode *mode, uint level, uint reset,
                                                                 char *com, boolean sense, boolean nvgen,
                                                                 char *nv_command)
{
    parseinfo *csb;
    privq *pq;
    priv_nvgenq *pnvq;
    priv_nvgenq *ptmp;
    priv_nvgenq *prev;

    csb = get_csb("Privilege CSB");
    if (!csb) {
                printf(nomemory);
                return;
    }

    csb->mode = mode;
    csb->nvgen = nvgen;
    csb->priv = PRIV_ROOT;
    csb->sense = TRUE;
    csb->priv_set = TRUE;
    push_node(csb, get_mode_top(csb->mode));

    if (csb->nvgen) {
                parse_token(csb);                               /* traverse parse tree */
                if (csb->priv_nvgenQ) {
                        /*
                         * Weed out duplicate entries
                         */
                        pnvq = csb->priv_nvgenQ->qhead;
                        prev = NULL;
                        while (pnvq) {

                                ptmp = pnvq->next;
                                while (ptmp) {
                                        if ((pnvq->level == ptmp->level) &&
                                                (strcmp(pnvq->command, ptmp->command) == 0)) {
                                                /* Duplicate entry */
                                                if (parser_priv_debug) {
                                                        printf("\nDuplicate priv entry 0x%x %d '%s'",
                                                                   (uint)mode,pnvq->level, pnvq->command);
                                                }
                                                if (prev) {
                                                        prev->next = pnvq->next;
                                                } else {
                                                        csb->priv_nvgenQ->qhead = pnvq->next;
                                                }
                                                PARSERfree(pnvq);
                                                pnvq = NULL;
                                                break;
                                        }
                                        ptmp = ptmp->next;
                                }
                                if (pnvq) {
                                        prev = pnvq;
                                        pnvq = pnvq->next;
                                } else {
                                        pnvq = csb->priv_nvgenQ->qhead;
                                }
                        }

                        /*
                         * NV generate remaining entries
                         */
                        while (! QUEUEEMPTY(csb->priv_nvgenQ)) {
                                /*
                                 * For each priv structure, remove from nvgenQ,
                                 * NV generate, and free priv structure
                                 */
                                pnvq = dequeue(csb->priv_nvgenQ);
                                /*
                                   nv_write(TRUE, "%s level %d %s", nv_command,
                                   pnvq->level, pnvq->command);
                                   */
                                printf("nv_write in parser_privilege.c\n");
                                PARSERfree(pnvq);
                        }
                }
                free_csb(&csb);
                return;
    }

    strcpy(csb->line, com);
    if (parser_priv_debug) {
                printf("\nSetting privilege level to %d for command '%s'",
                           level, csb->line);
    }

    /*
     * Parse the input buffer and find the
     * transitions that need to be changed.
     */
    parse_token(csb);
    if (!csb->privQ) {
                /* No transitions found, so report error */
                printf("\n%% Unknown command '%s'", csb->line);
    } else {
                /*
                 * We have some transitions, so change the
                 * privilege levels for them.
                 */
                set_priv_changed_in_mode(mode, TRUE);
                while (! QUEUEEMPTY(csb->privQ)) {
                        pq = dequeue(csb->privQ);
                        if (sense && !reset) {
                                if (parser_priv_debug) {
                                        printf("\nChanging privilege for 0x%x from %x-%x-%d-0x%x",
                                                   (uint)pq->priv, pq->priv->cur_priv,
                                                   pq->priv->def_priv, pq->priv->priv_changed,
                                                   TRANS_PRIV_FLAGS_GET(pq->priv->flags));
                                }
                                pq->priv->priv_changed = TRUE;
                                pq->priv->cur_priv = level;
                                if (parser_priv_debug) {
                                        printf(" to %x-%x-%d-0x%x", pq->priv->cur_priv,
                                                   pq->priv->def_priv, pq->priv->priv_changed,
                                                   TRANS_PRIV_FLAGS_GET(pq->priv->flags));
                                }
                        } else {
                                if (parser_priv_debug) {
                                        printf("\nChanging privilege for 0x%x from %x-%x-%d-0x%x",
                                                   (uint)pq->priv, pq->priv->cur_priv,
                                                   pq->priv->def_priv, pq->priv->priv_changed,
                                                   (uint)(TRANS_PRIV_FLAGS_GET(pq->priv->flags)));
                                }
                                pq->priv->cur_priv = pq->priv->def_priv;
                                if (reset) {
                                        pq->priv->priv_changed = FALSE;
                                }
                                if (parser_priv_debug) {
                                        printf(" to default %x-%x-%d-0x%x", pq->priv->cur_priv,
                                                   pq->priv->def_priv, pq->priv->priv_changed,
                                                   TRANS_PRIV_FLAGS_GET(pq->priv->flags));
                                }
                        }
                        PARSERfree(pq);
                }
    }
    free_csb(&csb);
}

/*
 * show_priv_command
 *
 * Display users current privilege level
 */
void show_priv_command (parseinfo *csb)
{
    printf("\nCurrent privilege level is %d", csb->priv);
}


#endif /* IOS_PARSER */





/*
 * priv_enqueue
 *
 * Add a privilege structure onto queue, allocating storage
 * for queue if necessary
 */
static  void priv_enqueue (queuetype **qp, void *pv)
{
    if (!*qp) {
                *qp = (queuetype *)PARSERmalloc(sizeof(queuetype));
                if (!*qp) {
                        /* No memory */
                        return;
                } else {
                        queue_init(*qp, 0);
                }
    }
    requeue(*qp, pv);
}

/*
 * priv_push
 *
 * Allocate storage for privilege structure and add to privQ
 */
void priv_push (parseinfo *csb, trans_priv *priv)
{
    privq *pq;

    /*
     * Modify the current privilege
     */
    if (parser_priv_debug) {
                printf("\nPushing transition on stack  0x%x", (uint)priv);
    }
    pq = (privq *)PARSERmalloc(sizeof(privq));
    if (!pq) {
                printf(nomemory);
                return;
    }
    pq->priv = priv;
    pq->next = NULL;
    /* Add priv structure to privQ */
    priv_enqueue(&csb->privQ, pq);
}

/*
 * nvgen_privilege
 *
 * Allocate storage for privilege structure and add to priv_nvgenQ
 */
boolean nvgen_privilege (parseinfo *csb, trans_priv *priv, const char *str)
{
    priv_nvgenq *pnvq;
    return TRUE; /* modified by rrams */
#if 0
    if ((! priv->priv_changed) || (csb->flags & CONFIG_ABORT)) {
                /*
                 * Privilege level hasn't changed, or a previous command
                 * failed and we shouldn't bother
                 */
                return(FALSE);
    }
#endif
    pnvq = (priv_nvgenq *)PARSERmalloc(sizeof(priv_nvgenq));
    if (!pnvq) {
                /*
                 * Out of memory, flag CONFIG_ABORT so furthur commands aren't
                 * NV generated and return
                 */
                csb->flags |= CONFIG_ABORT;
                printf(nomemory);
                return(FALSE);
    }
    /* Add text of command */
    if (csb->nv_command[0] != '\0') {
                sprintf(pnvq->command, "%s %s", csb->nv_command, str);
    } else {
                strcpy(pnvq->command, str);
    }
    pnvq->level = priv->cur_priv;
    pnvq->next = NULL;
    /* Add priv structure to priv_nvgenQ */
    priv_enqueue(&csb->priv_nvgenQ, pnvq);
    return(TRUE);
}


