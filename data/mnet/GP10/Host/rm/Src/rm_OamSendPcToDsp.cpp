/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMSENDPCTODSP_CPP__
#define __RM_OAMSENDPCTODSP_CPP__

//GP2
#if 0

#include <stdio.h>
#include "rm\rm_head.h"

void rm_OamSendPcToDsp(s8 trxNo)
{
    s16        length;            /* Length of the message to send to L1 */
    rm_ItcTxMsg_t   msgToL1;      /* Store an encoded message sent to L1 */
	s16 tmp;
	PowerControlPackage_t *pcPktPtr = &rm_OamData.powerControlPackage;
    
    /* Monitoring entrance to a function */
    RDEBUG__(("ENTER-rm_OamSendPcToDsp: Entering...\n" ));
    
	
	/* clear the memory first */
	memset(&msgToL1, 0, sizeof(rm_ItcTxMsg_t));
	length = 0;
	
    /* Encode TRX CONFIGure message */
    msgToL1.buffer[length++] = (unsigned char ) OAM_MANAGE_MSG;      // message discriminator 
 	msgToL1.buffer[length++] = (unsigned char ) (OAM_DPC_CONFIG >>8);  // high byte
    msgToL1.buffer[length++] = (unsigned char ) OAM_DPC_CONFIG;       // message type Low byte 0x5000,
    msgToL1.buffer[length++] = trxNo;   //0x00

    
	/*
    ** the size for the signal sample buffer used for averaging power
	** level[1-32]. one sample at every 480 ms (1 frame)
	*/

	/* Averaging Buffer Size For  Signal Strength (Hreqave) */
	msgToL1.buffer[length++] = pcPktPtr->pcAveragingLev->hreqave;
	
    /* 
    ** the size for the signal sample buffer used for averaging 
	** quality samples [1-32]. one sample at every 480 ms (1 frame)
	*/

	/* Averaging Buffer Size For Signal Quality (Hreqave) */
	msgToL1.buffer[length++] = pcPktPtr->pcAveragingQual->hreqave;
	

	/* 
    ** Threshold for activating uplink dynamic power control
    ** Note: the following variable is reused
    */                                             
    /*
    *   Need to Verify Which Value to USE, Using current One pass compiling
    */
	msgToL1.buffer[length++] =  pcPktPtr->pcLowerThresholdLevParam->rxLevelUL;

    msgToL1.msgLength = length;

	/* Hex Dump the message if __TDUMP__ defined */
	TDUMP__((TXMSG__, msgToL1.buffer, length));

    /* Finally send message to DSP */
    api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
    
} /* rm_OamSendPcToDsp() */

//GP2
#endif //#if 0

#endif /* __RM_OAMSENDPCTODSP_CPP__*/

