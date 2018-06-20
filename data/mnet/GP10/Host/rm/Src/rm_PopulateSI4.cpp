/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_POPULATESI4_CPP__
#define __RM_POPULATESI4_CPP__

#include "rm\rm_head.h"

void rm_PopulateSI4(T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   u8 i;

   RDEBUG__(("ENTER-rm_PopulateSI4\n"));

   //ZAP rm_UmSI4 to have a good clean-up
   memset(&rm_UmSI4, 0, sizeof(rm_UmSI4_t)); 

   //HEADER: PD,MT,SI
   rm_UmSI4.header.protocol_descriminator = RM_PD;
   rm_UmSI4.header.si_ti		  = RM_SI;
   rm_UmSI4.header.message_type	          = RM_SI4;

   //IE: Location Area Identification
   rm_UmSI4.locationId.ie_present = RM_TRUE;
   rm_UmSI4.locationId.mcc[0]	    = OAMrm_MCC(0); //0
   rm_UmSI4.locationId.mcc[1]	    = OAMrm_MCC(1); //0
   rm_UmSI4.locationId.mcc[2]	    = OAMrm_MCC(2); //1
   rm_UmSI4.locationId.mnc[0]	    = OAMrm_MNC(0); //0
   rm_UmSI4.locationId.mnc[1]	    = OAMrm_MNC(1); //1
   rm_UmSI4.locationId.mnc[2]	    = OAMrm_MNC(2); //1

   //printf("==========3-digit MNC[%x%x%x]\n", 
   //        rm_UmSI4.locationId.mnc[0],
   //        rm_UmSI4.locationId.mnc[1],
   //        rm_UmSI4.locationId.mnc[2]
   //	   );

   rm_UmSI4.locationId.lac 	    = OAMrm_LAC;    //1

   //IE: Cell Reselection Parameters
   rm_UmSI4.cellSelectionParam.ie_present     = RM_TRUE;
   rm_UmSI4.cellSelectionParam.cellReselectHysteresis = OAMrm_CELL_RESEL_HYSTERESIS; //2
   rm_UmSI4.cellSelectionParam.msTxpwrMaxCCH  =	OAMrm_MS_TX_PWR_MAX_CCH;             //0
   rm_UmSI4.cellSelectionParam.rxlevAccessMin = OAMrm_MS_RELEV_ACCESS_MIN;           //6
   rm_UmSI4.cellSelectionParam.neci 	    = CNI_RIL3_NECI_NOT_SUPPORTED;         //0
   rm_UmSI4.cellSelectionParam.acs	          = CNI_RIL3_ACS_USE_SI4_RESET_OCTETS;   //0

   //IE: Rach Control
   rm_UmSI4.rachControl.ie_present	     = RM_TRUE;
   rm_UmSI4.rachControl.maxRetrans       = OAMrm_RA_RETRY_MAX;  	       //3: 7 times
   rm_UmSI4.rachControl.txInt		     = OAMrm_RA_TX_INTEGER;          //0: wait 3 slots
   rm_UmSI4.rachControl.cellBarredAccess = barState;                     //OAMrm_RA_CELL_BARRED_STATE; 
   rm_UmSI4.rachControl.reestablish	     = OAMrm_RA_REST_ALLOWED_STATE;  //1: not allowed
   rm_UmSI4.rachControl.emergencyCall    = OAMrm_RA_EC_ALLOWED_STATE;    //1: not allowed
   for (i=0;i<16;i++)   
        rm_UmSI4.rachControl.accessClass[i]=OAMrm_RA_AC_ALLOWED_STATE(i);//1:not allowed

   //IE(optional): CBCH channel description
   if (OAMrm_CBCH_OPSTATE)
   {
       rm_UmSI4.cbchChannelDescription.ie_present       = RM_TRUE; 
       rm_UmSI4.cbchChannelDescription.hopping          = RM_FALSE; 
       rm_UmSI4.cbchChannelDescription.channelType	  = rm_pCbchChan->chanType;
       rm_UmSI4.cbchChannelDescription.subChannelNumber = rm_pCbchChan->subChanNumber;
       rm_UmSI4.cbchChannelDescription.TN               = (rm_pCbchChan->chanNumberMSB)&RM_U8_0X07;
       rm_UmSI4.cbchChannelDescription.TSC              = OAMrm_BTS_TSC; //pPhyChan->TSC;
       rm_UmSI4.cbchChannelDescription.arfcn            = OAMrm_ARFCN(rm_pCbchChan->trxNumber);
   }

   //IE: SI4 Rest Octets  
   rm_UmSI4.si4RestOctets.ie_present  = RM_TRUE;
   rm_UmSI4.si4RestOctets.pi		  = CNI_RIL3_SELECTION_PARAMETER_PRESENT; //1
   rm_UmSI4.si4RestOctets.cbq				  = OAMrm_CELL_BAR_QUALIFY;   //default 0
   rm_UmSI4.si4RestOctets.cellReselectOffset	  = OAMrm_CELL_RESELECT_OFFSET;
   rm_UmSI4.si4RestOctets.temporaryOffset		  = OAMrm_TEMP_OFFEST;
   rm_UmSI4.si4RestOctets.penaltyTime		  = OAMrm_PENALTY_TIME;  //20s
   rm_UmSI4.si4RestOctets.poi				  = CNI_RIL3_POWER_OFFSET_NOT_PRESENTE;
   rm_UmSI4.si4RestOctets.powerOffset		  = 0;  //Not applicable


} /* rm_PopulateSI4() */

#endif /* __RM_POPULATESI4_CPP__  */
