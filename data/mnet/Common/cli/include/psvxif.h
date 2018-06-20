#ifndef _PSVXIF_H_
#define _PSVXIF_H_


    /*
    **
    **    Primitive data types and declarations
    **
    */


#ifndef CHAR_SET
#define CHAR_SET    _ASCII_CHAR_
#endif

/*
 * Borrowed from cdecl.h
 */
#if (defined(__STDC__) || defined(__cplusplus)) && !defined(__P)
#define __P(x)  x
#endif

/*
 * Set BYTE_ORDER based on cpu family.
 *
 * This may need some adjustment if
 * a cpu can do both byte orders.
 */
#ifndef BYTE_ORDER

#if CPU_FAMILY == I80X86
#define BYTE_ORDER LITTLE_ENDIAN

#elif CPU_FAMILY == SIMNT
#define BYTE_ORDER LITTLE_ENDIAN

#elif CPU_FAMILY == PPC
#define BYTE_ORDER BIG_ENDIAN

#else
#error Unrecognized Platform -- missing BYTE_ORDER
#endif

#endif  /* BYTE_ORDER */


/*
**  Make RC_OK and RC_ERROR to match vxworks OK and ERROR
*/

#ifdef RC_OK
#undef RC_OK
#endif
#define RC_OK OK

#ifdef RC_ERR
#undef RC_ERR
#endif
#define RC_ERR ERROR

#define SIGERR 0xFFFFFFFF

        /*
        **vxWorks data data types
        **
        */
typedef SEM_B_STATE             tSEMBSTAT;
    


/* 
 * Task Stack Parameters
 */

typedef enum { 
        STACK_SIZE = 1,
        STACK_MARGIN,
        STACK_CURRENT
} tSTACKPARAM;

#define KERNEL_PID    1
#define KERNEL_UID    1
#define KERNEL_GID    1
    /*
    **  List Constant
    */

#define FIFO_LIST                1
#define PRIORITY_LIST            2
#define PROTECTED_LIST           PROTECT
#define UNPROTECTED_LIST         NO_PROTECT
#define BINARY_SEMAPHORE         1
#define COUNT_SEMAPHORE          2
#define MUTEX_SEMAPHORE          3

    /*
    ** vxWorks mutex semphore default flag
    */
#define PS_DEFAULT_MUTEX_SEM_FLAG \
        (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE)
#define PS_DEFAULT_BINARY_SEM_FLAG (SEM_Q_PRIORITY)
#define PS_DEFAULT_COUNT_SEM_FLAG  (SEM_Q_PRIORITY)
#define PS_DEFAULT_BINARY_SEM_STAT (SEM_FULL)
#define PS_DEFAULT_COUNT_SEM_COUNT 1


/*
 * Basic constants.
 */
#define MAXCHAR   0x7F          /* largest positive 8-bit signed num */
#define MAXUCHAR  0xFF          /* largest positive 8-bit unsigned */
#define MAXSHORT  0x7FFF        /* largest positive 16-bit number */
#define MAXUSHORT 0xFFFF        /* largest positive 16-bit number */
#define MAXINT    0x7FFFFFFFL       /* largest positive 32-bit number */
#define MAXUINT   0xFFFFFFFFL       /* largest unsigned 32-bit number */
#define MAXLONG   MAXINT        /* largest positive 32-bit number */
#define MAXULONG  MAXUINT       /* largest unsigned 32-bit number */
#define MINCHAR   0x80          /* smallest possible signed 8-bit */
#define MINSHORT  0x8000        /* smallest possible signed 16-bit */
#define MININT    0x80000000L       /* smallest possible signed 32-bit */
#define MINLONG   MININT        /* smallest possible signed 32-bit */
 

   /*
   ** vxWorks generic constants
   */
#define SEM_DEFAULT_TIMEOUT  WAIT_FOREVER
#define SEM_TIMEOUT_NOWAIT   NO_WAIT
    /*
    **
    **    Other data types
    **
    */
    
