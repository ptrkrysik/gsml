/* jccall.c
 *
 */

#include <gkadd.h>
#include <gktypes.h>
#include <gkrgi.h>
#include <gkcci.h>

#include "support.h"
#include "exchange.h"
#include "jccall.h"
#include "admin.h"

/*
 *
 */
INT32 CALLCONV jcStateHandler(
   IN     gkHCALL       hCall,
   IN OUT gkHAPPCALL*   phaCall,
   IN     gkcciState_t  State,
   IN     gkcciReason_t Reason )
{

   DebugLog( "Call %u, State '%s', Reason '%s'\n", hCall,
             gkcciStateName( State ), gkcciReasonName( Reason ) );

   switch( State )
   {
      case gkcciSttCannotComplete:
      {
         switch( Reason )
         {
            case gkcciRsnCannotSendLRQ:
               return jcMainRouting( hCall, phaCall );

            case gkcciRsnOutOfService:
            case gkcciRsnNoAnswer:
            case gkcciRsnAppTO:
               return jcCallSendToIVR( hCall, phaCall, cmReasonTypeUnreachableDestination );

            case gkcciRsnLinesBusy:
               return jcCallSendToIVR( hCall, phaCall, cmReasonTypeCalledPartyNotRegistered );

            case gkcciRsnDestBusy:
               return jcCallSendToIVR( hCall, phaCall, cmReasonTypeAdaptiveBusy );

            case gkcciRsnDestReject:
               return jcCallSendToIVR( hCall, phaCall, cmReasonTypeDestinationRejection );

            case gkcciRsnUndefined:
            default:
               return jcCallSendToIVR( hCall, phaCall, cmReasonTypeUndefinedReason );
         }
      }

      default:
         break;
   }

   return gkcciDefaultStateEventHandler( hCall, phaCall, State, Reason );
}

/*
 *
 */
INT32 CALLCONV jcBandwidthHandler(
   IN     gkHCALL                hCall,
   IN OUT gkHAPPCALL*            haCall,
   IN     gkcciBandwidthEvent_t  Event,
   IN     gkcciSide_t            Side,
   IN     gkcciBandwidthReason_t	Reason,
   IN     gktypeBw_t             RequestedBWInBPS )
{

   /* DebugLog( "jcBandwidthHandler(), Event = %d, Reason = %d\n", Event, Reason ); */

   switch( Event )
   {

      default:
         break;
   }

   return gkcciDefaultBandwidthEventHandler( hCall, haCall, Event, Side, Reason, RequestedBWInBPS );
}

/*
 *
 */
INT32 CALLCONV jcFacilityHandler(
   IN     gkHCALL               hCall,
   IN OUT gkHAPPCALL*           haCall,
   IN     INT32                 FacilityHandle,
   IN     gkcciFacilityEvent_t  Event,
   IN     gkcciSide_t           Side,
   IN     gkcciFacilityReason_t Reason )
{

   /* DebugLog( "jcFacilityHandler(), Event = %d, Reason = %d\n", Event, Reason ); */

   switch( Event )
   {

      default:
         break;
   }

   return gkcciDefaultFacilityEventHandler( hCall, haCall, FacilityHandle, Event, Side, Reason );
}

/*
 *
 */
INT32 CALLCONV jcCallMsgHandler(
   IN     gkHCALL     hCall,
   IN OUT gkHAPPCALL* haCall,
   IN     gkcciMsg_t  Msg,
   IN     gkcciSide_t Side )
{

   /* DebugLog( "jcCallMsgHandler(): %s\n", gkcciMsgName( Msg ) ); */

   switch( Msg )
   {
      case gkcciMsgSendSetup:
      default:
         break;
   }

   return gkcciOK;
}

/*
 *
 */
