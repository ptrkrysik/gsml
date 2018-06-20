/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* file name: frmsync.c              
*                       
* Description: synchronize burst processing with GSM frame number
*
*                                                                
* Remark: If called only on a super frame period, the calculations could
*         be simplified dramatically
*
* Warning: It is better to use the function only at super frame cycle(T2=T3=0)
*
*
*****************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "math.h"
#include "dsprotyp.h"

/*****************************************************************************
*
* Function: tsFrameSync
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void tsFrameSync(Uint8 rfCh, Uint8 timeSlot)
{                        
   Int32 temp;

   t_TNInfo *ts;

   t_BBInfo *bbInfo;

   Int32 dlSacchIdle;
   Int32 ulSacchIdle;



   /*
   * get to right RF carrier
   */
   bbInfo = & g_BBInfo[rfCh];

   /*
   * find right time slot
   */
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
 			                                  
   /*
   *  Check is the time slot need to be configured;
   */ 			                                  
   if(ts->tsConfig != TRUE) return;
        

		
   switch(ts->groupNum)
			{
		 case GSM_GROUP_1:
    		/*
    		* GSM Group I related indexes
	    	*/
		    bbInfo->frame.dlG1Index = 
			   (sysFrameNum.FN + bbInfo->frame.offset) % T2_FRAMES;

		    bbInfo->frame.ulG1Index = 
			   (sysFrameNum.FN + bbInfo->frame.offset +
			     T2_FRAMES - g_frameCounts.ulFNOffset ) % T2_FRAMES;
		    /*
		    * SACCH or Idle selection related to Time Slot number
		    */
    	    dlSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset) % (T2_FRAMES*4);
		    dlSacchIdle = dlSacchIdle / (T2_FRAMES/2);
                                     
			
			   /*
			   * calculate frame process indexes
			   */
			   ts->dlTblIndex = bbInfo->frame.dlG1Index; 
				
			   /*
			   * T2 + offset ???
			   */                                   
			   if( timeSlot < 8-g_frameCounts.TNNumInit)
						{
				      ts->ulTblIndex = bbInfo->frame.ulG1Index;
          ulSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset + 
			           			g_frameCounts.ulFNOffset) % (T2_FRAMES*4);
		    }
			   else
						{
          ts->ulTblIndex = (bbInfo->frame.ulG1Index+T2_FRAMES-1) % T2_FRAMES;
         	ulSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset +  T2_FRAMES*4-1 +
			      	g_frameCounts.ulFNOffset) % (T2_FRAMES*4);
		    }
      ulSacchIdle = ulSacchIdle / (T2_FRAMES/2);
      ts->u.group1.dlIndex = (8+dlSacchIdle - timeSlot) & 0x07;

			   ts->u.group1.ulIndex = (8+ulSacchIdle - timeSlot) & 0x07;

			   break;

   case GSM_GROUP_5:
			   /*
			   * GSM Group V downlink related indexes
			   */
			   bbInfo->frame.dlG5Index = 
		   	   	(sysFrameNum.FN + bbInfo->frame.offset) % T3_FRAMES;
			
			   /*
		   	* calculate downlink frame process indexes
			   */
   			ts->dlTblIndex = bbInfo->frame.dlG5Index;

     	/*
     	*  calculate PAGCH index
     	*/
      temp = (sysFrameNum.FN + bbInfo->frame.offset) / T3_FRAMES;

			   temp = temp % (g_BBInfo[rfCh].BSPaMfrms);


      if(sysFrameNum.t3 < 6) bbInfo->frame.dlG5PagchSubCh = temp * PAGE_BLOCKS_G5;
      else if(sysFrameNum.t3 < 12) bbInfo->frame.dlG5PagchSubCh = (temp * PAGE_BLOCKS_G5) + 1;
      else bbInfo->frame.dlG5PagchSubCh = (temp * PAGE_BLOCKS_G5) + 2;

      /*
      * calculate SYS INFO selection: (TN/51) Mod 8
      */
			   temp = ((sysFrameNum.FN + bbInfo->frame.offset)/(T3_FRAMES)) & 7;
                   ts->u.group5.dlSmscbTB = temp;
			   /*
			   *  check if BCCH in current multiframe has been sent
			   */
      if(sysFrameNum.t3 > 5) temp = (temp+1) & 7;
        		
      ts->u.group5.bcchBufIndex = temp;
        		
      /*
      * copy dl logical sub channel sig buffer pointer
      */        		      
			   ts->u.group5.dlSdcchSubCh = bbInfo->frame.dlG5SdcchSubCh;
			   ts->u.group5.dlSacchSubCh = bbInfo->frame.dlG5SacchSubCh;
			   ts->u.group5.dlPpchSubCh = bbInfo->frame.dlG5PagchSubCh;

			   /*
			   * calculate uplink frame process indexes
			   */
			   if( timeSlot < 8-g_frameCounts.TNNumInit)
						{
				     /*
				     * for time slot <  g_frameCounts.TNNumInit, uplink frame number
				     * equals to downlink frame number
				     */
				     ts->ulTblIndex = ts->dlTblIndex;
						}
			   else                                 
						{      
				     /*
				     * for time slot >=  g_frameCounts.TNNumInit, uplink frame number
				     * equals to downlink frame number - 1
				     */				
         ts->ulTblIndex = (ts->dlTblIndex + T3_FRAMES - 1) % T3_FRAMES;
						}
        
 		   /*
			   * calculate current Type V Uplink SDCCH sub-channel
			   */
			   if(ts->ulTblIndex < 4) ts->u.group5.ulSdcchSubCh = 3;
			   else if(ts->ulTblIndex < 41)ts->u.group5.ulSdcchSubCh = 0;
			   else if(ts->ulTblIndex < 45) ts->u.group5.ulSdcchSubCh = 1;
			   else ts->u.group5.ulSdcchSubCh = 2;
      /*
			   * calculate uplink SACCH process indexes
			   */
			   if( timeSlot < 8-g_frameCounts.TNNumInit)
			  	   temp = (sysFrameNum.FN + bbInfo->frame.offset) % (T3_FRAMES*2);
			   else
         temp = (sysFrameNum.FN + bbInfo->frame.offset + T3_FRAMES*2-1) % (T3_FRAMES*2);
        
			   if(temp < 46) ts->u.group5.ulSacchSubCh = 2;
			   else if(temp < 50) ts->u.group5.ulSacchSubCh = 3;
			   else if(temp < 97) ts->u.group5.ulSacchSubCh = 0;
			   else ts->u.group5.ulSacchSubCh = 1;
			   break; 


   case GSM_GROUP_7:
                          
		           /*
			   * GSM Group VII downlink related indexes
			   */

			   ts->dlTblIndex = 
		   	   	(sysFrameNum.FN + bbInfo->frame.offset) % T3_FRAMES;
			
			 		 
               temp = ts->dlTblIndex >>2;
			   
			   if (temp >7) 
				   temp = 0;
  
      /*
      * copy dl logical sub channel sig buffer pointer
      */        		      
			   ts->u.group7.dlSdcchSubCh = temp;

			   if ((ts->dlTblIndex -32) > 0){

                   temp = (ts->dlTblIndex)>>2;
				   if (temp >3){ temp = 0;}

			   } else {

				   temp = 0;
			   }
			   ts->u.group7.dlSacchSubCh = temp;


			   /*
			   * calculate uplink frame process indexes
			   */
			   if( timeSlot < 8-g_frameCounts.TNNumInit)
						{
				     /*
				     * for time slot <  g_frameCounts.TNNumInit, uplink frame number
				     * equals to downlink frame number
				     */
				     ts->ulTblIndex = ts->dlTblIndex;
						}
			   else                                 
						{      
				     /*
				     * for time slot >=  g_frameCounts.TNNumInit, uplink frame number
				     * equals to downlink frame number - 1
				     */
				
                                     ts->ulTblIndex = (ts->dlTblIndex + T3_FRAMES -1) % T3_FRAMES;
									
						}
        
 		   /*
			   * calculate current Type VII Uplink SDCCH sub-channel
			   */

			   if (ts->ulTblIndex >= 51)
			     temp = ts->ulTblIndex - 51;
			   else
			     temp = ts->ulTblIndex;



			   if(temp < 19) ts->u.group7.ulSdcchSubCh = 0;
			   else if (temp  <  23) ts->u.group7.ulSdcchSubCh = 1;
			   else if (temp  <  27) ts->u.group7.ulSdcchSubCh = 2;
                           else if (temp  <  31) ts->u.group7.ulSdcchSubCh = 3;
			   else if (temp  <  35) ts->u.group7.ulSdcchSubCh = 4;
                           else if (temp  <  39) ts->u.group7.ulSdcchSubCh = 5;
                           else if (temp  <  43) ts->u.group7.ulSdcchSubCh = 6;
			   else if (temp <47) ts->u.group7.ulSdcchSubCh = 7;
				   else ts->u.group7.ulSdcchSubCh = 0;
      /*
			   * calculate uplink SACCH process indexes
			   */
			   if( timeSlot < 8-g_frameCounts.TNNumInit)
			  	   temp = (sysFrameNum.FN + bbInfo->frame.offset) % (T3_FRAMES*2);
			   else
                                   temp = (sysFrameNum.FN + bbInfo->frame.offset + T3_FRAMES*2-1) % (T3_FRAMES*2);
        
			   if(temp < 4) ts->u.group7.ulSacchSubCh = 5;
                           else if(temp < 8) ts->u.group7.ulSacchSubCh = 6;
			   else if(temp < 12) ts->u.group7.ulSacchSubCh = 7;
			   else if(temp < 51) ts->u.group7.ulSacchSubCh = 0;
                           else if(temp < 55) ts->u.group7.ulSacchSubCh = 1;
                           else if(temp < 59) ts->u.group7.ulSacchSubCh = 2;
			   else if(temp < 63)  ts->u.group7.ulSacchSubCh = 3;
			   else ts->u.group7.ulSacchSubCh = 4;
			   

	                  break;

   case GSM_GROUP_11:
  /*
      * calculate frame process indexes,
      * For Group11 the Function table size is 13
      */
      ts->dlTblIndex = (sysFrameNum.FN + bbInfo->frame.offset) % (GPRS_MULTI_FRAMES);
      temp   = (sysFrameNum.FN + bbInfo->frame.offset) % (GPRS_MULTI_FRAMES/4);


      
      /*
      *  DL PTCCH function table size is 8
      */                
      ts->u.group11.dlPtcchIdleIndex = ((sysFrameNum.FN + bbInfo->frame.offset) / (GPRS_MULTI_FRAMES/4)) & 0x07;

      /* Notice +3 in first term since DL increments block number in dlPdchProc0 = first frame of block */
      ts->u.group11.dlBlock  = (temp +3)/4 + 
       (((sysFrameNum.FN + bbInfo->frame.offset) % GPRS_MULTI_FRAMES) / (GPRS_MULTI_FRAMES/4)) * 3;  /* result between 0-11 */
      

      if(ts->u.group11.dlBlock >= BLOCKS_PER_GPRS_MULTI_FRAMES)  ts->u.group11.dlBlock = 0;
      /*
      * T2 + offset ???
      */                                   
      if( timeSlot < 8-g_frameCounts.TNNumInit)
      {
         ulSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset +\
                 	g_frameCounts.ulFNOffset) % (GPRS_MULTI_FRAMES*8);
      }
      else
      { 
         ulSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset +  GPRS_MULTI_FRAMES*8-1 +
			               	    g_frameCounts.ulFNOffset) % (GPRS_MULTI_FRAMES*8);			            	    
      }

		    
      ts->ulTblIndex         = ulSacchIdle % (GPRS_MULTI_FRAMES);
      temp                   = ulSacchIdle % (GPRS_MULTI_FRAMES/4);
      ts->u.group11.ulBlock  = temp/4 + ((ulSacchIdle % GPRS_MULTI_FRAMES)/(GPRS_MULTI_FRAMES/4)) * 3;
      
         /* result between 0-11 */  
      if(ts->u.group11.ulBlock >= BLOCKS_PER_GPRS_MULTI_FRAMES)  ts->u.group11.ulBlock = 0;

      /*
       *  Timing Advance Index(Channel) 0-15
       B0(12,38,64,90), B2(116,142,168,194), B3(220,246,272,298), B4(324,350,376,402)
      */       			
      ts->u.group11.currentTAI = (ulSacchIdle + 13)/ (GPRS_MULTI_FRAMES/2);
            
      /*
      *  UL PTCCH function table size is 2
      */ 

      ts->u.group11.ulPtcchIdleIndex =  (ulSacchIdle / (GPRS_MULTI_FRAMES/4)) & 0x01;

      /*
      *  Notify MAC the buffer availability of different logical channels on this time slot
      */ 
      rtsCheck(rfCh, timeSlot, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group11.pdtch.dlSigBuf->frameCount));
      rtsCheck(rfCh, timeSlot, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group11.pacch.dlSigBuf->numMsgsReady));
      rtsCheck(rfCh, timeSlot, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group11.ptcch.dlSigBuf->numMsgsReady));
      
      break;			
                                
   case GSM_GROUP_13:
  /*
      * calculate frame process indexes,
      * For Group13 the Function table size is 13
      */
      ts->dlTblIndex = (sysFrameNum.FN + bbInfo->frame.offset) % (GPRS_MULTI_FRAMES/4); 
      /*
      *  DL PTCCH function table size is 8
      */                
      ts->u.group13.dlPtcchIdleIndex = ((sysFrameNum.FN + bbInfo->frame.offset) / (GPRS_MULTI_FRAMES/4)) & 0x07;

      /* Notice +3 in first term since DL increments block number in dlPdchProc0 = first frame of block */
      ts->u.group13.dlBlock  = (ts->dlTblIndex+3)/4 + 
       (((sysFrameNum.FN + bbInfo->frame.offset) % GPRS_MULTI_FRAMES) / (GPRS_MULTI_FRAMES/4)) * 3;  /* result between 0-11 */ 			

      if(ts->u.group13.dlBlock >= BLOCKS_PER_GPRS_MULTI_FRAMES)  ts->u.group13.dlBlock = 0;
      /*
      * T2 + offset ???
      */                                   
      if( timeSlot < 8-g_frameCounts.TNNumInit)
      {   
         /*
           ts->ulTblIndex = (sysFrameNum.FN + bbInfo->frame.offset +
			                GPRS_MULTI_FRAMES/4 - g_frameCounts.ulFNOffset ) % (GPRS_MULTI_FRAMES/4);
         */			                          
         ulSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset + 
			                 			g_frameCounts.ulFNOffset) % (GPRS_MULTI_FRAMES*8);               		      
      }
      else
      {                     
         /*
            ts->ulTblIndex = (sysFrameNum.FN + bbInfo->frame.offset +
			                GPRS_MULTI_FRAMES/4 - g_frameCounts.ulFNOffset - 1) % (GPRS_MULTI_FRAMES/4);

         */ 
         ulSacchIdle =  (sysFrameNum.FN + bbInfo->frame.offset +  GPRS_MULTI_FRAMES*8-1 +
			               	    g_frameCounts.ulFNOffset) % (GPRS_MULTI_FRAMES*8);			            	    
      }

		    
      ts->ulTblIndex = ulSacchIdle % (GPRS_MULTI_FRAMES/4);
      ts->u.group13.ulBlock  = ts->ulTblIndex/4 + ((ulSacchIdle % GPRS_MULTI_FRAMES)/(GPRS_MULTI_FRAMES/4)) * 3;  /* result between 0-11 */  
      if(ts->u.group13.ulBlock >= BLOCKS_PER_GPRS_MULTI_FRAMES)  ts->u.group13.ulBlock = 0;


			   /*
		   	*  Timing Advance Index(Channel) 0-15
			      B0(12,38,64,90), B2(116,142,168,194), B3(220,246,272,298), B4(324,350,376,402)
		   	*/       			
      ts->u.group13.currentTAI = (ulSacchIdle + 13)/ (GPRS_MULTI_FRAMES/2);
            
      /*
      *  UL PTCCH function table size is 2
      */ 
			   ts->u.group13.ulPtcchIdleIndex =  (ulSacchIdle / (GPRS_MULTI_FRAMES/4)) & 0x01;

      /*
      *  Notify MAC the buffer availability of different logical channels on this time slot
      */ 
      rtsCheck(rfCh, timeSlot, PDTCH, 0, (NUM_TCH_JBUF - ts->u.group13.pdtch.dlSigBuf->frameCount));
      rtsCheck(rfCh, timeSlot, PACCH, 0, (DL_SIG_Q_DEPTH - ts->u.group13.pacch.dlSigBuf->numMsgsReady));
      rtsCheck(rfCh, timeSlot, PTCCH, 0, (DL_SIG_Q_DEPTH - ts->u.group13.ptcch.dlSigBuf->numMsgsReady));
      
      break;			

   default:
      break;
   }
   ts->tsConfig = 0;
}

/*****************************************************************************
*
* Function: sysSyncProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void sysSyncProc(void)
{
	Int16 ts;
	Int16 rfCh;

	sysFrameNum = setFrameNum;

	sysFrameNum.t1 = sysFrameNum.FN / (T2_FRAMES * T3_FRAMES);
	sysFrameNum.t2 = sysFrameNum.FN % T2_FRAMES;
	sysFrameNum.t3 = sysFrameNum.FN % T3_FRAMES;


	for(rfCh=0; rfCh < NUM_RFS_PER_DSP; rfCh++)
	{
   		for(ts=0; ts<NUM_TS_PER_RF; ts++)
		{
           tsFrameSync(rfCh, ts);
		}
	}

}

/*****************************************************************************
*
* Function: onePPSIsr
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void onePPSIsr(void)
{
	switch(g_frmSync.state)
	{
	case SET_SYNC:
	  sysSyncProc();
	  break;

	default:
	  break;
	}
  	 
}
















