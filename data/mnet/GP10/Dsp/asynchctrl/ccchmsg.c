/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: ccchmsg.c
*
* Description:
*   This file contains functions that handle CCCH messages from Host to DSP.
*
* Public Functions:
*   ccchMsgProc
*
* Private Functions:
*   none
*
******************************************************************************/
#include "stdlib.h" 
#include "string.h" 
#include "gsmdata.h"
#include "dsp/dsphmsg.h" 
#include "dsprotyp.h"
#include "intr.h"

/*****************************************************************************
*
* Function: ccchMsgProc
*
* Description:
*   Handles CCCH messages from Host to DSP.
*
* Inputs:  
*   ccchMsg           ptr to CCCH message structure
*   ->ccchInfo          info substructure containing (what?)
*   ->trx      
*   ->typeLoByte    
*   ->typeHiByte     
*
* Outputs:
*   sysInfoBuf        fill in
*   bcchInfo          ptr to BCCH information structure
*   ->valid             indicates whether to send this SysInfo type  
*   ->update  
*   ->frameNumUpdate  
*
******************************************************************************/
ProcResult ccchMsgProc(msgUnionStruc *ccchMsg)
{                 
	t_sysInfoQueue *bcchInfo, *pBcchInfo;
	t_TNInfo *ts;	
	ProcResult Result;
	
	Uint8 subCh;
	Uint8 logCh;
	Uint16 msgType;
	Uint8 sysInfoBuf;	
       
         
	/*
	* find correct time slot
	*/	
	ts = & g_BBInfo[ccchMsg->trx & 0x01].TNInfo[ *(Uint8 *) & (ccchMsg->u.ccchInfo) & 0x7];
	       
	Result = SUCCESS;

	msgType = ccchMsg->typeLoByte |	(ccchMsg->typeHiByte<<8);
	
	switch(ts->groupNum)
	{
	case GSM_GROUP_5:
		switch(msgType)
		{
		case BCCH_INFO_REQ:
			/*
			* copy the SYS Info update to its correspondent sys info queue
 			*/             
 			switch(ccchMsg->u.ccchInfo.sysInfoType)
 			{
 			
 			case SYS_INFO_8:	
			 case SYS_INFO_1:
			 case SYS_INFO_2:
			 case SYS_INFO_3:
			 case SYS_INFO_4:
				   sysInfoBuf = ccchMsg->u.ccchInfo.sysInfoType;
				break;                                           

                        case SYS_INFO_13:
                                sysInfoBuf = TYPE_13;
                        break;
			default:
				ReportError(0,0,WRONG_SYS_INFO_TYPE,1, \
                                            & ccchMsg->u.ccchInfo.sysInfoType);
				return(FAILURE);
			} /* switch sysInfoType */
                        

			sendDiagMsg(30, 0, 0, 1, & ccchMsg->u.ccchInfo.sysInfoType);

		 	bcchInfo = ts->u.group5.bcch.dlSigBuf + sysInfoBuf;
 			
			if(ccchMsg->u.ccchInfo.msgSize == 0)
			{
				/*
				* stop sending this sysInfo type
				*/	
				bcchInfo->valid = 0;
			}
			else
			{   
				if(bcchInfo->state == 0)
				{
					memcpy((Uint8 *) & bcchInfo->data[1], \
                                               & ccchMsg->u.ccchInfo.sysInfo[0], 23);
				}
				else                                
				{     
					memcpy((Uint8 *) & bcchInfo->data[0], \
                                               & ccchMsg->u.ccchInfo.sysInfo[0], 23);
				}
				    
				/*
				* if change is immediate, change active buffer now
				*/
				if(ccchMsg->u.ccchInfo.startTime.IMM) bcchInfo->state ^= 0x01;
				else
				{
					/*
					* set sysinfo update frame number
					*/		                                          
					bcchInfo->frameNumUpdate = 
						(ccchMsg->u.ccchInfo.startTime.timeByte0 & 0xf8)>>3 *1326 +   /*t1*/
					
						((ccchMsg->u.ccchInfo.startTime.timeByte0 & 0x07)<<3 |   /*t3*/
						(ccchMsg->u.ccchInfo.startTime.timeByte1 & 0xE0)>>5) *
									              
						(ccchMsg->u.ccchInfo.startTime.timeByte1 & 0x1F)>>3;    /*t2*/
						
					bcchInfo->update = 1;
				}		
				/*                                                            
				* start sending this sysInfo type
				*/	
				bcchInfo->valid = 1; 
			}
                        
                        
                        
			break;
								     
		default:
			break;
		}  /* switch msgType */
                
		break;

        case GSM_GROUP_11:
        {
           switch (msgType){
                   	case BCCH_INFO_REQ:
                                switch(ccchMsg->u.ccchInfo.sysInfoType){
                                        
                                  case PSI_1:
                                    sysInfoBuf = PACKET_TYPE_1;
                                  break;
                                  case PSI_2:
                                    sysInfoBuf = PACKET_TYPE_2;
                                  break;
                                  case PSI_3:
                                       sysInfoBuf = PACKET_TYPE_3;
                                  break;
                                  case PSI_4:
                                       sysInfoBuf = PACKET_TYPE_4;   
                                  break;
                                  case PSI_5:
                                      sysInfoBuf = PACKET_TYPE_5;                 
                                  break;
                                  case PSI_13:
                                      sysInfoBuf = PACKET_TYPE_13;
                                  break;
                                  default:
	                        	ReportError(0,0,WRONG_SYS_INFO_TYPE,1, \
                                                    & ccchMsg->u.ccchInfo.sysInfoType);
				return(FAILURE);
                                break;
                                }   /* switch sysinfoType*/
                                
                                pBcchInfo = ts->u.group11.pbcch.dlSigBuf + sysInfoBuf;
 			
			       if(ccchMsg->u.ccchInfo.msgSize == 0){
                                       pBcchInfo->valid = 0;   /* indicate an invalid message */
			        } else {

                                        INTR_GLOBAL_DISABLE();
					memcpy((Uint8 *) & pBcchInfo->data, \
                                               & ccchMsg->u.ccchInfo.sysInfo[0], 23);
	         			pBcchInfo->valid = 1;   /* Indicate a valid message */
                                        INTR_GLOBAL_ENABLE();
                                }
                   default:
                   break;
                   
                               
           }  /* switch msgType */
           
           
                
                
           break;
	}  /* case Group 11 */
        
        
	
	default:
		break;
	}
	
	return(Result);		
}














