/* platform.c
 * $Id: platform.c,v 1.4 1999-07-30 10:38:42-07 olet Exp $
 */

#include <stdlib.h>
#include <stdio.h>

#include "support.h"

#if defined( WIN32 )

#  include <windows.h>

static HWND hMainWnd;
LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#define MNGM_TELNET_ACCEPT ( WM_USER + 1 )
#define MNGM_TELNET_READ   ( WM_USER + 2 )
#define MNGM_CLIENT_ACCEPT ( WM_USER + 3 )
#define MNGM_CLIENT_READ   ( WM_USER + 4 )

#elif defined( __VXWORKS__ )

/* VxWorks */
#  include <taskLib.h>
#  include <ioLib.h>

/* Rad */
#  include <mti.h>
#  include <seli.h>
#  include <pi.h>

#define RAD_MAX_TIMERS 100
static HSTIMER hsTimer;
static TIMER jcTimersArray[RAD_MAX_TIMERS];
void VxWorksTimerCallBack( void* pData );

void RadPipeCallBack( int hPipe, piEvents Event, BOOL Error, void* pData );

#else

#  include <signal.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/socket.h>

#endif

/* Telnet Support */

#define JC_MAX_TELNET_IN_STRING   256
#define JC_MAX_TELNET_OUT_STRING  16384
#define JCTELNETPROMPT  "\r\nViperBase> "

typedef struct
{
   SOCKET hSocket;
   char Command[JC_MAX_TELNET_IN_STRING]; /* current command */
   unsigned short nBytesInCommand;

}JCTELNET, *PJCTELNET;

static PLIST pListTelnets;
static pfnCommonCallBack pfnCommandCallBack;
static pfnCommonCallBack pfnMngmAPICallBack;
/*
 *
 */
BOOL PlatformLayerInit( void )
{

#  if defined( WIN32 )

   static char szAppName[] = "ViperBase";
   WNDCLASS wc;
   HANDLE hInstance = GetModuleHandle(NULL);

   /* increasing process priority */
   SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

   /* increasing scrolling buffer */
/*
   {
      HANDLE hStdout;
      COORD coord;

      coord.X = 80;
      coord.Y = 1000;

      hStdout = GetStdHandle( STD_OUTPUT_HANDLE );

      SetConsoleScreenBufferSize( hStdout, coord );
   }
*/
   /* creating invisible main window */
   wc.style = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = WndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
   wc.hCursor = LoadCursor( NULL, IDC_ARROW );
   wc.hbrBackground = GetStockObject( WHITE_BRUSH );
   wc.lpszMenuName = "";
   wc.lpszClassName = szAppName;

   RegisterClass( &wc ) ;

   hMainWnd = CreateWindow( szAppName, szAppName, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, hInstance, NULL );

#  elif defined( __VXWORKS__ )

   int i, hShell;

   if( ERROR != ( hShell = open( "/vio/0", O_CREAT | O_RDWR, 0 ) ) )
   {
      ioGlobalStdSet( STD_OUT, hShell );
      ioGlobalStdSet( STD_ERR, hShell );
   }


   if( 0 == ( hsTimer = mtimerInit( RAD_MAX_TIMERS, NULL ) ) )
   {
      StringPrint( "Error in mtimerInit()\n" );
      return FALSE;
   }

   for( i = 0; i < RAD_MAX_TIMERS; i++ )
      jcTimersArray[i].pfnCallBack = NULL; /* indicates it is available */

#  else

   signal( SIGPIPE, SIG_IGN );

#  endif

   pListTelnets = ListCreate();

   return TRUE;
}

/*
 *
 */
BOOL PlatformLayerDown( void )
{

#if defined( __VXWORKS__ )

   mtimerEnd( hsTimer );

#else

#endif

   return TRUE;
}

/*
 *
 */
void SpecialSleep( int nSeconds )
{
#  if defined( WIN32 )

      Sleep( nSeconds * 1000 );

#  elif defined( __VXWORKS__ )

      taskDelay( nSeconds * 100 ); /* not a precise timeout! */

#  else

      sleep( nSeconds );

#  endif

   return;
}

/*
 *
 */
void SpecialDebugBreak( void )
{
#  ifdef WIN32
      DebugBreak();

#  else
      abort();

#  endif

   return;
}

#if defined( __VXWORKS__ )

/*
 *
 */
BOOL MainLoop( void )
{
   while( 1 ) seliSelect();

   return TRUE;
}

/*
 *
 */
