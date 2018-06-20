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
#include <logLib.h>
#include <wdLib.h>      


#include <../include/pstypes.h>
#include <../include/psvxif.h>
#include <../include/platdef.h>
#include <../include/ttyutil.h>

#include <../include/imsg.h>
#include <../include/imc.h>

#include <../include/parsertypes.h>
#include <../include/address.h>
#include <../include/config.h>
#include <../include/parser_defs_exec.h>
#include <../include/parser_input.h>
#include <../include/parser_sim.h>
#include <../include/parser_util.h>
#include <../include/parser_commands.h>
#include <../include/nv.h>
#include <../include/parser_ModNum.h>
#include <../include/parser_errno.h>
#include <../include/defs.h>
#include <../include/parser.h>
#include <../include/cli_parser_private.h>
#include <../include/parser_actions.h>


#define DEVELOPMENT
#define ROOT_UID 0
#define USER_UID 1
#define INVALID_UID -1 
#define PS_TASK_NAME taskName(taskIdSelf())
#define bool boolean

/********* Global variables *********/
static char longCmdErrMsg[] ="\nCommand too long\n";
tpIOINFO pIoInfo = NULL;
char taskid[50];
boolean terminalEcho=TRUE;
boolean doneAuth;

char current_user[MAX_USERNAME_LEN];
char current_password[MAX_PASSWD_LEN];

/********* Function declarations ***********/
void func(int fd,char* str);
static void    process_del_char (tOCTET2 );
bool AuthenticateUser(parseinfo *csb);
tOCTET2 getInput( tINT outfd,tINT infd, char *pPrompt, char *pInput,
                                                tINT len, boolean hideInput );
extern BOOL load_running_config;
extern int backInFd;


/*
 * Write to fd in maxsz chunks.  This accomodates
 * the message oriented nature of vxWorks' pipes.
 */
static STATUS
io_write_buffer(tINT fd, const char *buf, size_t sz, size_t maxsz)
{
        STATUS ret = OK;
        int i;
        ssize_t cc;

        for (i = 0; i < sz; i += cc) {
                cc = sz - i > maxsz ? maxsz : sz - i;
                
                
                cc = write(fd, (void *)&buf[i], cc); 

                if (cc <= 0) {
                        ret = ERROR;
                        break;
                }
        }
        return ret;
}

/*****************************************************************
 *
 *   Func: io_process_response
 *
 *   Desc: Wait for and process response for command request
 *
 *   Inputs:
 *        
 *
 *   Result:  RC_OK   : Command was executed successfuly
 *            RC_ERR  : Error occured while processing command
 *            RC_MORE : 'more' processing required
 *
 *   Algorithm:
 *
 *****************************************************************/
static tOCTET2 io_process_response(tOCTET2 fd, tOCTET4 maxCount,tpIOINFO pIoInfo)
{
    tOCTET2 rc = RC_OK;
    tpCOMSIM pComSim;
    char buffer[256];
    char *pOutput = NULL;

    /* Wait till we get an indication of response from Parser */
    if ( PSsemTake( pIoInfo->ioSemId, WAIT_FOREVER ) == ERROR ) {
        PSpanic("io_process_reponse:Cannot wait on Console Semaphore\n");
    }


    pComSim = NULL;

    /* Get the interaction for us */
    mGET_WORK_SIM(pComSim, pIoInfo->ioModuleId );

    sprintf(buffer, "%s:", taskName(taskIdSelf()));

    if ( pComSim != NULL ) {
        /* This carries any output from modules */
        pOutput = ( char *)(pComSim->pLayerSim);

        /* Check if it is Request or Indication */
        if ( IS_INDICATION(pComSim->serviceid)) {
             if (pComSim->requestid == pIoInfo->requestId ) {
                 rc = pComSim->status;

                 if ( pComSim->pLayerSim ) {
                     io_write_buffer(fd, pOutput, strlen(pOutput), BUFSIZ);
                 }
                 if ( rc == RC_WAIT ) {
                    /* Wait till we are done  with More Processing
                     * or TCL Shell
                     */
                    do {
                        mFREESIM( pComSim );
                        pComSim = NULL;
                        /* Wait for indication from TCL Shell
                         * or tMore task
                         */
                         if ( PSsemTake( pIoInfo->ioSemId, WAIT_FOREVER )
                                                              == ERROR ) {
                               logMsg("Semaphore was deleted\n",0,0,0,0,0,0);
                               break;
                         }

                         mGET_WORK_SIM(pComSim, pIoInfo->ioModuleId );
                         rc = pComSim->status;

                    } while ( pComSim->serviceid != IO_WAIT_COMPLETE_IND);


                    /* TCL Sets the terminal to TERMINAL mode
                     * So reset the terminal characteristcs
                     */
                    ioSetTerminalMode( fd, IO_RAW_MODE );
                 }
             } else  {
                      sprintf(buffer,"io_process_response: Got %ld Expecting %ld\n",
                                                pComSim->requestid, pIoInfo->requestId );
                      write( fd, buffer, strlen( buffer ) );
             }
               
        } else if (IS_REQUEST(pComSim->serviceid)) {
                   /* Who will send a request to us via IMC ? */
                   strcat(buffer, " I/O Task cannot process request!\n" );
                   write(fd, buffer, strlen(buffer));
               } else {
                   /* What kind of service is this ? */
                   strcat(buffer, " io_process_response : Invalid Service Id\n");
                   write(fd, buffer, strlen(buffer));
               }
               /* Done.. See ya */
               if ( pComSim )
                   mFREESIM( pComSim );

    } else  {
            /* Hmm..We get an indication but a NULL SIM ? */
            strcat(buffer, " io_process_response : NULL SIM from GETWORK\n");
            write(fd, buffer, strlen(buffer));
   }

        return ( rc );
}
/*****************************************************************
 *
 *   Func: io_send_request
 *
 *   Desc: Send a request to execute command to parser
 *
 *   Inputs:
 *         line       : Command to be executed
 *         lastchar   : is last char '?', '\t', '\n';
 *         index      : cookie for 'more' processing
 *         maxCount   : Maximum lines to be displayed
 *
 *   Result:  None
 *
 *   Algorithm:
 *
 *****************************************************************/
