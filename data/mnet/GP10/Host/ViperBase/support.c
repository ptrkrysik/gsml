/* support.c
 * $Id: support.c,v 1.3 1999-07-30 10:38:43-07 olet Exp $
 *
 * miscellaneous general purpose functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "support.h"

static ULONG nMemoryChunks = 0;
static ULONG szMemoryChunks = 0;

static BOOL bDebugLog = FALSE;

/*
 *
 */
PVOID SpecialAlloc( size_t szBytes )
{
   PVOID pvoid;
   size_t szTotalBytes;

#  if defined( _DEBUG )
      szTotalBytes = szBytes + 2 * sizeof( ULONG );
#  else
      szTotalBytes = szBytes;
#  endif /* _DEBUG */

   ExecBreakIf( NULL == ( pvoid = malloc( szTotalBytes ) ), "Not enough memory" );

#  if defined( _DEBUG )

   nMemoryChunks++;
   szMemoryChunks += szBytes;

   *(ULONG*)pvoid = (ULONG)szBytes;
   pvoid = (ULONG*)pvoid + 1;

   *(ULONG*)pvoid = (ULONG)0xFEDCABCD; /* tag */
   pvoid = (ULONG*)pvoid + 1;

#  endif /* _DEBUG */

   return pvoid;
}

/*
 *
 */
void SpecialFree( PVOID pvoid )
{
   if( pvoid == NULL )
      return;

#  if defined( _DEBUG )
   {
      ULONG szThisChunk;

      pvoid = (ULONG*)pvoid - 1;

      ExecBreakIf( *(ULONG*)pvoid != (ULONG)0xFEDCABCD, "Invalid pointer" );

      pvoid = (ULONG*)pvoid - 1;
      szThisChunk = *(ULONG*)pvoid;

      nMemoryChunks--;
      szMemoryChunks -= szThisChunk;

      memset( pvoid, 0x00, szThisChunk + 2 * sizeof( ULONG ) );
   }
#  endif /* _DEBUG */

   free( pvoid );

   return;
}

/*
 *
 */
ULONG MemoryGetTotalBlocks( void )
{
   return nMemoryChunks;
}

/*
 *
 */
ULONG MemoryGetTotalSize( void )
{
   return szMemoryChunks;
}

/*
 *
 */
PSTR StringCreate( char* psz )
{
   PSTR pszNew;
   if( psz == NULL ) return NULL;
   pszNew = (PSTR)SpecialAlloc( strlen( psz ) + 1 );
   strcpy( pszNew, psz );
   return pszNew;
}

/*
 *
 */
void StringDelete( PSTR psz )
{
   if( psz != NULL )
      Delete( psz );
}

/*
 *
 */
size_t StringLength( char* psz )
{
   if( psz == NULL )
      return 0;
   else
      return strlen( psz );
}

/*
 *
 */
int StringICompare( char* psz1, char* psz2 )
{

#  if defined( WIN32 )
#     define stricmp _stricmp
#  elif defined( __VXWORKS__ )
#     define stricmp strcoll
#  else
#     define stricmp strcasecmp
#  endif

   if( psz1 == NULL || psz2 == NULL )
      return ~0;
   else
      return stricmp( psz1, psz2 );
}

/*
 *
 */
PSTR StringReplace( PSTR* ppszOld, char* psz )
{
   if( 0 == StringCompare( *ppszOld, psz ) )
      return *ppszOld;
   else
   {
      StringDelete( *ppszOld );
      return *ppszOld = StringCreate( psz );
   }
}

/*
 *
 */
char* StringSplit( char* psz, char SplitChar )
{
   char c;

   for( ; c = *psz; psz++ )
   {
      if( c == SplitChar )
      {
         *psz = '\0';
         return psz + 1;
      }
   }

   return psz;
}

/*
 *
 */
char* StringToken( char* pszThis, char* pszDelims, char** ppszNext )
{
   size_t szToken;

   if( pszThis == NULL )
      pszThis = *ppszNext;

   while( *pszThis != '\0' && 0 == ( szToken = strcspn( pszThis, pszDelims ) ) )
      pszThis++; /* skipping leading delimiters */

   switch( *pszThis )
   {
      case '\0':
         return NULL;

      case '"': /* remove this case if there's no need to treat quotation marks specially */
         return StringToken( pszThis + 1, "\"", ppszNext );

      default:
         if( *( *ppszNext = pszThis + szToken ) != '\0' )
            *(*ppszNext)++ = '\0';

         return pszThis;
   }
}

