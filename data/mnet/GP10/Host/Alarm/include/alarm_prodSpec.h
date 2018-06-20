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
/* File        : AlarmTask.h                                         */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 11/20/2000                                           */
/* Description : This file contains Gp10 specific Alarm defination    */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |11/20/00| First working copy as common alarm module     */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/

#if !defined(_ALARM_PRODSPEC_H_)
#define _ALARM_PRODSPEC_H_

/* Some condition compilation */
#define  _SNMP_AGENT_EXIST
#define  _TCP_SERVER_EXIST

#include  "GP10OsTune.h"
#include  "oam_api.h"
#include <string.h>


/* Gp10 specific module and product IDs */
#define MY_PRODUCT_ID                MNET_PRODUCT_GP10
#define MY_MODULE_ID                 MODULE_ALARM
#define MY_LOGID                     ALARM_LAYER
#define MNET_MODULE_SNMP             MODULE_OAM

/* GP10 specific MIB tags */
#define MY_MIB_SOURCE_NAME           MIB_viperCellName
#define MY_MIB_ERROR_INFO            MIB_viperCellErrorInfoMib
#define MY_MIB_OAM_OPERATION         MIB_oam_operation
#define MY_MIB_APM1_IP_ADDRESS       MIB_h323_GKIPAddress
#define MY_MIB_OPERATION_STATE       MIB_bts_operationalState
#define MY_MIB_ACCESS_CONTROL_TAG    MIB_mibAccessControlFlag


#endif // _ALARM_PRODSPEC_H_
