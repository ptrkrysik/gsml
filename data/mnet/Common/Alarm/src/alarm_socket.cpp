/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : AlarmTask.h                                         */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 9/18/2000                                           */
/* Description : This file contains alarm task socket handling       */
/*               routines                                            */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |07/14/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/* Bhawani  |03/10/01| Added alarm_raiseForGp10 Api                  */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#ifndef _ALARM_SOCKET_CPP_
#define _ALARM_SOCKET_CPP_

#include "AlarmTask.h"

#include "alarm_interface.h"
#include <time.h>
#include <selectLib.h>

typedef  int OS_SPECIFIC_SOCKET_HANDLE;

/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_SOCKET_HANDLE SOCKET_UdpCreate()
{
   alarm_fenter("SOCKET_UdpCreate");
   
   OS_SPECIFIC_SOCKET_HANDLE   soc = socket(AF_INET, SOCK_DGRAM, 0);
   
   if (ERROR == soc)
   {
      alarm_error(("Could not open datagram socket %s\n", strerror(errno)));
   }
   
   alarm_fexit();
   return soc;
}


/*-----------------------------------------------------------------------*/

static bool SOCKET_UdpBind(OS_SPECIFIC_SOCKET_HANDLE soc, ubyte2 port)
{
   struct sockaddr_in  LocalAddr;
   bool                status = true;
   alarm_fenter("SOCKET_UdpBind");
   
   
   
   /* ask the system to allocate a port and bind to INADDR_ANY */
   memset(&LocalAddr, 0x00, sizeof(LocalAddr));
   
   /* get system to allocate a port number by binding a host address */
   LocalAddr.sin_family        = AF_INET;
   LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
   LocalAddr.sin_port          = htons(port);
   
   /* bind socket to local address */
   if (ERROR == bind(soc, (struct sockaddr *) &LocalAddr, sizeof(LocalAddr)))
   {
      alarm_error(("Could not bind datagram socket to port %d:  %s\n", port, strerror(errno)));
      status = false;
      
   }
   
   alarm_fexit();
   return status;
}

/*-----------------------------------------------------------------------*/

static bool SOCKET_SetRecvBuffer(OS_SPECIFIC_SOCKET_HANDLE soc, int nBytes)
{
   alarm_fenter("SOCKET_SetRecvBuffer");
   bool status = true;
   
   if (ERROR == setsockopt(soc, SOL_SOCKET, SO_RCVBUF, (char *) &nBytes, sizeof(int)))
   {
      
      alarm_error(("Could not set Receive buffer to %d\n",  nBytes));
      status = false;
   }
   alarm_fexit();
   return status;
}


/*-----------------------------------------------------------------------*/

void SOCKET_UdpClearRecvQ(OS_SPECIFIC_SOCKET_HANDLE sock)
{
   struct timeval       timeout;
   fd_set               readFdSet;
   static char          buffer[512];
   int                  bufferlen = 512;
   struct sockaddr_in   Addr;
   int                  iAddrLen;

   
   alarm_fenter("SOCKET_UdpClearInQueue");

   
   timeout.tv_sec  = 0;
   timeout.tv_usec = 0;

   FD_ZERO(&readFdSet);
   FD_SET(sock, &readFdSet);

   
   if(select(FD_SETSIZE, &readFdSet, NULL, NULL, &timeout) > 0)
   {
      /* Socket is ready for reading */
      
      bufferlen = recvfrom(sock, buffer, bufferlen, 0, (struct sockaddr *)&Addr, &iAddrLen);
      if (bufferlen > 0)
      {
         alarm_warning (("Cleared %d bytes of unexpected data from %s:%u\n",
            bufferlen, inet_ntoa(Addr.sin_addr), Addr.sin_port));
      }
   }
   alarm_fexit();
} 

/*-----------------------------------------------------------------------*/

extern bool
SOCKET_UdpSendTo(OS_SPECIFIC_SOCKET_HANDLE sock, 
                 struct sockaddr_in *pAddrTo,
                 char *message, 
                 ubyte4 msglen)
{
   alarm_fenter("SOCKET_UdpSendTo");
   bool status = true;
   char pszIPbuf[20];
   
   
   if (ERROR == sendto(sock, message, msglen, 0,
      (struct sockaddr *) pAddrTo,
      sizeof(struct sockaddr)) )
   {
      status = false;
   }
   inet_ntoa_b(pAddrTo->sin_addr, pszIPbuf);
   alarm_debug(( "Sending %d bytes to Alarm server (ip=%s, port=%d) \n", msglen, pszIPbuf, ntohs(pAddrTo->sin_port)));
   alarm_dump((ubyte1 *) message, msglen);
   
   // clear any message in the incoming queue
   SOCKET_UdpClearRecvQ(sock);

   alarm_fexit();
   return status;
}

