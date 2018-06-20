#ifndef _PSTYPES_H_
#define _PSTYPES_H_


typedef char             tOCTET;        /* signed octet          */
typedef unsigned char    tUOCTET;       /* unsigned  octet       */
typedef int              tINT;          /* Integer               */
typedef short            tOCTET2;       /* signed   2 octets     */  
typedef short            s_size_t;      /* signed size_t         */
typedef unsigned         us_size_t;     /* unsigned size_t       */
typedef long             tOCTET4;       /* signed   4 octets     */
typedef unsigned short   tUOCTET2;      /* unsigned 2 octets     */
typedef unsigned long    tUOCTET4;      /* unsigned 4 octets     */
typedef FILE             tFILE;
typedef int              tTASKID;
typedef int              boolean;
typedef uint32_t         ipaddrtype;

    /*
    **
    **    Basic data type pointers
    **
    */

typedef char *            tpOCTET;     
typedef unsigned char *   tpUOCTET; 
typedef short *           tpOCTET2; 
typedef long *            tpOCTET4; 
typedef unsigned short *  tpUOCTET2;
typedef unsigned long *   tpUOCTET4;

/*
 * libc data types
 */
typedef FILE *            tpFILE;

/*
 * kernel data types
 */
typedef SEM_ID  tSEMAPHORE;                             /* semaphore                            */
typedef SEM_ID  tCSEMAPHORE;                    /* counting semaphore       */
typedef SEM_ID  tBSEMAPHORE;                    /* binary semaphore         */
typedef SEM_ID  tMSEMAPHORE;                    /* mutex semaphore          */

/*
 * missing from vxWorks' Posix types
 */
typedef long long int64_t;
typedef unsigned long long uint64_t;

/*
 * Cisco IOS compatibility
 */
typedef long long                       longlong;
typedef unsigned int                    uint;
typedef unsigned char                   uchar;
typedef unsigned long                   ulong;
typedef unsigned long long              ulonglong;
typedef unsigned long long              sys_timestamp;

/*
 * Special values for data types
 */
#ifndef NULLP
#define NULLP   ((void *) 0)
#endif 

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE   
#define FALSE   0
#endif

#define PSmalloc(Size)                          malloc(Size)
#define PScalloc(Num, Size)                     calloc(Num,Size)
#define PSrealloc(Block,Size)                   realloc(Block,Size)
#define PSfree(Block)                           free(Block)

#endif

