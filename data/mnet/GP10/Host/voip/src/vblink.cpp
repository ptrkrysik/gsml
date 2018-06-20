
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: vblink.cpp													*
 *																						*
 *	Description			: Socket interface for proprietary msging between VC & VB       *
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
#include <stdio.h>
#include <stdlib.h>
#include <vxworks.h>
#include <socklib.h>
#include <tasklib.h>
#include <sysLib.h>
#include <string.h>
#include <time.h>

#include "Os/JCTask.h"
#include "Os/JCModule.h"
#include "GP10OsTune.h"
#include "logging/vcmodules.h"
#include "logging/vclogging.h"
#include "voip/vblink.h"
#include "voip/mncalxfr.h"
#include "Jcc/LudbApi.h"
#include "oam_api.h"
#include "AlarmCode.h"
#include "MnetModuleId.h"
#include "util.h"

// ext-HO <chenj:05-15-01>
#include "Jcc/JCCEHoMsg.h"

// Instead of including JcVcToVc.h, just define the types that
// are needed in this module.  For the long term, must solve
// this cyclical include problem
// #include "Jcc/JcVcToVc.h"

#include "voip/exchangeho.h"


VBLINK_CONFIG    VBLinkConfig;
JCTask           VBLinkTaskObj("VBLink");
static int       KeepAliveInterval=20,MaxKeepAliveTxCount=3;


// ext-HO <chenj:06-15-01>
extern MSG_Q_ID ccMsgQId;

// MGPLA <chenj:08-07-01>
extern MSG_Q_ID mmMsgQId;


/*
 * SysCommand_VBLink
 */

int	SysCommand_VBLink(T_SYS_CMD	action)
{
	int tid;

	switch(action){
		case SYS_SHUTDOWN:
			printf("[VBLink] Received system shutdown notification\n");
			break;
		case SYS_START:
			printf("[VBLink] Received task start notification\n");
			if(!InitializeVBLink())
			printf("VBLink initialization failed\n");
			break;
		case SYS_REBOOT:
			printf("[VBLink] Reboot ready.\n");
			break;
		default:
			printf("[VBLink] Unknown system command received\n");
	}
	return 0;
}

/*
 * InitializeVBLink
 */
BOOL
InitializeVBLink()
{
    DBG_FUNC("InitializeVBLink",VBLINK_LAYER);
    DBG_ENTER();
    BOOL retStatus = FALSE;
    unsigned long GKIPAddress;
    unsigned char gkIpAddrOctets[4];

    VBLinkSetOAMTrap(VBLINK_VIPERBASE_LINK_NOT_UP);

    memset(&VBLinkConfig,0,sizeof(VBLinkConfig));

    VBLinkConfig.ConnectedToViperBase = FALSE;
    VBLinkConfig.RetryConnects = TRUE;

	if (oam_getMibIntVar(MIB_h323_GKIPAddress, (long int*)&GKIPAddress) != STATUS_OK)
	{
		DBG_ERROR("Unable to get GateKeeper IP address from OAM!!\n");
		return retStatus;
	}
    else {
        DBG_TRACE("GateKeeper IP address OAM value %ld\n",GKIPAddress);
    }

    gkIpAddrOctets[0] = (GKIPAddress >> 24) & 0x000000ff;
    gkIpAddrOctets[1] = (GKIPAddress >> 16) & 0x000000ff;
    gkIpAddrOctets[2] = (GKIPAddress >> 8) & 0x000000ff;
    gkIpAddrOctets[3] = (GKIPAddress) & 0x000000ff;

    memset(VBLinkConfig.szViperBaseAddress,0,VBLINK_MAX_IP_ADDR_STR_LEN);
    sprintf(VBLinkConfig.szViperBaseAddress,"%u.%u.%u.%u",gkIpAddrOctets[0],gkIpAddrOctets[1],gkIpAddrOctets[2],gkIpAddrOctets[3]);


    if (!VBLinkPrepareSocket()) {
        DBG_ERROR("Unable to connect to GMC, cannot proceed!!\n");
        DBG_LEAVE();
        return(retStatus);
    }
    if (VBLinkTaskObj.JCTaskSpawn(VBLINK_TASK_PRIORITY, VBLINK_TASK_OPTION, VBLINK_TASK_STACK_SIZE,
			(FUNCPTR) VBLink, 0,0,0,0, 0, 0, 0, 0, 0, 0,MODULE_VBLINK,JC_CRITICAL_TASK) == ERROR){  
	    DBG_ERROR("Unable to spawn VBLink task!!\n");
        close(VBLinkConfig.VBSocket);
    }
    else {
        DBG_TRACE("Spawned VBLink Task successfully!\n");
        retStatus=TRUE;
	}

    DBG_LEAVE();
    return(retStatus);
}



/*
 * VBLinkPrepareSocket
 */
BOOL
VBLinkPrepareSocket()
{
    DBG_FUNC("VBLinkPrepareSocket",VBLINK_LAYER);
    DBG_ENTER();

    struct sockaddr_in  localAddress;
    int AddrSize,KeepAliveOn= 1;
    BOOL retStatus=FALSE;

    VBLinkConfig.VBSocket = socket(AF_INET,SOCK_STREAM,0);
    
    if (VBLinkConfig.VBSocket == ERROR) {
        DBG_ERROR("Unable to allocate socket for connecting to GMC!!\n");
        perror("VBLink Socket");
        DBG_LEAVE();
        return(retStatus);
    }

  	setsockopt(VBLinkConfig.VBSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&KeepAliveOn, sizeof(KeepAliveOn));

    AddrSize = sizeof(localAddress);
    memset(&localAddress,0,sizeof(localAddress));

    localAddress.sin_family         = AF_INET;
    localAddress.sin_addr.s_addr    = INADDR_ANY;
    localAddress.sin_port           = 0;

    if (bind(VBLinkConfig.VBSocket,(struct sockaddr *)&localAddress,AddrSize)==ERROR) {
        DBG_ERROR("Unable to bind VBLink socket\n");
        perror("VBApi Bind");
        close(VBLinkConfig.VBSocket);
        DBG_LEAVE();
        return(retStatus);
    }

    memset(&VBLinkConfig.ViperBaseAddress,0,sizeof(VBLinkConfig.ViperBaseAddress));

    VBLinkConfig.ViperBaseAddress.sin_family         = AF_INET;
    VBLinkConfig.ViperBaseAddress.sin_addr.s_addr    = inet_addr(VBLinkConfig.szViperBaseAddress);
    VBLinkConfig.ViperBaseAddress.sin_port           = htons(VBLINK_PORT);

    /* Need to read these values from somewhere */
    VBLinkConfig.LastKeepAliveTimestamp = 0;
    VBLinkConfig.KeepAliveInterval      = KeepAliveInterval;
    VBLinkConfig.MaxKeepAliveTxCount    = MaxKeepAliveTxCount;
    VBLinkConfig.KeepAliveTxCount       = 0;


    retStatus = TRUE;
    DBG_LEAVE();
    return(retStatus);
}


/*
 * VBLinkStatus
 */
BOOL 
VBLinkStatus()
{
    DBG_FUNC("VBLinkStatus",VBLINK_LAYER);
    DBG_ENTER();
    DBG_LEAVE();
    return(VBLinkConfig.ConnectedToViperBase);
}






/*
 * VBLinkGetInterval
 */
BOOL
VBLinkGetInterval(struct timeval *WaitTime)
{
	DBG_FUNC("VBLinkGetInterval",VBLINK_LAYER);
    DBG_ENTER();
    time_t TimeNow;
    unsigned long delta;

    WaitTime->tv_usec = 0;
    /*
     * First check the time now and see if you need
     * to send the keepalive now
     */
    time(&TimeNow);
    
    if (TimeNow >= (VBLinkConfig.LastKeepAliveTimestamp+VBLinkConfig.KeepAliveInterval)) {
        /*
         * Yes, we should send one immediately
         */
        if (!VBLinkSendKeepAlive()) {
            DBG_LEAVE();
            return (FALSE);
        }
        VBLinkConfig.LastKeepAliveTimestamp = TimeNow;
        if (VBLinkConfig.KeepAliveTxCount >= VBLinkConfig.MaxKeepAliveTxCount) {
            /*
             * Looks like we sent the max permitted without getting
             * an ack back, so we should declare the GMC as dead ;-)
             */
             DBG_ERROR("Timed out on VBLINK Application Keep Alives with GMC\n",
                VBLinkConfig.KeepAliveTxCount,VBLinkConfig.MaxKeepAliveTxCount);
             DBG_LEAVE();
             return (FALSE);
        }
        DBG_TRACE("Last Timestamp %#x\n",VBLinkConfig.LastKeepAliveTimestamp);
        WaitTime->tv_sec  = VBLinkConfig.KeepAliveInterval;
        DBG_LEAVE();
        return(TRUE);
    }


    delta  = (VBLinkConfig.LastKeepAliveTimestamp + VBLinkConfig.KeepAliveInterval) - TimeNow;

    if (delta > VBLinkConfig.KeepAliveInterval) {
        DBG_ERROR("Error in VBLink Keepalives, last keep alive at %#x, time now %#x, delta %#x, configured interval %#xu\n",
            VBLinkConfig.LastKeepAliveTimestamp,TimeNow,delta,VBLinkConfig.KeepAliveInterval);
        WaitTime->tv_sec  = VBLinkConfig.KeepAliveInterval;
    }
    else {
        DBG_TRACE("KeepAlive Delta %#x seconds\n",delta);
        WaitTime->tv_sec  = delta;
    }

    DBG_LEAVE();
    return(TRUE);
}




/*
 * VBLinkSendKeepAlive
 */
BOOL
VBLinkSendKeepAlive()
{
    DBG_FUNC("VBLinkSendKeepAlive",VBLINK_LAYER);
    DBG_ENTER();
    time_t ltime;
    int nSize;
    UCHAR buffer[NON_STD_MSG_MAX_LEN];
    UCHAR* pszBuffer;
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    STATUS status;
    BOOL ret_status = FALSE;

    /* Get UNIX-style time and display as number and string. */
    time(&ltime);
    DBG_TRACE("Sending KeepAlive to GMC at %s: %#x UTC\n",ctime(&ltime),ltime);


    /* packing and sending echo back */
    pszBuffer = buffer;
    nSize = 0;
    jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
    jcUShortPack( NonStdRasMessageTypeKeepAlive, &pszBuffer, &nSize );

    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *)sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

    status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
    if (status == ERROR) {
        DBG_ERROR("Unable to send KeepAlive to GMC, %d!\n",errno);
    }
    else {
        ret_status = TRUE;
        VBLinkConfig.KeepAliveTxCount++;
        DBG_TRACE("Sent KeepAlive, UnAcked KeepAlive count %d\n",VBLinkConfig.KeepAliveTxCount);
    }

    DBG_LEAVE();
    return(ret_status);
}











/*
 * VBLinkShowStatus
 */
int
VBLinkShowStatus()
{
    DBG_FUNC("VBLinkShowStatus",VBLINK_LAYER);
    DBG_ENTER();
    
    printf("VBLink status = ");
    if (VBLinkConfig.ConnectedToViperBase) {
        printf("CONNECTED\n");
    }
    else {
        printf("DISCONNECTED\n");
    }

    DBG_LEAVE();
    return(0);
}


/*
 * VBLinkSetRetryConnect
 */
int
VBLinkSetRetryConnect(int a)
{
    if (a>0) {
        VBLinkConfig.RetryConnects = TRUE;
        printf("Will keep on retrying connecting to GMC forever\n");
    }
    else {
        VBLinkConfig.RetryConnects = FALSE;
        printf("Will no longer retry connecting to GMC\n");
    }
    return(0);
}


/*
 * VBLink
 */
