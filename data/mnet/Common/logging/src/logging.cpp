/*
 ****************************************************************************************
 *                                                                                      *
 *            Copyright Cisco Systems, Inc 2000 All rights reserved                     *
 *                                                                                      *
 *--------------------------------------------------------------------------------------*
 *                                                                                      *
 *    File                : Logging.cpp                                                 *
 *                                                                                      *
 *    Description         : Utility functions which can be used by modules in the       *
 *                          MNET systems     for logging and resource tracking          *
 *                                                                                      *
 *    Author              : Dinesh Nambisan                                             *
 *                                                                                      *
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description                                               *
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/18/99 | DSN    | Initial version created                                   *
 *       |08/14/01 | Igal   | Addded binary hex dump support                            *
 *       |         |        |                                                           *
 ****************************************************************************************
 */
/*
 * VxWorks and ANSI C Headers
 */

#include <assert.h>
#include <vxworks.h>
#include <msgqlib.h>
#include <sockLib.h>
#include <inetLib.h>
#include <stdioLib.h>
#include <strLib.h>
#include <hostLib.h>
#include <ioLib.h>
#include <tasklib.h>
#include <time.h>
#include <tickLib.h>
#include <string.h>
#include <drv\timer\timerdev.h>

/*
 * Application Headers
 */
#include "logmodules.h"
#include "dbgfunc.h"
#include "vclogging.h"


/* Length of the formatted */
#define LOG_FORMATTED_LINE_LENGTH   72
#define LOG_MAX_OVERHEAD_LENGTH     200
#define LOG_BUFFER_LENGTH           ((VIPERLOG_MAX_OUT_BUFF_LENGTH/16+1)*LOG_FORMATTED_LINE_LENGTH+LOG_MAX_OVERHEAD_LENGTH)

static VIPERLOG_CONFIGURATION *viperlog = NULL;

static char logOutputBuff [LOG_BUFFER_LENGTH];


/*
 * init_log_task
 */
void 
init_log_task (void)
{
    struct sockaddr_in local_addr;

    if(viperlog != NULL) {
        printf("ViperLog already initialized!\n");
        return;
    }

    sysTimestampEnable();

    viperlog = (VIPERLOG_CONFIGURATION*)malloc(sizeof(VIPERLOG_CONFIGURATION));
    if (viperlog == NULL) {
        printf("Unable to allocate memory for ViperLog, aborting!\n");
        viperlog = NULL;
        return;
    }

    memset(viperlog,0,sizeof(VIPERLOG_CONFIGURATION));

    viperlog->log_file_opened = FALSE;


    viperlog->log_file = fopen("viperlog.txt","wb");

    if (viperlog->log_file ==NULL) {
        printf("Unable to open log file for writing!!!\n");
        viperlog->log_file_opened = FALSE;
    }  
    else {
        viperlog->log_file_opened = TRUE;
    }

    viperlog->log_record_counter = 0;

    if ((viperlog->log_socket = socket (AF_INET, SOCK_DGRAM, 0)) == ERROR) {  
        perror ("socket");
        printf("Could not create UDP socket for logging!!\n");
        viperlog->log_socket_opened = FALSE;
    }  
      else {
        int optval;
        char dest_addr_str[128];

        viperlog->log_socket_opened = TRUE;

        setsockopt (viperlog->log_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof (optval));
        optval = 32768;
        setsockopt (viperlog->log_socket, SOL_SOCKET, SO_SNDBUF, (char *)&optval, sizeof (optval));


        memset((char *)&viperlog->dest_address, 0,sizeof(struct sockaddr_in));
        viperlog->dest_address.sin_family     = AF_INET;
        viperlog->dest_address.sin_port     = htons(VIPERLOG_PORT_NUM);
        memset(dest_addr_str,0,128);
        strcpy(dest_addr_str,viperlog_get_network_broadcast_addr());
        printf("Setting destination address to %s\n",dest_addr_str);


          viperlog->dest_address.sin_addr.s_addr = inet_addr(dest_addr_str);

        viperlog->msg_q_id = msgQCreate(VIPERLOG_MAX_MSGS_IN_Q,sizeof(VIPERLOG_PACKET),MSG_Q_FIFO);
        if (viperlog->msg_q_id == NULL)
        {
            printf("Unable to create logging message queue; aborting !!\n");
            close(viperlog->log_socket);
            if (viperlog->log_file_opened) fclose(viperlog->log_file);
            free(viperlog);
            viperlog = NULL;
            return;
        }
        printf("Logging initialized successfully\n");
        viperlog->shutdown_message_received = FALSE;
    }  
}


