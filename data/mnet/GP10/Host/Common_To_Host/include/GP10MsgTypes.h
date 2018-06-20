#ifndef _GP10MSGTYPES_H_
#define _GP10MSGTYPES_H_  /* include once only */

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
/* File        : GP10MsgTypes.h                                      */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/18/2000                                           */
/* Description : GSMsgTypes contains the message types for all       */
/*               modules in the GS project                           */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |09/28/00| Added SNDCP message types                     */
/* Igal     |10/19/00| Added LLC internal and BSSGP message types    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#include "MnetModuleId.h"


/* All platform specific msg types   should be entered below here. */
/* Format for message types is as follows:                         */
/*  +-------------------------------------------+                  */
/*  | Module Id          | MsgType              |                  */
/*  +-------------------------------------------+                  */
/*                                                                 */
/*  0x00000000 - First target module message types                 */
/*  0x00010000 - Second target module message types                */
/*      .                                                          */
/*      .                                                          */
/*  0xffff0000 - Last target module message types                  */

typedef enum
{
/* RLC_MAC receiving message types */
    RLCMAC_MSG_TYPE_START      = (MODULE_RLCMAC << 16),
    RLCMAC_RM_MD_MSG_IND,
    RLCMAC_L1_MSG_IND,
    RLCMAC_L1_RTS,
    RLCMAC_READY_TO_GO,
    RLCMAC_ADMIN_STATE_CHANGE,
    RLCMAC_OP_STATE_CHANGE,
    RLCMAC_TIMESLOT_CONFIG,
    RLCMAC_DL_PDU_REQ,
    RLCMAC_LAST_MSG_TYPE,
    RLCMAC_T3169_EXPIRY,
    RLCMAC_T3195_EXPIRY,
    RLCMAC_T3191_EXPIRY,
    RLCMAC_T3193_EXPIRY,
	RLCMAC_UL_ACTIVITY_TIMER_EXPIRY,
	RLCMAC_DL_ACTIVITY_TIMER_EXPIRY,


/* common message type could be received by multiple modules */
    GP10_COMMON_MSG_TYPE_START = 0x80000000,
    GP10_COMMON_LAST_MSG_TYPE

} GP10_MESSAGE_TYPES;


#endif /* _GP10MSGTYPES_H_ */