INT32 CALLCONV jcCallHookHandler(
   IN     gkHCALL     hCall,
   IN OUT gkHAPPCALL* haCall,
   IN     HCALL       hsCall,
   IN     HRAS        hsRas,
   IN     gkcciMsg_t  Msg,
   IN     gkcciSide_t Side )
{

   switch( Msg )
   {
      case gkcciMsgSendSetup:
      {
         char buf[16];
         sprintf( buf, "%d", (int)*haCall ); /* disconnect reason for IVR */
         cmCallSetParam( hsCall, cmParamUserUser, 0, strlen( buf ) + 1, buf );
         break;
      }

      default:
         break;
   }

   return gkcciOK;
}

/*
 *
 */
int jcMainRouting( gkHCALL hCall, gkHAPPCALL* phaCall )
{
   gkaddAlias_t Alias;
   gkaddTransport_t Transport;
   PJCGATEWAY pGateway;
   PJCIPPHONE pIPPhone;
   char *pszIPAddress, *pszName;

   if( gkcciOK != gkcciGetParameter( hCall, gkcciParamAlias, gkcciSideDest, 0, 0, &Alias ) )
      return jcCallSendToIVR( hCall, phaCall, cmReasonTypeBadFormatAddress );

   if( NULL != ( pIPPhone = IpPhoneFindByNumber( Alias.aliasStr ) ) )
   {
      pszName      = pIPPhone->pszName;
      pszIPAddress = pIPPhone->pszIPAddress;
   }
   else if( NULL != ( pGateway = jcGatewayFindByNumber( Alias.aliasStr ) ) )
   {
      pszName      = pGateway->pszName;
      pszIPAddress = pGateway->pszIPAddress;

      if( pGateway->bStripPrefix )
      {
         char *pszSrc, *pszDst;

         pszDst = Alias.aliasStr;
         pszSrc = Alias.aliasStr + pGateway->nPrefixSize;

         while( *pszDst++ = *pszSrc++ )
            ;
      }
   }
   else
   {
      DebugLog( "Number '%s' not found!\n", Alias.aliasStr );
      return jcCallSendToIVR( hCall, phaCall, cmReasonTypeCalledPartyNotRegistered );
   }

   Transport.ip = gkaddString2Ip( pszIPAddress );
   Transport.port = 1720;
   Transport.routeType = gkaddIpNoRoute;
   Transport.routeLength = 0;

   DebugLog( "Sending SETUP to %s, %s, %s\n", pszName, pszIPAddress, Alias.aliasStr );

   gkcciSetParameter( hCall, gkcciParamCallSignal, gkcciSideDest, 0, 0, &Transport );
   gkcciDeleteAlias(  hCall, gkcciParamAlias, gkcciSideDest, 0, 0 );
   gkcciSetParameter( hCall, gkcciParamAlias, gkcciSideDest, 0, 0, &Alias );

   gkcciReply( hCall, gkcciDontDisconnectStartAddressResolution, gkcciReplyRsnAddressFinal );
   return gkcciOK;
}

/*
 *
 */
int jcCallSendToIVR( gkHCALL hCall, gkHAPPCALL* phaCall, int Reason )
{
   gkaddAlias_t Alias;
   char* pszIVRNumber;
   char UserInfo[16];

   sprintf( UserInfo, "%d", Reason );

   if( NULL == ( pszIVRNumber = pCfgTree->pszIVRNumber ) ||
       gkcciOK != gkcciGetParameter( hCall, gkcciParamAlias, gkcciSideDest, 0, 0, &Alias ) ||
       0 == StringCompare( pszIVRNumber, Alias.aliasStr ) )
   {
      gkcciReply( hCall, gkcciConfirmDisconnect, gkcciReplyRsnUndefined );
      return gkcciOK;
   }

   *phaCall = (gkHAPPCALL)Reason; /* disconnect reason for IVR */

   strcpy( Alias.aliasStr, pszIVRNumber );
   gkcciDeleteAlias(  hCall, gkcciParamAlias, gkcciSideDest, 0, 0 );
   gkcciSetParameter( hCall, gkcciParamAlias, gkcciSideDest, 0, 0, &Alias );
   gkcciReply( hCall, gkcciDontDisconnectStartAddressResolution, gkcciReplyRsnAliasChange );

   return gkcciOK;
}
