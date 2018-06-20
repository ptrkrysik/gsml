/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_SYSINFO_CPP__
#define __GRR_SYSINFO_CPP__

#include "grr\grr_head.h"

void grr_PopulateSi2(T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   unsigned char i, idx;

   DBG_FUNC("grr_PopulateSi2", GRR_LAYER_OAM);
   DBG_ENTER();

   DBG_TRACE("grr_PopulateSi2: update SI2 with barState setting(%d)\n", barState);

   //ZAP grr_Si2 to have a good clean-up
   memset(&grr_Si2, 0, sizeof(grr_Si2_t)); 

   //HEADER: PD,MT,SI
   grr_Si2.header.protocol_descriminator = CNI_RIL3_PD_RRM;
   grr_Si2.header.si_ti		           = 0;
   grr_Si2.header.message_type	     = CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_2;

   //IE: Bcch Frequency List
   grr_Si2.bcchFreqList.ie_present       = true;
   grr_Si2.bcchFreqList.extInd	     = CNI_RIL3_EXT_IND_BA_COMPLETE;
   grr_Si2.bcchFreqList.baInd	           = CNI_RIL3_EXT_IND_BA_COMPLETE;
   idx=0;
   for (i=0;i<OAMgrr_RESEL_ADJCELL_NO_MAX;i++)
   {
        if (OAMgrr_RESEL_ADJCELL_ID(i) != OAMgrr_RESEL_ADJCELL_ID_NULL)
            grr_Si2.bcchFreqList.arfcn[idx++] = OAMgrr_RESEL_ADJCELL_BCCH_ARFCN(i);
   } 
   grr_Si2.bcchFreqList.numRFfreq	     = idx;
   grr_Si2.bcchFreqList.format = CNI_RIL3_SelectFrequencyListFormat(
  	                                              grr_Si2.bcchFreqList.arfcn,idx);
  
   //IE: Ncc Permitted
   grr_Si2.nccPermitted.ie_present	     = true;
   for (i=0;i<OAMgrr_NCC_PERMITTED_MAX;i++)
        grr_Si2.nccPermitted.nccPermitted[i] = OAMgrr_NCC_PERMITTED(i);
   
   //IE: Rach Control
   grr_Si2.rachControl.ie_present	     = true;
   grr_Si2.rachControl.maxRetrans        = OAMgrr_RA_RETRY_MAX;  	        //3: 7 times
   grr_Si2.rachControl.txInt		     = OAMgrr_RA_TX_INTEGER;          //0: wait 3 slots
   grr_Si2.rachControl.cellBarredAccess  = barState;                      //OAMgrr_RA_CELL_BARRED_STATE;
   grr_Si2.rachControl.reestablish	     = OAMgrr_RA_REST_ALLOWED_STATE;  //1: not allowed
   grr_Si2.rachControl.emergencyCall     = OAMgrr_RA_EC_ALLOWED_STATE;    //1: not allowed
   for (i=0;i<16;i++)   
        grr_Si2.rachControl.accessClass[i]=OAMgrr_RA_AC_ALLOWED_STATE(i); //1:not allowed

} //grr_PopulateSi2()


