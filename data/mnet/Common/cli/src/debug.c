#include <vxworks.h>
#include <stdioLib.h>
#include <lstLib.h>
#include <stdlib.h>
#include <inetLib.h>
#include <timers.h>
#include <ctype.h>
#include <msgQLib.h>
#include <sigLib.h>
#include <logLib.h>

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
#include <../include/parser_input.h>
#include <../include/debug.h>
#include <../include/sys_debug.h>
#include <../include/parser_debug.h>
#include <../include/mutex.h>


#define __DECLARE_DEBUG_VARS__
#include <../include/parser_debug_flags.h>

/*
 * debug_all_flag is set whenever "debug all" is in effect. Should be
 * static, but I believe that screws up run-from-ROM in Cisco images...
 * anyway, external users should use debug_all_p() to interrogate this
 * flag.
 */

static boolean debug_all_flag = FALSE;
extern mutex_t *pPrintDebugMutex;

/*
 * debug_init
 * Initialize the debugging system
 */

void debug_init (void)
{
        debug_all_flag = FALSE;
        sys_debug_init();
}

/*
 * debug_all_p
 * Tells the caller whether "debug all" is in effect. Used (at least) at
 * subsystem initialization time to determine whether a subsystem should
 * turn on all its debug flags.
 */

boolean debug_all_p (void)
{
        return(debug_all_flag);
}


/*
 * debug_command
 * Generic handler for all manner of "debug" and "undebug" commands.
 * Any subsystem can use this routine to handle its debug flags as long
 * as it uses an array of debug_item_type to describe them and as long
 * as it sets up its parse chains correctly.
 *
 * csb->sense = TRUE for "debug", FALSE for "undebug".
 * csb->which = index into flag definition array.
 * GETOBJ(int,1) = mempointer to beginning of array... 
 */

void debug_command (parseinfo *csb)
{
        int i;
        const debug_item_type *debugarr;

        debugarr = (debug_item_type *)GETOBJ(vptr,0);

        if ( debugarr == NULL ) 
                return;

        /*
         * Sanity check the index -- paranoia is truly a terrible scourge in
         * these troubled times.
         */
        for (i=0; i < csb->which; i++) {
                if (debugarr[i].var == NULL) {
                        bad_parser_subcommand(csb, csb->which);
                        return;
                }
        }
        *(debugarr[csb->which].var) = csb->sense;

        /* Confirm the change to the user */
        debug_show_flag(*(debugarr[csb->which].var), 
                                        debugarr[csb->which].txt);
}

#if 0
/*
 * RAMS : commented this for now
 */
extern void debug_http_all();
extern void debug_icp_all();
extern void debug_log_all();
extern void debug_translog_all();
extern void debug_radius_all();
extern void debug_tacacs_all();
extern void debug_snmp_all();
extern void debug_stat_all();
extern void debug_wccp_all();
extern void debug_wi_all();
extern void debug_url_all();
extern void debug_rule_all();
extern void debug_sreg_all();
extern void debug_https_all();
extern void debug_authentication_all();
#endif

/*
 * debug_all_cmd
 * Handler for "debug all" and "undebug all". csb->sense is TRUE for "debug",
 * FALSE for "undebug"
 */

void debug_all_cmd (parseinfo *csb)
{
        /* If we're about to start spewing huge amounts of debugging crap
         * that might make the system unusable, make sure the user really
         * wants it to happen...
     */
        if ((csb->sense) && 
                (!yes_or_no("This may severely impact network performance. "
                                        "Continue ",FALSE, TRUE ))) 
                return;

        debug_all_flag = csb->sense;

        /* Declare an event so the subsystems can set their local flags */
        invoke_debug_all(debug_all_flag);

#if 0
        /*
         *RAMS: commented this for now
         */
        /* These modules haven't registered correctly */
        /* This hack is the simplest way to fix the problem */
        debug_http_all(csb->sense);
        debug_icp_all(csb->sense);
        debug_log_all(csb->sense);  
        debug_translog_all(csb->sense);
        debug_radius_all(csb->sense);
        debug_tacacs_all(csb->sense);
        debug_snmp_all(csb->sense);
        debug_stat_all(csb->sense);
        debug_wccp_all(csb->sense);
        debug_wi_all(csb->sense);
        debug_url_all(csb->sense);
        debug_rule_all(csb->sense);
        debug_sreg_all(csb->sense);
        debug_https_all(csb->sense);
        debug_authentication_all(csb->sense);
#endif
        /* Confirm to the user */
        printf("All possible debugging has been turned %s\n",
                   debug_all_flag ? "on" : "off");
}

/*
 * show_debug
 * Show the state of debugging everywhere in the system.
 */

