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


typedef enum {
    HELP_TEXT, HELP_SHORT, HELP_LONG
} helpType;

#define FIELDS 4

static int line_index;          /* Lines printed since the last more?
                                 * prompt */

/*
 *  Forward references
 */
static int count_lines(save_buf * pbuffer);
static void find_field_widths(save_buf *pbuffer, short field_width[]);
static void sort_short_help(char *buffer, int length);
static int count_final_newlines(save_buf * pbuffer);
static int logical_line_length(char *s, helpType * ptype);
static int print_lines(char **, short [], boolean, boolean, long);
static void sort_lines(char **line_starts, int num);
static char **record_line_starts(save_buf * pbuf, int line_cnt);
static void paged_output(char *, int, short[], boolean, boolean);

/* cisco's strncpy puts a '\0' at the end of every destination.
 * This is at odds with UNIX's strncpy, which doesn't write '\0'
 * if it reachs the max length.
 */

static  void unix_strncpy (char *dst, char *src, ulong max)
{
    while ((max-- > 0) && (*dst++ = *src++));
}

/*
 * This function (print_buf) takes a save_buf and prints it on stdout,
 * after doing various formating functions:
 * 
 * - Lines of text are seperated by \n characters, but there's no guarentee
 * that the last line will be TERMINATED by a \n.
 * 
 * - Fields within lines are seperated by \t characters.  Fields are sized
 * dynamically; a field assumes the size of its largest component (plus
 * two chars padding).  If a line contains no \t characters, it does not
 * have fields and does not affect any field widths.
 * 
 * - Automatic wrap-around onto a new line may occur at any space (' ')
 * character.  If the line is wrapped within a field, the new line is
 * indented to the start of the field.
 * 
 * - After a screen-full of lines, a "More?" prompt is presented and accepts
 * single character input to control further output. Both '\r' and 'q' are
 * supported as in UNIX more.
 * 
 * - The size of the screen (both length and width) comes from the stdio
 * tt_soc structure. Here is what is not done:
 * 
 * - The more function doesn't support backing up in the input, although this
 * is certainly doable, since everything is in a buffer.
 * 
 * - The field function doesn't support wrapping any fields but the last one
 * on a line.  Predictable but bizarre things will occur if interior
 * fields extend past the end of line.
 * 
 */

int print_buf (save_buf buffer, boolean do_buginf, boolean do_formatting,
                           int  *pIndex, long maxCount)
{
    /* Dynamically determined field widths */
    short field_width[FIELDS] =
                {0, 0, 0, 0};
    char **line_starts = NULL;
    char **orig_line_starts = NULL;
    int final_newlines;
    int line_cnt = 2;
        int index = 0;

        int rc = RC_OK;

    /* Start line_cnt at 2 so we always malloc something below ! */

    /* Don't crash if no help buffer yet. */
    if (buffer.buf == NULL) {
                return ( 0 );
    }

    /*
     * We really need to ignore \n\n's at the end of the buffer. So we'll
     * count them and treat them specially. For now, we just won't output
     * any of the final \n's !
     */
    final_newlines = count_final_newlines(&buffer);

    line_cnt=count_lines(&buffer);

    /*
     * The variable line_cnt is now the number of lines seen. There should
     * be one newline starting each line, plus maybe a newline at the end.
     * We now need to allocate an array of pointers and record the
     * beginning of each useful line in it (i.e. we don't need to
     * alphabetize repeated newlines).
     * 
     * If we want to be hyper about memory, we should maybe make sure that
     * there's no interruption between line_starts allocation and freeing.
     * 
     * NOTE: record_line_starts sorts TABBED lines, so you have to
     * find field widths *after* calling it !
     *
     */

    line_starts = record_line_starts(&buffer, line_cnt);

    find_field_widths(&buffer, field_width);

    /* Print the paused output to screen. */
    line_index = 0;

    if (do_formatting) {
                /* automore_enable(NULL);*/
    }
        if  ( !pIndex ) 
                index = 0;
        else 
                index  = *pIndex;

    if ((index == 0) &&  (buffer.buf[0] != '\n')) {
                /* Make sure there is a leading newline */
            printf("\n");                       
    }

    if (line_starts == NULL) {
        /* Couldn't malloc, just dump it all out as is ! */
        paged_output(buffer.buf, buffer.used, field_width,
                                         do_buginf, do_formatting);
    } else {
                orig_line_starts = line_starts;
                line_starts = line_starts + index;
        rc = print_lines(line_starts, field_width, do_buginf, do_formatting, 
                                                 maxCount);
        PARSERfree( orig_line_starts );
                line_starts = NULL;
    }

    if (do_formatting) {
                /* automore_disable();*/
    }

        return ( rc );

}

