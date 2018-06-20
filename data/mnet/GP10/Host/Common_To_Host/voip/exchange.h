/* exchange.h
 *
 * (c) Copyright Cisco Systems 2000 - 2001
 * All Rights Reserved

 * VBLink messages - interface between GMC and GP
 */

#ifdef __cplusplus
extern "C" {
#endif

// Exchangeho.h is the external handover extension of exchange.h
#include "exchangeho.h"

//#include "ril3/ril3md.h"
typedef unsigned char T_CNI_MODULE_ID;

#define H323_MANUFACTURER_CODE_JETCELL  18  /* Cisco's man. code */
#define H323_COUNTRY_CODE_USA  181

#define NON_STD_MSG_MAX_LEN   1458
#define NON_STD_RAS_PROTOCOL  2 /* to be changed when GMC and GP are no longer compatible */

typedef ULONG NSRASHANDLE;

typedef enum
{
   NonStdRasMessageTypeRRQ,
   NonStdRasMessageTypeRCF,
   NonStdRasMessageTypeRRJ,
   NonStdRasMessageTypeURQ,
   NonStdRasMessageTypeUCF,

   NonStdRasMessageTypeCellLRQ,
   NonStdRasMessageTypeCellLCF,
   NonStdRasMessageTypeCellLRJ,

   NonStdRasMessageTypeMobileNotUsed, /* message is not currently in use */
   NonStdRasMessageTypeMobileProfile,

   NonStdRasMessageTypeCellSynchronize,
   NonStdRasMessageTypeCellInfo,

   NonStdRasMessageTypeMsToNetCISS,
   NonStdRasMessageTypeNetToMsCISS,

   NonStdRasMessageTypeSRQ, /* security info request */
   NonStdRasMessageTypeSRS, /* security info response */

   NonStdRasMessageTypeMsToNetSMSPP,    /* Sms PP message from MS to SM-SC */
   NonStdRasMessageTypeNetToMsSMSPP,    /* Sms PP message from SM-SC to MS */

   NonStdRasMessageTypeTransferRequest,
   NonStdRasMessageTypeTransferConfirm,
   NonStdRasMessageTypeTransferReject,

   NonStdRasMessageTypeKeepAlive,

   // ext-HO <chenj:05-15-01>
   NonStdRasMessageTypePerformHandoverRQ,
   NonStdRasMessageTypePerformHandoverAck,
   NonStdRasMessageTypeEndHandover,
   NonStdRasMessageTypePostHandoverMobEvent,
   NonStdRasMessageTypeHandoverAccessMsg,
   NonStdRasMessageTypeHandoverSuccessMsg,
   NonStdRasMessageTypePerformHandBackRQ,
   NonStdRasMessageTypePerformHandBackAck,
   NonStdRasMessageTypeReleaseHOA,

   // MGPLA <chenj:08-06-01>
   NonStdRasMessageTypePageRequest,
   NonStdRasMessageTypePageResponse
}NonStdRasMessageType;

typedef struct
{
   char* pszMobileID;

}NonStdRasMessageRRQ_t;

void jcNonStdRasMessageRRQPack( NonStdRasMessageRRQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageRRQUnpack( NonStdRasMessageRRQ_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;
   char* pszNumber;

}NonStdRasMessageRCF_t;

void jcNonStdRasMessageRCFPack( NonStdRasMessageRCF_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageRCFUnpack( NonStdRasMessageRCF_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;

}NonStdRasMessageRRJ_t;

void jcNonStdRasMessageRRJPack( NonStdRasMessageRRJ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageRRJUnpack( NonStdRasMessageRRJ_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;

}NonStdRasMessageURQ_t; /* can be sent both ways */

void jcNonStdRasMessageURQPack( NonStdRasMessageURQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageURQUnpack( NonStdRasMessageURQ_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;

}NonStdRasMessageUCF_t; /* can be sent both ways */

void jcNonStdRasMessageUCFPack( NonStdRasMessageUCF_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageUCFUnpack( NonStdRasMessageUCF_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszCellID;
   NSRASHANDLE hCell;

}NonStdRasMessageCellLRQ_t;

void jcNonStdRasMessageCellLRQPack( NonStdRasMessageCellLRQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageCellLRQUnpack( NonStdRasMessageCellLRQ_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszCellID;
   char* pszIpAddress;
   NSRASHANDLE hCell;

}NonStdRasMessageCellLCF_t;

void jcNonStdRasMessageCellLCFPack( NonStdRasMessageCellLCF_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageCellLCFUnpack( NonStdRasMessageCellLCF_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszCellID;
   NSRASHANDLE hCell;

}NonStdRasMessageCellLRJ_t;

void jcNonStdRasMessageCellLRJPack( NonStdRasMessageCellLRJ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageCellLRJUnpack( NonStdRasMessageCellLRJ_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;
   USHORT nProfileSize;
   PVOID pProfileData;

}NonStdRasMessageMobileProfile_t;

void jcNonStdRasMessageMobileProfilePack( NonStdRasMessageMobileProfile_t*,
                                          UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageMobileProfileUnpack( NonStdRasMessageMobileProfile_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszCellID;
   char* pszCellName;
   USHORT  sgsnRAC;    // shmin 09/17
   char*   pszSgsnIP;

}NonStdRasMessageCellInfo_t;

void jcNonStdRasMessageCellInfoPack( NonStdRasMessageCellInfo_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageCellInfoUnpack( NonStdRasMessageCellInfo_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;
   USHORT nMsgSize;
   PVOID pMsgData;

}NonStdRasMessageCISS_t;

void jcNonStdRasMessageMobileCISSPack( NonStdRasMessageCISS_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageMobileCISSUnpack( NonStdRasMessageCISS_t*, UCHAR** ppszBuffer );

typedef enum
{
   jcCellUnknown,
   jcCellDown,
   jcCellUp,

}NonStdRasCellMode_t;

typedef struct
{
   char* pszCellID;
   USHORT nMode;    /* see NonStdRasCellMode_t type */

}NonStdRasMessageCellSynchronize_t;

void jcNonStdRasMessageCellSynchronizePack( NonStdRasMessageCellSynchronize_t*,
                                            UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageCellSynchronizeUnpack( NonStdRasMessageCellSynchronize_t*,
                                              UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;

}NonStdRasMessageSRQ_t;

void jcNonStdRasMessageSRQPack( NonStdRasMessageSRQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageSRQUnpack( NonStdRasMessageSRQ_t*, UCHAR** ppszBuffer );

typedef struct
{
   char* pszMobileID;
   USHORT nAuthSets;          /* valid range: 1...SEC_MAX_TRIPLET (5), 0 indicates error */
   T_AUTH_TRIPLET* pAuthSets; /* pointer to array of T_AUTH_TRIPLET, NULL if nAuthSets == 0 */

}NonStdRasMessageSRS_t;

void jcNonStdRasMessageSRSPack( NonStdRasMessageSRS_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageSRSUnpack( NonStdRasMessageSRS_t*, UCHAR** ppszBuffer );

typedef NonStdRasMessageCISS_t NonStdRasMessageSMSPP_t;
void jcNonStdRasMessageMobileSmsppPack( NonStdRasMessageSMSPP_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageMobileSmsppUnpack( NonStdRasMessageSMSPP_t*, UCHAR** ppszBuffer );

/* local input messages */
typedef enum
{
   GmcCpAppInputMsgTypeTaskShutdown,

}GmcCpAppInputMsgType;

typedef struct
{
   GmcCpAppInputMsgType Type;
   /*
   union
   {

   };
   */

}GmcCpAppInputMsg_t;

BOOL MsgSendToGmcCpApp( GmcCpAppInputMsg_t* pMsg );

// MGPLA <chenj:08-06-01>
typedef struct
{
   UINT                       req_id;
   T_CNI_RIL3_IE_MOBILE_ID    imsi;
   USHORT                     channel_needed;
   UINT                       paging_timeout;
} NonStdRasMessagePageRequest_t;

void jcNonStdRasMessagePageRequestPack( NonStdRasMessagePageRequest_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePageRequestUnpack( NonStdRasMessagePageRequest_t*, UCHAR** ppszBuffer );

typedef struct
{
   UINT    req_id;
   bool    need_profile;
} NonStdRasMessagePageResponse_t;

void jcNonStdRasMessagePageResponsePack( NonStdRasMessagePageResponse_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessagePageResponseUnpack( NonStdRasMessagePageResponse_t*, UCHAR** ppszBuffer );


// Message Type for Intra L3 Messages
typedef enum {
  VBLINK_RR_PAGE_REQ
} VblinkMsgType_t;

union VblinkMsgData_t
{
  NonStdRasMessagePageRequest_t  pageReq;
};

// Message format between VBLINK and other subsystems
typedef struct {
  T_CNI_MODULE_ID     module_id;      // origin of a message 

  VblinkMsgType_t     message_type;
  VblinkMsgData_t     message_data;

} VblinkMsg_t;

#ifdef __cplusplus
}
#endif
