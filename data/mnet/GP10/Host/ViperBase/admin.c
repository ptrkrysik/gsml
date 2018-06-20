/* admin.c
 *
 */

#include <gkadd.h>
#include <gkrgi.h>
#include <gklrqi.h>
#include <gkstorage.h>
#include <cfgapi.h>
#include <li.h>

#include <SubscInfoType.h>

#include "support.h"
#include "exchange.h"
#include "admin.h"
#include "jcras.h"
#include "init.h"

static char pszEndpointPrintSpec[] = "%19s %6s %22s %22s %6s\n";

PJCCONFIG pCfgTree;

static char pszDelims[] = "\\/, ";
static char pszFieldSeparator[] = ", ";
static char pszLineSeparator[] = "\r\n";
static char pszDefaultDataFile[] = "jcvbase.dat";
static char pszDataFileHeader[] = "JetCell ViperBase Main Data File";

/*
 *
 */
BOOL EndpointsPrint( void )
{
   gkHREG hReg;
   gkHAPPREG haReg;

   StringPrint( "\n" );
   StringPrint( pszEndpointPrintSpec,
          "H323 ID", "Phone", "RAS Address", "Q.931 Address", "Online" );
   StringPrint( " ------------------------------------------------------------------------------\n" );

   switch( gkrgiGetFirstEntryHandle( &hReg, &haReg ) )
   {
      case GKRGI_OK:
         break;

      case GKRGI_NOT_FOUND:
         StringPrint( "There are no endpoints to show...\n" );
         return TRUE;

      default:
         DebugLog( "Error in gkrgiGetFirstEntryHandle()\n" );
         return FALSE;
   }

   do
   {
      EndpointPrint( hReg );

   }while( GKRGI_OK == gkrgiGetNextEntryHandle( hReg, &hReg, &haReg ) );


   return TRUE;
}

/*
 *
 */
BOOL EndpointPrint( gkHREG hReg )
{
   gkrgiParameterStatus_t nRegStatus;
   gkaddName_t Name; /* array */
   gkaddPhone_t Phone; /* array */
   gkaddTransport_t TransRAS, TransQ931;
   char pszRAS[32], pszQ931[32];
   int i, nPhones;

   if( GKRGI_OK != gkrgiMsgPending( hReg ) && !RegIsDynamic( hReg ) && !RegIsStatic( hReg ) )
      return TRUE;

   nRegStatus = RegIsDynamic( hReg ) ? gkrgiSttsDynamic : gkrgiSttsStatic;

   gkrgiGetParameter( hReg, gkrgiParamName, nRegStatus, 1, Name );

   gkrgiGetNumberOfAddresses( hReg, gkrgiParamPhone, nRegStatus, &nPhones );

   if( nPhones == 1 )
      gkrgiGetParameter( hReg, gkrgiParamPhone, nRegStatus, 1, Phone );
   else
      strcpy( Phone, ( nPhones < 1 ) ? "none" : "many" );

   gkrgiGetParameter( hReg, gkrgiParamCallSignal, nRegStatus, 1, &TransQ931 );
   gkrgiGetParameter( hReg, gkrgiParamRAS, nRegStatus, 1, &TransRAS );

   StringPrint( pszEndpointPrintSpec,
                Name,
                Phone,
                TransAddrToString( &TransRAS, pszRAS ),
                TransAddrToString( &TransQ931, pszQ931 ),
                RegIsDynamic( hReg ) ? "yes" : "no" );

   if( nPhones > 1 )
   {
      for( i = 1; i <= nPhones; i++ )
      {
         gkrgiGetParameter( hReg, gkrgiParamPhone, nRegStatus, i, Phone );
         StringPrint( ( i % 15 == 0 ) ? " %s\n" : " %s", Phone );
      }

      StringPrint( "\n" );
   }

/*
   nDataSize = 64;

   if( GKRGI_OK == gkrgiGetAppMemblock( hReg, &nDataSize, pszMobileUserData ) )
      StringPrint( "\t%s\n", pszMobileUserData );
*/
   return TRUE;
}

/*
 *
 */
char* TransAddrToString( gkaddTransport_t* pTransAddr, char* Buffer )
{
   char buf[16];

   sprintf( Buffer, "%s:%d", gkaddIp2String( pTransAddr->ip, buf ), pTransAddr->port );

   return Buffer;
}

/*
 *
 */
BOOL MobileAdd( char* pszIMSI, char* pszNumber, char* pszComment,
                USHORT nProfileSize, void* ProfileData, char* pszResult )
{
   PJCMOBILE pMobile;

   if( pszIMSI == NULL || pszNumber == NULL )
   {
      strcpy( pszResult, "Some parameters are not specified" );
      return FALSE;
   }

   if( MobileFindByIMSI( pszIMSI, NULL ) )
   {
      strcpy( pszResult, "Another mobile exists with the same IMSI" );
      return FALSE;
   }

   if( jcNumberExists( pszNumber ) )
   {
      strcpy( pszResult, "Another entity exists with the same number" );
      return FALSE;
   }

   DebugLog( "Adding mobile '%s', extension '%s'\n", pszIMSI, pszNumber );

   pMobile = New( JCMOBILE );
   pMobile->pszIMSI = StringCreate( pszIMSI );
   pMobile->pszNumber = StringCreate( pszNumber );
   pMobile->pszComment = StringCreate( pszComment );

   if( ( pMobile->nProfileSize = nProfileSize ) == 0 )
      pMobile->ProfileData = NULL;
   else
   {
      pMobile->ProfileData = SpecialAlloc( nProfileSize );
      memcpy( pMobile->ProfileData, ProfileData, nProfileSize );
   }

   pMobile->hReg = NULL;
   pMobile->hMobileCell = 0;

   ListAddTail( pCfgTree->pListMobiles, pMobile );

   sprintf( pszResult, "Mobile '%s' added", pszIMSI );
   return TRUE;
}

/*
 *
 */
BOOL MobileDelete( PJCMOBILE pMobile, PNODE pNode, char* pszResult )
{

   ListRemoveNode( pCfgTree->pListMobiles, pNode );

   if( pMobile->hReg != NULL )
   {
      gkrgiDeleteAddress( pMobile->hReg, gkrgiParamPhone, gkrgiSttsDynamic, pMobile->pszNumber );
      jcgkNonStdRasSendURQ( pMobile->hReg, pMobile );
   }

   StringDelete( pMobile->pszIMSI );
   StringDelete( pMobile->pszNumber );
   StringDelete( pMobile->pszComment );

   SpecialFree( pMobile->ProfileData );

   Delete( pMobile );

   strcpy( pszResult, "Mobile deleted" );
   return TRUE;
}

/*
 *
 */
BOOL IpPhoneDelete( PJCIPPHONE pIPPhone, PNODE pNode, char* pszResult )
{

   ListRemoveNode( pCfgTree->pListIPPhones, pNode );

   StringDelete( pIPPhone->pszName );
   StringDelete( pIPPhone->pszNumber );
   StringDelete( pIPPhone->pszIPAddress );
   StringDelete( pIPPhone->pszComment );
   Delete( pIPPhone );

   strcpy( pszResult, "IP phone deleted" );
   return TRUE;
}

/*
 *
 */