PRADPIPE RadPipeOpen( char* Name, int nMsgSize, int nMaxMsgs, pfnCommonCallBack pfnUserCallBack )
{
   int hPipe;
   PRADPIPE pPipe;
/*
   DebugLog( "Creating Pipe '%s', MsgSize = %d, MaxMsgs = %d\n", Name, nMsgSize, nMaxMsgs );
*/
   if( ERROR == piInit() )
   {
      StringPrint( "Error in piInit()\n" );
      return FALSE;
   }

   if( ERROR == ( hPipe = piOpen( Name, nMsgSize, nMaxMsgs ) ) &&
      0 > ( hPipe = open( Name, O_RDWR, 644 ) ) )
   {
      StringPrint( "Error in piOpen()\n" );
      return FALSE;
   }

   pPipe = New( RADPIPE );
   pPipe->hPipe = hPipe;
   pPipe->pfnUserCallBack = pfnUserCallBack;
   pPipe->nMsgSize = nMsgSize;
   pPipe->pMsg = SpecialAlloc( nMsgSize );

   piCallOn( hPipe, piEvRead, (piCallback)RadPipeCallBack, pPipe );

   return pPipe;
}

/*
 *
 */
BOOL RadPipeWrite( PRADPIPE pPipe, PVOID pMessage )
{

   if( pPipe->nMsgSize != piWrite( pPipe->hPipe, (UINT8*)pMessage, pPipe->nMsgSize ) )
   {
      DebugLog( "Error in piWrite()\n" );
      return FALSE;
   }

   return TRUE;
}

/*
 *
 */
void RadPipeCallBack( int hPipe, piEvents Event, BOOL Error, void* pData )
{
   PRADPIPE pPipe;
   int i, nPipeMsgs;

   pPipe = (PRADPIPE)pData;

   if( Error || pPipe->hPipe != hPipe )
   {
      DebugLog( "Error in RadPipeCallBack()\n" );
      return;
   }

   ioctl( hPipe, FIONMSGS, (int)&nPipeMsgs );

   for( i = 0; i < nPipeMsgs; i++ )
   {
      if( pPipe->nMsgSize != piRead( hPipe, (UINT8*)pPipe->pMsg, pPipe->nMsgSize ) )
      {
         DebugLog( "Error in piRead()\n" );
         return;
      }

      (*pPipe->pfnUserCallBack)( pPipe->pMsg );
   }

   return;
}

/*
 *
 */
BOOL RadPipeClose( PRADPIPE pPipe )
{
   piClose( pPipe->hPipe );
   piEnd();

   SpecialFree( pPipe->pMsg );
   Delete( pPipe );

   return TRUE;
}


#elif defined( WIN32 )

/*
 *
 */
