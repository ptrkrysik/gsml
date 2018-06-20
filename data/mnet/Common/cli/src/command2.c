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
#include <../include/parser_actions.h>
#include <../include/parser_defs_config.h>

boolean config_dirty = FALSE;

#if 0
#include <parser/parser.h>
#include <parser/parser_actions.h>
#include <parser/parser_defs_exec.h>
#include <parser/parser_defs_config.h>
#include <parser/nv.h>
#include <cenetdb.h>
#include <dosfs_posix.h>
#include "assert.h"
#include <syslog.h>
#include <dosfs/uvfatUsrApiExt.h>


static STATUS write_running_to_file( int fd , uint);




/*
 * write_terminal
 * Show the current configuration on the terminal
 */

void write_terminal (parseinfo *csb)
{
        size_t len, cc;
    char *curcfg = NULL;

    /*
     * Dump the current configuration to our buffer
     */
    curcfg = nv_current_to_buffer(TRUE,
                                                                  PRIV_MAX, 
                                                                  (csb->flags & HIDE_PASSWORDS)); 
        if ( curcfg == NULL ) {
                return;
        }

        printf("\nCurrent configuration:");
        len = strlen(curcfg);
        cc = fwrite(curcfg, 1, len, stdout);
        assert(cc == len);
        PARSERfree(curcfg);
}


/*
 * nv_save_config_info()
 * This function is called by GUI for saving configuration information 
 * to NVRAM
 *
 */

void nv_save_config_info()
{
         syslog(LOG_NOTICE, "Configured from GUI");
     write_memory(NULL);
}


/*
 * write_erase
 * Jacket for EXEC "write erase" command
 */

void write_erase (parseinfo *csb)
{
    (void) nv_erase(TRUE);
        config_dirty = TRUE;
}

/*
 * write_memory
 * Jacket for EXEC "write memory" command
 */

void write_memory (parseinfo *csb)
{
    if (ok_to_write_nv()) {
                (void) nv_current_to_nv(TRUE);
                config_dirty = FALSE;
    } else
                printf("No action taken because command was not confirmed\n");
}

void ping_command (parseinfo *csb)
{
        char host[256];
        int numpkts = GETOBJ(int, 1);

        strncpy(host, GETOBJ(string,1), 256);


    ping( host, numpkts, 0 );
}


/* 
 * cfg_dirty -- return status of config.  If user has modified
 * config and not saved then return true else return false
 */
boolean cfg_dirty (void)
{
    return(config_dirty);
}




/* 
 * Write the configuration to TFTP / DISK 
 */
