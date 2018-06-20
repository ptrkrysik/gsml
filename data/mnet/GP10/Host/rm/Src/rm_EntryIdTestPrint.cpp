/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_ENTRYIDTESTPRINT_CPP__
#define __RM_ENTRYIDTESTPRINT_CPP__

#include "rm\rm_head.h"

void rm_EntryIdTestPrint(u8 tchfIdx, u8 sdcch4Idx)
{
   u8	   		i, iEntryId;

   /* Print TCHF channel idx-entryId mapping */
   for (i=0; i<tchfIdx; i++)
   { 
        iEntryId = RM_TCHF_ENTRYIDX(rm_PhyChanTchf[i].trxNumber,
				        rm_PhyChanTchf[i].chanNumberMSB ); 

        PDEBUG__(("%dth TCHF entryId data: iEntryId=%d ,entryId=%d\n",
		    i,
		    iEntryId,
		    rm_TchfEntryId[iEntryId]
		  ));
   }

   /* Print SDCCH4 channel idx-entryId mapping */
   for (i=0; i<sdcch4Idx; i++)
   { 
        iEntryId = RM_SDCCH4_ENTRYIDX(rm_PhyChanSdcch4[i].trxNumber,
				        rm_PhyChanSdcch4[i].chanNumberMSB ); 

        PDEBUG__(("%dth SDCCH4 entryId data: iEntryId=%d,entryId=%d\n",
		    i,
		    iEntryId,
		    rm_Sdcch4EntryId[iEntryId]
		  ));
   }

} /* End of rm_EntryIdTestPrint() */

#endif /* __RM_ENTRYIDTESTPRINT_CPP__ */