BOOL MobilesImport( char* pszFile, char* pszResult )
{
   FILE* pfd;
   char pszLine[80], pszIMSI[80], pszNumber[80];

   if( NULL == pszFile )
   {
      strcpy( pszResult, "No filename specified" );
      return FALSE;
   }

   if( NULL == ( pfd = fopen( pszFile, "rt" ) ) )
   {
      sprintf( pszResult, "Error opening file '%s'", pszFile );
      return FALSE;
   }

   while( fgets( pszLine, 80, pfd ) )
      if( 2 == sscanf( pszLine, " %s %s ", pszIMSI, pszNumber ) )
         MobileAdd( pszIMSI, pszNumber, NULL, 0, NULL, pszResult );

   fclose( pfd );

   strcpy( pszResult, "Mobiles import completed" );
   return TRUE;
}

/*
 *
 */
PJCMOBILE MobileFindByIMSI( char* pszIMSI, PNODE* ppNode )
{
   PNODE pNode;
   PJCMOBILE pMobile;

   ListForEachNode( pCfgTree->pListMobiles, pNode )
   {
      pMobile = (PJCMOBILE)NodeGetPtr( pNode );

      if( 0 == StringCompare( pMobile->pszIMSI, pszIMSI ) )
      {
         if( ppNode ) *ppNode = pNode;
         return pMobile;
      }
   }

   if( ppNode ) *ppNode = NULL;
   return NULL;
}

/*
 *
 */
PJCIPPHONE IpPhoneFindByName( char* pszName, PNODE* ppNode )
{
   PNODE pNode;
   PJCIPPHONE pIPPhone;

   ListForEachNode( pCfgTree->pListIPPhones, pNode )
   {
      pIPPhone = (PJCIPPHONE)NodeGetPtr( pNode );

      if( 0 == StringCompare( pIPPhone->pszName, pszName ) )
      {
         if( ppNode ) *ppNode = pNode;
         return pIPPhone;
      }
   }

   if( ppNode ) *ppNode = NULL;
   return NULL;
}

/*
 *
 */
PJCIPPHONE IpPhoneFindByNumber( char* pszNumber )
{
   PNODE pNode;
   PJCIPPHONE pIPPhone;

   ListForEachNode( pCfgTree->pListIPPhones, pNode )
   {
      pIPPhone = (PJCIPPHONE)NodeGetPtr( pNode );

      if( 0 == StringCompare( pIPPhone->pszNumber, pszNumber ) )
         return pIPPhone;
   }

   return NULL;
}

/*
 *
 */
PJCGATEWAY jcGatewayFindByName( char* pszName, PNODE* ppNode )
{
   PNODE pNode;
   PJCGATEWAY pGateway;

   ListForEachNode( pCfgTree->pListGateways, pNode )
   {
      pGateway = (PJCGATEWAY)NodeGetPtr( pNode );

      if( 0 == StringCompare( pGateway->pszName, pszName ) )
      {
         if( ppNode ) *ppNode = pNode;
         return pGateway;
      }
   }

   if( ppNode ) *ppNode = NULL;
   return NULL;
}

/*
 *
 */
PJCGATEWAY jcGatewayFindByAddress( UINT32 ip, PNODE* ppNode )
{
   PNODE pNode;
   PJCGATEWAY pGateway;
   char buf[16];

   gkaddIp2String( ip, buf );

   ListForEachNode( pCfgTree->pListGateways, pNode )
   {
      pGateway = (PJCGATEWAY)NodeGetPtr( pNode );

      if( 0 == StringCompare( pGateway->pszIPAddress, buf ) )
      {
         if( ppNode ) *ppNode = pNode;
         return pGateway;
      }
   }

   if( ppNode ) *ppNode = NULL;
   return NULL;
}

/*
 *
 */
PJCGATEWAY jcGatewayFindByNumber( char* Number )
{
   PNODE pNode;
   PJCGATEWAY pGateway;

   if( NULL == Number || Number[0] == '\0' )
      return NULL;

   ListForEachNode( pCfgTree->pListGateways, pNode )
   {
      pGateway = (PJCGATEWAY)NodeGetPtr( pNode );

      if( 0 == strncmp( Number, pGateway->pszPrefix, pGateway->nPrefixSize ) )
         return pGateway;
   }

   return NULL;
}

/*
 *
 */
PJCMOBILE MobileFindByNumber( char* pszNumber )
{
   PNODE pNode;
   PJCMOBILE pMobile;

   ListForEachNode( pCfgTree->pListMobiles, pNode )
   {
      pMobile = (PJCMOBILE)NodeGetPtr( pNode );

      if( 0 == StringCompare( pMobile->pszNumber, pszNumber ) )
         return pMobile;
   }

   return NULL;
}

/*
 *
 */
BOOL IpPhonesImport( char* pszFile, char* pszResult )
{
   FILE* pfd;
   char pszLine[80], pszName[80], pszNumber[80], pszQ931Address[80];

   if( NULL == pszFile )
   {
      strcpy( pszResult, "No filename specified" );
      return FALSE;
   }

   if( NULL == ( pfd = fopen( pszFile, "rt" ) ) )
   {
      sprintf( pszResult, "Error opening file '%s'", pszFile );
      return FALSE;
   }

   while( fgets( pszLine, 80, pfd ) )
      if( 3 == sscanf( pszLine, " %s %s %s ", pszName, pszNumber, pszQ931Address ) )
         IpPhoneAdd( pszName, pszNumber, pszQ931Address, NULL, pszResult );

   fclose( pfd );

   strcpy( pszResult, "IP phones import completed" );
   return TRUE;
}

/*
 *
 */
BOOL jcGatewaysImport( char* pszFile, char* pszResult )
{
   FILE* pfd;
   char pszLine[80], pszName[80], pszPrefix[80], pszQ931Address[80];

   if( NULL == pszFile )
   {
      strcpy( pszResult, "No filename specified" );
      return FALSE;
   }

   if( NULL == ( pfd = fopen( pszFile, "rt" ) ) )
   {
      sprintf( pszResult, "Error opening file '%s'", pszFile );
      return FALSE;
   }

   while( fgets( pszLine, 80, pfd ) )
      if( 3 == sscanf( pszLine, " %s %s %s ", pszName, pszPrefix, pszQ931Address ) )
         jcGatewayAdd( pszName, pszPrefix, pszQ931Address, FALSE, NULL, pszResult );

   fclose( pfd );

   strcpy( pszResult, "Gateways import completed" );
   return TRUE;
}

/*
 *
 */
BOOL IpPhoneAdd( char* pszName, char* pszNumber, char* pszQ931Address,
                 char* pszComment, char* pszResult )
{
   PJCIPPHONE pIpPhone;

   if( pszName == NULL || pszNumber == NULL || pszQ931Address == NULL )
   {
      strcpy( pszResult, "Some parameters are not specified" );
      return FALSE;
   }

   if( IpPhoneFindByName( pszName, NULL ) )
   {
      strcpy( pszResult, "Another IP phone exists with the same name" );
      return FALSE;
   }

   if( jcNumberExists( pszNumber ) )
   {
      strcpy( pszResult, "Another entity exists with the same number" );
      return FALSE;
   }

   DebugLog( "Adding IP phone '%s', number '%s', address '%s'\n",
             pszName, pszNumber, pszQ931Address );

   pIpPhone = New( JCIPPHONE );
   pIpPhone->pszName =      StringCreate( pszName );
   pIpPhone->pszNumber =    StringCreate( pszNumber );
   pIpPhone->pszIPAddress = StringCreate( pszQ931Address );
   pIpPhone->pszComment =   StringCreate( pszComment );

   ListAddTail( pCfgTree->pListIPPhones, pIpPhone );

   sprintf( pszResult, "IP phone '%s' added", pszName );
   return TRUE;
}

