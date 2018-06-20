
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: viperip.h 													*
 *																						*
 *	Description			: Data Structures for IP pkt framing for viperip		        *
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#ifndef VIPER_IP_HDR_INC
#define VIPER_IP_HDR_INC



#include "vxworks.h"
#include "etherlib.h"
#include "netinet\if_ether.h"
#include "stdlib.h"
#include "stdio.h"
#include "semlib.h"

/*
 * Definitions
 */

#define VIPERCELL_ETHERNET_INTERFACE    "motfec0"
#define VIPER_IP_PROTOCOL_TYPE          0x0800
#define VIPER_IP_PROTOCOL_VERSION       4
#define VIPER_IP_HDR_OFFSET             14
#define VIPER_IP_HDR_SIZE               20
#define VIPER_IP_UDP_HDR_OFFSET         VIPER_IP_HDR_OFFSET+VIPER_IP_HDR_SIZE
#define VIPER_IP_UDP_HDR_SIZE           8
#define VIPER_IP_TYPE_OF_SERVICE        0xa0 /* For CRITIC/ECP IP Packet precedence */
#define VIPER_IP_PROTOCOL_TYPE_UDP      IPPROTO_UDP
#define VIPER_IP_DEFAULT_TTL            32

/* 
 * Data structures
 *
 */
typedef struct {
    unsigned char   version:4;
    unsigned char   hdrLength:4;
    unsigned char   tos;
    unsigned short  totalLength;
    unsigned short  id;
    unsigned short  flags:3;
    unsigned short  fragOffset:13;
    unsigned char   ttl;
    unsigned char   protocol;
    unsigned short  checksum;
    unsigned char   sourceAddress[4];
    unsigned char   destAddress[4];
}VIPER_IP_HDR, *PVIPER_IP_HDR;


typedef struct {
    unsigned short  sourcePort;
    unsigned short  destPort;
    unsigned short  udpLength;
    unsigned short  udpChecksum;
}VIPER_UDP_HDR, *PVIPER_UDP_HDR;



typedef struct {
    union {
        unsigned long s_addr;
        unsigned char s_bytes[4];
    } addr;
}internetAddress;




/*
 * Function prototypes
 */
void vipInit();
void vipSetDefaultGateway(char *gwIpAddress);
void vipGetEthernetAddress(char *ipAddress, unsigned char *etherAddress);
BOOL vipIsPeerOnSameSubnet(char *peerAddress);
void vipConstructNetHeaders(unsigned long remoteIp,
                    unsigned short remotePort,
                    unsigned short localPort,
                    unsigned char *pktBuffer,
                    unsigned short *dataOffset
                    );
char *vipGetLocalIpAddress(void);
unsigned short vipComputeIpCksum(unsigned short *addr,int len);
void vipSend(unsigned char *pktBuffer, int dataLength);
void vipAddRtpPort(unsigned short rtpPort, SEM_ID rtpSemId,unsigned char *recvBuffPtr);


#ifdef DEBUG
void StartVipTest(int pkts,int ticks, char *destAddr);
void VipTesterProc(void);
#endif

#endif /* #ifndef VIPER_IP_HDR_INC */