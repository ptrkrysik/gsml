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



enum {
    AUTOMORE_ABORT,
    AUTOMORE_LINE,
    AUTOMORE_PAGE,
};

#ifdef NOTHING

/*
 * automore_more
 * Asks the question " --More-- ", and returns:
 * 0 if user didn't want more output
 * 1 if user wants one more line (input was <cr>)
 * 2 if user wants another page (input was a SPACE, "Y", or "y".
 * Assumes TRUE always if screen length is zero.
 */

static inline int
automore_more (void)
{
    char c;

    if (!stdio)
        return(AUTOMORE_ABORT);

    if (stdio->tty_length == 0)
        return(AUTOMORE_PAGE);
    LOG_SYNC_REPRINT_REG(stdio,"\n --More-- ");
    while (TRUE) {
        printf("\n --More-- ");
        flush();
        c = getcwait(stdio);
        c &= 0177;
        /*       \-\-\e\r\o\M\-\-\--More-- \ \ \ \ \ \ \ \ \ */
        printf("\b\b\b\b\b\b\b\b\b        \b\b\b\b\b\b\b\b\b");
        switch (c) {
          case '?':
            printf(
                           "Press RETURN for another line, SPACE for another"
               " page, anything else to quit"
                           );
            break;
        case 040:               /* SPACE, Y, or y */
        case 'Y': 
        case 'y': 
            LOG_SYNC_REPRINT_END(stdio);
            return(AUTOMORE_PAGE);
        case '\r':              /* return */
            LOG_SYNC_REPRINT_END(stdio);
            return(AUTOMORE_LINE);
        default:                /* Anything else */
            LOG_SYNC_REPRINT_END(stdio);
            return(AUTOMORE_ABORT);
        }
}
}

#endif

/*
 * doprintc
 * Handle a single character for _doprnt
 */

void doprintc (
                           char **cp,
                           tt_soc *tty,
                           char ch)
{
    if (*cp) {                                                  /* if writing to a string */
                *(*cp)++ = ch;                                  /* don't do any formatting */
                return;
    }

    if (tty == NULLTTY) {                               /* If not really doing I/O */
                return;                                                 /* just get out */
    }

    putc(ch, stdout);

#ifdef NOTHING

    /*
     * Allow console output to be displayed directly to the
     * console before the system is up.
     */
    if ((!(systeminit_flag || system_configured))
                || tty == CONTTY) {
                console_putc(ch & 0x7f);
                return;
    }
    ch &= tty->tty_execmask;                    /* mask inappropriate bits */
    if (tty->statbits & LASTWASCR) {    /* was the last one a CR? */
        tty->statbits &= ~LASTWASCR;    /* yes, clear flag */
                if (ch == 012)                                  /* flush char if it is a LF */
                        return;                                         /* we already printed it */
    }

    /*
     * If the automore flush flag is set, and automore is enabled, and it
     * isn't being supressed, don't do any output.
     */

    if (tty->automore_flush && tty->automore_enabled && !tty->automore_supress)
                return;

    if ((uchar)ch < 040) {
                switch (ch) {
            case 07:                                            /* pass bell, tab, backspace */
            case 011:
            case 010:
                        break;
            case 015:
                        tty->statbits |= LASTWASCR; /* if next is LF, flush it */
            case 012:
                        if (tty->automore_enabled && !tty->automore_supress &&
                                tty->tty_length) {
                                tty->automore_count++;
                                if ((tty->automore_count >= (tty->tty_length-1)) &&
                                        (tty->automore_count > 1)) {

                                        /*
                                         * We are at the end of the page. Supress recursive
                                         * processing and set the flush flag based on
                                         * whether they want more output or not. Reset
                                         * the count and then recursively print out the
                                         * header.
                                         *
                                         * We fall through to print out the original
                                         * newline from the input string. This works
                                         * because all strings, including the header,
                                         * follow the convention of a leading newline.
                                         */

                                        tty->automore_supress++;
                                        switch (automore_more()) {
                                        case AUTOMORE_LINE:     /* one more line */
                                                /* subtraction values include the offset
                                                 * needed for the above test against
                                                 * tty->tty_length-1.
                                                 */
                                                tty->automore_count = tty->tty_length - 2;
                                                break;
                                        case AUTOMORE_PAGE:     /* one more page */
                                                if (tty->automore_header) {
                                                        printf("%s", tty->automore_header);
                                                        /*
                                                         * Output the CR we just swallowed
                                                         */
                                                        printc('\n');
                                                        tty->automore_count = 1;
                                                } else {
                                                        tty->automore_count = 0;
                                                }
                                                break;
                                        default:
                                        case AUTOMORE_ABORT: /* abort */
                                                tty->automore_count = 0;
                                                tty->automore_flush = TRUE;
                                                break;
                                        }
                                        tty->automore_supress--;
                                        /*
                                         * Don't output LF or CR since we've
                                         * cleared the current line.
                                         */
                                        return;
                                }                         
                        } else {
                                /*
                                 * No paging, so allow the process to suspend
                                 * if it's generating reams of output.
                                 */
                                process_may_suspend();
                        }
                        putc(tty,015);                          /* Output a CR */
                        ch = 012;                                       /* And finish with a LF */
                        break;
            case '\033':
                        if (tty->tty_international) {
                                break;
                        }
                        /* FALL THROUGH */
            default:                                                            /* all else to arrow format */
                        putc(tty,'^');
                        ch += '@';
                        break;
                }
    }
    putc(tty,ch);


#endif

}


