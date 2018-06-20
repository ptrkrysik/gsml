
/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.01                                                */
/* Status      : Unde development                                    */
/* File        : jccsmspp.h                                          */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 12/25/2000                                          */
/* Description : his file contains data types and const defined      */
/*               for SMS-PP and are shared between SMC and SMR.      */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |03/09/00| Initial Draft                                 */
/* Bhawani  |11/20/00| restructured and ported form NT to vxWorks    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/

#ifndef		__JCCSMSPP_H_
#define		__JCCSMSPP_H_

#define SMS_MAX_RP_MSG_LEN 	248
#define SMS_MAGIC_NUMBER	0x12345678

/* Ranges for SMS reference numbers used on VBLink interface 
   for the ViperCell (VC) and ViperBase (VB)                 */
#define INVALID_VC_SMS_REF_NUM  0
#define MIN_VC_SMS_REF_NUM      1
#define MAX_VC_SMS_REF_NUM      32767
#define MIN_VB_SMS_REF_NUM      32768
#define MAX_VB_SMS_REF_NUM      65535

/* This value is used in the SMS VBLink communication 
   in order to specify normal disconnection          */
#define SMS_VBLINK_NORMAL_DISCONNECT    255

typedef enum {
        MNSMS_MSG_BASE = 0,

        MNSMS_ABORT_Req, // = 1 ViperBase->ViperCell

        MNSMS_EST_Req  , // = 2 RP-DATA ViperBase->ViperCell
        MNSMS_DATA_Req , // = 3 RP-ACK (RP-ERROR) ViperBase->ViperCell

        MNSMS_EST_Ind  , // = 4 RP-DATA ViperCell->ViperBase 
        MNSMS_DATA_Ind , // = 5 RP-ACK (RP-ERROR) ViperCell->ViperBase

        MNSMS_ERROR_Ind, // = 6 ViperCell->ViperBase
        MNSMS_REL_Req  , // = 7 ViperBase->ViperCell

        MNSMS_MSG_MAX  

} SMS_VBLINK_MSGTYPE;


typedef struct 
{
	UINT32      magic  ;    // = SMS_MAGIC_NUMBER
	UINT32      refNum ;    // associated subscriber profile index 
	UINT32      msgType;	// value of type SMS_VBLINK_MSGTYPE
	UINT32      status ;    // holds disconnect cause, or 0
	UINT32      msgLen ;		
} SmsHdrVblinkMsg_t;

#define SMS_VBLINK_HDRSIZE  (sizeof(SmsHdrVblinkMsg_t))

typedef struct 
{
   SmsHdrVblinkMsg_t hdr;
	UINT8             msg[SMS_MAX_RP_MSG_LEN];
} SmsVblinkMsg_t;

#endif /* __JCCSMSPP_H_ */
