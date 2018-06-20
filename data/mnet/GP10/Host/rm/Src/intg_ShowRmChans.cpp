/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SHOWRMCHANS_CPP__
#define __INTG_SHOWRMCHANS_CPP__

#include "rm\rm_head.h"

void intg_ShowRmChans(void)
{
   u8	   i;

   /* Print Cell Bar status */
   printf("======== CELL BARRED STATUS ==========\n");
   printf("Overload CellBar Ctrl=%d, MIB Barred Setting=%d, Current Barred Control status=%d\n",
           OAMrm_OVERLOAD_CELLBAR,
           OAMrm_RA_CELL_BARRED_STATE, rm_VcIsBarred);


   /* Print TCHF channel description data */
   printf("========TCHF ALLOCATION STATUS========\n");
   for (i=0; i<15; i++)
   { 
        printf("TCH/F-%d:  state=%d,chan=%02x, mEntryId=%d, mSMstate=%d, amState=%d\n",
                   i,
		       rm_PhyChanTchf[i].state, 
		       rm_PhyChanTchf[i].chanNumberMSB,
		       rm_PhyChanTchf[i].pMSmCtxt->mEntryId,
		       rm_PhyChanTchf[i].pMSmCtxt->state,
                   rm_PhyChanTchf[i].amState
	           );
    }

   /* Print SDCCH4 channel description data */
   printf("\n========SDCCH ALLOCATION STATUS========\n");
   for (i=0; i<4; i++)
   { 
        printf("SDCCH4-%d: state=%d, chan=%02x, mEntryId=%d, mSMstate=%d, amState=%d\n",
	              i,
        	        rm_PhyChanSdcch4[i].state,
        	        rm_PhyChanSdcch4[i].chanNumberMSB,
                    rm_PhyChanSdcch4[i].pMSmCtxt->mEntryId,
			  rm_PhyChanSdcch4[i].pMSmCtxt->state,
			  rm_PhyChanSdcch4[i].amState
	            );
   }
}

#endif /* __INTG_SHOWRMCHANS_CPP__ */