/*
 *
 */
BOOL jcGatewayAdd( char* pszName, char* pszPrefix, char* pszIPAddress,
                   BOOL bStripPrefix, char* pszComment, char* pszResult )
{
   PJCGATEWAY pGateway;

   if( pszName == NULL || pszPrefix == NULL || pszIPAddress == NULL )
   {
      strcpy( pszResult, "Some parameters are not specified" );
      return FALSE;
   }

   if( jcGatewayFindByName( pszName, NULL ) )
   {
      strcpy( pszResult, "Another gateway exists with the same name" );
      return FALSE;
   }

   if( jcGatewayFindByNumber( pszPrefix ) )
   {
     strcpy( pszResult, "Another gateway exists with the same prefix" );
     return FALSE;
   }

   DebugLog( "Adding gateway '%s', prefix '%s', address '%s'\n", pszName, pszPrefix, pszIPAddress );

   pGateway = New( JCGATEWAY );
   pGateway->pszName = StringCreate( pszName );
   pGateway->pszPrefix = StringCreate( pszPrefix );
   pGateway->nPrefixSize = strlen( pszPrefix );
   pGateway->pszIPAddress = StringCreate( pszIPAddress );
   pGateway->bStripPrefix = bStripPrefix;
   pGateway->pszComment = StringCreate( pszComment );

   ListAddTail( pCfgTree->pListGateways, pGateway );

   sprintf( pszResult, "Gateway '%s' added", pszName );
   return TRUE;
}

/*
 *
 */
BOOL jcGatewayDelete( PJCGATEWAY pGateway, PNODE pNode, char* pszResult )
{

   ListRemoveNode( pCfgTree->pListGateways, pNode );

   StringDelete( pGateway->pszName );
   StringDelete( pGateway->pszPrefix );
   StringDelete( pGateway->pszIPAddress );
   StringDelete( pGateway->pszComment );

   Delete( pGateway );

   strcpy( pszResult, "Gateway deleted" );
   return TRUE;
}

/*
 *
 */
void MngmCmdCallBack( pMngmCommandData_t pData )
{
   static JCNAMEDPROC Commands[] = {
   "Add",    MngmCmdAdd,
   "Delete", MngmCmdDelete,
   "Set",    MngmCmdSet,
   "Get",    MngmCmdGet,
   "Show",   MngmCmdGet,
   "Save",   MngmCmdSave,
   "Import", MngmCmdImport,
   NULL, NULL
   };

   char *pszCommand, *pszParams;

   if( NULL == ( pszCommand = StringToken( pData->pszInput, pszDelims, &pszParams ) ) )
   {
      strcpy( pData->pszOutput, "No command specified" );
      pData->nOutput = FALSE;
      return;
   }

   pData->nOutput = jcProcCallByName( pszCommand, Commands, 0, pszParams, NULL, pData->pszOutput );
   return;
}

/*
 *
 */
int jcProcCallByName( char* pszName, JCNAMEDPROC* pProcArray,
                      int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   for( ; pProcArray->pszProc; pProcArray++ )
   {
      if( 0 == StringICompare( pszName, pProcArray->pszProc ) )
         return (*pProcArray->pfnProc)( nParam, pszParam, pParam, pszResult );
   }

   sprintf( pszResult, "Can't recognize '%s'", pszName );
   return FALSE;
}

/*
 *
 */
int MngmCmdNotImplemented( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   strcpy( pszResult, "This command is not implemented" );
   return FALSE;
}

/*
 *
 */
int MngmCmdNoValue( char* pszResult )
{
   strcpy( pszResult, "The value is not specified" );
   return FALSE;
}

/*
 *
 */
int MngmCmdValueSetOK( char* pszResult )
{
   strcpy( pszResult, "The value was changed" );
   return TRUE;
}

/*
 *
 */
int MngmCmdAdd( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   return MngmCmdRootObjs( jcMngmCmdAdd, pszParam, pszResult );
}

/*
 *
 */
int MngmCmdDelete( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   return MngmCmdRootObjs( jcMngmCmdDelete, pszParam, pszResult );
}

/*
 *
 */
int MngmCmdSet( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   return MngmCmdRootObjs( jcMngmCmdSet, pszParam, pszResult );
}

/*
 *
 */
int MngmCmdGet( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   return MngmCmdRootObjs( jcMngmCmdGet, pszParam, pszResult );
}

/*
 *
 */
int MngmCmdImport( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   return MngmCmdRootObjs( jcMngmCmdImport, pszParam, pszResult );
}

/*
 * finds the object in configuration tree by path, calls appropriate function
 */
int MngmCmdRootObjs( int nCommand, char* pszParam, char* pszResult )
{
   static JCNAMEDPROC RootObjs[] = {
   "MaxCalls",            MngmCmdMaxCalls,
   "RASPort",             MngmCmdRASPort,
   "Q931Port",            MngmCmdQ931Port,
   "RASResponseTimeout",  MngmCmdRASResponseTimeout,
   "Q931ResponseTimeout", MngmCmdQ931ResponseTimeout,
   "Q931ConnectTimeout",  MngmCmdQ931ConnectTimeout,
   "GatekeeperID",        MngmCmdGatekeeperID,
   "TelnetPort",          MngmCmdTelnetPort,
   "IVRNumber",           MngmCmdIVRNumber,
   "IPPhones",            MngmCmdIpPhones,
   "Gateways",            MngmCmdGateways,
   "Mobiles",             MngmCmdMobiles,
   "ViperCells",          MngmCmdViperCells,
   NULL, NULL };

   char *pszObjName;

   if( NULL == ( pszObjName = StringToken( NULL, pszDelims, &pszParam ) ) )
   {
      strcpy( pszResult, "No configuration object specified" );
      return FALSE;
   }
   else
      return jcProcCallByName( pszObjName, RootObjs, nCommand, pszParam, NULL, pszResult );
}

/*
 *
 */
