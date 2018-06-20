#ifndef VCLOGGING_HDR_INC
#define VCLOGGING_HDR_INC
/*
 ****************************************************************************************
 *    Copyright Cisco Systems, Inc 2000 All Rights reserved                                *
 *                                                                                        *
 *    File                : vclogging.h                                                    *
 *                                                                                        *
 *    Description            : Header file containing data structures, definitions, and        *
 *                          macros defined for logging use on the MNET solution             *
 *                                                                                        *
 *    Author                : Dinesh Nambisan                                                *
 *                                                                                        *
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description                                                 *
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/18/99 | DSN    | Initial version created                                    *
 *         |           |        |                                                            *
 ****************************************************************************************
 */

/*
 * VxWorks & ANSI C headers
 */
#include <vxworks.h>
#include <msgqlib.h>
#include <sockLib.h>
#include <hostlib.h>
#include <inetLib.h>
#include <stdioLib.h>
#include <strLib.h>
#include <ioLib.h>
#include <assert.h>
/* 
 * Module enumeration header file
 */
#include "logmodules.h"

#define _MNET_COMMON_LOGGING_MODULE_

#ifdef MNET_GP10
#define VIPERLOG_HOST_ETHERNET_INTERFACE "motfec0"
#elif MNET_GS
#define VIPERLOG_HOST_ETHERNET_INTERFACE "fei0"
#elif MNET_GMC
#define VIPERLOG_HOST_ETHERNET_INTERFACE "fei0"
#endif

#define ASCII_PRINTABLE_BEGIN           32
#define ASCII_PRINTABLE_END             126
#define ASCII_SPACE_CHAR                32

#define VIPERLOG_PORT_NUM               2105
#define VIPERLOG_MAX_NET_PKT_SIZE       1200
#define MAX_DEBUG_MSGS_IN_Q             3000
#define VIPERLOG_MSG_MAGIC_NO           0x1234

#define VIPERLOG_ALL_MODULES            1000
#define MAX_VIPERLOG_BUFFERED_PKTS      3

#define VIPERLOG_MAX_FUNC_NAME_LEN      64
#define VIPERLOG_MAX_DEBUG_MSG_LEN      256

#define MAX_SHORT_STRING_LENGTH         7
#define MAX_LONG_STRING_LENGTH          20
#define VIPERLOG_NULL_CALL_ID           0


#define VIPERLOG_MAX_RECORDS_IN_FILE    10000
#define VIPERLOG_MAX_MSGS_IN_Q          1000
#define VIPERLOG_MAX_OUT_BUFF_LENGTH    256


/*
  Bit mask for log options values

    |----|----|----|----|----|----|---|---|---|---|---|---|---|---|---|---|
    | 15 | 14 | 13 | 12 | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
    |----|----|----|----|----|----|---|---|---|---|---|---|---|---|---|---|

 ------------------------------
 Bit no |  Meaning
 -----------------------------
  0     : If set, indicates log to console enabled
  1     : If set, indicates log to network enabled
  2     : If set, indicates log to file enabled
  3     : If set, indicates function tracing required
  4     : If set, indicates call tracing required
  5    : If set, indicates logging of informational messages required
  6     : If set, indicates logging of warnings required
  7    : if set, indicates logging of errors required
  8    : if set, indicates use of timestamps
  9-15  : reserved; unused for now
*/

typedef enum {

    LOG_TO_CONSOLE_MASK     = 0x0001,
    LOG_TO_NETWORK_MASK     = 0x0002,
    LOG_TO_FILE_MASK        = 0x0004,
    LOG_CONTROL_TRACE_MASK  = 0x0008,
    LOG_CALL_TRACE_MASK     = 0x0010,
    LOG_INFORMATIONAL_MASK  = 0x0020,
    LOG_WARNINGS_MASK       = 0x0040,
    LOG_ERRORS_MASK         = 0x0080,
    LOG_TIMESTAMPS          = 0x0100

} VIPERLOG_FILTER_MASK;


/*
 * Inverse masks
 */
typedef enum {

    LOG_CONTROL_TRACE_INVERSE_MASK      = 0xfff7,
    LOG_CALL_TRACE_INVERSE_MASK         = 0xffef,
    LOG_INFORMATIONAL_INVERSE_MASK      = 0xffdf,
    LOG_WARNINGS_INVERSE_MASK           = 0xffbf,
    LOG_ERRORS_INVERSE_MASK             = 0xff7f,

} VIPERLOG_FILTER_INVERSE_MASK;


typedef enum {

    LOG_SEVERITY_FUNCTION_TRACE = LOG_CONTROL_TRACE_MASK,
    LOG_SEVERITY_TRACE          = LOG_INFORMATIONAL_MASK,
    LOG_SEVERITY_CALL_TRACE     = LOG_CALL_TRACE_MASK,
    LOG_SEVERITY_WARNING        = LOG_WARNINGS_MASK,
    LOG_SEVERITY_ERROR          = LOG_ERRORS_MASK

} VIPERLOG_SEVERITY;


