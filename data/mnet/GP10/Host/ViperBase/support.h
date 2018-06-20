/* support.h
 * $Id: support.h,v 1.4 1999-07-30 10:38:44-07 olet Exp $
 *
 * miscellaneous general purpose functions
 */

#ifndef _INC_SUPPORT
#define _INC_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <time.h>
#include <string.h>

#define FALSE  0
#define TRUE   1

#if !defined( __RVCOMMON_H ) && !defined( __INCvxTypesOldh )
typedef int BOOL;
#endif

#if !defined( __INCvxTypesOldh )
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
#endif

typedef void* PVOID;
typedef char* PSTR;

#define ArraySize( Array ) ( sizeof( Array ) / sizeof( Array[0] ) )

#define New( OBJECT )     ( (OBJECT*)SpecialAlloc( sizeof( OBJECT ) ) )
#define Delete( pObject ) ( SpecialFree( (PVOID)( pObject ) ) )

typedef int (*ObjectCompareProc)( void* pObjPattern, void* pObjFromList );

typedef struct _NODE* PNODE;

typedef struct _NODE
{
   PNODE pNodeNext;
   PNODE pNodePrev;
   PVOID pObject;
}NODE;

typedef struct _LIST
{
   PNODE pNodeHead;
   PNODE pNodeTail;
   unsigned nObjectsCount;
}LIST, *PLIST;


typedef void ( *pfnCommonCallBack )( PVOID );

typedef struct _TIMER
{
   pfnCommonCallBack pfnCallBack;
   PVOID pData;
   BOOL bMultiple;

}TIMER, *PTIMER;


#define ListGetHead( pList ) ( pList->pNodeHead )
#define ListGetTail( pList ) ( pList->pNodeTail )
#define ListGetCount( pList ) ( pList->nObjectsCount )

#define NodeGetNext( pNode ) ( pNode->pNodeNext )
#define NodeGetPrev( pNode ) ( pNode->pNodePrev )
#define NodeGetPtr( pNode ) ( pNode->pObject )

#define ListForEachNode( pList, pNode ) \
           for( pNode = ListGetHead( pList ); \
                pNode; \
                pNode = NodeGetNext( pNode ) )

#define ListForEachNodeSafe( pList, pNode, pNodeNext ) \
           for( pNodeNext = ( pNode = ListGetHead( pList ) ) ? NodeGetNext( pNode ) : NULL ; \
                pNode; \
                pNodeNext = ( pNode = pNodeNext ) ? NodeGetNext( pNode ) : NULL )

/* memory */
PVOID SpecialAlloc( size_t );
void  SpecialFree( PVOID );
ULONG MemoryGetTotalBlocks( void );
ULONG MemoryGetTotalSize( void );

/* list */
PLIST ListCreate( void );
void  ListDelete( PLIST );
void  ListRemoveAllNodes( PLIST );       /* doesn't remove objects itself! */
void  ListRemoveNode( PLIST, PNODE );
PNODE ListAddHead( PLIST, PVOID );
PNODE ListAddTail( PLIST, PVOID );
PNODE ListFindByPtr( PLIST, PVOID );
PNODE ListFindByData( PLIST, PVOID pObjPattern, ObjectCompareProc );

/* strings */
PSTR   StringCreate( char* psz );
void   StringDelete( PSTR );
PSTR   StringReplace( PSTR* ppszOld, char* psz );
size_t StringLength( char* psz );
int    StringICompare( char* psz1, char* psz2 );
char*  StringSplit( char* psz, char SplitChar );
void   StringPrint( char* pszFormat, ... );
void   StringIndent( int nSpaces );
char*  StringToken( char* pszThis, char* pszDelims, char** ppszNext );

#define StringCompare( psz1, psz2 ) \
        ( ( psz1 == NULL || psz2 == NULL ) ? ~0 : strcmp( psz1, psz2 ) )

/* log control */
BOOL DebugLevelInit( BOOL bLogEnabled );
void DebugLog( char* pszFormat, ... );

/* math */
int DivideInts( int nDivident, int nDivisor );

/* timer */
PTIMER TimerCreate( time_t tmInterval, pfnCommonCallBack, PVOID pData, BOOL bMultiple );
BOOL   TimerDelete( PTIMER );

/* message loop */
BOOL MainLoop( void );

/* RAD input pipe */
typedef struct
{
   int hPipe;
   pfnCommonCallBack pfnUserCallBack;
   int nMsgSize;
   PVOID pMsg;  /* place to read received message */

}RADPIPE, *PRADPIPE;

PRADPIPE RadPipeOpen( char* Name, int nMsgSize, int nMaxMsgs, pfnCommonCallBack );
BOOL RadPipeWrite( PRADPIPE, PVOID pMessage );
BOOL RadPipeClose( PRADPIPE );

/* debug */
#define ExecBreak( Text )   DebugFatalErrorBreak( Text, __FILE__, __LINE__ )
#define ExecBreakIf( Cond, Text )   if( Cond ) ExecBreak( Text )
void DebugFatalErrorBreak( char* pszText, char* pszFile, int nLine );
void SpecialDebugBreak( void );

/* misc */
char* GetDateTimeString( time_t );
char* GetOption( char* pszSection, char* pszOption, char* pszDefaultValue );
char* ParamGetValue( int argc, char** argv, char* pszParam, char* pszDefaultValue );
BOOL  ParamIsPresent( int argc, char** argv, char* pszParam );
void  SpecialSleep( int nSeconds );
BOOL  SocketKeepAlive( int hSocket, BOOL );
BOOL PlatformLayerInit( void );
BOOL PlatformLayerDown( void );

/* management */

typedef struct
{
   char* pszInput;
   char* pszOutput;
   int nOutput;

}MngmCommandData_t, *pMngmCommandData_t;

BOOL MngmPortInit( int MngmPort, pfnCommonCallBack pfnMngmCallBack, BOOL bTelent );
BOOL MngmTelnetSendString( int hSocket, char* pszString );

/* packing / unpacking for network messages */
void jcStringPack( char* pszString, UCHAR** ppszBuffer, int* pnSize );
char* jcStringUnpack( UCHAR** ppszBuffer );
void jcULongPack( ULONG uLong, UCHAR** ppszBuffer, int* pnSize );
ULONG jcULongUnpack( UCHAR** ppszBuffer );
void jcUShortPack( USHORT uShort, UCHAR** ppszBuffer, int* pnSize );
USHORT jcUShortUnpack( UCHAR** ppszBuffer );
void jcBufferPack( PVOID pData, USHORT nDataSize, UCHAR** ppszBuffer, int* pnSize );
PVOID jcBufferUnpack( USHORT nDataSize, UCHAR** ppszBuffer );

#ifdef __cplusplus
}
#endif

#endif /* _INC_SUPPORT */