/*
 * shutdown_log_task
 */
void
shutdown_log_task()
{
    viperlog->shutdown_message_received = TRUE;
}


/*
 * viper_log
 */
VOID 
viper_log ()
{
    VIPERLOG_PACKET packet;
    int             status;
    int             offset = 0;
    BOOL            output_file, output_console, output_network;

    memset (&packet, 0, sizeof(VIPERLOG_PACKET));

    while (!viperlog->shutdown_message_received)
    {
        if (msgQReceive (viperlog->msg_q_id,(char *)&packet,sizeof(VIPERLOG_PACKET),WAIT_FOREVER) != ERROR)
        {
            if (packet.magic != VIPERLOG_MSG_MAGIC_NO)
            {
                printf ("Invalid debug msg packet received!!\n");
            }
            else
            {
                // prepare header based on logging parameters
                output_file = output_console = output_network = FALSE;

                viperlog_get_output_device (packet.module_id, &output_file   ,
                                            &output_console , &output_network);

                if (output_console | output_network | output_file)
                {
                    if (packet.data_type == LOG_PLAIN_BINARY)
                    { // processing binary data to the buffer
                        PlainHexDump  ((unsigned char *)packet.message, packet.binary_length, logOutputBuff);
                    }
                    else if (packet.data_type == LOG_FORMAT_BINARY)
                    { // processing binary data to the buffer
                        FormatHexDump ((unsigned char *)packet.message, packet.binary_length, logOutputBuff);
                    }
                    else if (packet.data_type == LOG_ASCII) 
                    {
                        if (viperlog->console_output_detailed == TRUE)
                        { // prepare detailed log
                            sprintf (logOutputBuff, "%s : %s : %ld-%ld : %015.6f : %s : %s\n",
                                viperlog_get_severity_name((VIPERLOG_SEVERITY)packet.severity_level),
                                viperlog_get_module_name  ((LOGGING_MODULE_ID)packet.module_id     ),
                                packet.call_id      ,
                                packet.tick_count   ,
                                packet.timestamp    ,
                                packet.function_name,
                                packet.message      );
                        }
                        else
                            sprintf (logOutputBuff, "%s\n", packet.message);
                    }
                    else
                    {
                        sprintf (logOutputBuff, "\n");
                    }

                    /*
                     * Logging to console
                     */
                    if (output_console)
                    {
                        printf ("%s", logOutputBuff);
                    }

                    /*
                     * Logging to file
                     */
                    if (output_file)
                    {
                        if (viperlog->log_file_opened)
                        {
                            fprintf (viperlog->log_file,"%s", logOutputBuff);
                            fflush  (viperlog->log_file);
                            viperlog->log_record_counter++;

                            if (viperlog->log_record_counter >= VIPERLOG_MAX_RECORDS_IN_FILE)
                            {
                                viperlog->log_record_counter = 0;
                                status = fseek (viperlog->log_file, 0, 0);

                                if (status != 0)
                                {
                                    printf ("Unable to reset log file pointer to beginning of stream\n");
                                    fclose (viperlog->log_file);
                                    viperlog->log_file_opened = FALSE;
                                }
                            }
                        }
                    } /* End of if output_file */

                    /*
                     * Log to network
                     */
                    if (output_network)
                    {
                        viperlog_send_net_msg (logOutputBuff);
                    }
                }
            } /* end of log processing */
        } /* End of if msgQReceive */
    } /* End of while loop */

    msgQDelete(viperlog->msg_q_id);
    close(viperlog->log_socket);
    fclose(viperlog->log_file);
    viperlog->log_file = NULL;
    free(viperlog);
    viperlog = NULL;
    return ;
}


/*
 * viperlog_check_module_filter
 */
BOOL
viperlog_check_module_filter(int module_id, short severity_level)
{
    if (module_id >= LOGGING_MAX_MODULES) {
        printf("Invalid VIPERCELL MODULE specified\n");
        return(FALSE);
    }
    if (viperlog->module_filters[module_id] & severity_level) {
        return(TRUE);
    }
    else {
        return(FALSE);
    }
}


