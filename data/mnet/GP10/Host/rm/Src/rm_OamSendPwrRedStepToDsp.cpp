/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMSENDPWRREDSTEPTODSP_CPP__
#define __RM_OAMSENDPWRREDSTEPTODSP_CPP__

//GP2
#if 0

#include <stdio.h> 
#include "rm\rm_head.h"

#define	OAM_TXPWR_MAX_RDCT_CONFIG  0x5006
#define OAM_TXPWR_MAX_RDCT_CONFIG_ACK 0x5007

void rm_OamSendPwrRedStepToDsp(u8 trxNo)
{
    s16        length;            /* Length of the message to send to L1 */
    rm_ItcTxMsg_t   msgToL1;      /* Store an encoded message sent to L1 */
	static char fname[] = "rm_OamSendPwrRedStepToDsp";

    
    /* Monitoring entrance to a function */
    RDEBUG__(("ENTER-%s Entering...\n", fname ));
    
	
	/* clear the memory first */
	memset(&msgToL1, 0, sizeof(rm_ItcTxMsg_t));
	length = 0;
	
    /* Encode TRX CONFIGure message */
    msgToL1.buffer[length++] = (unsigned char ) OAM_MANAGE_MSG;           // message discriminator 
	msgToL1.buffer[length++] = (unsigned char ) (OAM_TXPWR_MAX_RDCT_CONFIG >>8);  // high byte
    msgToL1.buffer[length++] = (unsigned char ) OAM_TXPWR_MAX_RDCT_CONFIG;      // message type Low byte
    msgToL1.buffer[length++] = trxNo;          


	
	/* Transmit Power Maximum Reduction setps
     */
	if (trxNo == 0 || trxNo == 1)
	{
		msgToL1.buffer[length++] = (unsigned char)  rm_OamData.radioCarrierPackage[OAMrm_TRX_RC(trxNo)].txPwrMaxReduction;
	} else {
		EDEBUG__(("%s:Invalid TRX number passed (%d)\n", fname, trxNo));
		return;
	}
      
	

    msgToL1.msgLength = length;

	/* Hex Dump the message if __TDUMP__ defined */
   // TDUMP__((TXMSG__, msgToL1.buffer, length));

    /* Finally send message to DSP */
    api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
    
} /* rm_OamSendHoSCellToDsp() */

//GP2
#endif //#if 0

#endif /* __RM_OAMSENDPWRREDSTEPTODSP_CPP__*/