static void io_send_request(char *line, char lastchar, void *index,
                                                        tOCTET4  maxCount,tpIOINFO pIoInfo )
{
    tOCTET2 rc = RC_OK;
    tpPARSERSIM pSim = NULL;
    int uid;
    parseinfo *csb = pIoInfo->csb;

    /* Get the interaction module for parser */
    pSim = (tpPARSERSIM ) mGETSIM( sizeof( tPARSERSIM ) );
    if ( csb == NULL ) {
        PSpanic("Error occurred while sending Request !\n");
    }

    if ( !pSim ) {
        PSpanic("No Memory to allocate SIM\n");
    }

    /* NULL Command */
    if ( !line ) {
        printf("NULL Command sent as request\n");
    }

    /* Keep track of request */
    ++(pIoInfo->requestId);

    /* User Id on whose context to run */
    uid = csb->uid;

    /* Fill parser interaction module */
    mFILL_PARSER_SIM(pSim, csb, pIoInfo->ioModuleId, \
                         PARSER_MODULE, PARSER_CMD_PROC_REQ, pIoInfo->requestId, \
                         line, lastchar, index, maxCount );
        
    /* Send it to parser */
    mPOST_SIM_NREQ(((tpCOMSIM)pSim), rc );

    if ( rc != RC_OK ) {
        printf("Error occured while processing cosole_send_request\n");
    }
        
}
/*****************************************************************
 *
 *   Func: io_reset_cmdline
 *
 *   Desc: Erase the previously entered command
 *
 *   Inputs:
 *      fd         : Output file descriptor
 *      currentPos : Current Cursor Position
 *      line       : Current Command entered
 *      delPos     : Cursor position to delete from
 *
 *   Result:  None
 *
 *   Algorithm:
 *
 *****************************************************************/
static void io_reset_cmdline( tINT fd, tINT currentPos, tpOCTET line,
                                                          tINT delPos)
{
        tINT index = 0;

        /* Go back to the end of the entered command... */
        for(index = currentPos ;
                index < strlen(line); index++ ){
                if ( line[index] != '\0' ) {
                        write(fd, &line[index], 1);
                } else
                        break;
        }

        /* ...And erase the entire command */
        for( index = delPos; index < strlen(line); index++)
                process_del_char( fd );

}

/*****************************************************************
 *
 *   Func: process_special_char
 *
 *   Desc:   Process special characters for parser
 *
 *   Inputs:
 *         csb          : CSB for the source
 *         ch           : Character to be checked
 *         line         : Input string so far
 *         historyIndex : Pointer to  History Index
 *         currentIndex : Current History Index
 *         currentPos   : Current Cursor Position
 *
 *   Result:  RC_OK        : To echo and continue processing
 *            RC_CONT      : do not echo; go back and read input
 *   Algorithm:
 *
 *****************************************************************/
