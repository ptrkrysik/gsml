#ifndef __DEFS_H__
#define __DEFS_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : defs.h
// Author(s)   : Igal Gutkin
// Create Date : 6/21/00
// Description :  Common defs.h to be used by APM, CDR and possibly
//                other modules or applications.
//
// *******************************************************************

/*******************************************************************
 typedef
********************************************************************/

// Elementary type definitions
typedef unsigned char       ubyte ;
typedef unsigned short      ubyte2;
typedef unsigned long       ubyte4;
typedef char                sbyte ;
typedef short               sbyte2;
typedef long                sbyte4;


#ifndef min
 #define min(a,b)           (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX_PATH
 #define MAX_PATH           260
#endif

#define SOC_STATUS_OK       0

#define SOC_STATUS          int
#define RET_SOC_STATUS      int
#define ALARM_STATUS        int

#if defined(_WINDOWS_) || defined(WIN32)
 #define DIR_DELIMITER              '\\'
 #define ERROR_NUM                  (WSAGetLastError())
 #define SOC_STATUS_ERROR           SOCKET_ERROR
 #define OS_SPECIFIC_SOCKET_HANDLE  SOCKET

#else
 #define DIR_DELIMITER              '/'
 #define ERROR_NUM          errno
 #define SOC_STATUS_ERROR   (-1)
 typedef ubyte4             OS_SPECIFIC_SOCKET_HANDLE;
 typedef ubyte4             OS_SPECIFIC_DGRAM_HANDLE;

#endif // WINDOWS


// Windows definitions
#if !defined (_WINDOWS_) && !defined (WIN32)
 #define INVALID_SOCKET     (OS_SPECIFIC_SOCKET_HANDLE)(~0)

 // manifest constants for shutdown()
 #define SD_RECEIVE         0x00
 #define SD_SEND            0x01
 #define SD_BOTH            0x02

 // Windows redirected socket error codes
 #define WSAEMSGSIZE        EMSGSIZE
 #define WSAENOBUFS         ENOBUFS
 #define WSAEMSGSIZE        EMSGSIZE
 #define WINAPI
 #define FAR

 // Windows data types
 typedef char               *LPSTR , *PSTR;
 typedef char               *DWORD ;
 typedef const char         *LPCSTR;

#endif


#if defined (_WINDOWS_) || defined (WIN32)
    #define SysCloseSocket(x) closesocket(x)
#else
    #define SysCloseSocket(x) close(x)
#endif


// Internet address conversion
#define BYTE_1(a)       (((a)>>24)&0xFF)
#define BYTE_2(a)       (((a)>>16)&0xFF)
#define BYTE_3(a)       (((a)>>8)&0xFF)
#define BYTE_4(a)       ((a)&0xFF)

#if    defined CPU_FAMILY && (CPU_FAMILY==PPC)
 // PowerPC on VxWorks
 #define IP_BYTES(ip)   BYTE_1(ip),BYTE_2(ip),BYTE_3(ip),BYTE_4(ip)
#else
 // Wintel platform
 #define IP_BYTES(ip)   BYTE_4(ip),BYTE_3(ip),BYTE_2(ip),BYTE_1(ip)
#endif


// Socket error codes
#define SOCKET_STATUS_OK              SOC_STATUS_OK
#define SOCKET_STATUS_MSG_TOO_LONG    1
#define SOCKET_STATUS_TEMP_SOC_ERR    2
#define SOCKET_STATUS_FATAL_SOC_ERR   SOC_STATUS_ERROR

// Default root directory
#if defined   (MNET_GP10)
 #define DEF_ROOT_DIR   "/ata/vipercall";
#elif defined (MNET_GMC)
 #define DEF_ROOT_DIR   "/sd0/gmc";
#else
 #define DEF_ROOT_DIR   ".";
#endif

#endif /* __DEFS_H__ */