void
VBLink()
{
    DBG_FUNC("VBLink",VBLINK_LAYER);
    DBG_ENTER();
    STATUS status;
    BOOL retStatus,WaitingForFragment;
    char recvBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short totalLen, reqdSize, *actualPktLen,expectedLen;
	struct timeval WaitTime;

    VBLinkTaskObj.JCTaskEnterLoop();

    if (!VBLinkConfig.ConnectedToViperBase) {
        if (!VBLinkReconnectLoop()) {
            close(VBLinkConfig.VBSocket);
            return;
        }
    }

    totalLen            = 0;
    reqdSize            = VBLINK_MAX_MESSAGE_SIZE;
    WaitingForFragment  = FALSE;

	VBLinkGetInterval(&WaitTime);

      while(1) {

	    FD_ZERO(&VBLinkConfig.ReadSet);
	    FD_SET(VBLinkConfig.VBSocket,&VBLinkConfig.ReadSet);
	    status = select(FD_SETSIZE,&VBLinkConfig.ReadSet,0,0,&WaitTime);

        if (status == 0) {
            if (!VBLinkGetInterval(&WaitTime)) {
                /* Looks like the connection has timed out */
                DBG_ERROR("KeepAlive timed out, connection to GMC broken!!!\n");
                VBLinkConfig.ConnectedToViperBase = FALSE;
                VBLinkSetOAMTrap(VBLINK_VIPERBASE_LINK_LOST);
        		close(VBLinkConfig.VBSocket);
                FD_CLR(VBLinkConfig.VBSocket,&VBLinkConfig.ReadSet);
                if (!VBLinkPrepareSocket()) {
                    printf("Exiting VBLink application\n");
                    DBG_LEAVE();
                    return;
                }
                if (!VBLinkReconnectLoop()) {													 
                    printf("Exiting VBLink application\n");
                    DBG_LEAVE();
                    return;
                }
            }

        }
        else {
            status = recv(VBLinkConfig.VBSocket,(char *)&recvBuffer[totalLen],reqdSize,0);
            if ((status ==  ERROR) || (status == 0)){
                    /* Looks like the connection was reset */
                    DBG_ERROR("Connection to GMC broken!!!\n");
                    VBLinkConfig.ConnectedToViperBase = FALSE;
                    VBLinkSetOAMTrap(VBLINK_VIPERBASE_LINK_LOST);
        		    close(VBLinkConfig.VBSocket);
                    FD_CLR(VBLinkConfig.VBSocket,&VBLinkConfig.ReadSet);
                    if (!VBLinkPrepareSocket()) {
                        printf("Exiting VBLink application\n");
                        DBG_LEAVE();
                        return;
                    }
                    if (!VBLinkReconnectLoop()) {													 
                        printf("Exiting VBLink application\n");
                        DBG_LEAVE();
                        return;
                    }
            }
            else { 
                /* successful reception of data */
                totalLen += status;
                if (!WaitingForFragment) {
                    /* beginning of request msg, get the expected length */
                    actualPktLen = (unsigned short *)recvBuffer;
                    expectedLen = ntohs(*actualPktLen)+2;
                }
                DBG_TRACE("Received msg of %d bytes, expected msg size %d bytes\n",totalLen,expectedLen);
                reevaluate:
                if (totalLen == expectedLen) {
                    /* Looks like we have received the complete packet */
                    DBG_TRACE("Received complete message of size %d bytes\n",expectedLen);
                    retStatus=ProcessViperBaseMessage((unsigned char *)&recvBuffer[sizeof(short)],totalLen);
                    totalLen = 0;
                    reqdSize = VBLINK_MAX_MESSAGE_SIZE;
                    WaitingForFragment = FALSE;
                    memset(recvBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
                }
                else if (totalLen > expectedLen) {
                    DBG_TRACE("Received multiple msgs in packet, recvd size %d expected size %d\n",totalLen,expectedLen);
                    char scrapBuffer[VBLINK_MAX_MESSAGE_SIZE];
                    memcpy(scrapBuffer,recvBuffer,expectedLen);
                    retStatus=ProcessViperBaseMessage((unsigned char *)&scrapBuffer[sizeof(short)],expectedLen);
                    totalLen = totalLen - expectedLen;
                    DBG_TRACE("Second fragment size %d\n",totalLen);
                    memcpy(scrapBuffer,&recvBuffer[expectedLen],totalLen);
                    memset(recvBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
                    memcpy(recvBuffer,scrapBuffer,totalLen);
                    actualPktLen = (unsigned short *)recvBuffer;
                    expectedLen = ntohs(*actualPktLen)+2;
                    goto reevaluate;
                }
                else {
                    /* Received only a fragment, wait for the rest */
                    WaitingForFragment = TRUE;
                    reqdSize = expectedLen - totalLen;
                }

            }
        }
    }
    VBLinkTaskObj.JCTaskNormExit();
    DBG_LEAVE();
}






/*
 * ProcessViperBaseMessage
 */
BOOL 
ProcessViperBaseMessage(unsigned char *buffer,int nDataSize)
{
    DBG_FUNC("ProcessViperBaseMessage",VBLINK_LAYER);
    DBG_ENTER();

    UCHAR* pszBuffer;
    NonStdRasMessageType Msg;
    unsigned short nonStdMsgType;
	MOBILE_ID MobileId;
    LUDB_ID LudbId;
    int nonStdProtocolValue;
    DBG_TRACE("GMC message %d bytes long\n",nDataSize);

    pszBuffer = buffer;

    nonStdProtocolValue = jcUShortUnpack( &pszBuffer );
    if (nonStdProtocolValue != NON_STD_RAS_PROTOCOL) {
        DBG_ERROR("Non std protocol not specified in GMC message (buffer value=%d)!!!\n", nonStdProtocolValue);
        DBG_TRACE("       \nbuffer hexdump: ");
        DBG_HEXDUMP((unsigned char*) buffer, nDataSize );
        DBG_TRACE("\n");
    }
    nonStdMsgType = jcUShortUnpack( &pszBuffer );


    switch(nonStdMsgType) /* message type */
    {
        case NonStdRasMessageTypeKeepAlive: 
        {
            VBLinkConfig.KeepAliveTxCount--;
            DBG_TRACE("Received KeepAlive ACK from GMC; UnAcked KeepAlive count %d\n",VBLinkConfig.KeepAliveTxCount);
            break;
        }
        

        case NonStdRasMessageTypeRCF:
        {
            NonStdRasMessageRCF_t MsgRCF;

            jcNonStdRasMessageRCFUnpack( &MsgRCF, &pszBuffer );
			jcStringToBCD(MsgRCF.pszMobileID, MobileId.digits, &MobileId.numDigits);

            DBG_TRACE("RCF received for %s\n",MsgRCF.pszMobileID);

            SendVBLinkRegEvent(VBLINK_MOBILE_REG_EVENT_RECV_RCF, MobileId);

            break;
        }

        case NonStdRasMessageTypeRRJ:
        {
            NonStdRasMessageRRJ_t MsgRRJ;

            jcNonStdRasMessageRRJUnpack( &MsgRRJ, &pszBuffer );
			jcStringToBCD(MsgRRJ.pszMobileID, MobileId.digits, &MobileId.numDigits);

            DBG_TRACE("RRJ received for %s\n", MsgRRJ.pszMobileID);

            SendVBLinkRegEvent(VBLINK_MOBILE_REG_EVENT_RECV_RRJ, MobileId);


            break;
        }

        case NonStdRasMessageTypeUCF:
        {
            NonStdRasMessageUCF_t MsgUCF;

            jcNonStdRasMessageUCFUnpack( &MsgUCF, &pszBuffer );
			jcStringToBCD(MsgUCF.pszMobileID, MobileId.digits, &MobileId.numDigits);

            DBG_TRACE("UCF received for %s\n", MsgUCF.pszMobileID);

            SendVBLinkRegEvent(VBLINK_MOBILE_REG_EVENT_RECV_UCF, MobileId);

            break;
        }

        case NonStdRasMessageTypeURQ:
        {
            NonStdRasMessageURQ_t MsgURQ;

            jcNonStdRasMessageURQUnpack( &MsgURQ, &pszBuffer );
			jcStringToBCD(MsgURQ.pszMobileID, MobileId.digits, &MobileId.numDigits);

            DBG_TRACE("URQ received for %s\n", MsgURQ.pszMobileID);

            SendVBLinkRegEvent(VBLINK_MOBILE_REG_EVENT_RECV_URQ, MobileId);

            break;
        }

        case NonStdRasMessageTypeCellLRJ:
	    {
		    NonStdRasMessageCellLRJ_t CellLRJ;

		    jcNonStdRasMessageCellLRJUnpack(&CellLRJ, &pszBuffer);

		    DBG_TRACE("Could not get IP for Cell id %s, Location Request rejected!!\n",CellLRJ.pszCellID);

            // PR CSCdv18197 <chenj:08-27-01>
            // Send a string "0" instead of NULL for rejected case
		    SendIpAddressResponse(CellLRJ.pszCellID,"0\n",CellLRJ.hCell);

		    break;
	    }
	    case NonStdRasMessageTypeCellLCF:
        {
		    NonStdRasMessageCellLCF_t CellLCF;

		    jcNonStdRasMessageCellLCFUnpack( &CellLCF, &pszBuffer);

		    DBG_TRACE("Received IP %s for Cell Id %s\n",CellLCF.pszIpAddress,CellLCF.pszCellID);

		    SendIpAddressResponse(CellLCF.pszCellID,CellLCF.pszIpAddress,CellLCF.hCell);

            break;
        }

        case NonStdRasMessageTypeMobileProfile:
        {
            NonStdRasMessageMobileProfile_t MobileProfile;
            BOOL ludbProfileUpdateStatus;

            jcNonStdRasMessageMobileProfileUnpack(&MobileProfile, &pszBuffer);
			jcStringToBCD(MobileProfile.pszMobileID, MobileId.digits, &MobileId.numDigits);

            DBG_TRACE("Mobile profile received for %s\n", MobileProfile.pszMobileID);

      	    ludbProfileUpdateStatus=ludbProfileUpdate(MobileId, MobileProfile.nProfileSize, (char *)MobileProfile.pProfileData);
            if (!ludbProfileUpdateStatus) {
                 DBG_ERROR("LUDB returned error for Profile Update!!\n");
            }
            else {
                DBG_TRACE("Updated LUDB with Mobile Profile successfully\n");
            }
            break;
        }

		/* CISS message Network -> Mobile Station */
		case NonStdRasMessageTypeNetToMsCISS:
        {
            NonStdRasMessageCISS_t cissMsg;
			bool status;

            jcNonStdRasMessageMobileCISSUnpack(&cissMsg, &pszBuffer);
			jcStringToBCD(cissMsg.pszMobileID, MobileId.digits, &MobileId.numDigits);

            LudbId = ludbGetEntryByMobileId(MobileId); 

            DBG_TRACE("Network to Mobile message (ludb ID = %d)\n", LudbId);

      	    status = ccCissProcessViperBaseMsg(LudbId, (char *) cissMsg.pMsgData, cissMsg.nMsgSize);
            if (status) {
                 DBG_TRACE("CISS Mobile message was sent  successfully\n");
            } else {
                DBG_ERROR("Sending CISS mobile message failure\n");
            }
            break;
        }

		/* SMSPP message SMR -> SMC at network side */
		case NonStdRasMessageTypeNetToMsSMSPP:
        {
            NonStdRasMessageSMSPP_t smsppNonStdRasMsg;
			bool status;

            jcNonStdRasMessageMobileSmsppUnpack(&smsppNonStdRasMsg, &pszBuffer);
			jcStringToBCD(smsppNonStdRasMsg.pszMobileID, MobileId.digits, &MobileId.numDigits);

            LudbId = ludbGetEntryByMobileId(MobileId); 

            DBG_TRACE("SMSPP messgage from SMR to SMC at network side (ludb ID = %d)\n", LudbId);

      	    status = smsReceiveSmrMsg(LudbId, (unsigned char *) smsppNonStdRasMsg.pMsgData, smsppNonStdRasMsg.nMsgSize);
            if (status) {
                 DBG_TRACE("SMSPP SMR->SMC message was sent  successfully\n");
            } else {
                DBG_ERROR("SMSPP SMR->SMC message delivery failure\n");
            }
            break;
        }


        case NonStdRasMessageTypeCellSynchronize:
        {
            NonStdRasMessageCellSynchronize_t CellSync;
            BOOL Status=FALSE;
 	        VOIP_API_MESSAGE	ApiMsg;

            jcNonStdRasMessageCellSynchronizeUnpack(&CellSync,&pszBuffer);
            if (CellSync.nMode == jcCellDown) {
                DBG_TRACE("ViperCell with cell id %s has gone down!!\n",CellSync.pszCellID);
 	            Status=CcVBLinkCellStatusUpdate(CellSync.pszCellID,VC_STATUS_DOWN);
                if (!Status) {
                    DBG_ERROR("CC returned error for ViperCell %s down msg!!\n",CellSync.pszCellID);
                }
            }
            else {
                DBG_ERROR("VBLink :: Unsupported case!!!\n");
            }
        }
            break;


        case NonStdRasMessageTypeSRS:
		{

			NonStdRasMessageSRS_t MobileSRS;

			jcNonStdRasMessageSRSUnpack(&MobileSRS,&pszBuffer);
			jcStringToBCD(MobileSRS.pszMobileID, MobileId.digits, &MobileId.numDigits);

			ludbSRSUpdate(MobileId, MobileSRS.nAuthSets, MobileSRS.pAuthSets);
		}
		break;

		case NonStdRasMessageTypeTransferConfirm:
		{
			MnetRasTransferConfirm_t transferConfirm;
			bool status;
			VOIP_CALL_DESTINATION_ADDRESS	CalledParty;
			char destNumBuffer[256];

			memset(&CalledParty,0,sizeof(VOIP_CALL_DESTINATION_ADDRESS));

			jcNonStdRasMessageTransferConfirmUnpack(&transferConfirm,&pszBuffer );

  			VBLinkGetGsmNumberType(&CalledParty.numberType,transferConfirm.destinationAliasAddress);

			jcStringToBCD(transferConfirm.destinationAliasAddress.numberDigits,
						CalledParty.digits,&CalledParty.numDigits);

			DBG_TRACE("Transfer request confirmation received for dest %s, seq num %d\n",
						  transferConfirm.destinationAliasAddress.numberDigits,
						  transferConfirm.requestSeqNum);

			status = CcVBLinkTransferResponse(transferConfirm.requestSeqNum,0,CalledParty);
			if (!status) {
				DBG_ERROR("CC returns error for Transfer confirmation from GMC\n");
			}

		}
		break;

		case NonStdRasMessageTypeTransferReject:
		{
			MnetRasTransferReject_t transferReject;
			bool status;
			VOIP_CALL_DESTINATION_ADDRESS	CalledParty;

			memset(&CalledParty,0,sizeof(VOIP_CALL_DESTINATION_ADDRESS));

			jcNonStdRasMessageTransferRejecttUnpack(&transferReject,&pszBuffer );

			DBG_TRACE("Transfer request rejected for request seq num %d, cause %d\n",
						  transferReject.requestSeqNum,
						  transferReject.rejectCause);

			status = CcVBLinkTransferResponse(transferReject.requestSeqNum,transferReject.rejectCause,CalledParty);
			if (!status) {
				DBG_ERROR("CC returns error for Transfer confirmation from GMC\n");
			}
		}
		break;

        // ext-HO <chenj:05-15-01>
        case NonStdRasMessageTypePerformHandoverRQ:
		{
           NonStdRasMessagePerformHandoverRQ_t MsgPerformHandoverRQ;
           InterHoaVcMsg_t                     MsgtoCC;
           bool status;

           jcNonStdRasMessagePerformHandoverRQUnpack( &MsgPerformHandoverRQ, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): PerformHandoverRQ received.\n}\n");

           copyGenericToGP10Header( &MsgPerformHandoverRQ.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_PERFORM_HANDOVER_REQUEST;
           MsgtoCC.msgData.perfExtHandoverReq.globalCellID       = MsgPerformHandoverRQ.globalCellID;
           MsgtoCC.msgData.perfExtHandoverReq.hoCause            = MsgPerformHandoverRQ.hoCause;
           MsgtoCC.msgData.perfExtHandoverReq.hoNumberReqd       = MsgPerformHandoverRQ.hoNumberReqd;
           MsgtoCC.msgData.perfExtHandoverReq.handoverNumber     = MsgPerformHandoverRQ.handoverNumber;
           MsgtoCC.msgData.perfExtHandoverReq.mscNumber          = MsgPerformHandoverRQ.mscNumber;
           MsgtoCC.msgData.perfExtHandoverReq.A_HANDOVER_REQUEST = MsgPerformHandoverRQ.A_HANDOVER_REQUEST;


           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPerformHandoverRequest\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.perfExtHandoverReq hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.perfExtHandoverReq, sizeof (InterHoaVcPerformHandover_t) );
           DBG_TRACE("\n      }\n");

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPerformHandoverRequest\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.perfExtHandoverReq.A_HANDOVER_REQUEST hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.perfExtHandoverReq.A_HANDOVER_REQUEST, sizeof (A_INTERFACE_MSG_STRUCT_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Perform Handover Request!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypeEndHandover:
		{
           NonStdRasMessageEndHandover_t MsgEndHandover;
           InterHoaVcMsg_t               MsgtoCC;
           bool status;

           jcNonStdRasMessageEndHandoverUnpack( &MsgEndHandover, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): EndHandover received.\n}\n");  

           copyGenericToGP10Header( &MsgEndHandover.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_END_HANDOVER;
           MsgtoCC.msgData.extEndHandover.reason = MsgEndHandover.reason;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkEndHandover\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.extEndHandover hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.extEndHandover, sizeof (InterHoaVcEndHandover_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for End Handover!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypePostHandoverMobEvent:
		{
           NonStdRasMessagePostHandoverMobEvent_t MsgMobEvent;
           InterHoaVcMsg_t               MsgtoCC;
           bool status;

           jcNonStdRasMessagePostHandoverMobEventUnpack( &MsgMobEvent, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): PostHandoverMobEvent received.\n}\n");  

           copyGenericToGP10Header( &MsgMobEvent.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_HANDOVER_MOB_EVENT;
           MsgtoCC.msgData.postExtHoMobEventData.LAYER3PDU = MsgMobEvent.LAYER3PDU;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPostHandoverMobEvent\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.postExtHoMobEventData hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.postExtHoMobEventData, sizeof (InterHoaVcPostHoMobEventData_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Post Handover Mob Event!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypePerformHandoverAck:
		{
           NonStdRasMessagePerformHandoverAck_t MsgHandoverAck;
           InterHoaVcMsg_t                      MsgtoCC;
           bool status;

           jcNonStdRasMessagePerformHandoverAckUnpack( &MsgHandoverAck, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): Handover Ack received.\n}\n");  

           copyGenericToGP10Header( &MsgHandoverAck.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_PERFORM_HANDOVER_ACK;
           MsgtoCC.msgData.perfExtHandoverAck.handoverNumber  = MsgHandoverAck.handoverNumber;
           MsgtoCC.msgData.perfExtHandoverAck.reason          = MsgHandoverAck.reason;
           MsgtoCC.msgData.perfExtHandoverAck.A_INTERFACE_MSG = MsgHandoverAck.A_INTERFACE_MSG;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPerformHandoverAck\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.perfExtHandoverAck hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.perfExtHandoverAck, sizeof (InterHoaVcPerformHandoverAck_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Handover Ack!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypeHandoverAccessMsg:
		{
           NonStdRasMessageHandoverAccess_t MsgHandoverAccess;
           InterHoaVcMsg_t                  MsgtoCC;
           bool status;

           jcNonStdRasMessageHandoverAccessUnpack( &MsgHandoverAccess, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): Handover Access received.\n}\n");  

           copyGenericToGP10Header( &MsgHandoverAccess.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_HANDOVER_ACCESS;
           MsgtoCC.msgData.extHandoverAccess.A_INTERFACE_MSG = MsgHandoverAccess.A_INTERFACE_MSG;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkHandoverAccess\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.extHandoverAccess hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.extHandoverAccess, sizeof (InterHoaVcHandoverAccess_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Handover Access!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypeHandoverSuccessMsg:
		{
           NonStdRasMessageHandoverSuccess_t MsgHandoverSuccess;
           InterHoaVcMsg_t                      MsgtoCC;
           bool status;

           jcNonStdRasMessageHandoverSuccessUnpack( &MsgHandoverSuccess, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): Handover Success received.\n}\n");  

           copyGenericToGP10Header( &MsgHandoverSuccess.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_HANDOVER_SUCCESS;
           MsgtoCC.msgData.extHandoverSuccess.A_INTERFACE_MSG = MsgHandoverSuccess.A_INTERFACE_MSG;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkHandoverSuccess\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.extHandoverSuccess hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.extHandoverSuccess, sizeof (InterHoaVcHandoverSuccess_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Handover Success!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypePerformHandBackRQ:
		{
           NonStdRasMessagePerformHandbackRQ_t MsgPerformHandback;
           InterHoaVcMsg_t                      MsgtoCC;
           bool status;

           jcNonStdRasMessagePerformHandbackRQUnpack( &MsgPerformHandback, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): Perform Handback RQ received.\n}\n");  

           copyGenericToGP10Header( &MsgPerformHandback.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_PERFORM_HANDBACK_REQUEST;
           MsgtoCC.msgData.perfExtHandbackReq.globalCellID = MsgPerformHandback.globalCellID;
           MsgtoCC.msgData.perfExtHandbackReq.hoCause      = MsgPerformHandback.hoCause;
           MsgtoCC.msgData.perfExtHandbackReq.externalCell = MsgPerformHandback.externalCell;
           MsgtoCC.msgData.perfExtHandbackReq.mscNumber    = MsgPerformHandback.mscNumber;
           MsgtoCC.msgData.perfExtHandbackReq.A_HANDOVER_REQUEST = MsgPerformHandback.A_HANDOVER_REQUEST;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPerformHandbackReq\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.perfExtHandbackReq hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.perfExtHandbackReq, sizeof (InterHoaVcPerformHandback_t) );
           DBG_TRACE("\n      }\n");

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPerformHandbackReq\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.perfExtHandoverReq.A_HANDOVER_REQUEST hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.perfExtHandbackReq.A_HANDOVER_REQUEST, sizeof (A_INTERFACE_MSG_STRUCT_t) );
           DBG_TRACE("\n      }\n");

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Perform Handback RQ!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypePerformHandBackAck:
		{
           NonStdRasMessagePerformHandbackAck_t MsgPerformHandbackAck;
           InterHoaVcMsg_t                      MsgtoCC;
           bool status;

           jcNonStdRasMessagePerformHandbackAckUnpack( &MsgPerformHandbackAck, &pszBuffer );

           DBG_TRACE("{\nMNEThoTRACE(vblink::ProcessViperBaseMessage): Handback Ack received.\n}\n");  

           copyGenericToGP10Header( &MsgPerformHandbackAck.header, &MsgtoCC );

           MsgtoCC.msgType = INTER_EHO_PERFORM_HANDBACK_ACK;
           MsgtoCC.msgData.perfExtHandbackAck.A_INTERFACE_MSG = MsgPerformHandbackAck.A_INTERFACE_MSG;
           MsgtoCC.msgData.perfExtHandbackAck.reason = MsgPerformHandbackAck.reason;

           status=ExtHOGenericMessageHandler( &MsgtoCC );

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPerformHandbackAck\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoCC.msgData.perfExtHandbackAck hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoCC.msgData.perfExtHandbackAck, sizeof (InterHoaVcPerformHandbackAck_t) );
           DBG_TRACE("\n      }\n");

           if (!status) {
              DBG_ERROR("{\nMNEThoERROR(vblink::ProcessViperBaseMessage): CC returned error for Handback Ack!\n}\n");
           }
		}
        break;

        case NonStdRasMessageTypePageRequest:
		{
           NonStdRasMessagePageRequest_t MsgPageRequest;
           VblinkMsg_t                   MsgtoMM;
           MSG_Q_ID                      qid;

           jcNonStdRasMessagePageRequestUnpack( &MsgPageRequest, &pszBuffer );

           DBG_TRACE("{\nMGPLA(vblink::ProcessViperBaseMessage): Page Request received.\n}\n");  

           MsgtoMM.module_id                           = (T_CNI_MODULE_ID) MODULE_VBLINK;
           MsgtoMM.message_type                        = VBLINK_RR_PAGE_REQ;
           MsgtoMM.message_data.pageReq.imsi           = MsgPageRequest.imsi;
           MsgtoMM.message_data.pageReq.req_id         = MsgPageRequest.req_id;
           MsgtoMM.message_data.pageReq.channel_needed = MsgPageRequest.channel_needed;
           MsgtoMM.message_data.pageReq.paging_timeout = MsgPageRequest.paging_timeout;

           DBG_TRACE("    VBLINK MESSAGE DATA: VBLinkPageRequest\n");
           DBG_TRACE("      {\n");
           DBG_TRACE("         MsgtoMM.message_data.pageReq hexdump: ");
           DBG_HEXDUMP((unsigned char*) &MsgtoMM.message_data.pageReq, sizeof (IntraL3PageRequest_t) );
           DBG_TRACE("\n      }\n");

           DBG_TRACE("{\nMGPLA(vblink::PageMessageHandler):Sending Msg(%d) to MMTASK\n}\n",
                      (int)MsgtoMM.message_type);

           qid = mmMsgQId;

           // send the message.
           if (ERROR == msgQSend( qid,
                                  (char *) &MsgtoMM, 
                                  sizeof(struct  VblinkMsg_t), 
                                  NO_WAIT, 		//PR1553 <xxu> WAIT_FOREVER,
                                  MSG_PRI_NORMAL
                                 ) )
             {
                DBG_ERROR("{\nMGPLA(vblink::ProcessViperBaseMessage): MM returned error for Page Request!\n}\n");
             }
		}
        break;

        default:
         DBG_ERROR( "Non standard message from GMC : Error : unrecognized message %d received\n",nonStdMsgType);
		 DBG_LEAVE();
         return FALSE;
   }

   DBG_LEAVE();
   return TRUE;
}



/*
 * VBLinkRegisterMobile
 */
BOOL 
VBLinkRegisterMobile(MOBILE_ID MobileId)
{
	DBG_FUNC("VBLinkRegisterMobile",VBLINK_LAYER);
	DBG_ENTER();

   	int nSize;
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
   	NonStdRasMessageRRQ_t MsgRRQ;
	char pszMobileId[MAX_MOBILE_ID_LENGTH];
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    STATUS status;
    BOOL retStatus = FALSE;

	memset(pszMobileId,0,MAX_MOBILE_ID_LENGTH);
    jcBCDToString(pszMobileId, MobileId.digits, MobileId.numDigits );


   	DBG_TRACE("Registering Mobile %s\n", pszMobileId);

   	MsgRRQ.pszMobileID = pszMobileId;

   	pszBuffer = buffer;
   	nSize = 0;

   	jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   	jcUShortPack( NonStdRasMessageTypeRRQ, &pszBuffer, &nSize );
   	jcNonStdRasMessageRRQPack( &MsgRRQ, &pszBuffer, &nSize );



    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *)sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

    status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
    if (status == ERROR) {
        DBG_ERROR("Unable to send registration message to GMC, %d!\n",errno);
    }
    else {
        retStatus = TRUE;
    }
    DBG_LEAVE();
    return (retStatus);
}




/*
 *	VBLinkUnregisterMobile
 */
BOOL
VBLinkUnregisterMobile(MOBILE_ID	MobileId)
{
	DBG_FUNC("VBLinkUnregisterMobile",VBLINK_LAYER);
	DBG_ENTER();
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
   	int nSize;
   	NonStdRasMessageURQ_t MsgURQ;
	char pszMobileId[MAX_MOBILE_ID_LENGTH];
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    STATUS status;
    BOOL retStatus = FALSE;

	memset(pszMobileId,0,MAX_MOBILE_ID_LENGTH);
    jcBCDToString(pszMobileId, MobileId.digits, MobileId.numDigits );


   	DBG_TRACE( "Unregistering Mobile %s\n", pszMobileId );

	MsgURQ.pszMobileID = pszMobileId;

   	pszBuffer = buffer;
   	nSize = 0;

   	jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   	jcUShortPack( NonStdRasMessageTypeURQ, &pszBuffer, &nSize );
   	jcNonStdRasMessageURQPack( &MsgURQ, &pszBuffer, &nSize );


    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *)sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

    status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
    if (status == ERROR) {
        DBG_ERROR("Unable to send registration message to GMC, %d!\n",errno);
    }
    else {
        retStatus = TRUE;
    }
    DBG_LEAVE();
    return (retStatus);

}



/*
 * VBLinkSecurityRequest
 */
BOOL
VBLinkSecurityRequest(MOBILE_ID	MobileId)
{
	DBG_FUNC("VBLinkSecurityRequest",VBLINK_LAYER);
	DBG_ENTER();
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
   	int nSize;
	NonStdRasMessageSRQ_t SecurityRequest;
	char pszMobileId[MAX_MOBILE_ID_LENGTH];
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    STATUS status;
    BOOL retStatus = FALSE;

	memset(pszMobileId,0,MAX_MOBILE_ID_LENGTH);
    jcBCDToString(pszMobileId, MobileId.digits, MobileId.numDigits );

   	DBG_TRACE( "Security Request for %s\n", pszMobileId );

	SecurityRequest.pszMobileID	= pszMobileId;

   	pszBuffer = buffer;
   	nSize = 0;

   	jcUShortPack(NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   	jcUShortPack(NonStdRasMessageTypeSRQ, &pszBuffer, &nSize );
	jcNonStdRasMessageSRQPack(&SecurityRequest,&pszBuffer,&nSize );


    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *)sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

    status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
    if (status == ERROR) {
        DBG_ERROR("Unable to send registration message to GMC, %d!\n",errno);
    }
    else {
        retStatus = TRUE;
    }


	DBG_LEAVE();
	return(status);
}




/*
 * VBLinkCellLocationRequest
 */
BOOL 
VBLinkLocationRequest(VBLINK_API_CELL_LOCATION_REQUEST_MSG Msg)
{
	DBG_FUNC("VBLinkLocationRequest",VBLINK_LAYER);
	DBG_ENTER();
	
   	int nSize;
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
   	NonStdRasMessageCellLRQ_t MsgLRQ;
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    STATUS status;
    BOOL retStatus = FALSE;

	ASSERT(strlen(Msg.ViperCellId) <= MAX_VC_ID_STRING_LENGTH);

   	MsgLRQ.pszCellID = Msg.ViperCellId;
	MsgLRQ.hCell = Msg.TxnId;

   	pszBuffer = buffer;
   	nSize = 0;

   	jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   	jcUShortPack( NonStdRasMessageTypeCellLRQ, &pszBuffer, &nSize );
   	jcNonStdRasMessageCellLRQPack( &MsgLRQ, &pszBuffer, &nSize );

	DBG_TRACE("Sending LRQ message to GMC\n");
    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *)sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

    status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
    if (status == ERROR) {
        DBG_ERROR("Unable to send registration message to GMC, %d!\n",errno);
    }
    else {
        retStatus = TRUE;
    }
    DBG_LEAVE();
    return (retStatus);
}


/*
 * VBLinkSendCellId
 */
void
VBLinkSendCellId()
{   
    DBG_FUNC("VBLinkSendCellId",VBLINK_LAYER);
    DBG_ENTER();
    unsigned long ViperCellIdentifier=0;
    char ViperCellId[MAX_VC_ID_STRING_LENGTH];
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
    char szViperCellName[100];
    char szViperCellIp[20];
   	UCHAR* pszBuffer;
   	int nSize;
    NonStdRasMessageCellInfo_t CellInfo;
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    STATUS status;
    BtsBasicPackage        vbBtsBasicPackage;
    long int			sgsnIpAddress, sgsnRac;
    unsigned char	gs_ip_octets[4];

	/*
     * memset(szViperCellName,0,100);
     * strcpy(szViperCellName,"VIPERCELL");
	 */

	if (oam_getMibByteAry(MIB_viperCellName,(unsigned char *)szViperCellName,100) != STATUS_OK) {
		DBG_ERROR("Unable to get ViperCell Name from OAM!!\n");
		return ;
	}


    status = getMibStruct(MIB_btsBasicPackage, (unsigned char*)&vbBtsBasicPackage,
	 	               sizeof(BtsBasicPackage));
    if (status != STATUS_OK)  
    {
        // failed to get btsBasicPackage data.
        DBG_ERROR("OAM->Call FATAL ERROR: failed(status = %d) to get vbBtsBasicPackage\n",
                   status);
        printf("OAM->Call FATAL ERROR: failed(status = %d) to get vbBtsBasicPackage\n",
                   status);
    }   

		// Routing Area Code
    if (oam_getMibIntVar(MIB_gprsRac, (long int*)&sgsnRac) != STATUS_OK) {
	DBG_ERROR("Unable to get gprs RAC from OAM!!\n");
        DBG_LEAVE();
        return;
    }
    else {

        DBG_TRACE("gprs RAC OAM value %ld\n",sgsnRac);
    }
   
	// SGSN IP Address
    if (oam_getMibIntVar(MIB_gprsServerIpAddress, (long int*)&sgsnIpAddress) != STATUS_OK) {
	DBG_ERROR("Unable to get GS's IP Address from OAM!!\n");
        DBG_LEAVE();
        return;
    }
    else {
        gs_ip_octets[0] = (sgsnIpAddress >> 24) & 0x000000ff;
        gs_ip_octets[1] = (sgsnIpAddress >> 16) & 0x000000ff;
        gs_ip_octets[2] = (sgsnIpAddress >> 8) & 0x000000ff;
        gs_ip_octets[3] = (sgsnIpAddress) & 0x000000ff;

        sprintf(szViperCellIp,"%d.%d.%d.%d\n",gs_ip_octets[0],gs_ip_octets[1],
                gs_ip_octets[2],gs_ip_octets[3]);

        CellInfo.pszSgsnIP = szViperCellIp;
        DBG_TRACE("gprs IP OAM value %s\n",CellInfo.pszSgsnIP);
    }

    memset(ViperCellId,0,MAX_VC_ID_STRING_LENGTH);

    sprintf(ViperCellId,"%d:%d",vbBtsBasicPackage.bts_lac,vbBtsBasicPackage.bts_ci);

    DBG_TRACE( "Sending cell id %s\n", ViperCellId );

    CellInfo.pszCellID = ViperCellId;
    CellInfo.pszCellName = szViperCellName;
    CellInfo.sgsnRAC	= sgsnRac;


    pszBuffer = buffer;
    nSize = 0;
    
    jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
    jcUShortPack( NonStdRasMessageTypeCellInfo, &pszBuffer, &nSize );
    jcNonStdRasMessageCellInfoPack(&CellInfo,&pszBuffer,&nSize);

    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *)sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

    status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
    if (status == ERROR) {
        DBG_ERROR("Unable to send registration message to GMC, %d!\n",errno);
    }
	
    DBG_TRACE("    VBLinkSendCellId \n");
    DBG_TRACE("      {\n");
    DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
    DBG_TRACE("\n      }\n");

    DBG_LEAVE();
	return;
}



/*
 *  NonStdRasMessageTypeMsToNetCISS (Register CISS)
 */
bool 
VBLinkMsToNetCiss(LUDB_ID ludbIdx, unsigned char *cissMsg, int msgLen)
{

	DBG_FUNC("VBLinkMsToNetCiss",VBLINK_LAYER);
	DBG_ENTER();

   	int nSize;
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
   	NonStdRasMessageCISS_t cissReg;

	char pszMobileId[MAX_MOBILE_ID_LENGTH];
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    
	STATUS status;
    bool retStatus = false;

	MOBILE_ID MobileId = ludbGetMobileId(ludbIdx);
	memset(pszMobileId,0,MAX_MOBILE_ID_LENGTH);
    jcBCDToString(pszMobileId, MobileId.digits, MobileId.numDigits );
	cissReg.pszMobileID = pszMobileId;

	cissReg.nMsgSize = msgLen;
	cissReg.pMsgData = (PVOID) cissMsg;
   	pszBuffer = buffer;
   	nSize = 0;

   	jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   	jcUShortPack( NonStdRasMessageTypeMsToNetCISS, &pszBuffer, &nSize );
   	jcNonStdRasMessageMobileCISSPack( &cissReg, &pszBuffer, &nSize );

    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *) sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)], buffer, nSize);
   	DBG_TRACE("Sending CISS Register Message (LudbId = %d)\n", ludbIdx);

    status = send(VBLinkConfig.VBSocket, sendBuffer, (nSize+2), 0);

    if (status == ERROR) {
        DBG_ERROR("Unable to send CISS registration message (errno = %d)\n", errno);
    }
    else {
        retStatus = true;
    }

    DBG_LEAVE();
    return (retStatus);
}

/*
 *  NonStdRasMessageTypeMsToNetSMSPP (SMC -> SMR)
 */
bool 
VBLinkMsToNetSmspp(LUDB_ID ludbIdx, unsigned char *smsppMsg, unsigned int msgLen)
{

	DBG_FUNC("VBLinkMsToNetSmspp",VBLINK_LAYER);
	DBG_ENTER();

   	int nSize;
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
   	NonStdRasMessageSMSPP_t smsppNonStdRasMsg;

	char pszMobileId[MAX_MOBILE_ID_LENGTH];
    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
    unsigned short *pActualDataLen;
    
	STATUS status;
    bool retStatus = false;

	MOBILE_ID MobileId = ludbGetMobileId(ludbIdx);
	memset(pszMobileId,0,MAX_MOBILE_ID_LENGTH);
    jcBCDToString(pszMobileId, MobileId.digits, MobileId.numDigits );
	smsppNonStdRasMsg.pszMobileID = pszMobileId;

	smsppNonStdRasMsg.nMsgSize = msgLen;
	smsppNonStdRasMsg.pMsgData = (PVOID) smsppMsg;
   	pszBuffer = buffer;
   	nSize = 0;

   	jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   	jcUShortPack( NonStdRasMessageTypeMsToNetSMSPP, &pszBuffer, &nSize );
   	jcNonStdRasMessageMobileSmsppPack( &smsppNonStdRasMsg, &pszBuffer, &nSize );

    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *) sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)], buffer, nSize);
   	DBG_TRACE("Sending SMSPP message from SMC->SMR at network side (LudbId = %d)\n", ludbIdx);

    status = send(VBLinkConfig.VBSocket, sendBuffer, (nSize+2), 0);

    if (status == ERROR) {
        DBG_ERROR("Unable to send SMSPP message from SMC->SMR at network side (errno = %d)\n", errno);
    }
    else {
        retStatus = true;
    }

    DBG_LEAVE();
    return (retStatus);
}



