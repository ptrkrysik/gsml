/* jcras.c
 *
 */

#include <gkadd.h>
#include <gkrgi.h>
#include <gklrqi.h>
#include <li.h>

#include "support.h"
#include "exchange.h"
#include "admin.h"
#include "jcras.h"

/*
 *
 */
INT32 CALLCONV jcRegHandler(
   IN      gkHREG        hReg,
   IN OUT  gkHAPPREG*    phaReg,
   IN      gkrgiEvent_t  Event,
   IN      gkrgiReason_t Reason )
{
   PNODE pNode;
   PJCMOBILE pMobile;

   DebugLog( "'%s', Event '%s', Reason '%s'\n", jcRegGetNames( hReg ),
             gkrgiEventName( Event ), gkrgiReasonName( Reason ) );

   switch( Event )
   {
      case gkrgiEvApprovedRRQ:
         gkrgiReply( hReg, gkrgiRplyConfirm, gkrgiRplyRsnUndefined );
         return GKRGI_OK;

      case gkrgiEvSendingMessage:

         if( Reason != gkrgiRsnSendingGKURQ && Reason != gkrgiRsnSendingApplicationURQ )
            break;

         /* going through! */

      case gkrgiEvApprovedURQ:

         ListForEachNode( pCfgTree->pListMobiles, pNode )
         {
            pMobile = (PJCMOBILE)NodeGetPtr( pNode );

            if( pMobile->hReg == hReg )
            {
               pMobile->hReg = NULL;
               pMobile->hMobileCell = 0;
            }
         }

         break;

      default:
         break;
   }

   return gkrgiDefaultRegHandler( hReg, phaReg, Event, Reason );
}

/*
 *
 */
INT32 CALLCONV jcNonStdMsgHandler(
   IN     gkHREG                  hReg,
   IN OUT gkHAPPREG*              phaReg,
   IN     HRAS                    hsRas,
   IN     gktypeNonStandardParam* appParam )
{
   PJCMOBILE pMobile;
   UCHAR* pszBuffer;

   if( NULL == ( pszBuffer = (UCHAR*)appParam->data ) ||
       appParam->dataLength <= 0 ||
       appParam->t35CountryCode   != H323_COUNTRY_CODE_USA ||
       appParam->manufacturerCode != H323_MANUFACTURER_CODE_JETCELL ||
       NON_STD_RAS_PROTOCOL != jcUShortUnpack( &pszBuffer ) )
   {
      DebugLog( "jcNonStdMsgHandler() error: unrecognized non standard message\n" );
      return 0;
   }

   switch( jcUShortUnpack( &pszBuffer ) ) /* message type */
   {
      case NonStdRasMessageTypeRRQ:
      {
         NonStdRasMessageRRQ_t MsgRRQ;

         jcNonStdRasMessageRRQUnpack( &MsgRRQ, &pszBuffer );

         if( NULL == ( pMobile = MobileFindByIMSI( MsgRRQ.pszMobileID, NULL ) ) )
         {
            jcgkNonStdRasSendRRJ( hReg, MsgRRQ.hMobileCell );
         }
         else
         {
            if( pMobile->hReg != NULL && pMobile->hReg != hReg ) /* old registration */
            {
               gkrgiDeleteAddress( pMobile->hReg, gkrgiParamPhone, gkrgiSttsDynamic, pMobile->pszNumber );
               jcgkNonStdRasSendURQ( pMobile->hReg, pMobile );
            }

            gkrgiAddAddress( hReg, gkrgiParamPhone, gkrgiSttsDynamic, pMobile->pszNumber );
            jcgkNonStdRasSendRCF( hReg, pMobile, MsgRRQ.hMobileCell );

            pMobile->hReg = hReg;
            pMobile->hMobileCell = MsgRRQ.hMobileCell;
         }
         break;
      }

      case NonStdRasMessageTypeURQ:
      {
         NonStdRasMessageURQ_t MsgURQ;

         jcNonStdRasMessageURQUnpack( &MsgURQ, &pszBuffer );

         pMobile = (PJCMOBILE)MsgURQ.hMobileGK;

         if( NULL == ListFindByPtr( pCfgTree->pListMobiles, pMobile ) )
             /* only to make sure pMobile is valid */
         {
            DebugLog( "jcNonStdMsgHandler(): invalid mobile handle in URQ\n" );
         }
         else
         {
            gkrgiDeleteAddress( hReg, gkrgiParamPhone, gkrgiSttsDynamic, pMobile->pszNumber );
            pMobile->hReg = NULL;
            pMobile->hMobileCell = 0;
         }

         jcgkNonStdRasSendUCF( hReg, MsgURQ.hMobileGK, MsgURQ.hMobileCell );

         break;
      }

      case NonStdRasMessageTypeUCF:
         break; /* nothing to do */

      case NonStdRasMessageTypeCellLRQ:
      {
         NonStdRasMessageCellLRQ_t MsgCellLRQ;
         char* pszIpAddr;

         jcNonStdRasMessageCellLRQUnpack( &MsgCellLRQ, &pszBuffer );

         if( NULL == ( pszIpAddr = jcRegGetIpByName( MsgCellLRQ.pszCellID ) ) )
            jcgkNonStdRasSendCellLRJ( hReg, MsgCellLRQ.pszCellID, MsgCellLRQ.hCell );
         else
            jcgkNonStdRasSendCellLCF( hReg, MsgCellLRQ.pszCellID, pszIpAddr, MsgCellLRQ.hCell );

         break;
      }

      case NonStdRasMessageTypeMobileProfileRequest:
      {
         NonStdRasMessageMobileProfileRequest_t MsgReq;

         jcNonStdRasMessageMobileProfileRequestUnpack( &MsgReq, &pszBuffer );

         pMobile = (PJCMOBILE)MsgReq.hMobileGK;

         if( NULL == ListFindByPtr( pCfgTree->pListMobiles, pMobile ) )
             /* only to make sure pMobile is valid */
         {
            DebugLog( "jcNonStdMsgHandler(): invalid mobile handle in MobileProfileRequest\n" );
            jcgkNonStdRasSendProfile( hReg, MsgReq.hMobileGK, MsgReq.hMobileCell, 0, NULL );
         }
         else
            jcgkNonStdRasSendProfile( hReg, MsgReq.hMobileGK, MsgReq.hMobileCell,
                                      pMobile->nProfileSize, pMobile->ProfileData );
         break;
      }

      default:
         DebugLog( "jcNonStdMsgHandler() error: unrecognized message\n" );
         return 0;
   }

   return 0;
}


