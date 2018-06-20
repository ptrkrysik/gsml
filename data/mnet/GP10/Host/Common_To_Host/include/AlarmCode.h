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
/* Create Date : 9/18/2000                                           */
/* Description : GP10 Alarm Code files                               */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |07/14/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/

#if !defined(_ALARMCODE_H_)
#define _ALARMCODE_H_


/* This is GP10 */
#define MNET_PRODUCT_SPECIFIC_GP10

#include "MnetProductId.h"
#include "MnetModuleId.h"
#include "JcErr.h"
#include  "MibTags.h"


#define MAX_MODULE_SPECIFIC_ALARM    0xFFFF


/* Allocation default severity of AlarmCode space per module */
#define ALARM_CRITICAL_START_POINT  0x0001
#define ALARM_MAJOR_START_POINT     0x0100    
#define ALARM_MINOR_START_POINT     0x0800
#define ALARM_INFO_START_POINT      0x1000


/* Common across products */
#define MNET_MODULE_COMMON          0xFF

/* Mnet Alarm error codes are allocated as follows:                 */
/*                                                                  */
/*   31             23             15             7            0    */
/*  +----------------------------------------------------------+    */
/*  |    spare      | Module  ID   |       Alaram ID           |    */
/*  +----------------------------------------------------------+    */
/*                                                                  */

