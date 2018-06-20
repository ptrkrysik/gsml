/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: dsptypes.h
*
* Description:   
*   Data Types for the TMS320C6X
*
*****************************************************************************/
#ifndef _DSPTYPES_H_
#define _DSPTYPES_H_

/* For non-DSP code standard defintions from here */
#include "JCStdDef.h"

#if (CPU==TI_C6201)

typedef     char           Char;    /* 8 bit signed integer */
typedef     char           Int8;    /* 8 bit signed integer */
typedef     unsigned char  UChar;   /* 8 bit unsigned integer */
typedef     unsigned char  Uchar;   /* 8 bit unsigned integer */ 
typedef     unsigned char  Uint8;   /* 8 bit unsigned integer */ 
typedef     short          Short;   /* 16 bit signed integer */
typedef     short          Int16;   /* 16 bit signed integer */
typedef     unsigned short UShort;  /* 16 bit unsigned integer */ 
typedef     unsigned short Uint16;  /* 16 bit unsigned integer */  
typedef     int            Int;     /* 32 bit signed integer */
typedef     int            Int32;   /* 32 bit signed integer */
typedef     unsigned int   UInt;    /* 32 bit unsigned integer */
typedef     unsigned int   Uint32;  /* 32 bit unsigned integer */
typedef     long           Long;    /* 40 bit signed integer */
typedef     long           Int40;   /* 40 bit signed integer */
typedef     unsigned long  ULong;   /* 40 bit unsigned integer */
typedef     unsigned long  Uint40;  /* 40 bit unsigned integer */
typedef     float          Float;   /* 32 bit floating point value */
typedef     double         Double;  /* 64 bit floating point value */
typedef     unsigned char  Bool;    /* Bool value */
typedef     void           Void;

#endif

#define TRUE                1     
#define FALSE               0     

#define DISABLE 0
#define ENABLE  1

typedef enum
{
   SUCCESS,
   FAILURE,
   UN_PROCESSED
} ProcResult;

/*---------------------------------------------------------------------------
* For each variable, define INGLOB or EXGLOB as blank to allocate internal
* or external DSP memory, or else as "extern" to just reference the variable
*/
#ifdef ALLOCATE_MEMORY_INTERNAL
#define INGLOB
#else
#define INGLOB extern 
#endif

#ifdef ALLOCATE_MEMORY_EXTERNAL
#define EXGLOB
#else
#define EXGLOB extern 
#endif
/*-------------------------------------------------------------------------*/
                         
#endif