/*
 * VBLinkTransferRequest
 */
bool
VBLinkTransferRequest(unsigned short transferReqSeqNum,VOIP_CALL_DESTINATION_ADDRESS CalledPartyNumber,
		VOIP_CALL_CALLING_PARTY_NUMBER CallingPartyNumber)
{
	DBG_FUNC("VBLinkTransferRequest",VBLINK_LAYER);
	DBG_ENTER();
	bool retStatus = FALSE;
	STATUS status;
   	int nSize;
   	UCHAR buffer[NON_STD_MSG_MAX_LEN];
   	UCHAR* pszBuffer;
	MnetRasTransferRequest_t transferRequest;

    char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
	char origNumBuffer[256],destNumBuffer[256];
    unsigned short *pActualDataLen;
    

   	pszBuffer = buffer;
   	nSize = 0;


	memset(origNumBuffer,0,256);
	memset(destNumBuffer,0,256);
   	jcBCDToString(origNumBuffer, CallingPartyNumber.digits, CallingPartyNumber.numDigits);
   	jcBCDToString(destNumBuffer, CalledPartyNumber.digits, CalledPartyNumber.numDigits);

	DBG_TRACE("Call Transfer request from %s to %s; sequence number %d\n",
				origNumBuffer,destNumBuffer,transferReqSeqNum);

	transferRequest.requestSeqNum = transferReqSeqNum;

	transferRequest.destinationAliasAddress.numberDigits = destNumBuffer;

	transferRequest.sourceAliasAddress.numberDigits = origNumBuffer;

    VBLinkGetVoipNumberType(CallingPartyNumber.numberType,&transferRequest.sourceAliasAddress);

    VBLinkGetVoipNumberType(CalledPartyNumber.numberType,&transferRequest.destinationAliasAddress);


   	jcUShortPack(NON_STD_RAS_PROTOCOL,&pszBuffer,&nSize);
   	jcUShortPack(NonStdRasMessageTypeTransferRequest,&pszBuffer,&nSize);
   	jcNonStdRasMessageTransferRequestPack(&transferRequest, &pszBuffer, &nSize );
    memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
    pActualDataLen = (unsigned short *) sendBuffer;
    *pActualDataLen = htons(nSize);

    memcpy((char*)&sendBuffer[sizeof(short)], buffer, nSize);

    status = send(VBLinkConfig.VBSocket, sendBuffer, (nSize+2), 0);

    if (status == ERROR) {
        DBG_ERROR("Unable to send transfer request message from %s to %s \n",origNumBuffer,destNumBuffer);
    }
    else {
        retStatus = true;
    }


	DBG_LEAVE();
	return (retStatus);
}



