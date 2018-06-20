/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_ALLOCSMCTXT_CPP__
#define __RM_ALLOCSMCTXT_CPP__                

#include "rm\rm_head.h"

static u8 	 tch_pos=0;

rm_SmCtxt_t *rm_AllocSmCtxt(u8 chanType)
{
   u8	       i, iChan, quality;
   rm_SmCtxt_t *pSmCtxt;
   static u8 sdcch4_pos=0;

   //Monitoring entrance to a function 
   RDEBUG__(("ENTER-rm_AllocateSmCtxt\n"));

   //Check if the system locked/shutting down
   if (OAMrm_BTS_AMSTATE==locked 	       ||
 	 OAMrm_BTS_AMSTATE==shuttingDown 	 ||
       OAMrm_TRX_AMSTATE(0)==locked        ||
       OAMrm_TRX_AMSTATE(0)==shuttingDown  ||
       OAMrm_CHN_AMSTATE(0,0)==locked      ||
       OAMrm_CHN_AMSTATE(0,0)==shuttingDown )
   {
       IDEBUG__(("WARNING@rm_AllocSmCtxt:system blocked/shutting down(%d,%d,%d),chnType=%d\n",
                  OAMrm_BTS_AMSTATE, OAMrm_TRX_AMSTATE(0),OAMrm_CHN_AMSTATE(0,0),chanType));
       return RM_SMCTXT_NULL;
   }

   //Proceed to allocate the best channel if there is!
   quality = 0; iChan = 0xFF; pSmCtxt = RM_SMCTXT_NULL; 

   switch(chanType)
   {
   case RM_PHYCHAN_TCHF_S:
        //Find the best-quality channel to allocate 
        for (i=0; i<OAMrm_MAX_TCHFS; i++)
 	  {
             //Check if the concerned radio TS is blocked/shutting down
             if (rm_PhyChanTchf[tch_pos].amState == unlocked         &&
                 rm_PhyChanTchf[tch_pos].opState == opStateEnabled   &&
	           rm_PhyChanTchf[tch_pos].usable == RM_PHYCHAN_USABLE &&
		     rm_PhyChanTchf[tch_pos].state  == RM_PHYCHAN_FREE    )

             {   
	           if (rm_PhyChanTchf[tch_pos].quality > quality)
		     {
			   iChan = tch_pos;
                     quality = rm_PhyChanTchf[tch_pos].quality;
                 }
	       }
             tch_pos++;
             if (tch_pos>=OAMrm_MAX_TCHFS) tch_pos=0;
 	  }

        if ( iChan != 0xFF )
	  { 
	       //Got expected TCH/F channel
	       rm_PhyChanTchf[iChan].state  = RM_PHYCHAN_ASSIGNED;
	       pSmCtxt = rm_PhyChanTchf[iChan].pMSmCtxt;

             //DSPADDED
             tch_pos = iChan+1;
             if (tch_pos>=OAMrm_MAX_TCHFS) tch_pos=0;

             //PMADDED:attTCHSeizuresMeetingTCHBlockedState,attTCHSeizures
             //        meanNbrOfIdleTCHsPerInterferenceBand
             rm_NoOfIdleTchf--;
	       rm_PhyChanTchf[iChan].chanBusy  = RM_FALSE;
             //PM_CellMeasurement.attTCHSeizures.increment();
             PM_CellMeasurement.meanNbrOfIdleTCHsPerInterferenceBand.addRecord(rm_NoOfIdleTchf);
	  }
        //else
        //{
        //     rm_AllAvailableTchf.allocated = RM_TRUE;
        //     rm_AllAvailableTchf.startTime = tickGet();
        //     //PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.increment(); 
        //}

	  break;		
   
   case RM_PHYCHAN_SDCCH4:
	  //Find the best-quality channel to allocate
        
        for (i=0; i<OAMrm_MAX_SDCCH4S; i++)
   	  {
             if (rm_PhyChanSdcch4[sdcch4_pos].amState == unlocked          &&
                 rm_PhyChanSdcch4[sdcch4_pos].opState == opStateEnabled    &&
	           rm_PhyChanSdcch4[sdcch4_pos].usable  == RM_PHYCHAN_USABLE &&
		     rm_PhyChanSdcch4[sdcch4_pos].state   == RM_PHYCHAN_FREE   )
             {
	            if (rm_PhyChanSdcch4[sdcch4_pos].quality > quality)
		      {
			    iChan = sdcch4_pos;
                      quality = rm_PhyChanSdcch4[sdcch4_pos].quality;
                  }
	       }
             sdcch4_pos++;
             if (sdcch4_pos>=OAMrm_MAX_SDCCH4S) sdcch4_pos=0;
        }

        if ( iChan != 0xFF )
	  { 
	       //Got expected SDCCH4 channel
	       rm_PhyChanSdcch4[iChan].state  = RM_PHYCHAN_ASSIGNED;
	       pSmCtxt = rm_PhyChanSdcch4[iChan].pMSmCtxt;
             //DSPADDED
             sdcch4_pos = iChan+1;
             if (sdcch4_pos>=OAMrm_MAX_SDCCH4S) sdcch4_pos=0;

             //PMADDED:attSDCCHSeizuresMeetingSDCCHBlockedState
             rm_PhyChanSdcch4[iChan].chanBusy = RM_FALSE;
	  } 
        //else
        //{
        //     rm_AllAvailableSdcch4.allocated = RM_TRUE;
        //     rm_AllAvailableSdcch4.startTime = tickGet();
        //     PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.increment(); 
        //}

	  break;		

   default:
	  EDEBUG__(("ERROR-rm_AllocSmCtxt: unsupported chan type:%d\n",
		       chanType));
	  break;

   } //End of switch()

   return (pSmCtxt);

} //End of rm_AllocSmCtxt()

void intg_SetNextAssignment(u8 trx, u8 slot)
{
     u8 i, chan;

     chan = 0x08|slot;
     
     for (i=0; i<OAMrm_MAX_TCHFS; i++)
     {
          if ( rm_PhyChanTchf[i].trxNumber == trx     &&
               rm_PhyChanTchf[i].chanNumberMSB == chan )
          {
               tch_pos = i;
	         if ( rm_PhyChanTchf[tch_pos].usable  != RM_PHYCHAN_USABLE  || 
		        rm_PhyChanTchf[tch_pos].state   != RM_PHYCHAN_FREE    ||
                    rm_PhyChanTchf[tch_pos].amState != unlocked           || 
		        rm_PhyChanTchf[tch_pos].opState != opStateEnabled     )

               {
                    printf("TCHF chan (%d,%d) blocked or in use now usable=%d,state=%d\n",
                            trx, slot,rm_PhyChanTchf[tch_pos].usable,
                            rm_PhyChanTchf[tch_pos].state );
               } else
               {
                    printf("TCHF Chan (%d,%d) will be assigned to next TCHF chan req\n",
                    trx, slot);
               }
	    }
     }
}

#endif //__RM_ALLOCSMCTXT_CPP__