typedef enum {
        EC_NOERROR = 0,
        EC_COMMON_NOERROR = EC_NOERROR,  
        
        EC_GP10_START = (MNET_PRODUCT_GP10 << 24),         /* GS Alarm Start point */

            
      /* GP10 DSP Interface Driver Module */
        EC_L1_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_L1 << 16)),
        
        
        /* RIL3 Message Delivery */
        EC_MD_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_MD << 16)),
        

        //critical alarm code definitions
		//
		EC_GRR_START = ((MNET_PRODUCT_GP10 << 24) | (MODULE_GRR << 16)),
	    EC_GRR_BOOTING_UP,
		EC_GRR_OS_CALL_WDCREATE_FAILED,
		EC_GRR_OS_CALL_WDSTART_FAILED,
		EC_GRR_OS_CALL_MSGQCREATE_FAILED,
		EC_GRR_OS_CALL_MSGQRECEIVE_FAILED,
		EC_GRR_OS_CALL_MSGQSEND_FAILED,
		EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED,
		EC_GRR_OAM_CALL_SETTBLENTRYFIELD_FAILED,
		EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED,
		EC_GRR_OAM_CALL_GETMIBTBL_FAILED,
		EC_GRR_OAM_CALL_UNSETTRAPBYMODULE_FAILED,
		EC_GRR_OAM_CALL_SETTRAP_FAILED,
		EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED,
		
		EC_GRR_DSP_1_NOT_RESPONDING,
		EC_GRR_DSP_2_NOT_RESPONDING,

		EC_GRR_DSP_1_NOT_TICKING,
		EC_GRR_DSP_2_NOT_TICKING,

		EC_GRR_DSP_1_INIT_WATCH_TIMER_EXPIRED,
		EC_GRR_DSP_2_INIT_WATCH_TIMER_EXPIRED,

		EC_GRR_TRX_1_SLOT_1_ACT_NACK,
		EC_GRR_TRX_1_SLOT_2_ACT_NACK,
		EC_GRR_TRX_1_SLOT_3_ACT_NACK,
		EC_GRR_TRX_1_SLOT_4_ACT_NACK,
		EC_GRR_TRX_1_SLOT_5_ACT_NACK,
		EC_GRR_TRX_1_SLOT_6_ACT_NACK,
		EC_GRR_TRX_1_SLOT_7_ACT_NACK,
		EC_GRR_TRX_1_SLOT_8_ACT_NACK,
		EC_GRR_TRX_2_SLOT_1_ACT_NACK,
		EC_GRR_TRX_2_SLOT_2_ACT_NACK,
		EC_GRR_TRX_2_SLOT_3_ACT_NACK,
		EC_GRR_TRX_2_SLOT_4_ACT_NACK,
        EC_GRR_TRX_2_SLOT_5_ACT_NACK,
		EC_GRR_TRX_2_SLOT_6_ACT_NACK,
	    EC_GRR_TRX_2_SLOT_7_ACT_NACK,
		EC_GRR_TRX_2_SLOT_8_ACT_NACK,

		EC_GRR_TRX_1_AMSTATE_INVALID,
		EC_GRR_TRX_2_AMSTATE_INVALID,

		//Informational alarm code definitions
		// 
		EC_GRR_INFO_BASE = (EC_GRR_START | ALARM_INFO_START_POINT),
		EC_GRR_MIB_INVALID_CB_CONFIG,
		EC_GRR_TRX_1_SLOT_1_CHANCOMB_CHANGED,
		EC_GRR_TRX_1_SLOT_2_CHANCOMB_CHANGED,
	    EC_GRR_TRX_1_SLOT_3_CHANCOMB_CHANGED,
		EC_GRR_TRX_1_SLOT_4_CHANCOMB_CHANGED,
        EC_GRR_TRX_1_SLOT_5_CHANCOMB_CHANGED,
		EC_GRR_TRX_1_SLOT_6_CHANCOMB_CHANGED,
	    EC_GRR_TRX_1_SLOT_7_CHANCOMB_CHANGED,
		EC_GRR_TRX_1_SLOT_8_CHANCOMB_CHANGED,
		EC_GRR_TRX_2_SLOT_1_CHANCOMB_CHANGED,
		EC_GRR_TRX_2_SLOT_2_CHANCOMB_CHANGED,
	    EC_GRR_TRX_2_SLOT_3_CHANCOMB_CHANGED,
		EC_GRR_TRX_2_SLOT_4_CHANCOMB_CHANGED,
        EC_GRR_TRX_2_SLOT_5_CHANCOMB_CHANGED,
		EC_GRR_TRX_2_SLOT_6_CHANCOMB_CHANGED,
	    EC_GRR_TRX_2_SLOT_7_CHANCOMB_CHANGED,
		EC_GRR_TRX_2_SLOT_8_CHANCOMB_CHANGED,
	 	
		EC_GRR_BTS_LOCKED,
		EC_GRR_BTS_UNLOCKED,
		EC_GRR_BTS_SHUTTINGDOWN,

		EC_GRR_TRX_1_LOCKED,
		EC_GRR_TRX_2_LOCKED,
		
		EC_GRR_TRX_1_UNLOCKED,
		EC_GRR_TRX_2_UNLOCKED,
		
		EC_GRR_TRX_1_SHUTTINGDOWN,
		EC_GRR_TRX_2_SHUTTINGDOWN,

		EC_GRR_TRX_1_SLOT_1_LOCKED,
		EC_GRR_TRX_1_SLOT_2_LOCKED,
	    EC_GRR_TRX_1_SLOT_3_LOCKED,
		EC_GRR_TRX_1_SLOT_4_LOCKED,
        EC_GRR_TRX_1_SLOT_5_LOCKED,
		EC_GRR_TRX_1_SLOT_6_LOCKED,
	    EC_GRR_TRX_1_SLOT_7_LOCKED,
		EC_GRR_TRX_1_SLOT_8_LOCKED,
	    EC_GRR_TRX_2_SLOT_1_LOCKED,
		EC_GRR_TRX_2_SLOT_2_LOCKED,
	    EC_GRR_TRX_2_SLOT_3_LOCKED,
		EC_GRR_TRX_2_SLOT_4_LOCKED,
        EC_GRR_TRX_2_SLOT_5_LOCKED,
		EC_GRR_TRX_2_SLOT_6_LOCKED,
	    EC_GRR_TRX_2_SLOT_7_LOCKED,
		EC_GRR_TRX_2_SLOT_8_LOCKED,

		EC_GRR_TRX_1_SLOT_1_UNLOCKED,
		EC_GRR_TRX_1_SLOT_2_UNLOCKED,
	    EC_GRR_TRX_1_SLOT_3_UNLOCKED,
		EC_GRR_TRX_1_SLOT_4_UNLOCKED,
        EC_GRR_TRX_1_SLOT_5_UNLOCKED,
		EC_GRR_TRX_1_SLOT_6_UNLOCKED,
	    EC_GRR_TRX_1_SLOT_7_UNLOCKED,
		EC_GRR_TRX_1_SLOT_8_UNLOCKED,
	    EC_GRR_TRX_2_SLOT_1_UNLOCKED,
		EC_GRR_TRX_2_SLOT_2_UNLOCKED,
	    EC_GRR_TRX_2_SLOT_3_UNLOCKED,
		EC_GRR_TRX_2_SLOT_4_UNLOCKED,
        EC_GRR_TRX_2_SLOT_5_UNLOCKED,
		EC_GRR_TRX_2_SLOT_6_UNLOCKED,
	    EC_GRR_TRX_2_SLOT_7_UNLOCKED,
		EC_GRR_TRX_2_SLOT_8_UNLOCKED,

		EC_GRR_TRX_1_SLOT_1_SHUTTINGDOWN,
		EC_GRR_TRX_1_SLOT_2_SHUTTINGDOWN,
	    EC_GRR_TRX_1_SLOT_3_SHUTTINGDOWN,
		EC_GRR_TRX_1_SLOT_4_SHUTTINGDOWN,
        EC_GRR_TRX_1_SLOT_5_SHUTTINGDOWN,
		EC_GRR_TRX_1_SLOT_6_SHUTTINGDOWN,
	    EC_GRR_TRX_1_SLOT_7_SHUTTINGDOWN,
		EC_GRR_TRX_1_SLOT_8_SHUTTINGDOWN,
	    EC_GRR_TRX_2_SLOT_1_SHUTTINGDOWN, 
		EC_GRR_TRX_2_SLOT_2_SHUTTINGDOWN,
	    EC_GRR_TRX_2_SLOT_3_SHUTTINGDOWN, 
		EC_GRR_TRX_2_SLOT_4_SHUTTINGDOWN,
        EC_GRR_TRX_2_SLOT_5_SHUTTINGDOWN, 
		EC_GRR_TRX_2_SLOT_6_SHUTTINGDOWN,
	    EC_GRR_TRX_2_SLOT_7_SHUTTINGDOWN, 
		EC_GRR_TRX_2_SLOT_8_SHUTTINGDOWN,

        /* RIL3-RR functional entity */
        
        /* Critical alarm code definitions */
        EC_RM_START = ((MNET_PRODUCT_GP10 << 24) | (MODULE_RM << 16)),
        EC_RM_VC_INIT_FAILED,

        EC_RM_DSP_0_NOT_TICKING,
        EC_RM_DSP_1_NOT_TICKING,        
        EC_RM_DSP_0_NOT_RESPONDING,
        EC_RM_DSP_1_NOT_RESPONDING,

        EC_RM_DRF_SYNTH_0_FAILED_BCCH,
        EC_RM_DRF_SYNTH_1_FAILED_BCCH,

        EC_RM_CBCH_UNUSABLE,
        EC_RM_NOTCHFS_USABLE,
        EC_RM_NO_TRX_ENABLED,
      
        EC_RM_CHAN_COMB_5_MISPLACED,
        EC_RM_CHAN_COMB_UNSUPPORTED,
        EC_RM_TWO_CHAN_COMB_5_EXIST,
        EC_RM_TOO_MANY_CHAN_COMB_1_DETECTED,
        EC_RM_TOO_MANY_CHAN_COMB_5_DETECTED, 
        
        EC_RM_OS_CALL_WDCREATE_FAILED,
        EC_RM_OS_CALL_MSGQRECEIVE_FAILED,
        EC_RM_OAM_CALL_SETTBLENTRYFIELD_FAILED,
        EC_RM_OAM_CALL_SETMIBINTVAR_FAILED,
        EC_RM_RF_BOARD_NOT_DETECTED,
        
        //informational alarm code definitions
        EC_RM_INFO_BASE = (EC_RM_START | ALARM_INFO_START_POINT),

        EC_RM_DRF_SYNTH_0_FAILED_NON_BCCH,
        EC_RM_DRF_SYNTH_1_FAILED_NON_BCCH,

        EC_RM_BTS_LOCKED,
        EC_RM_BTS_UNLOCKED,
        EC_RM_BTS_SHUTTINGDOWN,
        EC_RM_TRX_1_LOCKED,
        EC_RM_TRX_2_LOCKED,
        EC_RM_TRX_1_UNLOCKED,
        EC_RM_TRX_2_UNLOCKED,
        EC_RM_TRX_1_SHUTTINGDOWN,
        EC_RM_TRX_2_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_1_LOCKED,
        EC_RM_TRX_1_SLOT_2_LOCKED,
        EC_RM_TRX_1_SLOT_3_LOCKED,
        EC_RM_TRX_1_SLOT_4_LOCKED,
        EC_RM_TRX_1_SLOT_5_LOCKED,
        EC_RM_TRX_1_SLOT_6_LOCKED,
        EC_RM_TRX_1_SLOT_7_LOCKED,
        EC_RM_TRX_1_SLOT_8_LOCKED,
        EC_RM_TRX_2_SLOT_1_LOCKED,
        EC_RM_TRX_2_SLOT_2_LOCKED,
        EC_RM_TRX_2_SLOT_3_LOCKED,
        EC_RM_TRX_2_SLOT_4_LOCKED,
        EC_RM_TRX_2_SLOT_5_LOCKED,
        EC_RM_TRX_2_SLOT_6_LOCKED,
        EC_RM_TRX_2_SLOT_7_LOCKED,
        EC_RM_TRX_2_SLOT_8_LOCKED,
        EC_RM_TRX_1_SLOT_1_UNLOCKED,
        EC_RM_TRX_1_SLOT_2_UNLOCKED,
        EC_RM_TRX_1_SLOT_3_UNLOCKED,
        EC_RM_TRX_1_SLOT_4_UNLOCKED,
        EC_RM_TRX_1_SLOT_5_UNLOCKED,
        EC_RM_TRX_1_SLOT_6_UNLOCKED,
        EC_RM_TRX_1_SLOT_7_UNLOCKED,
        EC_RM_TRX_1_SLOT_8_UNLOCKED,
        EC_RM_TRX_2_SLOT_1_UNLOCKED,
        EC_RM_TRX_2_SLOT_2_UNLOCKED,
        EC_RM_TRX_2_SLOT_3_UNLOCKED,
        EC_RM_TRX_2_SLOT_4_UNLOCKED,
        EC_RM_TRX_2_SLOT_5_UNLOCKED,
        EC_RM_TRX_2_SLOT_6_UNLOCKED,
        EC_RM_TRX_2_SLOT_7_UNLOCKED,
        EC_RM_TRX_2_SLOT_8_UNLOCKED,
        EC_RM_TRX_1_SLOT_1_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_2_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_3_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_4_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_5_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_6_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_7_SHUTTINGDOWN,
        EC_RM_TRX_1_SLOT_8_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_1_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_2_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_3_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_4_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_5_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_6_SHUTTINGDOWN,
        EC_RM_TRX_2_SLOT_7_SHUTTINGDOWN, 
        EC_RM_TRX_2_SLOT_8_SHUTTINGDOWN,
        
        EC_RM_TRX_0_RADIO_OFF, //Radio for TRX 0 is off
        EC_RM_TRX_1_RADIO_OFF, //Radio for TRX 1 is off
        EC_RM_RECV_CORRUPTED_DSP_MSG, //RM received corrupted DSP message
        EC_RM_END,
        
        /* RIL3-MM functional entity */
        EC_MM_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_MM << 16)),
        
        /* RIL3-CC functional entity */
        EC_CC_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_CC << 16)),
        
        
        /* LAPDm functional entity */
        EC_L2_DATA_START = ((MNET_PRODUCT_GP10 << 24) | (MODULE_L2 << 16)),
        EC_L2_DATA_CORRUPTED,      /* critical error, data corrupted */

        EC_L2_FREE_FRAME_EXHAUSTED =  (EC_L2_DATA_START | ALARM_MAJOR_START_POINT), /* non-critical error, no more free LAPDm available */
        EC_L2_END,
        
        
        /* RIL3-SMS functional entity */
        EC_SMS_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_SMS << 16)),
        
        /* OAM functional entity  informational alarms */
        EC_OAM_START = ((MNET_PRODUCT_GP10 << 24) | (MODULE_OAM << 16)),
        
        /* Oam critical */
        EC_OAM_REBOOT,         /* ViperCell is being reboot via OAM interface */
        
        EC_OAM_RELOAD_MIB  =  (EC_OAM_START | ALARM_INFO_START_POINT), // New MIB has been loaded (re-loaded */
        EC_OAM_DISABLE_TIMEX,  /* Timer for saving mib has be disabled */
        EC_OAM_ENABLE_TIMEX,   /* MIB saving timer has been enabled */
        EC_OAM_ALLOW_DUPS,     /* Allow duplicate saving */
        EC_OAM_SUPPRESS_DUPS,  /* Do not allow duplicate saving */
        EC_OAM_END,
        
        /* H323 Stack and Application Layers */
        EC_VOIP_START = ((MNET_PRODUCT_GP10 << 24) | (MODULE_H323 << 16)),
        EC_VIPERBASE_LINKDOWN,
        EC_VOIP_END,
        
        /* LUDB Module specific error code*/
        EC_LUDB_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_LUDB << 16)),
        
        
        /* ViperLog Task */
        EC_LOG_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_LOG << 16)),
        
        /* Maintenance Command Handler */
        EC_MCH_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_MCH << 16)),
        
        /* Alarm Module ID*/
        EC_ALARM_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_ALARM << 16)),
        
        /* System Initialization Module */
        EC_SYSINIT_START  = ((MNET_PRODUCT_GP10 << 24) | (MODULE_SYSINIT << 16)),
        EC_ROOT_MPC_SRAM_FAIL,                  /* MPC SRAM test failure */ 
        EC_ROOT_DSP0_INT_DATA_RAM_FAIL,         /* DSP0 internal data RAM test failure */ 
        EC_ROOT_DSP1_INT_DATA_RAM_FAIL,         /* DSP1 internal data RAM test failure */ 
        EC_ROOT_DSP0_INT_PROG_RAM_FAIL,         /* DSP0 internal program RAM test failure */ 
        EC_ROOT_DSP1_INT_PROG_RAM_FAIL,         /* DSP1 internal program RAM test failure */ 
        EC_ROOT_DSP0_EXT_RAM_FAIL,              /* DSP0 external RAM test failure */ 
        EC_ROOT_DSP1_EXT_RAM_FAIL,              /* DSP1 external RAM test failure */ 
        EC_ROOT_FPGA0_FAIL,                     /* FPGA0 test failure */ 
        EC_ROOT_FPGA1_FAIL,                     /* FPGA1 test failure */ 
        EC_ROOT_I2C_FAIL,                       /* I2C test failure */ 
        EC_ROOT_TASK_START_DELEY,               /* ViperTask startup delay detected */ 
        EC_ROOT_SYSCMD_SYM_NOT_FOUND,           /* SysCommand_xxx symbol not found */ 
        EC_ROOT_CRITICAL_TASK_SUSPEND,          /* ViperTask suspension detected */ 
        EC_ROOT_CRITICAL_TASK_MISSING,          /* ViperTask exit abnormally detected */ 
        EC_ROOT_LOAD_MODULE_FAIL,
        EC_SYSINIT_MAJOR_BASE = (EC_SYSINIT_START | ALARM_MAJOR_START_POINT),
        EC_ROOT_NON_CRITICAL_TASK_SUSPEND,      /* ViperTask suspension detected */ 
        EC_ROOT_NON_CRITICAL_TASK_MISSING,      /* ViperTask exit abnormally detected */ 
        EC_ROOT_TX_POWER_TABLE_ERROR,           /* TX power table is corrupted */ 
        EC_ROOT_END,

        /* CDR Client Alarm */
        EC_CDR_CLIENT_START_POINT          = ((MNET_PRODUCT_GP10 << 24) | (MODULE_CDR << 16)),
        EC_CDR_CLIENT_CRITICAL_START_POINT = (EC_CDR_CLIENT_START_POINT | ALARM_CRITICAL_START_POINT),
        EC_CDR_CLIENT_INIT_FAILED          ,
        EC_CDR_CLIENT_MAJOR_START_POINT    = (EC_CDR_CLIENT_START_POINT | ALARM_MAJOR_START_POINT   ),
        EC_CDR_CLIENT_UNABLE_CONNECT_SERVER,
        EC_CDR_CLIENT_INFO_START_POINT     = (EC_CDR_CLIENT_START_POINT | ALARM_INFO_START_POINT    ),        
        EC_CDR_CLIENT_BAD_SSL_PASSWORD_FILE,
        EC_CDR_CLIENT_BAD_SSL_KEY_FILE     ,
        EC_CDR_CLIENT_BAD_LOCAL_CERTIFICATE,
        EC_CDR_CLIENT_BAD_PEER_CERTIFICATE ,
        EC_CDR_CLIENT_END                  ,

        /* Common and Generic Alarm from 1 to 255 */
        EC_COMMON_START = ((MNET_PRODUCT_GP10 << 24) | (MNET_MODULE_COMMON << 16)),
        
        /* Common and Generic Alarm from 1 to 255 */
        EC_BOOTING_UP,          /* ViperCell  boot up */
        EC_SHUTTING_DOWN,       /* The module shutting down */
        
        EC_NO_MEMORY,           /* temporarily out of memory */
        EC_INTERNAL_ERROR,      /* memory curruption etc, exit is necessary */
        EC_START_REBOOT,        /* reboot started and captured using reboot hook. */
        EC_COMMON_END,
        
        EC_GP10_END
        
}AlarmCode_t;

/* For compatibility of the old version */
#define Alarm_errorCode_t AlarmCode_t

extern "C" JC_STATUS alarm_raise
(
      MNET_MODULE_ID    mid,              /* GP10 Source module ID              */
      MibTag            moduleErrorTag,   /* GP10 Source module's MIB error tag */
      AlarmCode_t       alarm_code        /* GP10 specific alarm code           */
);

extern "C" JC_STATUS alarm_raise_args
(
       MNET_MODULE_ID   mid,              /* GMC Source module ID              */
       MibTag           moduleErrorTag,   /* GMC Source module's MIB error tag */
       AlarmCode_t      alarm_code,       /* GMC specific alarm code           */
       int              arg1,             /* first arguments                   */
       int              arg2              /* second argument                   */
 );

extern "C" JC_STATUS alarm_clear
(
      MNET_MODULE_ID    mid,              /* GP10 Source module ID              */
      MibTag            moduleErrorTag,   /* GP10 Source module's MIB error tag */
      AlarmCode_t       alarm_code        /* GP10 specific alarm code           */
);


#endif // _ALARMCODE_H_