#define LOG_ASCII           0
#define LOG_PLAIN_BINARY    1
#define LOG_FORMAT_BINARY   2


/*
 * the packet format for log messages
 */
typedef struct {

    unsigned short  magic         ;
    unsigned short  severity_level;
    unsigned short  module_id     ;
    unsigned long   call_id       ;
    unsigned long   timestamp     ;
    unsigned long   tick_count    ;
    unsigned        data_type     ;
    unsigned        binary_length ;
    char            function_name[VIPERLOG_MAX_FUNC_NAME_LEN];
    char            message      [VIPERLOG_MAX_DEBUG_MSG_LEN];

} VIPERLOG_PACKET;


/*
 * The configuration information
 */
typedef struct {

    unsigned short  module_filters[LOGGING_MAX_MODULES];
    MSG_Q_ID        msg_q_id;
    int             log_socket;
    BOOL            log_socket_opened;

    FILE           *log_file;
    BOOL            log_file_opened;
    unsigned long   log_record_counter;
    unsigned long   log_pkt_counter;

    char            net_packet[VIPERLOG_MAX_NET_PKT_SIZE];
    int             net_buff_index;

    struct          sockaddr_in dest_address;

    BOOL            shutdown_message_received;

    BOOL            console_output_detailed;

} VIPERLOG_CONFIGURATION,*PVIPERLOG_CONFIGURATION;




/*
 * The DbgOutput class
 */
class DbgOutput
{
    private:

        VIPERLOG_PACKET m_log_packet;

        void PreparePacket (short severity_level, int binLength = 0, bool bFormat = false);

    public:

        DbgOutput() {};

        DbgOutput(char *function_name,int module_id)
        { 
            strncpy((char *)m_log_packet.function_name,(const char *)function_name,
                    VIPERLOG_MAX_FUNC_NAME_LEN);
            /* Making sure it is null-terminated */
            m_log_packet.function_name[VIPERLOG_MAX_FUNC_NAME_LEN-1] = 0;
            m_log_packet.module_id = module_id;
        };

        void Func(char *function_name,int module_id)
        { 
            strncpy((char *)m_log_packet.function_name,(const char *)function_name,
                    VIPERLOG_MAX_FUNC_NAME_LEN);
            /* Making sure it is null-terminated */
            m_log_packet.function_name[VIPERLOG_MAX_FUNC_NAME_LEN-1] = 0;
            m_log_packet.module_id = module_id;
        };
        void Enter    (void);
        void Leave    (void);
        void Trace    (char *format, ...);
        void Warning  (char *format, ...);
        void Error    (char *format, ...);
        void CallTrace(unsigned long call_id  , char *format,...);
        void HexDump  (unsigned char *StartPtr, int   byteCount , bool bFormat = false);

        /* Changes for new logging function */
        void SetModuleId(int module_id)
        {
            m_log_packet.module_id = module_id;
        }

        void SetFuncName(char *function_name)
        {
            strcpy((char *)m_log_packet.function_name, (const char *)function_name);
        }

};


#define DBG_FUNC(x,y);          DbgOutput Dbg(x,y);
#define DBG_ENTER();            Dbg.Enter();
#define DBG_LEAVE();            Dbg.Leave();
#define DBG_TRACE               Dbg.Trace
#define DBG_CALL_TRACE          Dbg.CallTrace
#define DBG_WARNING             Dbg.Warning
#define DBG_ERROR               Dbg.Error
#define DBG_HEXDUMP(x,y);       Dbg.HexDump(x,y);
#define DBG_HEXDUMP_FMT(x,y);   Dbg.HexDump(x,y,true);


#ifdef DBG
#define ASSERT            assert
#endif

#ifdef DEBUG
#define ASSERT            assert
#endif


void init_log_task               (void);
void shutdown_log_task           (void);
VOID viper_log                   (void);
BOOL viperlog_check_module_filter(int module_id, short severity_level);
void viperlog_display_filter_info(short filter_value);
void viperlog_get_output_device  (int module_id,BOOL *output_file,BOOL *output_console,BOOL *output_network);
void viperlog_send_net_msg       (char *msg_buff);
char *viperlog_get_severity_name (VIPERLOG_SEVERITY severity);
char *viperlog_get_module_name   (LOGGING_MODULE_ID module_id);
char *viperlog_get_network_broadcast_addr(void);


extern "C" 
{
  void ViperLogSetModuleFilters (int module_id,short severity_level);
  void ViperLogShowAllFilters();
  void ViperLogSetDestPort(int port_no);
  void ViperLogSetDestAddress(char *ip_address);
}

#endif /* #ifndef VCLOGGING_HDR_INC */ 