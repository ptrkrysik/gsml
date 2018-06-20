// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

#ifndef JCCbcCb_CPP
#define JCCbcCb_CPP

// *******************************************************************
// Include Files.
// *******************************************************************
#include "logging\vclogging.h"
#include "JCC/viperchannel.h"
#include "JCC/JCCbcCb.h"


extern MSG_Q_ID             rm_MsgQId;

bool JcMsgRcvBySmscbc(InterRxCbcCbMsg_t *msg)
{
  InterTxCbcCbMsg_t ack;
  int size;

  DBG_FUNC("JcMsgRcvBySMSCBC", RM_LAYER);
  // DBG_ENTER();

  size = 12 + msg->msgData.size;

  switch (msg->msgType)
  { 
  JcVipercellId_t            origVcId;       // origination ViperCell Id 
  UINT32                     origVcAddress;  // origination ViperCell IP address
  JcModuleId_t               origModuleId;   // origination Module Id
  JcSubId_t                  origSubId;      // optional origination Sub Id

  JcVipercellId_t            destVcId;       // destination ViperCell Id 
  UINT32                     destVcAddress;  // destination ViperCell IP address
  JcModuleId_t               destModuleId;   // destination Module Id
  JcSubId_t                  destSubId;      // optional destination Sub Id

  case INTER_SMSCBC_CB_START:
  case INTER_SMSCBC_CB_STOP:
       //Calculate the size of the received SMSCBC message
       size = 12 + msg->msgData.size;
 
       //Output indication that a SMSCBC message arrived at ViperCell
       DBG_TRACE("Received SMSCBC message: msgType=%d\n",msg->msgType);
       DBG_HEXDUMP((unsigned char*)&msg->msgData, size);
        //Send back acknowledgement to this msg
       if (msg->msgType==INTER_SMSCBC_CB_START)
           ack.msgType = INTER_SMSCBC_CB_START_ACK;
       else if (msg->msgType==INTER_SMSCBC_CB_STOP)
           ack.msgType = INTER_SMSCBC_CB_STOP_ACK;
       ack.origVcId = msg->destVcId;
       ack.origVcAddress = msg->destVcAddress;
       ack.origModuleId = msg->destModuleId;
       ack.origSubId = msg->destSubId;
       ack.destVcId = msg->origVcId;
       ack.destVcAddress = msg->origVcAddress;
       ack.destModuleId = msg->origModuleId;
       ack.destSubId = msg->origSubId;
       JcMsgSendToSmscbc(&ack);
	break;

   default:
       //Output warning that a wrong message is being delivered to RM module
       DBG_WARNING("WARNING@JcMsgRcvBySmscbc: unsupported SMSCBC msgType=%d\n",
                    msg->msgType);
       return (false);
	break;
  }

  // pass up message to module RM
  if (ERROR == msgQSend( rm_MsgQId,
		             (char *)&msg->msgData,
			       size,
			       NO_WAIT,
			       MSG_PRI_NORMAL
	  	       ) )
  {
      DBG_ERROR("ERROR@JcMsgRcvBySmscbc: failed in msgQSend, errno=%d\n ",errno);
      return (false);
  }
  else
  {
      //DBG_TRACE("INFO@JcMsgRcvBySmscbc: Sent Message from CBC to VC: msgType=%d\n",  
	//  	       msg->msgType);
      return (true);
  }
}

bool 
JcMsgSendToSmscbc(InterTxCbcCbMsg_t *msg)
{
    BOOL retVal;

    if ((retVal = ViperChannelSendMessage((unsigned char *)msg, 
				 	   sizeof(struct  InterTxCbcCbMsg_t),
					   msg->destVcAddress
					     ))
		== TRUE)
		{
			return(true);
		}
	else
		{
			return(false);
		}
		  
}


