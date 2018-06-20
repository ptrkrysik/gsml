/****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgpwin32.cpp												*
 *																						*
 *	Description			: Win32 test program for the BSS-GP layer/module				*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |10/10/00 | DSN    | File created												*
 *		 |		   |		|															*
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock.h>
#include "bssgp\bssgp_prot.h"
#include "bssgp\bssgp_util.h"
#include "bssgp\bssgp_decoder.h"
#include "bssgp\bssgp_encoder.h"

#include "bssgp\bssgp_api.h"
#include "bssgp\bssgp_unitdata.h"
#include "ril3\ril3_sm_msg.h"


/* test-tool configuration struct */
typedef struct {

    char            gs_ip_addr[32];
    SOCKET          msg_socket;
    int             msg_num;

} BSSGPWIN32_CONFIG;



/*
 * The Enumeration of the test messages
 */
typedef enum {


    GPRS_MIN_TEST_MSG_NUM            = 0,
    GPRS_TEST_MSG_SCRAP_UL_UNITDATA  = 1,
    GPRS_TEST_ATTACH_REQ_UL_UNITDATA = 2,

    GPRS_MAX_TEST_MSGS               

} GPRS_TEST_MSG;


/*
 * Global variables
 */
unsigned short g_debug_level=0, g_debug_device=1; /* debug flags required by dbgout.cpp */
BSSGPWIN32_CONFIG config;
char     llc_pdu  [1500];


/*
 * Function prototypes
 */
BOOL socklib_open_sock(void);
void socklib_close_sock(void);
void socklib_send_data(UINT8 *msg,UINT16 len);
void socklib_recv_data(UINT8 *msg,UINT16 *len);
BOOL socklib_initialize(void);
void socklib_shutdown(void);
BOOL bssgpwin32_parse_command_line_arguments(int argc, char **argv);
void bssgpwin32_display_usage(void);
BOOL WINAPI ctrl_brk_handler(DWORD dwCtrlType);
void bssgpwin32_set_test_msg(BSSGP_API_MSG *api_msg);


unsigned char LlcFrameUIAttachReq [] = 
{ 0x41, 0xC0, 0x01, 0x08, 0x01, 0x01, 0x88, 0x01, 0x10, 0x07,
  0x05, 0x04, 0x21, 0x43, 0x65, 0xF7, 0x00, 0x11, 0x00, 0x00,
  0x01, 0x01, 0x05, 0x22, 0x63, 0x00, 0x20, 0x00, 0xB4, 0x5D, 
  0x54    
};

main(int argc, char **argv)
{

    BOOL status;
    BSSGP_API_MSG api_msg;
    UINT16 msg_len=0;
    BSSGP_BVCI bvci;
    BSSGP_NETWORK_QOS_LEVEL qos_reqd;
    BOOL tx_msg=FALSE;
    UINT8 tx_buff[1024], rx_buff[1024];

    if (!bssgpwin32_parse_command_line_arguments(argc,argv)) {
        printf("Invalid arguments!\n");
        bssgpwin32_display_usage();
        return(0);
    }

	if (!(SetConsoleCtrlHandler(ctrl_brk_handler,TRUE))) {
        printf("Cannot set ctrl-break handler,aborting\n");
        return(0);
    }

    if (!socklib_initialize()) {
        printf("Unable to initialize Sockets library\n");
        return(0);
    }

    if (!socklib_open_sock()) {
        printf("Unable to create a socket\n");
        socklib_shutdown();
        return(0);
    }


    memset(&api_msg,0,sizeof(api_msg));

    bssgpwin32_set_test_msg(&api_msg);


    status = bssgp_encode_msg(&api_msg,tx_buff,&msg_len,
                &bvci,&qos_reqd, &tx_msg);

    if (!status) {
        printf("Error encoding BSSGP message!\n");
        socklib_close_sock();
        socklib_shutdown();
        return(0);
    }

    socklib_send_data((UINT8*)tx_buff,msg_len);

    msg_len = 1024;

    socklib_recv_data((UINT8*)rx_buff,&msg_len);


    return (0);

}




/*
 * socklib_open_sock
 */
BOOL
socklib_open_sock()
{
    SOCKET conn_socket;
    int status;
    struct sockaddr_in local_addr, gs_addr;

    conn_socket = socket(AF_INET,SOCK_DGRAM,0);
    if (conn_socket == INVALID_SOCKET) {
        printf("Unable to create a UDP socket, error %d\n",WSAGetLastError());
        return(FALSE);
    }

    memset(&local_addr,0,sizeof(local_addr));
    local_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    local_addr.sin_port             = htons(24240);
    local_addr.sin_family           = AF_INET;

    status = bind(conn_socket,(struct sockaddr*)&local_addr,sizeof(local_addr));
    if (status == SOCKET_ERROR) {
        printf("Unable to bind UDP socket, error %d\n",WSAGetLastError());
        closesocket(conn_socket);
        return(FALSE);
    }

    memset(&gs_addr,0,sizeof(gs_addr));
    gs_addr.sin_addr.S_un.S_addr = inet_addr(config.gs_ip_addr);
    gs_addr.sin_port             = htons(24240);
    gs_addr.sin_family           = AF_INET;
    
    status = connect(conn_socket,(struct sockaddr*)&gs_addr,sizeof(gs_addr));
    if (status == SOCKET_ERROR) {
        printf("Unable to connect UDP socket, error %d\n",WSAGetLastError());
        closesocket(conn_socket);
        return(FALSE);
    }
    printf("Setup IP path to GS server successfully\n");
    config.msg_socket = conn_socket;
    return (TRUE);

}


