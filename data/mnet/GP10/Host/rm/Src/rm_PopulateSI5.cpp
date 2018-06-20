/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_POPULATESI5_CPP__
#define __RM_POPULATESI5_CPP__

#include "rm\rm_head.h"

void rm_PopulateSI5(void)
{
   u8 idx,i;

   RDEBUG__(("ENTER-rm_PopulateSI5\n"));

   //ZAP rm_UmSI5 to have a good clean-up
   memset(&rm_UmSI5, 0, sizeof(rm_UmSI5_t)); 

   //HEADER: PD,MT,SI */
   rm_UmSI5.header.protocol_descriminator = RM_PD;
   rm_UmSI5.header.si_ti		  = RM_SI;
   rm_UmSI5.header.message_type	          = RM_SI5;

   //IE: Bcch Frequency List
   rm_UmSI5.bcchFreqList.ie_present     = RM_TRUE;
   rm_UmSI5.bcchFreqList.extInd		= RM_EXTIND_0;
   rm_UmSI5.bcchFreqList.baInd		= RM_BA_WHOLE;
   //rm_UmSI5.bcchFreqList.format		= RM_FRMT_VARBITMAP;

   idx=0;
   for (i=0;i<OAMrm_HO_ADJCELL_NO_MAX;i++)
   {
        if (OAMrm_HO_ADJCELL_ID(i) != OAMrm_HO_ADJCELL_ID_NULL)
            rm_UmSI5.bcchFreqList.arfcn[idx++] = OAMrm_HO_ADJCELL_BCCH_ARFCN(i);
   } 
   rm_UmSI5.bcchFreqList.numRFfreq	     = idx;
   rm_UmSI5.bcchFreqList.format = CNI_RIL3_SelectFrequencyListFormat(
  	                            rm_UmSI5.bcchFreqList.arfcn,idx);

} /* rm_PopulateSI5() */

#endif /* __RM_POPULATESI5_CPP__  */