/*
 * VBLinkGetVoipNumberType
 */
void
VBLinkGetVoipNumberType(
		T_CNI_RIL3_TYPE_OF_NUMBER numType,
		MnetH323AliasAddress_t *address
		)
{
	DBG_FUNC("VBLinkGetVoipNumberType",VBLINK_LAYER);
	DBG_ENTER();


		switch(numType) {
	        case CNI_RIL3_NUMBER_TYPE_UNKNOWN:
			{
	    		address->publicType =  MNET_H323_PUBLIC_UNKNOWN;
				address->partyNumber = MNET_H323_PUBLIC_PARTY_NUMBER;
			}
			break;
	        case CNI_RIL3_NUMBER_TYPE_INTERNATIONAL:
			{
	    		address->publicType =  MNET_H323_PUBLIC_INTERNATIONAL; 
				address->partyNumber = MNET_H323_PUBLIC_PARTY_NUMBER;
			}
			break;
	        case CNI_RIL3_NUMBER_TYPE_NATIONAL:
			{
	    		address->publicType = MNET_H323_PUBLIC_NATIONAL; 
				address->partyNumber = MNET_H323_PUBLIC_PARTY_NUMBER;
			}
			break;
	        case CNI_RIL3_NUMBER_TYPE_NETWORK_SPECIFIC:
			{
	    		address->publicType = MNET_H323_PUBLIC_NETWORK_SPECIFIC;
				address->partyNumber = MNET_H323_PUBLIC_PARTY_NUMBER;
			}
			break;
	        case CNI_RIL3_NUMBER_TYPE_DEDICATED_ACCESS:
			{
				address->privateType = MNET_H323_PRIVATE_LOCAL;
				address->partyNumber = MNET_H323_PRIVATE_PARTY_NUMBER;
			}
			break;
			default:
			{
			}
			break;
		}

	DBG_LEAVE();
}