void grr_PopulateSi3(T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   unsigned char  i;

   DBG_FUNC("grr_PopulateSi3", GRR_LAYER_OAM);
   DBG_ENTER();

   DBG_TRACE("grr_grr_PopulateSi3: update SI3 with barState setting(%d)\n", barState);

   //ZAP grr_Si3 to have a good clean-up
   memset(&grr_Si3, 0, sizeof(grr_Si3_t)); 

   //HEADER: PD,MT,SI
   grr_Si3.header.protocol_descriminator = CNI_RIL3_PD_RRM;
   grr_Si3.header.si_ti		           = 0;
   grr_Si3.header.message_type	     = CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_3;


   //IE: Cell ID
   grr_Si3.cellId.ie_present	    = true;
   grr_Si3.cellId.value	          = OAMgrr_CELLID; //0x0001

   //IE: Location Area Identification
   grr_Si3.locationId.ie_present = true;
   grr_Si3.locationId.mcc[0]	    = OAMgrr_MCC(0); //0
   grr_Si3.locationId.mcc[1]	    = OAMgrr_MCC(1); //0
   grr_Si3.locationId.mcc[2]	    = OAMgrr_MCC(2); //1
   grr_Si3.locationId.mnc[0]	    = OAMgrr_MNC(0); //0
   grr_Si3.locationId.mnc[1]	    = OAMgrr_MNC(1); //1
   grr_Si3.locationId.mnc[2]	    = OAMgrr_MNC(2); //1	kevinlim 05/11/01
   grr_Si3.locationId.lac 	    = OAMgrr_LAC;    //1
   
   //IE: Control Channel Description
   grr_Si3.controlChannelDescription.ie_present  = true;
   grr_Si3.controlChannelDescription.att	       = OAMgrr_ATT_STATE;
   grr_Si3.controlChannelDescription.bsAgBlksRes = OAMgrr_BS_AG_BLK_RES;
   grr_Si3.controlChannelDescription.bsPaMfrms   = OAMgrr_BS_PA_MFRMS;
   grr_Si3.controlChannelDescription.ccchConf    = OAMgrr_CCCH_CONF;
   grr_Si3.controlChannelDescription.T3212       = OAMgrr_T3212;

   //IE: Cell Options on BCCH
   grr_Si3.cellOptions.ie_present	     =  true;
   grr_Si3.cellOptions.channelType	     =  CNI_RIL3_CELL_OPTIONS_BCCH;  //1
   grr_Si3.cellOptions.pwrc		     =  CNI_RIL3_PWRC_NOT_SET;       //0:no hopping
   grr_Si3.cellOptions.radioLinkTimeout  =  OAMgrr_RADIO_LINK_TIMEOUT;   //3
   grr_Si3.cellOptions.bcchDtx	     =  OAMgrr_MS_UPLINK_DTX_STATEa; //2

   //IE: Cell Reselection Parameters
   grr_Si3.cellSelectionParam.ie_present             = true;
   grr_Si3.cellSelectionParam.cellReselectHysteresis = OAMgrr_CELL_RESEL_HYSTERESIS;      //2
   grr_Si3.cellSelectionParam.msTxpwrMaxCCH          = OAMgrr_MS_TX_PWR_MAX_CCH;          //0
   grr_Si3.cellSelectionParam.rxlevAccessMin         = OAMgrr_MS_RELEV_ACCESS_MIN;        //6
   grr_Si3.cellSelectionParam.neci 	                 = CNI_RIL3_NECI_NOT_SUPPORTED;       //0
   grr_Si3.cellSelectionParam.acs	                 = CNI_RIL3_ACS_USE_SI4_RESET_OCTETS; //0

   //IE: Rach Control
   grr_Si3.rachControl.ie_present	    = true;
   grr_Si3.rachControl.maxRetrans       = OAMgrr_RA_RETRY_MAX;  	       //3: 7 times
   grr_Si3.rachControl.txInt		    = OAMgrr_RA_TX_INTEGER;          //0: wait 3 slots
   grr_Si3.rachControl.cellBarredAccess = barState;                      //OAMgrr_RA_CELL_BARRED_STATE;
   grr_Si3.rachControl.reestablish	    = OAMgrr_RA_REST_ALLOWED_STATE;  //1: not allowed
   grr_Si3.rachControl.emergencyCall    = OAMgrr_RA_EC_ALLOWED_STATE;    //1: not allowed
   for (i=0;i<16;i++)   
        grr_Si3.rachControl.accessClass[i] = OAMgrr_RA_AC_ALLOWED_STATE(i); //1:not allowed

   //IE: SI3 Rest Octets  
   grr_Si3.si3RestOctets.ie_present         = true;
   grr_Si3.si3RestOctets.pi		        = CNI_RIL3_SELECTION_PARAMETER_PRESENT;  //1
   grr_Si3.si3RestOctets.cbq			  = OAMgrr_CELL_BAR_QUALIFY;               //Cell Bar Qualify
   grr_Si3.si3RestOctets.cellReselectOffset = OAMgrr_CELL_RESELECT_OFFSET;
   grr_Si3.si3RestOctets.temporaryOffset	  = OAMgrr_TEMP_OFFEST;
   grr_Si3.si3RestOctets.penaltyTime	  = OAMgrr_PENALTY_TIME;  			 //20s
   grr_Si3.si3RestOctets.poi			  = CNI_RIL3_POWER_OFFSET_NOT_PRESENTE;
   grr_Si3.si3RestOctets.powerOffset	  = 0;  						 //Not applicable
   grr_Si3.si3RestOctets.si2ter		  = CNI_RIL3_SI_2TER_NOT_PRESENT;
   grr_Si3.si3RestOctets.ecsi		        = CNI_RIL3_EARLY_CLASSMARK_SENDING_FORBIDDEN;
   grr_Si3.si3RestOctets.wi			  = CNI_RIL3_WHERE_NOT_PRESENT;
   grr_Si3.si3RestOctets.where		  = 0; 						 //Not applicable

   //G2
   if (OAMgrr_GPRS_FEATURE_SETTING)
   {
	grr_Si3.si3RestOctets.gprs_indicator.indicator_present = true;
	grr_Si3.si3RestOctets.gprs_indicator.ra_color = OAMgrr_GPRS_RA_COLOR;
	grr_Si3.si3RestOctets.gprs_indicator.si13_position = OAMgrr_SI13_POS; 
   } else
	grr_Si3.si3RestOctets.gprs_indicator.indicator_present = false;

} //grr_PopulateSi3()


