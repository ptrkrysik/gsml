/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SOLVEPAGINGGROUP_CPP__
#define __RM_SOLVEPAGINGGROUP_CPP__

#include "rm\rm_head.h"

u8 rm_SolvePagingGroup(rm_IeMobileId_t *pMobileId)
{
 u8   N,pageGroup, NDivBsPaMfrms;
 u16  imsiMod1000;

 imsiMod1000 = rm_GetImsiMod1000(pMobileId);

 N              = (u8) (3-OAMrm_BS_AG_BLK_RES)*(OAMrm_BS_PA_MFRMS+2);
 NDivBsPaMfrms  = (u8) N/(OAMrm_BS_PA_MFRMS+2);
 pageGroup      = (u8) (imsiMod1000%(u8)(OAMrm_BS_CC_CHANS*N)) % N;

 return (pageGroup);
  
// *pPaging51MF   = (u8) (pageGroup / NDivBsPaMfrms);
// *pPagingBIdx   = (u8) (pageGroup % NDivBsPaMfrms);
 
} /* rm_SolvePagingGroup() */

#endif /* __RM_SOLVEPAGINGGROUP_CPP__ */
