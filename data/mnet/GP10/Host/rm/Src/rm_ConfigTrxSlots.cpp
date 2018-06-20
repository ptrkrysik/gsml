/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_CONFIGTRXSLOTS_CPP__
#define __RM_CONFIGTRXSLOTS_CPP__

#include "rm\rm_head.h"

//void sys_init(void);  /* TEST: rml1 intg */

u8 rm_ConfigTrxSlots( u8 trx )
{
   u8	i;		/* loop-control variable 		*/
   u8	isBcchTrx;	/* tell if trx is carrying BCCH channel */
  
   isBcchTrx = RM_FALSE; 

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_ConfigTrx\n" ));

   /*TEST: RM-L1 Integration */
   //sys_Init(); 

   
   /* Wait L1 bootstrap confirmation */
   //rm_WaitForL1Bootstrap();

   /* It's caller's responsibility to make sure trx is valid */
   
   /* First: Send TRX CONFIGure and wait for response */

   /*
   ** TBD: Winfield May allow several retries of TRX Configure for
   ** real operation later 
   */
   
   rm_SendTrxConfigToTrx(trx);
   rm_WaitForL1Rsp(trx,RM_L1MG_TRXMGMT, RM_L1MT_TRXCONFIGACK_MSB,
					RM_L1MT_TRXCONFIGACK_LSB );
printf("WATCH:rm_ConfigTrxSlot--got rsp to TrxConfig\n");

   /* Second: Send SLOT ACTIVate and wait for response */
   i=0; 
   while ( i < OAMrm_SLOTS_PER_TRX )
   {
	/* TBD: send nothing to a slot if unused */
        if ( OAMrm_TRX_SLOT_COMB(trx,i) != OAMrm_SLOT_UNUSED )
        {
            /* Send SlotActvate and wait for rsp */
            rm_SendSlotActivateToTrx(trx,i);
            rm_WaitForSlotActivateRsp(trx,i);
        
            /* Check if the slot is carrying BCCH */
            if ( OAMrm_TRX_SLOT_COMB(trx,i) == OAMrm_SLOT_BEACON )
	         isBcchTrx = RM_TRUE;
        }
	i++;
   }

PDEBUG__(("SLOT ACTIVATION FINISHED\n"));
   /*
   ** Third: Send the following commands but order TBD with DSP guys:
   ** (1) SET Uplink GAIN (2) SET Downlink GAIN (3) TUNE SYNTHesizer
   ** (4) CONFIGure FPGA DELAY.No response is seeked for the four L1
   ** commands.
   */
   //rm_SendSetUGainToTrx(trx);
   //rm_SendSetDGainToTrx(trx);
   //rm_SendTuneSynthToTrx(trx);
   //rm_SendConfigFpgaDelayToTrx(trx);

PDEBUG__(("CONFIG TRX/SLOTS FINISHED\n"));

   return (isBcchTrx);

} /* End of rm_ConfigTrxSlots() */

#endif /* __RM_CONFIGTRXSLOTS_CPP__ */
