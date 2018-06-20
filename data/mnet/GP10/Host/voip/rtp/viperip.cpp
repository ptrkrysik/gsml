
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: ViperIp.cpp													*
 *																						*
 *	Description			: Interface library to permit RTP to directly call the motFec	*
 *                        Ethernet controller driver on the ViperCell system            *
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |08/28/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#include <sockLib.h>
#include <inetLib.h>
#include <vxworks.h>
#include <etherlib.h>
#include <netinet\if_ether.h>
#include <stdlib.h>
#include <stdio.h>
#include <taskLib.h>
#include <semlib.h>
#include <wdLib.h>
#include <sysLib.h>
#include <tasklib.h>
#include <iv.h>
#include <vme.h>
#include <net/mbuf.h>
#include <net/unixLib.h>
#include <net/protosw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <memLib.h>
#include <intLib.h>
#include <net/route.h>
#include <iosLib.h>
#include <errnoLib.h>
#include <vxLib.h>
#include <private/funcBindP.h>

#include <cacheLib.h>
#include <logLib.h>
#include <netLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysLib.h>

#include <etherLib.h>
#include <net/systm.h>
#include <sys/times.h>
#include <net/if_subr.h>

#undef ETHER_MAP_IP_MULTICAST
#include <etherMultiLib.h>
#include <end.h>
#include <semLib.h>
#define    END_MACROS
#include <endLib.h>
#include <lstLib.h>
#include <drv\end\motfecend.h>
    
#include "viperip.h"


/*
 * Global variables
 */
struct ifnet    *pInterface;
static unsigned long ipPacketIdentifier=0;
static unsigned char defaultGatewayEther[6];
static char defaultGatewayIpAddress[32];
DRV_CTRL *jetFecController;






/*
 * vipInit
 */
void
vipInit()
{
    jetFecController=getJetFecHandle();
    ipPacketIdentifier = 0;
    memset(defaultGatewayIpAddress,0,32);
    jetFecInit();
#ifdef DEBUG
    printf("jetFec driver handle returned as 0x%x\n",jetFecController);
#endif
}



/*
 * vipSetDefaultGateway
 */
void
vipSetDefaultGateway(char *gwIpAddress)
{
    strcpy(defaultGatewayIpAddress,gwIpAddress);
    memset(defaultGatewayEther,0,6);
    vipGetEthernetAddress(defaultGatewayIpAddress,defaultGatewayEther);
#ifdef DEBUG
    printf("Default gateway set to %s with ethernet address %02x.%02x.%02x.%02x.%02x.%02x for ViperIp\n",
            defaultGatewayIpAddress,defaultGatewayEther[0],defaultGatewayEther[1],defaultGatewayEther[2],
            defaultGatewayEther[3],defaultGatewayEther[4],defaultGatewayEther[5]);
#endif
}



/*
 * vipGetEthernetAddress
 */
void
vipGetEthernetAddress(char *ipAddress, unsigned char *etherAddress)
{
    STATUS status;
    unsigned char targetAddress[10];
    int count;

    /*
     * If the remote address is not even on the same subnet
     * no point in 'arping about it...;-)
     */    
    if(vipIsPeerOnSameSubnet(ipAddress)!=TRUE) {
        for(count=0;count<6;count++)
            etherAddress[count] = defaultGatewayEther[count];
        return;
    }

    pInterface = ifunit(VIPERCELL_ETHERNET_INTERFACE);
    if (pInterface != NULL) {
        memset(targetAddress,0,10);
        status = etherAddrResolve(pInterface,ipAddress,(char *)targetAddress,2,20);
        if (status == OK){
#ifdef DEBUG
            printf("Ethernet address of %s returned as %02x.%02x.%02x.%02x.%02x.%02x\n",
                    ipAddress,
                    targetAddress[0],targetAddress[1],targetAddress[2],targetAddress[3],
                    targetAddress[4],targetAddress[5]);
#endif
            for(count=0;count<6;count++)
                etherAddress[count] = targetAddress[count];
        }
        else {
#ifdef DEBUG
            printf("Returning default gateway ethernet address for %s\n",ipAddress);
#endif
            for(count=0;count<6;count++)
            etherAddress[count] = defaultGatewayEther[count];
        }
    }
    return;
}



/*
 * vipIsPeerOnSameSubnet
 */
