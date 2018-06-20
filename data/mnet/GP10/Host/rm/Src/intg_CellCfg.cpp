/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_CELLCFG_CPP__
#define __INTG_CELLCFG_CPP__

#include "rm/rm_head.h"

void intg_CellCfg(s16 arfcn, s16 narfcn, u8 bsic, u8 accessClass, u8 mnc1, u8 mnc2)
{
     ((CarrierFrequencyEntry*)
      (grr_OamData.radioCarrierPackage[
                  grr_OamData.transceiverPackage[0].relatedRadioCarrier
                  ].carrierFrequencyList))[0].carrierFrequency = arfcn;
//     ((CarrierFrequencyEntry*)
//      (grr_OamData.radioCarrierPackage[
//                  grr_OamData.transceiverPackage[1].relatedRadioCarrier
//                  ].carrierFrequencyList))[0].carrierFrequency = arfcn;
     ((CellAllocationEntry*)
      (grr_OamData.btsBasicPackage.cellAllocationTable))[0].cellAllocation = arfcn;
     ((AdjCell_HandoverEntry*)
      (grr_OamData.adjacentCellPackage.adjCell_handoverTable))[
                  0].adjCell_bCCHFrequency = narfcn; //1st neig-cell in ho-adj cell list
     ((AdjCell_ReselectionEntry*)
     (grr_OamData.adjacentCellPackage.adjCell_reselectionTable))[0
                  ].adjCell_reselectionBCCHFrequency = narfcn;
     grr_OamData.btsBasicPackage.bts_ncc = (bsic>>3)&0x00000007;
     grr_OamData.btsBasicPackage.bts_cid = bsic&0x00000007;
     for (int i=0;i<16;i++)   
         ((NotAllowedAccessClassEntry*)
          (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[
          i].notAllowedAccessClass = 1; //1:not allowed
     ((NotAllowedAccessClassEntry*)
      (grr_OamData.btsOptionsPackage.notAllowedAccessClassTable))[
       accessClass].notAllowedAccessClass = 0; //0: allowed
     ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[0]=mnc1;
     ((unsigned char*)&(grr_OamData.btsBasicPackage.bts_mnc))[1]=mnc2;
} /* intg_CellCfg() */

#endif /*__INTG_CELLCFG_CPP__*/