BOOL MainLoop( void )
{
   MSG msg;

   while( GetMessage( &msg, NULL, 0, 0 ) )
   {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   return TRUE;
}

/*
 *
 */
char* toHex(BYTE* data, int len)
{
    static char buf[100];
    char* digits="0123456789ABCDEF";
    unsigned i;
    for (i=0;i<min(len,sizeof(buf)/2-1);i++)
    {
        buf[i*2]=digits[data[i]>>4];
        buf[i*2+1]=digits[data[i]&0xf];
    }
    buf[i*2]=0;
    return buf;
}

/*
 *
 */
LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch( uMsg )
   {
      case WM_TIMER:
      {
         PTIMER pTimer = (PTIMER)wParam;

         (*pTimer->pfnCallBack)( pTimer->pData );

         if( FALSE == pTimer->bMultiple )
            TimerDelete( pTimer );

         return 0;
      }

      case MNGM_TELNET_ACCEPT:
      {
         SOCKET hSocket;
         PJCTELNET pTelnet;

         if( WSAGETSELECTERROR( lParam ) != 0 ||
             INVALID_SOCKET == ( hSocket = accept( (SOCKET)wParam, NULL, NULL ) ) ||
             SOCKET_ERROR == WSAAsyncSelect( hSocket, hMainWnd, MNGM_TELNET_READ,
                                             FD_READ | FD_CLOSE ) )
         {
            DebugLog( "Error accepting telnet client\n" );
            break;
         }

         MngmTelnetSendString( hSocket, "\r\nJetCell ViperBase Telnet Management Interface"
                                        "\r\nPlease see ViperBase manual for commands syntax\r\n" );

         MngmTelnetSendString( hSocket, JCTELNETPROMPT );

         pTelnet = New( JCTELNET );
         pTelnet->hSocket = hSocket;
         pTelnet->nBytesInCommand = 0;

         ListAddTail( pListTelnets, pTelnet );

         DebugLog( "Telnet client connected\n" );

         break;
      }

      case MNGM_CLIENT_ACCEPT:
      {
         SOCKET hSocket;

         if( WSAGETSELECTERROR( lParam ) != 0 ||
             INVALID_SOCKET == ( hSocket = accept( (SOCKET)wParam, NULL, NULL ) ) ||
             SOCKET_ERROR == WSAAsyncSelect( hSocket, hMainWnd, MNGM_CLIENT_READ,
                                             FD_READ | FD_CLOSE ) )
         {
            DebugLog( "Error accepting management client\n" );
            break;
         }

         DebugLog( "Management client connected\n" );

         break;
      }

      case MNGM_TELNET_READ:
      {
         char BufferIn[JC_MAX_TELNET_IN_STRING], BufferOut[JC_MAX_TELNET_OUT_STRING];
         int nBytes;
         PNODE pNode;
         PJCTELNET pTelnet;
         SOCKET hSocket = (SOCKET)wParam;
         MngmCommandData_t CommandData;

         ListForEachNode( pListTelnets, pNode )
         {
            pTelnet = (PJCTELNET)NodeGetPtr( pNode );

            if( hSocket == pTelnet->hSocket )
               break; /* found */
         }

         if( WSAGETSELECTEVENT( lParam ) == FD_CLOSE )
         {
            ListRemoveNode( pListTelnets, pNode );
            Delete( pTelnet );
            DebugLog( "Telnet client disconnected\n" );
            break;
         }

         if( SOCKET_ERROR == ( nBytes = recv( hSocket, BufferIn, JC_MAX_TELNET_IN_STRING, 0 ) ) )
         {
            DebugLog( "MNGM_TELNET_READ: error in recv()\n" );
            break;
         }

         if( BufferIn[0] >= 32 && BufferIn[0] <= 126 ) /* printable ASCII character */
         {
            if( pTelnet->nBytesInCommand + nBytes < JC_MAX_TELNET_IN_STRING - 1 )
            {
               memcpy( pTelnet->Command + pTelnet->nBytesInCommand, BufferIn, nBytes );
               pTelnet->nBytesInCommand += nBytes;
               send( hSocket, BufferIn, nBytes, 0 ); /* echo back */
            }
            else
               MngmTelnetSendString( hSocket, "\a" ); /* beep */
         }
         else
         {
            switch( BufferIn[0] )
            {
               case 0x0D: /* end of line */

                  if( pTelnet->nBytesInCommand > 0 )
                  {
                     MngmTelnetSendString( hSocket, "\r\n" );

                     pTelnet->Command[pTelnet->nBytesInCommand] = '\0';
                     CommandData.pszInput = pTelnet->Command;
                     CommandData.pszOutput = BufferOut;

                     (*pfnCommandCallBack)( (PVOID)&CommandData );

                     MngmTelnetSendString( hSocket, BufferOut );

                     pTelnet->nBytesInCommand = 0; /* reset */
                  }

                  MngmTelnetSendString( hSocket, JCTELNETPROMPT );

                  break;

               case 0x08: /* back space */
               case 0x7F: /* delete */
                  if( pTelnet->nBytesInCommand > 0 )
                  {
                     pTelnet->nBytesInCommand--;
                     MngmTelnetSendString( hSocket, "\b \b" ); /* erase last character */
                  }
                  else
                     MngmTelnetSendString( hSocket, "\a" ); /* beep */

                  break;

               default:  /* for now ignoring everything else */
                  MngmTelnetSendString( hSocket, "\a" ); /* beep */
                  break;
            }
         }

         break;
      }

      case MNGM_CLIENT_READ:
      {
         char BufferIn[1024], BufferOut[JC_MAX_TELNET_OUT_STRING];
         SOCKET hSocket = (SOCKET)wParam;
         MngmCommandData_t RequestData;

         if( WSAGETSELECTEVENT( lParam ) == FD_CLOSE )
         {
            DebugLog( "Management client disconnected\n" );
            break;
         }

         if( SOCKET_ERROR == recv( hSocket, BufferIn, 1024, 0 ) )
         {
            DebugLog( "MNGM_CLIENT_READ: error in recv()\n" );
            break;
         }

         RequestData.pszInput = BufferIn;
         RequestData.pszOutput = BufferOut;

         (*pfnMngmAPICallBack)( (PVOID)&RequestData );

         if( RequestData.nOutput > 0 )
            send( hSocket, BufferOut, RequestData.nOutput, 0 );

         break;
      }

      default:
         break;
   }

   return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

#endif


#if defined( WIN32 )

/*
 *
 */
PTIMER TimerCreate( time_t tmInterval, pfnCommonCallBack pfnCallBack, PVOID pData, BOOL bMultiple )
{
   PTIMER pTimer;

   ExecBreakIf( tmInterval <= 0, "Incorrect interval" );

   pTimer = New( TIMER );

   pTimer->pData = pData;
   pTimer->pfnCallBack = pfnCallBack;
   pTimer->bMultiple = bMultiple;

   if( FALSE == SetTimer( hMainWnd, (UINT)pTimer, 1000 * tmInterval, NULL ) )
   {
      Delete( pTimer );
      DebugLog( "Error in SetTimer()\n" );
      return NULL;
   }

   return pTimer;
}

/*
 *
 */
BOOL TimerDelete( PTIMER pTimer )
{
   if( NULL == pTimer )
      return FALSE;

   if( FALSE == KillTimer( hMainWnd, (UINT)pTimer ) )
      DebugLog( "Error in KillTimer()\n" );

   Delete( pTimer );

   return TRUE;
}


#elif defined( __VXWORKS__ )

/*
 *
 */
PTIMER TimerCreate( time_t tmInterval, pfnCommonCallBack pfnCallBack, PVOID pData, BOOL bMultiple )
{
   int i;
   PTIMER pTimer;

   ExecBreakIf( tmInterval <= 0, "Incorrect interval" );

   for( i = 0, pTimer = jcTimersArray; i < RAD_MAX_TIMERS; i++, pTimer++ )
      if( pTimer->pfnCallBack == NULL )
         goto Found;

   DebugLog( "Error in TimerCreate(): can't find available jcTimersArray slot\n" );
   return NULL;

Found:

   if( 0 > mtimerSet( hsTimer, (LPMTIMEREVENTHANDLER)VxWorksTimerCallBack,
                      (PVOID)pTimer, 1000 * tmInterval ) )
   {
      DebugLog( "Error in mtimerSet()\n" );
      return NULL;
   }

   pTimer->pData = pData;
   pTimer->pfnCallBack = pfnCallBack;
   pTimer->bMultiple = bMultiple;

   return pTimer;
}

/*
 *
 */
void VxWorksTimerCallBack( PVOID pData )
{
   PTIMER pTimer = (PTIMER)pData;

   (*pTimer->pfnCallBack)( pTimer->pData );

   if( FALSE == pTimer->bMultiple )
      TimerDelete( pTimer );

   return;
}

/*
 *
 */
BOOL TimerDelete( PTIMER pTimer )
{
   if( NULL == pTimer )
      return FALSE;

   mtimerResetByValue( hsTimer, (LPMTIMEREVENTHANDLER)VxWorksTimerCallBack, (PVOID)pTimer );

   pTimer->pfnCallBack = NULL; /* indicates it is available */

   return TRUE;
}

#endif

#if defined( WIN32 )

/*
 *
 */
BOOL MngmPortInit( int MngmPort, pfnCommonCallBack pfnMngmCallBack, BOOL bTelnet )
{
   struct sockaddr_in SockAddr;
   SOCKET hSocket;
   int nWinMsgType;

   SockAddr.sin_family = AF_INET;
   SockAddr.sin_addr.s_addr = INADDR_ANY;
   SockAddr.sin_port = htons( (u_short)MngmPort );

   nWinMsgType = bTelnet ? MNGM_TELNET_ACCEPT : MNGM_CLIENT_ACCEPT;

   if( INVALID_SOCKET == ( hSocket = socket( AF_INET, SOCK_STREAM, 0 ) ) ||
       SOCKET_ERROR == bind( hSocket, (LPSOCKADDR)&SockAddr, sizeof( SockAddr ) ) ||
       SOCKET_ERROR == listen( hSocket, 1 ) ||
       SOCKET_ERROR == WSAAsyncSelect( hSocket, hMainWnd, nWinMsgType, FD_ACCEPT ) )
   {
      DebugLog( "MngmPortInit(): error while initializing the socket\n" );
      return FALSE;
   }

   if( bTelnet )
      pfnCommandCallBack = pfnMngmCallBack;
   else
      pfnMngmAPICallBack = pfnMngmCallBack;

   return TRUE;
}

/*
 *
 */
BOOL MngmTelnetSendString( int hSocket, char* pszString )
{
   send( hSocket, pszString, strlen( pszString ), 0 );
   return TRUE;
}

#elif defined( __VXWORKS__ )

#endif

