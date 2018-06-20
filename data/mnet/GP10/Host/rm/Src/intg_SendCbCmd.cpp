/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDCBCMD_CPP__
#define __INTG_SENDCBCMD_CPP__

#include "rm/rm_head.h"

void intg_SendCbCmd(u8 cmdCode, u16 msgCode, u16 updateNo)
{
    rm_ItcRxVbMsg_t msg;
    int		  length;
    unsigned short  hdr12,hdr34;
    unsigned char   hdr5,hdr6;
    
    switch(cmdCode)
    {
    case 1: //Send test CB message to DSP
            memset((s8 *)&msg, 0, 110); //sizeof(rm_ItcRxVbMsg_t));
            hdr12=0;hdr34=0;hdr5=0;hdr6=0;
            msg.module_id = MODULE_SMSCBC;
            msg.message_type   = INTER_SMSCBC_CB_START;
            msg.size  = 88; //one page
            hdr12 = (00<<14)|(msgCode<<4)|(updateNo);
            length = 0;
            //Serial Number:cell wide immediate display
            msg.data[length++] = (u8)(hdr12>>8); 
            msg.data[length++] = (u8)(hdr12);
            //Message Identifier: 1 assumed
            msg.data[length++] = 0;
            msg.data[length++] = 1;
            //Data coding scheme: default alphabet, English
            msg.data[length++] = 0x01;
            //Page parameter: 0x11, one page only
            msg.data[length++] = 0x11;
            //contents: Go JetCell!
            msg.data[length++] = 0xc7;
            msg.data[length++] = 0x37;
            msg.data[length++] = 0x48;
            msg.data[length++] = 0x59;
            msg.data[length++] = 0xa6;
            msg.data[length++] = 0x0f;
            msg.data[length++] = 0xd9;
            msg.data[length++] = 0xec;
            msg.data[length++] = 0x10;
            msg.data[length++] = 0x08;
            msg.data[length++] = 0x04;
            msg.data[length++] = 0x02;
            msg.data[length++] = 0x81;
            break;
            
    case 0: //Stop sending cur CB message
            msg.module_id = MODULE_SMSCBC;
            msg.message_type   = INTER_SMSCBC_CB_STOP;
            msg.size = 0;
            break;
    }
    if (cmdCode<2)
    {
        //Send a test CB message to RM
        if (ERROR==msgQSend(rm_MsgQId, (s8 *)&msg, RM_MAX_TXQMSG_LENGTH,
	                      NO_WAIT, MSG_PRI_NORMAL) )
            printf("ERROR@intg_SendCbCmd: msgQSend errno:%d\n", errno);
    } else
    {
        printf("usage@intg_SendCbCmd: cmdCode: 0-stop CB, 1-broad test CB\n");
    } 

} 

#endif //__INTG_SENDCBCMD_CPP__