/*
 * viperlog_display_filter_info
 */
void
viperlog_display_filter_info(short filter_value)
{
    char display_enabled_buffer[2048],
         display_disabled_buffer[2048];

    memset(display_enabled_buffer,0,2048);
    memset(display_disabled_buffer,0,2048);

    strcpy(display_enabled_buffer ,"ENABLED  :-");
    strcpy(display_disabled_buffer,"DISABLED :-");

    if (filter_value & LOG_TO_CONSOLE_MASK) {
        strcat(display_enabled_buffer,"Console Logging ");
    }
    else strcat(display_disabled_buffer,"Console Logging ");

    if (filter_value & LOG_TO_NETWORK_MASK) {
        strcat(display_enabled_buffer,"Network Logging ");
    }
    else strcat(display_disabled_buffer,"Network Logging ");

    if (filter_value & LOG_TO_FILE_MASK) {
        strcat(display_enabled_buffer,"File Logging ");
    }
    else strcat(display_disabled_buffer,"File Logging ");

    if (filter_value & LOG_CONTROL_TRACE_MASK) {
        strcat(display_enabled_buffer,"Function Tracing ");
    }
    else strcat(display_disabled_buffer,"Function Tracing ");

    if (filter_value & LOG_CALL_TRACE_MASK) {
        strcat(display_enabled_buffer,"Call Traces ");
    }
    else strcat(display_disabled_buffer,"Call Traces ");

    if (filter_value & LOG_INFORMATIONAL_MASK) {
        strcat(display_enabled_buffer,"Traces ");
    }
    else strcat(display_disabled_buffer,"Traces ");

    if (filter_value & LOG_WARNINGS_MASK) {
        strcat(display_enabled_buffer,"Warnings ");
    }
    else strcat(display_disabled_buffer,"Warnings ");

    if (filter_value & LOG_ERRORS_MASK) {
        strcat(display_enabled_buffer,"Errors ");
    }
    else strcat(display_disabled_buffer,"Errors ");

    if (filter_value & LOG_TIMESTAMPS) {
        strcat(display_enabled_buffer,"Timestamps ");
    }
    else strcat(display_disabled_buffer,"Timestamps ");

    if (strlen(display_enabled_buffer) == strlen("ENABLED :-")){
        strcat(display_enabled_buffer," None ");
    }

    if (strlen(display_disabled_buffer) == strlen("DISABLED :-")) {
        strcat(display_disabled_buffer," None ");
    }
    printf("%s\n",display_enabled_buffer);    
    printf("%s\n",display_disabled_buffer);    
}


/*
 * viperlog_get_output_device
 */
void
viperlog_get_output_device(int module_id,BOOL *output_file,BOOL *output_console,BOOL *output_network)
{
    if (module_id >= LOGGING_MAX_MODULES) {
        printf("Invalid VIPERCELL MODULE specified\n");
        return;
    }
    if (viperlog->module_filters[module_id] & LOG_TO_CONSOLE_MASK) {
        *output_console = TRUE;
    }

    if (viperlog->module_filters[module_id] & LOG_TO_NETWORK_MASK) {
        *output_network = TRUE;
    }
    if (viperlog->module_filters[module_id] & LOG_TO_FILE_MASK) {
        *output_file = TRUE;
    }
    return;    
}



/*
 * viperlog_get_network_broadcast_addr
 */
char *
viperlog_get_network_broadcast_addr()
{
    static char addr_buff[128],interface_name[128];
    int status;

    memset(addr_buff,0,128);
    memset(interface_name,0,128);

    strcpy(interface_name,VIPERLOG_HOST_ETHERNET_INTERFACE);
    status = ifBroadcastGet(interface_name,addr_buff);
    if (status == ERROR)
    {
        printf("Error retrieving interface %s's broadcast address!\n",interface_name);
        return ("");
    }

    return(addr_buff);  
}


/*
 * viperlog_get_severity_name
 */