/*
 *
 */
void StringPrint( char* pszFormat, ... )
{
   va_list pArg;
   va_start( pArg, pszFormat );
   vfprintf( stderr, pszFormat, pArg );
   va_end( pArg );
}

/*
 *
 */
void StringIndent( int nSpaces )
{
   char pszBuf[128];
   int i;

   for( i = 0; i < nSpaces; i++ )
      pszBuf[i] = ' ';

   pszBuf[nSpaces] = '\0';

   StringPrint( pszBuf );

   return;
}

/*
 *
 */
int DivideInts( int nDivident, int nDivisor )
{
   int nQuotient;

   if( nDivisor == 0 )
      return ~0;

   nQuotient = nDivident / nDivisor;

   if( nDivident - nQuotient * nDivisor >= ( nQuotient + 1 ) * nDivisor - nDivident )
      nQuotient++;

   return nQuotient;
}

/*
 *
 */
PLIST ListCreate( void )
{
   PLIST pList;

   pList = New( LIST );

   pList->pNodeHead = pList->pNodeTail = NULL;
   pList->nObjectsCount = 0;

   return pList;
}

/*
 *
 */
void ListDelete( PLIST pList )
{
   ListRemoveAllNodes( pList );
   Delete( pList );
   return;
}

/*
 *
 */
PNODE ListAddHead( PLIST pList, PVOID pObj )
{
   PNODE pNode = New( NODE );

   pNode->pObject = pObj;

   pList->nObjectsCount++;

   if( NULL != ( pNode->pNodeNext = pList->pNodeHead ) )
      pNode->pNodeNext->pNodePrev = pNode;
   else
      pList->pNodeTail = pNode;

   pNode->pNodePrev = NULL;

   pList->pNodeHead = pNode;

   return pNode;
}

/*
 *
 */
PNODE ListAddTail( PLIST pList, PVOID pObj )
{
   PNODE pNode = New( NODE );

   pNode->pObject = pObj;

   pList->nObjectsCount++;

   if( NULL != ( pNode->pNodePrev = pList->pNodeTail ) )
      pNode->pNodePrev->pNodeNext = pNode;
   else
      pList->pNodeHead = pNode;

   pNode->pNodeNext = NULL;

   pList->pNodeTail = pNode;

   return pNode;
}

/*
 *
 */
void ListRemoveAllNodes( PLIST pList )
{
   PNODE pNode, pNodeNext;

   for( pNode = pList->pNodeHead; pNode; pNode = pNodeNext )
   {
      pNodeNext = pNode->pNodeNext;
      Delete( pNode );
   }

   pList->pNodeHead = pList->pNodeTail = NULL;
   pList->nObjectsCount = 0;

   return;
}

/*
 *
 */
void ListRemoveNode( PLIST pList, PNODE pNode )
{
   if( pNode == pList->pNodeHead )
      pList->pNodeHead = pNode->pNodeNext;
   else
      pNode->pNodePrev->pNodeNext = pNode->pNodeNext;

   if( pNode == pList->pNodeTail )
      pList->pNodeTail = pNode->pNodePrev;
   else
      pNode->pNodeNext->pNodePrev = pNode->pNodePrev;

   pList->nObjectsCount--;

   Delete( pNode );

   return;
}

/*
 *
 */
PNODE ListFindByPtr( PLIST pList, PVOID pObj )
{
   PNODE pNode;

   for( pNode = pList->pNodeHead; pNode; pNode = pNode->pNodeNext )
      if( pNode->pObject == pObj )
         return pNode;

   return NULL;
}

/*
 *
 */
PNODE ListFindByData( PLIST pList, PVOID pObjPattern, ObjectCompareProc pfnObjectCompare )
{
   PNODE pNode;

   for( pNode = pList->pNodeHead; pNode; pNode = pNode->pNodeNext )
      if( 0 == (*pfnObjectCompare)( pObjPattern, pNode->pObject ) )
         return pNode;

   return NULL;
}

/*
 *
 */
char* GetDateTimeString( time_t tm )
{
  static char szBuf[32];

  strftime( szBuf, 32, "[%m/%d/%y %H:%M:%S]", localtime( &tm ) );

  return szBuf;
}


/*
 *
 */
char* GetOption( char* pszSection, char* pszOption, char* pszDefaultValue )
{
   char* pszValue;

   pszValue = pszDefaultValue; /* !! */

   return pszValue;
}


/*
 *
 */