static tOCTET2 process_special_char( parseinfo *csb, char *ch, char *line,
                                                                        tINT *historyIndex, tINT currentIndex,
                                                                        tINT *currentPos ,tpIOINFO pIoInfo)
{
        tOCTET2 status = RC_OK;
        char chr[PARSEBUF + 1];
        int index = 0;
        char *pCmd = NULL;
        char ich = 0;
        boolean ringBell = TRUE;

        tpOCTET *history = NULL;

        extern parser_mode *interface_mode;

        if ( line == NULL )
                return(RC_CONT);

        /* Point to the correct 'history' array */
        if ((csb->mode == config_mode) ||
            (csb->mode == interface_mode)) 
                history = pIoInfo->configHistory;
        else
                history = pIoInfo->execHistory;

        /* Following is the methodology used to handle arrow keys etc.
     *
     * historyIndex points to the previous command to be displayed
     * currentIndex is the index where the most recently entered command
     * will be stored.
     * currentPos points to the current cursor position
     *
     * Upon an arrow key ( Detected using 'ESC+[' ( or 'ESC+O' for MSFT ?)
     *
     * UP    : Display the history pointed to by historyIndex, and update it
     *         Clear the previously entered command and re-write the new one
     * DOWN  : Update the historyIndex to point to next command, clear the
     *         previously entered command,
     * LEFT  : Write a back-space and update the cursor position
     * RIGHT : Echo back character in the next cursor position & update the
     *         cursor position
     */

        switch (*ch ) {

                /* Typically 'Arrow' keys have the encoding as follows
                 * UP-ARROW    <ESC>[A
                 * DOWN-ARROW  <ESC>[B
                 * LEFT-ARROW  <ESC>[C
                 * RIGHT-ARROW <ESC>[D
                 *
                 * Read to see if <ESC> is followed by '[' or else put it back
                 */
        case CH_ESC:
                status = RC_CONT;
                if ( stdin ) {
                        /* Somehow Microsoft Telnet Clients send ESC + O instead
             * of ESC + [
             */         read(csb->inFd,&ich,1);
                        if  ( ( ich )  == CH_LT_SQ_BRACE ||
                                 ( ich == 'O')  )    {

                                index = *historyIndex;
                                read(csb->inFd,&ich,1);
                                switch( ich ) {
                                case 'A':                               /* UP Arrow */
                                                                                                
                                        /* Get the next Index */
                                        index = (index + MAX_HISTORY - 1) % MAX_HISTORY;
                                                                
                                        /* Display most recently entered command*/
                                        pCmd = history[*historyIndex];

                                        if ( line && pCmd && *pCmd) {

                                               if (*(history[index]) != '\0' ) {
                                                        *historyIndex = index;
                                                }

                                                ringBell = FALSE;

                                                /* Reset the command previously entered */
                                                io_reset_cmdline(csb->outFd, *currentPos, line, 0);

                                                /* Rewrite the command from history */
                                                strcpy(line, pCmd );
                                                write(csb->outFd, pCmd, strlen(pCmd));

                                                /* Update the current cursor position */
                                                if ( currentPos )
                                                        *currentPos = strlen(line);

                                        }
                                        break;

                                case 'B':                               /* DOWN Arrow */
                                        index = ( index + 1 ) % MAX_HISTORY;
                                                                                
                                        if (*(history[index]) != '\0' ) {
                                                *historyIndex = index;
                                                pCmd = history[*historyIndex];
                                        }

                                        /* Reset the command previously entered */
                                        io_reset_cmdline(csb->outFd, *currentPos, line, 0);

                                        if ( line && pCmd && *pCmd) {
                                                ringBell = FALSE;

                                                /* Rewrite the command from history */
                                                strcpy(line, pCmd );
                                                write(csb->outFd, pCmd, strlen(pCmd));

                                                if ( currentPos )
                                                        *currentPos = strlen(line);
                                        } else {
                                                ringBell = TRUE;

                                                strcpy(line, "" );
                                                if ( currentPos )
                                                        *currentPos = 0;
                                        }
                                        break;

                                case 'C':

                                        if ( line && *line && currentPos &&
                                                (*currentPos < strlen(line)) ) {
                                                ringBell = FALSE;

                                                /* Echo back the current character */
                                                chr[0] = line[*currentPos];
                                                chr[1] = '\0';
                                                ++(*currentPos);
                                                write( csb->outFd, chr, 1 );
                                        }
                                        break;

                                case 'D':
                                        if ( line && *line && currentPos &&
                                                (*currentPos != 0) ) {
                                                chr[0] = CH_BS;
                                                chr[1] = '\0';

                                                --(*currentPos);

                                                /* Write a 'BACKSPACE' */
                                                write( csb->outFd, chr, 1 );
                                                ringBell = FALSE;
                                        }
                                        break;

                                default:
                                        break;
                                }
                        }
                }
                break;

                /* Backspace & DEL characters */
        case CH_BS :
        case CH_DEL:
                  
                if ( currentPos && *currentPos > 0 && strlen(line) > 0 ) {

                        io_reset_cmdline(csb->outFd, *currentPos, line,
                                                         (*currentPos - 1 ) );

                        /* Get the buffer from current position till end */
                        strncpy( chr, &(line[*currentPos]), PARSEBUF - 1);
                        chr[PARSEBUF] = '\0';

                        /* Now that we have erased old command after cursor
             * position, rewrite the updated part of command from
             * next to cursor till end
             */
                        write(csb->outFd, chr, strlen(chr));

                        /* Update the command buffer */
                        strncpy( &(line[*currentPos - 1]), chr, (PARSEBUF - *currentPos));
                        line[PARSEBUF] = '\0';
                        (*currentPos)--;

                        chr[0] = CH_BS;
                        chr[1] = '\0';

                        /* Move the cursor position to the new place */
                        for ( index = strlen(line); index > *currentPos; index-- ){
                                write(csb->outFd, chr, 1);
                        }
                        ringBell = FALSE;
                }
                status = RC_CONT;
               
                break;


        default:

                ich = *ch;
          
                /* We are Ok with '\t', '\n' etc... */
          /*      if ( !(isspace( ich ) )) {
                        
                        if ( iscntrl( ich ) ) {
                                if ( ich == CH_EXT )
                                        read(csb->inFd, chr,  2 );
                                status = RC_CONT;
                        } else if ( !isprint( ich ) ) {

                               
                                status = RC_CONT;
                        }
                }
                ringBell = FALSE;*/

                if ( currentPos )
                        (*currentPos)++;

                break;
        }

        /* Beep if the entered key is other than backspace
     * and go back and read input
     */
      /*  if ( ringBell ) {
                ch = CH_BEL;
                write(csb->outFd,&ch,1);
        }*/
        return ( status );
}



