/* init.h
 *
 */

#define JCGK_REGISTRATIONS             20
#define JCGK_CELLS_PER_REGISTRATION    100
#define JCGK_REG_CELL_SIZE             8

/* default values for configurable parameters */
#define JCGK_DEF_MAX_CALLS         100
#define JCGK_DEF_RAS_PORT          1719
#define JCGK_DEF_RAS_RESPONSE_TO   30
#define JCGK_DEF_Q931_PORT         1720
#define JCGK_DEF_Q931_RESPONSE_TO  30
#define JCGK_DEF_Q931_CONNECT_TO   180
#define JCGK_DEF_TELNET_PORT       11785
#define JCGK_MNGM_CLIENT_PORT      11786
        /* this is hardcoded on a client side, not for modifications */

BOOL MainInit( int argc, char** argv );
BOOL GKLibInit( int argc, char** argv );

INT32 CALLCONV jcStorageInit( void );
INT32 CALLCONV jcStorageAccess( IN gkstorageAccessMode_t mode, IN OUT UINT8* gkBuffer,
                                IN UINT32 bufferSize, IN gkstorageIdentifier_t appStorageID,
                                IN UINT32 offsetInStorage );
