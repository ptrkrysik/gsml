/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITIRTTESTPRINT_CPP__
#define __RM_INITIRTTESTPRINT_CPP__


#include "rm\rm_head.h"

void rm_InitIrtTestPrint(u8 tchfIdx, u8 sdcch4Idx)
{
   u8	   		i;
   rm_SmCtxt_t 		*pMSmCtxt, *pASmCtxt;
   rm_PhyChanDesc_t	*pPhyChan; 

   /* Print TCHF channel description data */
   for (i=0; i<tchfIdx; i++)
   { 
        DDEBUG__ (("%dth TCHF Chan Descrption: entryIDs and SmCtxtAddrs\n",i));

        DDEBUG__ (("mEntryId=0x%08x, aEntryId=0x%08x, mSmCtxt=0x%08x,aSmCtxt=0x%08x,trx=%x,chan=%x\n",
                    rm_PhyChanTchf[i].mEntryId,
                    rm_PhyChanTchf[i].aEntryId,
			  rm_PhyChanTchf[i].pMSmCtxt,
			  rm_PhyChanTchf[i].pASmCtxt,
			  rm_PhyChanTchf[i].trxNumber,
			  rm_PhyChanTchf[i].chanNumberMSB

	         ));
        DDEBUG__(("Output data from rm_SmCtxt[]::%dth\n", i));
        pMSmCtxt = &rm_SmCtxt[rm_PhyChanTchf[i].mEntryId];
	  pASmCtxt = &rm_SmCtxt[rm_PhyChanTchf[i].aEntryId];
        pPhyChan = pMSmCtxt->pPhyChan;
        DDEBUG__(("mSmCtxt=0x%08x,aSmCtxt=0x%08x,pPhyChan=0x%08x,&phyChan[i]=0x%08x\n",
		       pMSmCtxt, pASmCtxt, pPhyChan, &rm_PhyChanTchf[i]));
        DDEBUG__(("mSm.state=%d, mSm.smType=%d,mSm.mOId=0x%08x,mSm.aOId=0x%08x\n",
	             pMSmCtxt->state,
	             pMSmCtxt->smType,
	             pMSmCtxt->mOId,
	             pMSmCtxt->aOId
		    ));
        DDEBUG__(("aSm.state=%d, aSm.smType=%d,aSm.mOId=0x%08x,aSm.aOId=0x%08x\n",
	             pASmCtxt->state,
	             pASmCtxt->smType,
	             pASmCtxt->mOId,
	             pASmCtxt->aOId
		    ));

        DDEBUG__ (("%dth TCHF Chan Descrption with SmCtxt.pPhyChan\n",i));

        DDEBUG__ (("usable=0x%02x, state=0x%02x, chanType=0x%02x, subChanNo=0x%02x\n",
		        pPhyChan->usable, 
		        pPhyChan->state, 
		        pPhyChan->chanType,
		        pPhyChan->subChanNumber
	         ));
	  DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
		        pPhyChan->trxNumber,
	              pPhyChan->chanNumber,
	              pPhyChan->chanNumberMSB,
  	              pPhyChan->chanNumberLSB
	           ));
        DDEBUG__ (("TSC=0x%02x,rslCount=0x%02x,quality=%d\n",
	              pPhyChan->TSC,
	              pPhyChan->rslCount,
		        pPhyChan->quality
		     ));
        DDEBUG__ (("T3101.id=0x%08x,T3101.time=%d, T3101.state=%d\n",
	              pPhyChan->T3101.id,
	              pPhyChan->T3101.time,
	     	        pPhyChan->T3101.state
		     ));
        DDEBUG__ (("T3111.id=0x%08x,T3111.time=%d, T3111.state=%d\n",
	              pPhyChan->T3111.id,
	              pPhyChan->T3111.time,
	     	        pPhyChan->T3111.state
		     ));
        DDEBUG__ (("T3109.id=0x%08x,T3109.time=%d, T3109.state=%d\n",
	              pPhyChan->T3109.id,
	              pPhyChan->T3109.time,
	     	        pPhyChan->T3109.state
		     )); 
    }

   /* Print SDCCH4 channel description data */
   for (i=0; i<sdcch4Idx; i++)
   { 
        DDEBUG__ (("%dth SDCCH4 Chan Descrption: entryIDs and SmCtxtAddrs\n",i));

        DDEBUG__ (("mEntryId=0x%08x, aEntryId=0x%08x, mSmCtxt=0x%08x,aSmCtxt=0x%08x,trx=%x,chan=%x\n",
                    rm_PhyChanSdcch4[i].mEntryId,
                    rm_PhyChanSdcch4[i].aEntryId,
			  rm_PhyChanSdcch4[i].pMSmCtxt,
			  rm_PhyChanSdcch4[i].pASmCtxt,
			  rm_PhyChanSdcch4[i].trxNumber,
			  rm_PhyChanSdcch4[i].chanNumberMSB

	          ));
        DDEBUG__(("Output data from rm_SmCtxt[]::%dth\n", i));
        pMSmCtxt = &rm_SmCtxt[rm_PhyChanSdcch4[i].mEntryId];
	  pASmCtxt = &rm_SmCtxt[rm_PhyChanSdcch4[i].aEntryId];
        pPhyChan = pMSmCtxt->pPhyChan;
        DDEBUG__(("mSmCtxt=0x%08x,aSmCtxt=0x%08x,pPhyChan=0x%08x,&phyChan[i]=0x%08x\n",
		       pMSmCtxt, pASmCtxt, pPhyChan, &rm_PhyChanSdcch4[i]));
        DDEBUG__(("mSm.state=%d, mSm.smType=%d,mSm.mOId=0x%08x,mSm.aOId=0x%08x\n",
	             pMSmCtxt->state,
	             pMSmCtxt->smType,
	             pMSmCtxt->mOId,
	             pMSmCtxt->aOId
		    ));
        DDEBUG__(("aSm.state=%d, aSm.smType=%d,aSm.mOId=0x%08x,aSm.aOId=0x%08x\n",
	             pASmCtxt->state,
	             pASmCtxt->smType,
	             pASmCtxt->mOId,
	             pASmCtxt->aOId
		    ));

        DDEBUG__ (("%dth SDCCH4 Chan Descrption with SmCtxt.pPhyChan\n",i));

        DDEBUG__ (("usable=0x%02x, state=0x%02x, chanType=0x%02x, subChanNo=0x%02x\n",
		        pPhyChan->usable, 
		        pPhyChan->state, 
		        pPhyChan->chanType,
		        pPhyChan->subChanNumber
	         ));
	  DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
		        pPhyChan->trxNumber,
	              pPhyChan->chanNumber,
	              pPhyChan->chanNumberMSB,
  	              pPhyChan->chanNumberLSB
	           ));
        DDEBUG__ (("TSC=0x%02x,rslCount=0x%02x,quality=%d\n",
	              pPhyChan->TSC,
	              pPhyChan->rslCount,
		        pPhyChan->quality
		     ));
        DDEBUG__ (("T3101.id=0x%08x,T3101.time=%d, T3101.state=%d\n",
	              pPhyChan->T3101.id,
	              pPhyChan->T3101.time,
	     	        pPhyChan->T3101.state
		     ));
        DDEBUG__ (("T3111.id=0x%08x,T3111.time=%d, T3111.state=%d\n",
	              pPhyChan->T3111.id,
	              pPhyChan->T3111.time,
	     	        pPhyChan->T3111.state
		     ));
        DDEBUG__ (("T3109.id=0x%08x,T3109.time=%d, T3109.state=%d\n",
	              pPhyChan->T3109.id,
	              pPhyChan->T3109.time,
	     	        pPhyChan->T3109.state
		     )); 
   }

} /* End of rm_InitIrtTestPrint() */

void rm_PrintTchEntryId(u8 noOfTchfs,u8 noOfSdcch4s)
{
     u8  i,trx,slot,idx;
     T_CNI_IRT_ID entryId;
     printf("TCHF MAPPING TABLE btw TRX/SLOT and entryID\n");
     for (i=0;i<noOfTchfs;i++)
     {
          if (i<7)
          {
  	        trx=0; slot=i;
              idx=RM_TCHF_ENTRYIDX(trx, slot);
              entryId = rm_TchfEntryId[idx];
          } else
          {
  	        trx=1; slot=i-7;
              idx=RM_TCHF_ENTRYIDX(trx, slot);
              entryId = rm_TchfEntryId[idx];
          }
          printf("%dth TCHF: index=%d, entryId=%d\n",i,idx,entryId);
     }
     printf("SDCCH4 MAPPING TABLE btw TRX/SLOT and entryID\n");

     for (i=0;i<noOfSdcch4s;i++)
          printf("%dth SDCCH4: index=%d, entryId=%d\n",i,i,rm_Sdcch4EntryId[i]);
}


#endif /* __RM_INITIRTTESTPRINT_CPP__ */