char *
viperlog_get_severity_name(VIPERLOG_SEVERITY severity)
{
    static char severity_str[64];

    switch(severity){
        case LOG_SEVERITY_FUNCTION_TRACE:strcpy(severity_str,"Function Trace"  );break;
        case LOG_SEVERITY_TRACE         :strcpy(severity_str,"Trace"           );break;
        case LOG_SEVERITY_CALL_TRACE    :strcpy(severity_str,"Call Trace"      );break;
        case LOG_SEVERITY_WARNING       :strcpy(severity_str,"Warning"         );break;
        case LOG_SEVERITY_ERROR         :strcpy(severity_str,"Error"           );break;
        default                         :strcpy(severity_str,"Unknown severity");break;
    }

    return (severity_str);
}


/*
 * viperlog_get_module_name
 */
char *
viperlog_get_module_name(LOGGING_MODULE_ID id)
{
    return(GetLogModuleName(id));
}


/*
 * viperlog_send_net_msg
 */
void
viperlog_send_net_msg(char *msg_buff)
{
    if (!viperlog->log_socket_opened) {
        return;
    }

    int msg_size,gap;

    msg_size = strlen(msg_buff);

    msg_size = (msg_size <= VIPERLOG_MAX_NET_PKT_SIZE) ? msg_size : VIPERLOG_MAX_NET_PKT_SIZE;
        
    /*
     * We just accumulate as much log data as we can squeeze
     * into the single largest packet on ethernet, then out it
     * goes....simple state info retained in the config struct
     * allows us to keep accumulating data
     */

    if (viperlog->net_buff_index == 0)
    {
        strcpy(viperlog->net_packet,msg_buff);
        viperlog->net_buff_index = strlen(msg_buff);
    }
    else
    {
        gap = VIPERLOG_MAX_NET_PKT_SIZE - viperlog->net_buff_index;
        
        if (gap > msg_size)
        {
            strcat(viperlog->net_packet,msg_buff);
            viperlog->net_buff_index += msg_size;
        }
        else
        {
            /* We have to flush data to copy the latest arrival */
            if (sendto(viperlog->log_socket, (char *)viperlog->net_packet, 
                        viperlog->net_buff_index, 0,
                        (struct sockaddr *) &viperlog->dest_address,sizeof(struct sockaddr_in)) == ERROR){  
                perror ("sendto");
            } 
            strcpy(viperlog->net_packet,msg_buff);
            viperlog->net_buff_index = strlen(msg_buff);
        }
    }

    return;
}


/*
 **************************************************************************
 *   Functions which are externally called (from Shell,etc)
 **************************************************************************
 */

/*
 * ViperLogHelp
 */
extern "C" void
ViperLogHelp()
{
    printf("\n******************************************************************************************");
    printf("\n                     VIPERLOG HELP");
    printf("\n******************************************************************************************");
    printf("\nViperLogSetModuleFilters(moduleId,severityMask): Sets the specified severityMask filter");
    printf("\n                                                     for the specified moduleId");
    printf("\nViperLogShowModuleFilters(moduleId)            : Displays the log filters for moduleId");
    printf("\nViperLogShowAllFilters()                       : Displays the log filters for all modules");
    printf("\nViperLogSetDestAddress(ipAddrStr)              : Sets the destination address for logging");
    printf("\nViperLogSetDestPort(portNo)                    : Sets the destination port for logging");
    printf("\nViperLogShowLogModules()                       : Displays all the log modules & their ids");
    printf("\nViperLogShowCount()                            : Displays no of log packets sent so far");
    printf("\nViperLogSetConsolePrintOptions(BOOL On)        : If on displays all information; otherwise");
    printf("\n                                                 displays only message (default)");
    printf("\nViperLogShowSeverityMask()                     : Shows the bitmap for severity values");
    printf("\n******************************************************************************************");
}


/*
 * ViperLogSetModuleFilters
 */
extern "C" void
ViperLogSetModuleFilters(int module_id,short severity_level)
{
    int count;
    if (module_id == VIPERLOG_ALL_MODULES) {
        for(count=0;count<LOGGING_MAX_MODULES;count++) {
            viperlog->module_filters[count] = severity_level;
        }
        printf("For ALL MODULES:\n");
        viperlog_display_filter_info(severity_level);
    }
    else {
        if (module_id >= LOGGING_MAX_MODULES) {
            printf("Invalid module id specified!\n");
            return;
        }
        viperlog->module_filters[module_id] = severity_level;
        printf("For MODULE ID %s:\n",viperlog_get_module_name((LOGGING_MODULE_ID)module_id));
        viperlog_display_filter_info(severity_level);
    }
}


