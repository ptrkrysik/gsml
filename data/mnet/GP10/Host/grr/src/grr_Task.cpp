/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_TASK_CPP__
#define __GRR_TASK_CPP__

#include "grr\grr_head.h"

void grr_CreateQ(void)
{
  DBG_FUNC("grr_CreateQ", GRR_LAYER);
  DBG_ENTER();

  //Create message queue for GRR module
  grr_MsgQId = msgQCreate (
 	                     GRR_MAX_RXMSGS,	   //Max # of messages
	                     sizeof(grr_ItcRxMsg_t), //Max length of msg
	                     MSG_Q_PRIORITY          //Priority-type que
			       );

   if( grr_MsgQId == NULL )
   {
       printf("grr_CreateQ: failed to create GRR Queue, errno (%d)\n", errno);
       DBG_ERROR("grr_CreateQ: failed to create GRR Queue, errno (%d)\n", errno);
       grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQCREATE_FAILED);
       DBG_LEAVE();
       assert( (grr_MsgQId!=NULL) );
   }
   
   DBG_LEAVE();
}

void grr_Main( void )
{
  DBG_FUNC("grr_Main", GRR_LAYER);
  DBG_ENTER();

  //mainLoop
  grr_StartDspsWatch();
  grrTaskObj.JCTaskEnterLoop();
  //ViperTaskEnterLoop();

  //InitGRR
  grr_Init();
  printf("grr_Main: GRR finished initialization\n");
 
  while(true)
  {	
    int length;

    //Confirm GRR is staying in its main-loop
    DBG_TRACE("grr_Main: staying in mainloop\n");

    // Retrieve a message from Q.
    if ((length = msgQReceive(grr_MsgQId,(char*)&grr_ItcRxMsg, GRR_MAX_RXQMSG_LENGTH, WAIT_FOREVER))
         == ERROR )
    {
       //failed to receive message. VxWorks error. Skip this msg!
       grr_SubmitAlarm(EC_GRR_OS_CALL_MSGQRECEIVE_FAILED);
       DBG_ERROR("grr_Main: failed to read VxWorks GRR Queue err(%d)\n", errno);
       printf("grr_Main: failed to read VxWorks GRR msg Queue err(%d)", errno);
       assert(0);
    } else 
    {
       //Good receive! If needed dump out for debug purpose
       if (grr_DbgDump) DBG_HEXDUMP(( unsigned char *)&grr_ItcRxMsg, length);

       //Handle the message in terms of origin, apply sanitory check!
       grr_ItcRxOrigin = grr_ItcRxMsg.module_id;

       //Go in terms of origin
       switch (grr_ItcRxOrigin)
       {
       case MODULE_GRR:
            //Received GRR messages
            grr_pItcRxGrrMsg=(grr_ItcRxGrrMsg_t *)&grr_ItcRxMsg.itcRxGrrMsg;   

            //If needed output the fact RM got a RM message for trace
            DBG_TRACE("GRR<=GRR@grr_Main: primitive(%d) message(%x)\n",
                       grr_pItcRxGrrMsg->primitive_type,
                       grr_pItcRxGrrMsg->message_type);

            //Process watch-dog timer expiry here
            if (grr_pItcRxGrrMsg->primitive_type==GRR_DSP_WDOG_EXPIRED)
                grr_ProcGrrMsg();
            else
                DBG_WARNING("GRR<=GRR: unexpected msg (%d) msg(0x%x)\n",
				     grr_pItcRxGrrMsg->message_type);
            break;

       case MODULE_L1:
            //Received L1 messages
            grr_pItcRxDspMsg=(grr_ItcRxDspMsg_t *)&grr_ItcRxMsg.itcRxDspMsg;

            //If needed output trace for debugging purpose
            DBG_TRACE("GRR<=L1@grr_Main: received DSP MSG msg(%x,%x,%x) src(%x,%x,%x)\n",
                       grr_pItcRxDspMsg->buffer[0],
                       grr_pItcRxDspMsg->buffer[1],
                       grr_pItcRxDspMsg->buffer[2],
                       grr_pItcRxDspMsg->buffer[3],
                       grr_pItcRxDspMsg->buffer[4],
                       grr_pItcRxDspMsg->buffer[5]);
                      
            grr_ProcDspMsg();

            break;

       case MODULE_OAM:
            //Received OAM messages
            grr_pItcRxOamMsg = (grr_ItcRxOamMsg_t *)&grr_ItcRxMsg;

            //If needed output trace for debugging purpose
            DBG_TRACE("GRR<=OAM@grr_Main: received OAM MSG msgTyp(%d) Tag(%x) valTyp(%d) val(%d)\n",
                       grr_pItcRxOamMsg->msgType,
	                 grr_pItcRxOamMsg->mibTag,
		 	     grr_pItcRxOamMsg->valType,
			     grr_pItcRxOamMsg->val.varVal);

            //
            grr_ProcOamMsg();

            break;

       case MODULE_RM:
            //Received RRM messages
            grr_pItcRxRrmMsg = (grr_ItcRxRrmMsg_t *)&grr_ItcRxMsg;

            //If needed output trace for debugging purpose
            DBG_TRACE("GRR<=RRM@grr_Main: received RRM MSG msgTyp(%d) trx(%d) slot(%d)\n",
                       grr_pItcRxRrmMsg->message_type,
                       grr_pItcRxRrmMsg->trx,
                       grr_pItcRxRrmMsg->slot);
            //
            grr_ProcRrmMsg();
            break;

       case MODULE_RLCMAC:
            //Received RLC messages
            grr_pItcRxRlcMsg = (grr_ItcRxRlcMsg_t *)&grr_ItcRxMsg;

            //If needed output trace for debugging purpose
            DBG_TRACE("GRR<=RRM@grr_Main: received RRM MSG msgTyp(%d) trx(%d) slot(%d)\n",
                       grr_pItcRxRlcMsg->message_type,
                       grr_pItcRxRlcMsg->trx,
                       grr_pItcRxRlcMsg->slot);
            //
            grr_ProcRlcMsg();
            break;

       default:
            //Received unexpected messages
            DBG_WARNING("GRR<=???@grr_Main:received msg from invalid moduleID (%d)\n", 
                         grr_ItcRxOrigin);
            break;

       } //switch()

    } //else

  } //while()

} //grr_Main()

int SysCommand_GRR(T_SYS_CMD action)
{
   int tid;

   switch(action)
   {
   case SYS_SHUTDOWN:
 	  printf("[GRR] Received system shutdown notification\n");
	  break;

   case SYS_START:
  	  printf("[GRR] Received task start notification\n");
  	  grr_CreateQ();
	  tid = grrTaskObj.JCTaskSpawn(
					GRR_TASK_PRIORITY,
					GRR_TASK_OPTION,
					GRR_TASK_STACK_SIZE,
					(FUNCPTR)grr_Main,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					MODULE_GRR, JC_CRITICAL_TASK);
                    
      grrMonTaskObj.JCTaskSpawn(
			        GRR_MONITOR_TASK_PRIORITY,
			        GRR_MONITOR_TASK_OPTION,
			        GRR_MONITOR_TASK_STACK_SIZE,
			        (FUNCPTR)grr_RadioMonitor,
			        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    MODULE_GRR, JC_CRITICAL_TASK)				;
	  break;

   case SYS_REBOOT:
	  printf("[GRR] Reboot ready.\n");
	  break;

   default:
	  printf("[GRR] Received unknown system command\n");
   }

   return 0;
}

#endif //__GRR_TASK_CPP__

