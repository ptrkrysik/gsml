/* exchange.h
 * $Id: exchange.h 1.4 1999/07/30 17:38:38 olet Exp $
 * Non Standard Interface between GK and H.323 task
 */

#ifdef __cplusplus
extern "C" {
#endif

#define H323_MANUFACTURER_CODE_JETCELL  63846  /* !! have to get real one */
#define H323_COUNTRY_CODE_USA  181

#define NON_STD_MSG_MAX_LEN   1024
#define NON_STD_RAS_PROTOCOL  1 /* to be cahnged when GK and H.323 task are no longer compatible */

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

   NonStdRasMessageTypeMobileProfileRequest,
   NonStdRasMessageTypeMobileProfile,

}NonStdRasMessageType;

typedef struct
{
   char* pszMobileID;
   NSRASHANDLE hMobileCell;

}NonStdRasMessageRRQ_t;

void jcNonStdRasMessageRRQPack( NonStdRasMessageRRQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageRRQUnpack( NonStdRasMessageRRQ_t*, UCHAR** ppszBuffer );

typedef struct
{
   NSRASHANDLE hMobileGK;
   NSRASHANDLE hMobileCell;
   char* pszNumber;

}NonStdRasMessageRCF_t;

void jcNonStdRasMessageRCFPack( NonStdRasMessageRCF_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageRCFUnpack( NonStdRasMessageRCF_t*, UCHAR** ppszBuffer );

typedef struct
{
   NSRASHANDLE hMobileCell;

}NonStdRasMessageRRJ_t;

void jcNonStdRasMessageRRJPack( NonStdRasMessageRRJ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageRRJUnpack( NonStdRasMessageRRJ_t*, UCHAR** ppszBuffer );

typedef struct
{
   NSRASHANDLE hMobileGK;
   NSRASHANDLE hMobileCell;

}NonStdRasMessageURQ_t; /* can be sent both ways */

void jcNonStdRasMessageURQPack( NonStdRasMessageURQ_t*, UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageURQUnpack( NonStdRasMessageURQ_t*, UCHAR** ppszBuffer );

typedef struct
{
   NSRASHANDLE hMobileGK;
   NSRASHANDLE hMobileCell;

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
   NSRASHANDLE hMobileGK;
   NSRASHANDLE hMobileCell;

}NonStdRasMessageMobileProfileRequest_t;

void jcNonStdRasMessageMobileProfileRequestPack( NonStdRasMessageMobileProfileRequest_t*,
                                                 UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageMobileProfileRequestUnpack( NonStdRasMessageMobileProfileRequest_t*,
                                                   UCHAR** ppszBuffer );

typedef struct
{
   NSRASHANDLE hMobileGK;
   NSRASHANDLE hMobileCell;
   USHORT nProfileSize;
   PVOID pProfileData;

}NonStdRasMessageMobileProfile_t;

void jcNonStdRasMessageMobileProfilePack( NonStdRasMessageMobileProfile_t*,
                                          UCHAR** ppszBuffer, int* pnSize );
void jcNonStdRasMessageMobileProfileUnpack( NonStdRasMessageMobileProfile_t*, UCHAR** ppszBuffer );


#ifdef __cplusplus
}
#endif
