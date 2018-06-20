/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_POPULATESI6_CPP__
#define __RM_POPULATESI6_CPP__

#include "rm\rm_head.h"

void rm_PopulateSI6(void)
{
   u8 i;

   RDEBUG__(("ENTER-rm_PopulateSI6\n"));

   //ZAP rm_UmSI6 to have a good clean-up
   memset(&rm_UmSI6, 0, sizeof(rm_UmSI6_t)); 

   //HEADER: PD,MT,SI
   rm_UmSI6.header.protocol_descriminator = RM_PD;
   rm_UmSI6.header.si_ti		      = RM_SI;
   rm_UmSI6.header.message_type	      = RM_SI6;

   //IE: Cell ID
   rm_UmSI6.cellId.ie_present	    = RM_TRUE;
   rm_UmSI6.cellId.value	    = OAMrm_CELLID; //0x0001

   //IE: Location Area Identification
   rm_UmSI6.locationId.ie_present = RM_TRUE;
   rm_UmSI6.locationId.mcc[0]	    = OAMrm_MCC(0); //0
   rm_UmSI6.locationId.mcc[1]	    = OAMrm_MCC(1); //0
   rm_UmSI6.locationId.mcc[2]	    = OAMrm_MCC(2); //1
   rm_UmSI6.locationId.mnc[0]	    = OAMrm_MNC(0); //0
   rm_UmSI6.locationId.mnc[1]	    = OAMrm_MNC(1); //1
   rm_UmSI6.locationId.mnc[2]	    = OAMrm_MNC(2); //1
   rm_UmSI6.locationId.lac 	    = OAMrm_LAC;    //1

   //IE: Cell Options on BCCH
   rm_UmSI6.cellOptions.ie_present	     =  RM_TRUE;
   rm_UmSI6.cellOptions.channelType	     =  CNI_RIL3_CELL_OPTIONS_SACCH;      //1
   rm_UmSI6.cellOptions.pwrc		     =  CNI_RIL3_PWRC_NOT_SET;            //0:no hopping
   rm_UmSI6.cellOptions.radioLinkTimeout =  OAMrm_RADIO_LINK_TIMEOUT;         //3
//   rm_UmSI6.cellOptions.sacchDtx	     =  CNI_RIL3_DTX_SHALL_NOT_TCHF_TCHH; //2
   rm_UmSI6.cellOptions.sacchDtx	     =  OAMrm_MS_UPLINK_DTX_STATE;


   //IE: Ncc Permitted
   rm_UmSI6.nccPermitted.ie_present	     = RM_TRUE;
   for (i=0;i<OAMrm_NCC_PERMITTED_MAX;i++)
        rm_UmSI6.nccPermitted.nccPermitted[i] = OAMrm_NCC_PERMITTED(i);

} /* rm_PopulateSI6() */

#endif /* __RM_POPULATESI6_CPP__  */
