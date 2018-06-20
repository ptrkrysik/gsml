#ifndef _MNETMODULES_H_
#define _MNETMODULES_H_  /* include once only */

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
/* File        : MnetModules.h                                       */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/18/2000                                           */
/* Description :                                                     */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                                                 */
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#define MNET_MAX_MODULE_ID_NAME     128

typedef enum
{
   MODULE_L1  = 0x00,	/* DSP Interface Driver Module     */
   MODULE_MD  = 0x01,   /* RIL3 Message Delivery           */
   MODULE_RM  = 0x02,   /* RIL3-RR functional entity       */
   MODULE_MM  = 0x03,   /* RIL3-MM functional entity       */
   MODULE_CC  = 0x04,   /* RIL3-CC functional entity       */
   MODULE_L2  = 0x05,   /* LAPDm functional entity         */
   MODULE_SMS = 0x06,   /* RIL3-SMS functional entity      */
   MODULE_OAM = 0x07,   /* BTS OAM functional entity */
   MODULE_H323 = 0x08,  /* H323 Stack and Application Layers */
   MODULE_LUDB = 0x09,  /* LUDB Task - 5/17/99 klim          */
   MODULE_EXT_VC = 0x0A,  /* Msg from External ViperCell        */
   MODULE_LOG = 0x0B,   /* ViperLog Task - 7/14/99 klim    */
   MODULE_MCH = 0x0C,	/* Maintenance Command Handler - 7/15/99 klim */
   MODULE_ALARM = 0x0D,	/* Alarm  Module ID*/
   MODULE_SYSINIT = 0x0E,
   MODULE_PM = 0x0F,	/* Performance Measurement Module  */
   MODULE_SMSCBC = 0x10,/* SMS-Cell broadcast center, out of VC */
   MODULE_VBLINK = 0x11,/* ViperBase - ViperCell link client */
   MODULE_CDR = 0x12,	/* CDR Task */
   MODULE_CISS = 0x13,   /* CISS SUB MODULE */
   MODULE_TCPSRV = 0x14, /* TCP Server */
   MODULE_EXTHOA = 0x15, /* ext-HO <xxu:05-16-01> HOA */
   MODULE_BSSGP = 0x16,  /* BSSGP module */
   MODULE_RLCMAC = 0x17, /* RLC_MAC module */
   MODULE_GRR = 0x18,    /* GRR module */
   MODULE_GSLINK = 0x19,    /* GRR module */
   /*
   ......
   Other module IDs added here if needed later 
   ......
   */
    MNET_MAX_MODULE_IDS

} MNET_MODULE_ID;


extern char MnetModuleNames [][MNET_MAX_MODULE_ID_NAME];

#ifdef  __cplusplus
inline char *GetMnetModuleName(MNET_MODULE_ID id)
#else
static char *GetMnetModuleName(MNET_MODULE_ID id)
#endif
{
    if (id < MNET_MAX_MODULE_IDS)
    {
        return (MnetModuleNames[id]);
    }
    else
    {
        return("UNKNOWN_MODULE");
    }
}

#endif