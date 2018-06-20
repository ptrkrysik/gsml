/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_HOREQ_CPP__
#define __RM_HOREQ_CPP__

//HOADDED
//!!! TCH/F assumed
#include "rm\rm_head.h"

void rm_HoReq(void)
{
   rm_SmCtxt_t 	  	*pSmCtxt;
   rm_PhyChanActDesc_t  actDesc;

   /* Monitoring entrance to a function */
   RDEBUG__(("ENTER-rm_HoReq\n"));
  
   //Check if the system locked/shutting down
   if (OAMrm_BTS_AMSTATE==locked 	       ||
 	 OAMrm_BTS_AMSTATE==shuttingDown 	 ||
       OAMrm_TRX_AMSTATE(0)==locked        ||
       OAMrm_TRX_AMSTATE(0)==shuttingDown  ||
       OAMrm_CHN_AMSTATE(0,0)==locked      ||
       OAMrm_CHN_AMSTATE(0,0)==shuttingDown )
   {
       PDEBUG__(("INFO@rm_HoReq: System blocked/shutting down(%d,%d,%d) \n",
                  OAMrm_BTS_AMSTATE, OAMrm_TRX_AMSTATE(0),OAMrm_CHN_AMSTATE(0,0)));

       if (!rm_VcIsBarred)
       {
            rm_VcIsBarred = RM_TRUE;
            rm_SendSystemInfo2ToBcchTrx(0,2,CNI_RIL3_CELL_BARRED_ACCESS);        
            rm_SendSystemInfo3ToBcchTrx(0,3,CNI_RIL3_CELL_BARRED_ACCESS);        
            rm_SendSystemInfo4ToBcchTrx(0,4,CNI_RIL3_CELL_BARRED_ACCESS);  
       }

       return;
   }

   //Request for a TCH/F and activate it
   pSmCtxt = RM_SMCTXT_NULL;
   switch(rm_pItcRxMmMsg->l3_data.handReq.channelType.bearerType)
   {
   case BEARER_SPEECH:
        //Handover on-going speech. Allocate current-best TCH/F
        pSmCtxt = rm_AllocSmCtxt(RM_PHYCHAN_TCHF_S);

        if ( RM_SMCTXT_NULL==rm_PreAllocSmCtxt(RM_PHYCHAN_TCHF_S) ) 
        {
            //PM Added:
            if (!rm_AllAvailableTchf.allocated)
            {
                 rm_AllAvailableTchf.allocated = RM_TRUE;
                 PM_CellMeasurement.allAvailableTCHAllocatedTime.start();

                 //rm_AllAvailableTchf.startTime = tickGet();
            }
        }

        //Wirte current used speech version into HO Target Channel structure to solve mismatched speech versions
	  //for Calll Hold after HO
        if (pSmCtxt == RM_SMCTXT_NULL)
        {
            PDEBUG__(("INFO@rm_HoReq: No tchf available and temp. bar this cell\n"));
            //Modify SI2-4 and update DSP,MS as well as MIB for display in ViperWatch
            //OAMrm_RA_CELL_BARRED_STATEa = (Boolean) CNI_RIL3_CELL_BARRED_ACCESS;
            //if (STATUS_OK!=
            //    oam_setMibIntVar(MODULE_RM, MIB_cellBarred, OAMrm_RA_CELL_BARRED_STATEa) )
            //{ 
            //    EDEBUG__(("ERROR@rm_HoReq: OAM set cellBarred failed, state=%d\n",
		//               OAMrm_RA_CELL_BARRED_STATEa));
            //}

            PM_CellMeasurement.attTCHSeizuresMeetingTCHBlockedState.increment(); 

            if (OAMrm_OVERLOAD_CELLBAR)
            {
                //Cell is temporarily barred, not reflected in OAM GUI
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
//        else
//        {
//            pSmCtxt->spchVer = (u8)(rm_pItcRxCcMsg->l3_data.handReq.channelType.speechChann.version);
//            PDEBUG__(("INFO@rm_HoReq: assigned spch version(loc %d, cc %d, u8cc %d) entryId %d\n",
//                       pSmCtxt->spchVer, 
//                       rm_pItcRxCcMsg->l3_data.handReq.channelType.speechChann.version,
//                       (u8)(rm_pItcRxCcMsg->l3_data.handReq.channelType.speechChann.version),
//                       pSmCtxt->mEntryId));
//        }
  	  break;

   case BEARER_DATA:
   case BEARER_SIGNALING:
   default:
 	  //Unexpected channel type in received Handover Request
	  EDEBUG__(("ERROR-rm_HoReq: unsupported chanType:%d\n",
	             rm_pItcRxMmMsg->l3_data.channAssignCmd.channelType.bearerType));
	  return;
   }
   
   //Valid channel type i.e. TCH/F speech. Check if any free TCH/F found
   if ( pSmCtxt == RM_SMCTXT_NULL )
   {
        rm_SendHoReqNAck(rm_pItcRxMmMsg->l3_data.handReq.mmId);
        EDEBUG__(("ERROR-rm_HoReq: No Physical Channel available right now\n"));
   } else
   {
        //Got a free physical channel for this handover request

        rm_ItcTxMsg_t 	    msgToL1;
        u16			    length;
	
        pSmCtxt->codAlgo = rm_BeaerCapVerToChanMode(rm_pItcRxMmMsg->l3_data.handReq.channelType.speechChann.version);

        pSmCtxt->spchVer = (u8)(rm_pItcRxMmMsg->l3_data.handReq.channelType.speechChann.version);

        PDEBUG__(("INFO@rm_HoReq: assigned spch version(loc %d, cc %d, u8cc %d) entryId %d\n",
                       pSmCtxt->spchVer, 
                       rm_pItcRxMmMsg->l3_data.handReq.channelType.speechChann.version,
                       (u8)(rm_pItcRxMmMsg->l3_data.handReq.channelType.speechChann.version),
                       pSmCtxt->mEntryId));

        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
        msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_MSB;
        msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_LSB;
        msgToL1.buffer[length++] = pSmCtxt->pPhyChan->trxNumber;
        msgToL1.buffer[length++] = pSmCtxt->pPhyChan->chanNumberMSB;
        msgToL1.buffer[length++] = pSmCtxt->pPhyChan->chanNumberLSB;

        msgToL1.buffer[length++] = RM_L1ACTTYPE_ASYNC_HO; //actType

        //PR1248 Begin
        msgToL1.buffer[length] = 0;  // DTX ul&dl OFF

        //PR1381 BEGIN
        if ( ((int)OAMrm_MS_UPLINK_DTX_STATE==1) ||
             ((int)OAMrm_MS_UPLINK_DTX_STATE==0)  ) 
               msgToL1.buffer[length] |= 1; //DTX ul ON
        //PR1381 END

        //PR1319
        //if ( !(pSmCtxt->pPhyChan->trxNumber) && OAMrm_MS_DNLINK_DTX_STATE )
        if ( (pSmCtxt->pPhyChan->trxNumber) && OAMrm_MS_DNLINK_DTX_STATE )
              msgToL1.buffer[length] |= 2; //DTX dl ON
        length++;
        //PR1248 End
        
        msgToL1.buffer[length++] = RM_L1DATATTR_SPEECH;   //datAttr
        msgToL1.buffer[length++] = RM_L1RATTYPE_TCHF;     //ratType
//      msgToL1.buffer[length++] = RM_L1CODALGO_GSMV1;    //codAlgo
//EFRAdded
        msgToL1.buffer[length++] = pSmCtxt->codAlgo;

        //msgToL1.buffer[length++] = RM_L1CIPHTYPE_NCIPH;   //ciphSet
        //RM_MEMSET(&msgToL1.buffer[length], RM_L1CIPHKEY_LEN); //ciphKey
        //length = length + RM_L1CIPHKEY_LEN - 1; //TEMP Adjustment

        //Ciphering 
        IntraL3CipherModeCommand_t *p;
        p =(IntraL3CipherModeCommand_t *) &(rm_pItcRxMmMsg->l3_data.handReq.cipherCmd);
        PDEBUG__(("INFO@rm_HandReq: ciphData(%d, set%d,algo%d),ciphResp(%d,imeisv%d)\n",
                   p->cmd.cipherModeSetting.ie_present,
                   p->cmd.cipherModeSetting.ciphering,
                   p->cmd.cipherModeSetting.algorithm,
                   p->cmd.cipherResponse.ie_present,
                   p->cmd.cipherResponse.cipherResponse));

        int len,i;
        len = length;
        if (p->cmd.cipherModeSetting.ciphering == CNI_RIl3_CIPHER_START_CIPHERING)
        {
            //Order algorithm to be used for the ciphering
            msgToL1.buffer[len++] = ((u8)p->cmd.cipherModeSetting.algorithm) + 2;
            //Pass down the ciphering key
            for (i=0;i<8;i++)
                 msgToL1.buffer[len++] = p->Kc[i];
        } else 
        {
            msgToL1.buffer[len++] = 1; //No ciphering applied
        }
        length = length + RM_L1CIPHKEY_LEN; 

        msgToL1.buffer[length++] = rm_pItcRxMmMsg->l3_data.handReq.mmId; //hoRef
        msgToL1.buffer[length++] = 0; //bsPower
        msgToL1.buffer[length++] = OAMrm_MS_TX_PWR_MAX_CCH;  //MS init TX pwr
        msgToL1.buffer[length++] = 0; //TA 
	  msgToL1.buffer[length++] = 0; //UIC
	  msgToL1.msgLength = length;

	  api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);

        //Save handover reference and adjust state machine state
        pSmCtxt->isForHo = RM_TRUE;
        pSmCtxt->pPhyChan->ny1 = 0;	//Reset retry counter of sending Physical info msg
        pSmCtxt->pPhyChan->hoRef = rm_pItcRxMmMsg->l3_data.handReq.mmId;
        pSmCtxt->state = RM_S2ACTIVATING_SIGTRAFFIC_CHAN;

        //PMADDED:attTCHSeizures
        PM_CellMeasurement.attTCHSeizures.increment();

   }

} /* End rm_HoReq() */

#endif /* __RM_HOREQ_CPP__ */
