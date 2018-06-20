#ifndef __CLI_PRIVATE_H__
#define __CLI_PRIVATE_H__

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


/* 
 * copied from dosfs/uvfatTypes.h
 */ 

typedef uid_t   tUID;
typedef gid_t   tGID;
typedef tOCTET2          tMODE;
typedef unsigned long    tDIRNUM, *tpDIRNUM;
typedef tOCTET4          tCOMPATDWORD;


#define INVALID_UID             ((unsigned short)-1)
#define INVALID_GID             ((unsigned short)-1)
#define INVALID_MODE            -1
#define INVALID_DIRNUM          -1L
#define INVALID_FLAGS           -1L

#define INIT_DIRNUM                     0
#define ROOTDIR_DIRNUM          1

typedef tOCTET2  tACCESS;

#define setuid          setUid
#define getuid          getUid




/*
** vxWorks mutex semphore default flag
*/
#define PS_DEFAULT_MUTEX_SEM_FLAG \
        (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE)
#define PS_DEFAULT_BINARY_SEM_FLAG (SEM_Q_PRIORITY)
#define PS_DEFAULT_COUNT_SEM_FLAG  (SEM_Q_PRIORITY)
#define PS_DEFAULT_BINARY_SEM_STAT (SEM_FULL)
#define PS_DEFAULT_COUNT_SEM_COUNT 1

#endif  /* __CLI_PRIVATE_H_ */