/*
 * ViperLogSetGroupFilters
 */
extern "C" void
ViperLogSetGroupFilters(int module_id_begin,int module_id_end,short severity_level)
{
    int count;
    if (module_id_end == VIPERLOG_ALL_MODULES) {
        for(count=0;count<LOGGING_MAX_MODULES;count++) {
            viperlog->module_filters[count] = severity_level;
        }
        printf("For ALL MODULES:\n");
        viperlog_display_filter_info(severity_level);
    }
    else {
        if (module_id_begin >= LOGGING_MAX_MODULES) {
            printf("Invalid module id specified!\n");
            return;
        }
        printf("For MODULE IDs :-\n");
        for(count=module_id_begin;count<module_id_end;count++) {
            viperlog->module_filters[count] = severity_level;
            printf(" %s ",viperlog_get_module_name((LOGGING_MODULE_ID)count));
        }
        printf("Logging Filters are set as \n ");
        viperlog_display_filter_info(severity_level);
    }
}


/*
 * ViperLogSetConsolePrintOptions
 */
extern "C" void
ViperLogSetConsolePrintOptions(BOOL ConsolePrintFormatted)
{
    if (ConsolePrintFormatted) {
        printf("Turned ON formatting+additional info for console output of logging\n");
        viperlog->console_output_detailed = TRUE;
    }
    else {
        printf("Turned OFF formatting+additional info for console output of logging\n");
        viperlog->console_output_detailed = FALSE;
    }
}




/*
 * ViperLogShowModuleFilters
 */
extern "C" void
ViperLogShowModuleFilters(int module_id)
{
    int filter_value = 0;
    filter_value = viperlog->module_filters[module_id];
    printf("For MODULE ID %s:\n",viperlog_get_module_name((LOGGING_MODULE_ID)module_id));
    viperlog_display_filter_info(filter_value);
}


/*
 * ViperLogSetDestPort
 */
void
ViperLogSetDestPort(int port_no)
{
    char ip_addr_str[25];
    memset(ip_addr_str,0,25);
    viperlog->dest_address.sin_port     = htons(port_no);
    inet_ntoa_b(viperlog->dest_address.sin_addr,ip_addr_str);
    printf("Log address now set to %s, port %d\n",ip_addr_str,port_no);
}


/*
 * ViperLogSetDestAddress
 */
void
ViperLogSetDestAddress(char *ip_address)
{
    char ip_addr_str[25];
    memset(ip_addr_str,0,25);
      viperlog->dest_address.sin_addr.s_addr = inet_addr(ip_address);
    inet_ntoa_b(viperlog->dest_address.sin_addr,ip_addr_str);
    printf("Log address now set to %s, port %d\n",ip_addr_str,
        ntohs(viperlog->dest_address.sin_port));
}


/*
 * ViperLogShowSeverityMask
 */
extern "C" void
ViperLogShowSeverityMask()
{
    printf("\n***************************************************************************************\n");
    printf("  Bit mask for log options values\n");
    printf("    |----|----|----|----|----|----|---|---|---|---|---|---|---|---|---|---|\n");
    printf("    | 15 | 14 | 13 | 12 | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |\n");
    printf("    |----|----|----|----|----|----|---|---|---|---|---|---|---|---|---|---|\n");
    printf("\n");
    printf(" ------------------------------\n");
    printf(" Bit no |  Meaning\n");
    printf(" -----------------------------\n");
    printf("  0     : If set, indicates log to console enabled\n");
    printf("  1     : If set, indicates log to network enabled\n");
    printf("  2     : If set, indicates log to file enabled\n");
    printf("  3     : If set, indicates function tracing required\n");
    printf("  4     : If set, indicates call tracing required\n");
    printf("  5        : If set, indicates logging of informational messages required\n");
    printf("  6     : If set, indicates logging of warnings required\n");
    printf("  7        : if set, indicates logging of errors required\n");
    printf("  8        : if set, indicates use of timestamps\n");
    printf("  9-15  : reserved; unused for now\n");
    printf("***************************************************************************************\n");

}


#define DISPLAY_MODULE_NAME_OFFSET      2
#define DISPLAY_MODULE_ID_OFFSET        27
#define DISPLAY_MODULE_INFO_LENGTH      40