void write_config( parseinfo *csb ) 
{
        char fileName[MAX_FILENAME_LENGTH + 1 ];
        char remoteFileName[PATH_MAX + 1] = "";
        char tftpServerAddr[INET_ADDR_LEN + 1] = "";
        char buffer[256] = "";

        boolean tftpPut = FALSE;
        boolean tftpGet = FALSE;

        int fd = -1;
        int error = 0;
        int fileAccess = 0;

        int status = OK;

        /* For Disk Copy the file to open is the filename 
         * specified. For TFTP operation, need to create  
     * a temporary file locally and ask for Remote file
     * name also 
     */
        if ( ( csb->which == COPY_RUNNING_CONFIG_DISK )  || 
                ( csb->which == COPY_STARTUP_CONFIG_DISK )  || 
                ( csb->which == COPY_DISK_STARTUP_CONFIG )) { 

                strcpy(fileName, GETOBJ(string,1));

                if ( strcmp(fileName,"") == 0 ) { 
                        printf("Invalid Filename specfied\n");
                        return;
                }
        } else { 
                time_t current_time = 0;
        
                /* Temporary Filename creation to prevent
         * conflicts
         */
                if ( time( &current_time ) < 0 ) 
                        current_time = 0;
                sprintf(fileName, mVOLNAME".tmp.%d.%ld", taskIdSelf(),
                                current_time);

                startIOTimer(csb);
                status = getTftpInput(tftpServerAddr, remoteFileName ) ;
                if ( status != OK ) {
                        if ( status == RC_TIMEOUT ) {
                                setExecTimeout(csb);
                                stopIOTimer(csb);
                        }
                        return;
                }
                stopIOTimer(csb);
        } 

        /*
         * Make sure we do not create a file when we need
         * data from file, like copy disk startup-config
         */
        if ( (csb->which == COPY_DISK_STARTUP_CONFIG ) ) { 
                fileAccess = O_RDWR;
        } else {
                fileAccess = O_RDWR | O_CREAT;
        }

        /* Open the local file */
        if ( (fd = open(fileName, fileAccess , 0644)) < 0 ) { 
                if (!(tftpPut || tftpGet) ) 
                        printf("Cannot open file %s!\n", fileName);
                else 
                        printf("Cannot open temporary file for writing!\n");
        
                return;
        }

        error = 0 ;

        switch(csb->which ) { 
        
        case COPY_DISK_STARTUP_CONFIG:
                printf("Writing Startup Configuration from %s...", fileName);
                /* 
                 *  This is defined in nv_common.c. Writes the configuration
                 *  from NVRAM to the file descriptor 'fd'
                 */ 
                if ( write_nv_config_from_fd( fd ) == ERROR ) {
                        printf("[Failed]\n") ;
                } else { 
                        printf("[OK]\n");
                        printf("You need to reboot the box for the configuration to"
                   " take effect\n");
                }
                close(fd);
                break;

        case COPY_STARTUP_CONFIG_DISK:
                printf("Writing startup configuration to %s...", fileName);
                csb->flags |= CONFIG_FILE;
                SETOBJ(int,2)= fd;
                nv_review( csb );
                csb->flags &= ~CONFIG_FILE;
                if ( GETOBJ(int, 2) == FALSE ) 
                        printf("[OK]\n");
                else
                        printf("[Failed]\n");
                close( fd );
                break;

        case COPY_RUNNING_CONFIG_DISK:
                printf("Writing Current configuration to %s...", fileName);
                if ( write_running_to_file( fd , ( csb->flags & HIDE_PASSWORDS) )
                        == OK ) 
                        printf("[OK]\n");
                else
                        printf("[Failed]\n");
                close( fd );
                break;

        case COPY_RUNNING_CONFIG_TFTP:
                printf("Writing Current configuration to %s:%s...", tftpServerAddr,
               remoteFileName);
                if ( write_running_to_file( fd, ( csb->flags & HIDE_PASSWORDS ) )
                        == ERROR ) {
                        printf("[Failed] ( Configuration creation error)\n");
                        error = TRUE;
                }
                close( fd );
                tftpPut = TRUE;
                tftpGet = FALSE;
                break;

        case COPY_TFTP_STARTUP_CONFIG:
                /* The entire TFTP Operation is done at the end */
                printf("Writing Startup Configuration from %s:%s...", tftpServerAddr,
                           remoteFileName);
                tftpPut = FALSE;
                tftpGet = TRUE;
                break;

        case COPY_STARTUP_CONFIG_TFTP:
                printf("Writing Startup configuration to %s:%s...", tftpServerAddr,
               remoteFileName);
                csb->flags |= CONFIG_FILE;
                SETOBJ(int,2)= fd;
                nv_review( csb );
                csb->flags &= ~CONFIG_FILE;
                close( fd );
                error = GETOBJ(int, 2);
                tftpPut = TRUE;
                tftpGet = FALSE;
                break;

        default:
                break;

        }

        if ( tftpPut  || tftpGet ) { 
        
                /* Determine if it is 'get' or 'put' operation */
                strcpy(buffer, ( tftpPut == TRUE )?"put":"get");

                if ( !error ) { 
                        
                        /* The local temporary file would not have been opened
                         * for TFTP GET Operation yet or would have been closed 
                         * in case of TFTP PUT Operation
                         */
                        if ( (fd = open(fileName, O_RDWR | O_CREAT, 0644)) < 0 ) { 
                                printf("[Failed] ( Temporary file creation error)\n");
                        } else if ( tftpCopy( tftpServerAddr, 0, remoteFileName, 
                                                                  buffer, "binary", fd ) == ERROR ) {
                                printf("[Failed] ( TFTP Error)\n");
                        } else if ( tftpGet == TRUE ) { 

                                /* We close the 'fd' just to make sure the buffers are
                                 * flushed to the file
                                 */
                                close(fd);

                                /* Got the configuration file from TFTP Server. Need
                                 * to update the startup-configuration from the
                                 * temporary file
                                 */
                                if (!((( fd = open(fileName, O_RDWR | O_CREAT, 0644)) > 0 ) &&
                                          ( write_nv_config_from_fd(fd) == OK ))) {
                                        printf("[Failed]\n");
                                } else {
                                        printf("[OK]\n");
                                        printf("You need to reboot the box for the configuration "
                                                   "to take effect\n");
                                }
                        } else 
                                printf("[OK]\n");
                }
                close( fd );

                /* Remove the temporary file */
                if ( fileName ) 
                        remove( fileName );
        } 
}

