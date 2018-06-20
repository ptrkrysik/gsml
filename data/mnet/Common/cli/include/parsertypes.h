#ifndef __PARSERTYPES_H__
#define __PARSERTYPES_H__

/* A BIG KLUDGE */

typedef void tt_soc;

#define NULLTTY (( void *) -1 )

/* Check parser_util.c */
/* A BIG KLUDGE */

#define SCREEN_WIDTH 80

#define sstrncpy strncpy

/* FROM parser_defbreaksets.h */

/* Break character set for the exec and config command levels.
 */
#define EXEC_BREAKSET           "\r\n?\t"
#define CONFIG_BREAKSET         "\r\n\032?\t"
#define NVCONFIG_BREAKSET       "\r\n"

/* FROM parser_defbreaksets.h */


/* FROM sys/h/globs.h */

/*
 * define data structure for named things.
 * NAMEDTHINGMACRO defines those fields common to all named things.
 * This allows common code to find named things in queues, and so on.
 */
 
#define NAMEDTHINGNAMELEN 20
#define NAMEDTHINGMACRO \
    char  name[NAMEDTHINGNAMELEN+1];
/* FROM sys/h/globs.h */


#define NULLFUNC ((void *) 0)

#define EXEC_MODE   1
#define CONFIG_MODE 2
/*
 * Define forward references to major system types
 */

#define STRUCTURE_REF(str) typedef struct str ## _ str

STRUCTURE_REF(nvtype);          /* h/config.h */
STRUCTURE_REF(nvtype_private);  /* h/config.h */
STRUCTURE_REF(parseinfo);       /* h/config.h */
STRUCTURE_REF(parser_mode);     /* h/config.h */
STRUCTURE_REF(parser_alias);    /* h/config.h */

/*
 *      Can't use STRUCTURE_REF for transition because
 * it is a constant.
 */
typedef const struct transition_ transition;    /* h/config.h */

/*
 * Type Definitions  -- note that these are really compiler specific 
 */

#if !defined(_SIZE_T) && !defined(_GCC_SIZE_T)
#define _SIZE_T

#ifdef __GNUC__
#define _GCC_SIZE_T
/* 
        typedef __typeof__ (sizeof (0)) size_t;
*/
#else
typedef unsigned int size_t;
#endif /* __GNUC__ */

#endif /* _SIZE_T */

#ifdef caddr_t
#undef caddr_t
typedef unsigned char *caddr_t;
#endif

/*
 * Size specific defines
 */
typedef signed char        int8;
typedef signed short       int16;
typedef signed long        int32;
typedef signed long long   int64;
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned long      uint32;
typedef unsigned long long uint64;
typedef unsigned char tinybool;
typedef void forktype;

/*
 * Queuetype data structure
 */
typedef struct queuetype_ {
    void *qhead;                        /* head of queue */
    void *qtail;                        /* tail of queue */
    int count;                          /* possible count */
    int maximum;                        /* maximum entries */
} queuetype;


#define QUEUESIZE(q)    queuesize_inline(q)
#define QUEUEFULL(q)    queuefull_inline(q)
#define QUEUEFULL_RESERVE(q, reserve) \
            queuefull_reserve_inline(q, reserve)
#define QUEUEEMPTY(q)   queueempty_inline(q)
#define QUEUEMAXIMUM(q)     queuemax_inline(q)


typedef struct nexthelper_ {
    struct nexthelper_ *next;
    unsigned char       data[0];
} nexthelper;


/*
 * Define the PACKED macro
 */
#if defined(__mips) || defined(__mc68000) || defined(__PPC__)
#define PACKED(item) item __attribute__ ((packed))
#else
#define PACKED(item) item
#endif

#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))

#ifdef NOTHING
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


#endif __PARSERTYPES_H__

