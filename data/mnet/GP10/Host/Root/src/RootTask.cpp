// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : RootTask.cpp
// Author(s)   : Tim Olson
// Create Date : 10/18/2000
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include <stdio.h>
#include <rebootLib.h>
#include <sysLib.h>
#include <rebootLib.h>
#include "RootTask.h"
#include "GP10OsTune.h"
#include "Os/JCModule.h"
#include "Os/JCTask.h"
#include "MCH/post.h"
#include "oam_api.h"

// *******************************************************************
// external routines declarations.
// *******************************************************************
// SysCommand
extern int      SysCommand_Log      (T_SYS_CMD  action);
extern int      SysCommand_Alarm    (T_SYS_CMD  action);
extern int      SysCommand_MCH      (T_SYS_CMD  action);
extern int      SysCommand_Lapdm    (T_SYS_CMD  action);
extern int      SysCommand_Voip     (T_SYS_CMD  action);
extern int      SysCommand_VChan    (T_SYS_CMD  action);
extern int      SysCommand_Ludb     (T_SYS_CMD  action);
extern int      SysCommand_MM       (T_SYS_CMD  action);
extern int      SysCommand_CC       (T_SYS_CMD  action);
extern int      SysCommand_L1Proxy  (T_SYS_CMD  action);
extern int      SysCommand_MD       (T_SYS_CMD  action);
extern int      SysCommand_RM       (T_SYS_CMD  action);
extern int      SysCommand_PM       (T_SYS_CMD  action);
extern int      SysCommand_CDR      (T_SYS_CMD  action);
extern int      SysCommand_TcpSrv   (T_SYS_CMD  action);
extern int      SysCommand_VBLink   (T_SYS_CMD  action);
extern int      SysCommand_RlcMac   (T_SYS_CMD  action);
extern int      SysCommand_GRR      (T_SYS_CMD  action);
extern int      SysCommand_BSSGP    (T_SYS_CMD  action);
extern int      SysCommand_GSLink   (T_SYS_CMD  action);

extern void		go (unsigned int port);
extern STATUS   RestoreEepromFromNVRam(void);
extern STATUS   CheckEeprom(void);
extern "C" STATUS   SaveEepromInNVRam(void);
extern void		csu_TblInitAll(void);	/* from \GP10\Host\CsuNew\src\csu_table.cpp */

#define			L1PROXY_DSP_PORT0		0
#define			L1PROXY_DSP_PORT1		1

#define EEPROM_STATUS_OK            0
#define EEPROM_STATUS_UPDATE        1
#define EEPROM_STATUS_REPORT_ERROR  2

// Flag used to indicate that the one and only root task
// has been created.
bool RootTask::isOneCreated = FALSE;

// Pointer to the one and only root task object.
RootTask *RootTask::theRootTask = 0;

// flags
short   gRebootDelayInSec = 10;
short   gModuleCheckDelayInSec = 10;

int isSelfRebootEnabled = TRUE;