/*****************************************************************
 *
 *   Func: process_del_char
 *
 *   Desc:   Actions for backspace/delete key
 *
 *   Inputs:
 *         ioFd         : fd to read from and write to
 *
 *   Result:  None
 *
 *   Algorithm:
 *
 *****************************************************************/
static void process_del_char( tOCTET2 ioFd )
{
        char ch;

        /* When Backspace / Delete entered
     * the character displayed already
     * has to be erased too
     * BS/DEL = <BS> <SPC> <BS>
     */
    ch = CH_BS;
    write(ioFd,&ch,1);

    ch = CH_SPC;
    write(ioFd,&ch,1);

    ch = CH_BS;
    write(ioFd,&ch,1);

}

extern void ParserCallbackInit(void);
/*****************************************************************
 *
 *   Func: parser_io_init
 *
 *   Desc:   Initialize CSB etc for I/O task
 *
 *   Inputs: None
 *
 *   Result:  None
 *
 *   Algorithm:
 *
 *****************************************************************/
static void parser_io_init(tpIOINFO pIoInfo )
{
        parseinfo *csb = NULL;
        tpCOMSIM pComSim = NULL;
		int callbackTaskID;
		int index=0;
        /* Let IMC Know that a module id be assigned */
        pIoInfo->ioModuleId = ASSIGN_MODULE_ID;
        pIoInfo->requestId=0;
        /* Semaphore that will indicate presence of message */
        pIoInfo->ioSemId = PSsemCCreate( PS_DEFAULT_COUNT_SEM_FLAG , 0 );
        
        /* Add to IMC */
        if ( imc_add( &(pIoInfo->ioModuleId), pIoInfo->ioSemId,
                      taskIdSelf()  == RC_ERR ) ) {
          PSpanic("Could not create IMC for IO Task!\n");
        }
        
        /* Did IMC return valid module Id ? */
        if ( pIoInfo->ioModuleId <  0 ) {
          printf("parser_io_init:Negative Module Id\n");
        }
        else {
          
          /* Get CSB to interact with Parser */
          csb = pIoInfo->csb = get_csb(PS_TASK_NAME);
          
          if ( !csb  )
            printf("parser_io_init: Unable to obtain CSB\n");
          
          /* Set some CSB Flags */
          csb->flags = CONFIG_TERM;
          csb->priv = DEFAULT_PRIV;
          *(csb->line) = '\0';
                  
        }
		do{
			mGET_WORK_SIM(pComSim, pIoInfo->ioModuleId );  
		}while(pComSim != NULL);
}



/* BuildPrompt
 * Build promt at csb->prompt.
 *   This routine is global as it is used by SNA network management
 */
