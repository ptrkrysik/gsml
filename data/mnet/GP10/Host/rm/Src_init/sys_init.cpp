/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __SYS_INIT_CPP__
#define __SYS_INIT_CPP__

#include <vxWorks.h>
#include <msgQLib.h>

#include "rm\sys_init.h"

//DBGADDED
#include "logging\vclogging.h"

#include "rm\rm_itcmsg.h"
#include "rm\rm_data.h"
#include "rm\rm_oamif.h"
#include "rm\debug.h"


//DBGADDED
DbgOutput rm_Dbg;

T_CNI_L3_ID	         rm_id;
MSG_Q_ID             rm_MsgQId;
rm_SmCtxts_t	   rm_SmCtxt;
rm_IrtEntryId_t	   rm_TchfEntryId[16];   //TBD:RM_L1MAX_TCHFS];
rm_IrtEntryId_t	   rm_Sdcch4EntryId[5];  //TBDRM_L1MAX_SDCCH4S];
rm_OamData_t         rm_OamData; 

rm_PhyChanTchf_t   rm_PhyChanTchf;
rm_PhyChanSdcch4_t rm_PhyChanSdcch4;

unsigned char rm_DspCiphCap;

void sys_Init(void)
{
   //Monitoring entrance to a function
   //RDEBUG__(("ENTER-sys_Init\n"));

   sys_CreateMsgQs();
   sys_StartTasks();

} //End of sys_Init()

void sys_StartTasks(void)
{
   //Monitoring entrance to a function
   //RDEBUG__(("ENTER-sys_StartTasks\n"));

   //Start OAM task
   //TBD

   //Start LAPDm task
   //TBD: ToGo signal is self-contained and following a task start.
   //     e.g. LAPDm initialization function will follow start of LAPDm
   //     task: CNI_LAPDm_Initialization();
  
   //Start MD task
   //TBD

   //Start CC task
   //TBD

   //Start MM task
   //TBD

   //Start RM task
   //TBD

   //Start L1IF task
   //TBD

} //End of sys_StartTasks()

void sys_CreateMsgQs(void)
{
   //Monitoring entrance to a function
   //RDEBUG__(("ENTER-sys_CreateMsgQs\n"));

   //Create message queue for RM module
   rm_MsgQId = msgQCreate (
	              RM_MAX_RXMSGS,	        //Max # of messages
	              sizeof(rm_ItcRxMsg_t),  //Max length of msg
	              MSG_Q_PRIORITY          //Priority-type que
			        );

   if( rm_MsgQId == NULL )
       //EDEBUG__(("ERROR-sys_init:RM message queue creation failed\n"));
       printf("ERROR-sys_init:RM message queue creation failed\n");
   else
   {
       //Generate L3 ID for RM for use by rm_InitIRT()
       //TDEBUG__(("TRACK-sys_Init: RM incoming message Q created ! \n"));
       rm_id.msgq_id = rm_MsgQId;
       rm_id.sub_id  = RM_L3_SUBID;
   }

   //Create message queue for MM module
   //TBD
   //Create message queue for CC module
   //TBD

} //End of sys_CreateMsgQs()

#endif //__SYS_INIT_CPP__