/*
 * ViperLogShowLogModules
 */
extern "C" void
ViperLogShowLogModules()
{
    int count,index;
    char moduleName[128];
    char displayBuffer[256];

    printf("\nThe following modules are using ViperLog");
    printf("\n************************************************");

    memset(displayBuffer,0,256);
    strcpy(&displayBuffer[DISPLAY_MODULE_NAME_OFFSET],"MODULE NAME");
    strcpy(&displayBuffer[DISPLAY_MODULE_ID_OFFSET],"MODULE ID");
    for(index=0;index<DISPLAY_MODULE_INFO_LENGTH;index++)
        if (displayBuffer[index] == 0) displayBuffer[index]= 32;
    displayBuffer[DISPLAY_MODULE_INFO_LENGTH-1] = 0;
    printf("\n%s",displayBuffer);


    printf("\n************************************************");
    for(count=0;count<LOGGING_MAX_MODULES;count++) {
            memset(moduleName,0,128);
            strcpy(moduleName,viperlog_get_module_name((LOGGING_MODULE_ID)count));
            if (strstr(moduleName,"UNKNOWN") != NULL) continue;
            else {
                memset(displayBuffer,0,256);
                sprintf(&displayBuffer[DISPLAY_MODULE_NAME_OFFSET],"%s",moduleName);
                sprintf(&displayBuffer[DISPLAY_MODULE_ID_OFFSET],"%d",count);
                for(index=0;index<DISPLAY_MODULE_INFO_LENGTH;index++)
                    if (displayBuffer[index] == 0) displayBuffer[index]= 32;
                displayBuffer[DISPLAY_MODULE_INFO_LENGTH-1] = 0;
                printf("\n%s",displayBuffer);
            }
    }
    printf("\n************************************************\n");
}


/*
 * ViperLogShowAllFilters
 */
void
ViperLogShowAllFilters()
{
   int count;
   char moduleName[128];
   printf("\n************************************************");
   for(count=0;count<LOGGING_MAX_MODULES;count++) {
        memset(moduleName,0,128);
        strcpy(moduleName,viperlog_get_module_name((LOGGING_MODULE_ID)count));
        if (strcmp(moduleName,"UNKNOWN_LAYER") == 0) continue;
        else {
            ViperLogShowModuleFilters((LOGGING_MODULE_ID)count);
            printf("\n************************************************");
        }
   }
}


/*
 **************************************************************************
 *   DbgOutput class member functions
 **************************************************************************
 */


/*
 * DbgOutput::Enter
 */
void
DbgOutput::Enter()
{
    if (viperlog==NULL) return;
    if (viperlog_check_module_filter(m_log_packet.module_id,LOG_SEVERITY_FUNCTION_TRACE) == TRUE) {
        PreparePacket(LOG_SEVERITY_FUNCTION_TRACE);
        sprintf(m_log_packet.message,"Entering function %s",m_log_packet.function_name);
        msgQSend(viperlog->msg_q_id,(char *)&m_log_packet,sizeof(m_log_packet),NO_WAIT,MSG_PRI_NORMAL);
    }        
}


/*
 * DbgOutput::Leave
 */
void
DbgOutput::Leave()
{
    if (viperlog==NULL) return;
    if (viperlog_check_module_filter(m_log_packet.module_id,LOG_SEVERITY_FUNCTION_TRACE) == TRUE) {
        PreparePacket(LOG_SEVERITY_FUNCTION_TRACE);
        sprintf(m_log_packet.message,"Leaving function %s",m_log_packet.function_name);
        msgQSend(viperlog->msg_q_id,(char *)&m_log_packet,sizeof(m_log_packet),NO_WAIT,MSG_PRI_NORMAL);
    }        
}


/*
 * DbgOutput::Trace
 */
void
DbgOutput::Trace(char *format,...)
{
    if (viperlog==NULL) return;

    if (viperlog_check_module_filter(m_log_packet.module_id,LOG_SEVERITY_TRACE) == TRUE) {
        PreparePacket(LOG_SEVERITY_TRACE);
        va_list argument_list;
        va_start(argument_list,format);
        vsprintf(m_log_packet.message,format,argument_list);
        va_end(argument_list);
        msgQSend(viperlog->msg_q_id,(char *)&m_log_packet,sizeof(m_log_packet),NO_WAIT,MSG_PRI_NORMAL);
    }        
}


