/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef  __RM_SENDALLSYSTEMINFOSTOBCCHTRX_CPP__
#define  __RM_SENDALLSYSTEMINFOSTOBCCHTRX_CPP__

#include "rm\rm_head.h"

void rm_SendAllSystemInfosToBcchTrx( u8 trx)
{
   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_SendAllSystemInfosToBcchTrx\n"));

   // TEST: System information 1 is not needed now
   rm_SendSystemInfo2ToBcchTrx(trx,RM_L1SYSINFO_TYPE_2,OAMrm_RA_CELL_BARRED_STATE);
   rm_SendSystemInfo3ToBcchTrx(trx,RM_L1SYSINFO_TYPE_3,OAMrm_RA_CELL_BARRED_STATE);
   rm_SendSystemInfo4ToBcchTrx(trx,RM_L1SYSINFO_TYPE_4,OAMrm_RA_CELL_BARRED_STATE);

} /* End of rm_SendAllSystemIinfosToBcchTrx() */

#endif /* __RM_SENDALLSYSTEMINFOSTOBCCHTRX_CPP__ */
