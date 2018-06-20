/* jccall.h
 *
 */

INT32 CALLCONV jcStateHandler(
   IN     gkHCALL       hCall,
   IN OUT gkHAPPCALL*   haCall,
   IN     gkcciState_t  State,
   IN     gkcciReason_t Reason );

INT32 CALLCONV jcCallMsgHandler(
   IN     gkHCALL     hCall,
   IN OUT gkHAPPCALL* haCall,
   IN     gkcciMsg_t  Msg,
   IN     gkcciSide_t Side );

INT32 CALLCONV jcBandwidthHandler(
   IN     gkHCALL                hCall,
   IN OUT gkHAPPCALL*            haCall,
   IN     gkcciBandwidthEvent_t  Event,
   IN     gkcciSide_t            Side,
   IN     gkcciBandwidthReason_t	Reason,
   IN     gktypeBw_t             RequestedBWInBPS );

INT32 CALLCONV jcFacilityHandler(
   IN     gkHCALL               hCall,
   IN OUT gkHAPPCALL*           haCall,
   IN     INT32                 FacilityHandle,
   IN     gkcciFacilityEvent_t  Event,
   IN     gkcciSide_t           Side,
   IN     gkcciFacilityReason_t Reason );

INT32 CALLCONV jcCallHookHandler(
   IN     gkHCALL     hCall,
   IN OUT gkHAPPCALL* haCall,
   IN     HCALL       hsCall,
   IN     HRAS        hsRas,
   IN     gkcciMsg_t  Msg,
   IN     gkcciSide_t Side );

BOOL RegIsDynamic( IN gkHREG hReg );
BOOL RegIsStatic( IN gkHREG hReg );

int jcMainRouting( gkHCALL, gkHAPPCALL* );
int jcCallSendToIVR( gkHCALL, gkHAPPCALL*, int Reason );
