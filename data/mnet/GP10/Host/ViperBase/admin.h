/* admin.h
 *
 */

#define JC_DATA_FILE_VERSION  2
/* 0 - initial version;
 * 1 - mobile subscriber profile added;
 * 2 - pszIVRNumber added
 */

typedef struct
{
   int nMaxCalls;
   int RASPort;
   int Q931Port;
   int RASResponseTO;
   int Q931ResponseTO;
   int Q931ConnectTO;
   PSTR pszGatekeeperID;
   int TelnetPort;
   PSTR pszIVRNumber;

   PLIST pListIPPhones; /* JCIPPHONE */
   PLIST pListGateways; /* JCGATEWAY */
   PLIST pListMobiles;  /* JCMOBILE */

}JCCONFIG, *PJCCONFIG;

typedef struct
{
   PSTR pszIMSI;
   PSTR pszNumber;
   PSTR pszComment;

   USHORT nProfileSize;
   void* ProfileData;

   gkHREG hReg;             /* has ViperCell the mobile currently belongs to */
   NSRASHANDLE hMobileCell; /* ViperCell's mobile handle */

}JCMOBILE, *PJCMOBILE;

typedef struct
{
   PSTR pszName;
   PSTR pszNumber;
   PSTR pszIPAddress;
   PSTR pszComment;

}JCIPPHONE, *PJCIPPHONE;

typedef struct
{
   PSTR pszName;
   PSTR pszPrefix;
   size_t nPrefixSize;
   PSTR pszIPAddress;
   BOOL bStripPrefix;
   PSTR pszComment;

}JCGATEWAY, *PJCGATEWAY;

typedef enum
{
   jcMngmCmdAdd,
   jcMngmCmdDelete,
   jcMngmCmdSet,
   jcMngmCmdGet,
   jcMngmCmdSave,
   jcMngmCmdImport

}jcMngmCmd_t;

/* "Named Proc" mechanism, calls function by name provided as text string */

typedef int (*pfnCommand_t)( int nParam, char* pszParam, PVOID pParam, char* pszResult );

typedef struct
{
   char* pszProc;
   pfnCommand_t pfnProc;

}JCNAMEDPROC, *PJCNAMEDPROC;

int jcProcCallByName( char* pszName, JCNAMEDPROC* pProcArray,
                      int nParam, char* pszParam, PVOID pParam, char* pszResult );

BOOL EndpointsPrint( void );
BOOL EndpointPrint( gkHREG );
char* TransAddrToString( gkaddTransport_t*, char* Buffer ); /* returns Buffer */

BOOL MobilesImport( char* pszFile, char* pszResult );
BOOL IpPhonesImport( char* pszFile, char* pszResult );
BOOL jcGatewaysImport( char* pszFile, char* pszResult );
BOOL MobileAdd( char* pszIMSI, char* pszNumber, char* pszComment,
                USHORT nProfileSize, void* ProfileData, char* pszResult );
BOOL MobileDelete( PJCMOBILE, PNODE, char* pszResult );
BOOL IpPhoneAdd( char* pszName, char* pszNumber, char* pszQ931Address,
                 char* pszComment, char* pszResult );
BOOL IpPhoneDelete( PJCIPPHONE, PNODE pNode, char* pszResult );
BOOL jcGatewayAdd( char* pszName, char* pszPrefix, char* pszIPAddress,
                   BOOL bStripPrefix, char* pszComment, char* pszResult );
BOOL jcGatewayDelete( PJCGATEWAY, PNODE pNode, char* pszResult );
PJCMOBILE MobileFindByIMSI( char* pszName, PNODE* );
PJCMOBILE MobileFindByNumber( char* pszNumber );
PJCIPPHONE IpPhoneFindByName( char* pszName, PNODE* );
PJCIPPHONE IpPhoneFindByNumber( char* pszNumber );
PJCGATEWAY jcGatewayFindByName( char* pszName, PNODE* );
PJCGATEWAY jcGatewayFindByAddress( UINT32 ip, PNODE* );
PJCGATEWAY jcGatewayFindByNumber( char* pszNumber );
BOOL jcNumberExists( char* pszNumber );