#ifdef NOTHING

/*
 * printf - cisco convention
 * Print a format string on standard output
 */

int printf (const char *fmt, ...)
{
    va_list ap;

    if (stdio) {
                va_start(ap, fmt);
                return(_doprnt(NULL, stdio, fmt, ap, FALSE));
                va_end(ap);
    } else {
                errmsg(&msgsym(BADPRINT,SYS), "printf");
                return(0);
    }
}

/*
 * vprintf - cisco convention
 * Print a format string on standard output using va_args
 */
int vprintf (const char *fmt, va_list ap)
{
    if (stdio) {
                return(_doprnt(NULL, stdio, fmt, ap, FALSE));
    } else {
                errmsg(&msgsym(BADPRINT,SYS), "vprintf");
                return(0);
    }
}

/*
 * printc
 * Print a character on the primary output
 */

void printc (char ch)
{
    char *n = NULL;

    if (stdio) {
                doprintc(&n, stdio, ch);
    } else {
                errmsg(&msgsym(BADPRINT,SYS), "printc");
    }
}

/*
 * ttyprintc
 * Print a character on the specified output
 */

void ttyprintc (tt_soc *tty, char ch)
{
    char *n = NULL;

    doprintc(&n, tty, ch);
}

/*
 * ttyprintf - cisco convention
 * printf to other than the standard terminal
 */

int ttyprintf (tt_soc *tty, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    if (tty != CONTTY && tty != NULLTTY) {
                tty->automore_supress++;
                i = _doprnt(NULL, tty, fmt, ap, FALSE);
                tty->automore_supress--;
                return(i);
    } else
                return(_doprnt(NULL, tty, fmt, ap, FALSE));
    va_end(ap);
}

/*
 * _ttyprintf - like fprintf, only no automore supression
 */

int _ttyprintf (tt_soc *tty, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    return(_doprnt(NULL, tty, fmt, ap, FALSE));
    va_end(ap);
}

/*
 * sprintf - cisco convention
 * String version of printf.
 */

int sprintf (char *cp, const char *fmt, ...)
{
    va_list ap;
    int count;

    va_start(ap, fmt);
    count = _doprnt(cp, NULL, fmt, ap, FALSE);
    va_end(ap);
    cp[count] = '\0';
    return(count);
}