static int count_lines (save_buf *pbuffer)
{
    int line_cnt = 0;
    int buffer_index;

    for (buffer_index = 0; buffer_index < pbuffer->used; buffer_index++) {
                if (pbuffer->buf[buffer_index]=='\n') {
                        line_cnt++;
                }
    }

    /* Add one for final line: */
    return(line_cnt+1);
}

static void find_field_widths (save_buf *pbuffer, short field_width[])
{
    int char_index = 0;
    int field_index = 0;
    int buffer_index;

    /*
     * Run through the buffer once to figure out field widths.  Perhaps it
     * would be best to optionally disable this feature, for printing
     * large things like configuration files that won't use fields anyway.
     * 
     * To avoid lots of looping, we start by counting the widths of each
     * field into field_width.  After we've gone through the buffer, we
     * insert padding (2 chars per field) and sum up the relative field
     * widths into absolute tab positions.
     * 
     * To keep things consistent, we use one set of field widths for printing
     * the entire print buffer.
     * 
     */

    for (buffer_index = 0; buffer_index < pbuffer->used; buffer_index++) {
        switch (pbuffer->buf[buffer_index]) {
        case '\n':
            if (field_index && field_width[field_index] < char_index) {
                field_width[field_index] = char_index;
            }
            char_index = 0;
            field_index = 0;
            /* line_cnt++; */
            /*
             * This will overestimate if we later ignore \n\n sequences or
             * print in chunks, but that's ok, can be dealt with if it
             * becomes a problem.
             */
            break;

        case '\t':
            if (field_width[field_index] < char_index) {
                field_width[field_index] = char_index;
            }
            char_index = 0;
            if (field_index < (FIELDS-1)) {
                field_index++;
            } else {
                field_index = 0;
                                /*
                                 * Next keyword hangs off end of line and we don't care
                                 * what the field width is, so skip it.
                                 */
                                buffer_index++;
                                while (pbuffer->buf[buffer_index] &&
                                           (pbuffer->buf[buffer_index] != '\n') &&
                                           (pbuffer->buf[buffer_index] != '\t')) {
                                        buffer_index++;
                                }
                        }
            break;

        default:
            char_index++;
            break;
        }
    }

    if (field_index && field_width[field_index] < char_index) {
        field_width[field_index] = char_index;
    }

    /*
     * Adjust field widths to be starting positions vice widths.
     */
    field_width[0] += 2;
    for (field_index = 1; field_index < FIELDS; field_index++) {
        field_width[field_index] += field_width[field_index - 1] + 2;
    }
}

static int count_final_newlines (save_buf * pbuffer)
{
    int chars_to_use;
    int buffer_index;
    int final_newlines = 0;

    buffer_index = pbuffer->used - 1;

    while ((buffer_index >= 0) && (pbuffer->buf[buffer_index] == '\n')) {
        buffer_index--;
    }
    chars_to_use = buffer_index + 1;
    pbuffer->buf[chars_to_use] = '\0';
    final_newlines = pbuffer->used - chars_to_use;
    pbuffer->used = chars_to_use;
    return(final_newlines);                             /* number of final newlines */
}


static void paged_output (char *buf, int used, short field_width[],
                                                  boolean do_buginf, boolean do_formatting)
{
    int char_index = 0;
    int field_index = 0;
    int buffer_index;
    int word_len;
    int (* output)(const char *, ...);

    output = printf;

    /*
     * Now we run through the buffer "for real".
     */
    for (buffer_index = 0; buffer_index < used; buffer_index++) {
        switch (buf[buffer_index]) {
                case '\t':
            if (do_formatting && (field_index < FIELDS)) {
                for (; char_index < field_width[field_index]; char_index++) {
                    (*output)(" "); 
                }
                field_index++;
                break;
            }
            /* Otherwise, fall through to newline */
                case '\n':
                (*output)("\n");
            char_index = 0;
            field_index = 0;
            line_index++;
            break;
                case ' ':
                        for (word_len = 0; buffer_index + word_len + 1 < used &&
                                 !isspace(buf[buffer_index + word_len + 1]);
                                 word_len++) {
                                ;
                        }
                        if ((! SCREEN_WIDTH) ||
                                ((char_index + word_len + 1) < SCREEN_WIDTH) ||
                                !do_formatting) {
                                (*output)(" "); 
                                char_index++;
                        } else {
                                (*output)("\n"); 
                                char_index = 0;
                                line_index++;
                                if (field_index) {
                                        for (; char_index < field_width[field_index - 1];
                                                 char_index++) {
                                                (*output)(" "); 
                                        }
                                }
                        }
                        break;
                default:
            (*output)("%c", buf[buffer_index]); 
            char_index++;
            break;
        }
    }
    if (char_index) {
                (*output)("\n"); 
    }

}

