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
/* File        : JCErr.h                                             */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/18/2000                                           */
/* Description : JCErr contains error code defintions for common     */
/*               platform code.  Error codes for specific modules    */
/*               modules for a given target should be in the         */
/*               <target name>Err.h file.                            */
/*                                                                   */
/*********************************************************************/
#ifndef _JCERR_H_
#define _JCERR_H_  /* include once only */

typedef int JC_STATUS;          /* standard return type for common projects */


#define JC_TARGET_ERR_CODE_START    512

#define JC_OK       0           /* no error */

/* VxWorks reserves module id's 1-500 */
/* Common code uses - module number 0x1ff (511 decimal) */

/* All platform specific error codes should be entered below here. */
/* Format for error codes is as follows:                           */
/*  +-------------------------------------------+                  */
/*  | Module Id + 512    | Error Code           |                  */
/*  +-------------------------------------------+                  */
/*                                                                 */
/*  0x01ff0000 - Common code error codes                           */
/*  0x02000000 - First target module error codes                   */
/*  0x02010000 - Second target module error codes                  */
/*      .                                                          */
/*      .                                                          */
/*  0xffff0000 - Last target module error codes                    */

/* generic error code definitions -- module number 0x1ff (511 decimal) */
typedef enum
{
    JC_COMMON_ERR_CODE_START =  ((JC_TARGET_ERR_CODE_START-1) << 16),
    JC_ERROR,                   /* general error */
    JC_PARAM_INVALID,           /* parameter is out of range */
    JC_PARAM_MISSING,           /* parameter missing */
    JC_MSG_LENGTH_ERROR,        /* invalid message length */
    JC_INVALID_MSG              /* message not expected or
                                   invalid message type received */
} JC_ERROR_CODES;


#endif