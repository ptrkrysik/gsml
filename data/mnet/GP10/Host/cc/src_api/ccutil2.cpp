// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CCUtil2.cpp
// Author(s)   : Igal Gutkin
// Create Date : 04-04-00
// Description : This file contains common CC utility functions 
//               required by the ccm_api module.
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include <vxworks.h>

#include "jcc/JCCComMsg.h"
#include "cc/CCsessionHandler.h"
#include "cc/cc_util_ext.h"

//ext-HO <xxu:06-08-01>
//ext-HO <xxu:06-08-01>
short ccHoNumberCompare(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER *hoNumber,
			T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER *cpn,
			int length      )
{
  short result;

  DBG_FUNC("ccHoNumberCompare", CC_ETRG_LAYER);
  DBG_ENTER();

  result = 1;

  if ( (hoNumber->ie_present==cpn->ie_present)       &&
       (hoNumber->numberType==cpn->numberType)       &&
       (hoNumber->numberingPlan==cpn->numberingPlan) &&
       (hoNumber->numDigits==cpn->numDigits)          )
 
  {
	    if (memcmp(hoNumber->digits, cpn->digits, hoNumber->numDigits)==0)
		{
			DBG_TRACE("{\nccHoNumberCompare: CPN is Handove Number\n}\n");
			result = 0;
		} else
		{
			DBG_TRACE("{\nccHoNumberCompare: CPN is NOT Handove Number\n}\n");
		}
  } else
  {

	  DBG_TRACE("{\nccHoNumberCompare: CPN is NOT Handove Number\n}\n");
  }

  DBG_LEAVE();
  return result;
}

short ccIsHandoverCallSetup(IntraL3Msg_t *h323InMsg)
{
  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER cpn;
  cpn = h323InMsg->l3_data.voipMsg.callOffering.CalledPartyNumber;

  DBG_FUNC("ccIsHandoverCallSetup", CC_ETRG_LAYER);
  DBG_ENTER();

  //Dump cpn for  debugging purpose
  DBG_TRACE("{\nMNETeiTRACE(ccIsHandoverCallSetup): handover call SETUP cpn:\n");
  DBG_TRACE("              (cpn.ie_present=%d, cpn.numberType=%d, cpn.numberingPlan=%d\n",
						    cpn.ie_present,cpn.numberType,cpn.numberingPlan);
  DBG_TRACE("              (cpn.numDigits=%d digits:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n\n",
	                        cpn.numDigits,cpn.digits[0],cpn.digits[1],cpn.digits[2],
							cpn.digits[3],cpn.digits[4],cpn.digits[5],cpn.digits[6],
							cpn.digits[7],cpn.digits[8],cpn.digits[9],cpn.digits[10]);
  DBG_TRACE("               SETUP CPN Hex Dump:\n");
  DBG_HEXDUMP((unsigned char*)&cpn, sizeof(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER));
  DBG_TRACE("}\n\n");

  short i;
  for(i=0; i< CC_MAX_CC_CALLS; i++)
    {
      if ( (ccSession[i].free==false) &&
		   (ccSession[i].sessionHandler->handoverSession
		                 == ccSession[i].sessionHandler->targetHandoverSessionExtHo) )
	  {    
            if (ccHoNumberCompare(&ccSession[i].sessionHandler->handoverSession->hoNumber_, &cpn, 
				sizeof(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER)) == 0)
			{
              DBG_TRACE("\nMNETeiTRACE(ccIsHandoverCallSetup): found Handover Session(callId=%d)\n}\n",
				          i);
			  DBG_LEAVE();
              return i;
			}
         
      }
    }
  
  DBG_LEAVE();
  return JCC_ERROR;
}


//Overloaded method for searching according to ludbIndex
short mobInCall (const short ludbIndex)
{
  DBG_FUNC("mobInCall(ludbIndex)", CC_LAYER);
  DBG_ENTER();

  short i;
  for(i=0; i< CC_MAX_CC_CALLS; i++)
    {
      if (ccSession[i].free == false)
        {
          if (ccSession[i].sessionHandler->ludbIndex == ludbIndex)
            {
              DBG_LEAVE();
              return i;
            }
        }
    }
  
  DBG_LEAVE();
  return JCC_ERROR;
}

//Overloaded method for searching according to entry ID
short mobInCall2 (const T_CNI_IRT_ID entryId)
{
  DBG_FUNC("mobInCall(entryId)", CC_LAYER);
  DBG_ENTER();

  short index;
  for (index = 0; index < CC_MAX_CC_CALLS; index++)
  {
      if (ccSession[index].free == false)
      {
          if (ccSession[index].sessionHandler->entryId == entryId)
          {
              DBG_LEAVE();
              return   (index);
          }
      }
  }
  
  DBG_LEAVE();
  return JCC_ERROR;
}


// <Igal: 04-10-01>
// Unconditional release CC session(s) by the LUDB index
bool CcNotifyPurge (short ludbIndex)
{
  DBG_FUNC("CcNotifyPurge", CC_LAYER);
  DBG_ENTER();

  bool         retVal  = true;
  short        callIdx ;
  IntraL3Msg_t ccOutMsg;

    memset (&ccOutMsg, 0, sizeof(IntraL3Msg_t));

    ccOutMsg.module_id      = MODULE_LUDB                ;
    ccOutMsg.primitive_type = INTRA_L3_DATA              ;
    ccOutMsg.message_type   = INTRA_L3_LUDB_PURGE_PROFILE;

    for (callIdx = 0; callIdx < CC_MAX_CC_CALLS; ++callIdx)
    {
        if (ccSession[callIdx].free           != True && 
            ccSession[callIdx].sessionHandler != NULL   )
        {
            if (ccSession[callIdx].sessionHandler->ludbIndex == ludbIndex)
            {
                // Use priority in order to prevent CC from accessing removed LUDB entry
                // while processing other inputs
                if (msgQSend (ccSession[callIdx].msgQId    ,
                              (char *)&ccOutMsg            ,
                              sizeof (struct  IntraL3Msg_t),
                              NO_WAIT                      ,
                              MSG_PRI_URGENT               ) == ERROR)
                {
                    DBG_ERROR ("LUDB->CC API ERROR: send CC Call task [QID = %p] error.\n", 
                               (int)ccSession[callIdx].msgQId                             );
                    retVal = false;
                }
            }
        }
    }

  DBG_LEAVE();
  return (retVal);
}