/*
 * Sort groups of lines.  A group is a set of lines of the same type,
 * i.e. ones from short help, long help, or just text which needs
 * to be displayed.  Only help strings should be sorted before ouput.
 */
static char **record_line_starts (save_buf * pbuf, int line_cnt)
{
    char **line_starts = NULL;
    int current_line = 0;
    int buffer_index = 0;
    helpType type = HELP_TEXT;
    helpType prevtype = HELP_TEXT;
    int len;
    int scan_line = 0;

    line_cnt++;
    /* reserve room for NULL marker at end, too */

    line_starts = (char **)PARSERmalloc(sizeof(char *) * line_cnt);

    if (line_starts == NULL) {
        /*
         * If this is NULL (malloc failed), we'll just have to do without
         * alphabeticizing the output !
         */
        return(NULL);
    }

    buffer_index = 0;
    while ((buffer_index < pbuf->used)
           && (pbuf->buf[buffer_index] != '\0')) {

        line_starts[scan_line] = pbuf->buf + buffer_index;
        prevtype = type;
        len = logical_line_length(line_starts[scan_line], &type);

        if ((prevtype == HELP_LONG) && (type != prevtype)) {
            /* Got long chunk that needs sorting. */
            sort_lines(line_starts + current_line, scan_line - current_line);
            /* Start recording the next chunk. */
            current_line = scan_line;
        }

        scan_line++;
        buffer_index += len;

        if (type == HELP_SHORT) {
            /* Each short help line needs sorting by itself. */
            sort_short_help(line_starts[current_line], len);
            /* Don't maintain a "chunk". */
            current_line = scan_line;
        }
        else if (type == HELP_TEXT) {
            /* Don't maintain a "chunk". */
            current_line = scan_line;
        }
    }

    /* Sort final chunk. */
    if (type == HELP_LONG) {
        sort_lines(line_starts + current_line, scan_line - current_line);
        current_line = scan_line;
    }

    line_starts[current_line++] = NULL; /* mark end of array */
    return(line_starts);
}

/*
 * Perform bubble sort on <num> logical lines.
 * 
 * ASSUMES: line_starts is not NULL.
 * 
 */
static void sort_lines (char **line_starts, int num)
{
    int i;
    int j;
    char *s;

    for (i = 0; i < num; i++) {
        for (j = i + 1; j < num; j++) {
            /*
             * Note: we don't care about stuff beyond logical line length,
             * because if we swap two lines based on what's way out there,
             * it won't show. So we don't need strncmp() here.
             */
            if (strcmp(line_starts[i], line_starts[j]) > 0) {
                s = line_starts[i];
                line_starts[i] = line_starts[j];
                line_starts[j] = s;
            }
        }
    }
}

static  char *find_http_level_end (char * str)
{
    char *tmp;

    tmp = strstr(str, "/level/");
    if (tmp) {
                tmp += 7;
                while (isdigit(*tmp)) {
                        tmp++;
                }
                return(tmp);
    } else {
                return(str);
    }
}

