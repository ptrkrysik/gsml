/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_CCINTF_H__
#define __RM_CCINTF_H__

#include "lapdm\lapdm_l3intf.h"
#include "ril3\ril3irt.h"

T_CNI_LAPDM_OID rm_GetOid (T_CNI_IRT_ID entryId, T_CNI_LAPDM_SAPI sapi);
T_CNI_IRT_ID rm_GetSacchEntryId (T_CNI_IRT_ID entryId);

#endif //__RM_CCINTF_H__
