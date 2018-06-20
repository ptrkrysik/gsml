/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITLAPDMTESTPRINT_CPP__
#define __RM_INITLAPDMTESTPRINT_CPP__

#include "rm\rm_head.h"

void rm_InitLAPDmTestPrint(u8 tchfIdx, u8 sdcch4Idx)
{
   u8	   i;

   /* Print TCHF channel description data */
   for (i=0; i<tchfIdx; i++)
   { 
        DDEBUG__ (("%dth TCHF Chan Descrption\n",i));

        DDEBUG__ (("mOId=0x%08x, aOId=0x%08x\n",
                    rm_PhyChanTchf[i].mOId,
                    rm_PhyChanTchf[i].aOId
	         ));
    }

   /* Print SDCCH4 channel description data */
   for (i=0; i<sdcch4Idx; i++)
   { 
        DDEBUG__ (("%dth SDCCH4 Chan Descrption\n",i));
        DDEBUG__ (("mOId=0x%08x, aOId=0x%08x\n",
                    rm_PhyChanSdcch4[i].mOId,
                    rm_PhyChanSdcch4[i].aOId
	         ));

    }

    DDEBUG__ (("BCCH Chan Descrption\n"));
    DDEBUG__ (("mOId=0x%08x, aOId=0x%08x\n",
                rm_PhyChanBCcch[RM_BCCH_IDX].mOId,
                rm_PhyChanBCcch[RM_BCCH_IDX].aOId
             ));
	 
    DDEBUG__ (("RACH Chan Descrption\n"));
    DDEBUG__ (("mOId=0x%08x, aOId=0x%08x\n",
                rm_PhyChanBCcch[RM_RACH_IDX].mOId,
                rm_PhyChanBCcch[RM_RACH_IDX].aOId
             ));

} /* End of rm_InitLAPDmTestPrint() */

#endif /* __RM_INITLAPDMTESTPRINT_CPP__ */