/*
 * socklib_close_sock
 */
void
socklib_close_sock()
{
    closesocket(config.msg_socket);
}


/*
 * socklib_send_data
 */
void
socklib_send_data(UINT8 *msg,UINT16 len)
{
    int status;

    status = send(config.msg_socket,(char*)msg,len,0);
    if (status == SOCKET_ERROR) {
        printf("Unable to send data on UDP socket, error %d\n",WSAGetLastError());
    }
    else {
        printf("Sent %d bytes of data on UDP socket\n",len);
    }
    return;
}



/*
 * socklib_recv_data
 */
void
socklib_recv_data(UINT8 *msg,UINT16 *len)
{
    int status;
    BOOL free_buff=TRUE;

    status = recv(config.msg_socket,(char*)msg,*len,0);
    if (status == SOCKET_ERROR) {
        printf("Unable to recv data on UDP socket, error %d\n",WSAGetLastError());
    }
    else {
        printf("Received %d bytes of data on UDP socket;running it through BSSGP decoder\n",status);

        if (!bssgp_decode_msg(msg,status,TRUE,(BSSGP_NETWORK_QOS_LEVEL)0,0,&free_buff)) {
            printf("Error decoding BSSGP msg\n");
        }

        *len = status;
    }
    return;
}





/*
 * socklib_initialize
 */
BOOL
socklib_initialize()
{
    BOOL status = FALSE;
   	WORD wVersionRequested;
	WSADATA wsaData;
    int ret_status;
	wVersionRequested = MAKEWORD( 2, 2 ); 
	
	ret_status = WSAStartup( wVersionRequested, &wsaData );
	if (ret_status != 0) {
        status = FALSE;
    }
    else {
        status = TRUE;
    }
    return(status);
}



/*
 * socklib_shutdown
 */
void
socklib_shutdown()
{
	WSACleanup();
    return;
}


/*
 * bssgpwin32_parse_command_line_arguments
 */
BOOL
bssgpwin32_parse_command_line_arguments(int argc, char **argv)
{
	BOOL satisfied=FALSE;
	BOOL ret_status = FALSE, msg_num_specified=FALSE,gs_ip_specified=FALSE;
	int counter=1;
	if (argc < 2) {
		printf("Not enough arguments\n");
		return ret_status;
	}

	while (!satisfied) {
		if (stricmp(argv[counter],"-gs") == 0) {
			if (argc < (counter+2)) goto bye_bye_jose;
			strcpy(config.gs_ip_addr,argv[counter+1]);
            gs_ip_specified=TRUE;
			counter+=2;
		}
		else if (stricmp(argv[counter],"-msg") == 0) {
				if (argc < (counter+2)) goto bye_bye_jose;
				config.msg_num = atoi(argv[counter+1]);

                if ( (config.msg_num <= GPRS_MIN_TEST_MSG_NUM) || (config.msg_num >= GPRS_MAX_TEST_MSGS)) {
                    printf("Unknown/Invalid message template # specified\n");
                    return (FALSE);
                }
				counter+=2;
                msg_num_specified=TRUE;
		}
		else {
			bye_bye_jose:printf("Invalid arguments!\n");
			memset(&config,0,sizeof(config));
			return ret_status;
		}

		if (counter == argc) satisfied = TRUE;
	}
    if (!gs_ip_specified) {
        printf("Need to specify the GS ip address\n");
        return (ret_status);
    }

    if (!msg_num_specified) {
        printf("Need to specify the Message template number\n");
        return (ret_status);
    }
	if (satisfied) ret_status = TRUE;

	return (ret_status);
}



/*
 *  bssgpwin32_display_usage
 */
void
bssgpwin32_display_usage()
{
	printf("\n");
	printf("|*************** BSSGPWin32 Version 1.0 (c) Cisco 2000-2001  ***************************|\n");
	printf("|---------------------------------------------------------------------------------------|\n");
	printf("|>bssgp  -gs <a.b.c.d>  -msg x                                                          |\n");
	printf("|---------------------------------------------------------------------------------------|\n");
	printf("|       -gs <a.b.c.d>   : Use a.b.c.d as the GS IP address                              |\n");
	printf("|       -msg x          : Send pre-defined message x                                    |\n");
	printf("|---------------------------------------------------------------------------------------|\n");
	printf("\n");
}