static void sort_short_help (char *buffer, int length)
{
    char *spare;
    int i;
    int ctab = 0;
    struct tabstruct {
        char *field;
        int length;
    };
    struct tabstruct *tabfields = NULL;
    char *str1;
    char *str2;

    /* allocate room for a temporary copy (and safety margin) */
    spare = (char *)PARSERmalloc((length + 2));
    if (spare == NULL) {
        /*
         * If we can't allocate this paltry amount of memory, then we'll
         * just live without sorting.
         */
        return;
    }

    {
        int tabs = 1;

        /*
         * Count tabs and allocate room for pointers to fields ending in
         * tabs.
         */
        for (i = 1; i < length; i++) {
            if (buffer[i - 1] == '\t') {
                tabs++;
            }
        }

        tabfields = (struct tabstruct *)
            PARSERmalloc((sizeof(struct tabstruct) * tabs));

        if (tabfields == NULL) {
            /* Oh, well. Gotta do without a sort. */
            PARSERfree(spare);
                        spare = NULL;
            return;
        }
    }

    /*
     * Avoid storing a pointer to the '\n' at the end of this line, if
     * there is one.
     */
    if ((length > 0) && (buffer[length - 1] == '\n')) {
        length--;
    }

    /* Store pointers to tabbed fields. */
    {
        int last = 0;

        tabfields[ctab++].field = buffer;

        for (i = 1; i < length; i++) {
            if (buffer[i - 1] == '\t') {
                tabfields[ctab].field = buffer + i;
                tabfields[ctab - 1].length = i - last;
                last = i;
                ctab++;
            }
        }
        tabfields[ctab - 1].length = i - last;
    }

    {
        int j;
        struct tabstruct tmp;

        /* Sort them. */
        for (i = 0; i < ctab; i++) {
            for (j = i + 1; j < ctab; j++) {
                                if (strstr(tabfields[i].field, "/level/")) {
                                        str1 = find_http_level_end(tabfields[i].field);
                                        str2 = find_http_level_end(tabfields[j].field);
                                } else {
                                        str1 = tabfields[i].field;
                                        str2 = tabfields[j].field;
                                }
                if (strcmp(str1, str2) > 0) {
                    tmp = tabfields[i];
                    tabfields[i] = tabfields[j];
                    tabfields[j] = tmp;
                }
            }
        }
    }

    {
        int len;
        char *s = spare;

        /* Output to spare buffer. */
        for (i = 0; i < ctab; i++) {
            len = tabfields[i].length;
            unix_strncpy(s, tabfields[i].field, len);
            s += len;
        }

        /*
         * Now copy back to buffer (but don't put \0 at end -- length
         * should be unaltered.
         */
        spare[length] = '\0';
        s = spare;
        while (*s != '\0') {
            *buffer++ = *s++;
        }
    }

    PARSERfree(tabfields);
    PARSERfree(spare);
    tabfields = NULL;
    spare = NULL;
}

/*
 * ASSUMES: line_starts is not NULL, with end entry marked by NULL.
 * 
 */
static int print_lines (char **line_starts, short field_width[],
                                                boolean do_buginf, boolean do_formatting, long maxCount)
{
    int i = 0;
    char *s;
    helpType junk;

    while (line_starts[i] != NULL) {
        s = line_starts[i];
                if ( (maxCount != 0) &&  (i >= maxCount) )
                        return( RC_MORE );
        i++;
                paged_output(s, logical_line_length(s, &junk),
                                         field_width, do_buginf, do_formatting);
    }

        return ( RC_OK );
}

/*
 * Figure out the length of the logical line in s. Also figure out what
 * type the line is.
 * 
 * ASSUMES: buffer end marked with '\0'.
 * 
 */
static int logical_line_length (char *s, helpType * ptype)
{
    int i;
    boolean looking = FALSE;

    *ptype = HELP_TEXT;

    for (i = 0; s[i] != '\0'; i++) {
        switch (s[i]) {
        case '\n':
            looking = TRUE;
            if ((i > 0) && (s[i - 1] == '\t')) {
                /* \t\n is the sign of a SHORT help line. */
                *ptype = HELP_SHORT;
            }
            break;
        case '\t':
            /* \n\t is a continuation line, can only be type LONG */
            *ptype = HELP_LONG;
            looking = FALSE;
            break;
        default:
            if (looking) {
                return(i);
            }
            break;
        }
    }
    if ((i > 0) && (s[i - 1] == '\t')) {
        /* \t\0 at the end is also a short help line. */
        *ptype = HELP_SHORT;
    }
    /* At end of buffer: */
    return(i);
}

/* print_caret  - prints a caret at the specified location */

char caret[]="^";
void print_caret (register int offset)
{
    printf("%*.1s", (1 + offset), caret); 
}
#if 0
/*
 * RAMS : commented this out as there is a duplicate definition in parser_printf.cpp
 */
int doprnt (char *cp, tt_soc *tty, const char *fmt, va_list argp,
                        boolean do_precision)
{


        int nbytes = 0;

        if ( cp != NULL ) { 
                if ( (nbytes = vsprintf( cp, fmt, argp ) ) < 0 ) { 
                        nbytes = 0;
                }
        } else if (parserNullFp != NULL ) { 
                if ( (nbytes = vfprintf( parserNullFp, fmt, argp ) ) < 0 ) { 
                        nbytes = 0;
                } else  { 
                        fflush(parserNullFp);
                        ioctl(fileno(parserNullFp), FIOFLUSH, 0 );
                }
        }

        return(nbytes);
}
#endif
