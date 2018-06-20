/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/* *******************************************************************/

/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : l1proxy.h                                           */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 9/9/99                                              */
/* Description :                                                     */

/* *******************************************************************/

#ifndef l1proxy_H
#define l1proxy_H

#include "msgQLib.h"
#include <semLib.h>

extern SEM_ID      dspLoadSem;

void initInt(void);
void go(unsigned int port);
void initDsp(unsigned int port);

typedef enum {
   L1PROXY_LOG_ERROR,
   L1PROXY_LOG_WARNING,
   L1PROXY_LOG_TRACE
}l1ProxyLogType;


#define DSP_TIMEOUT  500                 /* DSP ready time out for 2 ms. */

#ifndef STAND_ALONE
int ulMobileMessagePreProc(unsigned char *msg);
#endif

#define NUM_DSPS         2               /* Number of DSPs */

#define DSPMSGLENMAX     64              /* max len downstream messages      */
#define DSPBUFFERMAX     0x204           /* max HPI buffer size              */
#define DSPUPBUFFER      0x80000000      /* HPI upstream buffer              */
#define DSPDNBUFFER      0x80000204      /* HPI downstream buffer            */
#define DSPMSGMAX        (0x204-4)/0x40  /* max downstream messages in one group  */

#define DSPLENOFFSET     0               /* offset of len in message         */
#define DSPHEADERLEN     4               /* length of message header         */

#define TASKUPPRIORITY   10              /* upstream task priority           */
#define TASKDNPRIORITY   10              /* downstream task priority         */
#define TASKUPSTK        (32 * 1024)     /* upstream task stack allocation   */
#define TASKDNSTK        (32 * 1024)     /* downstream task stack allocation */
#define UPNAME           "L1TaskUp"      /* upstream task name               */
#define DNNAME           "L1TaskDn"      /* downstream task name             */

#define DN_Q_SIZE        64              /* downstream message queue size    */

#endif /* L1PROXY_H */
