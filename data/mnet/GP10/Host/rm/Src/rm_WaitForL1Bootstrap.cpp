/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_WAITFORL1BOOTSTRAP_CPP__
#define __RM_WAITFORL1BOOTSTRAP_CPP__

#include "rm\rm_head.h"


void rm_WaitForL1Bootstrap(void)
{
   /* Monitoring entrance to a function */
   RDEBUG__(( "ENTER-rm_WaitForL1Bootstrap: Entering...\n" ));
   
   /* Wait until all configured TRXs bootstraped */
   rm_BootstrapCnf = 0;
  
   while ( rm_BootstrapCnf != OAMrm_TRX_ONOFF_MAP )
   {
      /* Read until get the expected message from L1 */
      rm_WaitForL1Rsp( RM_L1TRX_NULL, RM_L1MG_TRXMGMT,
		       RM_L1MT_BOOTSTRAPCNF_MSB, RM_L1MT_BOOTSTRAPCNF_LSB );

      /* Received a TRX bootstrap confirmation */
      rm_BootstrapCnf |= RM_BOOTSTRAPCNF(rm_pItcRxL1Msg->l3_data.buffer[3]);

      RDEBUG__(("TRACE:rm_WaitForL1Bootstrap: got bootstrap cnf from trx:%d\n",
       	                               rm_pItcRxL1Msg->l3_data.buffer[3]));
   } /* End of while() */

} /* End of rm_WaitForL1Bootstrap() */

#endif /* __RM_WAITFORL1BOOTSTRAP_CPP__ */
