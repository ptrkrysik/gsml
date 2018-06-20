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
/* File        : GP10Err.h                                           */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/18/2000                                           */
/* Description :                                                     */
/*                                                                   */
/*********************************************************************/
#ifndef _GSERR_H_
#define _GSERR_H_  /* include once only */

#include "JCErr.h"
#include "MnetModuleId.h"



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

typedef enum
{
    /* Logger error codes */
    MODULE_L1_ERR_CODE_START       = ((JC_TARGET_ERR_CODE_START + MODULE_L1) << 16)
    
    /* Add next module error codes below */
    
} GP10_ERROR_CODES;

#endif