//test command used for pre-CB index verstion
//void intg_JcSendCbCmd(unsigned char  cmdCode,
//                      unsigned short  msgCode,
//                      unsigned short updateNo)
//{
//    unsigned short   hdr12;
//    InterRxCbcCbMsg_t msg;
//    unsigned char    buffer[88];
//
//    switch(cmdCode)
//    {
//    case 0: //Stop sending cur CB message
//            msg.destModuleId = MODULE_SMSCBC;
//            msg.msgType = INTER_SMSCBC_CB_STOP;
//            msg.msgData.module_id = MODULE_SMSCBC;
//            msg.msgData.message_type = INTER_SMSCBC_CB_STOP;
//            msg.msgData.size = 0;
//		msg.msgData.message_id = msgCode;/*Danny, 6/29*/
//            break;
//
//    default: //Send test CB message to DSP
//            msg.destModuleId = MODULE_SMSCBC;
//            msg.msgType = INTER_SMSCBC_CB_STOP;
//            msg.msgData.module_id = MODULE_SMSCBC;
//            msg.msgData.message_type = INTER_SMSCBC_CB_START;
//
//            msg.msgData.module_id = MODULE_SMSCBC;
//            msg.msgData.message_type = INTER_SMSCBC_CB_START;
//            memset((char *)buffer, 0, 88);
//            hdr12=0;
//            hdr12 = (00<<14)|(msgCode<<4)|(updateNo);//00xxxxxxxxYYYY; x is msgCode, Y is Update#
//            //Serial Number:cell wide immediate display
//            buffer[0] = (unsigned char)(hdr12>>8); 
//            buffer[1] = (unsigned char)(hdr12);
//            //Message Identifier: 1 assumed
//            buffer[2] = 0;
//            buffer[3] = 1;
//            //Data coding scheme: default alphabet, English
//            buffer[4] = 0x01;
//            //Page parameter: page 1 of 3 
//            buffer[5] = 0x13;
//            //contents: JetCell, Cool!
//            buffer[6] = 0xca;
//            buffer[7] = 0x32;
//            buffer[8] = 0x7d;
//            buffer[9] = 0x58;
//            buffer[10] = 0x66;
//            buffer[11] = 0xb3;
//            buffer[12] = 0x59;
//            buffer[13] = 0xc3;
//            buffer[14] = 0xf7;
//            buffer[15] = 0x9b;
//            buffer[16] = 0x1d;
//            buffer[17] = 0x02;
//            buffer[18] = 0x81;
//
//            //Generate page 1 of 3
//            buffer[5] = 0x13;
//            buffer[19] = 0x62;
//            memcpy((unsigned char*)&msg.msgData.data[0], buffer, 88);
//
//            //Generate page 2 of 3
//            buffer[5] = 0x23;
//            buffer[19] = 0x64;
//            memcpy((unsigned char*)&msg.msgData.data[88], buffer, 88);
//
//            //Generate page 3 of 3
//            buffer[5] = 0x33;
//            buffer[19] = 0x66;
//            memcpy((unsigned char*)&msg.msgData.data[176], buffer, 88);
//              
//            //size of this CB msg
//            msg.msgData.size = 264;
//            break;            
//    }
//
//    //Send a test CB message to RM
//    if (!JcMsgRcvBySmscbc(&msg))
//        printf("FAILED in calling intg_JcSendCbCmd: cmdCode=%d, msgCode=%d, upNo=%d\n",
//                cmdCode, msgCode, updateNo);
//    else
//        printf("SUCCESS in calling intg_JcSendCbCmd: cmdCode=%d, msgCode=%d, upNo=%d\n",
//                cmdCode, msgCode, updateNo);
//}
//#endif

//Test command for CB index version
//message id = 1 assumed. 
void intg_JcSendCbCmd(unsigned char  cmdCode,
                      unsigned short  msgCode,
                      unsigned short updateNo)
{
    unsigned short   hdr12;
    InterRxCbcCbMsg_t msg;
    unsigned char    buffer[88];

    switch(cmdCode)
    {
    case 0: //Stop sending cur CB message
            msg.destModuleId = MODULE_SMSCBC;
            msg.msgType = INTER_SMSCBC_CB_STOP;

            msg.msgData.module_id = MODULE_SMSCBC;
            msg.msgData.message_type = INTER_SMSCBC_CB_STOP;
            msg.msgData.size = 0;
		msg.msgData.message_id = 1;   //Danny, 6/29
            break;

    default: //Send test CB message to DSP
            msg.destModuleId = MODULE_SMSCBC;
            msg.msgType = INTER_SMSCBC_CB_START;

            msg.msgData.module_id = MODULE_SMSCBC;
            msg.msgData.message_type = INTER_SMSCBC_CB_START;
            msg.msgData.message_id = 1;
            memset((char *)buffer, 0, 88);
            hdr12=0;
            hdr12 = (00<<14)|(msgCode<<4)|(updateNo);//00xxxxxxxxYYYY; x is msgCode, Y is Update#
            //Serial Number:cell wide immediate display
            buffer[0] = (unsigned char)(hdr12>>8); 
            buffer[1] = (unsigned char)(hdr12);
            //Message Identifier: 1 assumed
            buffer[2] = 0;
            buffer[3] = cmdCode;
            //Data coding scheme: default alphabet, English
            buffer[4] = 0x01;
            //Page parameter: page 1 of 3 
            buffer[5] = 0x13;
            //contents: JetCell, Cool!
            buffer[6] = 0xca;
            buffer[7] = 0x32;
            buffer[8] = 0x7d;
            buffer[9] = 0x58;
            buffer[10] = 0x66;
            buffer[11] = 0xb3;
            buffer[12] = 0x59;
            buffer[13] = 0xc3;
            buffer[14] = 0xf7;
            buffer[15] = 0x9b;
            buffer[16] = 0x1d;
            buffer[17] = 0x02;
            buffer[18] = 0x81;

            //Generate page 1 of 3
            buffer[5] = 0x13;
            buffer[19] = 0x62;
            memcpy((unsigned char*)&msg.msgData.data[0], buffer, 88);
 
            //Generate page 2 of 3
            buffer[5] = 0x23;
            buffer[19] = 0x64;
            memcpy((unsigned char*)&msg.msgData.data[88], buffer, 88);

            //Generate page 3 of 3
            buffer[5] = 0x33;
            buffer[19] = 0x66;
            memcpy((unsigned char*)&msg.msgData.data[176], buffer, 88);
              
            //size of this CB msg
            msg.msgData.size = 264;
            break;            
    }

    //Send a test CB message to RM
    if (!JcMsgRcvBySmscbc(&msg))
        printf("FAILED in calling intg_JcSendCbCmd: cmdCode=%d, msgCode=%d, upNo=%d\n",
                cmdCode, msgCode, updateNo);
    else
        printf("SUCCESS in calling intg_JcSendCbCmd: cmdCode=%d, msgCode=%d, upNo=%d\n",
                cmdCode, msgCode, updateNo);
}
     
