/* init.c
 *
 */

/* GKLib includes */
#include <gkmain.h>
#include <gkadd.h>
#include <gkrgi.h>
#include <gklrqi.h>
#include <gkcci.h>
#include <gkstorage.h>
#include <cfgapi.h>
#include <li.h>

#include "support.h"
#include "exchange.h"
#include "init.h"
#include "admin.h"
#include "jcras.h"
#include "jccall.h"

void CALLCONV GKLibPrintRegular( const char* pszString, int nLevel );
void CALLCONV GKLibPrintException( const char* pszString );

/*
 *
 */
BOOL MainInit( int argc, char** argv )
{
   FILE* hFile;
   USHORT nVersion; /* config file version */
   char pszBuffer[128];

   DebugLevelInit( !ParamIsPresent( argc, argv, "-nolog" ) );

   pCfgTree = New( JCCONFIG );
   pCfgTree->pListIPPhones = ListCreate();
   pCfgTree->pListGateways = ListCreate();
   pCfgTree->pListMobiles =  ListCreate();

   if( NULL == ( hFile = jcOpenConfigFile( ParamGetValue( argc, argv, "-db", NULL ), &nVersion ) ) )
   {
      pCfgTree->nMaxCalls       = JCGK_DEF_MAX_CALLS;
      pCfgTree->RASPort         = JCGK_DEF_RAS_PORT;
      pCfgTree->Q931Port        = JCGK_DEF_Q931_PORT;
      pCfgTree->RASResponseTO   = JCGK_DEF_RAS_RESPONSE_TO;
      pCfgTree->Q931ResponseTO  = JCGK_DEF_Q931_RESPONSE_TO;
      pCfgTree->Q931ConnectTO   = JCGK_DEF_Q931_CONNECT_TO;
      pCfgTree->pszGatekeeperID = StringCreate( "ViperBase" );
      pCfgTree->TelnetPort      = JCGK_DEF_TELNET_PORT;
      pCfgTree->pszIVRNumber    = NULL;
   }
   else
   {
      pCfgTree->nMaxCalls       = jcFileReadUShort( hFile );
      pCfgTree->RASPort         = jcFileReadUShort( hFile );
      pCfgTree->Q931Port        = jcFileReadUShort( hFile );
      pCfgTree->RASResponseTO   = jcFileReadUShort( hFile );
      pCfgTree->Q931ResponseTO  = jcFileReadUShort( hFile );
      pCfgTree->Q931ConnectTO   = jcFileReadUShort( hFile );
      pCfgTree->pszGatekeeperID = StringCreate(
                                  jcFileReadString( hFile, pszBuffer, ArraySize( pszBuffer ) ) );
      pCfgTree->TelnetPort      = jcFileReadUShort( hFile );

      if( nVersion < 2 )
         pCfgTree->pszIVRNumber = NULL;
      else
         pCfgTree->pszIVRNumber = StringCreate(
                                  jcFileReadString( hFile, pszBuffer, ArraySize( pszBuffer ) ) );
   }

   if( FALSE == PlatformLayerInit() ||
       FALSE == GKLibInit( argc, argv ) )
      return FALSE;


   if( NULL != hFile )
   {
      jcReadIpPhones( hFile, nVersion );
      jcReadGateways( hFile, nVersion );
      jcReadMobiles( hFile, nVersion );

      fclose( hFile );
   }

   if( FALSE == MngmPortInit( JCGK_MNGM_CLIENT_PORT, (pfnCommonCallBack)MngmAPICallBack, FALSE ) ||
       FALSE == MngmPortInit( pCfgTree->TelnetPort, (pfnCommonCallBack)MngmCmdCallBack, TRUE ) )
      return FALSE;

   return TRUE;
}

/*
 *
 */