// reboot status flag
static BOOL isSystemRebooting = FALSE;

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::RootTask
**
**    PURPOSE: RootTask constructor.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
RootTask::RootTask()
{
    isSystemRebooting = FALSE;
    isOneCreated = TRUE;
    
    if(rebootHookAdd((FUNCPTR)RootTask::RootRebootHook) != OK){
        printf("[RootTask] couldn't add reboot hook!\n");
    }

    // Create the task sync semaphore.
    pMnetSyncSem = new SEM_ID;
    *pMnetSyncSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    
    // Update tNetTask priority.
    taskPrioritySet(taskNameToId("tNetTask"), NET_TASK_PRIORITY);
    
    // create RootTask
    if(taskSpawn("RootTask", 
                 ROOT_TASK_PRIORITY1, 
                 ROOT_TASK_OPTION, 
                 ROOT_TASK_STACK_SIZE,
                 (FUNCPTR) RootTask::RootMain,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
    {
        // error creating task. log the error and return.
        printf("[RootTask] Fatal Error on taskSpawn(RootTask).\n");
    };  
}


extern "C"
{
int ShellInitRootTask()
{
    return (RootTask::InitRootTask());
}
}
/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::InitRootTask
**
**    PURPOSE: InitRootTask creates the one and only root task.  A static
**      member of RooTask called theRootTask is used to access the root task
**      object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): JC_OK - RootTask created.
**                     JC_ERROR - RootTask not created.
**
**----------------------------------------------------------------------------*/
int RootTask::InitRootTask()
{
    // Create the one and only RootTask object.
    if (isOneCreated == FALSE)
    {
        theRootTask = new RootTask();
    }
    
    if (theRootTask)
        return (JC_OK);
    else
        return (JC_ERROR);    
}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::SetSelfRebootMode
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): JC_OK - if mode changed.
**                     JC_ERROR - if mode not changed.
**
**----------------------------------------------------------------------------*/
extern "C"
{
JC_STATUS ShellSetSelfRebootMode(bool mode)
{
    return (RootTask::theRootTask->SetSelfRebootMode(mode));
}
}
JC_STATUS RootTask::SetSelfRebootMode(bool mode)
{
    if(mode)
    {
        if (isSelfRebootEnabled)
        {
            printf("[RootTask] Self Reboot Mode already in place.\n");
            return (JC_ERROR); 
        }
        else
        {
            isSelfRebootEnabled = mode;
            printf("[RootTask] Self Reboot Mode now turned on.\n");
            printf("(Reboot will happen on critical task suspend.)\n");
            return (JC_OK);
        }
    }
    else
    {
        if (!isSelfRebootEnabled)
        {
            printf("[RootTask] Self Reboot Mode already turned off.\n");
            return (JC_ERROR); 
        }
        else
        {
            isSelfRebootEnabled = mode;
            printf("[RootTask] Self Reboot Mode now turned off.\n");
            printf("(Reboot will not happen on critical task suspend.)\n");
            return (JC_OK);
        }
    }
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::EngageSelfReboot
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): JC_OK - if mode changed.
**                     JC_ERROR - if mode not changed.
**
**----------------------------------------------------------------------------*/
void RootTask::EngageSelfReboot(int i)
{
    if (isSelfRebootEnabled)
    {
    printf("[RootTask] Self Reboot Mode engaged\n");
    for(int j=0; j < MNET_MAX_MODULE_IDS; j++)
    {
        if((j!=i) && JCModule::systemModules[j])
        {
            JCModule::systemModules[j]->ShutdownModule();
        }
    }                
    
    printf("[RootTask] System will reboot in %d seconds.\n", gRebootDelayInSec);
    taskDelay(gRebootDelayInSec * sysClkRateGet());
    
    printf("[RootTask] Reboot...\n");
    reboot(BOOT_CLEAR);
    }
    else
    {
        printf("[RootTask] Self Reboot Mode NOT engaged\n");
    }
}