//PR 1323, Testing SMSCBC
//message id =0 assumed, msgCode = 0x2aa
void intg_JcSendCbcIndex(unsigned char  cmdCode,
                      unsigned short  msgCode,
                      unsigned short updateNo)
{
	unsigned short   hdr12;
    InterRxCbcCbMsg_t msg;
    unsigned char    buffer[88];

    switch(cmdCode)
    {
    case 0: //Stop sending cur CB message
            msg.destModuleId = MODULE_SMSCBC;
            msg.msgType = INTER_SMSCBC_CB_STOP;
            
            msg.msgData.module_id = MODULE_SMSCBC;
            msg.msgData.message_type = INTER_SMSCBC_CB_STOP;
            msg.msgData.size = 0;
		msg.msgData.message_id = 0;/*Danny, 6/29*/
            break;

    default: //Send test CB message to DSP
            msg.destModuleId = MODULE_SMSCBC;
            msg.msgType = INTER_SMSCBC_CB_START;

            msg.msgData.module_id = MODULE_SMSCBC;
            msg.msgData.message_type = INTER_SMSCBC_CB_START;
            msg.msgData.message_id = 0;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
                                                                                                      

            memset((char *)buffer, 0, 88);
            hdr12=0;
            hdr12 = (00<<14)|(msgCode<<4)|(updateNo);//001010101010YYYY; x is msgCode, Y is Update#
		//Message Identifier: 0 assumed
            buffer[2] = 0;
            buffer[3] = 0;
		//Data coding scheme: default alphabet, English
            buffer[4] = 0x01;
            //Page parameter: page 1 of 1 
            buffer[5] = 0x11;
	 
		 //contents: 20 CISSOM/A CA/A2 SF --- EI1<cr><lf>20<SP>CISSOM<cr><lf>A<sp>CA<cr><lf>A2<SP>SF<cr><lf>
            buffer[6] = 0xc5;
            buffer[7] = 0x64;
            buffer[8] = 0xac;
            buffer[9] = 0xa1;
            buffer[10] = 0x90;
            buffer[11] = 0xc1;
            buffer[12] = 0x40;
            buffer[13] = 0xc3;

            buffer[14] = 0xe4;
            buffer[15] = 0x74;
            buffer[16] = 0xfa;
            buffer[17] = 0x6c;
            buffer[18] = 0x2a;
		buffer[19] = 0x82;
	
            buffer[20] = 0xa0;
		buffer[21] = 0x61;
		buffer[22] = 0xb0;
		buffer[23] = 0xa1;
		buffer[24] = 0x0a;
		buffer[25] = 0xca;
		buffer[26] = 0x40;
		buffer[27] = 0x53;
		buffer[28] = 0x63;
		buffer[29] = 0x43;
		buffer[30] = 0x01;
		buffer[31] = 0x00;
		buffer[32] = 0x00;
            memcpy((unsigned char*)&msg.msgData.data[0], buffer, 88);
 
            //size of this CB msg
    		msg.msgData.size = 88;
            break;            
    }

    //Send a test CB message to RM
    if (!JcMsgRcvBySmscbc(&msg))
        printf("FAILED in calling intg_JcSendCbcIndex: cmdCode=%d, msgCode=%d, upNo=%d\n",
                cmdCode, msgCode, updateNo);
    else
        printf("SUCCESS in calling intg_JcSendCbcIndex: cmdCode=%d, msgCode=%d, upNo=%d\n",
                cmdCode, msgCode, updateNo);
}     

#endif // JCCbcCb_CPP