/*
 * Write the running configuration to a file with 
 * file descriptor fd
 */
static STATUS write_running_to_file( int fd , uint flags)
{
    char *curcfg = NULL;
        STATUS result = ERROR;

        /*
         * Dump the current configuration to our buffer
     */
        curcfg = nv_current_to_buffer( FALSE,  PRIV_MAX, flags ) ;


        if ( curcfg != NULL ) {
                if ( write(fd, curcfg, strlen(curcfg)) < strlen(curcfg)) { 
                        result = ( ERROR ) ;
                } 
                PARSERfree(curcfg);
                result = OK;
        }
        return( result );
}


int getTftpInput( char *pTftpServer, char *pFileName )
{
        char server[256] = "";
        char fileName[PATH_MAX + 1] = "";
        char buffer[1024] = "";

        struct hostent *pHostEnt = NULL;
        struct hostent hostEntry;
        struct in_addr serverAddr;

        int nbytes = 0;
        int error = 0;
        u_long addr;
        
        
        printf("TFTP Server Address: ");
        if ( ( nbytes = 
                   read( STD_IN, server, (sizeof(server) - 1)) ) <= 0) { 
                if ( errnoOfTaskGet(0) == S_ioLib_CANCELLED ) {
                        return( RC_TIMEOUT );
                } else { 
                        return(ERROR );
                }
        } 
        
        server[nbytes - 1] = '\0';

        if ( nbytes >= 256 ) { 
                printf("Hostname too long\n");
                return(ERROR);
        }

        /* Verify if the Server specified is valid
         * Check first to see if it is HOSTNAME. If
         * it is not valid, check to see if it is a
         * HOSTADDRESS. If both fail, then it is considered
         * an invalid address
         */
        if ( ( pHostEnt = 
                   gethostbyname_r(server, &hostEntry, buffer, 
                                                   sizeof(buffer), &error )) ==  NULL) { 
                if ( ( pHostEnt = gethostbyaddr_r((char *)&addr, 4, AF_INET, 
                                                                                  &hostEntry, buffer, 
                                                                                  sizeof(buffer), 
                                                                                  &error )) == NULL ) { 
                        printf("Cannot resolve the TFTP Server Address "
                                   "(Errno %d)\n", error);
                        return(ERROR);
                }
        }
        
        bcopy(pHostEnt->h_addr_list[0], (char *)&serverAddr, 
                  pHostEnt->h_length);
        
        inet_ntoa_b(serverAddr,  pTftpServer);
        
        printf("Remote Filename: ");
        if ( ( nbytes = 
                   read( STD_IN, fileName, 
                                 PATH_MAX ) ) <= 0) { 
                if ( errnoOfTaskGet(0) == S_ioLib_CANCELLED ) {
                        return( RC_TIMEOUT );
                } else { 
                        return(ERROR );
                }
        } 
        fileName[nbytes - 1] = '\0';
        
        if ( nbytes > PATH_MAX ) { 
                printf("Filename too long\n");
                return(ERROR);
        }
        
        strcpy(pFileName, fileName);
        
        return(OK);
}

#ifdef INCLUDE_TRACEROUTE
/************************************************
 *
 * func: traceroute_command
 *
 * desc: cli traceroute callback func
 *
 ************************************************/
extern int traceroute(char *host);