int MngmCmdMaxCalls( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->nMaxCalls, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->nMaxCalls, 1, 200, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdRASPort( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->RASPort, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->RASPort, 1025, 65535, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdQ931Port( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->Q931Port, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->Q931Port, 1025, 65535, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdRASResponseTimeout( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->RASResponseTO, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->RASResponseTO, 5, 360, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdQ931ResponseTimeout( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->Q931ResponseTO, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->Q931ResponseTO, 5, 360, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdQ931ConnectTimeout( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->Q931ConnectTO, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->Q931ConnectTO, 5, 360, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdTelnetPort( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdIntGet( pCfgTree->TelnetPort, pszResult );

      case jcMngmCmdSet:
         return MngmCmdIntSet( &pCfgTree->TelnetPort, 1025, 65535, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGatekeeperID( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pCfgTree->pszGatekeeperID, pszResult );

      case jcMngmCmdSet:
         return MngmCmdStringSet( &pCfgTree->pszGatekeeperID, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdIVRNumber( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pCfgTree->pszIVRNumber, pszResult );

      case jcMngmCmdSet:
      {
         char* pszNumber;

         if( NULL != ( pszNumber = StringToken( NULL, pszDelims, &pszParam ) ) &&
            jcNumberExists( pszNumber ) )
         {
            strcpy( pszResult, "Another entity exists with the same number" );
            return FALSE;
         }

         StringReplace( &pCfgTree->pszIVRNumber, pszNumber ); /* NULL is OK */

         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdIntGet( int nSource, char* pszResult )
{
   sprintf( pszResult, "%d", nSource );
   return TRUE;
}

/*
 *
 */
int MngmCmdIntSet( int* pnTarget, int nMin, int nMax, char* pszParam, char* pszResult )
{
   int nValue;
   char* pszValue;

   if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
      return MngmCmdNoValue( pszResult );

   nValue = atoi( pszValue );

   if( nValue < nMin || nValue > nMax )
   {
      sprintf( pszResult, "The value is out of valid range (%d...%d)", nMin, nMax );
      return FALSE;
   }

   *pnTarget = nValue;

   return MngmCmdValueSetOK( pszResult );
}

/*
 *
 */
int MngmCmdStringGet( char* pszSource, char* pszResult )
{
   if( pszSource )
   {
      strcpy( pszResult, pszSource );
      return TRUE;
   }
   else
   {
      strcpy( pszResult, "<no current value>" );
      return FALSE;
   }
}

/*
 *
 */
int MngmCmdStringSet( char** ppszTarget, char* pszParam, char* pszResult )
{
   char* pszValue;

   if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
      return MngmCmdNoValue( pszResult );

   StringReplace( ppszTarget, pszValue );

   return MngmCmdValueSetOK( pszResult );
}

/*
 *
 */
int MngmCmdMobiles( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   char *pszIMSI, *pszNumber, *pszComment;
   PNODE pNode;
   char buf[128];

   pszIMSI = StringToken( NULL, pszDelims, &pszParam );

   switch( nCommand )
   {
      case jcMngmCmdAdd:
         pszNumber  = StringToken( NULL, pszDelims, &pszParam );
         pszComment = StringToken( NULL, pszDelims, &pszParam );
         return MobileAdd( pszIMSI, pszNumber, pszComment, 0, NULL, pszResult );

      case jcMngmCmdGet:
         if( pszIMSI == NULL ) /* mobiles list */
         {
            pszResult[0] = '\0';

            ListForEachNode( pCfgTree->pListMobiles, pNode )
            {
               MngmCmdMobileGetAllParams( (PJCMOBILE)NodeGetPtr( pNode ), buf );
               strcat( pszResult, buf );
               strcat( pszResult, pszLineSeparator );
            }

            return TRUE;
         }
         else
            return MngmCmdMobile( pszIMSI, nCommand, pszParam, pszResult );

      case jcMngmCmdDelete:
      case jcMngmCmdSet:
         if( pszIMSI == NULL )
         {
            strcpy( pszResult, "No IMSI specified for the mobile" );
            return FALSE;
         }
         else
            return MngmCmdMobile( pszIMSI, nCommand, pszParam, pszResult );

      case jcMngmCmdImport:
         return MobilesImport( pszIMSI /* pszIMSI has filename in this case */, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdMobile( char* pszIMSI, int nCommand, char* pszParam, char* pszResult )
{
   static JCNAMEDPROC MobileObjs[] = {
   "IMSI",    MngmCmdMobileIMSI,
   /* "Number",  MngmCmdMobileNumber, blocked to avoid inconsistency with profile */
   "Comment", MngmCmdMobileComment,
   NULL, NULL };

   PJCMOBILE pMobile;
   PNODE pNode;
   char* pszObj;

   if( NULL == ( pMobile = MobileFindByIMSI( pszIMSI, &pNode ) ) )
   {
      sprintf( pszResult, "Mobile '%s' not found", pszIMSI );
      return FALSE;
   }

   pszObj = StringToken( NULL, pszDelims, &pszParam );

   switch( nCommand )
   {
      case jcMngmCmdDelete:
         return MobileDelete( pMobile, pNode, pszResult );

      case jcMngmCmdGet:
         if( pszObj == NULL ) /* get all parameters for this mobile */
            return MngmCmdMobileGetAllParams( pMobile, pszResult );
         else
            return jcProcCallByName( pszObj, MobileObjs, nCommand, pszParam, pMobile, pszResult );

      case jcMngmCmdSet:
         if( pszObj == NULL )
         {
            strcpy( pszResult, "No configuration object specified for the mobile" );
            return FALSE;
         }
         else
            return jcProcCallByName( pszObj, MobileObjs, nCommand, pszParam, pMobile, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdMobileGetAllParams( PJCMOBILE pMobile, char* pszResult )
{
   char *pszCellID, *pszComment;

   if( pMobile->hReg == NULL )
      pszCellID = "<offline>";
   else
      pszCellID = jcRegGetNames( pMobile->hReg );

   pszComment = pMobile->pszComment ? pMobile->pszComment : "<no comment>";

   sprintf( pszResult, "%s%s%s%s%s%s%s",
            pMobile->pszIMSI, pszFieldSeparator,
            pMobile->pszNumber, pszFieldSeparator,
            pszComment, pszFieldSeparator,
            pszCellID );

   return TRUE;
}

/*
 *
 */
int MngmCmdMobileIMSI( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCMOBILE pMobile = (PJCMOBILE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pMobile->pszIMSI, pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         if( MobileFindByIMSI( pszValue, NULL ) )
         {
            strcpy( pszResult, "Another mobile exists with the same IMSI" );
            return FALSE;
         }

         StringReplace( &pMobile->pszIMSI, pszValue );
         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdMobileNumber( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCMOBILE pMobile = (PJCMOBILE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pMobile->pszNumber, pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         if( jcNumberExists( pszValue ) )
         {
            strcpy( pszResult, "Another entity exists with the same number" );
            return FALSE;
         }

         if( pMobile->hReg != NULL )
         {
            gkrgiDeleteAddress( pMobile->hReg, gkrgiParamPhone, gkrgiSttsDynamic,
                                pMobile->pszNumber );
            gkrgiAddAddress(    pMobile->hReg, gkrgiParamPhone, gkrgiSttsDynamic, pszValue );
         }

         StringReplace( &pMobile->pszNumber, pszValue );
         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdMobileComment( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCMOBILE pMobile = (PJCMOBILE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pMobile->pszComment, pszResult );

      case jcMngmCmdSet:
         return MngmCmdStringSet( &pMobile->pszComment, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdSave( int nParam, char* pszParam, PVOID pParam, char* pszResult )
{
   char* pszFileName;
   FILE* hFile;

   if( NULL == ( pszFileName = StringToken( NULL, pszDelims, &pszParam ) ) )
      pszFileName = pszDefaultDataFile;

   if( NULL == ( hFile = fopen( pszFileName, "wb" ) ) )
   {
      sprintf( pszResult, "Unable to open file '%s'", pszFileName );
      return FALSE;
   }

   if( FALSE == jcFileWriteString( hFile, pszDataFileHeader ) ||
       FALSE == jcFileWriteUShort( hFile, JC_DATA_FILE_VERSION ) ||

       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->nMaxCalls ) ||
       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->RASPort ) ||
       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->Q931Port ) ||
       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->RASResponseTO ) ||
       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->Q931ResponseTO ) ||
       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->Q931ConnectTO ) ||
       FALSE == jcFileWriteString( hFile, pCfgTree->pszGatekeeperID ) ||
       FALSE == jcFileWriteUShort( hFile, (USHORT)pCfgTree->TelnetPort ) ||
       FALSE == jcFileWriteString( hFile, pCfgTree->pszIVRNumber ) ||

       FALSE == jcSaveIpPhones( hFile ) ||
       FALSE == jcSaveGateways( hFile ) ||
       FALSE == jcSaveMobiles( hFile ) )
   {
      fclose( hFile );
      sprintf( pszResult, "Error to write to file '%s'", pszFileName );
      return FALSE;
   }

   fclose( hFile );
   strcpy( pszResult, "Configuration saved" );
   return TRUE;
}

/*
 *
 */
BOOL jcFileWriteBuf( FILE* hFile, UCHAR* buf, int nSize )
{
   return 1 == fwrite( buf, nSize, 1, hFile );
}

/*
 *
 */
BOOL jcFileReadBuf( FILE* hFile, UCHAR* buf, int nSize )
{
   return 1 == fread( buf, nSize, 1, hFile );

}

/*
 *
 */
BOOL jcFileWriteString( FILE* hFile, char* pszString )
{
   int nSize;

   if( NULL == pszString )
      nSize = 0;
   else
      nSize = strlen( pszString );

   if( FALSE == jcFileWriteUShort( hFile, (USHORT)nSize ) )
      return FALSE;

   if( nSize > 0 )
      return jcFileWriteBuf( hFile, (UCHAR*)pszString, nSize );
   else
      return TRUE;
}

/*
 *
 */
char* jcFileReadString( FILE* hFile, char* pszString, int nMaxSize )
{
   int nSize;

   if( 0 == ( nSize = jcFileReadUShort( hFile ) ) ||
       nSize >= nMaxSize ||
       FALSE == jcFileReadBuf( hFile, (UCHAR*)pszString, nSize ) )
   {
      pszString[0] = '\0';
      return NULL;
   }

   pszString[nSize] = '\0';
   return pszString;
}

/*
 *
 */
BOOL jcFileWriteULong( FILE* hFile, ULONG uLong )
{
   UCHAR pszBuffer[4];

   pszBuffer[0] = (UCHAR)( ( uLong >> 0  ) & 0xFF );
   pszBuffer[1] = (UCHAR)( ( uLong >> 8  ) & 0xFF );
   pszBuffer[2] = (UCHAR)( ( uLong >> 16 ) & 0xFF );
   pszBuffer[3] = (UCHAR)( ( uLong >> 24 ) & 0xFF );

   return jcFileWriteBuf( hFile, pszBuffer, 4 );
}

/*
 *
 */
ULONG jcFileReadULong( FILE* hFile )
{
   UCHAR pszBuffer[4];

   if( FALSE == jcFileReadBuf( hFile, pszBuffer, 4 ) )
      return 0;
   else
      return ( (ULONG)pszBuffer[0] << 0  ) |
             ( (ULONG)pszBuffer[1] << 8  ) |
             ( (ULONG)pszBuffer[2] << 16 ) |
             ( (ULONG)pszBuffer[3] << 24 );
}

/*
 *
 */
BOOL jcFileWriteUShort( FILE* hFile, USHORT uShort )
{
   UCHAR pszBuffer[2];

   pszBuffer[0] = (UCHAR)( ( uShort >> 0  ) & 0xFF );
   pszBuffer[1] = (UCHAR)( ( uShort >> 8  ) & 0xFF );

   return jcFileWriteBuf( hFile, pszBuffer, 2 );
}

/*
 *
 */
USHORT jcFileReadUShort( FILE* hFile )
{
   UCHAR pszBuffer[2];

   if( FALSE == jcFileReadBuf( hFile, pszBuffer, 2 ) )
      return 0;
   else
      return ( (USHORT)pszBuffer[0] << 0  ) |
             ( (USHORT)pszBuffer[1] << 8  );
}

/*
 *
 */
BOOL jcSaveIpPhones( FILE* hFile )
{
   PNODE pNode;
   PJCIPPHONE pIPPhone;

   if( FALSE == jcFileWriteULong( hFile, ListGetCount( pCfgTree->pListIPPhones ) ) )
      return FALSE;

   ListForEachNode( pCfgTree->pListIPPhones, pNode )
   {
      pIPPhone = (PJCIPPHONE)NodeGetPtr( pNode );

      if( FALSE == jcFileWriteString( hFile, pIPPhone->pszName ) ||
          FALSE == jcFileWriteString( hFile, pIPPhone->pszNumber ) ||
          FALSE == jcFileWriteString( hFile, pIPPhone->pszIPAddress ) ||
          FALSE == jcFileWriteString( hFile, pIPPhone->pszComment ) )
         return FALSE;
   }

   return TRUE;
}

/*
 *
 */
BOOL jcSaveGateways( FILE* hFile )
{
   PNODE pNode;
   PJCGATEWAY pGateway;

   if( FALSE == jcFileWriteULong( hFile, ListGetCount( pCfgTree->pListGateways ) ) )
      return FALSE;

   ListForEachNode( pCfgTree->pListGateways, pNode )
   {
      pGateway = (PJCGATEWAY)NodeGetPtr( pNode );

      if( FALSE == jcFileWriteString( hFile, pGateway->pszName ) ||
          FALSE == jcFileWriteString( hFile, pGateway->pszPrefix ) ||
          FALSE == jcFileWriteString( hFile, pGateway->pszIPAddress ) ||
          FALSE == jcFileWriteUShort( hFile, (USHORT)pGateway->bStripPrefix ) ||
          FALSE == jcFileWriteString( hFile, pGateway->pszComment ) )
         return FALSE;
   }

   return TRUE;
}

/*
 *
 */
BOOL jcSaveMobiles( FILE* hFile )
{
   PNODE pNode;
   PJCMOBILE pMobile;

   if( FALSE == jcFileWriteULong( hFile, ListGetCount( pCfgTree->pListMobiles ) ) )
      return FALSE;

   ListForEachNode( pCfgTree->pListMobiles, pNode )
   {
      pMobile = (PJCMOBILE)NodeGetPtr( pNode );

      if( FALSE == jcFileWriteString( hFile, pMobile->pszIMSI ) ||
          FALSE == jcFileWriteString( hFile, pMobile->pszNumber ) ||
          FALSE == jcFileWriteString( hFile, pMobile->pszComment ) ||
          FALSE == jcFileWriteUShort( hFile, pMobile->nProfileSize ) )
         return FALSE;

      if( pMobile->nProfileSize > 0 )
         if( FALSE == jcFileWriteBuf( hFile, (UCHAR*)pMobile->ProfileData, pMobile->nProfileSize ) )
            return FALSE;
   }

   return TRUE;
}

/*
 *
 */
FILE* jcOpenConfigFile( char* pszFileName, USHORT* pnVersion )
{
   FILE* hFile;
   char pszBuffer[128];

   if( NULL == pszFileName ) pszFileName = pszDefaultDataFile;

   if( NULL == ( hFile = fopen( pszFileName, "rb" ) ) )
   {
      StringPrint( "Unable to open file '%s'\n", pszFileName );
      return NULL;
   }

   if( NULL == jcFileReadString( hFile, pszBuffer, 128 ) ||
       0 != StringCompare( pszBuffer, pszDataFileHeader ) )
   {
      fclose( hFile );
      StringPrint( "The file '%s' is not %s\n", pszFileName, pszDataFileHeader );
      return NULL;
   }

   if( ( *pnVersion = jcFileReadUShort( hFile ) ) > JC_DATA_FILE_VERSION )
   {
      fclose( hFile );
      StringPrint( "Main Data File Version: %d, Supported: %d\n",
                   *pnVersion, JC_DATA_FILE_VERSION );
      return NULL;
   }
   else
      return hFile;
}

/*
 *
 */
BOOL jcReadIpPhones( FILE* hFile, USHORT nVersion )
{
   char Name[256], Number[256], IPAddress[256], Comment[256], Result[256];
   char *pszName, *pszNumber, *pszIPAddress, *pszComment;
   ULONG nCount;

   nCount = jcFileReadULong( hFile );

   while( ( nCount-- ) > 0 )
   {
      pszName = jcFileReadString( hFile, Name, 256 );
      pszNumber = jcFileReadString( hFile, Number, 256 );
      pszIPAddress = jcFileReadString( hFile, IPAddress, 256 );
      pszComment = jcFileReadString( hFile, Comment, 256 );

      IpPhoneAdd( pszName, pszNumber, pszIPAddress, pszComment, Result );
   }

   return TRUE;
}

/*
 *
 */
BOOL jcReadGateways( FILE* hFile, USHORT nVersion )
{
   char Name[256], Prefix[256], IPAddress[256], Comment[256], Result[256];
   char *pszName, *pszPrefix, *pszIPAddress, *pszComment; 

   BOOL bStripPrefix;
   ULONG nCount;

   nCount = jcFileReadULong( hFile );

   while( ( nCount-- ) > 0 )
   {
      pszName      = jcFileReadString( hFile, Name, 256 );
      pszPrefix    = jcFileReadString( hFile, Prefix, 256 );
      pszIPAddress = jcFileReadString( hFile, IPAddress, 256 );
      bStripPrefix = jcFileReadUShort( hFile );
      pszComment   = jcFileReadString( hFile, Comment, 256 );

      jcGatewayAdd( pszName, pszPrefix, pszIPAddress, bStripPrefix, pszComment, Result );
   }

   return TRUE;
}

/*
 *
 */
BOOL jcReadMobiles( FILE* hFile, USHORT nVersion )
{
   char IMSI[256], Number[256], Comment[256], Result[256], Buffer[16*1024];
   char *pszIMSI, *pszNumber, *pszComment;
   ULONG nCount;
   USHORT nProfileSize;

   nCount = jcFileReadULong( hFile );

   while( ( nCount-- ) > 0 )
   {
      pszIMSI    = jcFileReadString( hFile, IMSI, 256 );
      pszNumber  = jcFileReadString( hFile, Number, 256 );
      pszComment = jcFileReadString( hFile, Comment, 256 );

      if( nVersion > 0 && ( nProfileSize = jcFileReadUShort( hFile ) ) > 0 )
      {
         jcFileReadBuf( hFile, (UCHAR*)Buffer, nProfileSize );
         MobileAdd( pszIMSI, pszNumber, pszComment, nProfileSize, Buffer, Result );
      }
      else
      {
         MobileAdd( pszIMSI, pszNumber, pszComment, 0, NULL, Result );
      }
   }

   return TRUE;
}

/*
 *
 */
int MngmCmdIpPhones( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   char *pszName, *pszNumber, *pszIPAddress, *pszComment;
   PNODE pNode;
   char buf[128];

   pszName = StringToken( NULL, pszDelims, &pszParam );

   switch( nCommand )
   {
      case jcMngmCmdAdd:
         pszNumber  = StringToken( NULL, pszDelims, &pszParam );
         pszIPAddress = StringToken( NULL, pszDelims, &pszParam );
         pszComment = StringToken( NULL, pszDelims, &pszParam );
         return IpPhoneAdd( pszName, pszNumber, pszIPAddress, pszComment, pszResult );

      case jcMngmCmdGet:
         if( pszName == NULL ) /* IP Phones list */
         {
            pszResult[0] = '\0';

            ListForEachNode( pCfgTree->pListIPPhones, pNode )
            {
               MngmCmdIpPhoneGetAllParams( (PJCIPPHONE)NodeGetPtr( pNode ), buf );
               strcat( pszResult, buf );
               strcat( pszResult, pszLineSeparator );
            }

            return TRUE;
         }
         else
            return MngmCmdIpPhone( pszName, nCommand, pszParam, pszResult );

      case jcMngmCmdDelete:
      case jcMngmCmdSet:
         if( pszName == NULL )
         {
            strcpy( pszResult, "No name specified for the IP phone" );
            return FALSE;
         }
         else
            return MngmCmdIpPhone( pszName, nCommand, pszParam, pszResult );

      case jcMngmCmdImport:
         return IpPhonesImport( pszName /* pszName has filename in this case */, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdIpPhoneGetAllParams( PJCIPPHONE pIPPhone, char* pszResult )
{
   sprintf( pszResult, "%s%s%s%s%s%s%s",
            pIPPhone->pszName, pszFieldSeparator,
            pIPPhone->pszNumber, pszFieldSeparator,
            pIPPhone->pszIPAddress, pszFieldSeparator,
            pIPPhone->pszComment ? pIPPhone->pszComment : "<no comment>" );
   return TRUE;
}

/*
 *
 */
int MngmCmdIpPhone( char* pszName, int nCommand, char* pszParam, char* pszResult )
{
   static JCNAMEDPROC IpPhoneObjs[] = {
   "Name",      MngmCmdIpPhoneName,
   "Number",    MngmCmdIpPhoneNumber,
   "IPAddress", MngmCmdIpPhoneIPAddress,
   "Comment",   MngmCmdIpPhoneComment,
   NULL, NULL };

   PJCIPPHONE pIPPhone;
   PNODE pNode;
   char* pszObj;

   if( NULL == ( pIPPhone = IpPhoneFindByName( pszName, &pNode ) ) )
   {
      sprintf( pszResult, "IP phone '%s' not found", pszName );
      return FALSE;
   }

   pszObj = StringToken( NULL, pszDelims, &pszParam );

   switch( nCommand )
   {
      case jcMngmCmdDelete:
         return IpPhoneDelete( pIPPhone, pNode, pszResult );

      case jcMngmCmdGet:
         if( pszObj == NULL ) /* get all parameters for this IP Phone */
            return MngmCmdIpPhoneGetAllParams( pIPPhone, pszResult );
         else
            return jcProcCallByName( pszObj, IpPhoneObjs, nCommand, pszParam, pIPPhone, pszResult );

      case jcMngmCmdSet:
         if( pszObj == NULL )
         {
            strcpy( pszResult, "No configuration object specified for the IP phone" );
            return FALSE;
         }
         else
            return jcProcCallByName( pszObj, IpPhoneObjs, nCommand, pszParam, pIPPhone, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdIpPhoneName( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCIPPHONE pIPPhone = (PJCIPPHONE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pIPPhone->pszName, pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         if( IpPhoneFindByName( pszValue, NULL ) )
         {
            strcpy( pszResult, "Another IP phone exists with the same name" );
            return FALSE;
         }

         StringReplace( &pIPPhone->pszName, pszValue );
         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdIpPhoneNumber( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCIPPHONE pIPPhone = (PJCIPPHONE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pIPPhone->pszNumber, pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         if( jcNumberExists( pszValue ) )
         {
            strcpy( pszResult, "Another entity exists with the same number" );
            return FALSE;
         }

         StringReplace( &pIPPhone->pszNumber, pszValue );
         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}
/*
 *
 */
int MngmCmdIpPhoneIPAddress( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCIPPHONE pIPPhone = (PJCIPPHONE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pIPPhone->pszIPAddress, pszResult );

      case jcMngmCmdSet:
         return MngmCmdStringSet( &pIPPhone->pszIPAddress, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdIpPhoneComment( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCIPPHONE pIPPhone = (PJCIPPHONE)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pIPPhone->pszComment, pszResult );

      case jcMngmCmdSet:
         return MngmCmdStringSet( &pIPPhone->pszComment, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGateways( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   char *pszName, *pszPrefix, *pszIPAddress, *pszStripPrefix, *pszComment;
   BOOL bStripPrefix;
   PNODE pNode;
   char buf[128];

   pszName = StringToken( NULL, pszDelims, &pszParam );

   switch( nCommand )
   {
      case jcMngmCmdAdd:
         pszPrefix      = StringToken( NULL, pszDelims, &pszParam );
         pszIPAddress   = StringToken( NULL, pszDelims, &pszParam );
         pszStripPrefix = StringToken( NULL, pszDelims, &pszParam );
         pszComment     = StringToken( NULL, pszDelims, &pszParam );

         bStripPrefix = ( 0 == StringICompare( pszStripPrefix, "yes" ) );

         return jcGatewayAdd( pszName, pszPrefix, pszIPAddress,
                              bStripPrefix, pszComment, pszResult );

      case jcMngmCmdGet:
         if( pszName == NULL ) /* gateways list */
         {
            pszResult[0] = '\0';

            ListForEachNode( pCfgTree->pListGateways, pNode )
            {
               MngmCmdGatewayGetAllParams( (PJCGATEWAY)NodeGetPtr( pNode ), buf );
               strcat( pszResult, buf );
               strcat( pszResult, pszLineSeparator );
            }

            return TRUE;
         }
         else
            return MngmCmdGateway( pszName, nCommand, pszParam, pszResult );

      case jcMngmCmdDelete:
      case jcMngmCmdSet:
         if( pszName == NULL )
         {
            strcpy( pszResult, "No name specified for the gateway" );
            return FALSE;
         }
         else
            return MngmCmdGateway( pszName, nCommand, pszParam, pszResult );

      case jcMngmCmdImport:
         return jcGatewaysImport( pszName /* pszName has filename in this case */, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGateway( char* pszName, int nCommand, char* pszParam, char* pszResult )
{
   static JCNAMEDPROC GatewayObjs[] = {
   "Name",        MngmCmdGatewayName,
   "Prefix",      MngmCmdGatewayPrefix,
   "IPAddress",   MngmCmdGatewayIPAddress,
   "StripPrefix", MngmCmdGatewayStripPrefix,
   "Comment",     MngmCmdGatewayComment,
   NULL, NULL };

   PJCGATEWAY pGateway;
   PNODE pNode;
   char* pszObj;

   if( NULL == ( pGateway = jcGatewayFindByName( pszName, &pNode ) ) )
   {
      sprintf( pszResult, "Gateway '%s' not found", pszName );
      return FALSE;
   }

   pszObj = StringToken( NULL, pszDelims, &pszParam );

   switch( nCommand )
   {
      case jcMngmCmdDelete:
         return jcGatewayDelete( pGateway, pNode, pszResult );

      case jcMngmCmdGet:
         if( pszObj == NULL ) /* get all parameters for this gateway */
            return MngmCmdGatewayGetAllParams( pGateway, pszResult );
         else
            return jcProcCallByName( pszObj, GatewayObjs, nCommand, pszParam, pGateway, pszResult );

      case jcMngmCmdSet:
         if( pszObj == NULL )
         {
            strcpy( pszResult, "No configuration object specified for the gateway" );
            return FALSE;
         }
         else
            return jcProcCallByName( pszObj, GatewayObjs, nCommand, pszParam, pGateway, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGatewayGetAllParams( PJCGATEWAY pGateway, char* pszResult )
{
   sprintf( pszResult, "%s%s%s%s%s%s%s%s%s",
            pGateway->pszName, pszFieldSeparator,
            pGateway->pszPrefix, pszFieldSeparator,
            pGateway->pszIPAddress, pszFieldSeparator,
            pGateway->bStripPrefix ? "yes" : "no", pszFieldSeparator,
            pGateway->pszComment ? pGateway->pszComment : "<no comment>" );
   return TRUE;
}

/*
 *
 */
int MngmCmdGatewayName( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCGATEWAY pGateway = (PJCGATEWAY)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pGateway->pszName, pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         if( jcGatewayFindByName( pszValue, NULL ) )
         {
            strcpy( pszResult, "Another gateway exists with the same name" );
            return FALSE;
         }

         StringReplace( &pGateway->pszName, pszValue );
         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGatewayPrefix( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCGATEWAY pGateway = (PJCGATEWAY)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pGateway->pszPrefix, pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         if( jcGatewayFindByNumber( pszValue ) )
         {
            strcpy( pszResult, "Another gateway exists with the same prefix" );
            return FALSE;
         }

         StringReplace( &pGateway->pszPrefix, pszValue );
         pGateway->nPrefixSize = strlen( pszValue );
         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGatewayIPAddress( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCGATEWAY pGateway = (PJCGATEWAY)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pGateway->pszIPAddress, pszResult );

      case jcMngmCmdSet:
         return MngmCmdStringSet( &pGateway->pszIPAddress, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGatewayStripPrefix( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCGATEWAY pGateway = (PJCGATEWAY)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pGateway->bStripPrefix ? "yes" : "no", pszResult );

      case jcMngmCmdSet:
      {
         char* pszValue;

         if( NULL == ( pszValue = StringToken( NULL, pszDelims, &pszParam ) ) )
            return MngmCmdNoValue( pszResult );

         pGateway->bStripPrefix = ( 0 == StringICompare( pszValue, "yes" ) );

         return MngmCmdValueSetOK( pszResult );
      }

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdGatewayComment( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   PJCGATEWAY pGateway = (PJCGATEWAY)pParam;

   switch( nCommand )
   {
      case jcMngmCmdGet:
         return MngmCmdStringGet( pGateway->pszComment, pszResult );

      case jcMngmCmdSet:
         return MngmCmdStringSet( &pGateway->pszComment, pszParam, pszResult );

      default:
         return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
   }
}

/*
 *
 */
int MngmCmdViperCells( int nCommand, char* pszParam, PVOID pParam, char* pszResult )
{
   gkHREG hReg;
   gkHAPPREG haReg;
   char buf[128];

   if( nCommand == jcMngmCmdGet )
   {
      pszResult[0] = '\0';

      if( GKRGI_OK != gkrgiGetFirstEntryHandle( &hReg, &haReg ) )
         return TRUE;

      do
      {
         MngmCmdViperCellGetAllParams( hReg, buf );
         strcat( pszResult, buf );
         strcat( pszResult, pszLineSeparator );

      }while( GKRGI_OK == gkrgiGetNextEntryHandle( hReg, &hReg, &haReg ) );

      return TRUE;
   }
   else
      return MngmCmdNotImplemented( nCommand, pszParam, NULL, pszResult );
}

/*
 *
 */
int MngmCmdViperCellGetAllParams( gkHREG hReg, char* pszResult )
{
   char* pszNames;
   gkaddTransport_t TransRAS;
   int nMobiles;
   char pszIP[16];

   pszNames = jcRegGetNames( hReg );

   if( GKRGI_OK == gkrgiGetParameter( hReg, gkrgiParamRAS, gkrgiSttsDynamic, 1, &TransRAS ) )
      gkaddIp2String( TransRAS.ip, pszIP );
   else
      strcpy( pszIP, "<unknown>" );

   if( GKRGI_OK != gkrgiGetNumberOfAddresses( hReg, gkrgiParamPhone, gkrgiSttsDynamic, &nMobiles ) )
      nMobiles = 0;

   sprintf( pszResult, "%s%s%s%s%d",
            pszNames, pszFieldSeparator,
            pszIP, pszFieldSeparator,
            nMobiles );

   return TRUE;
}

/*
 *
 */
BOOL jcNumberExists( char* pszNumber )
{
   if( MobileFindByNumber( pszNumber ) ||
       IpPhoneFindByNumber( pszNumber ) ||
       jcGatewayFindByNumber( pszNumber ) ||
       0 == StringCompare( pCfgTree->pszIVRNumber, pszNumber ) )
      return TRUE;
   else
      return FALSE;
}

/*
 *
 */
void MngmAPICallBack( pMngmCommandData_t pApiData )
{
   char *pszBuffer, *pszIMSI;
   int nSize;
   USHORT nProfileSize;
   char IMSI[16], Number[32];
   long nRequestID;
   PJCMOBILE pMobile;
   char Buffer[16*1024];

   pszBuffer = pApiData->pszInput;

   if( MNGM_API_VERSION != jcUShortUnpack( &pszBuffer ) )
   {
      DebugLog( "MngmAPICallBack(): wrong protocol or corrupted message\n" );
      pApiData->nOutput = 0; /* error, no response will be sent */
      return;
   }

   switch( jcUShortUnpack( &pszBuffer ) )
   {
      case MngmMsgRequestGeneric:
      {
         MngmCommandData_t CommandData;

         nRequestID = jcULongUnpack( &pszBuffer );

         CommandData.pszInput = pszBuffer;
         CommandData.pszOutput = Buffer;
         MngmCmdCallBack( &CommandData );

         jcMngmResponseGenericPack( pApiData, nRequestID,
                                     (short)CommandData.nOutput, CommandData.pszOutput );
         return;
      }

      case MngmMsgRequestGetMobileProfile:
      {
         nRequestID = jcULongUnpack( &pszBuffer );
         pszIMSI = jcStringUnpack( &pszBuffer );

         pMobile = MobileFindByIMSI( pszIMSI, NULL );

         pszBuffer = pApiData->pszOutput;
         nSize = 0;
         jcUShortPack( MNGM_API_VERSION, &pszBuffer, &nSize );
         jcUShortPack( MngmMsgResponseGetMobileProfile, &pszBuffer, &nSize );
         jcULongPack( nRequestID, &pszBuffer, &nSize );

         if( pMobile == NULL || pMobile->nProfileSize == 0 )
         {
            jcUShortPack( 0, &pszBuffer, &nSize );
         }
         else
         {
            jcUShortPack( pMobile->nProfileSize, &pszBuffer, &nSize );
            memcpy( pszBuffer, pMobile->ProfileData, pMobile->nProfileSize );
            nSize += pMobile->nProfileSize;
         }

         pApiData->nOutput = nSize;
         return;
      }

      case MngmMsgRequestSetMobileProfile:
      {
         nRequestID = jcULongUnpack( &pszBuffer );
         nProfileSize = jcUShortUnpack( &pszBuffer );

         if( nProfileSize <= 0 ||
             FALSE == jcMobileProfileGetParams( pszBuffer, nProfileSize, IMSI, Number ) ||
             NULL == ( pMobile = MobileFindByIMSI( IMSI, NULL ) ) )
         {
            jcMngmResponseGenericPack( pApiData, nRequestID, 0, "Wrong profile or unknown mobile" );
            return;
         }

         if( 0 != StringCompare( pMobile->pszNumber, Number ) ) /* number has changed */
         {
            if( FALSE == MngmCmdMobileNumber( jcMngmCmdSet, Number, (PVOID)pMobile, Buffer ) )
            {
               jcMngmResponseGenericPack( pApiData, nRequestID, 0, Buffer );
               return;
            }
         }

         if( nProfileSize != pMobile->nProfileSize )
         {
            SpecialFree( pMobile->ProfileData );
            pMobile->ProfileData = SpecialAlloc( nProfileSize );
            pMobile->nProfileSize = nProfileSize;
         }
         memcpy( pMobile->ProfileData, pszBuffer, nProfileSize );
         jcMngmResponseGenericPack( pApiData, nRequestID, 1, "The profile was changed" );

         if( pMobile->hReg != NULL )
            jcgkNonStdRasSendProfile( pMobile->hReg, (NSRASHANDLE)pMobile, pMobile->hMobileCell,
                                      pMobile->nProfileSize, pMobile->ProfileData );
         return;
      }

      case MngmMsgRequestAddMobileProfile:
      {
         nRequestID = jcULongUnpack( &pszBuffer );
         nProfileSize = jcUShortUnpack( &pszBuffer );

         if( nProfileSize <= 0 ||
             FALSE == jcMobileProfileGetParams( pszBuffer, nProfileSize, IMSI, Number ) )
         {
            jcMngmResponseGenericPack( pApiData, nRequestID, 0, "Wrong profile" );
            return;
         }

         jcMngmResponseGenericPack( pApiData, nRequestID,
                 (short)MobileAdd( IMSI, Number, NULL, nProfileSize, pszBuffer, Buffer ), Buffer );
         return;
      }

      default:
      {
         DebugLog( "MngmAPICallBack(): unsupported message\n" );
         pApiData->nOutput = 0; /* error, no response will be sent */
         return;
      }
   }

   return;
}

/*
 *
 */
BOOL jcMobileProfileGetParams( char* pszProfile, USHORT nProfileSize,
                               char* pszIMSI, char* pszNumber )
{
   T_SUBSC_INFO SubscrInfo;

   if( 0 != DecodeSubscInfo( pszProfile, nProfileSize, &SubscrInfo ) )
      return FALSE;

   memcpy( pszIMSI,     SubscrInfo.imsi.mcc,  3 );
   memcpy( pszIMSI + 3, SubscrInfo.imsi.mnc,  2 );
   memcpy( pszIMSI + 5, SubscrInfo.imsi.msin, 10 );
   pszIMSI[15] = '\0';

   memcpy( pszNumber, SubscrInfo.msisdn.digits, SubscrInfo.msisdn.num_digit );
   pszNumber[SubscrInfo.msisdn.num_digit] = '\0';

   return TRUE;
}

/*
 *
 */
void jcMngmResponseGenericPack( pMngmCommandData_t pApiData,
                                long nRequestID, short nResult, char* pszTextData )
{
   char *pszBuffer;
   int nSize;

   pszBuffer = pApiData->pszOutput;
   nSize = 0;

   jcUShortPack( MNGM_API_VERSION, &pszBuffer, &nSize );
   jcUShortPack( MngmMsgResponseGeneric, &pszBuffer, &nSize );
   jcULongPack( nRequestID, &pszBuffer, &nSize );
   jcUShortPack( nResult, &pszBuffer, &nSize );
   jcStringPack( pszTextData, &pszBuffer, &nSize );

   pApiData->nOutput = nSize;
   return;
}
