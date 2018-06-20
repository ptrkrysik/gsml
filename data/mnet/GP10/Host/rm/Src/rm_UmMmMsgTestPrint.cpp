/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_UMMMMSGTESTPRINT_CPP__
#define __RM_UMMMMSGTESTPRINT_CPP__
 
#include "rm\rm_head.h"

#include "ril3\ie_location_area_id.h"
#include "ril3\ie_classmark_1.h"
#include "ril3\ie_mobile_id.h"
#include "ril3\ril3_mm_msg.h"

void rm_UmMmMsgTestPrint( T_CNI_RIL3MM_MSG *pMsg ) 
{
     T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REQUEST *pLUP;

     switch (pMsg->header.message_type)
     {
     case CNI_RIL3MM_MSGID_LOCATION_UPDATING_REQUEST:
          pLUP = (T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REQUEST*) 
                  &(pMsg->locationUpdateRequest);

          /* Output the received LUP message */
          PDEBUG__(("LUP===========: PD=%d,MT=%\n",
                  pLUP->header.protocol_descriminator,
                  pLUP->header.message_type
                ));
          PDEBUG__(("LUP:Type IE: present=%d,forBit=%d,type=%d\n",
                   pLUP->locationUpdateType.ie_present,
                   pLUP->locationUpdateType.forBit,
		   pLUP->locationUpdateType.locationUpdateType
                 ));
          PDEBUG__(("LUP:CipherKey IE: present=%d,keySeq=%d\n",
		  pLUP->cipherKey.ie_present,
		  pLUP->cipherKey.keySeq
		));
          PDEBUG__(("LUP:LAI IE---------------------------\n"));
          TDUMP__((MAREA__,(u8*) &(pLUP->locationId),(u16)
                   sizeof(T_CNI_RIL3_IE_LOCATION_AREA_ID) ));
          PDEBUG__(("LUP:Classmark1 IE: present=%d,revLevel=%d,esInd=%d,a51=%d,rfPowerCap=%d\n",
		  pLUP->classmark1.ie_present,
		  pLUP->classmark1.revLevel,
		  pLUP->classmark1.a51,
		  pLUP->classmark1.rfPowerCap
		));
          PDEBUG__(("LUP:MobileID IE---------------------------\n"));
          TDUMP__((MAREA__,(u8*) &(pLUP->mobileId),(u16)
                   sizeof(T_CNI_RIL3_IE_MOBILE_ID) ));
          break;
 
     default:
	  PDEBUG__(("rm_UmMmMsgTestPrint: unsupport msgtype=%d\n",
		  pMsg->header.message_type));                  
          break;
     }
}

#endif /* __RM_UMMMMSGTESTPRINT_CPP__ */
