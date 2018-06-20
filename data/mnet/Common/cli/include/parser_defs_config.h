/* Terminal and line parameter defines */
enum TERM_LINE_PARAMS {
    TERM_LENGTH,
    TERM_WIDTH,
    TERM_MONITOR,
};

/* Configuration sources */
#define PARSER_CONF_NONE                 0      /* No source identified */
#define PARSER_CONF_TERM                 1      /* Config from terminal */
#define PARSER_CONF_MEM                  2      /* Config from NV memory */
#define PARSER_CONF_NET                  3      /* Config from a TFTP host */
#define PARSER_CONF_OVERWRITE_NET        4      /* Config from a TFTP host and o
verwrite config */
#define PARSER_CONF_BATCH                5      /* Not from terminal */
#define PARSER_CONF_HTTP                 6      /* Config from HTTP server */


/* Enable defines */
#define ENABLE_PASSWORD          1
#define ENABLE_SECRET            2

/* How many characters to save at the end of config buffers to guarantee
 * that "\nend" gets written.
 */
#define ENDSTRLEN   5

#define MAXTTYLENGTH 512