void traceroute_command (parseinfo *csb)
{
        addrtype *pHost = GETOBJ(paddr, 1);
        char host[INET_ADDR_LEN + 1 ];
        struct in_addr address;

        if ( martian(ntohl(pHost->ip_addr), MARTIAN_FLAG_IS_HOST) == TRUE ) {
                printf("Not a valid Host Address\n");
                return;
        }

        address.s_addr = pHost->ip_addr;
        
        inet_ntoa_b(address, host );

    traceroute( host);
        return;
}
#endif
#endif

void exit_config_command( parseinfo *csb )
{
    parser_mode *alt_mode;
        char *username = NULL;
        char unknown_name[]="Unknown";
 
        csb->giveConfigSem = TRUE;
    if (csb->nvgen) {
                if (csb->which == CMD_END) {
                /* Reset buffer size 
                         * to always save room for "\nend".
                         
                         nv_write(TRUE, "%s\n", csb->nv_command); */
                }
                return;
    }
    alt_mode = get_alt_mode(csb->mode);
    while (alt_mode) {
                (void) mode_save_vars(csb);
                csb->mode = alt_mode;
                alt_mode = get_alt_mode(csb->mode);
    }
 
    csb->mode = exec_mode;

#if 0
    /* RAMS commented this for now */
        if (!systemloading) {

                username = getNameByUid(csb->uid);
                if (username == NULL)
                        username = unknown_name;

                if (csb->inFd == consoleFd) {
                        syslog(LOG_NOTICE, "Configured from console by %s", username); 
                } else {
                        syslog(LOG_NOTICE, "Configured from telnet session by %s", 
                                   username);
                }
        }
#endif
}


/*
 * Display the '?' help text for the parser
 */
void
parser_help(parseinfo *csb)
{
        csb->MoreStatus = print_buf(csb->help_save, 
                                                                FALSE, 
                                                                !(csb->flags & CONFIG_HTTP), 
                                                                NULL, 
                                                                0
                                                                );
        csb->help_save.used = 0;
        csb->visible_ambig.ambig[0] = '\0';
        csb->visible_ambig.ambig_save[0] = '\0';
        csb->hidden_ambig.ambig[0] = '\0';
        csb->hidden_ambig.ambig_save[0] = '\0';
}


/*
 * enable_command
 */

void enable_command (parseinfo *csb)
{
    uint level = GETOBJ(int,1);

    /*
     * If this was really the disable command, turn off wizardliness
     * and return.
     */
    if (csb->which == CMD_DISABLE) {
                if (level <= csb->priv) {
                        csb->priv = level;
                        /* SET_PRIV(stdio->privilege, level); */
                        if (parser_priv_debug) {
                                printf("\nPrivilege level set to %d",level);
                                /* CUR_PRIV(stdio->privilege)); */
                        
                        }
                } else {
                        printf("\nNew privilege level must be "
                                   "less than current privilege level");
                }
                return;
    }
    /*
     * RAMS: commented this for now
     
        if ( !( isSuperUser( csb->uid ) ) )  {
                printf("You need administrative privilege to enter privileged mode\n");
        } else { 
        csb->priv = level;
        }
    */
    csb->priv = level;
    return;
}

/* 
 * Write the configuration to TFTP / DISK 
 */
void write_config( parseinfo *csb ) 
{
  printf("\n Write config called");
}


/*
 * manual_configure
 * Query user for configuration file
 */
void manual_configure (parseinfo *csb)
{
  csb->priv = 0xf;
    /*
     * If we are not doing a "conf term" ("conf net" or "conf mem"),
     * and we are not root, dont' let through
     */
    if ((csb->which != PARSER_CONF_TERM) &&
                (csb->priv != PRIV_ROOT)) {
                printf("\nInvalid privileges");
                return;
    }


    switch (csb->which) {

    case PARSER_CONF_TERM:
                csb->mode = config_mode;
                printf("Enter configuration commands, one per line. "
                           "End with CNTL/Z\n\n");
                csb->break_chars = CONFIG_BREAKSET;
                csb->batch_buf = NULL;
                csb->resolvemethod = RES_MANUAL;
                csb->flags = CONFIG_TERM;
                config_dirty = TRUE;
                csb->giveConfigSem = TRUE;
                return;

    default:
                bad_parser_subcommand(csb, csb->which);
                break;
    }
}