/*
 * VBLinkGetGsmNumberType
 */
void
VBLinkGetGsmNumberType(
		T_CNI_RIL3_TYPE_OF_NUMBER *numType,
		MnetH323AliasAddress_t address
		)
{
	DBG_FUNC("VBLinkGetGsmNumberType",VBLINK_LAYER);
	DBG_ENTER();


	if (address.publicType ==  MNET_H323_PUBLIC_UNKNOWN) {
		*numType = CNI_RIL3_NUMBER_TYPE_UNKNOWN;
	}
	else if (address.publicType ==  MNET_H323_PUBLIC_INTERNATIONAL) {
		*numType = CNI_RIL3_NUMBER_TYPE_INTERNATIONAL;
	}
	else if (address.publicType ==  MNET_H323_PUBLIC_NATIONAL) {
		*numType = CNI_RIL3_NUMBER_TYPE_NATIONAL;
	}
	else if (address.publicType ==  MNET_H323_PUBLIC_NETWORK_SPECIFIC) {
		*numType = CNI_RIL3_NUMBER_TYPE_NETWORK_SPECIFIC;
	}
	else if (address.publicType ==  MNET_H323_PRIVATE_LOCAL) {
		*numType = CNI_RIL3_NUMBER_TYPE_DEDICATED_ACCESS;
	}
	else {
		*numType = CNI_RIL3_NUMBER_TYPE_UNKNOWN;
	}

	DBG_LEAVE();
}






/*
 * VBLinkReconnectLoop
 */
BOOL
VBLinkReconnectLoop()
{
    DBG_FUNC("VBLinkReconnectLoop",VBLINK_LAYER);
    DBG_ENTER();
    BOOL retStatus = FALSE;
    BOOL connectStatus;
    /*
     * The only way we get out of here is through an
     * error or if connection is succesful
     */
    while (VBLinkConfig.RetryConnects) {
        DBG_TRACE("Attempting to connect to GMC..\n");
        connectStatus = VBLinkConnect();
        if (connectStatus) {
            retStatus=TRUE;
            DBG_LEAVE();
            return(retStatus);
        }
    }
    DBG_TRACE("Not attempting to connect to GMC any more..\n");
    DBG_LEAVE();
	return(retStatus);
}







/*
 * VBLinkConnect
 */
BOOL
VBLinkConnect()
{
    DBG_FUNC("VBLinkConnect",VBLINK_LAYER);
    DBG_ENTER();
    BOOL retStatus=FALSE;


    if (connect(VBLinkConfig.VBSocket,(struct sockaddr *)&VBLinkConfig.ViperBaseAddress,sizeof(VBLinkConfig.ViperBaseAddress)) == ERROR) {
        DBG_ERROR("Unable to connect to GMC at %s : %d ; error %d\n",VBLinkConfig.szViperBaseAddress,ntohs(VBLinkConfig.ViperBaseAddress.sin_port),errno);
        VBLinkConfig.ConnectedToViperBase = FALSE;
        if (close(VBLinkConfig.VBSocket)==ERROR) {
            DBG_ERROR("Unable to close VBSocket!! error %d\n",errno);
        }
        FD_CLR(VBLinkConfig.VBSocket,&VBLinkConfig.ReadSet);
        if (!VBLinkPrepareSocket()) {
            printf("Cannot create socket!! error %d\n",errno);
            DBG_LEAVE();
            return(retStatus);
        }
        taskDelay(VBLINK_CONNECT_TIMEOUT_SECS*sysClkRateGet());
    }
    else {
        FD_SET(VBLinkConfig.VBSocket,&VBLinkConfig.ReadSet);
        DBG_TRACE("Connected to GMC at %s : %d\n successfully\n",VBLinkConfig.szViperBaseAddress,ntohs(VBLinkConfig.ViperBaseAddress.sin_port));
        VBLinkSetOAMTrap(VBLINK_VIPERBASE_LINK_RESTORED);
        VBLinkConfig.ConnectedToViperBase = TRUE;
        VBLinkSendCellId();
        retStatus = TRUE;
    }


    DBG_LEAVE();
    return(retStatus);
}




/*
 * VBLinkSetOAMTrap
 */
void
VBLinkSetOAMTrap(VBLINK_OAM_TRAP Message)
{
    DBG_FUNC("VBLinkSetOAMTrap",VBLINK_LAYER);
    DBG_ENTER();

    printf("OAM event required to be indicated\n");

    switch(Message) {
        case VBLINK_VIPERBASE_LINK_LOST:
        {   
            printf("Indicating VIPERBASE_LINK_DOWN to OAM!!\n");
            if (alarm_raise(MODULE_H323,MIB_errorCode_H323, EC_VIPERBASE_LINKDOWN) != STATUS_OK)
	        {
		        printf("Unable to indicate VIPERBASE_LINK_DOWN to OAM!!\n");
                DBG_LEAVE();
		        return;
	        }
            else {
                printf("Indicated VIPERBASE_LINK_DOWN to OAM successfully!!\n");
            }
        }
        break;


        case VBLINK_VIPERBASE_LINK_RESTORED:
        {
            printf("Indicating VIPERBASE_LINK_RESTORED to OAM!!\n");
            if (alarm_clear(MODULE_H323,MIB_errorCode_H323, EC_NOERROR) != STATUS_OK)
            {
	            printf("Unable to indicate VIPERBASE_LINK_RESTORED to OAM!!\n");
                DBG_LEAVE();
	            return ;
            }
            else {
                printf("Indicated VIPERBASE_LINK_RESTORED to OAM successfully!!\n");
            }
        }
        break;


        case VBLINK_VIPERBASE_LINK_NOT_UP:
        {
            printf("Indicating VIPERBASE_LINK_NOT_UP to OAM!!\n");
            if (alarm_raise(MODULE_H323,MIB_errorCode_H323, EC_BOOTING_UP) != STATUS_OK)
            {
	            printf("Unable to indicate VIPERBASE_LINK_NOT_UP to OAM!!\n");
                DBG_LEAVE();
	            return ;
            }
            else {
                printf("Indicated VIPERBASE_LINK_NOT_UP to OAM successfully!!\n");
            }
        }
        break;


        default:
        {
            printf("Unknown message specified to be sent to OAM !!\n");
        }
        break;
    }

    DBG_LEAVE();
    return;
}













/*
 * SendVBLinkRegEvent
 */
void
SendVBLinkRegEvent(VBLINK_MOBILE_REG_EVENT RegEvent, MOBILE_ID MobileId)
{
	DBG_FUNC("SendVBLinkRegEvent",VBLINK_LAYER);
	DBG_ENTER();
    BOOL Status;

 	DBG_TRACE("Sending registration event to ludb \n");

 	Status=LudbVBLinkMessageHandler(RegEvent,MobileId);

    if (!Status) {
        DBG_ERROR("LUDB returned error for registration event %d from GK!!\n",RegEvent);
    }

	DBG_LEAVE();
  	return;
}




/*
 * SendIpAddressResponse
 */
void
SendIpAddressResponse(char *CellId,char *IpAddress,TXN_ID TxnId)
{
	DBG_FUNC("SendIpAddressResponse",VBLINK_LAYER);
	DBG_ENTER();
        bool Status=FALSE;
 	Status=CcVBLinkMessageHandler(CellId,IpAddress,TxnId);

    if (!Status) {
        DBG_ERROR("CC returned error for Cell Location response!!\n");
    }
			 
	DBG_LEAVE();
  	return;

}






/*
 * jcNonStdRasMessageRRQPack
 */
void 
jcNonStdRasMessageRRQPack( NonStdRasMessageRRQ_t* pMsgRRQ, UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgRRQ->pszMobileID, ppszBuffer, pnSize );

   return;
}



/*
 * jcNonStdRasMessageRCFUnpack
 */
void 
jcNonStdRasMessageRCFUnpack( NonStdRasMessageRCF_t* pMsgRCF, UCHAR** ppszBuffer )
{
   pMsgRCF->pszMobileID = jcStringUnpack( ppszBuffer );
   pMsgRCF->pszNumber = jcStringUnpack( ppszBuffer );

   return;
}



/*
 * jcNonStdRasMessageRRJUnpack
 */
void 
jcNonStdRasMessageRRJUnpack( NonStdRasMessageRRJ_t* pMsgRRJ, UCHAR** ppszBuffer )
{
   pMsgRRJ->pszMobileID = jcStringUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageURQPack
 */
void 
jcNonStdRasMessageURQPack( NonStdRasMessageURQ_t* pMsgURQ, UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgURQ->pszMobileID, ppszBuffer, pnSize );

   return;
}


/*
 * jcNonStdRasMessageURQUnpack
 */