/*-----------------------------------------------------------------------*/

extern bool
SOCKET_UdpReceive(OS_SPECIFIC_SOCKET_HANDLE sock,
                  char *msg,
                  ubyte4 *msgSize,
                  ubyte4 *peerAddr)
{
   struct sockaddr_in pAddr;
   int iAddrLen;
   int retval  = recvfrom(sock, msg, *msgSize,  0, (struct sockaddr *)&pAddr, &iAddrLen );
   
   if (retval == ERROR)
   {
      *msgSize = 0;
      return false;
   }
   
   *msgSize = retval;
   *peerAddr = pAddr.sin_addr.s_addr;
   return true;
}


/*-----------------------------------------------------------------------*/

extern bool SOCKET_Close(OS_SPECIFIC_SOCKET_HANDLE sock)
{
   bool retval = true;
   alarm_fenter("SOCKET_Close");
   
   if (ERROR == close(sock) )
   {
      alarm_error(("Could not close socket %s\n", strerror(errno)));
      retval = false;
   }
   
   alarm_fexit();
   return retval;
}

/*-----------------------------------------------------------------------*/

ubyte4 GetHostAddress(char *pszHostName)
{
   
   return  hostGetByName ( pszHostName );
   
}

/*-----------------------------------------------------------------------*/

static ubyte4 GetLocalHostAddress()
{
#define MAX_PATH 80
   char   szHostName[MAX_PATH];
   ubyte4  ulHostAddr = 0;
   
   if (ERROR != gethostname(szHostName, MAX_PATH-1))
      ulHostAddr = GetHostAddress ( szHostName );
   
   return (ulHostAddr);
}

/*-----------------------------------------------------------------------*/

static bool initialized = false;
/*
static AlarmViperCellToAmsMsg amsMsg;
*/
static struct sockaddr_in addrTo;
static OS_SPECIFIC_SOCKET_HANDLE sock;

bool
initSocketAndMsgHdr()
{
   char pszIpBuf[20];
   alarm_fenter("initSocketAndMsgHdr");

   if (!initialized)
   {
      int ip;
      /* 1.  prepare udp socket */
      sock = SOCKET_UdpCreate();
      
      if (sock == ERROR)
      {
         alarm_fexit();
         return false;
      }
      
      if (SOCKET_UdpBind(sock, ALARM_interfaceUdpPortClient) != true)
      {
         SOCKET_Close(sock);
         alarm_fexit();
         return false;

      }
      
      /* 2. get IP address from MIB */
      
      if (oam_getMibIntVar(MY_MIB_APM1_IP_ADDRESS, (long *) &ip) != STATUS_OK)
      {
         // alarm_error(("Unable to get alarm server's IP from the MIB\n"));
         SOCKET_Close(sock);
         return false;
      }
      
      /* 3. prepare the the server's address */
      memset (&addrTo, 0, sizeof(sockaddr_in));
      
      addrTo.sin_family = AF_INET;
      addrTo.sin_addr.s_addr = ip;
      addrTo.sin_port = htons((ubyte2)ALARM_interfaceUdpPortServer);
      
      inet_ntoa_b( addrTo.sin_addr, pszIpBuf);

      /* Set small receive buffer */
      SOCKET_SetRecvBuffer(sock, ALARM_defaultRecvBuffer);

      alarm_debug (("APM1 server: ip = %s, port=%d\n", pszIpBuf, ALARM_interfaceUdpPortServer));
      initialized = true;
   }
   alarm_fexit();
   return true;
}

/*-----------------------------------------------------------------------*/

bool
alarm_sendAlarmEventToAms(ubyte4 id, ubyte4 code, ubyte4 timestamp, ubyte4 moduleId, char* opt1, char* opt2)
{
   AlarmViperCellToAmsMsg amsMsg;
   ubyte4 opt;
   
   alarm_fenter("alarm_sendAlarmMsgToAms");
   
   if (!initSocketAndMsgHdr())
   {
      alarm_fexit();
      return false;
   }
   
   memset(&amsMsg, 0, sizeof(AlarmViperCellToAmsMsg));
   
   amsMsg.hdr.alarmSrcType = (ubyte1) MY_PRODUCT_ID;
   amsMsg.hdr.alarmMsgType = ALARM_msgTypAlarmEvent;
   amsMsg.hdr.alarmMsgLen = htons(sizeof(AlarmEventData));
   
   strncpy(amsMsg.body.eventData.alarmSrcModuleName, GetMnetModuleName((MNET_MODULE_ID)moduleId),
      ALARM_viperCellNameMaxSize);
   amsMsg.body.eventData.alarmSrcIp = GetLocalHostAddress();
   
   amsMsg.body.eventData.alarmId = htonl(id);
   amsMsg.body.eventData.alarmCode = htonl(code);
   amsMsg.body.eventData.alarmTimestamp = htonl(timestamp);
   amsMsg.body.eventData.alarmSrcModuleId = htonl(moduleId);
   if (NULL != opt1)
   {
      opt = atoi(opt1);
      amsMsg.body.eventData.opt1 = htonl(opt);
   }
   
   if (NULL != opt1)
   {
      opt = atoi(opt2);
      amsMsg.body.eventData.opt2 = htonl(opt);
   }
   SOCKET_UdpSendTo(sock, &addrTo, (char *) &amsMsg, ALARM_eventMsgLen);
   
   alarm_fexit();
   return true;
   
}

