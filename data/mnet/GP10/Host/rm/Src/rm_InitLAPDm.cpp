/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITLAPDM_CPP__
#define __RM_INITLAPDM_CPP__

#include "rm\rm_head.h"

void rm_InitLAPDm(void)
{
   u8		     i;		 /* Used for loop-control and array index */
   bool	             ret;
   T_CNI_LAPDM_L3_ID l3_id=0;	 /* This variable created for CNI API only */
   rm_LapdmOId_t     mOId,aOId;   /* Return from LAPDm-instance creat func */

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_InitLAPDm\n" ));

   /*
   ** Creat a LAPDm instance for each fuctional channel and furnish the RM
   ** internal data organization accordingly. <TBD> : PCH, AGCH, BCCH msgs
   ** will by-pass LAPDm, thus no need to create relevant instances.
   */

   /* Create LAPDm instance for BCCH function channel */
   mOId = CNI_LAPDM_Create_Instance( 
				     CT_BCCH,
				     rm_PhyChanBCcch[RM_BCCH_IDX].trxNumber,
				     rm_PhyChanBCcch[RM_BCCH_IDX].chanNumber,
				     RM_L1LINK_VOID  /*TBD*/
				   );
   if (mOId == CNI_LAPDM_NULL_OID)
       /* Failed to create BCCH LAPDm instance */
       EDEBUG__(("ERROR-rm_InitLAPDm: Failed to create BCCH LAPDm instance\n"));
   else
   {
       rm_PhyChanBCcch[RM_BCCH_IDX].mOId = mOId;

	 /* Register this RACH instance for SAPI0 and SAPI3 */
       ret =  CNI_LAPDM_Instance_Registration (
						mOId,
						RM_SAPI0,
						l3_id,
						CNI_RIL3MD_L3CallBack
					     );
       if (ret==RM_FALSE)
           EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater BCCH instance\n"));
   }

   /* Create LAPDm instance for RACH function channel */
   mOId = CNI_LAPDM_Create_Instance( 
				     CT_RACH,
				     rm_PhyChanBCcch[RM_RACH_IDX].trxNumber,
				     rm_PhyChanBCcch[RM_RACH_IDX].chanNumber,
				     RM_L1LINK_VOID  /*TBD*/
				   );
   if (mOId == CNI_LAPDM_NULL_OID)
       /* Failed to create RACH LAPDm instance */
       EDEBUG__(("ERROR-rm_InitLAPDm: Failed to create RACH LAPDm instance\n"));
   else
   {
       rm_PhyChanBCcch[RM_RACH_IDX].mOId = mOId;

	 /* Register this RACH instance for SAPI0 and SAPI3 */
       ret =  CNI_LAPDM_Instance_Registration (
						mOId,
						RM_SAPI0,
						l3_id,
						CNI_RIL3MD_L3CallBack
					     );
       if (ret==RM_FALSE)
           EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater RACH instance\n"));
   }
 
   
   /* Create LAPDm instance for TCH/F FACCH & SACCH function channels */
   for (i=0;i<OAMrm_MAX_TCHFS;i++)
   {   
//       if (rm_PhyChanTchf[i].usable == RM_PHYCHAN_USABLE)
       {
	   mOId = CNI_LAPDM_Create_Instance( 
					     CT_FACCH_F,	
			                     rm_PhyChanTchf[i].trxNumber,
				             rm_PhyChanTchf[i].chanNumber,
				             RM_L1LINK_MAIN
					   );

           aOId = CNI_LAPDM_Create_Instance( 
				             CT_SACCH,	
				             rm_PhyChanTchf[i].trxNumber,
				             rm_PhyChanTchf[i].chanNumber,
				             RM_L1LINK_ACCH
					   );

	   if ( mOId==CNI_LAPDM_NULL_OID || aOId==CNI_LAPDM_NULL_OID )
              /* Failed to create TCH/F FACCH or SACCH LAPDm instance */
	      EDEBUG__(("ERROR-rm_InitLAPDm: Failed to create TCH/F LAPDm instance:%d\n",i));
           else
           {
 	       /* Register FACCH instance for SAPI0 only */
               ret =  CNI_LAPDM_Instance_Registration (
							mOId,
							RM_SAPI0,
							l3_id,
							CNI_RIL3MD_L3CallBack
	  					     );

               if (ret==RM_FALSE)
                   EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater %d-th FACCH instance\n",
			     i));

 	       /* Register TCHF/SACCH instance for SAPI0 and SAPI3 */
               ret =  CNI_LAPDM_Instance_Registration (
							aOId,
							RM_SAPI0,
							l3_id,
							CNI_RIL3MD_L3CallBack
	  					     );

               if (ret==RM_FALSE)
                   EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater %d-th TCHF/SACCH instance\n"));

 	       /* Register TCHF/SACCH instance for SAPI0 and SAPI3 */
               ret =  CNI_LAPDM_Instance_Registration (
							aOId,
							RM_SAPI3,
							l3_id,
							CNI_RIL3MD_L3CallBack
	  					     );

               if (ret==RM_FALSE)
                   EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater %d-th TCHF/SACCH instance\n"));

	       /* Inter-relate relevant data */
               rm_PhyChanTchf[i].mOId = mOId;
               rm_PhyChanTchf[i].aOId = aOId;

//printf("rm_InitLAPDm: mOId[%d]=%d, mOId=%d\n",i, rm_PhyChanTchf[i].mOId,mOId);
//printf("rm_InitLAPDm: aOId[%d]=%d, aOId=%d\n",i, rm_PhyChanTchf[i].aOId,aOId);

           }

       } /* End of outer if-statement */


   } /* End of i-for-loop: TCH/F */

   /* Create LAPDm instance for SDCCH/4 SDCCH & SACCH func channels */
   for (i=0;i<OAMrm_MAX_SDCCH4S;i++)
   {   
//       if (rm_PhyChanSdcch4[i].usable == RM_PHYCHAN_USABLE)
       {
	   mOId = CNI_LAPDM_Create_Instance( 
					     CT_SDCCH,	
			                     rm_PhyChanSdcch4[i].trxNumber,
				             rm_PhyChanSdcch4[i].chanNumber,
				             RM_L1LINK_MAIN
					   );

           aOId = CNI_LAPDM_Create_Instance( 
				             CT_SACCH,	
				             rm_PhyChanSdcch4[i].trxNumber,
				             rm_PhyChanSdcch4[i].chanNumber,
				             RM_L1LINK_ACCH
					   );

	   if ( mOId==CNI_LAPDM_NULL_OID || aOId==CNI_LAPDM_NULL_OID )
              /* Failed to create TCH/F FACCH or SACCH LAPDm instance */
	      EDEBUG__(("ERROR-rm_InitLAPDm: Failed to create SDCCH4 LAPDm instance:%d\n",i));
           else
           {
 	       /* Register SDCCH4 instance for SAPI0 and SAPI3 */
               ret =  CNI_LAPDM_Instance_Registration (
							mOId,
							RM_SAPI0,
							l3_id,
							CNI_RIL3MD_L3CallBack
	  					     );

               if (ret==RM_FALSE)
                   EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater %d-th SDCCH4 instance\n"));

               ret =  CNI_LAPDM_Instance_Registration (
							mOId,
							RM_SAPI3,
							l3_id,
							CNI_RIL3MD_L3CallBack
	  					     );

               if (ret==RM_FALSE)
                   EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater %d-th SDCCH4 instance\n"));

 	       /* Register SDCCH4/SACHH instance for SAPI0 only */
               ret =  CNI_LAPDM_Instance_Registration (
							aOId,
							RM_SAPI0,
							l3_id,
							CNI_RIL3MD_L3CallBack
	  					     );

               if (ret==RM_FALSE)
                   EDEBUG__(("ERROR-rm_InitLAPDm: Failed to registrater %d-th SDCCH4/SACCH instance\n",
			     i));
	       /* Inter-relate relevant data */
               rm_PhyChanSdcch4[i].mOId = mOId;
               rm_PhyChanSdcch4[i].aOId = aOId;
           }

       } /* End of outer if-statement */

   } /* End of for-i loop: SDCCH4 */

   /* TEST ONLY */
rm_InitLAPDmTestPrint(2,4);

} /* End of rm_InitLAPDm() */

#endif /* __RM_INITLAPDM_CPP__ */
