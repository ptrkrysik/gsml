/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: diagtypes.h
*
* Description:
*   This file contains defined constants, enumerations and type definitions
*   of shared items to support diagnostics and error reporting.
*
******************************************************************************/
#ifndef _DIAGTYPES_H_
#define _DIAGTYPES_H_  /* include once only */

#include "dsp/dsptypes.h"
#include "gsmtypes.h"
                          
#define BURST_CAPTURE_SIZE       156           
#define SAMPLES_PER_CAPTURE_MSG   12           
#define NUM_CAPTURE_MSGS          13           
                                                    
/*-------------------------------------------------------------
 * MS Report structure -- allocate one per TS + one per SDCCH
 *------------------------------------------------------------*/ 
typedef struct {
   Uint32   cumErrs;
   Uint32   cumBits;
   Uint16   cumFrameErrs;
   Uint16   cumFrames;
   Uint8    sacchCount;
   Int8     minToa;
   Int8     maxToa;
   Int8     cumToa;
} t_diagMsReport;
      

/*-------------------------------------------------------------
 * GPRS MS Report structure -- allocate one per USF
 *------------------------------------------------------------*/ 
typedef struct {
   Uint32   cumErrs;
   Uint32   cumBits;
   Uint16   cumFrameErrs;
   Uint16   cumFrames;
   Uint8    rlcBlkCount;
   Int8     minToa;
   Int8     maxToa;
   Int8     cumToa;
} t_diagGprsMsReport;
                 
/*-------------------------------------------------------------
 * Burst Capture State. Armed by Burst Capture command.
 *------------------------------------------------------------*/ 
typedef enum {
   CAPTURE_OFF = 0,
   CAPTURE_ARMED,
   CAPTURE_DONE
} t_captureState;

/*-------------------------------------------------------------
 * Burst Capture structure -- one RF/TS per Host command
 *------------------------------------------------------------*/ 
typedef struct {
   t_captureState state;
   Uint8          rfCh;
   Uint8          timeSlot;
   Uint32         data[BURST_CAPTURE_SIZE];
} t_diagBurstCapture;

/*-------------------------------------------------------------
 * Diagnostic structure 
 *------------------------------------------------------------*/ 
typedef struct {
   Int16    enable;           /* Makes sure we send only once per SF */
   
   Uint8    msReportTsMask;   /* Enables MS (uplink) Reports, Bits 7...0 control TS7...TS0 */
   Uint8    dcchReportOnly;   /* Report on DCCH only (ie, not TCH)  */
   Uint8    hoReportTsMask;      /* Enables HO reports by TS, Bits 7...0 control TS7...TS0 */
   Uint8    dlMeasReportTsMask;  /* Enables DL Measurement reports by TS, same bit map */

   Uint32   gprsMsReportTfiMask;      /* Enables GPRS MS (uplink) Reports by TFI, bit mapped */
   Uint32   gprsDlMeasReportTfiMask;  /* Enables GPRS DL Measurement (Ack/Nack) Reports by TFI */

   Uint8    dcOffsetTsMask;      /* Enables DC Offset calculation by TS */
   Uint8    dcOffsetReport;      /* Enables DC Offset report (T/F) */
   Uint8    sendRadioLinkLost;
   Uint8    sendIdleFrameCapture;
   t_diagBurstCapture  burstCapture;
   /*
   *  Note on msReport: The first NUM_TN_PER_RF structures per RF are 
   *  for TS0-TS7. The next NUM_SDCCH_SUBCHANS structures are for SDCCH.
   *  The zero index structure, msReport[rfCh][0], is actually unused.
   */
   t_diagMsReport          msReport[NUM_RFS_PER_DSP][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS];
   t_diagGprsMsReport  gprsMsReport[NUM_RFS_PER_DSP][MAX_TFI];
   t_diagMsReport  sdcch8Report[NUM_RFS_PER_DSP][NUM_SDCCH8_SUBCHANS];
} t_diagData;
       
/*-------------------------------------------------------------
 * Loop back signal uplink-to-downlink or vice versa  
 *------------------------------------------------------------*/ 
typedef enum { 
   NO_LOOPBACK = 0,        /* normal operation */
   LOCAL_LOOPBACK = 1,     /* loopback on EVM card */
   BASEBAND_LOOPBACK = 2,  /* loopback on analog I/Q or RF */
   DSP_IQ_LOOPBACK = 3,    /* loopback on DSP RCV I/Q to TX I/Q with interpolation by 2 */
   DSP_TCH_LOOPBACK = 4,	/* loopback on DSP TCH frames from RCV to TX */   
   DSP_TCH_TO_HOST_LOOPBACK = 10    /* loopback on DSP TCH frames from TX to RCV*/    
} loopbackMode;

/*-------------------------------------------------------------
 * For saving I/Q samples of idle bursts  
 *------------------------------------------------------------*/ 
typedef struct
{
   Int16  rcvPower;  /* Uplink Signal Power Level*/
   Int32  data[156];
} idleProcStruc;


#endif  /* end of include once only */
                              