#define tPROCID         tOCTET4         /* process identifier       */
#define tPROCID         tOCTET4         /* process identifier       */
#define tSEMTYPE        tOCTET2         /* semaphore type           */
#define tEVENT          tUOCTET2        /* event descriptor         */
#define tpEVENT         tUOCTET2 *      /* event pointer            */
#define tTIMER          timer_t         /* timer identifier         */
#define tTIME           time_t
#define tSECTIME        tUOCTET4        /* time in seconds          */
#define tMSECTIME       tUOCTET4        /* time in milliseconds     */
#define PS_TIMER_GRAN   ( (tUOCTET4)50000 ) /* 50 msec    */
#define PS_MSECT_GRAN   ( (PS_TIMER_GRAN)/( (tUOCTET4)1000 ) )/* gran in msec*/
#define mMSEC_TO_TICKS(ms)  ((ms)*sysClkRateGet()/1000) 
#define SOCKET          int

         /*
         **
         ** system resource limites
         **
         */

#define MIN_SYS_MEM     25600 /* 25k */

        /*
        **normalized marcos
        */

#define PS_TASK_ARGS   \
(int a1,int a2,int a3,int a4,int a5,int a6,int a7,int a8,int a9,int a10)

#define PStaskDelay    taskDelay
#define PStaskDelete   taskDelete
#define PStaskSuspend  taskSuspend
#define PStaskResume   taskResume
#define PStaskRestart  taskRestart
#define PStaskIdSelf   taskIdSelf

/*semphore stuff*/

#define PSsemBCreate   semBCreate
#define PSsemCCreate   semCCreate
#define PSsemMCreate   semMCreate
#define PSsemGive      semGive
#define PSsemTake      semTake
#define PSsemGiveForce semGiveForce
#define PSsemDelete    semDelete
#define PSsemInfo      semInfo
#define PSsemFlush     semFlush

/* Timer functions */

#define PStimer_cancel    timer_cancel
#define PStimer_connect   timer_connect
#define PStimer_create    timer_create
#define PStimer_delete    timer_delete
#define PStimer_gettime   timer_gettime
#define PStimer_settime   timer_settime
#define PSclock_gettime   clock_gettime

#define PSprintf                     printf
#define PS_Printf                    printf
#define PSfprintf                    fprintf
#define PSfopen                      fopen
#define PSfclose                     fclose
#define PSfputs                      fputs
#define PSfgets                      fgets
#define PSfscanf                     fscanf
#define PSmemset(dest, val, len)     memset(dest, val, len)
#define PSmemcpy(dest, src, len)     memcpy(dest, src, len)
#define PSmemcmp(dest, src, len)     memcmp(dest, src, len)
#define PSmemmove(dest, src, len)    memmove(dest, src, len)
#define PSstrlen(src)                strlen(src)
#define PSstrrchr(dest, pat)         strrchr(dest, pat)
#define PSstrchr                     strchr
#define PSstrdup                     strdup
#define PSstrcmp(dest, src)          strcmp(dest, src)
#define PSstrncmp(dest, src, len)    strncmp(dest, src, len)
#define PSstrncasecmp(dest, src, len) strncasecmp(dest, src, len)
#define PSstrstr                     strstr
#define PSassert                     assert
#define PStoupper                    toupper
#define mPSATEXIT                    atexit

#ifndef MEMCPY_DEBUG
#define PSsprintf                    sprintf
#define PSstrcat(dest, src)          strcat(dest, src)
#define PSstrncat(dest, src, len)    strncat(dest, src, len)
#define PSstrcpy(dest, src)          strcpy(dest, src)
#define PSstrncpy(dest, src, len)    strncpy(dest, src, len)
#endif  /* MEMCPY_DEBUG */

#define PSabs(x)               ( ((x) < 0) ? -(x) : (x) )
#define PSmin(x, y)            ( ((x) < (y)) ? (x) : (y) )
#define PSmax(x, y)            ( ((x) < (y)) ? (y) : (x) )


       /* For FileSystem / HTTPD 
       *
       */

#define mVOLNAME "/local"


/*
** more macros
*/

#define PS_MSEC_TO_TICK(sec) ( sec * sysClkRateGet() )
#define PSabort()            {abort_brp(); abort();}
#define PSexit(i)            {exit_brp();exit(i);}
#define PSpanic(s)           panic(s)
#define mMYHOSTNAME          


typedef void (tFREEFUNC)(void *);
typedef void (* tpFREEFUNC)(void *);

#endif /* _PSVXIF_H_ */

