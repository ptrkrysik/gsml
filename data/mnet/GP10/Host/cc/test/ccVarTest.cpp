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
// File        : ccVarTest.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "stdio.h"

#include "vxWorks.h"
#include "oam_api.h"
#include "JCC/JCCL3Msg.h"

#include "lapdm/lapdm_l3intf.h"

#include "MM/MMConfig.h"
#include "MM/MMInt.h"


extern MobilityConfigData	mmCfg;

// Used by RRM to create IRT entries.
extern T_CNI_L3_ID mm_id;

#include "RIL3/ril3irt_db.h"

#include "CC/CallConfig.h"
#include "CC/CCconfig.h"
#include "CC/CCInt.h"
#include "CC/CCTypes.h"

#include "JCC/LUDBInstance.h"
#include "JCC/LUDBConfig.h"

#include "JCC/JCCLog.h"

extern void mmDataInit(void);
extern void ccDataInit(void);

extern short registerUser(const T_CNI_RIL3_IE_MOBILE_ID& imsi);

extern short testIMSIindex;

extern T_CNI_RIL3_IE_MOBILE_ID testIMSI[];

extern short origSoftMobDialedNumIndex;

extern T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER testDialedNum[];

extern T_CNI_IRT_ID softMobEntryId[]; 

extern T_CNI_LAPDM_OID softMobLapdmOid[];  // Any large number

extern int termSoftMobIndex,
           origSoftMobIndex;
 
T_CNI_LAPDM_L2L3CallBackData callback[CC_MAX_CC_CALLS_DEF_VAL];

T_CNI_L3_ID l3Id1, l3Id2;

bool readIMSITable( char* dnFile )
{
   FILE* pfd;
   short i, j;
   int inp;

   if( NULL == ( pfd = fopen( dnFile, "r" ) ) )
     {
       printf( "Error opening file '%s'\n", dnFile );
       return (false);
     }
   
   for(i=0; i< CC_MAX_CC_CALLS;i++)
	 {
	   testIMSI[i].numDigits = 15;
	   
	   for (j=0; j < testIMSI[i].numDigits; j++)
		 {
		   fscanf( pfd , "%d", &inp);
		   testIMSI[i].digits[j] = inp; 
		 }

	   fscanf(pfd, "\n");

	   testIMSI[i].ie_present = true;
	   
	   testIMSI[i].mobileIdType = CNI_RIL3_IMSI;
	   
	   for (j=0; j < testIMSI[i].numDigits; j++)
		 {
		   printf( "%d ", testIMSI[i].digits[j]);
		 }

	   printf("\n");
     }
       
   fclose( pfd );
   
   return (true);
}

void locReqSet(short imsiIndex)
{
  testIMSIindex = imsiIndex;
}

void origSoftMobSet(short dialedNum)
{
  origSoftMobDialedNumIndex = dialedNum;
}

void softMobSet(short imsiIndex)
{
  testIMSIindex = imsiIndex;
}

void softMobStart(short imsiIndex)
{

  // Prepare L3 Ids
  l3Id1.msgq_id = mmMsgQId;
  l3Id1.sub_id = (unsigned short) mmCfg.mm_max_mm_connections;

  l3Id2.msgq_id = ccMsgQId;
 
  softMobSet(imsiIndex);

  callback[imsiIndex].lapdm_oid = softMobLapdmOid[imsiIndex];
  callback[imsiIndex].data.msgLength = 2;
  callback[imsiIndex].sapi = 0;
  callback[imsiIndex].layer3_ID = 1;
  callback[imsiIndex].unserved_data_after_release = false;

  callback[imsiIndex].error_cause = EC_NO_ERROR;

  // Create the IRT entry

  softMobEntryId[imsiIndex] = 
  //                    lapdm_oid,                       rr_id,    mm_id,   cc_id
  CNI_RIL3_IRT_Register(softMobLapdmOid[imsiIndex],      l3Id2,    l3Id1,   l3Id2 );

}

extern CNI_RIL3IRT_DB			CNI_IRT_database;

