
/********************************************************************
*                          
*  FILE NAME: alarmCodeTool.cpp
*                      
*  DESCRIPTION: This file contains alarm code tools. It prints
*               alarm code to English Alarm message to the standard
*               output. The output file can be redirected to a disk file
*               and saved as AlarmCode.English at directory
*               /ViperCall/ViperWatch/Text. The alarm applet reads this
*               file to translate alarm code to English alarm message.
*
*      COMPONENTS:                      
*                              
*                                  
*  NOTES:                              
*                                  
*  (C) Copyright 2000 Cisco Systems, Inc., CA, USA http://www.cisco.com/
*                                 
*  REVISION HISTORY                            
*__________________________________________________________________
*----------+--------+----------------------------------------------
* Name     |  Date  |  Reason                
*----------+--------+----------------------------------------------
* Bhawani  |08/26/99| Initial Draft
*----------+--------+----------------------------------------------
********************************************************************
*/


#include "AlarmCode.h"
#include "stdio.h"


typedef struct AlarmTableEntry {
    int errCode;
    char *synopsis;
    char *detailMsg;
} AlarmTableEntry;


/*   The alarm table contains alarm list. Alarm entries are populated in the following format
 *   {ALARM_ERROR_CODE | "ALARM_SYNOPSIS"  | "ALARM_DETAIL_MESSAGE"},
 *
 *   In the ALARM_DETAIL_MESSAGE, the following variable can be used to replaced relevant texts:
 *   %alarmId     : Alarm Id in the alarm message.
 *   %timeStamp   : The time when the alarm occurred.
 *   %severity    : The alarm severity code [Critical, Major, Minor]
 *   %module      : Module identifier name
 *   %alarmCode   : Alarm code
 *   %1           : Optional argument 1
 *   %2           : Optional argument 2
*/