/*
 *
 */
/*
INT32 CALLCONV jcRegHook(
   IN     gkHREG         hReg,
   IN OUT gkHAPPREG*     phaReg,
   IN     HRAS           hsRas,
   IN     gkrgiMessage_t Msg )
{

   DebugLog( "jcRegHook(), Msg = %d\n", Msg );

   return TRUE;
}
*/
/*
 *
 */
INT32 CALLCONV jcLrqHandler(
   IN     gkHLRQ         hLRQ,
   IN OUT gkHAPPLRQ*     haLRQ,
   IN     gklrqiEvent_t  Event,
   IN     gklrqiReason_t Reason )
{

   DebugLog( "jcLrqHandler(), Event = %d\n", Event );

   switch( Event )
   {
      default:
         break;
   }

   return gklrqiDefaultLRQHandler( hLRQ, haLRQ, Event, Reason );
}

/*
 *
 */
BOOL RegIsDynamic( IN gkHREG hReg )
{
   BOOL isDynamic;

   if( gkrgiIsDynamic( hReg, &isDynamic ) < 0 )
      return FALSE;

   return isDynamic;
}

/*
 *
 */
BOOL RegIsStatic( IN gkHREG hReg )
{
   BOOL isStatic;

   if( gkrgiIsStatic( hReg, &isStatic ) < 0 )
      return FALSE;

  return isStatic;
}

/*
 *
 */
