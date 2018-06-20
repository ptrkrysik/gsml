#ifndef MnCalXfr_H
#define MnCalXfr_H

// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : MnCalXfr.h
// Author(s)   : Bhava Nelakanti
// Create Date : 
// Description : interface specification for call transfer
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#ifdef __cplusplus
extern "C" {
#endif

// *******************************************************************
// Constant Definitions
// *******************************************************************
#define MNET_H323_MAX_ALIAS_NUMBER_LENGTH (128 + 1)

typedef unsigned short MnetRasRequestSeqNum_t;
typedef unsigned char  MnetRasRejectCause_t;
typedef char*  MnetAliasString_t;

typedef enum {

  MNET_H323_PUBLIC_PARTY_NUMBER,
  MNET_H323_PRIVATE_PARTY_NUMBER
} MnetH323PartyNumber_t;

typedef enum {

  MNET_H323_PUBLIC_UNKNOWN,
  MNET_H323_PUBLIC_INTERNATIONAL,
  MNET_H323_PUBLIC_NATIONAL,
  MNET_H323_PUBLIC_NETWORK_SPECIFIC,
  MNET_H323_PUBLIC_SUBSCRIBER,
  MNET_H323_PUBLIC_ABBREVIATED,

} MnetH323PublicTypeOfNumber_t;

typedef enum {

  MNET_H323_PRIVATE_UNKNOWN,
  MNET_H323_PRIVATE_LEVEL2_REGIONAL,
  MNET_H323_PRIVATE_LEVEL1_REGIONAL,
  MNET_H323_PRIVATE_PISN_SPECIFIC,
  MNET_H323_PRIVATE_LOCAL,
  MNET_H323_PRIVATE_ABBREVIATED,

} MnetH323PrivateTypeOfNumber_t;

typedef struct {
  MnetAliasString_t               numberDigits;
  
  MnetH323PartyNumber_t           partyNumber;

  union
  {
    MnetH323PublicTypeOfNumber_t    publicType;
    MnetH323PrivateTypeOfNumber_t   privateType;
  };
} MnetH323AliasAddress_t;

typedef struct {
  MnetRasRequestSeqNum_t          requestSeqNum;

  MnetH323AliasAddress_t          destinationAliasAddress;

  MnetH323AliasAddress_t          sourceAliasAddress;

} MnetRasTransferRequest_t;

typedef struct {
  MnetRasRequestSeqNum_t          requestSeqNum;

  MnetH323AliasAddress_t          destinationAliasAddress;

} MnetRasTransferConfirm_t;

typedef struct {
  MnetRasRequestSeqNum_t          requestSeqNum;

  MnetRasRejectCause_t            rejectCause;   // uses Q.931 Cause values 

} MnetRasTransferReject_t;  

/* messages packing/unpacking */
void jcNonStdRasMessageTransferRequestPack( MnetRasTransferRequest_t* pMsg, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageTransferRequestUnpack( MnetRasTransferRequest_t* pMsg, UCHAR** ppszBuffer );

void jcNonStdRasMessageTransferConfirmPack( MnetRasTransferConfirm_t* pMsg, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageTransferConfirmUnpack( MnetRasTransferConfirm_t* pMsg, UCHAR** ppszBuffer );

void jcNonStdRasMessageTransferRejectPack( MnetRasTransferReject_t* pMsg, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageTransferRejecttUnpack( MnetRasTransferReject_t* pMsg, UCHAR** ppszBuffer );

void jcNonStdRasMessageH323AliasAddressPack( MnetH323AliasAddress_t* pMsg, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageH323AliasAddressUnpack( MnetH323AliasAddress_t* pMsg, UCHAR** ppszBuffer );

/* handlers for transfer request */
INT32 mnetRasHandleTransferRequest( UINT32 ip, MnetRasTransferRequest_t* pMsg);

#ifdef __cplusplus
}
#endif

#endif // MnCalXfr_H
