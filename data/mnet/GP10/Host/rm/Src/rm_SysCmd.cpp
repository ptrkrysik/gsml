/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SYSCMD_CPP__
#define __RM_SYSCMD_CPP__

#include "rm\rm_head.h"
#include "Os\JCModule.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cdcUtils/drfInterface.h"

#ifdef __cplusplus
}
#endif


int  SysCommand_RM (T_SYS_CMD action) 
{
    switch(action)
    {
    case SYS_SHUTDOWN:
         printf("[RM] shutdown notification received\n");
         break;
    case SYS_START:
	     printf("[RM] Received task start notification\n");
         sys_Init();
         rm_Task();
    	   break;
    case SYS_REBOOT:
         printf("[RM] Reboot: Turning off RF power\n");
         /* Turn off RF power */
         drfFpgaTxPwrDisable(0);   /* take away power control from dsp */
         drfFpgaTxPwrDisable(1);   /* take away power control from dsp */
         drfTxPwrSet(0,0);         /* turn off power */
         drfTxPwrSet(1,0);         /* turn off power */
         break;
    }
    return 0;
}

#endif /* __RM_SYSCMD_CPP__ */