char* ParamGetValue( int argc, char** argv, char* pszParam, char* pszDefaultValue )
{
   int i;

   for( i = 1; i < argc; i++ )
      if( 0 == StringCompare( argv[i], pszParam ) && ++i < argc )
         return argv[i];

   return pszDefaultValue;
}

/*
 *
 */
BOOL ParamIsPresent( int argc, char** argv, char* pszParam )
{
   int i;

   for( i = 1; i < argc; i++ )
      if( 0 == StringCompare( argv[i], pszParam ) )
         return TRUE;

   return FALSE;
}

/*
 *
 */
void DebugFatalErrorBreak( char* pszText, char* pszFile, int nLine )
{
   StringPrint( "\nFatal Error: %s, File: %s, Line: %d\n", pszText, pszFile, nLine );

   SpecialDebugBreak();

   return;
}

/*
 *
 */
BOOL DebugLevelInit( BOOL bLogEnabled )
{
   bDebugLog = bLogEnabled;
   return TRUE;
}

/*
 *
 */
void DebugLog( char* pszFormat, ... )
{
   va_list pArg;

   if( FALSE == bDebugLog )
      return;

   va_start( pArg, pszFormat );
   vfprintf( stderr, pszFormat, pArg );
   va_end( pArg );
   return;
}

/*
 *
 */
void jcStringPack( char* pszString, UCHAR** ppszBuffer, int* pnSize )
{
   size_t nSize;

   nSize = strlen( pszString ) + 1;
   strcpy( *ppszBuffer, pszString );

   *ppszBuffer += nSize;
   *pnSize += nSize;

   return;
}

/*
 *
 */
char* jcStringUnpack( UCHAR** ppszBuffer )
{
   char* pszString = *ppszBuffer;

   *ppszBuffer += strlen( pszString ) + 1;

   return pszString;
}

/*
 *
 */
void jcBufferPack( PVOID pData, USHORT nDataSize, UCHAR** ppszBuffer, int* pnSize )
{
   if( nDataSize == 0 ) return;

   memcpy( *ppszBuffer, pData, nDataSize );

   *ppszBuffer += nDataSize;
   *pnSize += nDataSize;

   return;
}

/*
 *
 */
PVOID jcBufferUnpack( USHORT nDataSize, UCHAR** ppszBuffer )
{
   PVOID pData;

   if( nDataSize == 0 ) return NULL;

   pData = *ppszBuffer;
   *ppszBuffer += nDataSize;

   return pData;
}

/*
 *
 */
void jcULongPack( ULONG uLong, UCHAR** ppszBuffer, int* pnSize )
{
   UCHAR* pszBuffer = *ppszBuffer;

   pszBuffer[0] = (UCHAR)( ( uLong >> 0  ) & 0xFF );
   pszBuffer[1] = (UCHAR)( ( uLong >> 8  ) & 0xFF );
   pszBuffer[2] = (UCHAR)( ( uLong >> 16 ) & 0xFF );
   pszBuffer[3] = (UCHAR)( ( uLong >> 24 ) & 0xFF );

   *ppszBuffer += 4;
   *pnSize += 4;

   return;
}

/*
 *
 */
ULONG jcULongUnpack( UCHAR** ppszBuffer )
{
   ULONG uLong;
   UCHAR* pszBuffer = *ppszBuffer;

   uLong = ( (ULONG)pszBuffer[0] << 0  ) |
           ( (ULONG)pszBuffer[1] << 8  ) |
           ( (ULONG)pszBuffer[2] << 16 ) |
           ( (ULONG)pszBuffer[3] << 24 );

   *ppszBuffer += 4;

   return uLong;
}

/*
 *
 */
void jcUShortPack( USHORT uShort, UCHAR** ppszBuffer, int* pnSize )
{
   UCHAR* pszBuffer = *ppszBuffer;

   pszBuffer[0] = (UCHAR)( ( uShort >> 0  ) & 0xFF );
   pszBuffer[1] = (UCHAR)( ( uShort >> 8  ) & 0xFF );

   *ppszBuffer += 2;
   *pnSize += 2;

   return;
}

/*
 *
 */
USHORT jcUShortUnpack( UCHAR** ppszBuffer )
{
   USHORT uShort;
   UCHAR* pszBuffer = *ppszBuffer;

   uShort = ( (USHORT)pszBuffer[0] << 0  ) |
            ( (USHORT)pszBuffer[1] << 8  );

   *ppszBuffer += 2;

   return uShort;
}