BOOL
vipIsPeerOnSameSubnet(char *peerAddress)
{
    BOOL retStatus=FALSE;
	static char InterfaceName[50],localAddress[20];
	int Status,netMask;
    struct in_addr nLocalAddress,nPeerAddress;
    unsigned long maskedRemote, maskedLocal;

	memset(InterfaceName,0,50);
	memset(localAddress,0,20);

	memset(&nLocalAddress,0,sizeof(struct in_addr));
	memset(&nPeerAddress,0,sizeof(struct in_addr));

    netMask=0;

	strcpy(InterfaceName,VIPERCELL_ETHERNET_INTERFACE);
	Status = ifMaskGet(InterfaceName,&netMask);
    if (Status == ERROR) {
        printf("Unable to get local netMask!!\n");
        return(retStatus);
    }

    strcpy(localAddress,vipGetLocalIpAddress());

    nLocalAddress.s_addr = inet_addr(localAddress);
    nPeerAddress.s_addr  = inet_addr(peerAddress);

    maskedLocal = nLocalAddress.s_addr & netMask;
    maskedRemote = nPeerAddress.s_addr & netMask;

    if (maskedLocal != maskedRemote) {
    }
    else {
        retStatus=TRUE;
    }

    return(retStatus);
}




/*
 * vipConstructNetHeaders
 */
void
vipConstructNetHeaders(unsigned long remoteIp,
                    unsigned short remotePort,
                    unsigned short localPort,
                    unsigned char *pktBuffer,
                    unsigned short *dataOffset
                    )
{
    char remoteIpString[20],localIpString[20];
    struct internetAddress remoteInAddr,localInAddr;
    int count;
    struct in_addr destAddress;
    struct ether_header *etherHeader;
    PVIPER_IP_HDR   ipHdr;
    PVIPER_UDP_HDR  udpHdr;
    unsigned char *offsetPtr;

    destAddress.s_addr = remoteIp;

    memset(remoteIpString,0,20);
    memset(localIpString,0,20);
    inet_ntoa_b(destAddress,remoteIpString);
    strcpy(localIpString,vipGetLocalIpAddress());

    etherHeader = (struct ether_header *)pktBuffer;
    offsetPtr = (unsigned char *)&pktBuffer[VIPER_IP_HDR_OFFSET];
    ipHdr = (PVIPER_IP_HDR)offsetPtr;
    offsetPtr = (unsigned char *)&pktBuffer[VIPER_IP_UDP_HDR_OFFSET];
    udpHdr = (PVIPER_UDP_HDR)offsetPtr;
    *dataOffset = (unsigned short)(VIPER_IP_UDP_HDR_OFFSET+VIPER_IP_UDP_HDR_SIZE);


    /* Validate the default gw's MAC everytime we setup a call : for DDTS CSCdt23589 */
    vipSetDefaultGateway(defaultGatewayIpAddress);


    /*
     * fill in the Ethernet header
     */
    vipGetEthernetAddress(remoteIpString,etherHeader->ether_dhost);
    vipGetEthernetAddress(localIpString,etherHeader->ether_shost);
    etherHeader->ether_type = VIPER_IP_PROTOCOL_TYPE;

    /*
     * fill in the UDP/IP header
     */
    ipHdr->version     = VIPER_IP_PROTOCOL_VERSION;
    ipHdr->hdrLength   = VIPER_IP_HDR_SIZE/4;  /* 5 = 20 bytes divided by 32 bit words */
    ipHdr->tos         = VIPER_IP_TYPE_OF_SERVICE;
    ipHdr->flags       = 0;
    ipHdr->fragOffset  = 0;
    ipHdr->ttl         = VIPER_IP_DEFAULT_TTL;
    ipHdr->protocol    = VIPER_IP_PROTOCOL_TYPE_UDP;

    memset(&remoteInAddr,0,sizeof(remoteInAddr));
    memset(&localInAddr,0,sizeof(localInAddr));
    remoteInAddr.addr.s_addr    = remoteIp;
    localInAddr.addr.s_addr     = inet_addr(localIpString);

    memcpy(ipHdr->sourceAddress,localInAddr.addr.s_bytes,4);
    memcpy(ipHdr->destAddress,remoteInAddr.addr.s_bytes,4);

    ipHdr->checksum     = 0;

    udpHdr->sourcePort  = htons(localPort);
    udpHdr->destPort    = htons(remotePort);
    udpHdr->udpChecksum = 0;
}


/*
 * vipSend
 */
void
vipSend(unsigned char *pktBuffer,int dataLength)
{
    PVIPER_IP_HDR   ipHdr;
    PVIPER_UDP_HDR  udpHdr;
    unsigned char *offsetPtr;
    unsigned short  totalEtherPktLength=0;
    offsetPtr = (unsigned char *)&pktBuffer[VIPER_IP_HDR_OFFSET];
    ipHdr = (PVIPER_IP_HDR)offsetPtr;
    offsetPtr = (unsigned char *)&pktBuffer[VIPER_IP_UDP_HDR_OFFSET];
    udpHdr = (PVIPER_UDP_HDR)offsetPtr;

    udpHdr->udpLength   = dataLength+8;
    ipHdr->id           = htons(ipPacketIdentifier++);
    ipHdr->totalLength  = htons(VIPER_IP_HDR_SIZE+VIPER_IP_UDP_HDR_SIZE+dataLength);
    totalEtherPktLength = ipHdr->totalLength + VIPER_IP_HDR_OFFSET;
    ipHdr->checksum     = 0;
    ipHdr->checksum     = vipComputeIpCksum((unsigned short *)ipHdr,VIPER_IP_HDR_SIZE);
    jetFecSend(jetFecController,pktBuffer,totalEtherPktLength);
}