void
irtEntryPrint(T_CNI_RIL3IRT_DB_ENTRY &irtDBEntry) 
{
  printf("IRT DBEntry: %d, %d, %p, %d, %p, %d, %p, %d \n", 
		 irtDBEntry.entry_id,
		 irtDBEntry.lapdm_oid,
		 irtDBEntry.rrm_id.msgq_id,
		 irtDBEntry.rrm_id.sub_id,
		 irtDBEntry.mm_id.msgq_id,
		 irtDBEntry.mm_id.sub_id,
		 irtDBEntry.cc_id.msgq_id,
		 irtDBEntry.cc_id.sub_id);
}

void irtDataPrint()
{
  short i;
  JCCLog("***************************************************************************************\n");
  JCCLog("**************************************Synchronize with ShowAll*************************\n");
  JCCLog("***************************************************************************************\n");

  for(i=0; i< 38; i++)
    {
      irtEntryPrint(CNI_IRT_database.m_entry[i]);
    }
}

//  void irtMMDataInit()
//  {
//    short i;
//    for(i=0; i< 25 ; i++)
//      {
//        CNI_IRT_database.m_entry[i].mm_id = mm_id;
//      }
//  }

//  void mmDataInit()
//  {
//    mmConnDataInit();
//    irtMMDataInit();
//  }

void softMobInit(short imsiIndex)
{
  softMobSet(imsiIndex);
  mmDataInit();
  ccDataInit();
}

void fromMS(short callbackIndex,
	    T_CNI_LAPDM_L2L3PrimitiveType prim,
	    unsigned char buff0, 
	    unsigned char buff1)
{
  callback[callbackIndex].primitive_type = prim;
  callback[callbackIndex].data.buffer[0] = buff0;
  callback[callbackIndex].data.buffer[1] = buff1; 

  CNI_RIL3MD_L3CallBack (&callback[callbackIndex]);
  printf("CC Test Log: Sent Message from Soft MS, Prim. Type, Buffer Contents: %d %d %d\n",  
         prim, 
	 buff0,
         buff1);
}

void fromRRtoMM(short                      callIndex,
				IntraL3PrimitiveType_t     prim,
				IntraL3MsgType_t           msgType)
{
  short entryId = softMobEntryId[callIndex];

  IntraL3Msg_t       rrToMMMsg;

  rrToMMMsg.module_id = MODULE_RM;
  rrToMMMsg.primitive_type = prim; 
  rrToMMMsg.message_type = msgType;

  rrToMMMsg.entry_id = entryId;

  // Fill in the required message fields.
  rrToMMMsg.l3_data.pageRsp.mobileId = testIMSI[callIndex];

  // send the message.
  if (ERROR == msgQSend( mmMsgQId,
			 (char *) &rrToMMMsg, 
			 sizeof(struct  IntraL3Msg_t), 
			 NO_WAIT,
			 MSG_PRI_NORMAL
			 ) )
    {
      printf("CC Test Error : fromRRtoMM msgQSend (QID=%d) error\n ", 
	     mmMsgQId);
    }
  else
    {
      printf("CC Test Log: Sent Message from RR to MM, Prim. Type, Msg Type: %d %d\n",  
	     prim, 
	     msgType);
    }
}

void fromRRtoCC(IntraL3PrimitiveType_t     prim,
		IntraL3MsgType_t           msgType,
		short                      callIndex)
{
  CCSession_t *sess = &ccSession[callIndex];
  short  entryId = softMobEntryId[callIndex];

  IntraL3Msg_t       rrToCCMsg;

  rrToCCMsg.module_id = MODULE_RM;
  rrToCCMsg.primitive_type = prim; 
  rrToCCMsg.message_type = msgType;

  rrToCCMsg.entry_id = entryId;

  // send the message.
  if (ERROR == msgQSend( sess->msgQId,
			 (char *) &rrToCCMsg, 
			 sizeof(struct  IntraL3Msg_t), 
			 NO_WAIT,
			 MSG_PRI_NORMAL
			 ) )
    {
      printf("CC Test Error : fromRRtoCC msgQSend (QID=%d) error\n ", 
	     ccMsgQId);
    }
  else
    {
      printf("CC Test Log: Sent Message from RR to CC, Prim. Type, Msg Type: %d %d\n",  
	     prim, 
	     msgType);
    }
}