/*
 * vsprintf - cisco convention
 * String version of printf using varargs as argument
 */
int vsprintf (char *cp, const char *fmt, va_list ap)
{
    int count;

    count = _doprnt(cp, NULL, fmt, ap, FALSE);
    cp[count] = '\0';
    return(count);
}

/* 
 * ansi_sprintf - ANSI convention
 */

int ansi_sprintf (char *cp, const char *fmt, ...)
{
    va_list ap;
    int count;

    va_start(ap, fmt);
    count = _doprnt(cp, NULL, fmt, ap, TRUE);
    va_end(ap);
    cp[count] = '\0';
    return (count);
}


/*
 * console_message
 * Print a console message using the bootstrap monitor routines
 */

void console_message (const char *str)
{
    /* emt_call() is used here instead of console_putc() so that 
     * it can be called from sched_cputype() when running on the
     * wrong hardware.
     */
    while (*str)
                emt_call(EMT_PUTCHAR, *str++);
}

/*
 * flush
 * flush pending output to stdio
 */

void flush (void)
{
    if (stdio)
                (void)startoutput(stdio);
}

/*
 * tt_flush
 * Flush pending output to given tt_soc *
 * Used to be called fflush, conflicting with UNIX stdio.
 */

void ttyflush (tt_soc *tty)
{
    if (tty)
                (void)startoutput(tty);
}


#endif

/*
 * _doprnt - master printf routine.
 * The conversions %o, %x, %d, %u, %s, %c, and %% are implemented.
 * The -, 0, *, and # options are supported for numbers only.
 * The <width> option is supported for numbers, strings, and characters.
 * The number of characters printed is returned.
 *
 * E/e prints a dotted 48 bit Ethernet address. Takes a string pointer.
 *
 * T/t prints a terminal line number in octal or decimal.  Integer arg.
 *
 */