void jcgkNonStdRasSendRCF( gkHREG hReg, PJCMOBILE pMobile, NSRASHANDLE hMobileCell )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageRCF_t MsgRCF;

   MsgRCF.hMobileGK = (NSRASHANDLE)pMobile;
   MsgRCF.hMobileCell = hMobileCell;
   MsgRCF.pszNumber = pMobile->pszNumber;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeRCF, &pszBuffer, &nSize );
   jcNonStdRasMessageRCFPack( &MsgRCF, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
void jcgkNonStdRasSendRRJ( gkHREG hReg, NSRASHANDLE hMobileCell )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageRRJ_t MsgRRJ;

   MsgRRJ.hMobileCell = hMobileCell;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeRRJ, &pszBuffer, &nSize );
   jcNonStdRasMessageRRJPack( &MsgRRJ, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
void jcgkNonStdRasSendUCF( gkHREG hReg, NSRASHANDLE hMobileGK, NSRASHANDLE hMobileCell )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageUCF_t MsgUCF;

   MsgUCF.hMobileGK = hMobileGK;
   MsgUCF.hMobileCell = hMobileCell;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeUCF, &pszBuffer, &nSize );
   jcNonStdRasMessageUCFPack( &MsgUCF, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
void jcgkNonStdRasSendURQ( gkHREG hReg, PJCMOBILE pMobile )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageURQ_t MsgURQ;

   MsgURQ.hMobileGK = (NSRASHANDLE)pMobile;
   MsgURQ.hMobileCell = pMobile->hMobileCell;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeURQ, &pszBuffer, &nSize );
   jcNonStdRasMessageURQPack( &MsgURQ, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
void jcgkNonStdRasSendCellLRJ( gkHREG hReg, char* pszCellID, NSRASHANDLE hCell )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageCellLRJ_t MsgCellLRJ;

   MsgCellLRJ.pszCellID = pszCellID;
   MsgCellLRJ.hCell = hCell;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeCellLRJ, &pszBuffer, &nSize );
   jcNonStdRasMessageCellLRJPack( &MsgCellLRJ, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
void jcgkNonStdRasSendCellLCF( gkHREG hReg, char* pszCellID, char* pszIpAddress, NSRASHANDLE hCell )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageCellLCF_t MsgCellLCF;

   MsgCellLCF.pszCellID = pszCellID;
   MsgCellLCF.pszIpAddress = pszIpAddress;
   MsgCellLCF.hCell = hCell;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeCellLCF, &pszBuffer, &nSize );
   jcNonStdRasMessageCellLCFPack( &MsgCellLCF, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
void jcgkNonStdRasSendProfile( gkHREG hReg, NSRASHANDLE hMobileGK, NSRASHANDLE hMobileCell,
                               USHORT nProfileSize, PVOID pProfileData )
{
   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;
   NonStdRasMessageMobileProfile_t Msg;

   Msg.hMobileGK = hMobileGK;
   Msg.hMobileCell = hMobileCell;
   Msg.nProfileSize = nProfileSize;
   Msg.pProfileData = pProfileData;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeMobileProfile, &pszBuffer, &nSize );
   jcNonStdRasMessageMobileProfilePack( &Msg, &pszBuffer, &nSize );

   jcgkNonStdMsgSend( hReg, buffer, nSize );

   return;
}

/*
 *
 */
BOOL jcgkNonStdMsgSend( gkHREG hReg, UCHAR* msg, int size )
{
   gktypeNonStandardParam NSP;

   if( size > NON_STD_MSG_MAX_LEN )
   {
      DebugLog( "jcgkNonStdMsgSend() error: message size is too big (%d > %d)\n",
                size, NON_STD_MSG_MAX_LEN );
      return FALSE;
   }

   NSP.data = (char*)msg;
   NSP.dataLength = size;
   NSP.objectLength = 0;
   NSP.t35CountryCode = H323_COUNTRY_CODE_USA;
   NSP.t35Extension = 0;
   NSP.manufacturerCode = H323_MANUFACTURER_CODE_JETCELL;

   if( GKRGI_OK != gkrgiSendNonStandardMsg( hReg, &NSP ) )
   {
      DebugLog( "Error in gkrgiSendNonStandardMsg()\n" );
      return FALSE;
   }
   else
      return TRUE;
}

/*
 *
 */
char* jcRegGetNames( gkHREG hReg )
{
   static char Buffer[512];
   gkaddName_t Name;
   int i, nNames;

   if( GKRGI_OK != gkrgiGetNumberOfAddresses( hReg, gkrgiParamName, gkrgiSttsDynamic, &nNames ) ||
       nNames <= 0 )
      return strcpy( Buffer, "<unknown>" );

   Buffer[0] = '\0';

   for( i = nNames; i > 0; i-- )
   {
      if( GKRGI_OK == gkrgiGetParameter( hReg, gkrgiParamName, gkrgiSttsDynamic, i, Name ) )
      {
         strcat( Buffer, Name );
         if( i > 1 ) strcat( Buffer, "/" );
      }
   }

   return Buffer;
}

/*
 *
 */
char* jcRegGetIpByName( char* pszName )
{
   gkHREG hReg;
   gkHAPPREG haReg;
   gkaddName_t Name;
   gkaddTransport_t TransRAS;
   int i, nNames;
   static char pszRAS[32];

   if( GKRGI_OK != gkrgiGetFirstEntryHandle( &hReg, &haReg ) )
      return NULL;

   do
   {
      if( GKRGI_OK != gkrgiGetNumberOfAddresses( hReg, gkrgiParamName, gkrgiSttsDynamic, &nNames ) ||
          nNames <= 0 )
         continue;

      for( i = 1; i <= nNames; i++ )
      {
         if( GKRGI_OK == gkrgiGetParameter( hReg, gkrgiParamName, gkrgiSttsDynamic, i, Name ) &&
             0 == StringCompare( pszName, Name ) )
         {
            if( GKRGI_OK == gkrgiGetParameter( hReg, gkrgiParamRAS, gkrgiSttsDynamic, 1, &TransRAS ) )
               return gkaddIp2String( TransRAS.ip, pszRAS );
            else
               return NULL;
         }
      }

   }while( GKRGI_OK == gkrgiGetNextEntryHandle( hReg, &hReg, &haReg ) );

   return NULL;
}
