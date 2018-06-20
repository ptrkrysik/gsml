#ifndef JCCComIE_H
#define JCCComIE_H

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
// File        : JCCComIE.h
// Author(s)   : Bhava Nelakanti
// Create Date : 05-10-99
// Description : Common ie interface among Application (Layer 3)
//               modules.
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included MD and IRT headers for messages from MS
#if !defined(_WINDOWS_) && !defined(WIN32)
#include "ril3/ril3md.h"
#endif

#include "ril3/ie_bearer_capability.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

// Typedefs
//

typedef struct {
  unsigned short                     networkId;               // Identifies the network
  unsigned short                     cellId;                  // Identifies the cell within the network
}JcVipercellId_t  ;

// Bearer Type
typedef enum {
  BEARER_BASE                         = 0,
  BEARER_SPEECH                       = BEARER_BASE,
  BEARER_DATA,
  BEARER_SIGNALING

} BearerType_t;

// Speech Chann Rate, etc
typedef struct {

  T_CNI_RIL3_SPEECH_VERSION  version;

} SpeechChann_t;

// Resource (Channel) Type - Needs expansion later.
typedef struct {
  BearerType_t               bearerType;
  union
  {
	// If bearer type indicates speech
	SpeechChann_t            speechChann;
	// If bearer type indicates data
	// DataChann_t              dataChann;
  };

} L3RMChannelType_t;

#endif                                       // JCCComIE_H
