/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: bcch.c              
*                       
* Description: Subroutines to Process downlink BCCH Messages
*              in 4 bursts, SCH and CCCH process
*
*****************************************************************************/
#include "stdlib.h" 
#include "gsmdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"

/**********************************************************
 *	Routine: void dlBcchProc0(Uint8 rfCh, Uint8 timeSlot)              
 *
 *	Description:
 *   Synchronousely process 1st of 4 BCCH bursts,
 *   Channel encode is required
 *
 *	History: Change Mapping from GSM 05.02-820 Pagaraph 6.3.1.3
 * 		
 ***********************************************************/


void dlBcchProc0(Uint8 rfCh, Uint8 timeSlot)
{
   t_sysInfoQueue *bcchInfo;
   t_TNInfo *ts;          
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
		          
	  switch(ts->groupNum)
   {
	  case GSM_GROUP_5:
		    switch(ts->u.group5.bcchBufIndex)
      {
		    case 0:
			      /*
         * set SysInfo Type 1
         */   		
            bcchInfo = ts->u.group5.bcch.dlSigBuf;

		      if(!bcchInfo->valid)
         /*
         * set SysInfo Type 3 if no Type 1
         */
			      bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_3;
                        
         break;		
		
		    case 1:  
			     /*
        * set SysInfo Type 2
        */   		
			     bcchInfo = ts->u.group5.bcch.dlSigBuf +  TYPE_2;
                  
        break;

      case 2:
        	/*
        	* set SysInfo Type 3
        	*/        
			      bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_3;                                             
         break; 

      case 3:                 
      	  /*
      	  * set SysInfo Type 4 or 8
      	  */
			      bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_4;
			      break;                        

		    case 4:
     	   /*
        	* set SysInfo Type 13, 9, or 2Ter
        	*/
			      bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_13;
                  
         if(!bcchInfo->valid)
			         bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_9;
                  
         if(!bcchInfo->valid)
            		bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_2_TER;  
            		
      			/*
			      * send SysInfo Type 2 if nothing to send 
			      */       
 			     if(!bcchInfo->valid)
            		bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_3;  
			      break;            		

		    case 5:
        	/*
        	* set SysInfo Type 2bis, 2 ter
        	*/
			      bcchInfo = ts->u.group5.bcch.dlSigBuf +  TYPE_2_BIS;

         if(!bcchInfo->valid)
          		bcchInfo = ts->u.group5.bcch.dlSigBuf +  TYPE_2_TER;
            		
      
      			/*
			      * send SysInfo Type 2 if nothing to send 
			      */       
 			     if(!bcchInfo->valid)
            		bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_2;  
            	            		            		
			      break;            		

		    case 6:
      			/*
			      * send SysInfo Type 3
			      */                  
       		bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_3;            			
			      break;            		

		    case 7:
        	/*
        	* set SysInfo Type 4
        	*/
			      bcchInfo = ts->u.group5.bcch.dlSigBuf + TYPE_4;
                          
			      break;		                       
      default:
        	break;
      }        
		    break;
			
   default:
		 break;
		
	  }

    if(bcchInfo->valid)
    {	
		     if(bcchInfo->frameNumUpdate == sysFrameNum.FN % (32*1326))
       {
			       bcchInfo->state ^= 0x01;
			       bcchInfo->frameNumUpdate = 0xffffffff;
       }    	    
		
       unpackFromByteBuffer(&bcchInfo->data[bcchInfo->state][0],
                           (UChar *)&g_DLCCHData.data[0], 
                           NUM_BITS_CCH_FRAME);
   }
   else
   {
	     	unpackFromByteBuffer(bcchFiller,
                           (UChar *)&g_DLCCHData.data[0], 
                           NUM_BITS_CCH_FRAME);
   }   
   g_DLCCHData.bufferValid = TRUE;
        		            
   chanEncoder(BCCH, 1, timeSlot,0);
   buildGSMBurst(BCCH, g_BBInfo[rfCh].tseqNum);
   GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, BCCH);			
}


/**********************************************************
 *	Routine: void dlBcchProc1(Uint8 rfCh, Uint8 timeSlot)              
 *
 *	Descriptions: Synchronousely process 2nd of 4 BCCH bursts,
 *                no Channel encode is required
 *
 *	History:
 *                                                          
 ***********************************************************/
void dlBcchProc1(Uint8 rfCh, Uint8 timeSlot)
{      
	chanEncoder(BCCH, 0, timeSlot,0);    
	buildGSMBurst(BCCH, g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, BCCH);			
}

/**********************************************************
 *	Routine: void dlBcchProc2(Uint8 rfCh, Uint8 timeSlot)              
 * *
 *	Descriptions: Synchronousely process 3rd of 4 BCCH bursts,
 *                no Channel encode is required
 *
 *	History:
 *                                                          
 ***********************************************************/