BOOL GKLibInit( int argc, char** argv )
{
   int nMemSize;
   gkmainRas_t RASParams;
   gkmainQ931_t Q931Params;

   static GKRGIEVENT regHandler = { jcRegHandler, NULL };
   static GKNSMEVENT nsmHandler = { jcNonStdMsgHandler };
   static GKLRQIEVENT lrqHandler = { jcLrqHandler, NULL };
   static GKCCIEVENT callHandler = { jcStateHandler, jcCallMsgHandler, jcCallHookHandler, jcBandwidthHandler, jcFacilityHandler };
   static GK_STORAGE_STRUCT storageHandler = { jcStorageInit, jcStorageAccess };

   /* print function for library debug output */
   gkmainSetNotify( GKLibPrintRegular );
   gkmainSetException( GKLibPrintException );
   gkmainSetDbgL( 0 );

   if( gkmainConfigMaxRegistrations( JCGK_REGISTRATIONS, JCGK_CELLS_PER_REGISTRATION,
                                     JCGK_REG_CELL_SIZE, &nMemSize ) < 0 )
   {
      DebugLog( "Error in gkmainConfigMaxRegistrations()\n" );
      return FALSE;
   }

   if( gkmainConfigMaxCalls( pCfgTree->nMaxCalls, NULL ) < 0 )
   {
      DebugLog( "Error in gkmainConfigMaxCalls()\n" );
      return FALSE;
   }

   RASParams.responseTO = pCfgTree->RASResponseTO;
   RASParams.port = pCfgTree->RASPort;
   strcpy( RASParams.multicastIp, GKMAIN_DEFAULT_multicastIp );
   RASParams.multicastPort = GKMAIN_DEFAULT_multicastPort;

   Q931Params.responseTO = pCfgTree->Q931ResponseTO;
   Q931Params.connectTO = pCfgTree->Q931ConnectTO;
   Q931Params.port = pCfgTree->Q931Port;

   if( gkmainConfigStack( &RASParams, &Q931Params ) < 0 )
   {
      DebugLog( "Error in gkmainConfigStack()\n" );
      return FALSE;
   }

   gkrgiSetEventHandler( &regHandler, sizeof( GKRGIEVENT ) );
   gkrgiSetNSMHandler( &nsmHandler, sizeof( GKNSMEVENT ) );
   gklrqiSetEventHandler( &lrqHandler, sizeof( GKLRQIEVENT ) );
   gkcciSetEventHandler( &callHandler, sizeof( GKCCIEVENT ) );
   gkstorageSetStorageFunctions( &storageHandler, sizeof( GK_STORAGE_STRUCT ) );

   if( 0 != gkmainInit() )
   {
      DebugLog( "Error in gkmainInit()\n" );
      return FALSE;
   }

   gkcfgiSetGKID( pCfgTree->pszGatekeeperID );
   gkcfgiZonePropertiesSet( GKCFGI_DHCP, TRUE );
   gkcfgiZonePropertiesSet( GKCFGI_ROUTED_CALLING_MODE, TRUE );
   gkcfgiZonePropertiesSet( GKCFGI_OPEN_ZONE, TRUE );
   gkcfgiZonePropertiesSet( GKCFGI_IRQ_INTERVAL, 25 /* minimal allowed */ );
   gkcfgiZonePropertiesSet( GKCFGI_ENABLE_MULTICAST, 0 );

   return TRUE;
}

/*
 *
 */
void CALLCONV GKLibPrintRegular( const char* pszString, int nLevel )
{
   if( NULL == pszString ) return;

   DebugLog( "GKL: %s\n", pszString + 11 /* to cut off timestamp */ );

   return;
}

/*
 *
 */
void CALLCONV GKLibPrintException( const char* pszString )
{
   if( NULL == pszString ) return;

   DebugLog( "GKL(!): %s\n", pszString + 11 /* to cut off timestamp */ );

   return;
}

/*
 *
 */
INT32 CALLCONV jcStorageInit( void )
{
   return GKSTORAGE_OK;
}

/*
 *
 */
INT32 CALLCONV jcStorageAccess( IN gkstorageAccessMode_t mode, IN OUT UINT8* gkBuffer,
                                IN UINT32 bufferSize, IN gkstorageIdentifier_t appStorageID,
                                IN UINT32 offsetInStorage )
{
   return GKSTORAGE_OK;
}
