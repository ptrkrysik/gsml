/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITIRT_CPP__
#define __RM_INITIRT_CPP__

#include "rm\rm_head.h"


void rm_InitIRT( void )
{
   u8		   i;	      /* For loop-control or index to array element */  
   u8 		   iEntryId;  /* Index to entryID arrays containing SM Ctxt */	
   rm_IrtEntryId_t mEntryId;  /* IRT Entry id returned fr. IRT registration */
   rm_IrtEntryId_t aEntryId;  /* IRT Entry id returned fr. IRT registration */
   rm_LapdmOId_t   mOId,aOId; /* Return from LAPDm-instance create function */

   /* TEST USE */
   u8		  noOfTchfs   = 7;
   u8		  noOfSdcch4s = 4;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_InitIRT\n"));

   /* TBD: Shausong */
   //   CNI_RIL3_IRT_Init(); moved to startup script integ.
   /*
   ** Registering an IRT entry for RACH LAPDmOID. <TBD>: Same as rm_InitLAPDm,
   ** no need to register entry for BCCH, PCH, and AGCH channels. In fact, no
   ** LAPDmOIDs have been generated for them by rm_InitLAPDm().
   */

    mEntryId = CNI_RIL3_IRT_Register (
	      rm_PhyChanBCcch[RM_RACH_IDX].mOId, rm_id, mm_id, cc_id
                                    );
  
   /* Check if registration successful */
   if ( mEntryId == CNI_IRT_NULL_ID )
        EDEBUG__(( "ERROR-rm_InitIRT: RACH IRT registration failed\n" ));
   else
	  rm_RachEntryId = mEntryId;
  
   /*
   ** Registering an IRT entry for each TCH/F LAPDmOID
   */
   for (i=0;i<OAMrm_MAX_TCHFS;i++)
//   for (i=0;i<noOfTchfs;i++)
   {
      /*
      ** Create entry id for MAIN signaling channel LAPDmOID
      */
      mEntryId = CNI_RIL3_IRT_Register (
 		   rm_PhyChanTchf[i].mOId, rm_id, mm_id, cc_id );
				    
      /* Check if registration successful */
      if ( mEntryId == CNI_IRT_NULL_ID )
           EDEBUG__(("ERROR-rm_InitIRT: TCH/F FACCH LAPDmOID IRT registration failed\n"));
      else
      {
	   /* TCH/F-FACCH LAPDmOID IRT registration success */
	   rm_SmCtxt[mEntryId].pPhyChan = &rm_PhyChanTchf[i];
	   rm_SmCtxt[mEntryId].state    = RM_S1RM_RUNNING;
	   rm_SmCtxt[mEntryId].smType   = RM_MAIN_SM;
           rm_SmCtxt[mEntryId].chanType = RM_CTFACCH_F;
	   rm_SmCtxt[mEntryId].mOId     = rm_PhyChanTchf[i].mOId;
	   rm_SmCtxt[mEntryId].aOId     = rm_PhyChanTchf[i].aOId;

         rm_PhyChanTchf[i].pMSmCtxt   = &rm_SmCtxt[mEntryId];

         iEntryId = RM_TCHF_ENTRYIDX( rm_PhyChanTchf[i].trxNumber,
				        rm_PhyChanTchf[i].chanNumberMSB ); 
         rm_TchfEntryId[iEntryId]     = mEntryId;

         //printf("rm_InitIrt: TCHFm-%d: iEntryId=%d, mEntryId=%d, mOId=%d\n",
         //        i, iEntryId, mEntryId, rm_PhyChanTchf[i].mOId);
      }

      /*
      ** Create entry id for ACCH signaling channel LAPDmOID
      */
      aEntryId = CNI_RIL3_IRT_Register (
 		   rm_PhyChanTchf[i].aOId, rm_id, mm_id, cc_id );

//      printf("rm_InitIrt: TCH/F=%d,aOId=%d\n",i,rm_PhyChanTchf[i].aOId);
  
      /* Check if registration successful */
      if ( aEntryId == CNI_IRT_NULL_ID )
           EDEBUG__(("ERROR-rm_InitIRT: TCH/F SACCH LAPDmOID IRT registration failed\n"));
      else
      {
	   /* TCH/F-SACCH LAPDmOID IRT registration success */
	   rm_SmCtxt[aEntryId].pPhyChan = &rm_PhyChanTchf[i];
	   rm_SmCtxt[aEntryId].state    = RM_S1RM_RUNNING;
	   rm_SmCtxt[aEntryId].smType   = RM_ACCH_SM;
           rm_SmCtxt[aEntryId].chanType = RM_CTSACCH;
	   rm_SmCtxt[aEntryId].mOId     = rm_PhyChanTchf[i].mOId;
	   rm_SmCtxt[aEntryId].aOId     = rm_PhyChanTchf[i].aOId;

           rm_PhyChanTchf[i].pASmCtxt  = &rm_SmCtxt[aEntryId];
      }

      /* Main/Acch SM remember each other */
      rm_SmCtxt[mEntryId].mEntryId = mEntryId;
      rm_SmCtxt[mEntryId].aEntryId = aEntryId;

      rm_SmCtxt[aEntryId].mEntryId = mEntryId;
      rm_SmCtxt[aEntryId].aEntryId = aEntryId;

      rm_PhyChanTchf[i].mEntryId   = mEntryId;
      rm_PhyChanTchf[i].aEntryId   = aEntryId;

   } /* End of for-i loop: TCHF */

   /*
   ** Registering an IRT entry for each SDCCH/4 LAPDmOID
   */

   for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
//   for (i=0;i<noOfSdcch4s;i++)
   {
      /*
      ** Create entry id for MAIN signaling channel LAPDmOID
      */
      mEntryId = CNI_RIL3_IRT_Register (
 		   rm_PhyChanSdcch4[i].mOId, rm_id, mm_id, cc_id );
      
//      printf("rm_InitIrt: SDCCH=%d,mOId=%d\n",i,rm_PhyChanSdcch4[i].mOId);
  
      /* Check if registration successful */
      if ( mEntryId == CNI_IRT_NULL_ID )
           EDEBUG__(("ERROR-rm_InitIRT: SDCCH4 SDCCH LAPDmOID IRT registration failed\n"));
      else
      {
	   /* SDCCH4-SDCCH LAPDmOID IRT registration success */
	   rm_SmCtxt[mEntryId].pPhyChan  = &rm_PhyChanSdcch4[i];
	   rm_SmCtxt[mEntryId].state     = RM_S1RM_RUNNING;
	   rm_SmCtxt[mEntryId].smType    = RM_MAIN_SM;
	   rm_SmCtxt[mEntryId].chanType  = RM_CTSDCCH;
	   rm_SmCtxt[mEntryId].mOId      = rm_PhyChanSdcch4[i].mOId;
	   rm_SmCtxt[mEntryId].aOId      = rm_PhyChanSdcch4[i].aOId;

           rm_PhyChanSdcch4[i].pMSmCtxt = &rm_SmCtxt[mEntryId];
           rm_PhyChanSdcch4[i].pMSmCtxt->state = RM_S1RM_RUNNING;

           iEntryId = RM_SDCCH4_ENTRYIDX( rm_PhyChanSdcch4[i].trxNumber,
				     rm_PhyChanSdcch4[i].chanNumberMSB ); 
           rm_Sdcch4EntryId[iEntryId]   = mEntryId;
      }

      /*
      ** Create entry id for ACCH signaling channel LAPDmOID
      */
      aEntryId = CNI_RIL3_IRT_Register (
 		   rm_PhyChanSdcch4[i].aOId, rm_id, mm_id, cc_id 
				       );
//      printf("rm_InitIrt: SDCCH=%d,aOId=%d\n",i,rm_PhyChanSdcch4[i].aOId);
  
      /* Check if registration successful */
      if ( aEntryId == CNI_IRT_NULL_ID )
           EDEBUG__(("ERROR-rm_InitIRT: SDCCH4 SACCH LAPDmOID IRT registration failed\n"));
      else
      {
	   /* SDCCH4-SACCH LAPDmOID IRT registration success */
	   rm_SmCtxt[aEntryId].pPhyChan  = &rm_PhyChanSdcch4[i];
	   rm_SmCtxt[aEntryId].state     = RM_S1RM_RUNNING;
	   rm_SmCtxt[aEntryId].smType    = RM_ACCH_SM;
	   rm_SmCtxt[aEntryId].chanType  = RM_CTSACCH;
	   rm_SmCtxt[aEntryId].mOId      = rm_PhyChanSdcch4[i].mOId;
	   rm_SmCtxt[aEntryId].aOId      = rm_PhyChanSdcch4[i].aOId;

           rm_PhyChanSdcch4[i].pASmCtxt = &rm_SmCtxt[aEntryId];
           rm_PhyChanSdcch4[i].pASmCtxt->state = RM_S1RM_RUNNING;

      }

      /* Main/Acch SM remember each other */
      rm_SmCtxt[mEntryId].mEntryId = mEntryId;
      rm_SmCtxt[mEntryId].aEntryId = aEntryId;

      rm_SmCtxt[aEntryId].mEntryId = mEntryId;
      rm_SmCtxt[aEntryId].aEntryId = aEntryId;

      rm_PhyChanSdcch4[i].mEntryId = mEntryId;
      rm_PhyChanSdcch4[i].aEntryId = aEntryId;

   } /* End of for-i loop: SDCCH4 */

   /* TEST PRINT */
   // rm_InitIrtTestPrint(2,4);
   // rm_EntryIdTestPrint(2,4);
	  
} /* End of rm_InitIRT() */

#endif /* __RM_INITIRT_CPP__ */