static AlarmTableEntry  alarmTable[] = {
    
    /* General Common Alarms */
    {EC_NOERROR, "Module alarm has cleared", "The alarm code %1 was cleared by module %module. The operational state of GSM Port depends on alarm status from all modules."},
    {EC_BOOTING_UP, "Module booting up",  "Module %module is booting up now"},
    {EC_SHUTTING_DOWN, "Module shutting down", "Module %module is shutting down now"},
    {EC_NO_MEMORY, "Out of Internal Memory Error", "The GSM Port ran out of memory. Module %module did not complete its intended operation. GSM Port must be rebooted to recover from this situation."},
    {EC_INTERNAL_ERROR, "System Internal Error", "GSM Port software system encountered an unexpected error, which resulted in data corruption. GSM Port must be rebooted to recover from this situation."},
    
    /* Reboot started reported by roottask upon invocation of reboot-hook */
    {EC_START_REBOOT, "Reboot Started", "The GSM Port is rebooting now."},

    
    /* RM module related Alarms */
    {EC_RM_DSP_0_NOT_TICKING, "First DSP/FPGA Hardware Failure", "Bit clock at the first DSP/Fpga is not functioning. GSM Port must be rebooted to recover from this situation."},
    {EC_RM_DSP_1_NOT_TICKING, "Second DSP/FPGA Hardware Failure", "Bit clock at the second DSP/Fpga is not functioning. GSM Port must be rebooted to recover from this situation."},

    {EC_RM_DSP_0_NOT_RESPONDING, "First DSP not responding", "Link between RM and first DSP is broken. GSM Port must be rebooted to recover from this situation."},
    {EC_RM_DSP_1_NOT_RESPONDING, "Second DSP not responding", "Link between RM and second DSP is broken. GSM Port can function in a reduced-Capacity mode if the first DSP can function properly. GSM Port must be rebooted to recover from this situation."},
     
    {EC_RM_DRF_SYNTH_0_FAILED_BCCH, "DRF SYNTH 0 PLL Unlocked - BCCH", "Radio Board BCCH Side TRX 1 PLL is not locked, BTS should be Shut down."},
    {EC_RM_DRF_SYNTH_1_FAILED_BCCH, "DRF SYNTH 1 PLL Unlocked - BCCH", "Radio Board BCCH Side TRX 2 PLL is not locked, BTS should be Shut down."},

    {EC_RM_CBCH_UNUSABLE, "Cell Broadcast Channel unusable", "Cell broadcast channel initialization failed. Cell broadcast will not be supported until the next reboot. This channel, however, does not affect the operational state of the GSM Port."},
    {EC_RM_NOTCHFS_USABLE, "No usable traffic channel", "Initialization of all the traffic channels failed. Please unlock them if they are locked. If the their admin state is already unlocked, please reboot the system."},
    {EC_RM_NO_TRX_ENABLED, "No TRX enabled", "Initialization of both TRX failed. Please unlock them if they are locked. If the their admin state is already unlocked, please reboot the system."},

    {EC_RM_CHAN_COMB_5_MISPLACED, "Illegal Configuration - No BCCH Channel", "No BCCH channel combination is specified in the MIB. Please check the first TRX configuration through GP Manager."},
    {EC_RM_CHAN_COMB_UNSUPPORTED, "Illegal Configuration - Unsupported Channel", "Unsupported channel combination is detected in the MIB. Please check both first and second TRX configuration through GP Manager"},
    {EC_RM_TWO_CHAN_COMB_5_EXIST, "Illegal Configuration - Two BCCH Channel", "Only one BCCH channel is supported at this time but two BCCH channel configuration found. Please correct this problem through GP Manager."},
    {EC_RM_TOO_MANY_CHAN_COMB_1_DETECTED, "Illegal Configuration - Too many Channel Combination I", "Too many channel combination I configuration found. Please correct this problem through GP Manager."},
    {EC_RM_TOO_MANY_CHAN_COMB_5_DETECTED, "Illegal Configuration - Multiple BCCH Channels", "Only one BCCH channel is supported at this time but more than two BCCH channels have been configured. Please correct this problem through GP Manager."},

    {EC_RM_OS_CALL_WDCREATE_FAILED, "System Call Failure - Watchdog timer create", "RM module detects failure in creating WatchDog Timer which is essential for proper software operation. GSM Port must be rebooted to recover from this situation."},
    {EC_RM_OS_CALL_MSGQRECEIVE_FAILED, "System Call Failure - Message queue receive", "RM module encountered failure in receiving message. GSM Port must be rebooted to recover from this situation."},
    
    {EC_RM_OAM_CALL_SETTBLENTRYFIELD_FAILED, "OAM Api Failure", "Could not set a field of a MIB table entry."},
    {EC_RM_OAM_CALL_SETMIBINTVAR_FAILED, "Oam Api Failure", "Could not set a MIB integer variable."},
    {EC_RM_RF_BOARD_NOT_DETECTED, "RF board not detected", "Failed to detect RF board. Please check if it is there."},


    //informational alarm code definitions
     
    {EC_RM_DRF_SYNTH_0_FAILED_NON_BCCH, "DRF TRX 1 PLL Unlocked -NON BCCH", "Radio Board NON-BCCH TRX 1 PLL is not locked, TRX 1 is not usable."},
    {EC_RM_DRF_SYNTH_1_FAILED_NON_BCCH, "DRF TRX 2 PLL Unlocked -NON BCCH", "Radio Board NON-BCCH TRX 2 PLL is not locked, TRX 2 is not usable."},

    {EC_RM_BTS_LOCKED, "GSM Port Locked", "The GSM Port was locked successfully. Existing calls (if any) were aborted. No new calls can be made through GSM Port."},
    {EC_RM_BTS_UNLOCKED, "GSM Port Unlocked", "The GSM Port was unlocked successfully. GSM Port can accept new calls if its operational state is enabled."},
    {EC_RM_BTS_SHUTTINGDOWN, "GSM Port Shutting Down", "The GSM Port is being shutdown. No new calls can be made through GSM Port. Existing calls are maintained until terminates normally."},
    
    {EC_RM_TRX_1_LOCKED, "First TRX Locked ", "The first TRX was locked successfully. Locking of first TRX is equivalent of locking of GSM Port. Existing calls (if any) were aborted. No new calls can be made through GSM Port."},
    {EC_RM_TRX_2_LOCKED, "Second TRX Locked", "The second TRX was locked successfully. Existing calls (if any) were aborted. No new calls will be made through this TRX."},
    
    {EC_RM_TRX_1_UNLOCKED, "First TRX Unlocked", "The first TRX was unlocked successfully. New calls can be made through this TRX."},
    {EC_RM_TRX_2_UNLOCKED, "Second TRX Unlocked", "The second TRX was unlocked successfully. New calls can be made through this TRX."},
    
    {EC_RM_TRX_1_SHUTTINGDOWN, "First TRX shutting down ", "The first TRX is being shutting down. Shutting down first TRX is equivalent of shutting down GSM Port. No new calls can be made through GSM Port. Existing calls are maintained until terminates normally."},
    {EC_RM_TRX_2_SHUTTINGDOWN, "Second TRX shutting down ", "The second TRX is being shutting down. No new calls can be made through this TRX. Existing calls are maintained until terminates normally."},


    {EC_RM_TRX_1_SLOT_1_LOCKED, "First TRX Time Slot 0 locked", "Locking of Time Slot 0 of first TRX is equivalent of locking GSM Port. Existing calls (if any) were aborted. No new calls can be made through GSM Port."},
    {EC_RM_TRX_1_SLOT_2_LOCKED, "First TRX Time Slot 1 locked", "Time slot 1 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_3_LOCKED, "First TRX Time Slot 2 locked", "Time slot 2 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_4_LOCKED, "First TRX Time Slot 3 locked", "Time slot 3 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_5_LOCKED, "First TRX Time Slot 4 locked", "Time slot 4 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_6_LOCKED, "First TRX Time Slot 5 locked", "Time slot 5 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_7_LOCKED, "First TRX Time Slot 6 locked", "Time slot 6 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_8_LOCKED, "First TRX Time Slot 7 locked", "Time slot 7 of the first TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},  
    {EC_RM_TRX_2_SLOT_1_LOCKED, "Second TRX Time Slot 0 locked", "Time slot 0 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_2_LOCKED, "Second TRX Time Slot 1 locked", "Time slot 1 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_3_LOCKED, "Second TRX Time Slot 2 locked", "Time slot 2 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_4_LOCKED, "Second TRX Time Slot 3 locked", "Time slot 3 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_5_LOCKED, "Second TRX Time Slot 4 locked", "Time slot 4 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_6_LOCKED, "Second TRX Time Slot 5 locked", "Time slot 5 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_7_LOCKED, "Second TRX Time Slot 6 locked", "Time slot 6 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_8_LOCKED, "Second TRX Time Slot 7 locked", "Time slot 7 of the second TRX was locked successfully. Existing call (if any) was aborted. No new call can be made through this time slot."},

    {EC_RM_TRX_1_SLOT_1_UNLOCKED, "First TRX Time Slot 0 Unlocked", "Time slot 0 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_2_UNLOCKED, "First TRX Time Slot 1 Unlocked", "Time slot 1 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_3_UNLOCKED, "First TRX Time Slot 2 Unlocked", "Time slot 2 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_4_UNLOCKED, "First TRX Time Slot 3 Unlocked", "Time slot 3 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_5_UNLOCKED, "First TRX Time Slot 4 Unlocked", "Time slot 4 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_6_UNLOCKED, "First TRX Time Slot 5 Unlocked", "Time slot 5 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_7_UNLOCKED, "First TRX Time Slot 6 Unlocked", "Time slot 6 of the first TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_8_UNLOCKED, "First TRX Time Slot 7 Unlocked", "Time slot 7 of the first TRX was unlocked successfully. New calls can be made through this time slot."},   
    {EC_RM_TRX_2_SLOT_1_UNLOCKED, "Second TRX Time Slot 0 Unlocked", "Time slot 0 of the second TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_2_UNLOCKED, "Second TRX Time Slot 1 Unlocked", "Time slot 1 of the second TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_3_UNLOCKED, "Second TRX Time Slot 2 Unlocked", "Time slot 2 of the second TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_4_UNLOCKED, "Second TRX Time Slot 3 Unlocked", "Time slot 3 of the second TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_5_UNLOCKED, "Second TRX Time Slot 4 Unlocked", "Time slot 4 of the second TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_6_UNLOCKED, "Second TRX Time Slot 5 Unlocked", "Time slot 5 of the second TRX was unlocked successfully. New calls can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_7_UNLOCKED, "Second TRX Time Slot 6 Unlocked", "Time slot 6 of the second TRX was unlocked successfully. New calls can be made through this time slot."}, 
    {EC_RM_TRX_2_SLOT_8_UNLOCKED, "Second TRX Time Slot 7 Unlocked", "Time slot 7 of the second TRX was unlocked successfully. New calls can be made through this time slot."},

    {EC_RM_TRX_1_SLOT_1_SHUTTINGDOWN, "First TRX Time Slot 0 shutting down", "Time slot 0 of the first TRX is being shutdown. Shutting down Time Slot 0 of First TRX is equivalent of shutting down GSM Port. Existing calls will be maintained until terminate normally. No new calls can be made through GSM Port."},
    {EC_RM_TRX_1_SLOT_2_SHUTTINGDOWN, "First TRX Time Slot 1 shutting down", "Time slot 1 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_3_SHUTTINGDOWN, "First TRX Time Slot 2 shutting down", "Time slot 2 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_4_SHUTTINGDOWN, "First TRX Time Slot 3 shutting down", "Time slot 3 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_5_SHUTTINGDOWN, "First TRX Time Slot 4 shutting down", "Time slot 4 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_6_SHUTTINGDOWN, "First TRX Time Slot 5 shutting down", "Time slot 5 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_7_SHUTTINGDOWN, "First TRX Time Slot 6 shutting down", "Time slot 6 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_1_SLOT_8_SHUTTINGDOWN, "First TRX Time Slot 7 shutting down", "Time slot 7 of the first TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_1_SHUTTINGDOWN, "Second TRX Time Slot 0 shutting down", "Time slot 0 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_2_SHUTTINGDOWN, "Second TRX Time Slot 1 shutting down", "Time slot 1 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_3_SHUTTINGDOWN, "Second TRX Time Slot 2 shutting down", "Time slot 2 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_4_SHUTTINGDOWN, "Second TRX Time Slot 3 shutting down", "Time slot 3 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_5_SHUTTINGDOWN, "Second TRX Time Slot 4 shutting down", "Time slot 4 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_6_SHUTTINGDOWN, "Second TRX Time Slot 5 shutting down", "Time slot 5 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_7_SHUTTINGDOWN, "Second TRX Time Slot 6 shutting down", "Time slot 6 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},
    {EC_RM_TRX_2_SLOT_8_SHUTTINGDOWN, "Second TRX Time Slot 7 shutting down", "Time slot 7 of the second TRX is being shutdown. Existing call (if any) will be maintained until terminate normally. No new call can be made through this time slot."},

    {EC_RM_TRX_0_RADIO_OFF,"Radio for the first TRX  is off", "Radio for the first TRX  is off"},
    {EC_RM_TRX_1_RADIO_OFF,"Radio for the second TRX  is off", "Radio for the second TRX  is off"},
    {EC_RM_RECV_CORRUPTED_DSP_MSG, "RM detects corrupted Layer 1 message ", "RM module detected a corrupted message from Layer 1."},


    /* LAPDM Error Alarm Code */
    {EC_L2_DATA_CORRUPTED, "LAPDm Internal Failure", "Data corruption in the LAPDm Layer software. The system needs a reboot"},
    {EC_L2_FREE_FRAME_EXHAUSTED, "LAPDm Resource Exhausted", "LAPDm Layer ran out of available frame buffer. This is most likely caused by radio link problem with MS."},
    
    /* OAM Alarm */
    {EC_OAM_RELOAD_MIB,   "(Re)Load MIB", "New MIB have been loaded on %1. You most likely need to reboot the GSM Port"},
    {EC_OAM_ENABLE_TIMEX, "MIB save option Change - Timer enabled", "MIB save timer has been disabled on  %1."},
    {EC_OAM_DISABLE_TIMEX, "MIB save option Change - Timer disabled", "MIB save timer has been disabled on  %1."},
    {EC_OAM_ALLOW_DUPS, "MIB Option Change - Duplicate save allowed", "MIB save option change on %1."},
    {EC_OAM_SUPPRESS_DUPS, "MIB option change - Duplicate save not allowed", "Duplicate saving of the MIB is now disabled. This will improve the response time  of the set operation at GP Manager"},
    {EC_OAM_REBOOT, "GSM Port Operator-Initiated Reboot", "The operator has initiated a reboot on %1. GSM Port will reboot within a few seconds."},


    /* GMC Alarm */
    {EC_VIPERBASE_LINKDOWN, "GMC Link Down", "Detected link lost to GMC. Please check the operational status of GMC and the physical network connectivity between GMC and GSM Port."},


    /* CDR Client application */
    { EC_CDR_CLIENT_INIT_FAILED          , "CDR Client is unable to start"                      , "Validate CDR Client settings and reboot." },
    { EC_CDR_CLIENT_UNABLE_CONNECT_SERVER, "CDR Client is unable to communicate with CDR server", "Check CDR client settings. Validate SSL certificate, key and password. Make sure that they are compatible with the CDR server certificates. Reboot application." },
    { EC_CDR_CLIENT_BAD_SSL_PASSWORD_FILE, "Missing or invalid CDR password"                    , "Validate that CDR password file is present and correct. Regenerate or replace CDR password file. Reboot application." },
    { EC_CDR_CLIENT_BAD_SSL_KEY_FILE     , "Missing or invalid CDR key file"                    , "Unable to decrypt the user certificate key using local password. Validate that CDR key file is present and correct. Check that CDR password is compatible with the key. Regenerate CDR password file if the password is incompatible or replace the key file. Reboot application." },
    { EC_CDR_CLIENT_BAD_LOCAL_CERTIFICATE, "Missing or invalid local SSL certificate"           , "Validate the user, root certificates and the key file. Reboot application."  },
    { EC_CDR_CLIENT_BAD_PEER_CERTIFICATE , "Invalid peer SSL certificate"                       , "Peer certificate verification has failed. Make sure that CDR Server is running in SSL-enabled mode and has correct certificates. Verify that CDR Server is using trusted root certificate. If the local certificate is changed, reboot application." },


    /* RootTask Alarm */
    {EC_ROOT_MPC_SRAM_FAIL,        "MPC SRAM test failure",         "MCH Module's Test results indicates that there was a failure in MPC SRAM Test."},  
    {EC_ROOT_DSP0_INT_DATA_RAM_FAIL,"DSP0 internal data RAM test failure",     "MCH Module's Post Test results indicates that there was a failure in DSP0's internal data RAM Test."},  
    {EC_ROOT_DSP1_INT_DATA_RAM_FAIL,"DSP1 internal data RAM test failure",     "MCH Module's Post Test results indicates that there was a failure in DSP1's internal data RAM Test."},  
    {EC_ROOT_DSP0_INT_PROG_RAM_FAIL,"DSP0 internal program RAM test failure", "MCH Module's Post Test results indicates that there was a failure in DSP0's internal program RAM Test."},  
    {EC_ROOT_DSP1_INT_PROG_RAM_FAIL,"DSP1 internal program RAM test failure", "MCH Module's Post Test results indicates that there was a failure in DSP1's internal program RAM Test."},  
    {EC_ROOT_DSP0_EXT_RAM_FAIL,    "DSP0 external RAM test failure",     "MCH Module's Post Test results indicates that there was a failure in DSP0's external RAM Test."},  
    {EC_ROOT_DSP1_EXT_RAM_FAIL,    "DSP1 external RAM test failure",     "MCH Module's Post Test results indicates that there was a failure in DSP1's external RAM Test."},  
    {EC_ROOT_FPGA0_FAIL,        "FPGA0 test failure",             "MCH Module's Post Test results indicates that there was a failure in FPGA0 Test."},  
    {EC_ROOT_FPGA1_FAIL,        "FPGA1 test failure",             "MCH Module's Post Test results indicates that there was a failure in FPGA1 Test."},  
    {EC_ROOT_I2C_FAIL,        "I2C test failure",             "MCH Module's Post Test results indicates that there was a failure in I2C Test."},  
    {EC_ROOT_TASK_START_DELEY,    "ViperTask startup delay detected",     "One or more ViperTask has a delay on startup, reporting readiness late."}, 
    {EC_ROOT_SYSCMD_SYM_NOT_FOUND,    "SysCommand_xxx symbol not found",     "One or more module's SysCommand_xxx() function symbol is not found.  Check module loading logs."}, 
    {EC_ROOT_CRITICAL_TASK_SUSPEND,        "Critical Task suspension detected",     "Critical Task suspension from ModuleId %1 detected."}, 
    {EC_ROOT_CRITICAL_TASK_MISSING,        "Critical exit abnormally detected",    "Critical Task abnormally exited from ModuleID %1 detected."}, 
    {EC_ROOT_LOAD_MODULE_FAIL,    "LoadModule failure",            "Loading module object codes had problems.  Check startup logs."},
    {EC_ROOT_NON_CRITICAL_TASK_SUSPEND,        "Non-Critical Task suspension detected",     "Non-Critical Task suspension from ModuleId %1 detected."}, 
    {EC_ROOT_NON_CRITICAL_TASK_MISSING,        "Non-Critical exit abnormally detected",    "Non-Critical Task abnormally exited from ModuleID %1 detected."}, 
	{EC_ROOT_TX_POWER_TABLE_ERROR,		"TX power table error",	"The TX power table in the radio board EEPROM is corrupt."}, 

    /* Don't alter the last line */
    {-1, 0, 0}
    
};


extern "C" char * getAlarmSynopsis(int code)
{
  int i;

    for (i=0; alarmTable[i].errCode != -1; i++)
    {
        if (alarmTable[i].errCode == code)
            return alarmTable[i].synopsis;
    }

  return NULL;
}

extern "C" char * getAlarmDetail(int code)
{
  int i;

    for (i=0; alarmTable[i].errCode != -1; i++)
    {
        if (alarmTable[i].errCode == code)
            return alarmTable[i].detailMsg;
    }

  return NULL;
}

int main(int argc, char* argv[])
{
    int i;
    
    printf("// AlarmCode.English\n"); 
    printf("// Copyright (C) 2000 Cisco Systems, Inc. \n");
    printf("// All rights reserved \n");
    printf("// WARNING:  This file is automatically generated by an AlarmCode tool \n");
    printf("// Any changes made to this file will be overwritten by \n");
    printf("// subsequent uses of the tool. \n");
    printf("//  \n");
    printf("// ALARM_ERROR_CODE | \"ALARM_SYNOPSIS\"  | \"ALARM_DETAIL_MESSAGE\" \n");
    
    for (i=0; alarmTable[i].errCode != -1; i++)
    {
        printf("%10d | ", alarmTable[i].errCode);
        printf("%s | ", alarmTable[i].synopsis);
        printf("%s\n", alarmTable[i].detailMsg);
    }

  return 0;
}

