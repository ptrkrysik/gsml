/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITL1_CPP__
#define __RM_INITL1_CPP__

//GP2
#if 0

#include "rm\rm_head.h"

void rm_InitL1(void)
{
   u8		   isBcchTrx;	    /* The TRX carrying BCCH chan. */
   u8		   i;		    /* Loop-control and trx number */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_InitL1\n" ));

   /*=TBD=*/
   /* 
   ** The whole L1 configuration is done in four steps:
   ** =================================================
   ** Step 1:Configures each individual TRX as per the command sequence
   ** Step 2:Passes SYNCHRONIZATION INFORMATION message to the BCCH TRX
   ** Step 3:Passes all required SYSTEM INFORMATION messages toTRX/BCCH
   ** Step 4:Passes SACCH FILLING message to each individual TRX in use

   ** 
   ** Configuration of an individual TRX is achieved by sending L1 the
   ** following DSP-command sequence:
   ** ===============================
   ** (1)TRX CONFIGURE     (2)SLOT ACTIVATE     (3)SET UPLINK GAIN
   ** (4)SET DOWNLINK GAIN (5)TUNE SYNTHESIZER  (6)CONFIGURE FPGA DELAY
   **
   */

   /* Step 1: One by one configure all usable TRXs */
   i = 0; oam_BcchTrx = RM_L1TRX_NULL;
   while ( i < OAMrm_MAX_TRXS )
   { 
      /* if i-th TRX present, configure it */
      if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
      {  

           isBcchTrx = rm_ConfigTrxSlots(i);	    	    
	
           /* ret=TRUE means i is BcchTrx */
           if (isBcchTrx) oam_BcchTrx = i;

PDEBUG__(("TRACE-rm_InitL1: i=%d, noTrx=%d, trxMap=%d, curTrx=%d, bcchTrx=%d, i=%d\n",
           i,OAMrm_MAX_TRXS, OAMrm_TRX_ONOFF_MAP, ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) ), oam_BcchTrx,i));

      }
      i++;
   } /* End of while() */

// Comment for TEST
// PDEBUG__(("End of rm_ConfigTrxSlots, bcchTrx=%d\n", bcchTrx));

//   /* Step 2: Pass SYNCHRONIZATION INFORMATION to the BCCH TRX */
//   rm_SendSynchInfoToBcchTrx(bcchTrx);
//   rm_WaitForL1Rsp(bcchTrx, RM_L1MG_TRXMGMT, RM_L1MT_SYNCHACK_MSB,
//                   RM_L1MT_SYNCHACK_LSB);

//PDEBUG__(("End of rm_SendSynchInfoToBcchTrx()\n"));
 
     /* Step 3: Pass all SYSTEM INFOs to the BCCH TRX */
   if ( oam_BcchTrx != RM_L1TRX_NULL )
	rm_SendAllSystemInfosToBcchTrx(oam_BcchTrx);
   else
	EDEBUG__(("ERROR-rm_InitL1: No TRX appointed carrying BCCH\n"));

PDEBUG__(("End of rm_SendAllSystemInfosToBcchTrx, oam_BcchTrx=%d\n",oam_BcchTrx));

   /* Step 4: Pass SACCH filling message to all TRX */
   i = 0;
   while ( i < OAMrm_MAX_TRXS )
   { 
      /* Pass SACCH FILLing to present TRX */
      if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
      {
           rm_SendSacchFillingSI5(i, RM_L1SYSINFO_TYPE_5);	    	    
           rm_SendSacchFillingSI6(i, RM_L1SYSINFO_TYPE_6);	    	    
      }
      i++;

   } /* End of while() */

PDEBUG__(("End of rm_SendSacchFillingSI5 and SI6\n"));

/* Step 2: Pass SYNCHRONIZATION INFORMATION to the BCCH TRX */
//rm_SendSynchInfoToBcchTrx(bcchTrx);   //TBD: Send one for each TRX later
//rm_WaitForL1Rsp(bcchTrx, RM_L1MG_TRXMGMT, RM_L1MT_SYNCHACK_MSB,
//                RM_L1MT_SYNCHACK_LSB);

   i = 0;
   while ( i < OAMrm_MAX_TRXS )
   { 
      /* Let DSP go by sending Sync Info */
      if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
      {
           rm_SendSynchInfoToBcchTrx(i);
      }
      i++;

   } /* End of while() */

   PDEBUG__(("End of rm_SendSynchInfoToBcchTrx()\n"));

} /* End of rm_InitL1() */

//GP2
#endif //#if 0

#endif /* __RM_INITL1_CPP__ */
