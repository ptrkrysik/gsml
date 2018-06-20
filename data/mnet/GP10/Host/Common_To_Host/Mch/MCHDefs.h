#ifndef MCHDefs_H
#define MCHDefs_H

/* *******************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/* *******************************************************************/

/* *******************************************************************/

/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : MCHDefs.h                                           */
/* Author(s)   : Lou Travaglione                                     */
/* Create Date : 6/16/00                                             */
/* Description :                                                     */

/* *******************************************************************/

#define MCH_MAX_COMMAND_LINE_LEN    256
#define MCH_MAX_COMMAND_LEN         80
#define MAX_DIAGNOSTIC_MSG          256

/* Valid functions codes recognized by the maintenance command handler. */
typedef enum {
    MCH_NETWORK_REQ = 0x55550000,
    MCH_TERMINAL_REQ,
    MCH_DELETE_DIAGNOSTIC,
    MCH_DIAG_SPECIFIC_MSG
} MCHFuncType;


/* These are the diagnostic test messages.  These are processed in the                */
/* DiagnosticMsgLoop function.  If a derived diagnostic wants to process additional   */
/* messages of its own it should define the function codes here.  NOTE: DO NOT        */
/* SHARE FUNCTION IDS BETWEEN DIAGNOSTICS EXCEPT FOR COMMON FUNCTIONS!!!!!            */
enum DIAG_TEST_MSGS {DIAG_TEST_STOP_TEST     = 0x000,    /* Common function ids       */
                     DIAG_TEST_INTERNAL_STOP_TEST,
                     
                     CDC_DIAG_FUNC_IDS       = 0x100,
                     I2C_LOOPBACK_CONTINUE,
                     DSP_MEM_TEST_CONTINUE,
                     
                     DSP_DIAG_FUNC_IDS       = 0x200,
                     DIAG_MS_REPORT,
                     DIAG_HO_PN_REPORT,
                     DIAG_HO_CAND_REPORT,
                     DIAG_DL_REPORT,
                     DIAG_DSP_ECHO_MSG,
                     HPI_ECHO_TEST_CONTINUE,
                     DSP_TONE_GENERATE_RSP,
                     DSP_TONE_MEASURE_RSP,
                     DSP_BBLB_BOOTUP_RSP,
                     DSP_EXTMEM_BOOTUP_RSP,
                     DSP_EXTMEM_TEST_RSP,
                     DSP_EXTMEM_ERROR_RSP,
                     DIAG_IF_REPORT,
                     DIAG_GPRS_MS_REPORT,
                     
                     RF_DIAG_FUNC_IDS        = 0x300,
                     
                     CLK_DIAG_FUNC_IDS       = 0x400,
                     
                     GPS_DIAG_FUNC_IDS       = 0x500,
                     
                     SYSTEM_DIAG_FUNC_IDS    = 0x600,
                     NETWORK_TEST_CONTINUE
                     };





typedef struct
{
   int func;
   int msgLen;
   char msgBody[MAX_DIAGNOSTIC_MSG];
} DiagnosticMsgType;


#endif 