int doprnt (char *cp, tt_soc *tty, const char *fmt, va_list argp,
                        boolean do_precision)
{
    uchar c, *q;
    int radix, d, length;
    int charcount;
    int digitcount;
    ulong n;
    uchar digits[25];
    int hash,rjust,width,size,temp,plus,precision;
    char fill;
    /* ulong host; */
    boolean spaces;
    boolean is_negative;
    ulonglong nn = 0;

    /*
     * Protect against being called incorrectly from interrupt level.
     * Also, if we were passed a null terminal pointer and string
     * pointer, figure that stdio hasn't been set up yet and do the I/O
     * to the console
     */

#ifdef NOTHING

    if (tty == NULL) {
                if (cp == NULL) {
                        if (get_interrupt_level() != 0) {
                                errmsg(&msgsym(INTPRINT,SYS));
                                return(0);
                        }
                        tty = console;
                }
    } else if ((tty != CONTTY) && systeminit_flag) {
                if (get_interrupt_level() != 0) {
                        errmsg(&msgsym(INTPRINT,SYS));
                        return(0);
                }
    }

#endif

    nn = 0;
    charcount = 0;                                              /* nothing printed yet */
    while ((c = *fmt++)) {                              /* work until end of string */
                if (c != '%') {
                        charcount++;
                        doprintc(&cp,tty,c);            /* output normal characters */
                } else {
                        width = -1;                                     /* default width is unspecified */
                        length = 0;                                     /* no length yet */
                        fill = ' ';                                     /* default fill is space */
                        rjust = 0;                                      /* default is left justified */
                        hash = 0;                                       /* default is no leading 0 or 0x */
                        size = 0;                                       /* default is that args are short */
                        plus = 0;                                       /* default is no plus sign */
                        precision = -1;                         /* no precision yet */
                        if (*fmt == '-') {
                                rjust++;                                /* want right justified */
                                fmt++;
                        }
                        if (*fmt == '+') {
                                plus++;                                 /* add a plus if positive integer */
                                fmt++;
                        }
                        if (*fmt == '0') {
                                fill = '0';                             /* set fill char to '0' */
                                fmt++;
                        }
                        if (*fmt == '#') {
                                hash++;                                 /* want leading 0 or 0x */
                                fmt++;
                        }
                        if (*fmt == '*') {                      /* look to args for width */
                                width = va_arg(argp, int);
                                fmt++;
                        } else                                          /* width possibly specified */
                                while ((*fmt >= '0') && (*fmt <= '9')) {
                                        if (width == -1)
                                                width = 0;
                                        width = width*10 + (*fmt++ - '0');
                                }

                        if (do_precision) {                     /* Check for precision */
                                if (*fmt == '.') {              /* precision possibly specified */
                                        fmt++;
                                        precision = 0;
                                        if (*fmt == '*') {      /* look to args for precision */
                                                precision = va_arg(argp, int);
                                                if (precision < 0) /* check for nonsense precision */
                                                        precision = 0;
                                                fmt++;
                                        } else
                                                while ((*fmt >= '0') && (*fmt <= '9'))
                                                        precision = precision*10 + (*fmt++ - '0');
                                }
                        } else
                                precision = width;      /* cisco convention derives */
                        /* precision from width */

                        if (*fmt == 'l') {                      /* use Longs instead of shorts */
                                size++;
                                fmt++;
                        }
                        if (*fmt == 'l') {                      /* use Long Longs instead of longs */
                                size++;
                                fmt++;
                        }

                        if (!*fmt)
                                break;                                  /* Check for trailing nulls - bad format */

                        switch (*fmt++) {                       /* get char after the %, switch */

#ifdef NOTHING

                        case 'B':                                       /* hexadecimal or ethernet */
                        case 'b': 
                                temp = va_arg(argp, int);
                                if (temp == APOLLO_TYPE) {
                                        q = va_arg(argp, uchar *);
                                        bcopy(&q[2], (char *)&host, 4);
                                        if (cp) {
                                                length = sprintf(cp, "%x", host);
                                                cp += length;
                                        } else
                                                length = _ttyprintf(tty,"%x", host);
                                        charcount += length;
                                        break;
                                }               
                /* Else fall thru to Ethernet style address */
                        case 'e':                                       /* print an Ethernet address */
                        case 'E':                                       /* 64-bit Ethernet style SMDS address */
                                q = va_arg(argp, uchar *);
                                for (d = 0; d <= 2; d++) {
                                        doprintc(&cp,tty,tohexchar((*q)>>4));
                                        doprintc(&cp,tty,tohexchar(*q++));
                                        doprintc(&cp,tty,tohexchar((*q)>>4));
                                        doprintc(&cp,tty,tohexchar(*q++));
                                        if (d < 2)
                                                doprintc(&cp,tty,'.');
                                }
                                charcount += 14;
                                /*
                                 * Do E.164 style by adding 16 more bits.
                                 */
                                if (*(char *)(fmt-1) != 'E' || *q == 0xff) {
                                        break;
                                }
                                doprintc(&cp,tty,'.');
                                doprintc(&cp,tty,tohexchar((*q)>>4));
                                doprintc(&cp,tty,tohexchar(*q++));
                                doprintc(&cp,tty,tohexchar((*q)>>4));
                                doprintc(&cp,tty,tohexchar(*q++));
                                charcount += 5;
                                break;

                        case 't':                                       /* terminal line number */
                                if (decimaltty_enable)
                                        goto do_decimal;
                                else
                                        goto do_octal;
                                break;

#endif

                        case 'o':                                       /* octal */
                        case 'O': 
                                /*do_octal:*/       radix = 8;
                                if (precision == -1)
                                        precision = 1;
                                if (size > 1) {
                                        nn = va_arg(argp, ulonglong);
                                        n = (nn != 0);
                                } else if (size) {
                                        n = va_arg(argp, ulong);
                                } else {
                                        n = va_arg(argp, uint);
                                }
                                if (hash && (n > 0)) {
                                        doprintc(&cp,tty,'0');
                                        length++;
                                        charcount++;
                                }
                                goto compute;

#ifdef NOTHING

                        case 'q':                                       /* hexadecimal or decimal */
                        case 'Q': 
                                temp = va_arg(argp, int);
                                if ((temp == XNS_TYPE) || (temp == UBXNS_TYPE))
                                        goto do_unsigned;
                
                                /* else fall thru to hex */
                
                        case 'x':                                       /* hexadecimal */
                        case 'X': 
                                radix = 16;
                                if (precision == -1)
                                        precision = 1;
                                if (size > 1) {
                                        nn = va_arg(argp, ulonglong);
                                        n = (nn != 0);
                                } else if (size) {
                                        n = va_arg(argp, ulong);
                                } else {
                                        n = va_arg(argp, uint);
                                }
                                if (hash) {
                                        doprintc(&cp,tty,'0');
                                        doprintc(&cp,tty,'x');
                                        length += 2;
                                        charcount += 2;
                                }
                                goto compute;
#endif

                
                        case 'u':                                       /* unsigned decimal */
                        case 'U':
                                /*do_unsigned:*/        radix = 10;
                                if (precision == -1)
                                        precision = 1;
                                if (size > 1) {
                                        nn = va_arg(argp, ulonglong);
                                        n = (nn != 0);
                                } else if (size) {
                                        n = va_arg(argp, ulong);
                                } else {
                                        n = va_arg(argp, uint);
                                }
                                goto compute;
                
                        case 'd':                                       /* signed decimal */
                        case 'D': 
                                /* do_decimal:    */  radix = 10;
                                if (precision == -1)
                                        precision = 1;
                                if (size > 1) {
                                        nn = va_arg(argp, longlong);
                                        n = (nn != 0);
                                        is_negative = ((longlong)nn < 0);
                                } else if (size) {
                                        n = va_arg(argp, long);
                                        is_negative = ((long)n < 0);
                                } else {
                                        n = va_arg(argp, int);
                                        /*
                                         * For backward compatibility reasons, this test for a
                                         * negative number is based upon a 'long', not an 'int'.
                                         */
                                        is_negative = ((long)n < 0);
                                }
                                if (is_negative) {
                                        doprintc(&cp, tty, '-');
                                        length++; charcount++;
                                        n = -n;
                                } else if (plus) {
                                        doprintc(&cp, tty, '+');
                                        length++; charcount++;
                                }
                        compute: 
                                digitcount = 0;
                                if (n == 0) {
                                        q = &digits[0];
                                        if (!do_precision) {
                                                *q++ = '0';
                                                length++;
                                        }                       
                                } else if (nn != 0) {
                                        /*
                                         * Do not attempt to collapse this into the following case for
                                         * 32 bit numbers.  All you will accomplish is to use 64bit
                                         * arithmetic for all numbers, which is slower in general, but
                                         * is much slower for division.
                                         */
                                        printf("Printing Decimal....???parser_printf.c\n");
#ifdef MOD_ERRORS
                                        for (q = &digits[0]; nn != 0; nn = nn / radix) {
                                                d = nn % radix;  length++; digitcount++;
                                                *q++ = d + (d < 10 ? '0' : '7');
                                        }
#endif 

                                } else
                                        for (q = &digits[0]; n != 0; n = n / radix) {
                                                d = n % radix;  length++; digitcount++;
                                                *q++ = d + (d < 10 ? '0' : '7');
                                        }
                                if (do_precision && (precision > 0)
                                        && (digitcount < precision))
                                        for (d = digitcount; d < precision; d++) {
                                                *q++ = '0';
                                                length++;
                                        }
                                q--;                                    /* back up the pointer a bit */
                                if ((rjust || fill == '0') && width != -1 && (length < width))
                                        for (d = 0; d < (width - length); d++) {
                                                doprintc(&cp,tty,fill); /* leading fill */
                                                charcount++;
                                        }
                                while (q >= &digits[0]) {
                                        doprintc(&cp,tty, *q--);
                                        charcount++;
                                }
                                if (!rjust && fill != '0' && width != -1 && (length < width))
                                        for (d = 0; d < (width-length); d++) {
                                                doprintc(&cp,tty,' '); /* trailing fill - spaces*/
                                                charcount++;
                                        }
                                break;
                
                        case 'c':                                       /* character */
                                if (rjust && width != -1)
                                        for (d = 0; d < width-1; d++) {
                                                doprintc(&cp,tty,' '); /* leading fill - spaces */
                                                charcount++;
                                        }
                                temp = va_arg(argp, int);
                                doprintc(&cp,tty,temp);
                                charcount++;
                                if (!rjust && width != -1)
                                        for (d = 0; d < width-1; d++) {
                                                doprintc(&cp,tty,' '); /* trailing fill - spaces*/
                                                charcount++;
                                        }
                                break;
                
                        case 's':                                       /* string */ 
                        case 'S':                                       /* string parsed by STRING macro */ 
                                q = va_arg(argp, uchar *);
                                if (q == NULL)                  /* insure proper formatting */
                                        q = "";
                                spaces = FALSE;                 /* No spaces */
                                temp = (int) q;                 /* stash copy of pointer */
                                while (*q) {
                                        if (*(fmt-1) == 'S') {
                                                if (*q == ' ') {
                                                        spaces = TRUE; /* Contains spaces */
                                                } else {
                                                        if (*q == '"') {
                                                                length++; /* Escape quotes */
                                                        } else {
                                                                if ((*q >= 127) ||
                                                                        (*q == '\r') ||
                                                                        (*q == '\n')) {
                                                                        length += 3;/*Number escape sequence */
                                                                }
                                                        }
                                                }
                                        }
                                        ++q;
                                        ++length;                       /* figure its length */
                                }
                                if (spaces) {
                                        length += 2;
                                }
                                if (rjust && (width != -1) && (length < width)) {
                                        for (d = 0; d < (width - length); d++) {
                                                doprintc(&cp,tty,fill); /* leading fill */
                                                charcount++;
                                        }
                                }
                                if (spaces) {
                                        doprintc(&cp, tty, '"');
                                        charcount++;
                                }
                                q = (uchar *) temp;             /* get pointer again */
                                while (*q != 0) {
                                        if ((precision != -1) &&
                                                ((int)(q - (uchar *)temp) >= precision)) {
                                                break;
                                        }
                                        if (*(fmt-1) == 'S') {
                                                if ((*q < 127) && (*q != '\r') && (*q != '\n')) {
                                                        if (*q == '"') {
                                                                doprintc(&cp, tty, '\\');
                                                                charcount++;
                                                        }
                                                        doprintc(&cp,tty, *q);
                                                        charcount++;
                                                } else {
                                                        doprintc(&cp, tty, '\\');
                                                        doprintc(&cp, tty, 'x');
                                                        doprintc(&cp, tty, (((*q/16) < 0x0A) ? 
                                                                                                ((*q/16) + '0') :
                                                                                                ((*q/16) + 'A' - 0x0A)));
                                                        doprintc(&cp, tty, (((*q%16) < 0x0A) ?
                                                                                                ((*q%16) + '0') :
                                                                                                ((*q%16) + 'A' - 0x0A)));
                                                        charcount += 4;
                                                }
                                        } else {
                                                doprintc(&cp,tty, *q);
                                                charcount++;
                                        }
                                        q++;
                                }
                                if (spaces) {
                                        doprintc(&cp, tty, '"');
                                        charcount++;
                                }
                                if (!rjust && (width != -1) && (length < width)) {
                                        for (d = 0; d < (width-length); d++) {
                                                doprintc(&cp,tty,' '); /* trailing fill - spaces */
                                                charcount++;
                                        }
                                }
                                break;
                
                        case 'r':                                       /* 'raw' display string */
                                q = va_arg(argp, uchar *); /* point to string */
                                if (q == NULL)                  /* check for null pointer */
                                        break;
                                while (*q != 0) {               /* field widths don't make */
                                        if (cp)                         /* any sense for displays */
                                                *cp++ = *q;
                                        else {
                                                ;
#ifdef NOTHING
                                                if (tty != NULLTTY) {
                                                        if (system_running && tty != CONTTY) {
                                                                putc(tty,*q);
                                                        } else {
                                                                console_putc(*q);
                                                        }
                                                }
#endif

                                        }
                                        charcount++;
                                        q++;
                                }
                                break;
                
                        case '%':                                       /* percent sign */ 
                                doprintc(&cp,tty, '%');
                                charcount++;
                                break;

#ifdef NOTHING

                        case 'C':                                       /* cisco specials */
                                fmt++;
                                /* fall thru */
#endif

                        default:

                                vprintf(fmt, argp );
                                /*
                                   perror("Default in doprnt");
                                   charcount += reg_invoke_printf(*(fmt-1), &fmt, &cp, tty, &argp,
                                   rjust, width, size, hash,
                                   precision);
                                   */
                                break;
                        }
                }
    }
    return(charcount);
}