void dlBcchProc2(Uint8 rfCh, Uint8 timeSlot)
{
	chanEncoder(BCCH, 0, timeSlot,0);    
	buildGSMBurst(BCCH, g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, BCCH);			
}
  
/**********************************************************
 *	Routine: void dlBcchProc3(Uint8 rfCh, Uint8 timeSlot)              
 * *
 *	Descriptions: Synchronousely process 4th of 4 BCCH bursts,
 *                no Channel encode is required. Needs to prepare
 *                next SYS INFO buffer.
 *
 *	History:
 * 		
 ***********************************************************/  
void dlBcchProc3(Uint8 rfCh, Uint8 timeSlot)
{   
	t_sysInfoQueue *bcchInfo;
	t_TNInfo *ts;          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
		          
	switch(ts->groupNum)
	{
	case GSM_GROUP_5:
		bcchInfo = ts->u.group5.bcch.dlSigBuf +
                  ts->u.group5.bcchBufIndex++;
		/*
		* use next BCCH of the SYS INFO queue at next BCCH
		* sub frame, (FN/51) Mod 8
		*/                  
		ts->u.group5.bcchBufIndex &= 0x7;
					
		break;
        case  GSM_GROUP_11:
                if (++ts->u.group11.dlBlock >= MAX_RADIO_BLOCKS){
                        ts->u.group11.dlBlock = 0;
                }

                /* increment buffer index */
                if (++ts->u.group11.pbcchBufIndex >= NUM_SYSINFO_BUFS){
                        ts->u.group11.pbcchBufIndex = PACKET_TYPE_1;
                }
                
                break;
                
			
	default:
		break;
		
	}
 	chanEncoder(BCCH, 0, timeSlot,0);    
	buildGSMBurst(BCCH, g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, BCCH);			
}

/**********************************************************
 *	Routine: void dlFcchProc(Uint8 rfCh, Uint8 timeSlot)              
 *
 *	Descriptions: Synchronousely process FCCH bursts,
 *
 *	History:
 *                                                          
 ***********************************************************/                                  
void dlFcchProc(Uint8 rfCh, Uint8 timeSlot)
{
	t_TNInfo *ts;          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
		          
	switch(ts->groupNum)
	{
	case GSM_GROUP_5:
		break;
	default:
		break;
	}                
	buildGSMBurst(FCCH,  g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, FCCH);	
}

                                

                                
/**********************************************************
 *	Routine: dlSchProc(Uint8 rfCh, Uint8 timeSlot))              
 *
 *	Descriptions: Synchronousely process SCH bursts
 *                
 *
 *	History:
 * 		
 *                                                          
 ***********************************************************/      
void dlSchProc(Uint8 rfCh, Uint8 timeSlot)
{
	t_TNInfo *ts;          
	t_SCHSigBufs *schInfo;
	Uint8 t3_mod;
	
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
			          
	switch(ts->groupNum)
	{
	case GSM_GROUP_5:
		schInfo = ts->u.group5.sch.dlSigBuf;			                
		break;
	default:
		break;
	}
    if(schInfo->valid)
    {	
		if(schInfo->frameNumUpdate == sysFrameNum.FN % (32*1326))
		{
			schInfo->state ^= 0x01;
			schInfo->frameNumUpdate = 0xffffffff;
		}
		                                                      
		/*
		* there are 6 bits BSIC code and 2bit of t1 in the first byte 
		* there are 8 bits of t1 in byte 2
		* there are 1 bit of t1, 5 bits of t2 and 2 bits of t3' in byte 3 
		* there are 1 bit of t3' in byte 4
		*/		                                                      
		schInfo->data[schInfo->state][0] &= 0xFC;
		schInfo->data[schInfo->state][0] |= sysFrameNum.t1>>9;
		schInfo->data[schInfo->state][1] = sysFrameNum.t1>>1;
		schInfo->data[schInfo->state][2] = sysFrameNum.t1<<7;
		schInfo->data[schInfo->state][2] |= sysFrameNum.t2<<2;
		
		t3_mod = (sysFrameNum.t3-1)/10;
		schInfo->data[schInfo->state][2] |= t3_mod>>1;
		schInfo->data[schInfo->state][3] = t3_mod & 0x01;
														                        
      unpackFromByteBuffer(&schInfo->data[schInfo->state][0],
                           (UChar*)&g_DLCCHData.data[0], 
                           NUM_BITS_SCH_FRAME+7);
                                 
      g_DLCCHData.bufferValid = TRUE;
        		       		    	                
		chanEncoder(SCH, 1, timeSlot,0);
		buildGSMBurst(SCH, g_BBInfo[rfCh].tseqNum);
		GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SCH);
	}  
	else 
	{
		buildGSMBurst(IDLE, g_BBInfo[rfCh].tseqNum);
		GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, IDLE);			
	}		
}










