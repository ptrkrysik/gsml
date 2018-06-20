/* jcras.h
 *
 */

INT32 CALLCONV jcRegHandler(
   IN     gkHREG        hReg,
   IN OUT gkHAPPREG*    phaReg,
   IN     gkrgiEvent_t  Event,
   IN     gkrgiReason_t Reason );

INT32 CALLCONV jcNonStdMsgHandler(
   IN     gkHREG                  hReg,
   IN OUT gkHAPPREG*              phaReg,
   IN     HRAS                    hsRas,
   IN     gktypeNonStandardParam* appParam );

/*
INT32 CALLCONV jcRegHook(
   IN     gkHREG         hReg,
   IN OUT gkHAPPREG*     haReg,
   IN     HRAS           hsRas,
   IN     gkrgiMessage_t Msg );
*/
INT32 CALLCONV jcLrqHandler(
   IN     gkHLRQ         hLRQ,
   IN OUT gkHAPPLRQ*     haLRQ,
   IN     gklrqiEvent_t  Event,
   IN     gklrqiReason_t Reason );

BOOL RegIsDynamic( IN gkHREG );
BOOL RegIsStatic( IN gkHREG );

BOOL jcgkNonStdMsgSend( gkHREG, UCHAR* msg, int size );

void jcgkNonStdRasSendRCF( gkHREG, PJCMOBILE, NSRASHANDLE hMobileCell );
void jcgkNonStdRasSendRRJ( gkHREG, NSRASHANDLE hMobileCell );
void jcgkNonStdRasSendUCF( gkHREG, NSRASHANDLE hMobileGK, NSRASHANDLE hMobileCell );
void jcgkNonStdRasSendURQ( gkHREG, PJCMOBILE );
void jcgkNonStdRasSendCellLCF( gkHREG hReg, char* pszCellID, char* pszIpAddress, NSRASHANDLE hCell );
void jcgkNonStdRasSendCellLRJ( gkHREG, char* pszCellID, NSRASHANDLE hCell );
void jcgkNonStdRasSendProfile( gkHREG, NSRASHANDLE hMobileGK, NSRASHANDLE hMobileCell,
                               USHORT nProfileSize, PVOID pProfileData );

char* jcRegGetNames( gkHREG );
char* jcRegGetIpByName( char* pszName );
