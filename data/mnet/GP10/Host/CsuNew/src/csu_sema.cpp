/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __CSU_SEMA_CPP__
#define __CSU_SEMA_CPP__

#include "CsuNew\csu_head.h"

void csu_CreateSemaphore(void)
{
     DBG_FUNC("csu_CreateSemaphore", CSU_LAYER);
     // DBG_ENTER();

     csu_SemaId = semMCreate(SEM_Q_PRIORITY|SEM_INVERSION_SAFE);
     if (csu_SemaId == NULL)
         DBG_ERROR("ERROR@csu_CreateSemaphore: semMCreate failed\n");    
}

void csu_TakeSemaphore(void)
{
     semTake(csu_SemaId,WAIT_FOREVER);
}

void csu_GiveSemaphore(void)
{
    semGive(csu_SemaId);
}

#endif //__CSU_SEMA_CPP__

 