void grr_PopulateSi4(T_CNI_RIL3_CELL_BARRED_ACCESS barState)
{
   unsigned char i;

   DBG_FUNC("grr_PopulateSi4", GRR_LAYER_OAM);
   DBG_ENTER();

   DBG_TRACE("grr_grr_PopulateSi4: update SI4 with barState setting(%d)\n", barState);

   //ZAP grr_Si4 to have a good clean-up
   memset(&grr_Si4, 0, sizeof(grr_Si4_t)); 

   //HEADER: PD,MT,SI
   grr_Si4.header.protocol_descriminator = CNI_RIL3_PD_RRM;
   grr_Si4.header.si_ti		           = 0;
   grr_Si4.header.message_type	     = CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_4;

   //IE: Location Area Identification
   grr_Si4.locationId.ie_present = true;
   grr_Si4.locationId.mcc[0]	    = OAMgrr_MCC(0); //0
   grr_Si4.locationId.mcc[1]	    = OAMgrr_MCC(1); //0
   grr_Si4.locationId.mcc[2]	    = OAMgrr_MCC(2); //1
   grr_Si4.locationId.mnc[0]	    = OAMgrr_MNC(0); //0
   grr_Si4.locationId.mnc[1]	    = OAMgrr_MNC(1); //1
   grr_Si4.locationId.mnc[2]	    = OAMgrr_MNC(2); //1
   grr_Si4.locationId.lac 	    = OAMgrr_LAC;    //1

   //IE: Cell Reselection Parameters
   grr_Si4.cellSelectionParam.ie_present             = true;
   grr_Si4.cellSelectionParam.cellReselectHysteresis = OAMgrr_CELL_RESEL_HYSTERESIS;      //2
   grr_Si4.cellSelectionParam.msTxpwrMaxCCH          = OAMgrr_MS_TX_PWR_MAX_CCH;          //0
   grr_Si4.cellSelectionParam.rxlevAccessMin         = OAMgrr_MS_RELEV_ACCESS_MIN;        //6
   grr_Si4.cellSelectionParam.neci 	                 = CNI_RIL3_NECI_NOT_SUPPORTED;       //0
   grr_Si4.cellSelectionParam.acs	                 = CNI_RIL3_ACS_USE_SI4_RESET_OCTETS; //0

   //IE: Rach Control
   grr_Si4.rachControl.ie_present	     = true;
   grr_Si4.rachControl.maxRetrans        = OAMgrr_RA_RETRY_MAX;  	        //3: 7 times
   grr_Si4.rachControl.txInt		     = OAMgrr_RA_TX_INTEGER;          //0: wait 3 slots
   grr_Si4.rachControl.cellBarredAccess  = barState;                      //OAMgrr_RA_CELL_BARRED_STATE; 
   grr_Si4.rachControl.reestablish	     = OAMgrr_RA_REST_ALLOWED_STATE;  //1: not allowed
   grr_Si4.rachControl.emergencyCall     = OAMgrr_RA_EC_ALLOWED_STATE;    //1: not allowed
   for (i=0;i<16;i++)   
        grr_Si4.rachControl.accessClass[i]=OAMgrr_RA_AC_ALLOWED_STATE(i); //1:not allowed

   //IE(optional):CBCH channel description
   if (OAMgrr_CB_FEATURE_SETTING)
   {
       grr_Si4.cbchChannelDescription.ie_present       = true; 
       grr_Si4.cbchChannelDescription.hopping          = false; 
       grr_Si4.cbchChannelDescription.channelType	 = CNI_RIL3_SDCCH_ACCH_4orCBCH;
       grr_Si4.cbchChannelDescription.subChannelNumber = 2;
       grr_Si4.cbchChannelDescription.TN               = OAMgrr_CBCH_TRX_SLOT; 
       grr_Si4.cbchChannelDescription.TSC              = OAMgrr_POCKETBTS_TSC;
       grr_Si4.cbchChannelDescription.arfcn            = OAMgrr_ARFCN(OAMgrr_CBCH_TRX);
   }

   //IE: SI4 Rest Octets  
   grr_Si4.si4RestOctets.ie_present         = true;
   grr_Si4.si4RestOctets.pi		        = CNI_RIL3_SELECTION_PARAMETER_PRESENT; //1
   grr_Si4.si4RestOctets.cbq			  = OAMgrr_CELL_BAR_QUALIFY;              //default 0
   grr_Si4.si4RestOctets.cellReselectOffset = OAMgrr_CELL_RESELECT_OFFSET;
   grr_Si4.si4RestOctets.temporaryOffset	  = OAMgrr_TEMP_OFFEST;
   grr_Si4.si4RestOctets.penaltyTime	  = OAMgrr_PENALTY_TIME;                  //20s
   grr_Si4.si4RestOctets.poi			  = CNI_RIL3_POWER_OFFSET_NOT_PRESENTE;
   grr_Si4.si4RestOctets.powerOffset	  = 0;                                    //Not applicable

   //G2
   if (OAMgrr_GPRS_FEATURE_SETTING)
   {
	grr_Si4.si4RestOctets.gprs_indicator.indicator_present = true;
	grr_Si4.si4RestOctets.gprs_indicator.ra_color = OAMgrr_GPRS_RA_COLOR;
	grr_Si4.si4RestOctets.gprs_indicator.si13_position = OAMgrr_SI13_POS; 
   } else
	grr_Si4.si4RestOctets.gprs_indicator.indicator_present = false;

} //grr_PopulateSi4()



