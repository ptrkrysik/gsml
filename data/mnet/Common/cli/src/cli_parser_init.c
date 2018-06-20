#include <vxworks.h>
#include <stdioLib.h>
#include <taskLib.h>
#include <tyLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN */
#include <timers.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <../include/cli_parser_private.h>
#include <../include/pstypes.h>
#include <../include/psvxif.h>
#include <../include/imsg.h>
#include <../include/imc.h>
#include <../include/cli_parser_api.h>
#include <../include/address.h>
#include <../include/parsertypes.h>
#include <../include/config.h>
#include <../include/parser_sim.h>
#include <../include/parser.h>

static tCSEMAPHORE parserSemId;
static tTASKID     parserTaskId;

SEM_ID systemloadingSemId = NULL;
SEM_ID parse_chain_sem = NULL;
extern int consoleFd;
extern void BasicParserInit(void);


#if 0
#include "cli_private.h"

#include <parser/parser_init.h>
#include <parser/parser_sim.h>
#include <parser/parser_actions.h>
#include <imc/imc.h>
#include <imc/imsg.h>
#include <parser/nv.h>
#include <parser/sys_debug.h>
#include <parser/debug.h>
#include <syslog.h>
#include <h/os/bootflags.h>




int terminalLength = LINES;
int waitingTaskCount = 0;
int parserNullFd = -1;

FILE *parserNullFp = NULL; 

/* Extern Declarations */
extern SEM_ID waitingSemId;
extern SEM_ID rootBSemId;
extern tCSEMAPHORE  initSemId;
extern void mace_chain_init(void);
extern void sysconf_init(void);

/* Prototypes */
static void parser_process_request( tpCOMSIM );
static void parser_process_response( tpCOMSIM , tINT);
static void parser_work(void);






void
parser_configure(void)
{
        if ( sysBootParams.flags & BYPASS_NVRAM_CONFIG) { 
                printf("Bypassing NVRAM Configuration...\n");
        } else { 
                printf("Configuring from NVRAM...\n");
                nv_configure( PRIV_ROOT );
        }
}

void
parser_check(void)
{
        extern int ip_name_server_config_check(void);

        /*
         * check if all the required configuration has been done.
         * if not, generate syslog messages
         */
        ip_name_server_config_check();
}
void parser_process_response( tpCOMSIM pComSim, tINT expectId )
{

        if (pComSim->requestid != expectId ) {
                printf("Invalid Request Id obtained in Indication\n");
        }

        if ( pComSim->status != RC_OK ) {
                printf("Invalid Status obtained\n");
        }

        if ( pComSim->pLayerSim ) {
                printf("%s\n", (char *)(pComSim->pLayerSim));
        }

    return;

}

void parse_configure(char *buffer, int permanent,
              int resolvemethod, uchar priv ) 
{
        tOCTET4 moduleId = ASSIGN_MODULE_ID;
    boolean end_found;
    parseinfo *csb;
        tpCOMSIM pComSim = NULL;
    tpPARSERSIM pSim = NULL;
        int requestId = 0;
        SEM_ID configparserSemId;
 
    configparserSemId =  semCCreate(  _DEFAULT_COUNT_SEM_FLAG , 0 );
    if ( imc_add( &moduleId, configparserSemId, taskIdSelf() ) == ERROR ) { 
                 panic("Cannot add Parser Configuration to IMC\n");
        }

    csb = get_csb("Parser Configuration CSB");
    if (csb == NULL) {
        return;
    }
 
        csb->uid = 0; /* ROOT */
    csb->priv = priv;
    csb->resolvemethod = resolvemethod;
    csb->mode = config_mode;
    csb->break_chars = NVCONFIG_BREAKSET;
    csb->batch_buf = buffer;
    csb->batch_offset = buffer;
    csb->flags = 0;
        csb->inFd = csb->outFd = consoleFd;
 
    end_found = FALSE;
    while (batch_input(csb, csb->line, PARSEBUF,
               csb->break_chars, &csb->lastchar)) {
        if (*csb->line && (*csb->line != '!')) {    /* Skip empty lines */

                if ((strcmp(csb->line, "end") == 0) || 
                    (strcmp(csb->line, " end") == 0))
                {
                        end_found = TRUE;
                break;
                }

                pSim = (tpPARSERSIM ) mGETSIM( sizeof( tPARSERSIM ) );
 
                if ( !pSim ) {
                        printf("Not enough memory.... please try later\n");
                                return;
                }
 
                mFILL_PARSER_SIM(pSim, csb, moduleId,
                                moduleId, PARSER_CMD_PROC_REQ, requestId,
                                csb->line, csb->lastchar, 0, 0 );
         
                        processCommand( pSim );

                if (  semTake( configparserSemId, WAIT_FOREVER ) == ERROR ) {
                 panic("parser_work:Cannot wait on Parser Semaphore\n");
                }
 
                pComSim = NULL;
        
                        mGET_WORK_SIM( pComSim, moduleId );
        
                        if ( pComSim ) {
                                if ( IS_INDICATION(pComSim->serviceid)) {
                                        parser_process_response( pComSim, requestId );
                }
                else if (IS_REQUEST(pComSim->serviceid)) {
                                printf("\nInvalid Request in Parser\n");
                }
                else
                                        printf("\ntParser:Invalid Service Obtained\n");
                                mFREESIM( pComSim );
                        }
                        if (  semTake(csb->configSemId, WAIT_FOREVER) == ERROR ) 
                                 panic("Cannot wait on CSB Configuration Semaphore\n");
                
                        requestId++;
                }
    }
    if (!end_found) {
         panic("Cannot complete Configuration!\n");
    }

    /* Mind our Free's & Q's */
    free_csb(&csb);

         semDelete( configparserSemId );

        imc_remove( moduleId );

}

