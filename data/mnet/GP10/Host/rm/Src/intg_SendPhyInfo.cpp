/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __INTG_SENDPHYINFO_CPP__
#define __INTG_SENDPHYINFO_CPP__

//HOADDED
//!!! TCH/F assumed
#include "rm/rm_head.h"

void intg_SendPhyInfo(u16 chan)
{
     T_CNI_IRT_ID     entryId;
     rm_L3Data_t      l3_data;

     l3_data.buffer[0] = (u8)RM_PD;
     l3_data.buffer[1] = (u8)CNI_RIL3RRM_MSGID_PHYSICAL_INFORMATION;
     l3_data.buffer[2] = rm_pSmCtxt->pPhyChan->hoTa;
     l3_data.msgLength = 3;

     if (!rm_TrxSlotToEntryId(chan, &entryId))
          EDEBUG__(("intg_SendPhyInfo: Failed to convert chan=%x to entryId\n",chan));

     //Send out l3_data via DL_UNIT_DATA_REQ
     CNI_LAPDM_Dl_Unit_Data_Request(rm_SmCtxt[entryId].mOId, 0, L2T_NORMAL_HEADER, &l3_data);

} 
#endif /*__INTG_SENDPHYINFO_CPP__*/
