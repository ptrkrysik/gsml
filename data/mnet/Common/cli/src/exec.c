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


#if 0
#include <parser/parser.h>
#include <parser/parser_actions.h>
#include <parser/parser_defs_config.h>
#include <parser/parser_errno.h>
#include <parser/parser_undo.h>
#include <envLib.h>
#include <dosfs/uvfatif.h>


extern char **buildargv_k( char *, int *, int);
extern tclMain( int, int, int, int, int, int, int, int );


/* BuildPrompt
 * Build promt at csb->prompt.
 *   This routine is global as it is used by SNA network management
 */
void BuildPrompt (parseinfo *csb, tt_soc *tty)
{
    int prompt_length;                                  /* leftover length of prompt */

    char Pprompt[MAXPROMPTLEN + 1];
        char sHostName[MAXHOSTNAMELEN + 1];

        gethostname(sHostName, sizeof(sHostName));

        strncpy(Pprompt, sHostName, MAXPROMPTLEN);
        Pprompt[MAXPROMPTLEN] = '\0';

    if ( Pprompt[0] != '\0' ) {
                ;
    } else if ( sysBootParams.targetName && 
                           *(sysBootParams.targetName) ) {
        strncpy(Pprompt, sysBootParams.targetName, MAXPROMPTLEN );
                Pprompt[MAXPROMPTLEN] = '\0';
    } else {
                PSpanic("This Cannot be possible ? ? ? \n");
        strcpy( Pprompt, "Cache Engine");
    }
 
    /* Build the prompt. */
    if (config_prompt || (csb->mode == exec_mode) ||
                !(csb->flags & CONFIG_TERM)) {
                const char *src;
                char *dst, trailer;
        
                trailer = (csb->priv > PRIV_USER) ? '#' : '>';
                dst = csb->prompt;
                src = Pprompt;
                prompt_length = MAXPROMPTLEN;
                while (*src && prompt_length--) {
                *dst++ = *src++;
                }
        
                /* dst += reg_invoke_parse_cli_prompt(tty->parse_cli_type,
                   csb, dst, &prompt_length);
                   */
                src = get_mode_prompt(csb);
                if (src && (prompt_length > 3)) {
                prompt_length -= 3;
                *dst++ = '(';
                while (*src && prompt_length--) {
                                *dst++ = *src++;
                }
                *dst++ = ')';
                } else {
                        /*
                         * Prompt is too long, so cut off end
                         */
                        if (prompt_length <= 0) {
                                dst = &csb->prompt[MAXPROMPTLEN-1];
                        }
                }
                *dst++ = trailer;
                *dst = '\0';
    } else {
                csb->prompt[0] = '\0';
    }
}




void terminalCmd( parseinfo *csb )
{

        int realOutFd = csb->savedFd;

        csb->giveConfigSem = TRUE;

        if ( csb->nvgen ) { 
                nv_write( ( ( csb->which == TERM_LENGTH) && 
                                   ( terminalLength !=   LINES )) ,
                                 "%s %d", csb->nv_command, terminalLength);
                return;
        }

        if ( csb->which == TERM_LENGTH ) {
                parse_undo(csb, "terminal length %d", terminalLength);
                terminalLength = GETOBJ(int, 1);
                return;
        } else if ( csb->which == TERM_MONITOR) { 
                if ( realOutFd == consoleFd ) {
                        printf("Console is always monitored\n");
                        return;
                }
                
                if ( csb->sense ) {
                        if ( registerFd( realOutFd ) != RC_OK ) 
                                printf("Cannot monitor the terminal\n");
                } else {
                        if ( unregisterFd( realOutFd ) != RC_OK );
                        printf("Cannot stop monitoring the terminal\n");
                }
        }
}