#ifdef NOTHING

/*
 * automore_enable() - Turn on automatic "more" processing
 *
 * This routine is used to enable automatic "more" processing. After
 * calling it, printf() will pause every page asking the user if they
 * want more. If they say no, all further output will be supressed until
 * automore processing is disabled.
 */

void automore_enable (const char *header)
{
    if (!stdio)
                return;
    stdio->automore_count = 0;                  /* Set the line number */
    stdio->automore_flush = FALSE;              /* Not flushing output */
    stdio->automore_enabled = TRUE;             /* Enable automore */
    automore_header(header);                    /* Set the header pointer */
}

/*
 * automore_header() - Change automore's header in midstream
 *
 * Once automore is running, it is sometimes useful to be able to change
 * the header.  This can happen if you are printing a table and want to
 * print some explanatory text before the table header.
 */

void automore_header (const char *header)
{
    if (!stdio)
                return;
    if (header) {
                stdio->automore_header = NULL;  /* To avoid printing header twice */
                printf("\n%s", header);
    }
    stdio->automore_header = header;    /* Save the header pointer */
}

/*
 * automore_disable() - Disable automatic "more" processing
 *
 * This routine is used to disable any further more processing.
 */


void automore_disable (void)
{
    if (!stdio)
                return;
    stdio->automore_enabled = FALSE;    /* Disable automore processing */
    stdio->automore_flush = FALSE;              /* No more output flushing */
}

/*
 * automore_quit
 * If the user quits out of automore, allow the user process to find out.
 * This is normally used by having the user process poll this function.  If
 * it returns TRUE, then the output is being flushed.  This is very useful
 * if the output is frequently very long.
 */

boolean automore_quit (void)
{
    if (!stdio)
                return(TRUE);
    return(stdio->automore_flush);
}

/*
 * automore_conditional - Conditionally check for stopping output
 *
 * This routine is used to conditionally ask for permission to do
 * more output. Rather than actually doing the I/O here, we simply
 * adjust the line count so that the regular automore processing
 * will handle it.
 */

void automore_conditional (int lines)
{
    /*
     * If output has not been flushed, and this terminal has a length,
     * and a line count of zero or a line count greater than what would
     * fit on the page was specified, set the lines printed to the
     * page length so that the next output will cause the more prompt
     * to appear.
     */

    if (!stdio)
                return;
    if (!stdio->automore_flush && stdio->tty_length &&
                (!lines || (lines > (stdio->tty_length - stdio->automore_count))))
                stdio->automore_count = stdio->tty_length;
}

#endif