void grr_PopulateSi5(void)
{
   unsigned char idx,i;

   DBG_FUNC("grr_PopulateSi5", GRR_LAYER_OAM);
   DBG_ENTER();

   DBG_TRACE("grr_grr_PopulateSi5: Update SI5\n");

   //ZAP grr_Si5 to have a good clean-up
   memset(&grr_Si5, 0, sizeof(grr_Si5_t)); 

   //HEADER: PD,MT,SI
   grr_Si5.header.protocol_descriminator = CNI_RIL3_PD_RRM;
   grr_Si5.header.si_ti		           = 0;
   grr_Si5.header.message_type	     = CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_5;

   //IE: Bcch Frequency List
   grr_Si5.bcchFreqList.ie_present       = true;
   grr_Si5.bcchFreqList.extInd	     = CNI_RIL3_EXT_IND_BA_COMPLETE;
   grr_Si5.bcchFreqList.baInd	           = CNI_RIL3_EXT_IND_BA_COMPLETE;

   idx=0;
   for (i=0;i<OAMgrr_HO_ADJCELL_NO_MAX;i++)
   {
        if (OAMgrr_HO_ADJCELL_ID(i) != OAMgrr_HO_ADJCELL_ID_NULL)
            grr_Si5.bcchFreqList.arfcn[idx++] = OAMgrr_HO_ADJCELL_BCCH_ARFCN(i);
   } 

   grr_Si5.bcchFreqList.numRFfreq = idx;
   grr_Si5.bcchFreqList.format    = CNI_RIL3_SelectFrequencyListFormat(
  	                                          grr_Si5.bcchFreqList.arfcn,idx);
} //grr_PopulateSi5()


void grr_PopulateSi6(void)
{
   unsigned char i;

   DBG_FUNC("grr_PopulateSi6", GRR_LAYER_OAM);
   DBG_ENTER();

   DBG_TRACE("grr_grr_PopulateSi6: Update SI6\n");

   //ZAP grr_Si6 to have a good clean-up
   memset(&grr_Si6, 0, sizeof(grr_Si6_t)); 

   //HEADER: PD,MT,SI
   grr_Si6.header.protocol_descriminator = CNI_RIL3_PD_RRM;
   grr_Si6.header.si_ti		           = 0;
   grr_Si6.header.message_type	     = CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_6;

   //IE: Cell ID
   grr_Si6.cellId.ie_present = true;
   grr_Si6.cellId.value	     = OAMgrr_CELLID; //0x0001

   //IE: Location Area Identification
   grr_Si6.locationId.ie_present = true;
   grr_Si6.locationId.mcc[0]	   = OAMgrr_MCC(0); //0
   grr_Si6.locationId.mcc[1]	   = OAMgrr_MCC(1); //0
   grr_Si6.locationId.mcc[2]	   = OAMgrr_MCC(2); //1
   grr_Si6.locationId.mnc[0]	   = OAMgrr_MNC(0); //0
   grr_Si6.locationId.mnc[1]	   = OAMgrr_MNC(1); //1
   grr_Si6.locationId.mnc[2]	   = OAMgrr_MNC(2); //1
   grr_Si6.locationId.lac 	   = OAMgrr_LAC;    //1

   //IE: Cell Options on BCCH
   grr_Si6.cellOptions.ie_present	     = true;
   grr_Si6.cellOptions.channelType	     = CNI_RIL3_CELL_OPTIONS_SACCH; //1
   grr_Si6.cellOptions.pwrc		     = CNI_RIL3_PWRC_NOT_SET;       //0:no hopping
   grr_Si6.cellOptions.radioLinkTimeout  = OAMgrr_RADIO_LINK_TIMEOUT;   //3
   grr_Si6.cellOptions.sacchDtx	     = OAMgrr_MS_UPLINK_DTX_STATE;

   //IE: Ncc Permitted
   grr_Si6.nccPermitted.ie_present	     = true;
   for (i=0;i<OAMgrr_NCC_PERMITTED_MAX;i++)
        grr_Si6.nccPermitted.nccPermitted[i] = OAMgrr_NCC_PERMITTED(i);

} //grr_PopulateSi6()

