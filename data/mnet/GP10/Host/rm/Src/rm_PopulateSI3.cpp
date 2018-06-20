/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_POPULATESI3_CPP__
#define __RM_POPULATESI3_CPP__

#include "rm\rm_head.h"

void rm_PopulateSI3(T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   u8	i;

   RDEBUG__(("ENTER-rm_PopulateSI3\n"));

   //ZAP rm_UmSI3 to have a good clean-up
   memset(&rm_UmSI3, 0, sizeof(rm_UmSI3_t)); 

   //HEADER: PD,MT,SI
   rm_UmSI3.header.protocol_descriminator = RM_PD;
   rm_UmSI3.header.si_ti		      = RM_SI;
   rm_UmSI3.header.message_type	      = RM_SI3;

   //IE: Cell ID
   rm_UmSI3.cellId.ie_present	    = RM_TRUE;
   rm_UmSI3.cellId.value	    = OAMrm_CELLID; //0x0001

   //IE: Location Area Identification
   rm_UmSI3.locationId.ie_present = RM_TRUE;
   rm_UmSI3.locationId.mcc[0]	    = OAMrm_MCC(0); //0
   rm_UmSI3.locationId.mcc[1]	    = OAMrm_MCC(1); //0
   rm_UmSI3.locationId.mcc[2]	    = OAMrm_MCC(2); //1
   rm_UmSI3.locationId.mnc[0]	    = OAMrm_MNC(0); //0
   rm_UmSI3.locationId.mnc[1]	    = OAMrm_MNC(1); //1
   rm_UmSI3.locationId.mnc[2]	    = OAMrm_MNC(2); //1
   rm_UmSI3.locationId.lac 	    = OAMrm_LAC;    //1
   
   //IE: Control Channel Description
   rm_UmSI3.controlChannelDescription.ie_present  = RM_TRUE;
   rm_UmSI3.controlChannelDescription.att	        = OAMrm_ATT_STATE;
   rm_UmSI3.controlChannelDescription.bsAgBlksRes = OAMrm_BS_AG_BLK_RES;
   rm_UmSI3.controlChannelDescription.bsPaMfrms   = OAMrm_BS_PA_MFRMS;
   rm_UmSI3.controlChannelDescription.ccchConf    = OAMrm_CCCH_CONF;
   rm_UmSI3.controlChannelDescription.T3212       = OAMrm_T3212;

   //IE: Cell Options on BCCH
   rm_UmSI3.cellOptions.ie_present	     =  RM_TRUE;
   rm_UmSI3.cellOptions.channelType	     =  CNI_RIL3_CELL_OPTIONS_BCCH;  //1
   rm_UmSI3.cellOptions.pwrc		     =  CNI_RIL3_PWRC_NOT_SET;       //0:no hopping
   rm_UmSI3.cellOptions.radioLinkTimeout =  OAMrm_RADIO_LINK_TIMEOUT;    //3
   rm_UmSI3.cellOptions.bcchDtx	     =  OAMrm_MS_UPLINK_DTX_STATEa;   //2

   //IE: Cell Reselection Parameters
   rm_UmSI3.cellSelectionParam.ie_present     = RM_TRUE;
   rm_UmSI3.cellSelectionParam.cellReselectHysteresis = OAMrm_CELL_RESEL_HYSTERESIS; //2
   rm_UmSI3.cellSelectionParam.msTxpwrMaxCCH  =	OAMrm_MS_TX_PWR_MAX_CCH;             //0
   rm_UmSI3.cellSelectionParam.rxlevAccessMin = OAMrm_MS_RELEV_ACCESS_MIN;           //6
   rm_UmSI3.cellSelectionParam.neci 	    = CNI_RIL3_NECI_NOT_SUPPORTED;         //0
   rm_UmSI3.cellSelectionParam.acs	          = CNI_RIL3_ACS_USE_SI4_RESET_OCTETS;   //0

   //IE: Rach Control
   rm_UmSI3.rachControl.ie_present	     = RM_TRUE;
   rm_UmSI3.rachControl.maxRetrans       = OAMrm_RA_RETRY_MAX;  	       //3: 7 times
   rm_UmSI3.rachControl.txInt		     = OAMrm_RA_TX_INTEGER;          //0: wait 3 slots
   rm_UmSI3.rachControl.cellBarredAccess = barState;                     //OAMrm_RA_CELL_BARRED_STATE;
   rm_UmSI3.rachControl.reestablish	     = OAMrm_RA_REST_ALLOWED_STATE;  //1: not allowed
   rm_UmSI3.rachControl.emergencyCall    = OAMrm_RA_EC_ALLOWED_STATE;    //1: not allowed
   for (i=0;i<16;i++)   
        rm_UmSI3.rachControl.accessClass[i]=OAMrm_RA_AC_ALLOWED_STATE(i);//1:not allowed

   //IE: SI3 Rest Octets  
   rm_UmSI3.si3RestOctets.ie_present  = RM_TRUE;
   rm_UmSI3.si3RestOctets.pi		  = CNI_RIL3_SELECTION_PARAMETER_PRESENT; //1
   rm_UmSI3.si3RestOctets.cbq			   = OAMrm_CELL_BAR_QUALIFY;  //Cell Bar Qualify
   rm_UmSI3.si3RestOctets.cellReselectOffset = OAMrm_CELL_RESELECT_OFFSET;
   rm_UmSI3.si3RestOctets.temporaryOffset	   = OAMrm_TEMP_OFFEST;
   rm_UmSI3.si3RestOctets.penaltyTime	   = OAMrm_PENALTY_TIME;  //20s
   rm_UmSI3.si3RestOctets.poi			   = CNI_RIL3_POWER_OFFSET_NOT_PRESENTE;
   rm_UmSI3.si3RestOctets.powerOffset	   = 0;  //Not applicable
   rm_UmSI3.si3RestOctets.si2ter		   = CNI_RIL3_SI_2TER_NOT_PRESENT;
   rm_UmSI3.si3RestOctets.ecsi		   = CNI_RIL3_EARLY_CLASSMARK_SENDING_FORBIDDEN;
   rm_UmSI3.si3RestOctets.wi			   = CNI_RIL3_WHERE_NOT_PRESENT;
   rm_UmSI3.si3RestOctets.where		   = 0; //Not applicable

} /* rm_PopulateSI3() */

#endif /* __RM_POPULATESI3_CPP__  */
