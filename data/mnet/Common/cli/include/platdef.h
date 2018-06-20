#ifndef _PLATDEF_H_
#define _PLATDEF_H_

#include <stddef.h>     /* _LITTLE_ENDIAN, _BIG_ENDIAN */

        /*
        **
        **      some basic defines
        **
        */
        
#define NO_PROTECT      0
#define PROTECT         1

#define RC_NONE         0
#define RC_OK           OK
#define RC_ERR          ERROR

#if 0
#define _LITTLE_ENDIAN_ 1234
#define _BIG_ENDIAN_    4321
#define LITTLE_ENDIAN   _LITTLE_ENDIAN_
#define BIG_ENDIAN      _BIG_ENDIAN_

#define BYTE_ORDER      _BYTE_ORDER
#endif
#define LITTLE_ENDIAN   _LITTLE_ENDIAN
#define BIG_ENDIAN      _BIG_ENDIAN

#define _ASCII_CHAR_    abcd

#define PRIVATE static /* local routines, can be changed to make some routines
                          visible for debugging */

/*
** INCLUDES
*/

/* common includes */



/* system specific includes.  They contain primitive data type definitions */


#ifdef  UNIX
#ifdef SUNSOL24
#define PStypeString ((tOCTET *) "Sun Solaris 2.4")
#endif
#include <psunif.h>
#endif


#ifdef VXWORKS
#ifdef VXWORKS53
#define PStypeString((tOCTET *) "vxWorks 5.3")
#endif
#endif

        /*
        **
        **      Basic defines
        **
        */


#ifndef BYTE_ORDER              /* byte order should be defined in platform file */
#error Unrecognized Platform
#endif

#ifndef CHAR_SET
#error Unrecognized Platform
#endif

#define PSNONSTDIO  1
#define PSSTDIO     0

/*
** Macros
*/


/* Data representation macros */                  

        /* 
        **
        **      Macros for data conversion are:
        **              mPS_2OCTET_TO_NET:  convert 2 octets to network format
        **              mPS_4OCTET_TO_NET:      convert 4 octets to network format
        **              mPS_2OCTET_TO_HOST:     convert 2 octets to host format
        **              mPS_4OCTET_TO_HOST:     convert 4 octets to host format
        **
        */
   
#if (BYTE_ORDER == _BIG_ENDIAN_)

#define mPS_2OCTET_TO_NET(pdest)                                                                \
                         (*(pdest))= ((*(pdest))<<8)                                            \
                                                | ( ((*(pdest))>>8) & 0x00ff)

#define mPS_4OCTET_TO_NET(pdest)                                                                \
                        (*(pdest))= ( ((*(pdest))>>24) & 0x000000ff)            \
                                                | ((*(pdest))<<24)                                              \
                                                | (0x0000ff00 & ((*(pdest)) >> 8))      \
                                                | ( 0x00ff0000 & ((*(pdest)) << 8))

#define mPS_2OCTET_TO_HOST      mPS_2OCTET_TO_NET

#define mPS_4OCTET_TO_HOST      mPS_4OCTET_TO_NET
                                                        

#else /* LITTLE_ENDIAN (this is the chosen network order) */

#define   mPS_2OCTET_TO_NET(pdest)                
#define   mPS_4OCTET_TO_NET(pdest)               
#define   mPS_2OCTET_TO_HOST(pdest)               
#define   mPS_4OCTET_TO_HOST(pdest)     
         
#endif /* BIG_ENDIAN */


#if     (CHAR_SET == _ASCII_CHAR_SET_)

#define mPS_OCTET_TO_HOST( pField, len)  

#define mPS_OCTET_TO_NET( pField, len)

#else

XXX not supported

#endif /* CHAR_SET */



/* 
** prototypes for remaining platform specific services
*/


/* Any platform specific initialization and auxilary functions */

extern tOCTET2  PS_Init (tOCTET2 ioType, tOCTET4 hInst, tOCTET4 hPInst, void *cmdParam, 
                         tOCTET2 nCmdShow);
                                                                
extern tOCTET2  PSWaitForEvent (tpEVENT pEvent, tOCTET4 *p1, tOCTET4 *p2, tOCTET4 *p3,
                                struct timeval *pTimeout);

/* Timer Tic (hearbeat) Service Interface  */


extern tOCTET2          PS_TimerInit (void (*tickhandler)(tOCTET2 tick_gran));
extern tOCTET2          PS_TimerTerm (void);
extern tMSECTIME        PS_MsecTime (void);      
extern tSECTIME         PS_SecTime (void);      
extern char        *PS_GetAscTime (void);

        /* 
        **
        **      Memory management and other macros defined in 
        **      platform specific include files
        **
        */


        /*
        **
        **      Events 
        **
        */
        
extern  tEVENT  or_timer_event;
extern  tEVENT  or_network_event;
extern  tEVENT  ij_cr_event;
extern  tEVENT  ij_cag_event;
extern  tEVENT  ij_db_event;


        /* 
        **      
        **      I/O, logging and tracing
        **
        */

#ifndef PS_getopt
extern  int  PS_getopt (int     argc, tOCTET **argv, char *opts);
#endif
#ifndef PS_Printf
extern  void     PS_Printf (char *lpszFormat);
#endif
extern  void     PS_ErrPrintf ( int errCode, const char *szFormat, ...);


        /* 
        **
        **      data buffer management 
        **
        */

tOCTET  *PS_alloc_buf(tOCTET4 bufsz);
tOCTET2 PS_Heap_Verify(char *pMem);
void PS_free_buf(void *pBuf);


/* Exit Function */

void PS_AtExit(void (*func) (void));
void PS_CleanUp(void);
void PS_kickSched(void);

/* Various status registration functions */

void PS_Reg_Status(void (*func) (void));
void PS_Reg_Trace(void (*func) (tOCTET2 type, tOCTET2 onOff));
void PS_Rreg_GetNextServer(char * (*func) (char *server, tOCTET2 *selected));
void PS_Rreg_SelectServer(tUOCTET2 (*func) (char *server));
void PS_Reg_MaskFunc(void (*func) (void));

#endif /* _PLATDEF_H */