void grr_PopulateSi13(void)
{
   DBG_FUNC("grr_PopulateSi13", GRR_LAYER_OAM);
   DBG_ENTER();

   DBG_TRACE("grr_grr_PopulateSi13: update SI13\n");

   //ZAP grr_Si13 to have a good clean-up
   memset(&grr_Si13, 0, sizeof(grr_Si13_t)); 

   //HEADER: PD,MT,SI
   grr_Si13.header.protocol_descriminator = CNI_RIL3_PD_RRM;
   grr_Si13.header.si_ti	          = 0;
   grr_Si13.header.message_type	= CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_13;

   //IE: SI13 Rest Octets  
   grr_Si13.si13RestOctets.ie_present = true;
   grr_Si13.si13RestOctets.bcch_change_mark = OAMgrr_BCCH_CHANGE_MARK;  //0-4
   grr_Si13.si13RestOctets.si_change_field  = OAMgrr_SI_CHANGE_FIELD;   //0-15

   grr_Si13.si13RestOctets.flag_sm = false;
   grr_Si13.si13RestOctets.si13_change_mark = 0;
   grr_Si13.si13RestOctets.gprs_ms_allocation.ie_present = false;

   grr_Si13.si13RestOctets.flag_pbcch = false;
   grr_Si13.si13RestOctets.without_pbcch.rac = OAMgrr_GPRS_RAC;
   grr_Si13.si13RestOctets.without_pbcch.spgc_ccch_sup = false;
   grr_Si13.si13RestOctets.without_pbcch.priority_access_thr = 6; // priority 1-4
   grr_Si13.si13RestOctets.without_pbcch.network_control_order = 1; 
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.ie_present = true;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.nmo = OAMgrr_GPRS_NMO;

   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.t3168 = OAMgrr_GPRS_T3168;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.t3192 = OAMgrr_GPRS_T3192;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.drx_timer_max = OAMgrr_GPRS_DRX_TIMER_MAX;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.access_burst_type = OAMgrr_GPRS_ACCESS_BURST_TYPE;

   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.control_ack_type = OAMgrr_GPRS_CONTROL_ACK_TYPE;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.bs_cv_max = OAMgrr_GPRS_BS_CV_MAX;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.pan_dec = OAMgrr_GPRS_PAN_DEC;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.pan_inc = OAMgrr_GPRS_PAN_INC;
   grr_Si13.si13RestOctets.without_pbcch.gprs_cell_options.pan_max = OAMgrr_GPRS_PAN_MAX;

   grr_Si13.si13RestOctets.without_pbcch.gprs_power_control_parameters.alpha = OAMgrr_GPRS_ALPHA;
   grr_Si13.si13RestOctets.without_pbcch.gprs_power_control_parameters.t_avg_w = OAMgrr_GPRS_T_AVG_W;
   grr_Si13.si13RestOctets.without_pbcch.gprs_power_control_parameters.t_avg_t = OAMgrr_GPRS_T_AVG_T;
   grr_Si13.si13RestOctets.without_pbcch.gprs_power_control_parameters.pc_meas_chan = OAMgrr_GPRS_PC_MEAS_CHAN;
   grr_Si13.si13RestOctets.without_pbcch.gprs_power_control_parameters.n_avg_i = OAMgrr_GPRS_N_AVG_I;

   DBG_LEAVE();

} //grr_PopulateSi13()

#endif //__GRR_SYSINFO_CPP__

