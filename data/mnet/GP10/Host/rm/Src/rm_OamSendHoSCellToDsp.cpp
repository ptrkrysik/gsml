/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMSENDHOSCELLTODSP_CPP__
#define __RM_OAMSENDHOSCELLTODSP_CPP__

//GP2
#if 0

#include <stdio.h> 
#include "rm\rm_head.h"

void rm_OamSendHoSCellToDsp(s8 trxNo)
{
    s16        length;            /* Length of the message to send to L1 */
    rm_ItcTxMsg_t   msgToL1;      /* Store an encoded message sent to L1 */
	s16 tmp;
	HandoverControlPackage_t *hoPktPtr = &rm_OamData.handoverControlPackage;
    
    /* Monitoring entrance to a function */
    RDEBUG__(("ENTER-rm_OamSendHoSCellToDsp.cpp: Entering...\n" ));
    
	
	/* clear the memory first */
	memset(&msgToL1, 0, sizeof(rm_ItcTxMsg_t));
	length = 0;
	
    /* Encode TRX CONFIGure message */
    msgToL1.buffer[length++] = (unsigned char ) OAM_MANAGE_MSG;           // message discriminator 
	msgToL1.buffer[length++] = (unsigned char ) (OAM_HO_SCELL_CONFIG >>8);  // high byte
    msgToL1.buffer[length++] = (unsigned char ) OAM_HO_SCELL_CONFIG;      // message type Low byte
    msgToL1.buffer[length++] = trxNo;          
	
	
	/* RXLEV threshold on the uplink for handover process to commence. Typical
     * range -103 to -73 dBm.
     */

	tmp = hoPktPtr->hoThresholdLevParam->rxLevelUL;
    // msgToL1.buffer[length++] = (unsigned char) (tmp >>8); 
	msgToL1.buffer[length++] = (unsigned char) tmp; 
	
	
	
    /* RXLEV threshold on the downlink for handover process to commence.
     * Typical range -103 to -73 dBm.
	 */

	tmp = hoPktPtr->hoThresholdLevParam->rxLevelDL;
    // msgToL1.buffer[length++] = (unsigned char) (tmp >>8);    //0x5D
	msgToL1.buffer[length++] = (unsigned char) tmp; 
	
    /* The number of averages (out of total averages) that have to be upper/lower than
     * the threshold, before making a handover decision (P5).
	 */	
	
	msgToL1.buffer[length++] = hoPktPtr->hoThresholdLevParam->px;


	/* The number of averages that have to be taken into account, when making a
     * handover decision (N5).
	 */

	msgToL1.buffer[length++] = hoPktPtr->hoThresholdLevParam->nx;

	/****************** Quality Parameters **************************/

	/* RXLEV threshold on the uplink for handover process to commence. Typical
     * range -103 to -73 dBm.
     */

	tmp = hoPktPtr->hoThresholdQualParam->rxQualUL;
    // msgToL1.buffer[length++] = (unsigned char) (tmp >>8); 
	msgToL1.buffer[length++] = (unsigned char) tmp; 
	
	
	
    /* RXLEV threshold on the downlink for handover process to commence.
     * Typical range -103 to -73 dBm.
	 */

	tmp = hoPktPtr->hoThresholdQualParam->rxQualDL;
    // msgToL1.buffer[length++] = (unsigned char) (tmp >>8);    //0x5D
	msgToL1.buffer[length++] = (unsigned char) tmp; 
	
    /* The number of averages (out of total averages) that have to be upper/lower than
     * the threshold, before making a handover decision (P5).
	 */	
	
	msgToL1.buffer[length++] = (unsigned char) hoPktPtr->hoThresholdQualParam->px;


	/* The number of averages that have to be taken into account, when making a
     * handover decision (N5).
	 */

	msgToL1.buffer[length++] = (unsigned char) hoPktPtr->hoThresholdQualParam->nx;


	/* Handover Margin Default
	 */

	msgToL1.buffer[length++] = (unsigned char) hoPktPtr->hoMarginDef;

	/* The following three parameters are added on Aug 6, 1999 -- Bhawani */

	/* Maximum Mobil Transmit Power (Default)
	 */

	msgToL1.buffer[length++] = (unsigned char) hoPktPtr->mxTxPwrMaxCellDef;


	/*  Minimum Mobile Receive Power (Default)
	 */

	msgToL1.buffer[length++] = (unsigned char) hoPktPtr->rxLevMinCellDef;
	
    msgToL1.msgLength = length;

	/* Hex Dump the message if __TDUMP__ defined */
//	TDUMP__((TXMSG__, msgToL1.buffer, length));

    /* Finally send message to DSP */
    api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
    
} /* rm_OamSendHoSCellToDsp() */

//GP2
#endif //#if 0

#endif /* __RM_OAMSENDHOSCELLTODSP_CPP__*/

