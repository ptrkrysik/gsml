/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMSENDHONCELLTODSP_CPP__
#define __RM_OAMSENDHONCELLTODSP_CPP__

//GP2
#if 0

#include <stdio.h>
#include "rm\rm_head.h"

u8 rm_OamGetTotalAvailableHoNCells(void)
{
   u8 i,count;
   count=0;
   //PR1223 Begin: add suffix "t" only
   for (i=0;i<OAMrm_HO_ADJCELL_NO_MAX;i++)
       if (OAMrm_HO_ADJCELL_ID_t(i)!=OAMrm_HO_ADJCELL_ID_NULL)
           count++;
   return count;
}
void rm_OamSendHoNCellToDsp(u8 trxNo, u8  nCellIdx, u8 totalNCells)
{
    s16     length;               /* Length of the message to send to L1 */
    rm_ItcTxMsg_t   msgToL1;      /* Store an encoded message sent to L1 */
    s16 tmp;

    if (OAMrm_HO_ADJCELL_ID_t(nCellIdx)==OAMrm_HO_ADJCELL_ID_NULL) return;

    AdjCell_HandoverEntry *entry 
	= (AdjCell_HandoverEntry *) rm_OamData.adjacentCellPackage_t.adjCell_handoverTable;
    
    // point to the correct index;
    entry  += nCellIdx;
   
    /* Monitoring entrance to a function */
    RDEBUG__(("ENTER-rm_OamSendHoNCellToDsp: Entering...\n" ));
    
	
	/* clear the memory first */
	memset(&msgToL1, 0, sizeof(rm_ItcTxMsg_t));
	length = 0;
	
    /* Encode TRX CONFIGure message */
    msgToL1.buffer[length++] = (unsigned char ) OAM_MANAGE_MSG;           // message discriminator 
    msgToL1.buffer[length++] = (unsigned char ) (OAM_HO_NCELL_CONFIG >>8);  // high byte
	msgToL1.buffer[length++] = (unsigned char ) OAM_HO_NCELL_CONFIG;      // message type Low byte 0x5000,
    msgToL1.buffer[length++] = trxNo;          
	msgToL1.buffer[length++] = nCellIdx;
	msgToL1.buffer[length++] = totalNCells;
	
	/* ARFCN */
	tmp = entry->adjCell_bCCHFrequency;

    msgToL1.buffer[length++] = (unsigned char) (tmp >>8); 
	msgToL1.buffer[length++] = (unsigned char) tmp; 
	
	
	
    /* Base station identity code  is generated by three bit of ncc and 3 bit
	 * bit of bcc
	 */

	msgToL1.buffer[length++] = (unsigned char) ((unsigned char) entry->adjCell_ncc << 3 |
		(unsigned char) entry->adjCell_cid); 

     /* Handover margin */
	
	msgToL1.buffer[length++] = (unsigned char) entry->adjCell_hoMargin;

	/* Maximum Mobile Transmit Power for Neighbour Cell */
	tmp =  entry->adjCell_msTxPwrMaxCell;
   // msgToL1.buffer[length++] = (unsigned char) (tmp >>8);
   msgToL1.buffer[length++] = (unsigned char) tmp; 
 
	

	/* Minimum Mobile receive Power for Neighbour Cell */
	tmp = entry->adjCell_rxLevMinCell;
    // msgToL1.buffer[length++] = (unsigned char) (tmp >>8);
	msgToL1.buffer[length++] = (unsigned char) tmp; 

	
    msgToL1.msgLength = length;

	/* Hex Dump the message if __TDUMP__ defined */
	TDUMP__((TXMSG__, msgToL1.buffer, length));

    /* Finally send message to DSP */
    api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
    
} /* rm_OamSendHoNCellToDsp() */


/* send Hand over Neighbouring Cell Configuration parameter for all the
 *  available cells of all TRX
 */

void rm_OamSendHoAllNCellToDsp()
{
	int i, j;
        u8 ncellNo;
        ncellNo=rm_OamGetTotalAvailableHoNCells();
	for (i = 0; i < TOTAL_AVAILABLE_TRX; i++)
		for (j=0; j <TOTAL_AVAILABLE_CELL; j++)
			rm_OamSendHoNCellToDsp(i,j,ncellNo);
}

//GP2
#endif //#if 0

#endif /* __RM_OAMSENDHONCELLTODSP_CPP__*/