void 
jcNonStdRasMessageURQUnpack( NonStdRasMessageURQ_t* pMsgURQ, UCHAR** ppszBuffer )
{
   pMsgURQ->pszMobileID = jcStringUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageUCFUnpack
 */
void 
jcNonStdRasMessageUCFUnpack( NonStdRasMessageUCF_t* pMsgUCF, UCHAR** ppszBuffer )
{
   pMsgUCF->pszMobileID = jcStringUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageCellLRQPack
 */
void 
jcNonStdRasMessageCellLRQPack( NonStdRasMessageCellLRQ_t* pMsgCellLRQ,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellLRQ->pszCellID, ppszBuffer, pnSize );
   jcULongPack( pMsgCellLRQ->hCell, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageCellLRQUnpack
 */
void 
jcNonStdRasMessageCellLRQUnpack( NonStdRasMessageCellLRQ_t* pMsgCellLRQ, UCHAR** ppszBuffer )
{
   pMsgCellLRQ->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellLRQ->hCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 * jcNonStdRasMessageCellLCFPack
 */
void 
jcNonStdRasMessageCellLCFPack( NonStdRasMessageCellLCF_t* pMsgCellLCF,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellLCF->pszCellID, ppszBuffer, pnSize );
   jcStringPack( pMsgCellLCF->pszIpAddress, ppszBuffer, pnSize );
   jcULongPack( pMsgCellLCF->hCell, ppszBuffer, pnSize );

   return;
}


/*
 * jcNonStdRasMessageCellLCFUnpack
 */
void 
jcNonStdRasMessageCellLCFUnpack( NonStdRasMessageCellLCF_t* pMsgCellLCF, UCHAR** ppszBuffer )
{
   pMsgCellLCF->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellLCF->pszIpAddress = jcStringUnpack( ppszBuffer );
   pMsgCellLCF->hCell = jcULongUnpack( ppszBuffer );

   return;
}



/*
 * jcNonStdRasMessageCellLRJPack
 */
void 
jcNonStdRasMessageCellLRJPack( NonStdRasMessageCellLRJ_t* pMsgCellLRJ,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellLRJ->pszCellID, ppszBuffer, pnSize );
   jcULongPack( pMsgCellLRJ->hCell, ppszBuffer, pnSize );

   return;
}


/*
 * jcNonStdRasMessageCellLRJUnpack
 */
void 
jcNonStdRasMessageCellLRJUnpack( NonStdRasMessageCellLRJ_t* pMsgCellLRJ, UCHAR** ppszBuffer )
{
   pMsgCellLRJ->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellLRJ->hCell = jcULongUnpack( ppszBuffer );

   return;
}



/*
 * jcNonStdRasMessageMobileProfilePack
 */
void 
jcNonStdRasMessageMobileProfilePack( NonStdRasMessageMobileProfile_t* pMsg,
                                          UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsg->pszMobileID, ppszBuffer, pnSize );
   jcUShortPack( pMsg->nProfileSize, ppszBuffer, pnSize );
   jcBufferPack( pMsg->pProfileData, pMsg->nProfileSize, ppszBuffer, pnSize );

   return;
}


/*
 * jcNonStdRasMessageMobileProfileUnpack
 */
void 
jcNonStdRasMessageMobileProfileUnpack( NonStdRasMessageMobileProfile_t* pMsg,
                                            UCHAR** ppszBuffer )
{
   pMsg->pszMobileID = jcStringUnpack( ppszBuffer );
   pMsg->nProfileSize = jcUShortUnpack( ppszBuffer );
   pMsg->pProfileData = jcBufferUnpack( pMsg->nProfileSize, ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageCellInfoPack
 */
void 
jcNonStdRasMessageCellInfoPack( NonStdRasMessageCellInfo_t* pMsgCellInfo,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellInfo->pszCellID, ppszBuffer, pnSize );
   jcStringPack( pMsgCellInfo->pszCellName, ppszBuffer, pnSize );
   jcUShortPack( pMsgCellInfo->sgsnRAC, ppszBuffer, pnSize );  // shmin 09/17
   jcStringPack( pMsgCellInfo->pszSgsnIP, ppszBuffer, pnSize );


   return;
}

/*
 * jcNonStdRasMessageCellInfoUnpack
 */
void 
jcNonStdRasMessageCellInfoUnpack( NonStdRasMessageCellInfo_t* pMsgCellInfo, UCHAR** ppszBuffer )
{
   pMsgCellInfo->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellInfo->pszCellName = jcStringUnpack( ppszBuffer );

   if( pMsgCellInfo->pszCellName && strlen( pMsgCellInfo->pszCellName ) > 32 )
      pMsgCellInfo->pszCellName = NULL; /* !! patch for older version */

   pMsgCellInfo->sgsnRAC = jcUShortUnpack( ppszBuffer );   // shmin 09/17
   pMsgCellInfo->pszSgsnIP = jcStringUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageMobileCISSPack
 */
void 
jcNonStdRasMessageMobileCISSPack( NonStdRasMessageCISS_t* pMsg,
                                       UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsg->pszMobileID, ppszBuffer, pnSize );
   jcUShortPack( pMsg->nMsgSize, ppszBuffer, pnSize );
   jcBufferPack( pMsg->pMsgData, pMsg->nMsgSize, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageMobileCISSUnpack
 */
void 
jcNonStdRasMessageMobileCISSUnpack( NonStdRasMessageCISS_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->pszMobileID = jcStringUnpack( ppszBuffer );
   pMsg->nMsgSize = jcUShortUnpack( ppszBuffer );
   pMsg->pMsgData = jcBufferUnpack( pMsg->nMsgSize, ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageCellSynchronizePack
 */
void 
jcNonStdRasMessageCellSynchronizePack( NonStdRasMessageCellSynchronize_t* pMsg,
                                            UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsg->pszCellID, ppszBuffer, pnSize );
   jcUShortPack( pMsg->nMode, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageCellSynchronizeUnpack
 */
void 
jcNonStdRasMessageCellSynchronizeUnpack( NonStdRasMessageCellSynchronize_t* pMsg,
                                              UCHAR** ppszBuffer )
{
   pMsg->pszCellID = jcStringUnpack( ppszBuffer );
   pMsg->nMode = jcUShortUnpack( ppszBuffer );

   return;
}

/*
 * jcNonStdRasMessageSRQPack
 */
void 
jcNonStdRasMessageSRQPack( NonStdRasMessageSRQ_t* pMsg, UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsg->pszMobileID, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageSRQUnpack
 */
void 
jcNonStdRasMessageSRQUnpack( NonStdRasMessageSRQ_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->pszMobileID = jcStringUnpack( ppszBuffer );

   return;
}

/*
 * jcNonStdRasMessageSRSPack
 */
void 
jcNonStdRasMessageSRSPack( NonStdRasMessageSRS_t* pMsg, UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsg->pszMobileID, ppszBuffer, pnSize );
   jcUShortPack( pMsg->nAuthSets, ppszBuffer, pnSize );
   jcBufferPack( pMsg->pAuthSets, (USHORT)( pMsg->nAuthSets * sizeof( T_AUTH_TRIPLET ) ),
                 ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageSRSUnpack
 */
void 
jcNonStdRasMessageSRSUnpack( NonStdRasMessageSRS_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->pszMobileID = jcStringUnpack( ppszBuffer );
   pMsg->nAuthSets = jcUShortUnpack( ppszBuffer );
   pMsg->pAuthSets = (T_AUTH_TRIPLET*)jcBufferUnpack(
                           (USHORT)( pMsg->nAuthSets * sizeof( T_AUTH_TRIPLET ) ), ppszBuffer );
   return;
}

/*
 * jcNonStdRasMessageMobileSmsppPack
 */
void 
jcNonStdRasMessageMobileSmsppPack( NonStdRasMessageSMSPP_t* pMsg,
                                       UCHAR** ppszBuffer, int* pnSize)
{
    jcNonStdRasMessageMobileCISSPack(pMsg, ppszBuffer, pnSize);
}

/*
 * jcNonStdRasMessageMobileSmsppUnpack
 */
void 
jcNonStdRasMessageMobileSmsppUnpack(NonStdRasMessageSMSPP_t* pMsg,
                                         UCHAR** ppszBuffer)
{
   jcNonStdRasMessageMobileCISSUnpack(pMsg, ppszBuffer);
}

/*
 * jcNonStdRasMessageTransferRequestPack
 */
void 
jcNonStdRasMessageTransferRequestPack( MnetRasTransferRequest_t* pMsg,
                                            UCHAR** ppszBuffer, int* pnSize )
{
   jcUShortPack( pMsg->requestSeqNum, ppszBuffer, pnSize );
   jcNonStdRasMessageH323AliasAddressPack( &pMsg->destinationAliasAddress, ppszBuffer, pnSize );
   jcNonStdRasMessageH323AliasAddressPack( &pMsg->sourceAliasAddress, ppszBuffer, pnSize );
   return;
}

/*
 * jcNonStdRasMessageTransferRequestUnpack
 */
void 
jcNonStdRasMessageTransferRequestUnpack( MnetRasTransferRequest_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->requestSeqNum = jcUShortUnpack( ppszBuffer );
   jcNonStdRasMessageH323AliasAddressUnpack( &pMsg->destinationAliasAddress, ppszBuffer );
   jcNonStdRasMessageH323AliasAddressUnpack( &pMsg->sourceAliasAddress, ppszBuffer );
   return;
}

/*
 * jcNonStdRasMessageTransferConfirmPack
 */
void 
jcNonStdRasMessageTransferConfirmPack( MnetRasTransferConfirm_t* pMsg,
                                            UCHAR** ppszBuffer, int* pnSize )
{
   jcUShortPack( pMsg->requestSeqNum, ppszBuffer, pnSize );
   jcNonStdRasMessageH323AliasAddressPack( &pMsg->destinationAliasAddress, ppszBuffer, pnSize );
   return;
}

/*
 * jcNonStdRasMessageTransferConfirmUnpack
 */
void 
jcNonStdRasMessageTransferConfirmUnpack( MnetRasTransferConfirm_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->requestSeqNum = jcUShortUnpack( ppszBuffer );
   jcNonStdRasMessageH323AliasAddressUnpack( &pMsg->destinationAliasAddress, ppszBuffer );
   return;
}

/*
 * jcNonStdRasMessageTransferRejectPack
 */
void 
jcNonStdRasMessageTransferRejectPack( MnetRasTransferReject_t* pMsg,
                                           UCHAR** ppszBuffer, int* pnSize )
{
   jcUShortPack( pMsg->requestSeqNum, ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->rejectCause, ppszBuffer, pnSize );
   return;
}

/*
 * jcNonStdRasMessageTransferRejecttUnpack
 */
void 
jcNonStdRasMessageTransferRejecttUnpack( MnetRasTransferReject_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->requestSeqNum = jcUShortUnpack( ppszBuffer );
   pMsg->rejectCause = (MnetRasRejectCause_t)jcUShortUnpack( ppszBuffer );
   return;
}

/*
 * jcNonStdRasMessageH323AliasAddressPack
 */
void 
jcNonStdRasMessageH323AliasAddressPack( MnetH323AliasAddress_t* pMsg,
                                             UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsg->numberDigits, ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->partyNumber, ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->publicType, ppszBuffer, pnSize );
   return;
}

/*
 * jcNonStdRasMessageH323AliasAddressUnpack
 */
void 
jcNonStdRasMessageH323AliasAddressUnpack( MnetH323AliasAddress_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->numberDigits = jcStringUnpack( ppszBuffer );
   pMsg->partyNumber = (MnetH323PartyNumber_t)jcUShortUnpack( ppszBuffer );
   pMsg->publicType = (MnetH323PublicTypeOfNumber_t)jcUShortUnpack( ppszBuffer );
   return;
}



/*
 * VBLinkShowKeepAliveInterval
 */
int
VBLinkShowKeepAliveInterval()
{
    printf("Using KeepAlive Interval of %d seconds\n",KeepAliveInterval);
}

/*
 * VBLinkShowMaxKeepAliveCount
 */
int
VBLinkShowMaxKeepAliveCount()
{
    printf("Maximum number of Unacknowledged KeepAlives : %d \n",MaxKeepAliveTxCount);
}



/*
 * VBLinkSetKeepAliveInterval
 */
int
VBLinkSetKeepAliveInterval(int interval)
{
    KeepAliveInterval = VBLinkConfig.KeepAliveInterval = interval;
    VBLinkShowKeepAliveInterval();
}


/*
 * VBLinkSetMaxKeepAliveCount
 */
int
VBLinkSetMaxKeepAliveCount(int count)
{
    MaxKeepAliveTxCount = VBLinkConfig.MaxKeepAliveTxCount = count;
    VBLinkShowMaxKeepAliveCount();
}

// -------------------------------- EXTERNAL HANDOVER ------------------------------
// ext-HO <chenj:05-15-01>
/*
 * jcNonStdRasMessageGlobalCellIdPack
 */
void 
jcNonStdRasMessageGlobalCellIdPack( GlobalCellId_t* pMsg, UCHAR** ppszBuffer, int* pnSize )
{
   jcUShortPack( (USHORT)pMsg->ci_disc, ppszBuffer, pnSize );
   jcBufferPack(  (PVOID)pMsg->mcc, (USHORT) NUM_MCC_DIGITS, ppszBuffer, pnSize );
   jcBufferPack(  (PVOID)pMsg->mnc, (USHORT) NUM_MNC_DIGITS, ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->lac, ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->ci, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageGlobalCellIdUnpack
 */
void
jcNonStdRasMessageGlobalCellIdUnpack( GlobalCellId_t* pMsg, UCHAR** ppszBuffer )
{
   PVOID  pdata;

   pMsg->ci_disc = (char) jcUShortUnpack( ppszBuffer );
   pdata         =        jcBufferUnpack( (USHORT) NUM_MCC_DIGITS, ppszBuffer );
   memcpy( pMsg->mcc, (char *)pdata, NUM_MCC_DIGITS );
   pdata         =        jcBufferUnpack( (USHORT) NUM_MNC_DIGITS, ppszBuffer );
   memcpy( pMsg->mnc, (char *)pdata, NUM_MNC_DIGITS );
   pMsg->lac     = (unsigned short) jcUShortUnpack( ppszBuffer );
   pMsg->ci      = (unsigned short) jcUShortUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageHoaVcHeaderPack
 */
void 
jcNonStdRasMessageHoaVcHeaderPack( InterHoaVcHeader_t* pMsg, UCHAR** ppszBuffer, int* pnSize )
{
   // Orig Info
   jcUShortPack( (USHORT)pMsg->origSubId, ppszBuffer, pnSize );

   // Dest Info
   jcUShortPack( (USHORT)pMsg->destSubId, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageHoaVcHeaderUnpack
 */
void 
jcNonStdRasMessageHoaVcHeaderUnpack( InterHoaVcHeader_t* pMsg, UCHAR** ppszBuffer )
{
   // Orig Info
   pMsg->origSubId        = (short)  jcUShortUnpack( ppszBuffer );

   // Dest Info
   pMsg->destSubId        = (short)  jcUShortUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageAInterfaceMsgPack
 */
void 
jcNonStdRasMessageAInterfaceMsgPack( A_INTERFACE_MSG_STRUCT_t* pMsg,
                                     UCHAR** ppszBuffer, int* pnSize )
{
   jcUShortPack(         pMsg->msglength, ppszBuffer, pnSize );
   jcBufferPack( (PVOID) pMsg->A_INTERFACE_DATA, pMsg->msglength, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageAInterfaceMsgUnpack
 */
void 
jcNonStdRasMessageAInterfaceMsgUnpack( A_INTERFACE_MSG_STRUCT_t* pMsg, UCHAR** ppszBuffer )
{
   PVOID  pdata;

   pMsg->msglength        = jcUShortUnpack( ppszBuffer );

   if (pMsg->msglength < MAX_A_INTERFACE_LEN) {
      pdata               = jcBufferUnpack( pMsg->msglength, ppszBuffer );
      memcpy( pMsg->A_INTERFACE_DATA, (char *)pdata, pMsg->msglength);
   } else
      memset( pMsg->A_INTERFACE_DATA, 0, MAX_A_INTERFACE_LEN);

   return;
}

/*
 * jcNonStdRasMessageE164NumberPack
 */
void 
jcNonStdRasMessageE164NumberPack( T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER* pMsg,
                                  UCHAR** ppszBuffer, int* pnSize )
{
   // Store boolean in a USHORT
   if (pMsg->ie_present)
      jcUShortPack( (USHORT) 1, ppszBuffer, pnSize );
   else 
      jcUShortPack( (USHORT) 0, ppszBuffer, pnSize );

   jcULongPack( (ULONG)pMsg->numberType, ppszBuffer, pnSize );
   jcULongPack( (ULONG)pMsg->numberingPlan, ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->numDigits, ppszBuffer, pnSize );
   jcBufferPack( (PVOID) pMsg->digits, (USHORT) CNI_RIL3_MAX_BCD_DIGITS, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageE164NumberUnpack
 */
void 
jcNonStdRasMessageE164NumberUnpack( T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER* pMsg, UCHAR** ppszBuffer )
{
   PVOID  pdata;

   // Unpack boolean from USHORT
   if (jcUShortUnpack( ppszBuffer ) == 1)
      pMsg->ie_present             = TRUE;
   else
      pMsg->ie_present             = FALSE;

   pMsg->numberType      = (T_CNI_RIL3_TYPE_OF_NUMBER) jcULongUnpack( ppszBuffer );
   pMsg->numberingPlan   = (T_CNI_RIL3_NUMBERING_PLAN_ID) jcULongUnpack( ppszBuffer );
   pMsg->numDigits       = (short) jcUShortUnpack( ppszBuffer );

   pdata                 = jcBufferUnpack( (USHORT) CNI_RIL3_MAX_BCD_DIGITS, ppszBuffer );
   memcpy( pMsg->digits, (unsigned char *)pdata, CNI_RIL3_MAX_BCD_DIGITS);

   return;
}



/*
 * jcNonStdRasMessagePerformHandoverRQPack
 */
void 
jcNonStdRasMessagePerformHandoverRQPack( NonStdRasMessagePerformHandoverRQ_t* pMsg, 
                                         UCHAR** ppszBuffer,
                                         int* pnSize )
{

DBG_FUNC("jcNonStdRasMessagePerformHandoverRQPack",VBLINK_LAYER);
DBG_ENTER();
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcNonStdRasMessageGlobalCellIdPack( &(pMsg->globalCellID), ppszBuffer, pnSize );
   jcUShortPack( (USHORT)pMsg->hoCause, ppszBuffer, pnSize );

   // Store boolean in a USHORT
   if (pMsg->hoNumberReqd)
      jcUShortPack( (USHORT) 1, ppszBuffer, pnSize );
   else
      jcUShortPack( (USHORT) 0, ppszBuffer, pnSize );

   jcNonStdRasMessageE164NumberPack( &(pMsg->handoverNumber), ppszBuffer, pnSize );
   jcNonStdRasMessageE164NumberPack( &(pMsg->mscNumber), ppszBuffer, pnSize );
   jcNonStdRasMessageAInterfaceMsgPack( &(pMsg->A_HANDOVER_REQUEST), ppszBuffer, pnSize );

DBG_LEAVE();

   return;
}

/*
 * jcNonStdRasMessagePerformHandoverRQUnpack
 */
void 
jcNonStdRasMessagePerformHandoverRQUnpack( NonStdRasMessagePerformHandoverRQ_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   jcNonStdRasMessageGlobalCellIdUnpack  ( &(pMsg->globalCellID), ppszBuffer );
   pMsg->hoCause = (short) jcUShortUnpack( ppszBuffer );

   // Unpack boolean from USHORT
   if (jcUShortUnpack( ppszBuffer ) == 1)
      pMsg->hoNumberReqd             = TRUE;
   else
      pMsg->hoNumberReqd             = FALSE;

   jcNonStdRasMessageE164NumberUnpack( &(pMsg->handoverNumber), ppszBuffer);
   jcNonStdRasMessageE164NumberUnpack( &(pMsg->mscNumber), ppszBuffer);
   jcNonStdRasMessageAInterfaceMsgUnpack( &(pMsg->A_HANDOVER_REQUEST), ppszBuffer);

   return;
}

/*
 * jcNonStdRasMessageEndHandoverPack
 */
void 
jcNonStdRasMessageEndHandoverPack( NonStdRasMessageEndHandover_t* pMsg, 
                                   UCHAR** ppszBuffer,
                                   int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcUShortPack( pMsg->reason, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageEndHandoverUnpack
 */
void 
jcNonStdRasMessageEndHandoverUnpack( NonStdRasMessageEndHandover_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   pMsg->reason = jcUShortUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageLayer3MsgPack
 */
void 
jcNonStdRasMessageLayer3MsgPack( LAYER3PDU_STRUCT_t* pMsg,
                                 UCHAR** ppszBuffer, int* pnSize )
{
   jcUShortPack(         pMsg->msglength, ppszBuffer, pnSize );
   jcBufferPack( (PVOID) pMsg->LAYER3DATA, pMsg->msglength, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageLayer3MsgUnpack
 */
void 
jcNonStdRasMessageLayer3MsgUnpack( LAYER3PDU_STRUCT_t* pMsg, UCHAR** ppszBuffer )
{
   PVOID  pdata;

   pMsg->msglength        = jcUShortUnpack( ppszBuffer );

   if (pMsg->msglength < MAX_LAYER3_PDU_LEN) {
      pdata = jcBufferUnpack( pMsg->msglength, ppszBuffer );
      memcpy( pMsg->LAYER3DATA, (char *)pdata, pMsg->msglength);
   } else
      memset( pMsg->LAYER3DATA, 0, MAX_A_INTERFACE_LEN);

   return;
}

/*
 * jcNonStdRasMessagePostHandoverMobEventPack
 */
void 
jcNonStdRasMessagePostHandoverMobEventPack( NonStdRasMessagePostHandoverMobEvent_t* pMsg, 
                                            UCHAR** ppszBuffer,
                                            int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcNonStdRasMessageLayer3MsgPack   ( &(pMsg->LAYER3PDU), ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessagePostHandoverMobEventUnpack
 */
void 
jcNonStdRasMessagePostHandoverMobEventUnpack( NonStdRasMessagePostHandoverMobEvent_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   jcNonStdRasMessageLayer3MsgUnpack( &(pMsg->LAYER3PDU), ppszBuffer);

   return;
}


/*
 * jcNonStdRasMessagePerformHandoverAckPack
 */
void 
jcNonStdRasMessagePerformHandoverAckPack( NonStdRasMessagePerformHandoverAck_t* pMsg, 
                                          UCHAR** ppszBuffer,
                                          int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcNonStdRasMessageE164NumberPack( &(pMsg->handoverNumber), ppszBuffer, pnSize );
   jcNonStdRasMessageAInterfaceMsgPack( &(pMsg->A_INTERFACE_MSG), ppszBuffer, pnSize );
   jcUShortPack(pMsg->reason, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessagePerformHandoverAckUnpack
 */
void 
jcNonStdRasMessagePerformHandoverAckUnpack( NonStdRasMessagePerformHandoverAck_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   jcNonStdRasMessageE164NumberUnpack( &(pMsg->handoverNumber), ppszBuffer);
   jcNonStdRasMessageAInterfaceMsgUnpack( &(pMsg->A_INTERFACE_MSG), ppszBuffer);
   pMsg->reason = jcUShortUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageHandoverAccessPack
 */
void 
jcNonStdRasMessageHandoverAccessPack( NonStdRasMessageHandoverAccess_t* pMsg, 
                                      UCHAR** ppszBuffer,
                                      int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcNonStdRasMessageAInterfaceMsgPack( &(pMsg->A_INTERFACE_MSG), ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageHandoverAccessUnpack
 */
void 
jcNonStdRasMessageHandoverAccessUnpack( NonStdRasMessageHandoverAccess_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   jcNonStdRasMessageAInterfaceMsgUnpack( &(pMsg->A_INTERFACE_MSG), ppszBuffer);

   return;
}

/*
 * jcNonStdRasMessageHandoverSuccessPack
 */
void 
jcNonStdRasMessageHandoverSuccessPack( NonStdRasMessageHandoverSuccess_t* pMsg, 
                                       UCHAR** ppszBuffer,
                                       int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcNonStdRasMessageAInterfaceMsgPack( &(pMsg->A_INTERFACE_MSG), ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageHandoverSuccessUnpack
 */
void 
jcNonStdRasMessageHandoverSuccessUnpack( NonStdRasMessageHandoverSuccess_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   jcNonStdRasMessageAInterfaceMsgUnpack( &(pMsg->A_INTERFACE_MSG), ppszBuffer);

   return;
}


/*
 * jcNonStdRasMessagePerformHandbackRQPack
 */
void 
jcNonStdRasMessagePerformHandbackRQPack( NonStdRasMessagePerformHandbackRQ_t* pMsg, 
                                        UCHAR** ppszBuffer,
                                        int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );

   jcUShortPack( (USHORT) pMsg->hoCause, ppszBuffer, pnSize );

   jcNonStdRasMessageGlobalCellIdPack( &(pMsg->globalCellID), ppszBuffer, pnSize );

   // Store boolean in a USHORT
   if (pMsg->externalCell)
      jcUShortPack( (USHORT) 1, ppszBuffer, pnSize );
   else
      jcUShortPack( (USHORT) 0, ppszBuffer, pnSize );

   jcNonStdRasMessageE164NumberPack( &(pMsg->mscNumber), ppszBuffer, pnSize );
   jcNonStdRasMessageAInterfaceMsgPack( &(pMsg->A_HANDOVER_REQUEST), ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessagePerformHandbackRQUnpack
 */
void 
jcNonStdRasMessagePerformHandbackRQUnpack( NonStdRasMessagePerformHandbackRQ_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );

   pMsg->hoCause = jcUShortUnpack( ppszBuffer );

   jcNonStdRasMessageGlobalCellIdUnpack  ( &(pMsg->globalCellID), ppszBuffer );

   // Unpack boolean from USHORT
   if (jcUShortUnpack( ppszBuffer ) == 1)
      pMsg->externalCell             = TRUE;
   else
      pMsg->externalCell             = FALSE;

   jcNonStdRasMessageE164NumberUnpack( &(pMsg->mscNumber), ppszBuffer);
   jcNonStdRasMessageAInterfaceMsgUnpack( &(pMsg->A_HANDOVER_REQUEST), ppszBuffer);

   return;
}


/*
 * jcNonStdRasMessagePerformHandbackAckPack
 */
void 
jcNonStdRasMessagePerformHandbackAckPack( NonStdRasMessagePerformHandbackAck_t* pMsg, 
                                          UCHAR** ppszBuffer,
                                          int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );
   jcNonStdRasMessageAInterfaceMsgPack( &(pMsg->A_INTERFACE_MSG), ppszBuffer, pnSize );
   jcUShortPack(pMsg->reason, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessagePerformHandbackAckUnpack
 */
void 
jcNonStdRasMessagePerformHandbackAckUnpack( NonStdRasMessagePerformHandbackAck_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );
   jcNonStdRasMessageAInterfaceMsgUnpack( &(pMsg->A_INTERFACE_MSG), ppszBuffer);
   pMsg->reason = jcUShortUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessageReleaseHOAPack
 */
void 
jcNonStdRasMessageReleaseHOAPack( NonStdRasMessageReleaseHOA_t* pMsg, 
                                  UCHAR** ppszBuffer,
                                  int* pnSize )
{
   jcNonStdRasMessageHoaVcHeaderPack ( &(pMsg->header), ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessageReleaseHOAUnpack
 */
void 
jcNonStdRasMessageReleaseHOAUnpack( NonStdRasMessageReleaseHOA_t* pMsg, UCHAR** ppszBuffer )
{
   jcNonStdRasMessageHoaVcHeaderUnpack ( &(pMsg->header), ppszBuffer );

   return;
}



/*
 * VBLinkPerformHandoverRequest
 */
BOOL 
VBLinkPerformHandoverRequest(NonStdRasMessagePerformHandoverRQ_t* Msg)
{
   DBG_FUNC("VBLinkPerformHandoverRequest",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePerformHandoverRQ, &pszBuffer, &nSize );

   jcNonStdRasMessagePerformHandoverRQPack( Msg, &pszBuffer, &nSize );


   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandoverRequest): Sending Perform Handover message to GMC\n}\n");

   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA:\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPerformHandoverRequest): Unable to send Perform Handover Request message to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_LEAVE();
   return (retStatus);
}

/*
 * VBLinkEndHandover
 */
BOOL 
VBLinkEndHandover(NonStdRasMessageEndHandover_t* Msg)
{
   DBG_FUNC("VBLinkEndHandover",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeEndHandover, &pszBuffer, &nSize );
   jcNonStdRasMessageEndHandoverPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandoverRequest): Sending End Handover message to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPerformHandoverRequest): Unable to send End Handover message to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_LEAVE();
   return (retStatus);
}


/*
 * VBLinkPostHandoverMobEvent
 */
BOOL 
VBLinkPostHandoverMobEvent(NonStdRasMessagePostHandoverMobEvent_t* Msg)
{
   DBG_FUNC("VBLinkPostHandoverMobEvent",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePostHandoverMobEvent, &pszBuffer, &nSize );
   jcNonStdRasMessagePostHandoverMobEventPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPostHandoverMobEvent): Sending Post Handover Mob Event to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPostHandoverMobEvent): Unable to send Post Handover Mob Event to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_LEAVE();
   return (retStatus);
}


/*
 * VBLinkPerformHandoverAck
 */
BOOL 
VBLinkPerformHandoverAck(NonStdRasMessagePerformHandoverAck_t* Msg)
{
   DBG_FUNC("VBLinkPerformHandoverAck",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePerformHandoverAck, &pszBuffer, &nSize );

   jcNonStdRasMessagePerformHandoverAckPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandoverAck): Sending Handover Ack to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPerformHandoverAck): Unable to send Handover Ack to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_LEAVE();
   return (retStatus);
}


/*
 * VBLinkHandoverAccess
 */
BOOL 
VBLinkHandoverAccess(NonStdRasMessageHandoverAccess_t* Msg)
{
   DBG_FUNC("VBLinkHandoverAccess",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeHandoverAccessMsg, &pszBuffer, &nSize );

   jcNonStdRasMessageHandoverAccessPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkHandoverAccess): Sending Handover Access to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkHandoverAccess): Unable to send Handover Access to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkHandoverAccess): leaving......\n}\n");
   DBG_LEAVE();
   return (retStatus);
}


/*
 * VBLinkHandoverSuccess
 */
BOOL 
VBLinkHandoverSuccess(NonStdRasMessageHandoverSuccess_t* Msg)
{
   DBG_FUNC("VBLinkHandoverSuccess",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeHandoverSuccessMsg, &pszBuffer, &nSize );

   jcNonStdRasMessageHandoverSuccessPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkHandoverSuccess): Sending Handover Success to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkHandoverSuccess): Unable to send Handover Success to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkHandoverSuccess): leaving......\n}\n");
   DBG_LEAVE();
   return (retStatus);
}



/*
 * VBLinkPerformHandback
 */
BOOL 
VBLinkPerformHandback(NonStdRasMessagePerformHandbackRQ_t* Msg)
{
   DBG_FUNC("VBLinkPerformHandback",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePerformHandBackRQ, &pszBuffer, &nSize );

   jcNonStdRasMessagePerformHandbackRQPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandback): Sending Handback Request to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPerformHandback): Unable to send Handback Request to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandback): leaving......\n}\n");
   DBG_LEAVE();
   return (retStatus);
}


/*
 * VBLinkPerformHandbackAck
 */
BOOL 
VBLinkPerformHandbackAck(NonStdRasMessagePerformHandbackAck_t* Msg)
{
   DBG_FUNC("VBLinkPerformHandbackAck",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePerformHandBackAck, &pszBuffer, &nSize );

   jcNonStdRasMessagePerformHandbackAckPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandbackAck): Sending Handback Ack to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPerformHandbackAck): Unable to send Handback Ack to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPerformHandbackAck): leaving......\n}\n");
   DBG_LEAVE();
   return (retStatus);
}


/*
 * VBLinkReleaseHOA
 */
BOOL 
VBLinkReleaseHOA(NonStdRasMessageReleaseHOA_t* Msg)
{
   DBG_FUNC("VBLinkReleaseHOA",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypeReleaseHOA, &pszBuffer, &nSize );

   jcNonStdRasMessageReleaseHOAPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkReleaseHOA): Sending Release HOA to GMC\n}\n");
   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA: PART I\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkReleaseHOA): Unable to send Release HOA to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkReleaseHOA): leaving......\n}\n");
   DBG_LEAVE();
   return (retStatus);
}


/*
 * copyGenericToGP10Header
 */
void 
copyGenericToGP10Header( InterHoaVcHeader_t* genheader,
                         InterHoaVcMsg_t* gp10header )
{
   gp10header->origSubId          = (JcSubId_t)  genheader->origSubId;
   gp10header->destSubId          = (JcSubId_t)  genheader->destSubId;

   return;
}


bool
ExtHOGenericMessageHandler(InterHoaVcMsg_t* Msg)
{
  MSG_Q_ID qid;

  DBG_FUNC("ExtHOGenericMessageHandler",VBLINK_LAYER);
  DBG_ENTER()

  DBG_TRACE("{\nMNEThoTRACE(vblink::ExtHOGenericMessageHandler):Sending Msg(%d)\n}\n", (int)Msg->msgType);

  if (Msg->destSubId == -1)
    { // First msg from HOA, send to CCTASK
       DBG_TRACE("   First message from HOA (destSubId=-1), sending to CCTASK (ccMsgQid=%d)\n", (int)ccMsgQId);
       qid = ccMsgQId;
    }
  else
    { // Send directly to appropriate CCSESSION

      DBG_TRACE("   Destination SubID known (destSubId=%d), sending to CALLTASK\n", (int)Msg->destSubId);

      if ((0 <= Msg->destSubId) &&
        ( Msg->destSubId < CC_MAX_CC_CALLS))
        {
          qid = ccSession[Msg->destSubId].msgQId;
        }
      else
        {
          // send it to the cc main task so it can print out a log.
          // Don't log from here.
          DBG_TRACE("   Destination SubID OUT OF RANGE ERROR (destSubId=%d), sending to CCTASK\n", (int)Msg->destSubId);
          qid = ccMsgQId;
        }
    }


  Msg->module_id = (T_CNI_MODULE_ID) MODULE_EXTHOA;

  DBG_TRACE("   Setting Module ID: (MODULE_EXTHOA=%x) (Msg->module_id=%x)\n", 
                MODULE_EXTHOA, Msg->module_id);

  // send the message.
  if (ERROR == msgQSend( qid,
                         (char *) Msg, 
                         sizeof(struct  InterHoaVcMsg_t), 
                         NO_WAIT, 		//PR1553 <xxu> WAIT_FOREVER,
                         MSG_PRI_NORMAL
                         ) )
    {
      DBG_ERROR("   SendCC msgQSend (QID = %p) ERROR\n", (int)qid);
      DBG_LEAVE();
      return(false);
    }
  else
    {
      DBG_LEAVE();
      return(true);
    }
}


// ----------------------- MULTIPLE GP10 PER LAC --------------------------
// MGPLA <chenj:08-06-01>

/*
 * jcNonStdRasMessageIMSIPack
 */
void 
jcNonStdRasMessageIMSIPack( T_CNI_RIL3_IE_MOBILE_ID* pMsg, UCHAR** ppszBuffer, int* pnSize )
{
   // Store boolean in a USHORT
   if (pMsg->ie_present)
      jcUShortPack( (USHORT) 1, ppszBuffer, pnSize );
   else
      jcUShortPack( (USHORT) 0, ppszBuffer, pnSize );

   jcUShortPack( (USHORT)pMsg->mobileIdType, ppszBuffer, pnSize );

   switch (pMsg->mobileIdType)
     {
       case CNI_RIL3_IMSI:
       case CNI_RIL3_IMEI:
       case CNI_RIL3_IMEISV:
         jcUShortPack( (USHORT)pMsg->numDigits, ppszBuffer, pnSize );
         jcBufferPack(  (PVOID)pMsg->digits, (USHORT) CNI_RIL3_MAX_ID_DIGITS, ppszBuffer, pnSize );
         break;

       case CNI_RIL3_TMSI:
         jcULongPack( (USHORT)pMsg->tmsi, ppszBuffer, pnSize );
         break;

       default:
         // Don't pack anything
         break;
     }

   return;
}

/*
 * jcNonStdRasMessageIMSIUnpack
 */
void
jcNonStdRasMessageIMSIUnpack( T_CNI_RIL3_IE_MOBILE_ID* pMsg, UCHAR** ppszBuffer )
{
   PVOID  pdata;

   // Unpack boolean from USHORT
   if (jcUShortUnpack( ppszBuffer ) == 1)
      pMsg->ie_present             = TRUE;
   else
      pMsg->ie_present             = FALSE;

   pMsg->mobileIdType = (T_CNI_RIL3_MOBILE_ID_TYPE) jcUShortUnpack( ppszBuffer );

   switch (pMsg->mobileIdType)
     {
       case CNI_RIL3_IMSI:
       case CNI_RIL3_IMEI:
       case CNI_RIL3_IMEISV:
         pMsg->numDigits = jcUShortUnpack( ppszBuffer );
         pdata = jcBufferUnpack( (USHORT) CNI_RIL3_MAX_ID_DIGITS, ppszBuffer );
         memcpy( pMsg->digits, (char *)pdata, CNI_RIL3_MAX_ID_DIGITS );
         break;

       case CNI_RIL3_TMSI:
         pMsg->tmsi = jcULongUnpack( ppszBuffer );
         break;

       default:
         // Don't unpack anything
         break;
     }

   return;
}

/*
 * jcNonStdRasMessagePageRequestPack
 */
void 
jcNonStdRasMessagePageRequestPack( NonStdRasMessagePageRequest_t* pMsg, 
                                   UCHAR** ppszBuffer,
                                   int* pnSize )
{
   jcULongPack(pMsg->req_id, ppszBuffer, pnSize );

   jcNonStdRasMessageIMSIPack( &(pMsg->imsi), ppszBuffer, pnSize );

   jcUShortPack(pMsg->channel_needed, ppszBuffer, pnSize );
   jcULongPack(pMsg->paging_timeout, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessagePageRequestUnpack
 */
void 
jcNonStdRasMessagePageRequestUnpack( NonStdRasMessagePageRequest_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->req_id = (UINT) jcULongUnpack( ppszBuffer );

   jcNonStdRasMessageIMSIUnpack( &(pMsg->imsi), ppszBuffer );

   pMsg->channel_needed = (short) jcUShortUnpack( ppszBuffer );
   pMsg->paging_timeout = (UINT) jcULongUnpack( ppszBuffer );

   return;
}


/*
 * jcNonStdRasMessagePageResponsePack
 */
void 
jcNonStdRasMessagePageResponsePack( NonStdRasMessagePageResponse_t* pMsg, 
                                    UCHAR** ppszBuffer,
                                    int* pnSize )
{
   jcULongPack(pMsg->req_id, ppszBuffer, pnSize );

   // Store boolean in a USHORT
   if (pMsg->need_profile)
      jcUShortPack( (USHORT) 1, ppszBuffer, pnSize );
   else
      jcUShortPack( (USHORT) 0, ppszBuffer, pnSize );

   return;
}

/*
 * jcNonStdRasMessagePageResponseUnpack
 */
void 
jcNonStdRasMessagePageResponseUnpack( NonStdRasMessagePageResponse_t* pMsg, UCHAR** ppszBuffer )
{
   pMsg->req_id = (UINT) jcULongUnpack( ppszBuffer );

   // Unpack boolean from USHORT
   if (jcUShortUnpack( ppszBuffer ) == 1)
      pMsg->need_profile             = TRUE;
   else
      pMsg->need_profile             = FALSE;

   return;
}


/*
 * VBLinkPageRequest
 */
BOOL 
VBLinkPageRequest(NonStdRasMessagePageRequest_t* Msg)
{
   DBG_FUNC("VBLinkPageRequest",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePageRequest, &pszBuffer, &nSize );

   jcNonStdRasMessagePageRequestPack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPageRequest): Sending Page Request to GP10\n}\n");

   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA:\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPageResponse): Unable to send Page Request to GP10, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_LEAVE();
   return (retStatus);
}

/*
 * VBLinkPageResponse
 */
BOOL 
VBLinkPageResponse(NonStdRasMessagePageResponse_t* Msg)
{
   DBG_FUNC("VBLinkPageResponse",VBLINK_LAYER);
   DBG_ENTER();

   int nSize;
   UCHAR buffer[NON_STD_MSG_MAX_LEN];
   UCHAR* pszBuffer;

   char sendBuffer[VBLINK_MAX_MESSAGE_SIZE];
   unsigned short *pActualDataLen;
   STATUS status;
   BOOL retStatus = FALSE;

   pszBuffer = buffer;
   nSize = 0;

   jcUShortPack( NON_STD_RAS_PROTOCOL, &pszBuffer, &nSize );
   jcUShortPack( NonStdRasMessageTypePageResponse, &pszBuffer, &nSize );

   jcNonStdRasMessagePageResponsePack( Msg, &pszBuffer, &nSize );

   DBG_TRACE("{\nMNEThoTRACE(vblink::VBLinkPageResponse): Sending Page Response to GMC\n}\n");

   memset(sendBuffer,0,VBLINK_MAX_MESSAGE_SIZE);
   pActualDataLen = (unsigned short *)sendBuffer;
   *pActualDataLen = htons(nSize);

   memcpy((char*)&sendBuffer[sizeof(short)],buffer,nSize);

   DBG_TRACE("    VBLINK MESSAGE DATA:\n");
   DBG_TRACE("      {\n");
   DBG_TRACE("        \nMsg hexdump: ");
   DBG_HEXDUMP((unsigned char*) &sendBuffer, nSize+2 );
   DBG_TRACE("\n      }\n");

   status = send(VBLinkConfig.VBSocket,sendBuffer,(nSize+2),0);
   if (status == ERROR) {
      DBG_ERROR("{\nMNEThoTRACE(vblink::VBLinkPageResponse): Unable to send Page Response to GMC, %d!\n}\n",errno);
   }
   else {
       retStatus = TRUE;
   }
   DBG_LEAVE();
   return (retStatus);
}