bool readDialedNumberTable( char* dnFile )
{
   FILE* pfd;
   short i, j;
   int inp;

   if( NULL == ( pfd = fopen( dnFile, "r" ) ) )
     {
       printf( "Error opening file '%s'\n", dnFile );
       return (false);
     }
   
   for(i=0; i< CC_MAX_CC_CALLS;i++)
	 {
	   testDialedNum[i].numDigits = 4;

	   for (j=0; j < testDialedNum[i].numDigits; j++)
		 {
		   fscanf( pfd , "%d", &inp);
		   testDialedNum[i].digits[j] = inp; 
		 }
	   fscanf(pfd, "\n");
	   
	   testDialedNum[i].ie_present = true;
	   testDialedNum[i].numberType = CNI_RIL3_NUMBER_TYPE_NETWORK_SPECIFIC; 
	   testDialedNum[i].numberingPlan = CNI_RIL3_NUMBERING_PLAN_PRIVATE;
	   
	   
	   for (j=0; j < testDialedNum[i].numDigits; j++)
		 {
		   printf( "%d ", testDialedNum[i].digits[j]);
		 }

	   printf("\n");

     }
       
   fclose( pfd );

   return (true);
}

/*

void readIMSITable()
{
  int i = origSoftMobIndex;

  testIMSI[i].ie_present = true;

  testIMSI[i].mobileIdType = CNI_RIL3_IMSI;

  testIMSI[i].numDigits = 15;

  testIMSI[i].digits[0] = 3;
  testIMSI[i].digits[1] = 1;
  testIMSI[i].digits[2] = 0;
  testIMSI[i].digits[3] = 1;
  testIMSI[i].digits[4] = 7;
  testIMSI[i].digits[5] = 0;
  testIMSI[i].digits[6] = 1;
  testIMSI[i].digits[7] = 0;
  testIMSI[i].digits[8] = 7;
  testIMSI[i].digits[9] = 4;
  testIMSI[i].digits[10] = 1;
  testIMSI[i].digits[11] = 2;
  testIMSI[i].digits[12] = 2;
  testIMSI[i].digits[13] = 2;
  testIMSI[i].digits[14] = 2;

  i = termSoftMobIndex;

  testIMSI[i].ie_present = true;

  testIMSI[i].mobileIdType = CNI_RIL3_IMSI;

  testIMSI[i].numDigits = 15;

  testIMSI[i].digits[0] = 3;
  testIMSI[i].digits[1] = 1;
  testIMSI[i].digits[2] = 0;
  testIMSI[i].digits[3] = 1;
  testIMSI[i].digits[4] = 7;
  testIMSI[i].digits[5] = 0;
  testIMSI[i].digits[6] = 1;
  testIMSI[i].digits[7] = 0;
  testIMSI[i].digits[8] = 7;
  testIMSI[i].digits[9] = 5;
  testIMSI[i].digits[10] = 9;
  testIMSI[i].digits[11] = 9;
  testIMSI[i].digits[12] = 9;
  testIMSI[i].digits[13] = 9;
  testIMSI[i].digits[14] = 2;
}

*/

/*
void readDialedNumberTable()
{
   short i = origSoftMobIndex;

   testDialedNum[i].ie_present = true;
   testDialedNum[i].numberType = CNI_RIL3_NUMBER_TYPE_NETWORK_SPECIFIC; 
   testDialedNum[i].numberingPlan = CNI_RIL3_NUMBERING_PLAN_PRIVATE;
   
   testDialedNum[i].numDigits = 4;

   testDialedNum[i].digits[0] = 2;
   testDialedNum[i].digits[1] = 2;
   testDialedNum[i].digits[2] = 0;
   testDialedNum[i].digits[3] = 4;

   i = termSoftMobIndex;
   
   testDialedNum[i].digits[0] = 2;
   testDialedNum[i].digits[1] = 2;
   testDialedNum[i].digits[2] = 0;
   testDialedNum[i].digits[3] = 5;

   i = 4;
   
   testDialedNum[i].digits[0] = 8;
   testDialedNum[i].digits[1] = 8;
   testDialedNum[i].digits[2] = 2;
   testDialedNum[i].digits[3] = 4;

   i = 5;
   
   testDialedNum[i].digits[0] = 8;
   testDialedNum[i].digits[1] = 8;
   testDialedNum[i].digits[2] = 2;
   testDialedNum[i].digits[3] = 5;

   i = 6;
   
   testDialedNum[i].digits[0] = 8;
   testDialedNum[i].digits[1] = 8;
   testDialedNum[i].digits[2] = 2;
   testDialedNum[i].digits[3] = 6;

   
}
*/
 