void show_debug (void)
{
        if (debug_all_p()) {
                printf("\"debug all\" is in effect.\n");
        }
        invoke_debug_show();
}

/*
 * debug_show_flag
 * Show the value of a single debugging flag in a standardized format.
 * Used by subsystems that need to display individual flags, and when
 * a flag is set or reset. The "flag" argument is the boolean value of
 * the flag to display. "text" is a description of the debugging the 
 * flag controls. 
 */
void debug_show_flag (boolean flag, char *text)
{
    printf("%s debugging is %s\n", text, flag ? "on" : "off");
}

/*
 * debug_show_flags
 * Show a whole array of flags. Called by subsystems do do their parts
 * of "show debug". The "heading" argument contains a string that describes
 * the category into which all the flags to be displayed fall. It will
 * be displayed if and only if one or more of the flags is true. If the
 * heading is non-null, all the flags under it will be indented.
 */

void debug_show_flags (const debug_item_type *array, const char *heading, 
                       boolean printLevel )
{
        int i;
        boolean hasheading = FALSE;
        boolean hdisplayed = FALSE;
        boolean printNewLine = FALSE;

        if ((heading != NULL) && (*heading != '\0')) 
                hasheading = TRUE;
        for (i = 0; array[i].var != (boolean *) NULL; i++) {
                if (*(array[i].var)) {
                        if (hasheading && !hdisplayed) {
                                printf("%s:\n", heading);
                                hdisplayed = TRUE;
                        }
                        printf("%s%s debugging is %s", hasheading ? "  " : "",
                                   array[i].txt, (*array[i].var) ? "on" : "off");
                        printNewLine = TRUE;
                        if ( *(array[i].var) && printLevel )  {
                                printf(" (Level : %d)\n", *(array[i].var));
                        }
                        printf("\n");
                }
        }
        if ( printNewLine ) 
                printf("\n");
}

/*
 * generic_debug_all
 *
 * a generic routine that <routine_name>_debug_all can call
 * whenever anybody issues
 * a "debug all" or "undebug all" command... or whenever you want to
 * set the state of all the debug_items flags at once. The argument is
 * TRUE for "debug all", FALSE for "undebug all".
 */

void generic_debug_all (const debug_item_type *debug_items, boolean flag)
{
        while (debug_items->var) {
                *(debug_items->var) = flag;
                debug_items++;
        }
}

/*
 * generic_debug_show
 *
 * a generic routine that <routine_name>_debug_show can call
 * whenever anybody issues
 * a "show debug" command...
 */

void generic_debug_show (const debug_item_type *debug_items, 
                                                 const char* heading)
{
        debug_show_flags(debug_items, heading, FALSE );
}


STATUS generic_debug_init( const debug_item_type *debug_items , 
                                                   const char *heading )
{

        if ( add_debug_all(NULL, debug_items) == ERROR )  {
                return(ERROR);
        }
        if ( add_debug_show(NULL, debug_items, heading) == ERROR ) {
                return(ERROR);
        }
        return(OK);
}


int print_debug(char *fmt, ... )
{
        int fd = 0;
        char *buffer = NULL;
        int nbytes = 0;
        va_list ap;
        char *ptr = NULL;
        /*
         *following is commented by rams for now
         
         mutex_lock (pPrintDebugMutex); */
        va_start( ap, fmt );
        nbytes =  doprnt(NULL, NULL, fmt, ap, TRUE);

        if ( nbytes > 0 ) { 
                buffer = ( char *)PARSERmalloc(sizeof(char) * (nbytes + 1)) ;
                va_end( ap );
                if ( buffer ) { 
                        va_start( ap, fmt );
                        nbytes = doprnt(buffer, NULL, fmt, ap, TRUE);
                        va_end( ap );
                } else { 
                  /*
                   *following is commented by rams for now
                   
                   mutex_unlock (pPrintDebugMutex); */
                        return(-1);
                }
        }

        buffer[nbytes] = '\0';

        /*
         * RAMS: commented this for now
        for ( fd = 0; fd < MAX_FDS ; fd++ ) {
                if (  fdArray[fd] != 0 )  
                        write( fdArray[fd], buffer, strlen(buffer) );
        }
        */

        /* We need this to make sure that the buffer
         * does not contain "\r\n" ( example SMTP Response )
         *
         * Syslog will filter out additional "\n"s
         */
        if ( (ptr = strchr(buffer, '\r') ) != NULL ) { 
                *ptr = ' ';
        }
        logMsg("%s",buffer,0,0,0,0,0);

        PARSERfree( buffer );
        /*
         *following is commented by rams for now
         mutex_unlock (pPrintDebugMutex); */

        return(nbytes);
}