/*
 * ctrl_brk_handler
 */
BOOL WINAPI 
ctrl_brk_handler(DWORD dwCtrlType)
{
	printf("Ctrl-Break keys presses, exiting application\n");
    closesocket(config.msg_socket);
    return(TRUE);
}















/*
 * bssgpwin32_set_test_msg
 *
 * This function to be modified to add new test messages...
 *
 *
 *
 */
void
bssgpwin32_set_test_msg(BSSGP_API_MSG *api_msg)
{
    unsigned frameLength;

    /* 
     * Most of these values are pretty much going to be the same
     * for most test msgs, except the GMM<->GMM signalling msgs,
     * and the person creating those test msgs needs to contact
     * me first : Dinesh 
     */
    api_msg->magic = BSSGP_MAGIC;
    api_msg->msg_type = BSSGP_API_MSG_UL_UNITDATA;
    api_msg->msg.ul_unitdata.bvci                                = 0;
    api_msg->msg.ul_unitdata.cell_id.ie_present                  = TRUE;
    api_msg->msg.ul_unitdata.cell_id.value                       = 21; /* dunno, some random cell id */
    api_msg->msg.ul_unitdata.qos_profile.ie_present              = TRUE;
    api_msg->msg.ul_unitdata.qos_profile.bucket_leak_rate[0]     = 1;
    api_msg->msg.ul_unitdata.qos_profile.bucket_leak_rate[1]     = 1;
    api_msg->msg.ul_unitdata.qos_profile.precedence_class        = CNI_RIL3_QOS_PRECEDENCE_CLASS_HIGH_PRIORITY;
    api_msg->msg.ul_unitdata.qos_profile.a_bit                   = RADIO_INT_USES_RLC_MAX_UDT;
    api_msg->msg.ul_unitdata.qos_profile.t_bit                   = SDU_CONTAINS_SIGNALLING;
    api_msg->msg.ul_unitdata.qos_profile.cr_bit                  = SDU_LLC_ACK_SACK_ABSENT;

    api_msg->msg.ul_unitdata.tlli                                = 1; /* for lack of a better choice ;-)  */



    /*
     *  Here is the part where you actually stick in the upper layer
     *  message (handcoded or otherwise) as a byte-stream....
     *  You also need to add your message template number to the enumeration
     *  at the beginning of this file called GPRS_TEST_MSG, and add that option
     *  to the switch statement below...
     */

    switch(config.msg_num) {

        case GPRS_TEST_MSG_SCRAP_UL_UNITDATA: {
            printf("Sending pre-defined message template #1 : Scrap UL-Unitdata\n");
            memset(llc_pdu,'A',256);
            api_msg->msg.ul_unitdata.pdu.data   = (UINT8*)llc_pdu;
            api_msg->msg.ul_unitdata.pdu.data_len = 256;

        }
        break;

        case GPRS_TEST_ATTACH_REQ_UL_UNITDATA:

            memset (llc_pdu, 0, 1500);
            frameLength = (sizeof(LlcFrameUIAttachReq)/sizeof(LlcFrameUIAttachReq[0]));

            printf ("Sending pre-defined message template #2: Attach Request (Frame length %d)\n", 
                    frameLength);

            memcpy (llc_pdu, LlcFrameUIAttachReq, frameLength);

            api_msg->msg.ul_unitdata.pdu.data   = (UINT8*)llc_pdu;
            api_msg->msg.ul_unitdata.pdu.data_len = frameLength;

            DbgHexDump ((unsigned char*)api_msg->msg.ul_unitdata.pdu.data, api_msg->msg.ul_unitdata.pdu.data_len);

            break;

        default: {
            printf("Unknown message template # specified !!\n");
            socklib_close_sock();
            socklib_shutdown();
            exit(0);
        }
    } /* End of switch */
}



/*
 * bssgp_api_send_ul_unitdata
 */
void
bssgp_api_send_ul_unitdata(TLLI tlli,T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 *pdu_data,UINT16 pdu_len)
{

    printf("Do something here ;-) \n");    
}





/*
 * bssgp_util_get_rx_buff
 */
char *
bssgp_util_get_rx_buff(int buff_size_reqd)
{
    char *return_buff=NULL;
    return_buff = (char*)malloc(buff_size_reqd);
    return (return_buff);
}


/*
 * bssgp_util_return_rx_buff
 */
void 
bssgp_util_return_rx_buff(char *buff)
{
    free(buff);
}




/*
 * llcwin32_pdu_handler
 */
void 
llcwin32_pdu_handler(LLC_PDU pdu)
{
    int count,index=0;
    printf("LLC PDU of length %d\n",pdu.data_len);

    DbgHexDump ((unsigned char*)pdu.data, pdu.data_len);

    printf("End of LLC PDU\n");    
}
