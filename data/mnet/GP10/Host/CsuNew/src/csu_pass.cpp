    /*
    *******************************************************************
    **
    ** (c) Copyright Cisco 2000
    ** All Rights Reserved
    **
    ******************************************************************
    */
    #ifndef __CSU_PASS_CPP__
    #define __CSU_PASS_CPP__
    
    #include "CsuNew\csu_head.h"
    //#include "assert.h"
    
    // Create a global DbgOutput for posting logs to ViperLog.  A global is used
    // so that an object does not need to be created each time a function is 
    // called in the RTP data path.
    DbgOutput csu_passDbg;
    
    //play announcement
    int ann(void)
    {
  	//T_CSU_PORT_LIST src;
  	DBG_FUNC("ann()", CSU_LAYER);
        //DBG_ENTER();
  
        for (int j=0;j<CSU_ANN_SRC_PORT_MAX;j++)
        {
             if (csu_AnnSrcPort[j].state==CSU_RESSTATE_USED)
             {
  	
  		  // Pass announcement to all the sinks associated with ANN src
        	  int i;
        	  for (i=0;i<CSU_PEER_PORT_MAX;i++)    
        	  {
        		  if (csu_AnnSrcPort[j].peer[i].type == CSU_RTP_PORT || 
  			      csu_AnnSrcPort[j].peer[i].type == CSU_GSM_PORT )
       			  {
  				  if( csu_AnnSrcPort[j].peer[i].speech_version == 0)//codec FR is 0;
  				  {
        			        csu_PassVoice(&csu_AnnSrcPort[j].peer[i], 	
  					              (unsigned char *)&csu_annbuf_FR[csu_AnnSrcPort[j].annHandler][csu_AnnSrcPort[j].peer[i].ppos],0,0xd0);
  				  } 
  				  if( csu_AnnSrcPort[j].peer[i].speech_version == 2)//codec EFR is 2;
  				  {
  					csu_PassVoice(&csu_AnnSrcPort[j].peer[i], 	
    					  	      (unsigned char *)&csu_annbuf_EFR[csu_AnnSrcPort[j].annHandler][csu_AnnSrcPort[j].peer[i].ppos],0,0xc0);
  				  }
  				  csu_AnnSrcPort[j].peer[i].ppos += ANN_PERIOD;
       				  if (csu_AnnSrcPort[j].peer[i].ppos >8250) 
                                      csu_AnnSrcPort[j].peer[i].ppos=0;
            		  }
       		  } 
      	     }
        }
  	DBG_LEAVE();
  	return TRUE;
    }
  
    Int32 procUlTchFrame(Uint8 *buf)
    {
          Uint8           i,trx,slot,count,c_count,test,cd; //cd added for PR1352
          T_CSU_SRC_PEER  *peer;
          T_CSU_PORT_ID   src;
          T_CSU_RESULT    ret;
    
          //csu_passDbg.Enter();
    
          //csu_TakeSemaphore();
    
          trx  = buf[CSU_GSM_SPEECH_MSG_TRX_POS]&0x01;
          slot = buf[CSU_GSM_SPEECH_MSG_SLOT_POS]&0x7;
    
    	  //Get ANN signal from L1
          if ( trx==0 && slot==0 ) 
          {
             csu_passDbg.Trace("trx=0 and slot=0 in received speech frame\n");
			
			 //Update global timestamp for RTP frames
			 csu_TimeStamp += CSU_RTP_SPEECH_DAT_RAT;

  	     //Play announcement
  	     ann();
             return (CSU_RESULT_SUCCESS);
          }
    
          src.portType = CSU_GSM_PORT;
          src.portId.gsmHandler = (trx<<8)|slot;  //entryId vs. trx/slot
    
          //Check if it's leftover from last codec setting
          if ( !rm_CodecMatch(src.portId.gsmHandler, buf[CSU_GSM_SPEECH_DAT_POS]) )
               return CSU_RESULT_FAILED;
    
          //PR1352 BEGIN
          else
          {
               cd = buf[CSU_GSM_SPEECH_DAT_POS] & 0xF0;
               if ((cd != 0xd0) && (cd != 0xc0))
               {
                    csu_passDbg.Trace("CSU-procUlTchFrame: invalide codec val(%x,%x)\n",
                                       cd, buf[CSU_GSM_SPEECH_DAT_POS]);
                    return CSU_RESULT_FAILED;
               }
          }
          //PR1352 END
  
          // Retrieve sinks connected with this gsm source port
          test = CSU_TEST_FUNC_NULL;
          if (CSU_RESULT_SUCCESS != (ret=csu_TblGetSnk(&src, &test, &count, &peer)))
          {
              if (++csu_GsmSrcPort[trx][slot].nopr>=CSU_TRAFFIC_FLOW_6_SEC_COUNT)
              {
                  csu_passDbg.Trace("FAILED in getting sink for gsm src (%x,%d,%d)\n",
                                     src.portId.gsmHandler, ret, 
                                     csu_GsmSrcPort[trx][slot].nopr);
                  csu_GsmSrcPort[trx][slot].nopr=0;
              }
              //csu_GiveSemaphore();
              return CSU_RESULT_FAILED;
          }
    
          // Record or play speech file according to relevant setting
          if (test != CSU_TEST_FUNC_NULL)
          {
              switch(test)
              {
              case CSU_TEST_RECORD_SRC_SPEECH_TO_FILE:
    	         csu_RecdSpeechToBuffer(&buf[CSU_GSM_SPEECH_DAT_POS]);
                   break;
              case CSU_TEST_LOOPBACK_SPEECH_TO_SRC_SELF:
                   csu_LoopbackSpeechToSrcSelf(&src,&buf[CSU_GSM_SPEECH_DAT_POS]);
                   break;
              default:
                   csu_passDbg.Trace("Invalid test function selection:%d\n",test);
                   break;
              }
    //          csu_GiveSemaphore();
     	    return CSU_RESULT_SUCCESS;
          }
    
          // Pass voice data to all the sinks associated with src
          c_count=0;
          for (i=0;i<CSU_PEER_PORT_MAX;i++)    
          {
               if (peer[i].type != CSU_NIL_PORT)
               {
                   //PR1352: cd added
                   csu_PassVoice(&peer[i], &buf[CSU_GSM_SPEECH_DAT_POS],0, cd);
                   c_count++;
               }
          } 
    
          if (c_count!=count)
          {
              csu_passDbg.Trace("CSU internal constraint:count=%d, c_count=%d\n",count,c_count);
    //          csu_GiveSemaphore();
              return CSU_RESULT_FAILED;
          }
    
    //      csu_GiveSemaphore();
          return CSU_RESULT_SUCCESS;
    }
    
    int csu_TestSkipSendDsp=0;
    int csu_TestRetAfterJcRtpRead=0;
    int csu_TestSkipPassVoice=0;
    
    //void H323RTPEventHandler(HRTPSESSION rtpHandler, H323CALL hCall, T_CNI_IRT_ID gsmHandler)
    void RTPFastReadHandler(HJCRTPSESSION rtpHandler, unsigned char *buf,
                                  int leng, rtpParam param,VOIP_CALL_HANDLE CallHandle,T_CNI_IRT_ID MobileHandle)
    {
         Uint8           i, count, c_count, test;
    //     Int8            buf[CSU_RTP_SPEECH_MSG_LEN];
         T_CSU_SRC_PEER  *peer;
         T_CSU_PORT_ID   src;
         T_CSU_RESULT    ret;
    
         //csu_passDbg.Enter();
    
         // msecs record arrival time of incoming RTP payload frames. Ideally
         // payload frames from each traffic are supposed to come every 20 ms.
    
    //     csu_TakeSemaphore();
    
    #if 0
         Uint32          msecs;
         msecs = 1000*tickGet()/sysClkRateGet(); //NOTE: needed for local time fed here
         if( ERROR == (leng=jcRtpReadEx(rtpHandler,buf,CSU_RTP_SPEECH_MSG_LEN,8*msecs, &param)) )
         {
             csu_passDbg.Error("FAILED in rtpReadEx/jcRtpReadEx: rtpHandler=%d, leng=%d\n",
                        rtpHandler,leng);
    //         csu_GiveSemaphore();
             return;
         }
    #endif
    
         // Retrieve sinks connected with this rtp source port
         src.portType = CSU_RTP_PORT;
         src.portId.rtpHandler = rtpHandler;
         ret = csu_TblGetSnk(&src, &test, &count, &peer);
         if (ret!= CSU_RESULT_SUCCESS)
         {
             //csu_passDbg.Error("FAILED in getting sinks for rtpHandler '%x': err=%d,leng=%d\n",
             //                   src.portId.rtpHandler, ret, leng);
             //csu_GiveSemaphore();
             return;
         }
    
    #if 0
    //printf("INFO-CSU-TRACE: port=%x, seqn=%d\n", rtpHandler, param.sequenceNumber);
    
         // Showing occurrence of reiceiving data from a given src RTP port
         //TESTADDED
         for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
         {
              if (csu_RtpSrcPort[i].state==CSU_RESSTATE_USED &&
                  csu_RtpSrcPort[i].rtpHandler==(HRTPSESSION)rtpHandler )
              { 
    //            if (csu_RtpSrcPort[i].prnt==CSU_TEST_PRINT_SRC_HANDLER_USED)
    //            {
    //                printf("RTPEventHandler:Got data from RTP port:0x%08x,%08d\n",
    //                        rtpHandler,csu_RtpSrcPort[i].wcnt);
    //                csu_RtpSrcPort[i].wcnt++;
    //            }
    
    
    //            //TESTADDED for checking if any combined rtp frames
    //            if (param.len>45)
    //                printf("CSU-INFO-LEN: port=0x08x,seqn=%d,leng=%d\n",
    //                        rtpHandler,param.sequenceNumber,param.len);
    
                  //TESTADDED for checking if packets come in sequence
                  if ((csu_RtpSrcPort[i].seqn+1) != param.sequenceNumber)
                  {
                      csu_RtpSrcPort[i].seqc++;
    			FILE *fp;
                      fp=fdopen(1,"wr");
    			fflush(fp);
    //                  printf("CSU-INFO-OOS: port=0x%08x, pseq=0x%X, cseq=0x%X, toos=%d\n",
    //                          rtpHandler,
    //                          csu_RtpSrcPort[i].seqn, 
    //                          param.sequenceNumber,
    //                          csu_RtpSrcPort[i].seqc);
                   //csu_RtpSrcPort[i].seqn = param.sequenceNumber;
                   } 
                   csu_RtpSrcPort[i].seqn = param.sequenceNumber;
         
    	         break;
              }
         }
     
         if (i>=CSU_RTP_SRC_PORT_MAX)
         {
              printf(" RTP port:0x%08x not engaged in any call\n", rtpHandler);
    //          csu_GiveSemaphore();
              return;
         }
         
         // Record or play speech file according to relevant setting
         if (test != CSU_TEST_FUNC_NULL)
         {
             switch(test)
             {
             case CSU_TEST_RECORD_SRC_SPEECH_TO_FILE:
                  csu_RecdSpeechToBuffer((Uint8*)&buf[CSU_RTP_SPEECH_DAT_POS]);
                  break;
             case CSU_TEST_LOOPBACK_SPEECH_TO_SRC_SELF:
                  csu_LoopbackSpeechToSrcSelf(&src,
                                              (Uint8*)&buf[CSU_RTP_SPEECH_DAT_POS]);
                  break;
             default:
                  csu_passDbg.Error("Invalid test function selection:%d\n",test);
                  break;
             }
    //         csu_GiveSemaphore();
             return;
         }
    #endif 
    
         Uint8 cd;
         if (param.payload == CSU_RTP_SPEECH_DAT_PAYLOADefr)
             cd = 0xc0;
         else //if (param.payload == CSU_RTP_SPEECH_DAT_PAYLOAD)
             cd = 0xd0;

         // Pass voice data to all the sinks associated with src
         c_count=0;
         for (i=0;i<CSU_PEER_PORT_MAX;i++)    
         {
              if (peer[i].type != CSU_NIL_PORT)
              {
    //printf("INFO-CSU-TRACE: port=%x, seqn=%d\n", rtpHandler, param.sequenceNumber);
    
                  csu_PassVoice(&peer[i], (Uint8*)&buf[CSU_RTP_SPEECH_DAT_POS],
                                param.sequenceNumber,cd); //PR1352: 0 added
                  c_count++;
              }
         } 
    
         if (c_count!=count||c_count==0)
             csu_passDbg.Error("CSU internal constraint:count=%d, c_count=%d\n",count,c_count);
    
    //     csu_GiveSemaphore();
    }
    
    RTP_READ_STATUS RTPEventHandler( HJCRTPSESSION rtpHandler, VOIP_CALL_HANDLE VoipCallHandle, TXN_ID entryId)
    
    {
         Uint8           i, count, c_count, test, cd;
         Int8            buf[CSU_RTP_SPEECH_MSG_LEN];
         Uint32          msecs;
         rtpParam        param;
         T_CSU_SRC_PEER  *peer;
         T_CSU_PORT_ID   src;
         T_CSU_RESULT    ret;
         int		   leng;
    
         //csu_passDbg.Enter();
    
         // msecs record arrival time of incoming RTP payload frames. Ideally
         // payload frames from each traffic are supposed to come every 20 ms.
    
    //     csu_TakeSemaphore();
    
         msecs = 1000*tickGet()/sysClkRateGet(); //NOTE: needed for local time fed here
         if( ERROR == (leng=jcRtpReadEx(rtpHandler,buf,CSU_RTP_SPEECH_MSG_LEN,8*msecs, &param)) )
         {
             csu_passDbg.Error("FAILED in rtpReadEx/jcRtpReadEx: rtpHandler=%d, leng=%d\n",
                        rtpHandler,leng);
    //         csu_GiveSemaphore();
             return RTP_READ_STATUS_READ_FAILED;
         }

         if (param.payload == CSU_RTP_SPEECH_DAT_PAYLOADefr)
             cd = 0xc0;
         else //if (param.payload == CSU_RTP_SPEECH_DAT_PAYLOAD)
             cd = 0xd0;
    
    if (csu_TestRetAfterJcRtpRead)
         return RTP_READ_STATUS_OK;
    
         // Retrieve sinks connected with this rtp source port
         src.portType = CSU_RTP_PORT;
         src.portId.rtpHandler = rtpHandler;
         ret = csu_TblGetSnk(&src, &test, &count, &peer);
         if (ret!= CSU_RESULT_SUCCESS)
         {
             //assert(0); 
             //csu_passDbg.Error("FAILED in getting sinks for rtpHandler '%x': err=%d,leng=%d\n",
             //                    src.portId.rtpHandler, ret, leng);
             //csu_GiveSemaphore();
             return RTP_READ_STATUS_NOT_CONNECTED;
         }
    
    #if 0
    //printf("INFO-CSU-TRACE: port=%x, seqn=%d\n", rtpHandler, param.sequenceNumber);
    
         // Showing occurrence of reiceiving data from a given src RTP port
         //TESTADDED
         for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
         {
              if (csu_RtpSrcPort[i].state==CSU_RESSTATE_USED &&
                  csu_RtpSrcPort[i].rtpHandler==(HRTPSESSION)rtpHandler )
              { 
    //            if (csu_RtpSrcPort[i].prnt==CSU_TEST_PRINT_SRC_HANDLER_USED)
    //            {
    //                printf("RTPEventHandler:Got data from RTP port:0x%08x,%08d\n",
    //                        rtpHandler,csu_RtpSrcPort[i].wcnt);
    //                csu_RtpSrcPort[i].wcnt++;
    //            }
    
    
    //            //TESTADDED for checking if any combined rtp frames
    //            if (param.len>45)
    //                printf("CSU-INFO-LEN: port=0x08x,seqn=%d,leng=%d\n",
    //                        rtpHandler,param.sequenceNumber,param.len);
    
                  //TESTADDED for checking if packets come in sequence
                  if ((csu_RtpSrcPort[i].seqn+1) != param.sequenceNumber)
                  {
                      csu_RtpSrcPort[i].seqc++;
    			FILE *fp;
                      fp=fdopen(1,"wr");
    			fflush(fp);
    //                  printf("CSU-INFO-OOS: port=0x%08x, pseq=0x%X, cseq=0x%X, toos=%d\n",
    //                          rtpHandler,
    //                          csu_RtpSrcPort[i].seqn, 
    //                          param.sequenceNumber,
    //                          csu_RtpSrcPort[i].seqc);
                   //csu_RtpSrcPort[i].seqn = param.sequenceNumber;
                   } 
                   csu_RtpSrcPort[i].seqn = param.sequenceNumber;
         
    	         break;
              }
         }
    
         if (i>=CSU_RTP_SRC_PORT_MAX)
         {
    //        printf(" RTP port:0x%08x not engaged in any call\n", rtpHandler);
    //        csu_GiveSemaphore();
              return;
         }
         
         // Record or play speech file according to relevant setting
         if (test != CSU_TEST_FUNC_NULL)
         {
             switch(test)
             {
             case CSU_TEST_RECORD_SRC_SPEECH_TO_FILE:
                  csu_RecdSpeechToBuffer((Uint8*)&buf[CSU_RTP_SPEECH_DAT_POS]);
                  break;
             case CSU_TEST_LOOPBACK_SPEECH_TO_SRC_SELF:
                  csu_LoopbackSpeechToSrcSelf(&src,
                                              (Uint8*)&buf[CSU_RTP_SPEECH_DAT_POS]);
                  break;
             default:
                  csu_passDbg.Error("Invalid test function selection:%d\n",test);
                  break;
             }
    //         csu_GiveSemaphore();
             return;
         }
    #endif 
    
         // Pass voice data to all the sinks associated with src
         c_count=0;
         for (i=0;i<CSU_PEER_PORT_MAX;i++)    
         {
              if (peer[i].type != CSU_NIL_PORT)
              {
    //printf("INFO-CSU-TRACE: port=%x, seqn=%d\n", rtpHandler, param.sequenceNumber);
    if (!csu_TestSkipPassVoice)
                  csu_PassVoice(&peer[i], (Uint8*)&buf[CSU_RTP_SPEECH_DAT_POS],
                                 param.sequenceNumber, cd); //PR1352: cd added
                  c_count++;
              }
         } 
    
         if (c_count!=count||c_count==0)
             csu_passDbg.Error("CSU internal constraint:count=%d, c_count=%d\n",count,c_count);
    
    //     csu_GiveSemaphore();
         
         return RTP_READ_STATUS_OK;
    }
    

    //PR1352: cd added
    T_CSU_RESULT csu_PassVoice(T_CSU_SRC_PEER *peer, Uint8 *buf, Uint16 seqNumber,Uint8 cd)
    {
          Int32     len;
          Uint8     msg[CSU_MAX_SPEECH_MSG_LEN];	 
          T_CSU_SNK_PORT *snk;
    //      Uint8     *pBuf;
    
          //csu_passDbg.Enter();
         
          switch(peer->type)
          {
          case CSU_GSM_PORT:
               //Pass voice over to a GSM sink port
               Uint8 trx, slot;
               trx  = (Uint8)(peer->pidx>>8);
               slot = (Uint8)(peer->pidx);
               len = 4;		
               msg[len++] = CSU_GSM_SPEECH_MSG_MD;
               msg[len++] = CSU_GSM_SPEECH_DLMSG_MSB;
               msg[len++] = CSU_GSM_SPEECH_DLMSG_LSB;
               msg[len++] = trx;						//Trx Num
               msg[len++] = CSU_GSM_BM_ACCH_CBITS|slot;			//Chan MSB
               msg[len++] = 0;						//Chan LSB
               msg[len++] = (Uint8)seqNumber;				//Reserved
               msg[len++] = (Uint8)(seqNumber>>8);			//Reserved
    
               snk = &csu_GsmSnkPort[trx][slot];
    
               //Check if play recorded speech to peer or real talk in buf
               if (snk->test == CSU_TEST_PLAY_SPEECH_FROM_FILE_TO_SNK)
               {
                   memcpy(&msg[len],&csu_playbuf[snk->ppos],CSU_GSM_SPEECH_DAT_LEN);
                   snk->ppos += CSU_REC_SPEECH_DAT_LEN; 
                   if (snk->ppos>=26400) snk->ppos = 0;           
               } else
                   memcpy(&msg[len],buf,CSU_GSM_SPEECH_DAT_LEN);		   
     
               len += CSU_GSM_SPEECH_DAT_LEN;						
    
               //Go back to fill the length in 1st 4 bytes in little-endian format 
               msg[0] = (Uint8)len;
               msg[1] = (Uint8)(len>>8);
               msg[2] = (Uint8)(len>>16);
               msg[3] = (Uint8)(len>>24);
        
               //Send to L1Proxy by calling L1Proxy callback function
               sendDsp(msg,(len+4));
    //TESTADDED
               if (snk->prnt == CSU_TEST_PRINT_SNK_HANDLER_USED)
               {
                   printf("csu_PassVoice:Write to GSM port:0x%02x,%08d\n",snk->gsmHandler,snk->wcnt);
                   snk->wcnt++;
               }
    
               break;
     
          case CSU_RTP_PORT:
               //Pass voice over to a RTP sink port
               rtpParam  param;
               Uint8     pBuf[100];
               
               if (peer->pidx > CSU_RTP_SNK_PORT_MAX)
               {
                   csu_passDbg.Error("Invalid peer->pidx\n", peer->pidx);
                   return CSU_RESULT_FAILED;
               }
               
               snk = &csu_RtpSnkPort[peer->pidx];   
               csu_TblGetRtpParam(peer,&param,cd); //PR1352: cd added
    
               //pBuf = GetPacketBufferFromFreePool();
    
               //Check if play recorded speech to peer or real talk in buf
               if (snk->test == CSU_TEST_PLAY_SPEECH_FROM_FILE_TO_SNK)
               {
                   //Play recorded speech to the snk
                   memcpy(&pBuf[CSU_RTP_SPEECH_DAT_POS],&csu_playbuf[snk->ppos],
                           CSU_RTP_SPEECH_DAT_LEN);
                   snk->ppos += CSU_REC_SPEECH_DAT_LEN; 
                   if (snk->ppos>=26400) snk->ppos = 0;           
               } else
                          {       
                   //Play real talk to the snk
                   memcpy(&pBuf[CSU_RTP_SPEECH_DAT_POS],buf,CSU_RTP_SPEECH_DAT_LEN);		   
               }
               
               //PR1352: BEGIN
               if   (0xd0==cd) len = CSU_RTP_SPEECH_MSG_LEN;
               else            len = CSU_RTP_SPEECH_MSG_LENefr;
               if (rtpWrite(snk->rtpHandler, pBuf, len, &param)<0)
               //PR1352: END
               {
                   csu_passDbg.Error("FAILED in calling rtpWrite/jcRtpWrite: rtpHandler=%x\n",
                              csu_RtpSnkPort[peer->pidx].rtpHandler);
                   return CSU_RESULT_FAILED;
               }
               
    //TESTADDED
               if (snk->prnt == CSU_TEST_PRINT_SNK_HANDLER_USED)
               {
                   printf("csu_PassVoice:Write to RTP port:0x%08x,%08d\n",snk->rtpHandler,snk->wcnt);
                   snk->wcnt++;
               }
    
               break;        
              
          default: 
               csu_passDbg.Error("Invalid sink port: type=%d\n",peer->type);
               return CSU_RESULT_FAILED;
          }
    
          return CSU_RESULT_SUCCESS;
    }
    
    //Get parameters for rtpWrite from src RTP table

    T_CSU_RESULT csu_TblGetRtpParam(T_CSU_SRC_PEER *peer, rtpParam *param, Uint8 cd)
    {
         //Mark start of MS->RTP speech 
         if ( peer->start )
         {
              param->marker = 1;
              peer->start   = 0;
         } 
         else
    	  param->marker = 0;
    
    //DECIDEPT:     
    //   param->payload   = CSU_RTP_SPEECH_DAT_PAYLOAD;
         if   (0xd0==cd) param->payload = CSU_RTP_SPEECH_DAT_PAYLOAD;
         else            param->payload = CSU_RTP_SPEECH_DAT_PAYLOADefr;
    //     param->payload   = peer->ldty;
    
         param->sByte     = CSU_RTP_SPEECH_DAT_PTSBYTE;
         
		 //Use global timestamp instead to match GW DSP dynamic jitter buffer design
		 //param->timestamp = peer->tick;
         //peer->tick      += CSU_RTP_SPEECH_DAT_RAT;

		 param->timestamp = csu_TimeStamp;
       
         return CSU_RESULT_SUCCESS;
    }
    
    
    T_CSU_RESULT csu_TblGetSnk(T_CSU_PORT_ID *src, Uint8 *test, 
                               Uint8 *count, T_CSU_SRC_PEER **peer)
    {
         Uint8 i,trx,slot;
    
         //csu_passDbg.Error();
    
         switch (src->portType)
         {
         case CSU_GSM_PORT:
              trx  = (Uint8)(src->portId.gsmHandler>>8);
              slot = (Uint8)(src->portId.gsmHandler);
    
              if (csu_GsmSrcPort[trx][slot].state != CSU_RESSTATE_USED ||
                  csu_GsmSrcPort[trx][slot].count == 0)
              {
    	        //csu_passDbg.Error("GSM src port (%d, %x) not engaged: state=%d, count=%d\n",
                  //                   src->portType, src->portId.gsmHandler,
    		  // 	                 csu_GsmSrcPort[trx][slot].state,
                  //                   csu_GsmSrcPort[trx][slot].count);
                  return CSU_RESULT_NOT_RESSTATE_USED;
              }
              *test  = csu_GsmSrcPort[trx][slot].test;
              *count = csu_GsmSrcPort[trx][slot].count;
              *peer  = csu_GsmSrcPort[trx][slot].peer;
              break;
    
         case CSU_RTP_PORT:
              for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
              {
    		   if (csu_RtpSrcPort[i].state != CSU_RESSTATE_NULL &&
                       csu_RtpSrcPort[i].rtpHandler == src->portId.rtpHandler)
                   {
                       if (csu_RtpSrcPort[i].state == CSU_RESSTATE_USED &&
                           csu_RtpSrcPort[i].count != 0)
                       {
                           *test  = csu_RtpSrcPort[i].test;
     		           *count = csu_RtpSrcPort[i].count;
                           *peer  = csu_RtpSrcPort[i].peer;
                           break;
                       } else
                       {
                           if (++csu_RtpSrcPort[i].nopr>=CSU_TRAFFIC_FLOW_6_SEC_COUNT)
                           {
                               csu_passDbg.Trace("FAILED in getting sink for rtp src (%x,%d,%d)\n",
                                     csu_RtpSrcPort[i].rtpHandler,
                                     CSU_RESULT_NOT_RESSTATE_USED,
                                     csu_RtpSrcPort[i].nopr);
                               csu_RtpSrcPort[i].nopr=0;
                               //csu_GiveSemaphore();
                           }
                           return CSU_RESULT_NOT_RESSTATE_USED;
                       }
                   }
              }
              if (i>=CSU_RTP_SRC_PORT_MAX)
              {
                  if (++csu_RtpSrcPort[i].nopr>=CSU_TRAFFIC_FLOW_6_SEC_COUNT)
                  {
                      csu_passDbg.Trace("FAILED in getting sink for rtp src (%x,%d,%d)\n",
                                         csu_RtpSrcPort[i].rtpHandler,
                                         CSU_RESULT_NOT_RESSTATE_USED,
                                         csu_RtpSrcPort[i].nopr);
                      csu_RtpSrcPort[i].nopr=0;
                  }
                  return CSU_RESULT_RTP_HANDLER_NOT_FOUND;
              }
              break;
              
         case CSU_ANN_PORT:
  	    for (i=0;i<CSU_ANN_SRC_PORT_MAX;i++)
            {
  		   if (csu_AnnSrcPort[i].state != CSU_RESSTATE_NULL &&
                     csu_AnnSrcPort[i].annHandler == src->portId.annHandler)
                 {
                     if (csu_AnnSrcPort[i].state == CSU_RESSTATE_USED &&
                         csu_AnnSrcPort[i].count != 0)
                     {
                         *test  = csu_AnnSrcPort[i].test;
   		           *count = csu_AnnSrcPort[i].count;
                         *peer  = csu_AnnSrcPort[i].peer;
                         break;
                     } else
                     {
                         if (++csu_AnnSrcPort[i].nopr>=CSU_TRAFFIC_FLOW_6_SEC_COUNT)
                         {
                             csu_passDbg.Trace("FAILED in getting sink for Ann src (%x,%d,%d)\n",
                                   csu_AnnSrcPort[i].annHandler,
                                   CSU_RESULT_NOT_RESSTATE_USED,
                                   csu_AnnSrcPort[i].nopr);
                             csu_AnnSrcPort[i].nopr=0;
                             //csu_GiveSemaphore();
                         }
                         return CSU_RESULT_NOT_RESSTATE_USED;
                     }
                 }
            }
            if (i>=CSU_ANN_SRC_PORT_MAX)
            {
                if (++csu_AnnSrcPort[i].nopr>=CSU_TRAFFIC_FLOW_6_SEC_COUNT)
                {
                    csu_passDbg.Trace("FAILED in getting sink for Ann src (%x,%d,%d)\n",
                                       csu_AnnSrcPort[i].annHandler,
                                       CSU_RESULT_NOT_RESSTATE_USED,
                                       csu_AnnSrcPort[i].nopr);
                    csu_AnnSrcPort[i].nopr=0;
                }
                return  CSU_RESULT_NOT_RESSTATE_USED;
            }
            break;
  
         default:
              //csu_passDbg.Error("Src port type unsupported: type=%d, id=%x\n",
    	      //                   src->portType, src->portId.rtpHandler);
    	    return CSU_RESULT_INVALID_PORT_TYPE;
         }
    
         return CSU_RESULT_SUCCESS;
    }
    
    #endif /*__CSU_PASS_CPP__*/
    
