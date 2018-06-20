/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __INTG_CLEARRMCHANS_CPP__
#define __INTG_CLEARRMCHANS_CPP__
 
#include "rm/rm_head.h"

void intg_ClearRmChans(void)
{
   u8	   i;

   for (i=0; i<15; i++)
   { 
        rm_PhyChanTchf[i].state           = RM_PHYCHAN_FREE;
        rm_PhyChanTchf[i].pMSmCtxt->state = RM_S1RM_RUNNING;
        rm_PhyChanTchf[i].pASmCtxt->state = RM_S1RM_RUNNING;
        wdCancel(rm_PhyChanTchf[i].T3101.id);
        wdCancel(rm_PhyChanTchf[i].T3109.id);
        wdCancel(rm_PhyChanTchf[i].T3111.id);
        wdCancel(rm_PhyChanTchf[i].T3L01.id);
   }

   for (i=0; i<4; i++)
   { 
        rm_PhyChanSdcch4[i].state           = RM_PHYCHAN_FREE;
        rm_PhyChanSdcch4[i].pMSmCtxt->state = RM_S1RM_RUNNING;
        rm_PhyChanSdcch4[i].pASmCtxt->state = RM_S1RM_RUNNING;
        wdCancel(rm_PhyChanSdcch4[i].T3101.id);
        wdCancel(rm_PhyChanSdcch4[i].T3109.id);
        wdCancel(rm_PhyChanSdcch4[i].T3111.id);
        wdCancel(rm_PhyChanSdcch4[i].T3L01.id);
   }

   /* Print TCHF channel description data */
   printf("========TCHF ALLOCATION STATUS========\n");
   for (i=0; i<15; i++)
   { 
        printf("TCH/F-%d:  state=%d,chan=%02x, mEntryId=%d,mSMstate=%d\n",
                   i,
		       rm_PhyChanTchf[i].state, 
		       rm_PhyChanTchf[i].chanNumberMSB,
		       rm_PhyChanTchf[i].pMSmCtxt->entryId,
		       rm_PhyChanTchf[i].pMSmCtxt->state
	           );
    }

   /* Print SDCCH4 channel description data */
   printf("\n========SDCCH ALLOCATION STATUS========\n");
   for (i=0; i<4; i++)
   { 
        printf("SDCCH4-%d: state=%d, chan=%02x, mEntryId=%d,mSMstate=%d\n",
	              i,
        	        rm_PhyChanSdcch4[i].state,
        	        rm_PhyChanSdcch4[i].chanNumberMSB,
                    rm_PhyChanSdcch4[i].pMSmCtxt->entryId,
			  rm_PhyChanSdcch4[i].pMSmCtxt->state
	            );
   }
}

#endif /* __INTG_CLEARRMCHANS_CPP__ */