static void
tclshTask(int argc, const char *const*argv, parseinfo *csb)
{
        SEM_ID sema;
        long moduleId;
        parseinfo *tclCsb;
        char *uvfatBuffer = NULL;
        tpUVFATFREEFUNC pFreeFunc = NULL;
        char *pCwd = NULL;
        tUID uid = csb->uid;
        int err;
        tCOMSIM *pComSim = NULL;
        short rc = RC_OK;
        int inFd = csb->inFd;
        int outFd = csb->outFd;
        extern int setenv(const char *, const char *);
        extern int tclStart(int, const char *const *, const char *const*, const char *);

        ioSetTerminalMode(inFd, IO_LINE_MODE);
        ioTaskStdSet( 0, STD_IN, inFd );
        ioTaskStdSet( 0, STD_OUT, outFd );
        ioTaskStdSet( 0, STD_ERR, outFd );

        if (setUid(uid) == FALSE) {
                fprintf( stderr, "Invalid UID Obtained %d\n", uid);
                fflush( stderr );
                return;
        }

        moduleId = ASSIGN_MODULE_ID;
        sema = PSsemCCreate(PS_DEFAULT_COUNT_SEM_FLAG, 0);
        if (imc_add(&moduleId, sema, taskIdSelf()) == ERROR) {
                PSpanic("Cannot add TCL Task to IMC\n");
        }
        if ( (tclCsb = get_csb("TCL CSB")) == NULL ) { 
                PSpanic("NULL CSB For TCL!!!!\n");
        }
                
        tclCsb->flags = CONFIG_TERM;
        tclCsb->priv  = csb->priv;
        tclCsb->uid   = csb->uid;
        tclCsb->inFd  = inFd;
        tclCsb->outFd  = outFd;
        tclCsb->ioTaskId  = csb->ioTaskId;
        tclCsb->io_timer = csb->io_timer;
        *(tclCsb->line) = '\0';

        /* Get the current working directory of the I/O Source 
         */
        pCwd  = getTaskCWD_k( ((parseinfo*)csb)->ioTaskId, &pFreeFunc ) ;
        if ( pCwd )  {
                setCWD( pCwd );
                pFreeFunc( pCwd );
                pFreeFunc = NULL;
        }

        fflush( stdin ) ;
        fflush( stdout ) ;

        uvfatBuffer = getHomeDir_k(&pFreeFunc);
    setenv( "HOME", (uvfatBuffer == NULL)?mVOLNAME:uvfatBuffer );
        pFreeFunc( uvfatBuffer );
        uvfatBuffer = NULL;
        pFreeFunc = NULL;

        uvfatBuffer = getNameByUid_k( uid , &pFreeFunc);
    setenv( "USER", uvfatBuffer );
        pFreeFunc( uvfatBuffer );
        uvfatBuffer = NULL;
        pFreeFunc = NULL;

        err = tclStart(argc, argv, NULL, NULL);

        /*
         * Clean up the mess and 
         * Send a message back to the parser
         * to let them know we are done.
         */
        if (csb != NULL) {
                ((parseinfo *)csb)->MoreStatus = RC_OK ;
                pComSim = NULL;
                pComSim = mGETSIM( sizeof( tCOMSIM ) ) ;

                if (pComSim == NULL)
                {
                        printf("Not enough memory...please try later\n");
                        rc = RC_ERR;
        }
                else
                {
                    mFILL_COM_SIM( pComSim, moduleId, \
                                           ((parseinfo *)csb)->moduleId,
                                           IO_WAIT_COMPLETE_IND, 0, NULL, 0, RC_OK );

                    mPOST_SIM_NIND( pComSim, rc );
        }

                free_csb(&(tclCsb)); 

                if ( imc_remove( moduleId ) != RC_OK ) {
                        printf("tTcl: Cannot remove IMC \n");
                }
                if ( semDelete( sema ) == ERROR ) {
                        printf("tTcl: Could not delete TCL Semaphore\n");
                }
                sema = NULL;
                tclCsb = NULL;
        }

        /* Check if we got to free the argv[] in our case */
        if (argv != NULL) {

                /* Free argv[argc - 1] till argv[0] */
                
                while ( --argc >= 0 ) {
                        PSkfree( ((char **)argv)[argc], M_TCL);
                }

                PSkfree( (char **) argv, M_TCL );

        }

        /* Require this for CGI
     * This effectively puts EOF in the FD which 
     * may be the terminating condition of 'fgets' or
     * 'read' 
     */
        if ( outFd ) {
                ioctl(outFd, FIOCANCEL, 0);
                write(outFd, "", 0 );
        }
}


void tclshCommand( parseinfo *csb )
{
    char **argv = NULL;
    int argc = 0;
    char *pArg = GETOBJ(string, 1 );
    char *temp = NULL;
        int tid = 0;
        char tname[20];
        
        temp = PARSERmalloc((strlen(pArg) + strlen("tclsh") + 3));
    strcpy(temp,"tclsh");

        if ( strcmp( pArg, "") != 0 ){
        strcat(temp, " ");
        strcat(temp, pArg);
        }
        
        /* Note: argv is NOT allocated from Parser Pool */
    argv = buildargv_k( temp, &argc, M_TCL ) ;
 
        PARSERfree( temp );

        if ( argv == NULL ) {
                csb->MoreStatus = RC_OK;
                return;
        }

        sprintf(tname, "tTcl%u", csb->outFd);
        tid = PStaskSpawn(tname, PS_STD_PRIORITY, VX_PRIVATE_ENV|VX_FP_TASK, 
                                        PS_ENORMOUS_STACK, (FUNCPTR) tclshTask, (int)argc,
                                        (int)argv,(int)csb,
                                        0,0,0,0,0,0,0);
        if (tid == ERROR) {
                csb->MoreStatus = RC_OK;
                return;
        }
 
        /* Indicate to the source that it has a child that it has
     * to take care of 
     */
        csb->pIndex = (void*) tid;

        /* Wait till TCL completes */
    csb->MoreStatus = RC_WAIT;
}
#endif


/*
 * exec_help_command
 *
 * Provide the user with an introduction to the interactive
 * help mechanism of the new parser.
 */
void exec_help_command (parseinfo *csb)
{
        csb->giveConfigSem = TRUE;

    if (csb->nvgen) {
        return;
    }

    printf("\nHelp may be requested at any point in a command by entering"
           "\na question mark '?'.  If nothing matches, the help list will"
           "\nbe empty and you must backup until entering a '?' shows the"
           "\navailable options."
           "\nTwo styles of help are provided:"
           "\n1. Full help is available when you are ready to enter a"
           "\n   command argument (e.g. 'show ?') and describes each possible"
           "\n   argument."
           "\n2. Partial help is provided when an abbreviated argument is entered"
           "\n   and you want to know what arguments match the input"
           "\n   (e.g. 'show stat?'.)\n");
}

void show_help_command (parseinfo *csb)
{
    printf("\n%% Type \"%s ?\" for a list of subcommands\n", "show");
        parser_return(csb, S_parser_SYNTAX_INCOMPLETE);
}

void clear_help_command (parseinfo *csb)
{
    printf("\n%% Type \"%s ?\" for a list of subcommands\n", "clear");
        parser_return(csb, S_parser_SYNTAX_INCOMPLETE);
}

void debug_help_command (parseinfo *csb)
{
    printf("\n%% Type \"%s ?\" for a list of subcommands\n", "debug");
        parser_return(csb, S_parser_SYNTAX_INCOMPLETE);
}

void show_version (parseinfo *csb)
{
  /*    printVersion(); */
  printf("\n SHOW version called");
}
