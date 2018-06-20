/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_POPULATESI2_CPP__
#define __RM_POPULATESI2_CPP__

#include "rm\rm_head.h"

void rm_PopulateSI2(T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   u8	i,idx;

   RDEBUG__(("ENTER-rm_PopulateSI2\n"));

   //ZAP rm_UmSI2 to have a good clean-up
   memset(&rm_UmSI2, 0, sizeof(rm_UmSI2_t)); 

   //HEADER: PD,MT,SI
   rm_UmSI2.header.protocol_descriminator = RM_PD;
   rm_UmSI2.header.si_ti		      = RM_SI;
   rm_UmSI2.header.message_type	      = RM_SI2;

   //IE: Bcch Frequency List
   rm_UmSI2.bcchFreqList.ie_present       = RM_TRUE;
   rm_UmSI2.bcchFreqList.extInd		= RM_EXTIND_0;
   rm_UmSI2.bcchFreqList.baInd		= RM_BA_WHOLE;
   //rm_UmSI2.bcchFreqList.format		= RM_FRMT_VARBITMAP;
   idx=0;
   for (i=0;i<OAMrm_RESEL_ADJCELL_NO_MAX;i++)
   {
        if (OAMrm_RESEL_ADJCELL_ID(i) != OAMrm_RESEL_ADJCELL_ID_NULL)
            rm_UmSI2.bcchFreqList.arfcn[idx++] = OAMrm_RESEL_ADJCELL_BCCH_ARFCN(i);
   } 
   rm_UmSI2.bcchFreqList.numRFfreq	     = idx;
   rm_UmSI2.bcchFreqList.format = CNI_RIL3_SelectFrequencyListFormat(
  	                            rm_UmSI2.bcchFreqList.arfcn,idx);
  
   //IE: Ncc Permitted
   rm_UmSI2.nccPermitted.ie_present	     = RM_TRUE;
   for (i=0;i<OAMrm_NCC_PERMITTED_MAX;i++)
        rm_UmSI2.nccPermitted.nccPermitted[i] = OAMrm_NCC_PERMITTED(i);
   
   //IE: Rach Control
   rm_UmSI2.rachControl.ie_present	     = RM_TRUE;
   rm_UmSI2.rachControl.maxRetrans       = OAMrm_RA_RETRY_MAX;  	      //3: 7 times
   rm_UmSI2.rachControl.txInt		     = OAMrm_RA_TX_INTEGER;         //0: wait 3 slots
   rm_UmSI2.rachControl.cellBarredAccess = barState;                    //OAMrm_RA_CELL_BARRED_STATE;
   rm_UmSI2.rachControl.reestablish	     = OAMrm_RA_REST_ALLOWED_STATE; //1: not allowed
   rm_UmSI2.rachControl.emergencyCall    = OAMrm_RA_EC_ALLOWED_STATE;   //1: not allowed
   for (i=0;i<16;i++)   
        rm_UmSI2.rachControl.accessClass[i]=OAMrm_RA_AC_ALLOWED_STATE(i); //1:not allowed

} //rm_PopulateSI2()

#endif /__RM_POPULATESI2_CPP__
