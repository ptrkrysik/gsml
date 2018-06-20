/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_OAMUTIL_CPP__
#define __GRR_OAMUTIL_CPP__

#include "grr\grr_head.h"

void grr_RegOamTrap(void)
{
   STATUS ret;

   DBG_FUNC("grr_RegOamTrap", GRR_LAYER_OAM);
   DBG_ENTER();

   //Reset any trap setting to get a clean start
   if ( STATUS_OK != (ret=oam_unsetTrapByModule(MODULE_GRR)) )
   {
       DBG_ERROR("grr_RegOamTrap: unsetTrapByModule failed err(%d)\n", ret);
       printf("grr_RegOamTrap: unsetTrapByModule failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_UNSETTRAPBYMODULE_FAILED);
       assert(0);
   }
    
   //Regsiter to trap BtsBasicPackage
   if ( STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_btsBasicPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap btsBasicPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap btsBasicPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap BtsOptionsPackage
   if ( STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_btsOptionsPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap BtsOptionsPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap BtsOptionsPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap first TransceiverPackages
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_firstTransceiverPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap firstTransceiverPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap firstTransceiverPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap second TransceiverPackages
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_secondTransceiverPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap secondTransceiverPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap secondTransceiverPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap first RadioCarrierPackages
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_firstRadioCarrierPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap firstRadioCarrierPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap firstRadioCarrierPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap second RadioCarrierPackages
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_secondRadioCarrierPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap secondRadioCarrierPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap secondRadioCarrierPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }
	
   //Regsiter to trap T31xx timers
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_t31xxPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap t31xxPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap t31xxPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap AdjacentCellPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_adjacentCellPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap adjacentCellPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap adjacentCellPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap HandoverControlPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_handoverControlPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap HandoverControlPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap HandoverControlPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoAveragingAdjCellPackage	
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoAveragingAdjCellPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoAveragingAdjCellPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoAveragingAdjCellPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoAveragingDistPackage 
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoAveragingDistPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoAveragingDistPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoAveragingDistPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoAveragingQualPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoAveragingLevPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoAveragingLevPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoAveragingLevPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap 
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoAveragingQualPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoAveragingQualPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoAveragingQualPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoThresholdDistPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoThresholdDistPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoThresholdDistPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoThresholdDistPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoThresholdInterfacePackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoThresholdInterfacePackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoThresholdInterfacePackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoThresholdInterfacePackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoThresholdLevPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoThresholdLevPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoThresholdLevPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoThresholdLevPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap hoThresholdQualPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_hoThresholdQualPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap hoThresholdQualPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap hoThresholdQualPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap interferenceAveragingPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_interferenceAveragingPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap interferenceAveragingPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap interferenceAveragingPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap PowerControlPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_powerControlPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap powerControlPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap powerControlPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap pcAveragingLevPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_pcAveragingLevPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap pcAveragingLevPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap pcAveragingLevPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap pcAveragingQualPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_pcAveragingQualPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap pcAveragingQualPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap pcAveragingQualPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap pcLowerThresholdLevPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_pcLowerThresholdLevPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap pcLowerThresholdLevPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap pcLowerThresholdLevPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap pcLowerThresholdQualPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_pcLowerThresholdQualPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap pcLowerThresholdQualPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap pcLowerThresholdQualPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap pcUpperThresholdLevPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_pcUpperThresholdLevPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap pcUpperThresholdLevPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap pcUpperThresholdLevPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   //Register to trap pcUpperThresholdQualPackage
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_pcUpperThresholdQualPackage)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap pcUpperThresholdQualPackage failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap pcUpperThresholdQualPackage failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }
	
   //Register to trap ResourceConfigData 
   if (STATUS_OK != (ret=oam_setTrap(MODULE_GRR, grr_MsgQId, MIB_resourceConfigData)) )
   {
       DBG_ERROR("grr_RegOamTrap: oam_setTrap resourceConfigData failed err(%d)\n", ret);
       printf("grr_RegOamTrap: oam_setTrap resourceConfigData failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTRAP_FAILED);
       assert(0);
   }

   DBG_LEAVE();
}

void grr_FixOamData(void)
{
   unsigned char i,j;

   DBG_FUNC("grr_FixOamData", GRR_LAYER_OAM);
   DBG_ENTER();

   //Complete the half-done lock intent 
   if ( OAMgrr_BTS_AMSTATE != unlocked      ||
        OAMgrr_TRX_AMSTATE(0) != unlocked   ||
        OAMgrr_CHN_AMSTATE(0,0) != unlocked )
   {
        //Lock the whole Bts 
        if (OAMgrr_BTS_AMSTATE != locked)
            grr_SetOamBtsAmState(locked);

        for (i=0;i<OAMgrr_MAX_TRXS;i++)
        {
             if (OAMgrr_TRX_AMSTATE(i) != locked)
                 grr_SetOamTrxAmState(i,locked);

             for (j=0;j<8;j++)
             {
                 if (OAMgrr_CHN_AMSTATE(i,j) != locked)
                     grr_SetOamTrxSlotAmState(i,j,locked);
             }
        } 
   } else
   {  
        for (i=0;i<OAMgrr_MAX_TRXS;i++)
        {
             //Lock the whole trx res if needed
             if (OAMgrr_TRX_AMSTATE(i) != unlocked)
             {
                 if (OAMgrr_TRX_AMSTATE(i) != locked)
	               grr_SetOamTrxAmState(i,locked);

                 for (j=0;j<8;j++)
                 {
                      if (OAMgrr_CHN_AMSTATE(i,j) != locked)
                          grr_SetOamTrxSlotAmState(i,j,locked);
                 }
             } else
             {
                 //TRX is unlocked, handle slots admin states
                 for (j=0;j<8;j++)
                 {
                      if (OAMgrr_CHN_AMSTATE(i,j) != unlocked)
                      {
                          if (OAMgrr_CHN_AMSTATE(i,j) != locked)
 	                        grr_SetOamTrxSlotAmState(i,j,locked);
                      }
                 }
            }
        }
   } 

   DBG_LEAVE();
}

unsigned char grr_GetOamTotalAvailableHoNCells(void)
{
   unsigned char i, count;
   count=0;

   for (i=0;i<OAMgrr_HO_ADJCELL_NO_MAX;i++)
       if (OAMgrr_HO_ADJCELL_ID_t(i)!=OAMgrr_HO_ADJCELL_ID_NULL)
           count++;

   return count;
}

void grr_OverloadedCellBarOff(void)
{
   DBG_FUNC("grr_OverloadedCellBarOff", GRR_LAYER_INTF);
   DBG_ENTER();

   if ( (OAMgrr_BTS_AMSTATE==unlocked) &&
        (OAMgrr_TRX_AMSTATE(0)==unlocked) &&
        (OAMgrr_CHN_AMSTATE(0,0)==unlocked) )
   {
        grr_PassOamMsSystemInformation2(0,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
        grr_PassOamMsSystemInformation3(0,GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
        grr_PassOamMsSystemInformation4(0,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);
   }

   DBG_LEAVE();
}

#endif //__GRR_OAMUTIL_CPP__

