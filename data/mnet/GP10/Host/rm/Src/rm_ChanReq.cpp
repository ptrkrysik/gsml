/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_CHANREQ_CPP__
#define __RM_CHANREQ_CPP__

#include "rm\rm_head.h"
extern int ta_HoldFacch;

void rm_ChanReq( rm_ChanReq_t *pChanReq )
{
   rm_SmCtxt_t 	  	*pSmCtxt;
   rm_PhyChanActDesc_t  actDesc;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_ChanReq\n"));

   //PMADDED:attImmediateAssingProcs,attImmediateAssingProcsPerCauseTable
   PM_CellMeasurement.attImmediateAssingProcs.increment();
   PM_CellMeasurement.attImmediateAssingProcsPerCauseTable.increment(pChanReq->estCause);

   //Check if the system locked/shutting down
   if (OAMrm_BTS_AMSTATE==locked 	       ||
 	 OAMrm_BTS_AMSTATE==shuttingDown 	 ||
       OAMrm_TRX_AMSTATE(0)==locked        ||
       OAMrm_TRX_AMSTATE(0)==shuttingDown  ||
       OAMrm_CHN_AMSTATE(0,0)==locked      ||
       OAMrm_CHN_AMSTATE(0,0)==shuttingDown )
   {
       PDEBUG__(("INFO@rm_ChanReq: System blocked/shutting down(%d,%d,%d),cause=%d\n",
                  OAMrm_BTS_AMSTATE, OAMrm_TRX_AMSTATE(0),OAMrm_CHN_AMSTATE(0,0),
                  pChanReq->estCause));

       //GP2
       //if (!rm_VcIsBarred)
       //{
       //     rm_VcIsBarred = RM_TRUE;
       //     rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);        
       //     rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);        
       //     rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);  
       //}

       return;
   }

   pSmCtxt = RM_SMCTXT_NULL;
   switch(pChanReq->estCause)
   {
   case RM_CHANREQ_COS_LUP: //0x00
	  //LUP call: allocate currently best SDCCH/4 
	  pSmCtxt = rm_AllocSmCtxt(RM_PHYCHAN_SDCCH4); 
        //pSmCtxt = rm_AllocSmCtxt(RM_PHYCHAN_TCHF_S);

        //PM Added:
        if (RM_SMCTXT_NULL == rm_PreAllocSmCtxt(RM_PHYCHAN_SDCCH4) )
        { 
            if (!rm_AllAvailableSdcch4.allocated)
            {
                 rm_AllAvailableSdcch4.allocated = RM_TRUE;
                 PM_CellMeasurement.allAvailableSDCCHAllocatedTime.start();

                 //rm_AllAvailableSdcch4.startTime = tickGet();
            }
        }

        if (pSmCtxt == RM_SMCTXT_NULL)
            PM_CellMeasurement.attSDCCHSeizuresMeetingSDCCHBlockedState.increment(); 
        
	break;

   case RM_CHANREQ_COS_EMC: //0xA0
   case RM_CHANREQ_COS_MTC: //0x80
   case RM_CHANREQ_COS_MOC: //0xE0
	  //MOC&MTC call: Allocate the best TCH/F channel
        pSmCtxt = rm_AllocSmCtxt(RM_PHYCHAN_TCHF_S);

        if (RM_SMCTXT_NULL == rm_PreAllocSmCtxt(RM_PHYCHAN_TCHF_S) )
        {
            //PM Added:
            if (!rm_AllAvailableTchf.allocated)
            {
                 rm_AllAvailableTchf.allocated = RM_TRUE;
                 PM_CellMeasurement.allAvailableTCHAllocatedTime.start();

                 //rm_AllAvailableTchf.startTime = tickGet();
            }
        } 

        if (pSmCtxt == RM_SMCTXT_NULL)
        {
            PDEBUG__(("INFO@rm_ChanReq: No tchf available and temp. bar this cell\n"));

            PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.increment(); 
                    
            //Modify SI2-4 and update DSP,MS as well as MIB for display in ViperWatch
            //OAMrm_RA_CELL_BARRED_STATEa = (Boolean)CNI_RIL3_CELL_BARRED_ACCESS;
            //if (STATUS_OK!=
            //    oam_setMibIntVar(MODULE_RM, MIB_cellBarred, OAMrm_RA_CELL_BARRED_STATEa) )
            //{ 
            //    EDEBUG__(("ERROR@rm_ChanReq: OAM set cellBarred failed, state=%d\n",
		//               OAMrm_RA_CELL_BARRED_STATEa));
            //}
            if (OAMrm_OVERLOAD_CELLBAR)
            {
                //Cell is temporarily barred, not reflected in MIB thus
                rm_VcIsBarred = RM_TRUE;
                rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);        
                rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);        
                rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);  
            } else
            {
                if (rm_VcIsBarred)
                { 
                    rm_VcIsBarred = RM_FALSE;
                    rm_SendSystemInfo2ToBcchTrx(0,2,OAMrm_RA_CELL_BARRED_STATE);        
                    rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);        
                    rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
                }
            }
      }
	  break;

   default:
  	  //TBD: Process other est causes later
	  EDEBUG__(( "ERROR-rm_ChanReq: unsupported est cause:%d\n",pChanReq->estCause ));
	  return; 

   } //End of switch()

   if (pSmCtxt == RM_SMCTXT_NULL)
   {
        PDEBUG__(("INFO@rm_ChanReq: all channels happen to be in use now\n"));
        rm_SendImmAssignRej((int)pChanReq->estCause);
   }else
   {      
        //Activate the allocated channel for this channel request
        actDesc.actType = RM_L1ACTTYPE_IMMASS;          //Activation type

        //PR1248 Begin
		actDesc.dtxCtrl = 0;					//DTX ul&dl both OFF

        //PR1381 BEGIN
        if ( ((int)OAMrm_MS_UPLINK_DTX_STATE==1) ||
             ((int)OAMrm_MS_UPLINK_DTX_STATE==0)  ) 
             actDesc.dtxCtrl |= 1; 				      //DTX: ul ON
        //PR1381 END

        //PR1319
        //if ( !(pSmCtxt->pPhyChan->trxNumber) && OAMrm_MS_DNLINK_DTX_STATE )
        if ( (pSmCtxt->pPhyChan->trxNumber) && OAMrm_MS_DNLINK_DTX_STATE )
			actDesc.dtxCtrl |= 2;                     //DTX: dl ON
 	  //PR1248 End

        actDesc.datAttr = RM_L1DATATTR_SIGNALING;	  //Chan mode: byte 2
        //actDesc.datAttr = RM_L1DATATTR_SPEECH; 	  //Speech mode
        switch(pChanReq->estCause)
        {
        case RM_CHANREQ_COS_LUP:
             actDesc.ratType = RM_L1RATTYPE_SDCCH;	 //Channel mode:byte 3 
             //actDesc.ratType = RM_L1RATTYPE_TCHF;	 //Channel mode:byte 3 
	       break;

        case RM_CHANREQ_COS_MOC:
             actDesc.ratType = RM_L1RATTYPE_TCHF;	 //Channel mode:byte 3 
             //actDesc.ratType = RM_L1RATTYPE_SDCCH;	 //Channel mode:byte 3 
             break;

        case RM_CHANREQ_COS_EMC: //0xA0
        case RM_CHANREQ_COS_MTC:
             actDesc.ratType = RM_L1RATTYPE_TCHF;	//Channel mode:byte 3 
             //actDesc.ratType = RM_L1RATTYPE_SDCCH;	//Channel mode:byte 3 
	       break;
        }

        actDesc.codAlgo = RM_L1CODALGO_NORES;
        actDesc.ciphSet = RM_L1CIPHTYPE_NCIPH;
        RM_MEMSET(&actDesc.ciphKey, RM_L1CIPHKEY_LEN);
        actDesc.bsPower = RM_L1BSPOWER_PREALPHA;
        actDesc.msPower = RM_L1MSPOWER_PREALPHA;
        actDesc.taValue = rm_pItcRxMdMsg->l3_data.buffer[3]; 
        actDesc.UIC     = 0; 

        rm_SendChanActivation(pSmCtxt->pPhyChan,&actDesc);

        //Save Channel Request info for use by Imm Assign
        pSmCtxt->isForHo  = RM_FALSE; //Tell hoReq from normal chanReq

        pSmCtxt->callType = (u8)(pChanReq->estCause);
        RM_MEMCPY( pSmCtxt->pPhyChan->savChanReq, 
                   rm_pItcRxMdMsg->l3_data.buffer, RM_SAVCHANREQ_LEN );

        //PMADDED: 
        pSmCtxt->pPhyChan->estCause = (u8)pChanReq->estCause;

        //Adjust State Machine state accordingly
        pSmCtxt->state = RM_S2ACTIVATING_SIGTRAFFIC_CHAN;
   }

} //End rm_ChanReq()