void wait_for_system_load( void ) 
{
        if (  semTake( waitingSemId, WAIT_FOREVER ) == ERROR ) {
                printf("wait_for_system_load(%s): Cannot wait on waitingSemId\n", 
                            _TASK_NAME);
                return;
        } 

        waitingTaskCount++;

        if (  semGive( waitingSemId) == ERROR ) {
                printf("wait_for_system_load(%s): Cannot Give waitingSemId\n",
                           PS_TASK_NAME);
        } 
}


void parser_svcinit(void)
{
        parser_sema_init();
        parser_iowork_init();
}

void parser_configure(void)
{
        parser_callback_init();
        parser_task_init();
        parser_configure();
}

void parser_start(void)
{
        parser_check();
}

#endif

void ProcessCommand ( tpPARSERSIM pParserSim )
{
  tOCTET2 error    = PARSE_ERROR_NOERR;
  char *line       = pParserSim->command;
  char lastchar    = pParserSim->lastchar;
  parseinfo *csb   = pParserSim->csb;
  
  if ( csb == NULL ) {
    panic("processCommand : NULL CSB Obtained\n");
    return;
  }
  
  if (lastchar == '?') {
    csb->in_help = PARSER_HELP;
    csb->lastchar = '?';
  } else if (lastchar == '\t') {
    csb->in_help = PARSER_COMMAND_COMPLETION;
    csb->lastchar = '\t';
  } else {
    csb->in_help = PARSER_NO_HELP;
    csb->lastchar = '\0';
  }
  
  strcpy(csb->line, line);
  error = parse_cmd(csb, pParserSim);
}
 
void CliParserMain ( void )
{

  tpCOMSIM pComSim = NULL;
  
  while ( 1 )  {
    if (  semTake( parserSemId, WAIT_FOREVER ) == ERROR ) {
      panic("parser_work:Cannot wait on Parser Semaphore\n");
    }
    
    pComSim = NULL;
    
    mGET_WORK_SIM( pComSim, (tOCTET4)PARSER_MODULE );
    
    if ( pComSim ) {
      /* Check if it is Request or Indication */
      if ( IS_INDICATION(pComSim->serviceid)) {
        printf("Invalid Indication obtained in parser_work\n");
        /*      parser_send_response(pComSim, NULL, RC_ERR ); */
        mFREESIM( pComSim );
      }
      else if (IS_REQUEST(pComSim->serviceid)) {
        ProcessCommand(( tpPARSERSIM)pComSim);
      }
      else
        printf("\ntParser:Invalid Service Obtained\n");
    }
  }
}

void ParserSemInit(void)
{
  tOCTET4 moduleId = PARSER_MODULE;
  
  parse_chain_sem =  semMCreate( PS_DEFAULT_MUTEX_SEM_FLAG ) ;
  if ( parse_chain_sem == NULL ) {
    panic("parser_init:Cannot create parse chain semaphore\n");
  }
  
    systemloadingSemId =  semCCreate(PS_DEFAULT_COUNT_SEM_FLAG, 0 );
    if ( systemloadingSemId == NULL) { 
      panic("Cannot create System Initialization Semaphore\n");
    }
    
    /*    initSemId =  semCCreate(PS_DEFAULT_COUNT_SEM_FLAG, 0); */
    
    /* TO be removed */
    if ( imc_init() != RC_OK ) {
    panic("Cannot Initialize IMC\n");
    }
    
    parserSemId =  semCCreate( PS_DEFAULT_COUNT_SEM_FLAG , 0 );
    parserTaskId = taskIdSelf();

    if ( imc_add( &moduleId, parserSemId, parserTaskId ) == ERROR ) { 
      panic("Cannot add Parser to IMC\n");
    }
    
}


void ParserTaskInit (void)
{
  STATUS errcode = 0;
  
  errcode = taskSpawn("tParser",PARSER_TASK_PRIORITY, PARSER_TASK_OPTION,
                      PARSER_TASK_STACK_SIZE,
                      (FUNCPTR)CliParserMain,0,0,0,0, 0, 0, 0, 0, 0, 0);

  
  if (errcode == ERROR) {
    printf("\nCLI Parser Task spawn failed");
  } 
}

void parser_init ( void )
{
  config_prompt = TRUE;
                
  debug_init();
  parser_modes_init();
  command_parser_init();
  exec_parser_init();
  
  ip_address_init();
  /*
   * RAMS: commented this for now
  show_os_chain_init();
  
  nv_init();
  sysconf_init();
  */
}


void SysCommandCli (void)
{
  ParserSemInit ();
  parser_init();
  ParserTaskInit();
  ParserCallbackInit();
  BasicParserInit();
#if 0
  parser_iowork_init ();
  parser_configure();
  parser_check ();
#endif
}
