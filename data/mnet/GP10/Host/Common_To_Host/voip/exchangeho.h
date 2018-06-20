#ifndef EXCHANGEHO_H
#define EXCHANGEHO_H
/* exchangeho.h
 *
 * (c) Copyright Cisco Systems 2000 - 2001
 * All Rights Reserved

 * VBLink messages - handoff interface between GMC and GP
 */

#ifdef __cplusplus
extern "C" {
#endif

// *******************************************************************
// Includes
// *******************************************************************
#include "ril3/ie_called_bcd.h"
#include "ril3/ril3_l2intf.h"


// *******************************************************************
// Constant Definitions
// *******************************************************************
const int NUM_MCC_DIGITS         = 3;
const int NUM_MNC_DIGITS         = 3;
const int MAX_A_INTERFACE_LEN    = 251;  // From Tai Yu's A_Handover_Common.h's definition of AIF_MSG_LENGTH
const int MAX_LAYER3_PDU_LEN     = CNI_LAPDM_MAX_L3MSG_LENGTH;

// *******************************************************************
// Type Definitions
// *******************************************************************

//<mukund:06/19/01> Handover Cause values
typedef enum
{
   MC_HO_SUCCESS,
   MC_HO_SYSTEM_FAILURE,
   MC_HO_DATA_MISSING,
   MC_HO_UNEXPECTED_DATA_VALUE,
   MC_HO_HANDOVER_NUMBER_UNAVAIL,
   MC_HO_UNKNOWN_MSC,
   MC_HO_SUBHO_FAILURE,
   MC_HO_T204_TIMEOUT,
   MC_HO_T202_TIMEOUT,
   MC_HO_RLS_FAILURE,
   MC_HO_ITP_LINK_DOWN,
   MC_HO_MAX_CAUSE_VALUES
} HandoverCause_t;

// [mukund:07-10-01]
#ifdef __cplusplus
inline char *GetHandCauseString(HandoverCause_t cause)
#else
static char *GetHandCauseString(HandoverCause_t cause)
#endif
{
  char *HandCauseString[] = 
  {
    "Success",
    "System failure",
    "Data Missing",
    "Unexpected data value",
    "Handover number unavailable",
    "Unknown MSC",
    "Subsequent Handover Failure",
    "T204 timeout",
    "T202 timeout",
    "Release failure",
    "ITP link down"
  };
  
  if (cause < MC_HO_MAX_CAUSE_VALUES)
    {
      return (HandCauseString[cause]);
    }
  else
    {
      return("UNKNOWN_CAUSE");
    }
}


//<taiyu:05-18-01> Global Cell ID as defined in GSM 08.08 3.2.2.17
typedef struct {
  char                  ci_disc;        // Cell Id Discriminator
                                        // 0000 The whole Cell Global Id is used
                                        // 0001 LAC, and CI, is used
                                        // 0010 Cell Identity, CI, is used
                                        // 0011 No cell is associated with the transaction.
  char                  mcc[NUM_MCC_DIGITS];  // 3 digits of Mobile Country Code
  char                  mnc[NUM_MNC_DIGITS];  // 3 digits of Mobile Network Code
  unsigned short        lac;            // Local Area Code
  unsigned short        ci;             // Cell ID
} GlobalCellId_t ;

void jcNonStdRasMessageGlobalCellIdPack( GlobalCellId_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageGlobalCellIdUnpack( GlobalCellId_t*, UCHAR** ppszBuffer );

// Message Header
typedef struct {
  short                  origSubId;      // originating hoAgID or sessionID
  short                  destSubId;      // destination hoAgID or sessionID
} InterHoaVcHeader_t;

void jcNonStdRasMessageHoaVcHeaderPack( InterHoaVcHeader_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageHoaVcHeaderUnpack( InterHoaVcHeader_t*, UCHAR** ppszBuffer );

// A Interface Message
typedef struct {
  USHORT                     msglength;
  unsigned char              A_INTERFACE_DATA[MAX_A_INTERFACE_LEN];
} A_INTERFACE_MSG_STRUCT_t;

void jcNonStdRasMessageAInterfaceMsgPack( A_INTERFACE_MSG_STRUCT_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageAInterfaceMsgUnpack( A_INTERFACE_MSG_STRUCT_t*, UCHAR** ppszBuffer );

// For E.164 packing (structure defined in common RIL3 module)
void jcNonStdRasMessageE164NumberPack( T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER* pMsg,
                                       UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageE164NumberUnpack( T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER* pMsg,
                                         UCHAR** ppszBuffer );

// Perform Handover Request Message
typedef struct
{
   InterHoaVcHeader_t                     header;
   GlobalCellId_t                         globalCellID;
   USHORT                                 hoCause;
   bool                                   hoNumberReqd;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  handoverNumber;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  mscNumber;
   A_INTERFACE_MSG_STRUCT_t               A_HANDOVER_REQUEST;
} NonStdRasMessagePerformHandoverRQ_t;

void jcNonStdRasMessagePerformHandoverRQPack( NonStdRasMessagePerformHandoverRQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePerformHandoverRQUnpack( NonStdRasMessagePerformHandoverRQ_t*, UCHAR** ppszBuffer );

// End Handover Message
typedef struct
{
   InterHoaVcHeader_t header;
   USHORT             reason;
} NonStdRasMessageEndHandover_t;

void jcNonStdRasMessageEndHandoverPack( NonStdRasMessageEndHandover_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageEndHandoverUnpack( NonStdRasMessageEndHandover_t*, UCHAR** ppszBuffer );

// Layer 3 PDU Message
typedef struct {
  USHORT                     msglength;
  char                       LAYER3DATA[MAX_LAYER3_PDU_LEN];
} LAYER3PDU_STRUCT_t;

void jcNonStdRasMessageLayer3MsgPack( LAYER3PDU_STRUCT_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageLayer3MsgUnpack( LAYER3PDU_STRUCT_t*, UCHAR** ppszBuffer );

// Post Handover Mob Event Message
typedef struct
{
   InterHoaVcHeader_t header;
   LAYER3PDU_STRUCT_t LAYER3PDU;
} NonStdRasMessagePostHandoverMobEvent_t;

void jcNonStdRasMessagePostHandoverMobEventPack( NonStdRasMessagePostHandoverMobEvent_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePostHandoverMobEventUnpack( NonStdRasMessagePostHandoverMobEvent_t*, UCHAR** ppszBuffer );

// Perform Handover Ack Message
typedef struct
{
   InterHoaVcHeader_t                     header;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  handoverNumber;
   A_INTERFACE_MSG_STRUCT_t               A_INTERFACE_MSG;
   USHORT                                 reason;
} NonStdRasMessagePerformHandoverAck_t;

void jcNonStdRasMessagePerformHandoverAckPack( NonStdRasMessagePerformHandoverAck_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePerformHandoverAckUnpack( NonStdRasMessagePerformHandoverAck_t*, UCHAR** ppszBuffer );

// Perform Handover Access Message
typedef struct
{
   InterHoaVcHeader_t        header;
   A_INTERFACE_MSG_STRUCT_t  A_INTERFACE_MSG;
} NonStdRasMessageHandoverAccess_t;

void jcNonStdRasMessageHandoverAccessPack( NonStdRasMessageHandoverAccess_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageHandoverAccessUnpack( NonStdRasMessageHandoverAccess_t*, UCHAR** ppszBuffer );

// Perform Handover Success Message
typedef struct
{
   InterHoaVcHeader_t        header;
   A_INTERFACE_MSG_STRUCT_t  A_INTERFACE_MSG;
} NonStdRasMessageHandoverSuccess_t;

void jcNonStdRasMessageHandoverSuccessPack( NonStdRasMessageHandoverSuccess_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageHandoverSuccessUnpack( NonStdRasMessageHandoverSuccess_t*, UCHAR** ppszBuffer );

// Perform Handback Request Message
typedef struct
{
   InterHoaVcHeader_t                     header;
   USHORT                                 hoCause;
   GlobalCellId_t                         globalCellID;
   bool                                   externalCell;
   T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  mscNumber;
   A_INTERFACE_MSG_STRUCT_t               A_HANDOVER_REQUEST;
} NonStdRasMessagePerformHandbackRQ_t;

void jcNonStdRasMessagePerformHandbackRQPack( NonStdRasMessagePerformHandbackRQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePerformHandbackRQUnpack( NonStdRasMessagePerformHandbackRQ_t*, UCHAR** ppszBuffer );

// Perform Handback Ack Message
typedef struct
{
   InterHoaVcHeader_t        header;
   A_INTERFACE_MSG_STRUCT_t  A_INTERFACE_MSG;
   USHORT                    reason;
} NonStdRasMessagePerformHandbackAck_t;

void jcNonStdRasMessagePerformHandbackAckPack( NonStdRasMessagePerformHandbackAck_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePerformHandbackAckUnpack( NonStdRasMessagePerformHandbackAck_t*, UCHAR** ppszBuffer );


// Release HOA Message
typedef struct
{
   InterHoaVcHeader_t        header;
} NonStdRasMessageReleaseHOA_t;

void jcNonStdRasMessageReleaseHOAPack( NonStdRasMessageReleaseHOA_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageReleaseHOAUnpack( NonStdRasMessageReleaseHOA_t*, UCHAR** ppszBuffer );


#ifdef __cplusplus
}
#endif

#endif                                       // EXCHANGEHO_H

