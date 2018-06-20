/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : JCStdDef.h                                          */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/18/2000                                           */
/* Description :                                                     */
/*                                                                   */
/*********************************************************************/
#ifndef _JCSTDDEF_H_
#define _JCSTDDEF_H_  /* include once only */

/* If not building for vxWorks then we need to define the processors */
/* used below.  If vxWorks is  used then these processors are        */
/* already defined.                                                  */
#if !defined(__VXWORKS__)
#define PENTIUM     1
#define PPC860      2
#define TI_C6201    3
#endif

/* If vxWorks is defined then the standard defs are specified in     */
/* the vxWorks tree.                                                 */

#if defined(__VXWORKS__)

#include <vxworks.h>

#endif

/* Standard definitions for PENTIUM */


#if (CPU==PENTIUM && !defined(__VXWORKS__))

typedef char           INT8; 
typedef short          INT16;
typedef long           INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
typedef unsigned char  bool;

#endif /* CPU=PENTIUM and __VXWORKS__*/



/* Standard definitions for PPC860 */

#if (CPU==PPC860 && !defined(__VXWORKS__))

typedef char           INT8; 
typedef short          INT16;
typedef long           INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
typedef unsigned char  bool;

#endif /* CPU=PPC860 and __VXWORKS__ */



/* Standard definitions for TI_C6201 */

#if (CPU==TI_C6201 && !defined(__VXWORKS__))

typedef char           INT8; 
typedef short          INT16;
typedef int            INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef unsigned char  bool;

#endif /* CPU=TI_C6201 */


/*
 * Added for Win32 test enviroment : DSN
 */
#ifdef WIN32

typedef short          INT16;
typedef int            INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
#include <windows.h>
#include <stddef.h>

#endif


#endif