void rm_SendImmAssignRej(int cause)
{
   int              i;
   u16		        length;	          //Length of a msg sent to L1
   rm_EdRet_t       rrEdRet;	      //Return code from RR decode
   rm_L3Data_t      l3_data; 	      //Encoded Paging Req Type 1
   rm_ItcTxMsg_t    msgToL1;	      //Send PH_DATA_REQ msg to L1
   T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT_REJECT
                    *pUmImmAssignRej; //Data for encoding PageReq1

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_SendImmAssignRej\n"));

   //Choose Imm Assign message structure
   pUmImmAssignRej =
     (T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT_REJECT *)&(rm_UmMsg.immediateAssignmentReject);
 
   
   //Populate the contents so that encoder knows what to encode but
   //before we start populating the message content, zap it clean
   //first.
   
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   //Header--PD,MT,SI
   pUmImmAssignRej->header.protocol_descriminator = RM_PD;
   pUmImmAssignRej->header.si_ti		          = RM_SI;
   pUmImmAssignRej->header.message_type	          = CNI_RIL3RRM_MSGID_IMMEDIATE_ASSIGNMENT_REJECT;

   //IE--Page Mode
   pUmImmAssignRej->pageMode.ie_present	          = RM_TRUE;
   pUmImmAssignRej->pageMode.pm	 	              = CNI_RIL3_PAGING_SAME_AS_BEFORE;
   
   //IE--Channel Description
   for (i=0;i<4;i++)
   {
       pUmImmAssignRej->reqReference[i].ie_present	     = RM_TRUE;
       pUmImmAssignRej->reqReference[i].randomAccessInfo = rm_pItcRxMdMsg->l3_data.buffer[0]; 
       pUmImmAssignRej->reqReference[i].T1               = (rm_pItcRxMdMsg->l3_data.buffer[1]>>3) & 0x1F;
       pUmImmAssignRej->reqReference[i].T2               = (rm_pItcRxMdMsg->l3_data.buffer[2]   ) & 0x1F;
       pUmImmAssignRej->reqReference[i].T3               = (((rm_pItcRxMdMsg->l3_data.buffer[1]<<3) & 0x38)|
				                                           ((rm_pItcRxMdMsg->l3_data.buffer[2]>>5) & 0x07) );

	   pUmImmAssignRej->waitIndication[i].ie_present	 = RM_TRUE;
       pUmImmAssignRej->waitIndication[i].T3122  	     = 4;  //sec
   }

   //Call RR message encoding functionality
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   //check the encoding result before sending the message
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	
	    //Wrap the encoded Paging message in PH_DATA_REQ
        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_LNKMGMT;
        msgToL1.buffer[length++] = RM_L1MT_PHDATAREQ_MSB;
        msgToL1.buffer[length++] = RM_L1MT_PHDATAREQ_LSB;
 	    msgToL1.buffer[length++] = OAMrm_BCCH_TRX;
        msgToL1.buffer[length++] = RM_L1AGCH_CHANNUMBER_MSB;
        msgToL1.buffer[length++] = 28;
	    msgToL1.buffer[length++] = RM_L1LINK_VOID;

        RM_MEMCPY(&msgToL1.buffer[length], l3_data.buffer, l3_data.msgLength);
        msgToL1.msgLength   = length + l3_data.msgLength;

	    //Send PH_DATA_REQ to L1
        PostL3SendMsLog( &l3_data );
        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
        PDEBUG__(("========rm_SendImmAssignRej: (cause=%x, randomInfo=%x, TA=%d, T1-3(%x,%x)\n", 
                  cause,
                  rm_pItcRxMdMsg->l3_data.buffer[0],
                  rm_pItcRxMdMsg->l3_data.buffer[3],
                  rm_pItcRxMdMsg->l3_data.buffer[1],
                  rm_pItcRxMdMsg->l3_data.buffer[2]
                 ));

        //PMADDED:succImmediateAssingProcs,succImmediateAssingProcsPerCauseTable
        //PM_CellMeasurement.succImmediateAssingProcs.increment();
        //PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.increment(pPhyChan->estCause);

	    break;

   default:
	    //Um message encoding failed
        EDEBUG__(("ERROR-rm_SendImmAssignRej: Um encode error:%d\n", rrEdRet ));
   	    break;

   } //End of switch(rrEdRet)

} //End of rm_SendImmAssignRej()

#endif //__RM_CHANREQ_CPP__
