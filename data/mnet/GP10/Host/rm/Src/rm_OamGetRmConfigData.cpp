/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_OAMGETRMCONFIGDATA_CPP__
#define __RM_OAMGETRMCONFIGDATA_CPP__

//GP2
#if 0

#include "rm\rm_head.h"

void rm_OamGetRmConfigData(void)
{
   STATUS ret;
   
   RDEBUG__(("ENTER-rm_OamGetRmConfigData\n")); 
   ret = getMibStruct(MIB_resourceConfigData, (u8*)&rm_OamData.rmConfigData,
                      sizeof(ResourceConfigData));
   if (ret != STATUS_OK)
       EDEBUG__(("ERROR-rm_OamGetRmConfigData: getMibVarAdr error=%d\n",ret));

//   rm_TestPrintOamRmConfigData();
}

//GP2
#endif //#if 0

#endif /* __RM_OAMGETRMCONFIGDATA_CPP__ */