/*
 * DbgOutput::Warning
 */
void
DbgOutput::Warning(char *format,...)
{
    if (viperlog==NULL) return;

    if (viperlog_check_module_filter(m_log_packet.module_id,LOG_SEVERITY_WARNING) == TRUE) {
        PreparePacket(LOG_SEVERITY_WARNING);
        va_list argument_list;
        va_start(argument_list,format);
        vsprintf(m_log_packet.message,format,argument_list);
        va_end(argument_list);
        msgQSend(viperlog->msg_q_id,(char *)&m_log_packet,sizeof(m_log_packet),NO_WAIT,MSG_PRI_NORMAL);
    }        
}


/*
 * DbgOutput::Error
 */
void
DbgOutput::Error(char *format,...)
{
    if (viperlog==NULL) return;

    if (viperlog_check_module_filter(m_log_packet.module_id,LOG_SEVERITY_ERROR) == TRUE) 
    {
        PreparePacket(LOG_SEVERITY_ERROR);
        va_list argument_list;
        va_start(argument_list,format);
        vsprintf(m_log_packet.message,format,argument_list);
        va_end(argument_list);
        msgQSend(viperlog->msg_q_id,(char *)&m_log_packet,sizeof(m_log_packet),NO_WAIT,MSG_PRI_NORMAL);
    }        
}


/*
 * DbgOutput::CallTrace
 */
void
DbgOutput::CallTrace(unsigned long call_id,char *format,...)
{
    if (viperlog==NULL) return;

    if (viperlog_check_module_filter(m_log_packet.module_id,LOG_SEVERITY_CALL_TRACE) == TRUE)
    {
        PreparePacket (LOG_SEVERITY_CALL_TRACE);
        va_list argument_list;
        va_start (argument_list, format);
        vsprintf (m_log_packet.message , format,argument_list);
        va_end   (argument_list);
        m_log_packet.call_id = call_id;
        msgQSend (viperlog->msg_q_id,(char *)&m_log_packet,sizeof(m_log_packet),NO_WAIT,MSG_PRI_NORMAL);
    }        
}


/*
 * DbgOutput::HexDump
 */
void
DbgOutput::HexDump (unsigned char *pSrc, int byteCount, bool bFormat)
{
    if (viperlog == NULL || byteCount <= 0) return;

    if (viperlog_check_module_filter (m_log_packet.module_id, LOG_SEVERITY_TRACE) == TRUE)
    {
        int            tmpCount = byteCount;
        unsigned char *pData    = pSrc     ;

        PreparePacket (LOG_SEVERITY_TRACE, byteCount, bFormat);

        while (tmpCount > 0)
        { // take care of overflow
            m_log_packet.binary_length  = (tmpCount < VIPERLOG_MAX_DEBUG_MSG_LEN)
                                        ?  tmpCount : VIPERLOG_MAX_DEBUG_MSG_LEN;

            memcpy   (m_log_packet.message, pData, m_log_packet.binary_length);

            msgQSend (viperlog->msg_q_id     ,
                      (char *)&m_log_packet  ,
                      sizeof(m_log_packet)   ,
                      NO_WAIT, MSG_PRI_NORMAL);

            tmpCount -= VIPERLOG_MAX_DEBUG_MSG_LEN;
            pData    += VIPERLOG_MAX_DEBUG_MSG_LEN;
        }
    }
}


/*
 * DbgOutput::PreparePacket
 */
void
DbgOutput::PreparePacket (short severity_level, int binLength, bool bFormat)
{
    m_log_packet.severity_level   = severity_level            ;
    m_log_packet.magic            = VIPERLOG_MSG_MAGIC_NO     ;
    m_log_packet.timestamp        = sysTimestamp ()           ;
    m_log_packet.tick_count       = tickGet      ()           ;
    m_log_packet.call_id          = VIPERLOG_NULL_CALL_ID     ;
    m_log_packet.binary_length    = binLength                 ;
    m_log_packet.data_type        = (!binLength) ? LOG_ASCII            // ASCII 
                                  : (bFormat   ) ? LOG_FORMAT_BINARY    // Formatted HEX
                                                 : LOG_PLAIN_BINARY ;   // Plain HEX
}

