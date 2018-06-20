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
/* Description : This file contains interface defination between     */
/*               Alarm Source (GP, GMC, GS) and Alarm Monitoring     */
/*               Server (APM1)                                       */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |07/14/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/* Bhawani  |12/05/00| Added UDP server and client port so that the  */
/*          |        | server and clients can reside on the same IP  */
/* Bhawani  |12/05/00| swaped the server and client port to make the */
/*          |        | original port as the server port.             */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#if !defined(_ALARM_INTERFACE_H_)
#define _ALARM_INTERFACE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define ALARM_viperCellNameMaxSize     41
#define ALARM_viperCellIpMaxSize       16
#define ALARM_unknownAlarmId           0x00

/* Alarm Server's IP address  is Gate Keeper's IP address */

/* Alarm Client/Server's  Universal UPD ports */
#define ALARM_interfaceUdpPortClient    12791
#define ALARM_interfaceUdpPortServer    11788   

#define ALARM_periodicUpdateInterval   300    /* seconds */    


/* Alarm Source Type */
#define ALARM_srcTypViperCell          128
#define ALARM_srcTypViperBase          129


/* Alarm Message Type */
#define ALARM_msgTypSummaryUpdate	1
#define ALARM_msgTypAlarmEvent		2

typedef char            sbyte1;
typedef short           sbyte2;
typedef long            sbyte4;

typedef unsigned char    ubyte1;
typedef unsigned short   ubyte2;
typedef unsigned long    ubyte4;

/* Message header */
typedef struct AlarmInterfaceMsgHeader_{
    ubyte1  alarmSrcType;
    ubyte1  alarmMsgType;
    ubyte2  alarmMsgLen;
}AlarmInterfaceMsgHeader;

#define ALARM_interfaceMsgHdrLen    4

/* Summary update data */
typedef struct  AlarmSummaryUpdateData_{
    ubyte4 alarmSrcIp;
    sbyte1 alarmSrcName[ALARM_viperCellNameMaxSize];
    ubyte1 alarmCriticalCount;
    ubyte1 alarmMajorCount;
    ubyte1 alarmMinorCount;
}AlarmSummaryUpdateData;

#define ALARM_summaryUpdateDataLen  (7 + ALARM_viperCellNameMaxSize)
#define ALARM_summaryUpdateMsgLen   (ALARM_interfaceMsgHdrLen + ALARM_summaryUpdateDataLen + sizeof(long))

#define ALARM_eventDataLen			(28 + ALARM_viperCellNameMaxSize)
#define ALARM_eventMsgLen			(ALARM_interfaceMsgHdrLen + ALARM_eventDataLen + sizeof(long))


/* Stubs for new event data */
typedef struct AlarmEventData_ {
    ubyte4  alarmId;
    ubyte4  alarmCode;
    ubyte4  alarmTimestamp;
    ubyte4  alarmSrcIp;
    sbyte1  alarmSrcModuleName[ALARM_viperCellNameMaxSize];
    ubyte4  alarmSrcModuleId;
    ubyte4	opt1;
    ubyte4	opt2;
}AlarmEventData;
  

/* Alarm Complete message */
typedef long Align;
typedef struct Alarm 
{
    AlarmInterfaceMsgHeader hdr;
    union {
        AlarmSummaryUpdateData summaryUpdateData;
        AlarmEventData eventData;
    }body;
	Align x;
}AlarmViperCellToAmsMsg;


#if defined(__cplusplus)
}
#endif

#endif /* _ALARM_INTERFACE_H_ */