/* parsing mngm commands */
void MngmCmdCallBack( pMngmCommandData_t );
int MngmCmdNotImplemented( int nParam, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdNoValue( char* pszResult );
int MngmCmdValueSetOK( char* pszResult );
int MngmCmdIntGet( int nSource, char* pszResult );
int MngmCmdIntSet( int* pnTarget, int nMin, int nMax, char* pszParam, char* pszResult );
int MngmCmdStringGet( char* pszSource, char* pszResult );
int MngmCmdStringSet( char** ppszTarget, char* pszParam, char* pszResult );

int MngmCmdAdd( int nParam, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdDelete( int nParam, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGet( int nParam, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdSet( int nParam, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdSave( int nParam, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdImport( int nParam, char* pszParam, PVOID pParam, char* pszResult );

int MngmCmdRootObjs( int nCommand, char* pszParam, char* pszResult );
int MngmCmdMaxCalls( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdRASPort( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdQ931Port( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdRASResponseTimeout( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdQ931ResponseTimeout( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdQ931ConnectTimeout( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGatekeeperID( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdTelnetPort( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdIVRNumber( int nCommand, char* pszParam, PVOID pParam, char* pszResult );

int MngmCmdMobiles( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdMobile( char* pszIMSI, int nCommand, char* pszParam, char* pszResult );
int MngmCmdMobileIMSI( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdMobileNumber( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdMobileComment( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdMobileGetAllParams( PJCMOBILE, char* pszResult );

int MngmCmdIpPhones( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdIpPhone( char* pszName, int nCommand, char* pszParam, char* pszResult );
int MngmCmdIpPhoneName( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdIpPhoneNumber( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdIpPhoneIPAddress( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdIpPhoneComment( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdIpPhoneGetAllParams( PJCIPPHONE, char* pszResult );

int MngmCmdGateways( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGateway( char* pszName, int nCommand, char* pszParam, char* pszResult );
int MngmCmdGatewayName( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGatewayPrefix( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGatewayIPAddress( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGatewayStripPrefix( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGatewayComment( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdGatewayGetAllParams( PJCGATEWAY, char* pszResult );

int MngmCmdViperCells( int nCommand, char* pszParam, PVOID pParam, char* pszResult );
int MngmCmdViperCellGetAllParams( gkHREG, char* pszResult );

/* Data File I/O */
BOOL jcFileWriteBuf( FILE* hFile, UCHAR* buf, int nSize );
BOOL jcFileReadBuf( FILE* hFile, UCHAR* buf, int nSize );
BOOL jcFileWriteString( FILE* hFile, char* pszString );
char* jcFileReadString( FILE* hFile, char* pszString, int nMaxSize );
BOOL jcFileWriteULong( FILE* hFile, ULONG );
ULONG jcFileReadULong( FILE* hFile );
BOOL jcFileWriteUShort( FILE* hFile, USHORT );
USHORT jcFileReadUShort( FILE* hFile );

BOOL jcSaveIpPhones( FILE* hFile );
BOOL jcSaveGateways( FILE* hFile );
BOOL jcSaveMobiles( FILE* hFile );
BOOL jcReadIpPhones( FILE* hFile, USHORT nVersion );
BOOL jcReadGateways( FILE* hFile, USHORT nVersion );
BOOL jcReadMobiles( FILE* hFile, USHORT nVersion );
FILE* jcOpenConfigFile( char* pszFileName, USHORT* pnVersion );

extern PJCCONFIG pCfgTree;

/* Non-telnet management client API support */

#define MNGM_API_VERSION   0

typedef enum
{
   MngmMsgRequestGeneric = 0,
   MngmMsgResponseGeneric = 1,

   MngmMsgAlertGeneric = 100,

   MngmMsgRequestGetMobileProfile = 200,
   MngmMsgResponseGetMobileProfile = 201,
   MngmMsgRequestSetMobileProfile = 202,
   MngmMsgRequestAddMobileProfile = 203

}MngmMsg_t;

void MngmAPICallBack( pMngmCommandData_t );

BOOL jcMobileProfileGetParams( char* pszProfile, USHORT nProfileSize,
                               char* pszIMSI, char* pszNumber );
void jcMngmResponseGenericPack( pMngmCommandData_t, long nRequestID, short nResult, char* pszTextData );
