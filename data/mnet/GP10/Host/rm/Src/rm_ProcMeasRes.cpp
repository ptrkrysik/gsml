/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_PROCMEASRES_CPP__
#define __RM_PROCMEASRES_CPP__

#include "rm\rm_head.h"

u8 rm_ProcMeasRes( void )
{
   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_ProcMeasRes\n"));

   if (rm_pItcRxL1Msg->l3_data.buffer[RM_SACCHBAD_POS]==RM_TRUE)
   {
       if (++rm_pSmCtxt->pPhyChan->rslCount > RM_RSLCOUNT_BOUND)
       {	
	   rm_pSmCtxt->pPhyChan->rslCount = 0;
	   return RM_RSLIND;
       }
   }
   else
   {
       rm_pSmCtxt->pPhyChan->rslCount = 0;
       /*
       ** TBD: process measurement data to decide if HO needed.
       ** If HO needed, send a HO indication message to MM
       */
   }
   
   return RM_LINKED;

} /* Enf of rm_ProcMeasRes() */

#endif /* __RM_PROCMEASRES_CPP__ */