#define MOD_CHECK_DELAY_SECS    1
#define ROOT_MAX_MOD_CHECK_ATTEMPTS 60

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::WaitForModuleStatusCheck
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RootTask::WaitForModuleStatusCheck()
{
    bool allModulesReady = FALSE;
    int modCheckDelay = MOD_CHECK_DELAY_SECS * sysClkRateGet();
    int numAttempts = ROOT_MAX_MOD_CHECK_ATTEMPTS;
    
    while ((!allModulesReady) && (numAttempts != 0))
    {
        allModulesReady = JCModule::AllTasksInMainLoop();
        taskDelay(modCheckDelay);
        numAttempts--;
    }
    
    if (!allModulesReady)
    {
        printf ("[RootTask] WARNING: not all tasks are ready...continuing anyway!\n");
    }
    
    JCModule::ShowModuleStat();    
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::SendAlarm
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RootTask::SendAlarm(Alarm_errorCode_t err_code, int arg1=0, int arg2=0 )
{
    if (err_code  !=  EC_NOERROR)
    {
        if (JC_OK!=alarm_raise_args(MODULE_SYSINIT, MIB_errorCode_SYSINIT, 
            err_code, arg1, arg2))
        {
           printf("Unable to raise alarm: \"alarm_raise(0x%02x, 0x%08x)\" failed\n",
               MODULE_SYSINIT, err_code);
        }
    } else {
        if (JC_OK!=alarm_clear(MODULE_SYSINIT, MIB_errorCode_SYSINIT, EC_NOERROR))
        {
           printf("Unable to raise alarm: \"alarm_clear(0x%02x, 0x%08x)\" failed\n",
               MODULE_SYSINIT, err_code);
        }
    }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::UpdateRadioBoardEeprom
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RootTask::UpdateRadioBoardEeprom()
{
    int status;
    
    // Now check if any updates need to be made to EEPROM
    if ((status = CheckEeprom()) == EEPROM_STATUS_REPORT_ERROR)
    {
        SendAlarm(EC_ROOT_TX_POWER_TABLE_ERROR);
        printf("[RootTask] WARNING - Radio Board EEPROM is corrupted!\n"
               "Default power tables will be used!\n");
    }
    else if (status == EEPROM_STATUS_UPDATE)
    {
        printf("[RootTask] Saving updated Radio Board EEPROM into NV Ram!\n");
        if (SaveEepromInNVRam() != STATUS_OK)
        {
            printf("[RootTask] WARNING - Unable to save Radio Board EEPROM!\n");
        }
    }
}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::CheckForMCHTest
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
short RootTask::CheckForMCHTest()
{
    /* looks like post test results booleans are kept differently from
       the name of functions - "1" means fail.  
       Interpreted correctly here : kevinelim */

    bool any_fail = postAllTestsOkay();
    bool mpc_sram_fail = postMpcSramOkay();
    bool dsp0_int_data_fail = postDsp0IntDataRamOkay();
    bool dsp1_int_data_fail = postDsp1IntDataRamOkay();
    bool dsp0_int_prog_fail = postDsp0IntProgRamOkay();
    bool dsp1_int_prog_fail = postDsp1IntProgRamOkay();
    bool dsp0_ext_fail = postDsp0ExtRamOkay();
    bool dsp1_ext_fail = postDsp1ExtRamOkay();
    bool fpga0_fail = postFPGA0Okay();
    bool fpga1_fail = postFPGA1Okay();
    bool i2c_fail = postI2COkay();
    short ret = 0;

    printf("[RootTask] checking for post test - any failed?: %d\n", any_fail);
    if(any_fail){
        if(mpc_sram_fail){
            printf("-- mpc_sram fail\n");
            SendAlarm(EC_ROOT_MPC_SRAM_FAIL);
        }
        if(dsp0_int_data_fail){
            printf("-- dsp0_int_data fail\n");
            SendAlarm(EC_ROOT_DSP0_INT_DATA_RAM_FAIL);
        }
        if(dsp1_int_data_fail){
            printf("-- dsp1_int_data fail\n");
            SendAlarm(EC_ROOT_DSP1_INT_DATA_RAM_FAIL);
        }
        if(dsp0_int_prog_fail){
            printf("-- dsp0_int_prog fail\n");
            SendAlarm(EC_ROOT_DSP0_INT_PROG_RAM_FAIL);
        }
        if(dsp1_int_prog_fail){
            printf("-- dsp1_int_prog fail\n");
            SendAlarm(EC_ROOT_DSP1_INT_PROG_RAM_FAIL);
        }
        if(dsp0_ext_fail){
            printf("-- dsp0_ext fail\n");
            SendAlarm(EC_ROOT_DSP0_EXT_RAM_FAIL);
        }
        if(dsp1_ext_fail){
            printf("-- dsp1_ext fail\n");
            SendAlarm(EC_ROOT_DSP1_EXT_RAM_FAIL);
        }
        if(fpga0_fail){
            printf("-- fpga0 fail\n");
            SendAlarm(EC_ROOT_FPGA0_FAIL);
        }
        if(fpga1_fail){
            printf("-- fpga1 fail\n");
            SendAlarm(EC_ROOT_FPGA1_FAIL);
        }
        if(i2c_fail){
            printf("-- i2c fail\n");
            SendAlarm(EC_ROOT_I2C_FAIL);
        }
        ret = 1;
    }
    return ret;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::InitializeSystemModuleList
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RootTask::InitializeSystemModuleList()
{
    for (int i=0; i < MNET_MAX_MODULE_IDS; i++)
    {
        JCModule::systemModules[i] = 0;
    }

    JCModule::systemModules[0] = new JCModule(MODULE_LOG, SysCommand_Log, "SysCommand_Log");
    JCModule::systemModules[1] = new JCModule(MODULE_TCPSRV, SysCommand_TcpSrv, "SysCommand_TcpSrv");
    JCModule::systemModules[2] = new JCModule(MODULE_ALARM, SysCommand_Alarm, "SysCommand_Alarm");
    JCModule::systemModules[3] = new JCModule(MODULE_MCH, SysCommand_MCH, "SysCommand_MCH");
    JCModule::systemModules[4] = new JCModule(MODULE_VBLINK, SysCommand_VBLink, "SysCommand_VBLink");
    JCModule::systemModules[5] = new JCModule(MODULE_CDR, SysCommand_CDR, "SysCommand_CDR");
    JCModule::systemModules[6] = new JCModule(MODULE_PM, SysCommand_PM, "SysCommand_PM");
    JCModule::systemModules[7] = new JCModule(MODULE_L2, SysCommand_Lapdm, "SysCommand_Lapdm");
    JCModule::systemModules[8] = new JCModule(MODULE_H323, SysCommand_Voip, "SysCommand_Voip");
    JCModule::systemModules[9] = new JCModule(MODULE_EXT_VC, SysCommand_VChan, "SysCommand_VChan");
    JCModule::systemModules[10] = new JCModule(MODULE_LUDB, SysCommand_Ludb, "SysCommand_Ludb");
    JCModule::systemModules[11] = new JCModule(MODULE_MM, SysCommand_MM, "SysCommand_MM");
    JCModule::systemModules[12] = new JCModule(MODULE_CC, SysCommand_CC, "SysCommand_CC");
    JCModule::systemModules[13] = new JCModule(MODULE_L1, SysCommand_L1Proxy, "SysCommand_L1Proxy");
    JCModule::systemModules[14] = new JCModule(MODULE_MD, SysCommand_MD, "SysCommand_MD");
    JCModule::systemModules[15] = new JCModule(MODULE_RLCMAC, SysCommand_RlcMac, "SysCommand_RlcMac");
    JCModule::systemModules[16] = new JCModule(MODULE_GRR, SysCommand_GRR, "SysCommand_GRR");
    JCModule::systemModules[17] = new JCModule(MODULE_RM, SysCommand_RM, "SysCommand_RM");
    JCModule::systemModules[18] = new JCModule(MODULE_BSSGP, SysCommand_BSSGP, "SysCommand_BSSGP");
    JCModule::systemModules[19] = new JCModule(MODULE_GSLINK, SysCommand_GSLink, "SysCommand_GSLink");
    return;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::RootMain
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int RootTask::RootMain(void)
{
    int i;
    bool isSysModuleMissing = FALSE;
    
	csu_TblInitAll();                                          /* klim 5/9/01 */

    theRootTask->InitializeSystemModuleList();
    
    for(i=0; i<MNET_MAX_MODULE_IDS; i++)
    {
        if (JCModule::systemModules[i])
        {
            if (!JCModule::systemModules[i]->StartModule())
            {
                isSysModuleMissing = TRUE;
            }
        }
    }

    // Verify the EEPROM on the radio board.
    theRootTask->UpdateRadioBoardEeprom();
        
    theRootTask->WaitForModuleStatusCheck();
    
    if (pMnetSyncSem)
        semFlush(*pMnetSyncSem);
    
    go (L1PROXY_DSP_PORT1);
    go (L1PROXY_DSP_PORT0);

    if (isSysModuleMissing)
    {
        theRootTask->SendAlarm(EC_ROOT_SYSCMD_SYM_NOT_FOUND);
    }
    
    theRootTask->CheckForMCHTest();
    
    taskPrioritySet(taskNameToId("RootTask"), ROOT_TASK_PRIORITY2);

    while (1) {
        for(i=0; i<MNET_MAX_MODULE_IDS; i++)
        {
            if ((JCModule::systemModules[i]) && (!isSystemRebooting))
            {
                JCTask *pTask;
                SYSTEM_MODULE_STATUS modStat;
                if (((modStat = JCModule::systemModules[i]->CheckModule(&pTask)) != MODULE_OK)) 
                {
                    if ((modStat == TASK_MISSING) && (!pTask->GetTaskAlarmStatus()))
                    {
                        if (pTask->GetTaskImportance() == JC_CRITICAL_TASK)
                            theRootTask->SendAlarm(EC_ROOT_CRITICAL_TASK_MISSING, 
                                JCModule::systemModules[i]->GetModuleId());
                        else  
                            theRootTask->SendAlarm(EC_ROOT_NON_CRITICAL_TASK_MISSING,
                                JCModule::systemModules[i]->GetModuleId());  
                        pTask->SetTaskAlarmStatus(TRUE);
                    }
                  
                    if ((modStat == TASK_SUSPENDED) && (!pTask->GetTaskAlarmStatus()))
                    {
                        if (pTask->GetTaskImportance() == JC_CRITICAL_TASK)
                            theRootTask->SendAlarm(EC_ROOT_CRITICAL_TASK_SUSPEND, 
                                JCModule::systemModules[i]->GetModuleId());
                        else
                            theRootTask->SendAlarm(EC_ROOT_NON_CRITICAL_TASK_SUSPEND,
                                JCModule::systemModules[i]->GetModuleId());
                        pTask->SetTaskAlarmStatus(TRUE);
                    }
                  
                    if (pTask->GetTaskImportance() == JC_CRITICAL_TASK)
                    {
                        theRootTask->EngageSelfReboot(i);
                    }
                }
            }
        }
        taskDelay(gModuleCheckDelayInSec * sysClkRateGet());
    }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RootTask::RootRebootHook
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void RootTask::RootRebootHook()
{
    int i, j;
    printf("[RootTask] Reboot preparation in progress\n");
    for(i=0; i<MNET_MAX_MODULE_IDS; i++)
    {
        if (JCModule::systemModules[i])
        {
            JCModule::systemModules[i]->RebootModule();
        }
    }
    isSystemRebooting = TRUE;
    printf("[RootTask] System reboot in progress...\n");
    for(i=0; i<1000; i++) for(j=0; j<5000; j++);    // give some time to print last line
}


