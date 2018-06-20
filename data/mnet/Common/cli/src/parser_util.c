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



static  char netascii2cisco (char **buf)
{
    char ch;

    if ((**buf == '\r') &&
                (*(*buf+1) == '\n')) {
                *buf += 2;
                return('\n');
    }
    if ((**buf == '\r') &&
                (*(*buf+1) == '\0')) {
                *buf += 2;
                return('\r');
    }
    ch = **buf;
    (*buf)++;
    return(ch);
}

/*
 * BATCH_INPUT
 *
 * If there is an active batch input buffer which
 * contains more data, read a line from it and return TRUE.
 *
 * This function must operate much the same way as readline so
 * that the parser does not know the difference.
 */
boolean batch_input (parseinfo *csb, char *buf, int buflen,
                                         char *breakset, char *lastcharp)
{
    char *cp = buf;

    *lastcharp = '\0';
    
    if (csb->batch_buf && *csb->batch_offset) {
        /* Copy a line, stopping at any breakset character, 
         * NULL, or a full buffer 
         */
        while (*csb->batch_offset
                           && ! strchr(breakset, *csb->batch_offset)
                           && ((cp - buf) < (buflen - 1))) {
                        *(cp++) = netascii2cisco(&csb->batch_offset);
                }

                *lastcharp = netascii2cisco(&csb->batch_offset);
        *cp = '\0';
        return(TRUE);
    }
    return(FALSE);
}

/*
 * SAVE_LINE
 *
 * Save lines into a block buffer.  Used for comments, which are output at the
 * begining of the NV generation; generated command lines, which are output
 * after the comments; and help, which is output after a line is parsed.
 * For generation, make sure we have enough space to save both comments and
 * configuration.  If there isn't enough space in NV ram, truncate comments.
 *
 * Unlike an earlier version of save_line, newline characters aren't added
 * automatically - call save_line a second time if needed (since lines are
 * added sequentially into the buffer).  Tabs are used to delimit fields;
 * this feature is used only by the help strings.
 */

void save_line_init (save_buf *bp)
{
    if (bp->buf) {
        PARSERfree(bp->buf);
    }
    bp->buf = NULL;
    bp->size = 0;
    bp->used = 0;
}

void save_line (parseinfo *csb, save_buf *bp, const char *fmt, ...)
{
    va_list args;
    int n;
    int newsize;

        char *mybuf;

    if (!fmt) {
        return;
    }

    va_start(args, fmt);

        n = doprnt(NULL, NULLTTY, fmt, args, FALSE); 

    /*
     * We expect to be able to parse args again later for the real
     * write. However, some processors have to store state in the
     * va_list block for va_arg() reads so refresh the state
     * again by ending and starting again.
     */

    va_end(args);

    va_start(args, fmt);


    /* Add 1, for the null termination */
    while (( bp->used + n + 1) > bp->size) {
                newsize = bp->size + SABUFSIZE;
                mybuf = (char *)PARSERmalloc(newsize);
                memset((void *)mybuf, 0, newsize );
        if (mybuf) {
                bp->size = newsize;
                if (bp->buf == NULL) {
                                *mybuf = '\0';
                } else {
                                strcpy(mybuf, bp->buf);
                                PARSERfree(bp->buf);
                }
                } else {
                va_end(args); 
            return;
                }
        bp->buf = mybuf;
    }

        /* We do not need to use doprnt if we are printing 
     * to a non-NULL buffer, instead use vsprintf
     * doprnt has some issues with command completion in our
     * case.
     */

    n = vsprintf(bp->buf + bp->used, fmt, args); 

    bp->used += n;
    va_end(args);
}

boolean confirm (char *s)
{
    char c;
        int fd;
        boolean result = FALSE;

        fd = ioTaskStdGet(0, STD_IN);
        
        ioSetTerminalMode(fd, IO_RAW_MODE);

    while (TRUE) {
        printf("%s[confirm]", s);
                fflush(stdout);
        if ( read( fd, &c, 1 ) < 0 ) {
                        printf("Cannot read input\n");
                        goto DONE;
                }
        /* c &= 0177; */
                write(fd, &c, 1); 
                write(fd, "\r\n", 2); 
        switch (c) {
        case CH_NL:
        case CH_CR:
        case 'Y':
        case 'y':
                result = (TRUE);
                        goto DONE;

        case '?':
                printf("Press RETURN to continue, anything else to abort\n");
                break;
        default:
                result = (FALSE);
                        goto DONE;
                        
        }
    }
        
 DONE:
        ioSetTerminalMode(fd, IO_LINE_MODE);

        return(result);
}

/*
 * yes_or_no
 * Print a prompt and wait for a yes or no answer.  The supplied default
 * answer is used if a single return character is typed.  If usedef is
 * false set then no default is used and the user must type yes or no.
 */

#define MAXBUF 100

boolean yes_or_no (const char *prompt, boolean def, boolean usedef)
{
    char *c, buffer[MAXBUF];
        int fd = -1;

        fd = ioTaskStdGet(0, STD_IN);
 
        FOREVER { 

        if (prompt)
                printf("%s?[%s]: ", prompt, usedef ? def ? "yes" : "no" :
                           "yes/no");

                fflush(stdout);
        if ( read( fd, buffer, MAXBUF-1) < 0  )
                return(FALSE);

        c = deblank(buffer);

        if (usedef && ( null(c) || *c == '\n') ) 
                return(def);

        if (tolower(c[0]) == 'y')
                return(TRUE);

        if (tolower(c[0]) == 'n')
                return(FALSE);

        printf("%% Please answer 'yes' or 'no'.\n");
        }

    return(FALSE);
}

void
parser_return(parseinfo *csb, int errno_val)
{
        csb->return_val = FALSE;
        csb->errno_val = errno_val;
}
