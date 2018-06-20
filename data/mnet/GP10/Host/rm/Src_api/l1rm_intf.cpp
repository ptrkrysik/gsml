/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
  */

#ifndef __L1RM_INTF_CPP__
#define __L1RM_INTF_CPP__

#include "rm\rml1_intf.h"

void sendDsp ( unsigned char *buffer, int len );

void api_SendMsgFromL1ToRm(unsigned short length, unsigned char* msg)
{
     STATUS       result;
     rml1_Msg_t   l1ToRmMsg;

     //Safeguard only <TBD>
     if (msg[3]>=2 || length<4)
     {   //Received invalid L1 message, ignore it !
         EDEBUG__(("ERROR@api_SendMsgFromL1ToRm: L1 msg err (%d,%d)\n",
                    msg[3],length));
         return;
     }

     l1ToRmMsg.module_id         = MODULE_L1;
     l1ToRmMsg.primitive_type    = MPH_INFO_IND;
     l1ToRmMsg.l3_data.msgLength = length;
     memcpy(l1ToRmMsg.l3_data.buffer, msg, length);

//     TDUMP__(( RXMSG__, (unsigned char *)&l1ToRmMsg,
//	       (sizeof(rml1_Msg_t)-L1RM_MAXMSG_LEN+length) ));		
	   
     result = msgQSend( 
			 rm_MsgQId, 
			 (char *) &l1ToRmMsg, 
			 (sizeof(rml1_Msg_t)-L1RM_MAXMSG_LEN + length),
			 NO_WAIT,
			 MSG_PRI_NORMAL 
		      );
     if ( result==ERROR )
	  printf("ERROR-api_SendMsgFromL1ToRm: VxWorks sending error:%d\n",
		   errno);
     
}

void api_SendMsgFromRmToL1 (
		unsigned short 	length,
		unsigned char	*msg
		   )

{
     STATUS	result; 

     typedef struct {
	    unsigned char module_id;
	    unsigned char primitive_type;
	    struct {
	        int 	msgLength;
                unsigned char buffer[L1RM_MAXMSG_LEN];
            } l3_data;
     } rmToL1Msg_t;

     rmToL1Msg_t rmToL1Msg; 

//   rmToL1Msg.module_id         = MODULE_RM;
//   rmToL1Msg.primitive_type    = MPH_INFO_REQ;

//   Byte-by-byte assignment to guarantee little-endian format
//   rmToL1Msg.l3_data.msgLength = (int) length;
     unsigned char *pDes; 
     int len;
     pDes = (unsigned char*) &(rmToL1Msg.l3_data.msgLength);
     len  = (int) length;
     *pDes++ = (unsigned char) len;
     *pDes++ = (unsigned char) (len>>8);
     *pDes++ = (unsigned char) (len>>16);
     *pDes   = (unsigned char) (len>>24);

     memcpy(rmToL1Msg.l3_data.buffer, msg, length);

//     TDUMP__(( TXMSG__, (unsigned char *) &rmToL1Msg,
//	           (sizeof(rmToL1Msg_t)-L1RM_MAXMSG_LEN+length) ));		
//
//     result = msgQSend( 
//			 l1_MsgQId, 
//			 (char *) &rmToL1Msg, 
//			 (sizeof(rmToL1Msg_t)-L1RM_MAXMSG_LEN+length),
//			 NO_WAIT,
//			 MSG_PRI_NORMAL 
//		      );
//
//     result = msgQSend( 
//			 l1_MsgQId, 
//			 (char *) &(rmToL1Msg.l3_data), 
//			 (length + 4),
//			 NO_WAIT,
//			 MSG_PRI_NORMAL 
//		      );

//     TDUMP__(( TXMSG__, (unsigned char *) &(rmToL1Msg.l3_data),(length+4)));

     sendDsp((unsigned char*)&(rmToL1Msg.l3_data), (length+4));

//   if ( result==ERROR )
//	  EDEBUG__(("ERROR-api_SendMsgFromRmToL1: VxWorks sending error:%d\n",
//		   errno));
    
}

#endif /* __L1RM_INTF_CPP__ */