/*
 * vipGetLocalIpAddress
 */
char *
vipGetLocalIpAddress()
{
	static char DirtyBuffer[50],InterfaceName[50];
	int Status;

	memset(DirtyBuffer,0,50);
	memset(InterfaceName,0,50);

	strcpy(InterfaceName,VIPERCELL_ETHERNET_INTERFACE);
	Status = ifAddrGet(InterfaceName,DirtyBuffer);
	if (Status == ERROR)
	{
		printf("Error retrieving interface %s's IP address!\n",InterfaceName);
		return "";
	}
	return(DirtyBuffer);
	
}



/*
 * vipComputeIpCksum
 */
unsigned short 
vipComputeIpCksum(unsigned short *addr,int len)
{
    register int nleft = len;
    register unsigned short *w = addr;
    register int sum = 0;
    unsigned short answer = 0;
        
    while (nleft > 1) { 
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(u_char *)(&answer) = *(u_char *)w ;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}


void
vipAddRtpPort(unsigned short rtpPort, SEM_ID rtpSemId,unsigned char *recvBuffPtr)
{
    jetFecAddRtpPort(rtpPort,rtpSemId,recvBuffPtr);
}


int
vipShowDefaultGateway()
{
    printf("Default Gateway IP address is set as %s\n",defaultGatewayIpAddress);
    return (0);
}


/*---------------------------------- TEST CODE SECTION BEGIN -------------------------------------------*/

#ifdef DEBUG

#define MAX_VIPER_IP_TEST_DATA_SIZE        60

unsigned char   testPktBuffer[2000];
unsigned char   testRecvPktBuffer[2000];
unsigned short  testDataOffset;
int             testInterPktDelay;
unsigned long   testTotalPkts,recvPktCount;
SEM_ID          testRecvSem;


void VipRecvProc(void);
void VipShowRecv(void);



/*
 * StartVipTest
 */
void
StartVipTest(int pkts,int ticks, char *destAddr)
{
    struct in_addr remoteAddr;

    testInterPktDelay   = ticks;
    testTotalPkts       = pkts;
    recvPktCount=0;
    
    vipInit();

	printf("Ip Test will send %ld packets to IP %s :UDP port 2200 with inter packet delay of %d ticks \n", 
	    testTotalPkts,destAddr,testInterPktDelay);


    remoteAddr.s_addr = inet_addr(destAddr);

    memset(testPktBuffer,0,2000);

    testDataOffset = 0;

    vipConstructNetHeaders(remoteAddr.s_addr,
                        2200,
                        2100,
                        testPktBuffer,
                        &testDataOffset);

    printf("RTP Data offset specified as %d\n",testDataOffset);
    testPktBuffer[testDataOffset] = 0x80;
    memset(&testPktBuffer[testDataOffset+1],'A',MAX_VIPER_IP_TEST_DATA_SIZE);

    if(taskSpawn("VipTask", 
                 10, 
                 0, 
                 20480,
                 (FUNCPTR) VipTesterProc,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
    {

        printf("Error:Could not spawn VipTesterProc\n");

	} 
    testRecvSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    if (testRecvSem==NULL) {
        printf("Could not create receiver semaphore\n");
        return;
    }
    vipAddRtpPort(2200,testRecvSem,testRecvPktBuffer);
    if(taskSpawn("VipRecv", 
                 10, 
                 0, 
                 20480,
                 (FUNCPTR)VipRecvProc,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
    {

        printf("Error:Could not spawn VipTesterProc\n");

	} 

}


void
VipTesterProc()
{
    unsigned long count=0;
    unsigned char *testRtpType;

    testRtpType = (unsigned char *)testPktBuffer[42];
    *testRtpType = 0x80;
    while(1) {
        count++;
        if (count == testTotalPkts) {
            printf("Completed sending %ld packets\n",testTotalPkts);
            return;
        }
        vipSend(testPktBuffer,45);
        taskDelay(1);
    }
}

void
VipRecvProc()
{
    unsigned long count=0;
    while(1) {
        count++;
        if (count == testTotalPkts) {
            printf("Completed receiving %ld packets\n",testTotalPkts);
            return;
        }
        if (semTake(testRecvSem, WAIT_FOREVER) != ERROR) {
            recvPktCount++;
        }
    }
}

void
VipShowRecv()
{
    printf("Received %ld packets so far\n",recvPktCount);
}

#endif /* #ifdef DEBUG */
/*---------------------------------- TEST CODE SECTION END ----------------------------------------------*/
