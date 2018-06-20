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
/* File        : GSOsTune.h                                          */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/18/2000                                           */
/* Description : This file contains task specific tuning parameters  */
/*               such task priorities, stack sizes, etc.             */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/

#ifndef _GP10OSTUNE_H_
#define _GP10OSTUNE_H_  /* include once only */

/* RootTask Os parameters */
#define NET_TASK_PRIORITY               10
#define ROOT_TASK_PRIORITY1				9
#define ROOT_TASK_PRIORITY2				150
#define ROOT_TASK_OPTION				0
#define ROOT_TASK_STACK_SIZE			10000

/* Add next module Os parameters below */

#define RTPMAIN_TASK_PRIORITY			10
#define RTPMAIN_TASK_OPTION				0
#define RTPMAIN_TASK_STACK_SIZE			60000

#define RTPSUB_TASK_PRIORITY			10
#define RTPSUB_TASK_OPTION				0
#define RTPSUB_TASK_STACK_SIZE			10240

#define CC_SUB_TASK_PRIORITY			70
#define CC_SUB_TASK_OPTION				0
#define CC_SUB_TASK_STACK_SIZE			10000

#define VPCHAN_TASK_PRIORITY			50
#define VPCHAN_TASK_OPTION				0
#define VPCHAN_TASK_STACK_SIZE			100000

#define VBLINK_TASK_PRIORITY			150
#define VBLINK_TASK_OPTION				0
#define	VBLINK_TASK_STACK_SIZE			30000

#define LOG_TASK_PRIORITY           	150
#define LOG_TASK_OPTION             	0
#define LOG_TASK_STACK_SIZE         	100000

#define MCH_TASK_PRIORITY           	150
#define MCH_TASK_OPTION             	0
#define MCH_TASK_STACK_SIZE         	100000

#define CNI_LAPDM_PH_SERVER_PRIORITY   	10
#define CNI_LAPDM_PH_SERVER_OPTION     	0
#define CNI_LAPDM_PH_SERVER_STACK_SIZE 	20000

#define CNI_LAPDM_DL_SERVER_PRIORITY   	50
#define CNI_LAPDM_DL_SERVER_OPTION     	0
#define CNI_LAPDM_DL_SERVER_STACK_SIZE 	20000

#define H323_TASK_PRIORITY          	70
#define H323_TASK_OPTION            	0
#define H323_TASK_STACK_SIZE        	100000

#define CC_TASK_PRIORITY            	70
#define CC_TASK_OPTION              	0
#define CC_TASK_STACK_SIZE          	10000

#define RM_TASK_PRIORITY            	70
#define RM_TASK_OPTION              	0
#define RM_TASK_STACK_SIZE          	20000

#define GRR_MONITOR_TASK_PRIORITY       150
#define GRR_MONITOR_TASK_OPTION         0
#define GRR_MONITOR_TASK_STACK_SIZE     5000

#define L1UP_TASK_PRIORITY          	10
#define L1UP_TASK_OPTION            	0
#define L1UP_TASK_STACK_SIZE        	(32 * 1024)

#define L1DN_TASK_PRIORITY          	10
#define L1DN_TASK_OPTION            	0
#define L1DN_TASK_STACK_SIZE        	(32 * 1024)

#define DSP_TASK_PRIORITY           	50
#define DSP_TASK_OPTION             	0
#define DSP_TASK_STACK_SIZE            	(1024 * 1024)

#define LUDB_TASK_PRIORITY          	70
#define LUDB_TASK_OPTION            	0
#define LUDB_TASK_STACK_SIZE        	200000

#define MM_TASK_PRIORITY            	70
#define MM_TASK_OPTION              	0
#define MM_TASK_STACK_SIZE          	200000

#define PM_TASK_PRIORITY            	150
#define PM_TASK_OPTION              	0
#define PM_TASK_STACK_SIZE          	10000

#define ALARM_TASK_PRIORITY				100
#define ALARM_TASK_OPTION				0
#define ALARM_TASK_STACK_SIZE			10000

#define CDR_TASK_PRIORITY				150
#define CDR_TASK_OPTION					0
#define CDR_TASK_STACK_SIZE				20000

#define TCPSRV_TASK_PRIORITY			150
#define TCPSRV_TASK_OPTION				0
#define TCPSRV_TASK_STACK_SIZE			20000

#define RLCMAC_TASK_PRIORITY           	70
#define RLCMAC_TASK_OPTION             	0
#define RLCMAC_TASK_STACK_SIZE         	40000

#define BSSGP_TASK_PRIORITY				70 
#define BSSGP_TASK_OPTION				0
#define BSSGP_TASK_STACK_SIZE			10240

#define GSLINK_TASK_PRIORITY           	150
#define GSLINK_TASK_OPTION             	0
#define GSLINK_TASK_STACK_SIZE         	10000


#endif