/*-----------------------------------------------------------------------*/
#ifdef MNET_PRODUCT_SPECIFIC_GMC

JC_STATUS alarm_raiseForGp10
(
 MNET_MODULE_ID mid,        /* Gp10 source module ID      */
 AlarmCode_t alarm_code,    /* Gp10 Alarm code            */
 unsigned long ip,          /* GP10 IP address            */
 char *sourceName           /* GP10's official name       */
 )
{
   AlarmViperCellToAmsMsg amsMsg;
   const int MODULE_VBLINK = 0x11;           /* ViperBase - ViperCell link client */
   static  char  gp10SrcName[] = "VBLink";   /* GP10's Module name                */
   
   alarm_fenter("alarm_raiseForGp10");
   
   if (!initSocketAndMsgHdr())
   {
      alarm_fexit();
      return JC_ERROR;
   }

   
   memset(&amsMsg, 0, sizeof(AlarmViperCellToAmsMsg));
   
   amsMsg.hdr.alarmSrcType = (ubyte1) MNET_PRODUCT_GP10;
   amsMsg.hdr.alarmMsgType = ALARM_msgTypAlarmEvent;
   amsMsg.hdr.alarmMsgLen = htons(sizeof(AlarmEventData));
   
   strncpy(amsMsg.body.eventData.alarmSrcModuleName, gp10SrcName, ALARM_viperCellNameMaxSize);
   amsMsg.body.eventData.alarmSrcIp = ip;
   
   amsMsg.body.eventData.alarmId = htonl(ALARM_unknownAlarmId);
   amsMsg.body.eventData.alarmCode = htonl(alarm_code);
   amsMsg.body.eventData.alarmTimestamp = htonl(time(NULL));
   amsMsg.body.eventData.alarmSrcModuleId = htonl(mid);
   SOCKET_UdpSendTo(sock, &addrTo, (char *) &amsMsg, ALARM_eventMsgLen);
   
   alarm_fexit();
   return true;
   
}

#endif

/*-----------------------------------------------------------------------*/

bool AlarmTask::sendSummaryUpdateToAms()
{
   AlarmViperCellToAmsMsg amsMsg;
   
   alarm_fenter("AlarmTask::sendSummaryUpdateToAms");
   ubyte1 critical, major, minor;
   
   if (!initSocketAndMsgHdr())
   {
      alarm_fexit();
      return false;
   }
   
   /* 4. initialize the fixed fields */
   memset(&amsMsg, 0, sizeof(AlarmViperCellToAmsMsg));
   
   // populate the header
   amsMsg.hdr.alarmSrcType = ALARM_srcTypViperCell;
   amsMsg.hdr.alarmMsgType = ALARM_msgTypSummaryUpdate;
   amsMsg.hdr.alarmMsgLen = htons(sizeof(AlarmSummaryUpdateData));
   
   strncpy(amsMsg.body.summaryUpdateData.alarmSrcName, AlarmTask::srcName(),
      ALARM_viperCellNameMaxSize);
   amsMsg.body.summaryUpdateData.alarmSrcIp = GetLocalHostAddress();
   
   getOutstandingAlarmCount(critical, major, minor);
   amsMsg.body.summaryUpdateData.alarmCriticalCount = critical;
   amsMsg.body.summaryUpdateData.alarmMajorCount = major;
   amsMsg.body.summaryUpdateData.alarmMinorCount = minor;
   SOCKET_UdpSendTo(sock, &addrTo, (char *) &amsMsg, ALARM_summaryUpdateMsgLen);
   
   alarm_fexit();
   return true;
}


void AlarmTask::quit ()
{
   _quit = true;
}

void AlarmTask::closeSocket ()
{
   initialized = false;
   SOCKET_Close(sock);
}


#endif /* _ALARM_SOCKET_CPP_ */