void BuildPrompt (parseinfo *csb, tt_soc *tty)
{
    int prompt_length;                                  /* leftover length of prompt */

    char Pprompt[MAXPROMPTLEN + 1];
    

    gethostname(Pprompt,MAXPROMPTLEN);
    Pprompt[MAXPROMPTLEN] = '\0';
  
        
 
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
/*****************************************************************
 *
 *   Func: io_cleanup
 *
 *   Desc: Cleanup the I/O Task
 *
 *   Inputs: tpIOINFO
 *
 *   Result:  None
 *
 *   Algorithm:
 *
 *****************************************************************/

 void io_cleanup(   int TaskID)
{

   parseinfo *csb = NULL;
   int index = 0 ;
  

   if ( pIoInfo == NULL ){
	    return;
   }
   
   csb = pIoInfo->csb;
   if ( csb == NULL ){

           return;
   }
   free_csb(&csb);

   /* Delete all associated semaphores */
   if ( PSsemDelete( pIoInfo->ioSemId ) == ERROR ) {
           printf("ioWork: Cannot delete semaphore\n");
   }
   /* Close socket for telnet connections 
   if ( (pIoInfo->sockFd > 0) &&
    (iosFdValue (pIoInfo->sockFd) != ERROR)) {
           close ( pIoInfo->sockFd );
   }*/




   /* Delete History Array */
   for( index = 0; index < MAX_HISTORY; index++ ) {
           /* Exec Level History */
           if ( pIoInfo->execHistory[index] )
                   PARSERfree( pIoInfo->execHistory[index] );

           /* Config Level History */
           if ( pIoInfo->configHistory[index] )
                   PARSERfree( pIoInfo->configHistory[index] );
   }
   

   /* Remove yourself from IMC */
   if ( imc_remove( pIoInfo->ioModuleId ) != RC_OK ) {
           printf("ioWork: Cannot delete Module from IMC\n");
   }
   cleanupWorkerTask();
   PARSERfree( pIoInfo ); 
   
   pIoInfo=NULL; 
   

 
}

void getPrompt( parseinfo *csb, char* prompt, char *complete )
{
  if ( csb != NULL ) {
    BuildPrompt( csb, NULLTTY );
    
    strncpy(prompt, csb->prompt, MAXPROMPTLEN); 
    prompt[MAXPROMPTLEN] = EOS;
    
    strncpy(complete, csb->line, PARSEBUF);     
    complete[PARSEBUF] = EOS ;
  }
  else {
    panic("NULL CSB Obtained in getPrompt\n");
  }
}
/*****************************************************************
 *
 *   Func: io_work
 *
 *   Desc:   Task Entry point for  shell
 *
 *   Inputs:
 *         inFd         : File descriptor to read from
           outFd        : File descriptor to write to
 *         errFd        : File descriptor for standard error
 *         interactive  :True for console & telnet , False for startup script
 *
 *   Result:  None
 *
 *   Algorithm:
 *         forever {
 *            Display prompt;
 *            Read a character at a time
 *            Send request to parser either if \n, ?, \t,ctrl-z
 *            Wait for response
 *         }
 *
 **********************************************************************/

void io_work( tINT inFd , 
              tINT outFd,
              tINT errFd,
              boolean interactive  /*** True for console & telnet , False for script **/
            )
{


  char line[PARSEBUF + 1];
  char complete[PARSEBUF + 1];
  char chr[PARSEBUF + 1];
  char prompt[MAXPROMPTLEN + 1];
  char lastchar;
  char ch ;
  void *pIndex;
  parseinfo *csb;
  parser_mode *prevMode ;
  extern parser_mode *interface_mode;
  
  tOCTET2  rc ;
  tINT  execIndex ;
  tINT  configIndex ;
  tINT  index ;

  tpOCTET  *currentHistory ;
 
  tINT *historyIndex;
  tINT currentIndex ;
  tINT currentPos ;

	restart:
	ch = '\0';
	pIndex = NULL;
	prevMode = NULL;
	rc  = RC_OK;
	execIndex = 0;
	configIndex = 0;
	index = 0;
	currentHistory = NULL;
	historyIndex = &execIndex;
	currentIndex = 0;
	currentPos = 0;

  /* Check if we have valid fd's */
 
  if ( outFd <= 0 || inFd < 0 || errFd < 0) {
      PSpanic("Invalid fd for I/O Task\n");
  }

  doneAuth = FALSE;


  /*****Allocate memory for pIoInfo *******/
  if ( (pIoInfo = (tpIOINFO)
      PARSERcalloc(1, sizeof( tIOINFO))) == NULL ) {
      PSpanic("ioWork: No memory to allocate IO Info\n");
  }

  /* Create the history array for Exec & Config Modes */
  for (index = 0; index < MAX_HISTORY; index++ ) {
       pIoInfo->execHistory[index] =
                   (tpOCTET) malloc(( sizeof(tOCTET) * (PARSEBUF+1)) );
       strcpy(pIoInfo->execHistory[index], "");

       pIoInfo->configHistory[index] =
                  (tpOCTET)malloc(( sizeof(tOCTET) * (PARSEBUF+1)));
       strcpy(pIoInfo->configHistory[index], "");
  }      

  /* This creates the CSB, adds to IMC etc */
  parser_io_init(pIoInfo);

  /***********/

 
  /* Get CSB to with parser */
  csb = pIoInfo->csb;

  if (interactive){
       csb->uid=USER_UID;
       csb->priv=PRIV_MIN;
  }else{
        csb->uid=ROOT_UID;
        csb->priv=PRIV_ROOT;
        doneAuth=TRUE;
  }

  /* CSB may not need inFd & outFd ; But ... */
  if(interactive){
		csb->inFd =infd;
		csb->outFd =outfd;
  }else{
		csb->inFd =inFd;
		csb->outFd =outFd;
  }
  /* For More processing cookie.. */
  csb->pIndex = NULL;
  
  
  /* For Task level CWD */
    csb->ioTaskId = taskIdSelf();

  /* Whats my Module Id ? */
  csb->moduleId = pIoInfo->ioModuleId;

  
  line[0] = lastchar = '\0';
        
  /* We will be in RAW Mode so that we can read a character
   * at a time and we will do the ECHO
   */
  ioSetTerminalMode( csb->inFd, IO_RAW_MODE );

  ioTaskStdSet( 0, STD_IN, csb->inFd );
  ioTaskStdSet( 0, STD_OUT, csb->outFd );
  ioTaskStdSet( 0, STD_ERR, csb->outFd );
 

  ioGlobalStdSet( STD_IN, csb->inFd );
  ioGlobalStdSet( STD_OUT, csb->outFd );
  ioGlobalStdSet( STD_ERR, csb->outFd );

  while( 1 )  {
        pIoInfo->flags = 0;

        ioSetTerminalMode( csb->inFd, IO_RAW_MODE );
   
        /************ Authenticate User ********************************/
        if ( doneAuth == FALSE ) {
            if(!AuthenticateUser(csb)){extern isLoggedOut;
			    csb->uid=ROOT_UID;
				csb->priv=PRIV_ROOT;
				if(isLoggedOut==TRUE){
					isLoggedOut=FALSE;
					goto cleanup;
				}
  				strcpy(line,"exit");
                goto CommandInput;
               
             }
            
            doneAuth=TRUE;
        }
        /***************************************************/

        prompt[0] = '\0';
        complete[0] = '\0';
        getPrompt(csb,prompt, complete );


        /* The user did not enter \n last time; and so
         * we may not have echoed it.. So put \n
         */
        if ( lastchar == '\t')
             write(csb->outFd,"\n",1);

         write( csb->outFd,prompt, strlen(prompt) );
      
        /* If it is command completion, display the
         * the completed command or it is help
         * display previously entered string
         */

        if ( *complete ) {
             strcpy( line, complete );
             write( csb->outFd, complete, strlen(complete));
        }else {
             strcpy(line,"\0");
        }

        /* Reset 'more' cookie */
        pIndex = csb->pIndex = NULL;

        /* Current Cursor Position */
        currentPos = strlen(line);

        while (1) {

              ch = '\0';
              ioSetTerminalMode( csb->inFd, IO_RAW_MODE );
            
	      /* Read one character from fd */
              if ( read(csb->inFd, &ch, 1 ) <= 0 ) {
				  if(load_running_config){
					load_running_config=FALSE;
					csb->mode=exec_mode;
					csb->inFd=backInFd;
				  }else
					goto cleanup;
              }
               
              /*
               * Just in case...
               */
              if ( ch == '\0' ) 
                  break;
            
              rc = process_special_char(csb, &ch, line, historyIndex, 
                                       currentIndex, &currentPos,pIoInfo );
              /* Go back and read the input */
              if ( rc == RC_CONT ) 
                   continue;

              /* Used for 'strcat' to line  */
              chr[0] = ch;
              chr[1] = '\0';
              if( ch == CH_CTRL_Z ){
		       	   strcpy(line,"disable");
 	               goto CommandInput;
	   	      }	
			  if ( ch == '\t' || ch == '?' || ch == '\n'){
                  write(csb->outFd, &ch, sizeof(char));
                  lastchar = ch;
                  break;
              } else {

                 io_reset_cmdline(csb->outFd, currentPos, line,
                                  currentPos );

                 if ( strlen(line) > 0 ) {
                      /* Get the buffer from current position till end */
                       strncat(chr, &(line[currentPos - 1]), PARSEBUF);
                       chr[PARSEBUF] = EOS;
                  }
       
                  /* Now that we have erased old command after cursor
                   * position, rewrite the updated part of command from
                   * next to cursor till end
                   */
                  write(csb->outFd, chr, strlen(chr));

                  /* Build the command to be executed
                   * Make sure we take into account that the 'arrow'
                   * keys may have been used and so replace the right
                   * character in the 'line'
                   */
                  if ((currentPos + strlen(chr)) < PARSEBUF){

                      strncpy(&(line[currentPos - 1]), chr, (PARSEBUF-currentPos));
                      line[PARSEBUF] = EOS;
                  } else {
                      /* Make sure that the command does not exceed
                       * the maximum length 
                       */
                       write(csb->outFd, longCmdErrMsg, strlen(longCmdErrMsg) );
                       line[PARSEBUF] = EOS;
                       goto process_next_command;
                  }

                  chr[0] = CH_BS;
                  chr[1] = '\0';
  
                  /* Move the cursor position to the new place */
                  for ( index = strlen(line); index > currentPos; index-- ){
                       write(csb->outFd, chr, 1);
                  }

              }
   
        } /* end while */

    

        /* Update the 'more' cookie */
        pIndex = csb->pIndex;
        /* Store the mode the command was issued */
 
        prevMode = csb->mode;
  
        /* Send the request to parser */
        CommandInput:

        io_send_request(line, lastchar, NULL, 0,pIoInfo);
         /* Wait for the response from the parser */
        io_process_response(csb->outFd, 0,pIoInfo);
        /* store_history: */
        /* Update the correct History Array. We have config
         * mode history and exec mode history arrays
         */
        if ( prevMode == config_mode )
            currentHistory = pIoInfo->configHistory;
		else 
            currentHistory = pIoInfo->execHistory;


        if ( !currentHistory )
            PSpanic("NULL History pointer in I/O Task\n");

        /* This flag is set *ONLY* if the command entered
         * is  *NOT* for command completion
         */
        if ( csb->flags & COMMAND_COMPLETE )  {
            /* Update the current history and make it point to
             * right index
             *
             * currentIndex always is the index where the most
             * recently entered command will be stored
             * historyIndex is the index where we are at by using
             * arrow keys etc. This will be set by process_special_char
             * routine
             */
             if ( *line ) {
                 strncpy(currentHistory[currentIndex], line, PARSEBUF);
                 currentHistory[currentIndex][PARSEBUF]=EOS;  
                 *historyIndex = currentIndex;
                  currentIndex = (currentIndex + 1) % (MAX_HISTORY );
             }

            /* Make sure we update the correct history index
             * in each mode
             */
            historyIndex = ( csb->mode == exec_mode )
                           ? &execIndex
                           : &configIndex;


           /* There has been a mode change ? Lets revert back to
            * what the mode history index was previously
            */
           if ( prevMode != csb->mode )
                currentIndex = ((*historyIndex) + 1) % (MAX_HISTORY);
               
           csb->flags &= ~COMMAND_COMPLETE;
   
        }
        process_next_command:
       
  } /* end while */
  cleanup:
  io_cleanup( taskIdSelf() );
  

 
 
}
 

/*
 * parser_spawn - create a parser task and wait for it to finish
 */
int CliIOInit (int inFd,int outFd,int errFd,boolean interactive)
{
        int ret = 0;    
        int tid;
        char tname[20];
        static int name=0;
        int ttymode;      

        ttymode = ioctl(inFd, FIOGETOPTIONS, 0);
        
       /* sprintf(tname, "tCli%d", name++);*/
		sprintf(tname, "tCli");

        /***** Creates shell task for console and telnet session **********/
        tid = taskSpawn(tname, PARSER_TASK_PRIORITY, 0, PARSER_TASK_STACK_SIZE,
                       (FUNCPTR)io_work, 
                       inFd,
                       outFd,                      
                       errFd,              
                       interactive,       
                       0,
                       0,0,0,0,0);

        ret=tid;
        ioctl(inFd, FIOSETOPTIONS, ttymode);
        return ret;
}


/*****************************************************************
 *
 *   Func: ioSetTerminalMode
 *
 *   Desc: Set the terminal mode
 *
 *   Inputs:
 *      fd       : File descriptor of the terminal
 *      termMode : RAW / TERMINAL ( enums )
 *c
 *   Result:  Current Terminal Options
 *            ERROR on Error
 *
 *   Algorithm:
 *
 *****************************************************************/
tINT ioSetTerminalMode( int fd, tTERMINALMODE termMode )
{


        switch( termMode ) {

        /* RAW Terminal Mode */
        case IO_RAW_MODE:
                if (fd == consoleFd)
                {
                        if ( ioctl (fd, FIOSETOPTIONS, OPT_RAW | OPT_CRMOD | 
                                                OPT_7_BIT | OPT_MON_TRAP )
                                == ERROR ) {
                                return( ERROR ) ;
                        }
                }
                else
                {    
                        if ( ioctl (fd, FIOSETOPTIONS, OPT_RAW | OPT_CRMOD |
                                                OPT_7_BIT)
                == ERROR ) {
                return( ERROR ) ;
                }
                }
                break;

        case IO_LINE_MODE:
                if (fd == consoleFd)
                {
                        if ( ioctl( fd, FIOSETOPTIONS, OPT_TERMINAL ) == ERROR ) {
                                return ( ERROR );
                        }
                }
                else
                {
                        if ( ioctl (fd, FIOSETOPTIONS, OPT_ECHO | OPT_CRMOD | OPT_TANDEM | 
                                                OPT_7_BIT | OPT_ABORT | OPT_LINE) 
                                                == ERROR) {
                                return (ERROR);
                        }
                }
                break;

        case IO_ECHO_MODE:
                if (fd == consoleFd)
                {
                        if ( ioctl( fd, FIOSETOPTIONS,
                                                OPT_ECHO | OPT_CRMOD | OPT_7_BIT | OPT_MON_TRAP ) 
                                                == ERROR ) {
                                return ( ERROR );
                        }
                }
                else 
                {
                        if ( ioctl( fd, FIOSETOPTIONS,
                                                OPT_ECHO | OPT_CRMOD | OPT_7_BIT) == ERROR ) {
                return ( ERROR );
                        }
                }
                break;
        default:
                return( ERROR );
        }

        return(ioctl( fd, FIOGETOPTIONS, 0 ));
}

/************* Authentication function *************/

bool AuthenticateUser(parseinfo *csb){

char usrPrompt[] ="Username: ";
char passwdPrompt[] = "Password: ";
char usrName[MAX_USERNAME_LEN] = "";
char passwd[256]  = "";
char confirm_password[256]="";
int retries=6;
bool rc = FALSE;

	

	if(defaultUserInUse == TRUE){
		printf("\nSystem is being booted for the first time\n\n");
		printf("PASSWORD FOR THE DEFAULT USER(user) NOT SET\n");
		printf("Please enter the password for default user,\"user\".\n");
		printf("Password length should be atleast 8 characters\n");

		if(getInput( STD_OUT,STD_IN,"password :", passwd, 127, TRUE )==FALSE){
			printf("Could not read password \n");
            return FALSE;
        }
   		if(getInput( STD_OUT,STD_IN,"\nconfirm password :", confirm_password, 127, TRUE )==FALSE){
  			printf("Could not read password \n");
            return FALSE;
        }
		if(strcmp(passwd,confirm_password)!=0)
		{
			printf("\nPasswords do not match\n");
			return FALSE;
		}
		if(sysUserAdd("user",(char*)passwd,0)==OK){
			 defaultUserInUse=FALSE;
             printf("\nAdded user successfully\n");
             strcpy(current_user,"user");
			 strcpy(current_password,passwd);
			 csb->uid=0;
			 csb->priv=PRIV_MIN;
			return TRUE;   
		}else{
			  printf("\nUser could not be added\n");
			  return FALSE;
		}	
		
	}

	if( ShellTask == SSHTask)
        return TRUE;

    while ( retries-- > 0 ) {
                
        if ( write( csb->outFd, "\n", 1 ) < 0 )
                        return ( FALSE );
        
            /* Don't prompt for password unless the user enters
             * something
             */
                while( strcmp(usrName, "") == 0 ) {
                rc = getInput( csb->outFd,csb->inFd,usrPrompt, usrName, MAX_USERNAME_LEN, FALSE );
                       if ( rc == FALSE )
                                return ( FALSE );
                }
       
                /* Prompt for password */
                rc = getInput( csb->outFd,csb->inFd,passwdPrompt, passwd, MAX_PASSWD_LEN, TRUE );
        
                if ( ( rc == FALSE ) ||
                                        ( write( csb->outFd, "\n", 1 ) <= 0 ) )
                        return ( FALSE );

        
	        /* set remote IP address to display user's settings */
        

               
               if((csb->uid=sysUserVerify(usrName,passwd))<0){
                         write(csb->outFd,"Login incorrect",strlen("Login incorrect"));
                         rc=FALSE; 
               }else{
                         strcpy(current_user,usrName);
                         strcpy(current_password,passwd);
                         rc=TRUE;
                         break;
               }

               csb->priv = (csb->uid==ROOT_UID)
                           ?  PRIV_MIN
                           :  PRIV_MIN;
             
               strcpy(usrName, "");
               strcpy(passwd, "");


    } /* while retries*/
    
    return rc;

}


/*****************************************************************
 *
 *   Func: getInput
 *
 *   Desc:   Display specified prompt and get input
 *
 *   Inputs:
 *         infd       : fd to read from 
           outfd      :fd to write to
 *         pPrompt    : Prompt to be displayed
 *         pInput     : Fill the entered input
 *         len        : Maximum length of input
 *         hideInput  : Echo input back ?
 *
 *   Result:  TRUE : If input was successful
 *            FALSE: otherwise
 *
 *   Algorithm:
 *
 *****************************************************************/
tOCTET2 getInput( tINT outfd,tINT infd, char *pPrompt, char *pInput,
                                                tINT len, boolean hideInput )
{
    tOCTET2 rc = TRUE;
    tINT nbytes =0;
        int ttymode;

        /* No way .. */
    if ( !pPrompt || !pInput ) {
        return ( FALSE );
    }

        /* Well just in case ? */
    if (write (outfd, pPrompt, strlen(pPrompt)) != strlen( pPrompt) ){
        return( FALSE) ;
    }

        /* Wait till '\n' is entered */
        ttymode = ioSetTerminalMode( outfd, IO_LINE_MODE );

        /* Do not echo if not required */
    if ( hideInput ) {
		/* terminalEcho=FALSE;*/
        ( void ) ioctl(outfd, FIOSETOPTIONS, (ttymode & ~OPT_ECHO));
    }
	
        /* Read maximum specified characters or new-line */
    if ( (nbytes = read(infd, pInput, len - 1) ) <= 0 ) {
        rc = FALSE;
    } else {
        pInput[nbytes-1] = '\0';
    }
    /* Reset terminal characteristics */
    ioSetTerminalMode( outfd, IO_RAW_MODE );
	terminalEcho=TRUE;
    return ( rc );

}
/*****************************************************************
 *
 *   Func: cancelIO
 *
 *   Desc: Cancel the pending I/O operation on a fd
 *
 *   Inputs:
 *      fd         : I/O file descriptor
 *
 *   Result:  None
 *
 *   Algorithm: Used by the Watchdog timer.
 *
 *****************************************************************/
 void cancelIO( tINT fd )
{
        ioctl(fd, FIOCANCEL, 0 );
}


