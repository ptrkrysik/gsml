/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_S5SIGMODEESTABLISHED_CPP__
#define __RM_S5SIGMODEESTABLISHED_CPP__

#include "rm\rm_head.h"

void rm_S5SigModeEstablished( void )
{
   u8	ret;
   rm_LapdmOId_t       		lapdmOId;

   //Monitoring entrance to a function 
   RDEBUG__(("ENTER-rm_S5SigModeEstablished\n"));

   // TBD:
   // Now at dedicated mode, RM monitors all the Measurement Result msgs
   // from L1. L1 sends up this message even a bad receive encounered on
   // the SACCH. This message is used to judge if RSL occurs.
   
   switch(rm_ItcRxOrigin)
   {
   case MODULE_L1:
	  // Check if gotot a Measurement Result msg on SACCH from L1 
        if ( rm_pItcRxL1Msg->l3_data.buffer[0]==RM_L1MG_DEDMGMT      &&  
             rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_MEASRES_MSB  &&
             rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_MEASRES_LSB  )
        {
             //SMS-SAPI3-Begin

             // Got it. Process Measurement Result message. Also report
             // Radio Signaling Link (RSL) broken if judged as such
             //ret = rm_ProcMeasRes();
             //switch (ret)
             //{
	       //case RM_RSLIND:
  		 //     // Radio Signaling Link loss detected 
		 //     rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
		 //     if (rm_pSmCtxt->smType==RM_MAIN_SM)
		 //         lapdmOId = rm_pSmCtxt->mOId;
		 //     else 
		 //         lapdmOId = rm_pSmCtxt->aOId;
             //
             //         //CT_CHN  rm_SendDlRelReq(lapdmOId, RM_SAPI0,       
             //         //                        rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
             //     rm_SendDlRelReq(lapdmOId, RM_SAPI0, RM_LOCAL_RELEASE);
             //  
		 //     if (rm_pSmCtxt->sapi != RM_SAPI0)
             //         //CT_CHN  rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,
             //         //                        rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
             //         rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);
             //  
		 //     rm_SendRrRelInd(RM_RSLIND, rm_ItcRxEntryId);
		 //     rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
             //
		 //     // Adjust SM state to S7 
		 //     rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
	       //     rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	
             //
		 //     break;
             //
             //default:
		 //     // TBD: Unsupported results including HO needed. But
		 //     // HO may not make sense for this case since no talk.
             //     
		 //     UDEBUG__(("UNKNOWN-rm_S5SigModeEstablished:unsupported result:%d\n",
		 //	           ret));
		 //     break;
	       //}

             //Unexpected L1 message 
             UDEBUG__(("WARNING@rm_S5SigModeEstablished:unexpected L1 msg(%d,%d,%d)\n",
                        rm_pItcRxL1Msg->l3_data.buffer[0],
                        rm_pItcRxL1Msg->l3_data.buffer[1],
                        rm_pItcRxL1Msg->l3_data.buffer[2]));

             //SMS-SAPI3-End

        } else
	       // Got an unexpected message from L1
	       EDEBUG__(( "ERROR-rm_S5SigModeEstablished:L1 msg: PD:%x;MT:%x,%x\n",
		             rm_pItcRxL1Msg->l3_data.buffer[0],
		             rm_pItcRxL1Msg->l3_data.buffer[1],
		             rm_pItcRxL1Msg->l3_data.buffer[2] ));
	  break;

   case MODULE_MD:
        //SMS-SAPI3-Begin

	  //Receive MDL error report
	  //if ( rm_pItcRxMdMsg->primitive_type == MDL_ERR_RPT || 
	  //     rm_pItcRxMdMsg->primitive_type == DL_REL_IND   )
        //{ 

	  if ( rm_pItcRxMdMsg->primitive_type == DL_REL_IND )
        { 
        //SMS-SAPI3-End

             if (rm_pSmCtxt->smType==RM_MAIN_SM)
             {
                 //SMS-SAPI3-Begin

	           //lapdmOId = rm_pSmCtxt->mOId;
	           rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
                 //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);
                 rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
                 rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);


                 //CT_CHN rm_SendDlRelReq(lapdmOId, RM_SAPI0,
                 //                       rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(lapdmOId, RM_SAPI0, RM_LOCAL_RELEASE);
        
  	           //if (rm_pSmCtxt->sapi != RM_SAPI0)
                 //    //CT_CHN rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,
                 //    //			          rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
                 //    rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);
        
                 //The following is for safety-guard purpose, thus may be redundancy
                 //rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);

                 //rm_SmCtxt_t *pASmCtxt;
                 //pASmCtxt = rm_pSmCtxt->pPhyChan->pASmCtxt;
                 //pASmCtxt->state = RM_S1RM_RUNNING;

                 //SMS-SAPI3-End

	           rm_StartTimer(rm_ItcRxEntryId, RM_T3109);

                 rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED;
                                        //CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER; 
                                        //CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
        	
	           // Adjust SM state to S7
	           rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
	           rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	

             //SMS-SAPI3-Begin
             }
             //} else 
             //{ 
	  	     //lapdmOId = rm_pSmCtxt->aOId;
               
                 //SMS-SAPI3-Begin

                 //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);
                 //CT_CHN rm_SendDlRelReq(lapdmOId, RM_SAPI0,
                 //				rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(lapdmOId, RM_SAPI3, RM_LOCAL_RELEASE);

                 //SMS-SAPI3-End

	           //rm_pSmCtxt->state           = RM_S1RM_RUNNING;
             //}  
             //SMS-SAPI3-End

	  } else if ( rm_pItcRxMdMsg->primitive_type == DL_DATA_IND)
        {
             rm_ProcMsCiphModeCmplt();
        } else
	       //Unexpected L2 Message received
	       IDEBUG__(("WARNING@rm_S5SigModeEstablished: unexpect L2 msg (%d, %d %d, %d, %d)\n",
		            rm_pItcRxMdMsg->primitive_type,
                        rm_pItcRxMdMsg->lapdm_oid,
                        rm_pItcRxMdMsg->sapi,
                        rm_pItcRxMdMsg->rel_mode,
                        rm_pItcRxMdMsg->est_mode,
                        rm_pItcRxMdMsg->error_cause));
   	  break;

   case MODULE_CC:
	  // Got a message from CC. Only accept a Channel Assign Command 
	  if (rm_pItcRxCcMsg->message_type==INTRA_L3_RR_CHANN_ASSIGN_CMD)
        {
	      rm_pSmCtxt->spchVer = 0xFF; //Invalid speech version
            rm_ProcChanAssignCmd();

            //PMADDED:attTCHSeizures
            PM_CellMeasurement.attTCHSeizures.increment();
        } 
        break;

   case MODULE_MM:
	  if ( rm_pItcRxMmMsg->primitive_type == RR_REL_REQ )
	  {
	       //Release RF resource in terms of MM command
	       rm_SendChanRelease(rm_ItcRxEntryId);
	       rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	       rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
             rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING; //16

	       //Adjust SM state to S7
             rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;
	       rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;

             //PM Added: 
             rm_pSmCtxt->pPhyChan->chanBusy = RM_FALSE;

             if ( rm_pSmCtxt->pPhyChan->chanType==RM_PHYCHAN_SDCCH4)
             { 
                  if (--rm_NoOfBusySdcch4 >= 0)
                      PM_CellMeasurement.meanNbrOfBusySDCCHs.addRecord(rm_NoOfBusySdcch4);
                  else
                      rm_NoOfBusySdcch4 = 0;

                  if (--rm_maxNbrOfBusySDCCHs >= 0)
                      PM_CellMeasurement.maxNbrOfBusySDCCHs.setValue(rm_maxNbrOfBusySDCCHs); // decrement();
                      //PM_CellMeasurement.maxNbrOfBusySDCCHs.addRecord(rm_maxNbrOfBusySDCCHs); // decrement();
                  else
                      rm_maxNbrOfBusySDCCHs = 0;
             }
   	  } 
        else if (rm_pItcRxMmMsg->message_type == INTRA_L3_RR_CIPHER_COMMAND)
        {
             //Initiate Ciphering from MM layer: command both DSP and MS !
             rm_SendCiphModeCmd(rm_pSmCtxt);
             rm_SendEncryptCmdToTrx(rm_pSmCtxt);
        }
	  else  
	       //Unexpected message from MM
	       EDEBUG__(("ERROR@rm_S5SigModeEstablished:MM msgType:%d\n",
		            rm_pItcRxMmMsg->message_type));
	  break;

   default:
	  //Unexpected message orgination
        EDEBUG__(("ERROR-rm_S5SigModeEstablished:wrong msg origination:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } // End of switch()
}

u8 rm_BeaerCapVerToChanMode(T_CNI_RIL3_SPEECH_VERSION ver)
{
   u8 mode;

   if (ver==0) mode = 1;
   else if (ver==2) mode = 0x21;
   else if (ver==1) mode = 0x41;
   else 
   {
        EDEBUG__(("ERROR@rm_BeaerCapVerToChanMode: bearerCap '%d'\n", ver));
        return 1;
   }

   PDEBUG__(("INFO@rm_BeaerCapVerToChanMode: BearerCap '%d'<-->Mode '%x'\n", ver, mode));
   return mode;
}

void rm_ProcChanAssignCmd(void)
{
   rm_PhyChanActDesc_t 		actDesc;

   PDEBUG__(("INFO@rm_ProcChanAssignCmd: entryId=%d,callLeg=%d\n",
              rm_ItcRxEntryId, rm_pItcRxCcMsg->primitive_type));
 
   //if ( rm_pSmCtxt->spchVer ==
   //    (u8)(rm_pItcRxCcMsg->l3_data.channAssignCmd.channelType.speechChann.version))
   //{
   //     //Commanded speech version is the same as before, ack it!
   //	  rm_pSmCtxt->callIdx = (u8)rm_pItcRxCcMsg->primitive_type;
   //     rm_SendChanAssignComplete();
   //     return;
   //}

   if ( rm_pSmCtxt->smType != RM_MAIN_SM )
   {
	  EDEBUG__(("ERROR@rm_ProcChanAssignCmd: ChannAssign KO\n"));
	  return;
   }

   //Check what channel type is assigned from CC
   switch( rm_pItcRxCcMsg->l3_data.channAssignCmd.channelType.bearerType)
   {
   case BEARER_SPEECH:
    	  //Update both DSP and MS with the commanded speech algo version
        //ChnAssign    
	  rm_pSmCtxt->callIdx  = (unsigned char)rm_pItcRxCcMsg->primitive_type;
	  rm_pSmCtxt->spchVer  = (u8)(rm_pItcRxCcMsg->l3_data.channAssignCmd.channelType.speechChann.version);

        //EFRAdded
        rm_pSmCtxt->codAlgo = rm_BeaerCapVerToChanMode(rm_pItcRxCcMsg->l3_data.channAssignCmd.channelType.speechChann.version);

        //PR1248 Begin
		actDesc.dtxCtrl = 0;						  //DTX ul&dl both OFF

        //PR1381 BEGIN
        if ( ((int)OAMrm_MS_UPLINK_DTX_STATE==1) ||
             ((int)OAMrm_MS_UPLINK_DTX_STATE==0)  ) 
             actDesc.dtxCtrl |= 1; 				      //DTX: ul ON
        //PR1381 END

        //PR1319
        //if ( !(rm_pSmCtxt->pPhyChan->trxNumber) && OAMrm_MS_DNLINK_DTX_STATE )
        if ( (rm_pSmCtxt->pPhyChan->trxNumber) && OAMrm_MS_DNLINK_DTX_STATE )
			actDesc.dtxCtrl |= 2;                     //DTX: dl ON
		//PR1248 End

        actDesc.datAttr = RM_L1DATATTR_SPEECH;		//Channel mode:byte 2
        actDesc.ratType = RM_L1RATTYPE_TCHF;	      //Channel mode:byte 3 
        //actDesc.codAlgo = RM_L1CODALGO_GSMV1;		//Channel mode:byte 4			
        //actDesc.codAlgo = OAMrm_GSMCODEC;	 	//Channel mode:byte 4			
 
        //EFRAdded
        actDesc.codAlgo = rm_pSmCtxt->codAlgo;
	  rm_SendModeModify(rm_pSmCtxt->pPhyChan,&actDesc);
		 
	  //Change SM state to S6 Activating Traffic mode
	  rm_pSmCtxt->state = RM_S6ACTIVATING_TRAFFICMODE;
	  break;

   case BEARER_DATA:
   case BEARER_SIGNALING:
   default:
	  //Unexpected channel type in the ChannAssignCmd
	  EDEBUG__(("ERROR@rm_ProcChanAssignCmd: unsupported bearerType:%d\n",
	             rm_pItcRxMmMsg->l3_data.channAssignCmd.channelType.bearerType));
 	  break;
   }
}

void rm_SendCiphModeCmd(rm_SmCtxt_t *pSmCtxt )
{
   rm_EdRet_t       rrEdRet;	            //Return code from RR decode
   rm_L3Data_t      l3_data; 	            //Encoded Paging Req Type 1
   T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMMAND  *pUmCiphModeCmd, *p; 

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_SendCiphModeCmd\n"));

l3_data.msgLength = 0;
l3_data.buffer[0]=0;
l3_data.buffer[1]=0;
l3_data.buffer[2]=0;

   //Ciphering can happen on any channel and SAPIs

   //Choose Cipher Mode Command
   pUmCiphModeCmd = (T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMMAND *) &(rm_UmMsg.cipherModeCommand);
   p = (T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMMAND *)&(rm_pItcRxMmMsg->l3_data.cipherCmd.cmd);

   //Populate the contents so that encoder knows what to encode but
   //before we start populating the message content, zap it clean
   //first.
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   //Header--PD,MT,SI
   pUmCiphModeCmd->header.protocol_descriminator = RM_PD;
   pUmCiphModeCmd->header.si_ti	       = RM_SI;
   pUmCiphModeCmd->header.message_type = CNI_RIL3RRM_MSGID_CIPHERING_MODE_COMMAND;

   //IE--CipherModeSetting
   pUmCiphModeCmd->cipherModeSetting.ie_present = p->cipherModeSetting.ie_present;
   pUmCiphModeCmd->cipherModeSetting.ciphering  = p->cipherModeSetting.ciphering;
   pUmCiphModeCmd->cipherModeSetting.algorithm  = p->cipherModeSetting.algorithm;

   //IE--cipherResponse
   pUmCiphModeCmd->cipherResponse.ie_present = p->cipherResponse.ie_present;
   pUmCiphModeCmd->cipherResponse.cipherResponse = p->cipherResponse.cipherResponse;

   PDEBUG__(("INFO@rm_SendCiphModeCmd:ciphDataUm(%d, set%d,algo%d),ciphRespUm(%d,imeisv%d)\n",
              pUmCiphModeCmd->cipherModeSetting.ie_present,
              pUmCiphModeCmd->cipherModeSetting.ciphering,
              pUmCiphModeCmd->cipherModeSetting.algorithm,
              pUmCiphModeCmd->cipherResponse.ie_present,
              pUmCiphModeCmd->cipherResponse.cipherResponse));
    
   PDEBUG__(("INFO@rm_SendCiphModeCmd:ciphData(%d, set%d,algo%d),ciphResp(%d,imeisv%d)\n",
              p->cipherModeSetting.ie_present,
              p->cipherModeSetting.ciphering,
              p->cipherModeSetting.algorithm,
              p->cipherResponse.ie_present,
              p->cipherResponse.cipherResponse));
  
   //Call RR message encoding functionality
   rrEdRet = CNI_RIL3RRM_Encode(&rm_UmMsg, &l3_data);

   PDEBUG__(("INFO@rm_SendCiphModeCmd:l3_data.buffer(%d,%x,%x,%x)\n",
              l3_data.msgLength,
              l3_data.buffer[0],
              l3_data.buffer[1],
              l3_data.buffer[2]));

   //check the encoding result before sending the message
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	  //Wrap the encoded Paging message in DL_DATA_REQ
        rm_SendDlDataReq(pSmCtxt->mOId, pSmCtxt->sapi, &l3_data);
        PostL3SendMsLog(pSmCtxt->mOId, pSmCtxt->sapi, &l3_data);
        break;

   default:
	  //Um message encoding failed
        EDEBUG__(("ERROR@rm_SendCiphModeCmd: Um encode error:%d\n", rrEdRet ));
   	  break;
   }
} 

void rm_SendEncryptCmdToTrx(rm_SmCtxt_t *pSmCtxt )
{
   int i;
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;
  IntraL3CipherModeCommand_t *p;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_SendEncryptCmdToTrx\n"));

   p =(IntraL3CipherModeCommand_t *) &(rm_pItcRxMmMsg->l3_data.cipherCmd);
   PDEBUG__(("INFO@rm_SendEncryptCmdToTrx:ciphData(%d, set%d,algo%d),ciphResp(%d,imeisv%d)\n",
              p->cmd.cipherModeSetting.ie_present,
              p->cmd.cipherModeSetting.ciphering,
              p->cmd.cipherModeSetting.algorithm,
              p->cmd.cipherResponse.ie_present,
              p->cmd.cipherResponse.cipherResponse));
	
   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_ENCRYPTION_MSB;
   msgToL1.buffer[length++] = RM_L1MT_ENCRYPTION_LSB;
   msgToL1.buffer[length++] = pSmCtxt->pPhyChan->trxNumber;
   msgToL1.buffer[length++] = pSmCtxt->pPhyChan->chanNumberMSB;
   msgToL1.buffer[length++] = pSmCtxt->pPhyChan->chanNumberLSB;

   //Indicate link identifier to which ciphering is applied
   if (pSmCtxt->sapi==RM_SAPI0) 
       msgToL1.buffer[length++] = 0x00; //main signaling
   else if (pSmCtxt->sapi==RM_SAPI3)
       msgToL1.buffer[length++] = 0x40; //asso signaling
   else
   {
       msgToL1.buffer[length++] = 0x00; //main signaling
       EDEBUG__(("ERROR@rm_SendEncryptCmdToTrx: invalid sapi '%d'\n",
                  pSmCtxt->sapi));
   }
  
   //Ciphering 
   if (p->cmd.cipherModeSetting.ciphering == CNI_RIl3_CIPHER_START_CIPHERING)
   {
       //Order algorithm to be used for the ciphering
       msgToL1.buffer[length++] = ((u8)p->cmd.cipherModeSetting.algorithm) + 2;
       //Pass down the ciphering key
       for (i=0;i<8;i++)
            msgToL1.buffer[length++] = p->Kc[i];
   } else 
   {
       msgToL1.buffer[length++] = 1; //No ciphering applied
       IDEBUG__(("INFO@rm_SendEncryptCmdToTrx:OFF ciphering on (%d,%d,%d,%d)\n",
                  pSmCtxt->pPhyChan->trxNumber,
                  pSmCtxt->pPhyChan->chanNumberMSB,
 		      pSmCtxt->pPhyChan->chanNumberLSB,
                  p->cmd.cipherModeSetting.ciphering));
   }
   //db_BDump(0,msgToL1.buffer,23);
   msgToL1.msgLength = length;
   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
}

void rm_ProcMsCiphModeCmplt(void)
{
     int i;
     rm_ItcTxL3Msg_t  msgToMm;//Send inter MM-Rm siganling
     T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMPLETE * pUmCiphCmplt;
     rm_EdRet_t       rrEdRet;


     //Monitoring entry to a function
     RDEBUG__(("ENTER@rm_ProcMsCiphModeCmplt\n"));

     //Decode this L3 message received to see if it is CHN MODE MODIFY ACK
     //PR1104 RR STATUS MESSAGE
     //rrEdRet = CNI_RIL3RRM_Decode( &(rm_pItcRxMdMsg->l3_data), &rm_UmMsg );
     rrEdRet = rm_MsgAnalyze(); 

     if ( rrEdRet != RM_RRED_SUCCESS )
     {
	  EDEBUG__(("ERROR@rm_ProcMsCiphModeCmplt: L2/l3_data decoded KO\n"));
        return;
     }

     //Decoding successful. Further check if got expected message
     if ( rm_UmMsg.header.protocol_descriminator != RM_PD            ||
          rm_UmMsg.header.message_type != CNI_RIL3RRM_MSGID_CIPHERING_MODE_COMPLETE )   
     {
          IDEBUG__(("INFO@rm_ProcMsCiphModeCmplt:unexpected msg PD:%d;MT:%d",
	             rm_UmMsg.header.protocol_descriminator, 
	             rm_UmMsg.header.message_type ));
          rm_SendRrStatus(CNI_RIL3_RR_MESSAGE_TYPE_NOT_COMPATIBLE);
          return;
     }

     // Got expected msg i.e. Ciphering Mode Complete, 
     pUmCiphCmplt = (T_CNI_RIL3RRM_MSG_CIPHER_MODE_COMPLETE *) &(rm_UmMsg.cipherModeComplete);
     msgToMm.module_id      = MODULE_RM;
     msgToMm.entry_id	    = rm_ItcRxEntryId;
     msgToMm.primitive_type = INTRA_L3_DATA;
     msgToMm.message_type   = INTRA_L3_RR_CIPHER_COMPLETE;
     msgToMm.l3_data.cipherCmplt.cmp.imeisv.ie_present = pUmCiphCmplt->imeisv.ie_present;
     if (pUmCiphCmplt->imeisv.ie_present)
     {
         msgToMm.l3_data.cipherCmplt.cmp.imeisv.mobileIdType = pUmCiphCmplt->imeisv.mobileIdType;
         msgToMm.l3_data.cipherCmplt.cmp.imeisv.numDigits   = pUmCiphCmplt->imeisv.numDigits;
         for (i=0;i<16;i++)
              msgToMm.l3_data.cipherCmplt.cmp.imeisv.digits[i]=pUmCiphCmplt->imeisv.digits[i];
         IDEBUG__(("INFO@rm_ProcMsCiphModeCmplt:mobileIdType:%d, numDigits: %d, digits %d%d\n",
                    pUmCiphCmplt->imeisv.mobileIdType,
                    pUmCiphCmplt->imeisv.numDigits,
                    pUmCiphCmplt->imeisv.digits[0],
                    pUmCiphCmplt->imeisv.digits[1]));
     }

     //Send the complete MM Paging Response message out
     api_SendMsgFromRmToMm( sizeof(rm_ItcTxL3Msg_t), (u8*) &msgToMm);
}

#endif //__RM_S5SIGMODEESTABLISHED_CPP__

