/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_INITRM_CPP__
#define __RM_INITRM_CPP__

#include "rm\rm_head.h"

void rm_InitRm(void)
{
   u8	     i,j,k;	    //Used as loop-control or for array index 
   u8	     tchfIdx;   //Index to TCH/F-type channel pool element
   u8      sdcch4Idx; //Index to SDCCH/4 type chan. pool element
   WDOG_ID timerId;   //Used to store timerID ret. from wdCreate

   i=j=k=0; tchfIdx=0; sdcch4Idx=0;

   //Monitoring entrance to a func
   RDEBUG__(("ENTER@rm_InitRm\n"));

   //Clear control and dedicated channel pools clean first
   RM_MEMSET( (u8 *)rm_PhyChanTchf,   sizeof(rm_PhyChanTchf_t)   );
   RM_MEMSET( (u8 *)rm_PhyChanSdcch4, sizeof(rm_PhyChanSdcch4_t) );
   RM_MEMSET( (u8 *)rm_PhyChanBCcch,  sizeof(rm_PhyChanBCcch_t)  );

   //Initialize RM mgmt structs in terms of OAM configuration data
   for (i=0; i<OAMrm_MAX_TRXS; i++ )
   { 
        //Check if ith TRX is existent
        if ( (OAMrm_TRX_ONOFF_MAP)&(1<<i) )
        {  
             for (j = 0; j<OAMrm_SLOTS_PER_TRX; j++)
             {
                  switch( OAMrm_TRX_SLOT_COMB(i,j) )
	            {
	            //case OAMrm_SLOT_COMB_1:
				  default:
	                 //Establish TCH/F physcial channel pool
                       //G2
                       rm_PhyChanTchf[tchfIdx].usable        = RM_PHYCHAN_USABLE; 
	                 rm_PhyChanTchf[tchfIdx].state         = RM_PHYCHAN_FREE;
		           rm_PhyChanTchf[tchfIdx].chanType      = RM_PHYCHAN_TCHF_S;
		           rm_PhyChanTchf[tchfIdx].subChanNumber = 0;	
	                 rm_PhyChanTchf[tchfIdx].trxNumber     = (0x01&i);
	                 rm_PhyChanTchf[tchfIdx].chanNumber    = (0x08|j)<<8;
		           rm_PhyChanTchf[tchfIdx].chanNumberMSB = (0x08|j);
		           rm_PhyChanTchf[tchfIdx].chanNumberLSB = 0;
		           rm_PhyChanTchf[tchfIdx].TSC		 = OAMrm_BTS_TSC;
		           rm_PhyChanTchf[tchfIdx].rslCount	 = 0;

		           if ( (timerId = wdCreate()) == NULL )
                       {
                            rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                            EDEBUG__(("rm_InitRm: error from calling wdCreate() for tchf T3101\n"));
                       }
      	           rm_PhyChanTchf[tchfIdx].T3101.id    = timerId;
		           rm_PhyChanTchf[tchfIdx].T3101.time  = OAMrm_T3101;
     	                 rm_PhyChanTchf[tchfIdx].T3101.state = RM_FALSE;

		           if ( (timerId = wdCreate()) == NULL )
                       {
                            rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                            EDEBUG__(("rm_InitRm: error from calling wdCreate() for tchf T3111\n"));
                       }
      	           rm_PhyChanTchf[tchfIdx].T3111.id    = timerId;
		           rm_PhyChanTchf[tchfIdx].T3111.time  = OAMrm_T3111;
     	                 rm_PhyChanTchf[tchfIdx].T3111.state = RM_FALSE;

		           if ( (timerId = wdCreate()) == NULL )
                       {
                            rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                            EDEBUG__(("rm_InitRm: error from calling wdCreate() for tchf T3109\n"));
                       }
      	           rm_PhyChanTchf[tchfIdx].T3109.id    = timerId;
		           rm_PhyChanTchf[tchfIdx].T3109.time  = OAMrm_T3109;
     	                 rm_PhyChanTchf[tchfIdx].T3109.state = RM_FALSE;

                       //HOADDED
		           if ( (timerId = wdCreate()) == NULL )
                       {
                            rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                            EDEBUG__(("rm_InitRm: error from calling wdCreate() for tchf T3105\n"));
                       }
      	           rm_PhyChanTchf[tchfIdx].T3105.id    = timerId;
		           rm_PhyChanTchf[tchfIdx].T3105.time  = OAMrm_T3105;
     	                 rm_PhyChanTchf[tchfIdx].T3105.state = RM_FALSE;

		           if ( (timerId = wdCreate()) == NULL )
                       {
                            rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                            EDEBUG__(("rm_InitRm: error from calling wdCreate() for tchf T3L01\n"));
                       }
      	           rm_PhyChanTchf[tchfIdx].T3L01.id    = timerId;
            	     rm_PhyChanTchf[tchfIdx].T3L01.time  = OAMrm_T3L01;
      	           rm_PhyChanTchf[tchfIdx].T3L01.state = RM_FALSE;

		           if ( (timerId = wdCreate()) == NULL )
                       {
                            rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                            EDEBUG__(("rm_InitRm: error from calling wdCreate() for tchf T3L02\n"));
                       }
      	           rm_PhyChanTchf[tchfIdx].T3L02.id    = timerId;
		           rm_PhyChanTchf[tchfIdx].T3L02.time  = OAMrm_T3L02;
     	                 rm_PhyChanTchf[tchfIdx].T3L02.state = RM_FALSE;

                       if (tchfIdx>=OAMrm_MAX_TCHFS)
			     {
			         //Make sure no more than wanted comb1 configuration
			         rm_SetRmOpState(EC_RM_TOO_MANY_CHAN_COMB_1_DETECTED);
			         EDEBUG__(("ERROR@rm_InitRm(): tchfIdx=%d, maxAllowed=%d\n", 
						  tchfIdx, OAMrm_MAX_TCHFS));
			     }

			     //Init PhyChanTchf's opState and amState from mib data
                       

                 if ( OAMrm_TRX_SLOT_COMB(i,j) == OAMrm_SLOT_COMB_1 )
				      rm_PhyChanTchf[tchfIdx].amState = OAMrm_CHN_AMSTATE(i,j);
				 else
				 {
					  printf("{\nRM<-MIB: non-TCH combination used for (trx=%d, slot=%d, comb=%d)\n}\n",
				 		      i, j, OAMrm_TRX_SLOT_COMB(i,j));
				      rm_PhyChanTchf[tchfIdx].amState = locked;
				 }

                 rm_PhyChanTchf[tchfIdx].opState = OAMrm_CHN_OPSTATE(i,j);
                 rm_PhyChanTchf[tchfIdx].chanIdx = tchfIdx;
			     rm_PhyChanTchf[tchfIdx].trxNo   = i;
			     rm_PhyChanTchf[tchfIdx].slotNo  = j;

			     // DDEBUG__(("%dth TCHF T3101.id=0x%08x, T3111.id=0x%08x, T3109.id=0x%08x, T3L01.id=0x%08x\n",
                       //	         tchfIdx,
			     //      	   rm_PhyChanTchf[tchfIdx].T3101.id,
			     //      	   rm_PhyChanTchf[tchfIdx].T3111.id,
			     //      	   rm_PhyChanTchf[tchfIdx].T3109.id,
			     //      	   rm_PhyChanTchf[tchfIdx].T3L01.id
			     //         ));

                       //GP2
                       if ( (rm_PhyChanTchf[tchfIdx].opState==opStateEnabled)&&
                            (rm_PhyChanTchf[tchfIdx].amState!=locked) )
                       {
                           //PM counter:nbrOfAvailableTCHs
                           rm_NoOfUsableTchf++;
                           PM_CellMeasurement.nbrOfAvailableTCHs.setValue(rm_NoOfUsableTchf);
                       }

	                 rm_PhyChanTchf[tchfIdx++].quality   = RM_PHYCHAN_QUALITY_PREALPHA;

		           break;

 	            case OAMrm_SLOT_COMB_5:
      	           //Come across conflict in slot combination
                       if ( sdcch4Idx > 0 )
		                EDEBUG__(("ERROR@rm_InitRm: BCCH Configuration conflict\n"));

                       //Establish BCCH & CCCH physical channel pools 
		  
		           //BCCH Channel
	                 rm_PhyChanBCcch[RM_BCCH_IDX].trxNumber     = (0x01&i);
	                 rm_PhyChanBCcch[RM_BCCH_IDX].chanNumber    = (0x80|j)<<8;
		           rm_PhyChanBCcch[RM_BCCH_IDX].chanNumberMSB = (0x80|j);
		           rm_PhyChanBCcch[RM_BCCH_IDX].chanNumberLSB = 0;
	 
			     //RACH Channel
	                 rm_PhyChanBCcch[RM_RACH_IDX].trxNumber     = (0x01&i);
	                 rm_PhyChanBCcch[RM_RACH_IDX].chanNumber    = (0x88|j)<<8;
		           rm_PhyChanBCcch[RM_RACH_IDX].chanNumberMSB = (0x88|j);
		           rm_PhyChanBCcch[RM_RACH_IDX].chanNumberLSB = 0;

                       //PAGE Channel
	                 rm_PhyChanBCcch[RM_PCH_IDX].trxNumber      = (0x01&i);
	                 rm_PhyChanBCcch[RM_PCH_IDX].chanNumber     = (0x90|j)<<8;
		           rm_PhyChanBCcch[RM_PCH_IDX].chanNumberMSB  = (0x90|j);
		           rm_PhyChanBCcch[RM_PCH_IDX].chanNumberLSB  = 0;
                       rm_PhyChanBCcch[RM_PCH_IDX].amState        = OAMrm_CHN_AMSTATE(i,j);
 	                 rm_PhyChanBCcch[RM_PCH_IDX].usable         = RM_PHYCHAN_USABLE;
 	                 rm_PhyChanBCcch[RM_PCH_IDX].state          = RM_PHYCHAN_FREE;

	      
	                 //AGCH Channel
	                 rm_PhyChanBCcch[RM_AGCH_IDX].trxNumber     = (0x01&i);
	                 rm_PhyChanBCcch[RM_AGCH_IDX].chanNumber    = (0x90|j)<<8;
		           rm_PhyChanBCcch[RM_AGCH_IDX].chanNumberMSB = (0x90|j);
		           rm_PhyChanBCcch[RM_AGCH_IDX].chanNumberLSB = 0;
	  
	                 //Establish SDCCH/4 physcial channel pool
	                 for (k=0; k<OAMrm_MAX_SDCCH4S; k++)
	                 {
                          //GP2
                          rm_PhyChanSdcch4[sdcch4Idx].usable        = RM_PHYCHAN_USABLE; 

		              rm_PhyChanSdcch4[sdcch4Idx].state         = RM_PHYCHAN_FREE;
		              rm_PhyChanSdcch4[sdcch4Idx].chanType      = RM_PHYCHAN_SDCCH4;

		              rm_PhyChanSdcch4[sdcch4Idx].subChanNumber = k;
		              rm_PhyChanSdcch4[sdcch4Idx].trxNumber     = (0x01&i);
		              rm_PhyChanSdcch4[sdcch4Idx].chanNumber    = ((0x20|(k<<3))|j)<<8;
		              rm_PhyChanSdcch4[sdcch4Idx].chanNumberMSB = (0x20|(k<<3))|j;
		              rm_PhyChanSdcch4[sdcch4Idx].chanNumberLSB = 0;
			        rm_PhyChanSdcch4[sdcch4Idx].TSC		  = OAMrm_BTS_TSC;
		              rm_PhyChanSdcch4[tchfIdx].rslCount	  = 0;

 		              if ( (timerId = wdCreate()) == NULL )
                          {
                                rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                                EDEBUG__(("rm_InitRm: error from calling wdCreate() for sdcch4 T3101\n"));
                          }
	     	              rm_PhyChanSdcch4[sdcch4Idx].T3101.id    = timerId;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3101.time  = OAMrm_T3101;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3101.state = RM_FALSE;

 		              if ( (timerId = wdCreate()) == NULL )
                          {
                                rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                                EDEBUG__(("rm_InitRm: error from calling wdCreate() for sdcch4 T3111\n"));
                          }
      	              rm_PhyChanSdcch4[sdcch4Idx].T3111.id    = timerId;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3111.time  = OAMrm_T3111;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3111.state = RM_FALSE;

 		              if ( (timerId = wdCreate()) == NULL )
                          {
                                rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                                EDEBUG__(("rm_InitRm: error from calling wdCreate() for sdcch4 T3109\n"));
                          }
	     	              rm_PhyChanSdcch4[sdcch4Idx].T3109.id    = timerId;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3109.time  = OAMrm_T3109;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3109.state = RM_FALSE;

                          //HOADDED
 		              if ( (timerId = wdCreate()) == NULL )
                          {
                                rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                                EDEBUG__(("rm_InitRm: error from calling wdCreate() for sdcch4 T3105\n"));
                          }
      	              rm_PhyChanSdcch4[sdcch4Idx].T3105.id    = timerId;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3105.time  = OAMrm_T3105;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3105.state = RM_FALSE;

 		              if ( (timerId = wdCreate()) == NULL )
                          {
                                rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                                EDEBUG__(("rm_InitRm: error from calling wdCreate() for sdcch4 T3L01\n"));
                          }
      	              rm_PhyChanSdcch4[sdcch4Idx].T3L01.id    = timerId;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3L01.time  = OAMrm_T3L01;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3L01.state = RM_FALSE;

 		              if ( (timerId = wdCreate()) == NULL )
                          {
                                rm_SetRmOpState(EC_RM_OS_CALL_WDCREATE_FAILED);
                                EDEBUG__(("rm_InitRm: error from calling wdCreate() for sdcch4 T3L02\n"));
                          }
      	              rm_PhyChanSdcch4[sdcch4Idx].T3L02.id    = timerId;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3L02.time  = OAMrm_T3L02;
      	              rm_PhyChanSdcch4[sdcch4Idx].T3L02.state = RM_FALSE;

                          //Make sure MIB config is proper
                          if (sdcch4Idx>=OAMrm_MAX_SDCCH4S)
				  {
				      //Something wrong in MIB configuration data
				      rm_SetRmOpState(EC_RM_TWO_CHAN_COMB_5_EXIST);
				      EDEBUG__(("ERROR@rm_InitRm(): sdcch4Idx=%d,maxAllowed=%d\n",
                                         sdcch4Idx,OAMrm_MAX_SDCCH4S));
				  }

			        //Init PhyChanTchf's opState and amState from mib conf data
                          rm_PhyChanSdcch4[sdcch4Idx].amState = OAMrm_CHN_AMSTATE(i,j);
                          rm_PhyChanSdcch4[sdcch4Idx].opState = OAMrm_CHN_OPSTATE(i,j);
                          rm_PhyChanSdcch4[sdcch4Idx].chanIdx = sdcch4Idx;
			        rm_PhyChanSdcch4[sdcch4Idx].trxNo   = i;
			        rm_PhyChanSdcch4[sdcch4Idx].slotNo  = j;

                          // DDEBUG__(("%dth SDCCH4 T3101.id=0x%08x, T3111.id=0x%08x, T3109.id=0x%08x, T3L01.id=0x%08x\n",
                          //	         sdcch4Idx,
                          // 	         rm_PhyChanSdcch4[sdcch4Idx].T3101.id,
                          // 	         rm_PhyChanSdcch4[sdcch4Idx].T3111.id,
                          // 	         rm_PhyChanSdcch4[sdcch4Idx].T3109.id,
                          // 	         rm_PhyChanSdcch4[sdcch4Idx].T3L01.id
                          //      ));

                           //GP2
                           if ( (rm_PhyChanSdcch4[sdcch4Idx].amState!=locked)&&
                                (rm_PhyChanSdcch4[sdcch4Idx].opState==opStateEnabled) )
                           {
                              //PM counter:nbrOfAvailableSDCCHs
                              if (++rm_nbrOfAvailableSDCCHs>4)
                              { 
                                  rm_nbrOfAvailableSDCCHs = 4;
                                  printf("INFO@rm_InitRm: nbrOfAvailableSDCCHs %d\n",rm_nbrOfAvailableSDCCHs);
                              }
                              PM_CellMeasurement.nbrOfAvailableSDCCHs.setValue(rm_nbrOfAvailableSDCCHs);
 
	                   }

 	              	   rm_PhyChanSdcch4[sdcch4Idx++].quality = RM_PHYCHAN_QUALITY_PREALPHA;
                           }

	   	           break;

	            //default:
		           //Unsupported slot combination configuration
                       //GP2
                       //rm_SetRmOpState(EC_RM_CHAN_COMB_UNSUPPORTED);
		        //   UDEBUG__(("UNKNOWN@rm_InitRm: unsupported slot comb:%d", OAMrm_TRX_SLOT_COMB(i,j) ));
		        //   break;

                  } //End of switch()

             } //End of for-j loop

        } //End of if-statement

   } //End of for-i loop

   //Test setting for PreAlpha Release integration
   //rm_PhyChanTchf[0].usable = RM_PHYCHAN_UNUSABLE;
   //rm_PhyChanTchf[1].usable = RM_PHYCHAN_UNUSABLE;

   //Output for integration test 
   //rm_InitRmTestPrint(tchfIdx,sdcch4Idx);

   //PR 1323, Init rm CB msg table, SMSCBC 
   for (i=0;i<SmsCbc_TableRow_MAX;i++)
   {
        rm_CbMsgTable.table[i].state = 0;
        rm_CbMsgTable.table[i].curPageInCurMsg = 1;
   }        
   rm_CbMsgTable.curMsg = 0;
   
} //End of rm_InitRm()

#endif //__RM_INITRM_CPP__
