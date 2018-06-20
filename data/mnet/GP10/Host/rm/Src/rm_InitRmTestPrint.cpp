/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITRMTESTPRINT_CPP__
#define __RM_INITRMTESTPRINT_CPP__

#include "rm\rm_head.h"

void rm_InitRmTestPrint(u8 tchfIdx,u8 sdcch4Idx)
{
   u8	   i;

   /* Print TCHF channel description data */
   for (i=0; i<tchfIdx; i++)
   { 
        DDEBUG__ (("%dth TCHF Chan Descrption\n",i));

        DDEBUG__ (("usable=0x%02x, state=0x%02x, chanType=0x%02x, subChanNo=0x%02x\n",
		    rm_PhyChanTchf[i].usable, 
		    rm_PhyChanTchf[i].state, 
		    rm_PhyChanTchf[i].chanType,
		    rm_PhyChanTchf[i].subChanNumber
	         ));
	DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
		 rm_PhyChanTchf[i].trxNumber,
	         rm_PhyChanTchf[i].chanNumber,
	         rm_PhyChanTchf[i].chanNumberMSB,
  	         rm_PhyChanTchf[i].chanNumberLSB
	       ));
        DDEBUG__ (("TSC=0x%02x,rslCount=0x%02x,quality=%d\n",
	         rm_PhyChanTchf[i].TSC,
	         rm_PhyChanTchf[i].rslCount,
		 rm_PhyChanTchf[i].quality
		));
        DDEBUG__ (("T3101.id=0x%08x,T3101.time=%d, T3101.state=%d\n",
	         rm_PhyChanTchf[i].T3101.id,
	         rm_PhyChanTchf[i].T3101.time,
	     	 rm_PhyChanTchf[i].T3101.state
		));
        DDEBUG__ (("T3111.id=0x%08x,T3111.time=%d, T3111.state=%d\n",
	         rm_PhyChanTchf[i].T3111.id,
	         rm_PhyChanTchf[i].T3111.time,
	     	 rm_PhyChanTchf[i].T3111.state
		));
        DDEBUG__ (("T3111.id=0x%08x,T3111.time=%d, T3109.state=%d\n",
	         rm_PhyChanTchf[i].T3109.id,
	         rm_PhyChanTchf[i].T3109.time,
	     	 rm_PhyChanTchf[i].T3109.state
		));
        DDEBUG__ (("T3111.id=0x%08x,T3L01.time=%d, T3L01.state=%d\n",
	         rm_PhyChanTchf[i].T3L01.id,
	         rm_PhyChanTchf[i].T3L01.time,
	     	 rm_PhyChanTchf[i].T3L01.state
		));
    }

   /* Print SDCCH4 channel description data */
   for (i=0; i<sdcch4Idx; i++)
   { 
        DDEBUG__ (("%dth SDCCH4 Chan Descrption\n",i));
        DDEBUG__ (("usable=0x%02x, state=0x%02x, chanType=0x%02x, subChanNo=0x%02x\n",
	         rm_PhyChanSdcch4[i].usable,
        	 rm_PhyChanSdcch4[i].state,
         	 rm_PhyChanSdcch4[i].chanType,
        	 rm_PhyChanSdcch4[i].subChanNumber
	       ));
	DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
		 rm_PhyChanSdcch4[i].trxNumber,
	         rm_PhyChanSdcch4[i].chanNumber,
	         rm_PhyChanSdcch4[i].chanNumberMSB,
  	         rm_PhyChanSdcch4[i].chanNumberLSB
	       ));
        DDEBUG__ (("TSC=0x%02x,rslCount=0x%02x,quality=%d\n",
	         rm_PhyChanSdcch4[i].TSC,
	         rm_PhyChanSdcch4[i].rslCount,
		 rm_PhyChanSdcch4[i].quality
		));
        DDEBUG__ (("T3101.id=0x%08x,T3101.time=%d, T3101.state=%d\n",
	         rm_PhyChanSdcch4[i].T3101.id,
	         rm_PhyChanSdcch4[i].T3101.time,
	     	 rm_PhyChanSdcch4[i].T3101.state
		));
        DDEBUG__ (("T3111.id=0x%08x,T3111.time=%d, T3111.state=%d\n",
	         rm_PhyChanSdcch4[i].T3111.id,
	         rm_PhyChanSdcch4[i].T3111.time,
	     	 rm_PhyChanSdcch4[i].T3111.state
		));
        DDEBUG__ (("T3111.id=0x%08x,T3111.time=%d, T3109.state=%d\n",
	         rm_PhyChanSdcch4[i].T3109.id,
	         rm_PhyChanSdcch4[i].T3109.time,
	     	 rm_PhyChanSdcch4[i].T3109.state
		));
        DDEBUG__ (("T3111.id=0x%08x,T3L01.time=%d, T3L01.state=%d\n",
	         rm_PhyChanSdcch4[i].T3L01.id,
	         rm_PhyChanSdcch4[i].T3L01.time,
	     	 rm_PhyChanSdcch4[i].T3L01.state
		));
    }

    DDEBUG__ (("BCCH Chan Descrption\n",i));
    DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
             rm_PhyChanBCcch[RM_BCCH_IDX].trxNumber,
             rm_PhyChanBCcch[RM_BCCH_IDX].chanNumber,
	     rm_PhyChanBCcch[RM_BCCH_IDX].chanNumberMSB,
	     rm_PhyChanBCcch[RM_BCCH_IDX].chanNumberLSB
	   ));
	 
    DDEBUG__ (("RACH Chan Descrption\n",i));
    DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
             rm_PhyChanBCcch[RM_RACH_IDX].trxNumber,
	     rm_PhyChanBCcch[RM_RACH_IDX].chanNumber,
	     rm_PhyChanBCcch[RM_RACH_IDX].chanNumberMSB,
	     rm_PhyChanBCcch[RM_RACH_IDX].chanNumberLSB
	   ));
    DDEBUG__ (("RACH Chan Descrption\n",i));
    DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
	     rm_PhyChanBCcch[RM_PCH_IDX].trxNumber,
	     rm_PhyChanBCcch[RM_PCH_IDX].chanNumber,
	     rm_PhyChanBCcch[RM_PCH_IDX].chanNumberMSB,
	     rm_PhyChanBCcch[RM_PCH_IDX].chanNumberLSB
           ));
	      
    DDEBUG__ (("AGCH Chan Descrption\n",i));
    DDEBUG__ (("trx=0x%02x,chanNumber=0x%04x,chanNumberMSB=0x%02x,chanNumberLSB=0x%02x\n",
	     rm_PhyChanBCcch[RM_AGCH_IDX].trxNumber,
	     rm_PhyChanBCcch[RM_AGCH_IDX].chanNumber,
	     rm_PhyChanBCcch[RM_AGCH_IDX].chanNumberMSB,
  	     rm_PhyChanBCcch[RM_AGCH_IDX].chanNumberLSB
	   ));
}

#endif /* __RM_INITRMTESTPRINT_CPP__ */

