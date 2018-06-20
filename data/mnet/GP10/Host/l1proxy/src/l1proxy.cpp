/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
 * The code in this file implement a vxWorks layer 1 interface
 * to a TI CX6 DSP over the DSP's HPI interface. The message
 * structure over the interface is defined by JetCell. The basic
 * buffer consists of four bytes giving the number of blocks "n"
 * followed by "n" 64 byte blocks or messages. The first four
 * bytes of each message or block represent "len", or the
 * the length of the message block, minus an 8 byte header.
 *
 * Downstream messaging is initiated by calling the vxWorks API
 * sendDsp(void *msg, int len). sendDsp, executing in the thread
 * of the caller, will do a buffer copy to a vxWorks queue and exit
 * A vxWorks task, taskDspDn, blocks on this queue, awakening to
 * perform the actual blocking and transmission to to the DSP over
 * HPI.
 *
 * Upstream messaging is initiated by the DSP issuing an interrupt
 * to vxWorks. The interrupt is handled by the routine fooInt, which
 * will "give" the semaphore semDspUp. A vxWorks task, taskDspUp,
 * blocks on this semaphore. The HPI buffer is transferred and unblocked
 * by this task. Notice that all buffer processing is done at task level.
 * All vxWorks artifacts, queues, tasks, interrupt routines, semaphores,
 * are intialialized by initL1. The DSP is initialized by initDSP. The
 * go routine is used to enable interrupts and start the DSP in
 * HPI boot mode.
 */

#include "vxWorks.h"
#include "memLib.h"
#include "string.h"
#include "intLib.h"
#include "logLib.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "vxLib.h"
#include "errnoLib.h"
#include "stdio.h"
#include "cdcim.h" 
#include "iv.h"
#include "l1proxy/l1proxy.h"
#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"
#include "MnetModuleId.h"
#include "logging/vclogging.h"
#include "cdcUtils/cdcUtils.h"
#include "Os/JCTask.h"
#include "Os/JCModule.h"
#include "GP10OsTune.h"
#include "grr/grr_intf.h"
#include "Rlc_Mac/RlcMacIntf.h"

#ifdef __cplusplus
extern "C" {
#endif
void         hpi2dsp (unsigned int port, void *buffer, unsigned int addr, int nBytes);   
void         dsp2hpi (unsigned int port, unsigned int addr, void *buffer, int nBytes);
#include "l1proxy/l1proxyhpi.h"
#ifdef __cplusplus
}
#endif

//#define STAND_ALONE 
#undef STAND_ALONE 

/* layer-2 API for DSP-payload delivery */
extern void  CNI_LAPDM_Ph_Callback(
	int			length,
   unsigned char		*msg_unit
   );

/* layer-3 API for DSP-payload delivery */
#ifndef STAND_ALONE
extern void api_SendMsgFromL1ToRm(unsigned short, unsigned char*);
#endif


extern int g_l1proxyDiagPrint;
extern DbgOutput l1proxyDbg;

/* end: integration */


SEM_ID semDspUp;                         /* upstream semaphore               */
unsigned char dspMsgUpRdy[NUM_DSPS];

extern MSG_Q_ID msgDspDn[NUM_DSPS];      /* downstream message queue         */

int taskDspUpId;                         /* upstream task id                 */
int taskDspDnId[NUM_DSPS];               /* downstream task id               */
JCTask taskDspUpObj("L1TaskUp");
JCTask *taskDspDnObjs[NUM_DSPS];

extern int l1pTrxSwapState;              /* 1/0 - swap/don't swap trx-dsp mapping */

static char singleModeSet = FALSE;
static unsigned int singleModePort = 0;

int g_l1proxyPrintAllDspMsg = 0;

static const unsigned int hpiBase[NUM_DSPS] = {PORT0_BASE_ADDR, PORT1_BASE_ADDR};
#define HPIC_OFFSET 0x0
#define HPIA_OFFSET 0x8
#define HPID_OFFSET 0xC
#define HPIS_OFFSET 0x4

/******************************************************************************/
/*                                                                            */
/*  SWAP4BYTE() - Swap the order of bytes in a long.                          */
/*                                                                            */
/******************************************************************************/
void swap4byte(unsigned long *addr)
   
{
   unsigned long *value = addr;
   unsigned long temp1, temp2, temp3, temp4;

   temp1 = (*value)       & 0xFF;
   temp2 = (*value >> 8)  & 0xFF;
   temp3 = (*value >> 16) & 0xFF;
   temp4 = (*value >> 24) & 0xFF;

   *value = (temp1 << 24) | (temp2 << 16) | (temp3 << 8) | temp4;
}

void swap4(void *addr, int n)
     
{
   int i;
   unsigned long *p = (unsigned long *) addr;
   for (i = 0; i < n/4; i++)
   {
       swap4byte(&p[i]);
   }
}

/******************************************************************************/
/*                                                                            */
/*  SWAP2BYTE() - Swap the order of bytes in a short.                         */
/*                                                                            */
/******************************************************************************/
void swap2byte(unsigned short *addr)
{
   unsigned short *value = addr;
   unsigned short temp1,temp2;

   temp1 = temp2 = *value;
   *value = ((temp2 & 0xFF) << 8) | ((temp1 >> 8) & 0xFF);
}

/*
 * hpi2dspD is a layer 1 utility routine, transferring a 32-bit word
 * from the vxWorks host to the DSP.
 */

void hpi2dspD
    (
    unsigned int port,
    unsigned int addr,
    unsigned int data
    )
    {
    HPIA(port) = addr;
    int lock;

    lock = intLock();
    *(unsigned int *)HPID(port) = data;
    intUnlock(lock);
    }

/*
 * dsp2hpiD is a layer 1 utility routine, transferring a 32-bit word
 * from the DSP to the vxWorks host.
 */

unsigned int dsp2hpiD
    (
    unsigned int port,
    unsigned int addr
    )
    {
    unsigned int data;
    HPIA(port) = addr;
    int lock;

    lock = intLock();
    data = *(unsigned int *)HPID(port);
    intUnlock(lock);

    return data;
    }

/*
 * hpi2dsp is a layer 1 utility routine, transferring nBytes bytes
 * from the vxWorks host to the DSP.
 */

void hpi2dsp
    (
    unsigned int port,
    void *buffer,
    unsigned int addr,
    int nBytes
    )
    {
    unsigned int *bufferWord = (unsigned int *) buffer;
    int n;
    volatile unsigned int* hpisAddr = (unsigned int*)(hpiBase[port] + HPIS_OFFSET);
    int lock;

    lock = intLock();
    *(volatile unsigned int*)(hpiBase[port] + HPIA_OFFSET) = addr;
    for (n = 0; n < nBytes; n += 4)
        {
        *hpisAddr = *bufferWord++;
        }
    intUnlock(lock);
    }

/*
 * hpi2dsp_swap is a layer 1 utility routine, transferring nBytes bytes
 * from the vxWorks host to the DSP. It uses inline assembly to swap bytes.
 */

void hpi2dsp_swap
    (
    unsigned int port,
    void *buffer,
    unsigned int addr,
    int nBytes
    )
    {
    unsigned int *bufferWord = (unsigned int *) buffer;
    int n;
    volatile unsigned int* hpisAddr = (unsigned int*)(hpiBase[port] + HPIS_OFFSET);
    int lock;
  
    lock = intLock();
    *(volatile unsigned int*)(hpiBase[port] + HPIA_OFFSET) = addr;
    for (n = 0; n < nBytes; n += 4)
        {
         __asm__ ("stwbrx %0, %1, %2" : : "r" (*bufferWord++), "r" (0), "r" (hpisAddr));         
        }
    intUnlock(lock);
    }

/*
 * dsp2hpi is a layer 1 utility routine, transferring nBytes bytes
 * from the DSP to the vxWorks host.
 */

void dsp2hpi
    (
    unsigned int port,
    unsigned int addr,
    void *buffer,
    int nBytes
    )
    {
    unsigned int *bufferWord = (unsigned int *) buffer;
    int n;
    volatile unsigned int* hpisAddr = (unsigned int *)(hpiBase[port] + HPIS_OFFSET);
    int lock;

    lock = intLock();
    *(volatile unsigned int*)(hpiBase[port] + HPIA_OFFSET) = addr;    
    for (n = 0; n < nBytes; n += 4)
        {
        *bufferWord++ =  *hpisAddr;
        }
    intUnlock(lock);
    }

/*
 * dsp2hpi_swap is a layer 1 utility routine, transferring nBytes bytes
 * from the DSP to the vxWorks host. It uses inline assembly to swap bytes.
 */

void dsp2hpi_swap
    (
    unsigned int port,
    unsigned int addr,
    void *buffer,
    int nBytes
    )
    {
    unsigned int *bufferWord = (unsigned int *) buffer;
    int n;
    volatile unsigned int* hpisAddr = (unsigned int *)(hpiBase[port] + HPIS_OFFSET);
    int lock;

    lock = intLock();
    *(volatile unsigned int*)(hpiBase[port] + HPIA_OFFSET) = addr;    
    for (n = 0; n < nBytes; n += 4)
        {
        *bufferWord =  *(unsigned int *)hpisAddr;
         __asm__ ("stwbrx %0, %1, %2" : : "r" (*bufferWord), "r" (0), "r" (bufferWord));
        bufferWord++;
        }
    intUnlock(lock);
    }

/*
 * hpiInt handles the DSP interrupt. After acknowledging the interrupt,
 * it gives the upstream semaphore.
 */

void hpiInt ( unsigned int port )
{  
    STATUS status;

    unsigned int hpic = HPIC(port);

#ifdef STAND_ALONE
    l1proxyDbg.Trace("got interrupt port %d->%08x %08x!\n", port, hpic, hpic & 0x00040004); 
#endif
    if ((hpic & 0x00040004) == 0) return;
    HPIC(port) = 0x00040004;
#ifdef STAND_ALONE
    l1proxyDbg.Trace("reset interrupt port %d->%08x!\n", port, HPIC(port)); 
#endif
    dspMsgUpRdy[port] = 1;
    if ((status = semGive(semDspUp)) != OK) 
        {
        l1proxyDbg.Error("semGive returns ERROR.%d\n", errno);
        }
    return;
    }

/*
 * dspDump prints a buffer.
 */

void dspDump(char *p, int len, l1ProxyLogType logType)
{
    char buf[DSPBUFFERMAX];
    int n;
    int curPtr = 0;
    for (n = 0; n < len; n++)
    {
      if (n==0)
      {
         curPtr += sprintf(buf+curPtr, "%08x: ", (unsigned int) &p[n]);
      }
      else if (!(n % 16))
      {
         curPtr += sprintf(buf+curPtr, "\n%08x: ", (unsigned int) &p[n]);
      }
      curPtr += sprintf (buf+curPtr, "%02x ", p[n] & 0xff);
    }
    sprintf(buf+curPtr, "\n");
    
    switch (logType)
    {
        case L1PROXY_LOG_ERROR :
            l1proxyDbg.Error(buf);
            break;
        case L1PROXY_LOG_WARNING :
            l1proxyDbg.Warning(buf);
            break;
        case L1PROXY_LOG_TRACE :
            l1proxyDbg.Trace(buf);
            break;
    }   
}



void l1ProxyMemDump(char *p, int len)
{
   char buf[DSPBUFFERMAX];
   int n;
   int curPtr = 0;

   curPtr += sprintf(buf+curPtr, "\n%08x: \n", (unsigned int) &p[n]); 
   len += len%4;

   for (n = 0; n <len/4; n++)
   {     
      if(!(n%4))
      {
         curPtr += sprintf(buf+curPtr, "\n");
      }
      curPtr += sprintf(buf+curPtr, "%02x %02x %02x %02x ",
                        p[4*n+3], p[4*n+2], p[4*n+1], p[4*n]);
   }
   curPtr += sprintf(buf+curPtr, "\n");
   l1proxyDbg.Trace(buf); 
}

/*
 * taskDspDn is the downstream handler task. It blocks on a downstream
 * message queue. Upon awakening, it will do a buffer copy from the
 * message queue to a temporary buffer. Messages will be concatenated
 * int the temporary buffer until the DSP is ready to accept the buffer,
 * when the buffer will be transferred to the DSP over HPI.
 */

void taskDspDn ( unsigned int port )
{
    STATUS status;
    char dspBuffer[DSPBUFFERMAX];
    int i, nbuff, dspNum;
    int numMsgs;
    msgStruc* pMsg;
    UINT  waitCnt;

    taskDspDnObjs[port]->JCTaskEnterLoop();   /* entering loop, klim 3-2-00 */
    for (;;)
    {
      /* get messages from the queue */
      numMsgs = 0;
      pMsg = (msgStruc*)dspBuffer;
      do
      {
        status = msgQReceive(msgDspDn[port], (char*)pMsg, DSPMSGLENMAX, WAIT_FOREVER);

        if (status == ERROR)
        {
          l1proxyDbg.Error("\nL1proxy -> msgQReceive returns ERROR\n");
        }
        /*
        *  DSP expects TRX number equal "0"
        */
        pMsg->trx = 0;

        numMsgs++;         /* increament no. of messages */
        pMsg++;            /* point to next message in the buffer */

      } while (msgQNumMsgs(msgDspDn[port]) && (numMsgs < DSPMSGMAX));

/*	  // for information only
	  {
		int i=msgQNumMsgs(msgDspDn[port]);
		if(i>0) logMsg("L1PROXY-DN: INFO remaining messages in the msg queue = %d\n",i,0,0,0,0,0);
	  }
*/
      dspNum = port ^ (l1pTrxSwapState & 1);
      if (singleModeSet)
      {
        if (port != 0)
          continue;
        else 
          dspNum = singleModePort;
      }

      /* Send messages to dsp */
    
      /* wait till dsp is ready */
      waitCnt = DSP_TIMEOUT;   /* 2 ms. time out */
      do 
      {      /* wait till dsp is ready or time out */ 
       dsp2hpi(dspNum, DSPDNBUFFER, &nbuff, 0x4); /* check no. of unread msgs */
       waitCnt--;
      }
      while ((nbuff != 0) && (waitCnt != 0));

      /* if dsp is still not ready poll dsp at lower rate */
      if (nbuff != 0)
      {      /* dsp is not ready */
        l1proxyDbg.Warning("L1proxy -> DSP %d not ready for messages\n", dspNum);
        while (nbuff != 0)
        {    /* wait till dsp is ready */ 
          dsp2hpi(dspNum, DSPDNBUFFER, &nbuff, 0x4);
          taskDelay(2);  /* let lower prioroty task run */      
        }
        l1proxyDbg.Warning("L1proxy -> DSP %d became ready for messages\n", dspNum);
      }

      /* write the messages */

      hpi2dsp_swap(dspNum, dspBuffer, DSPDNBUFFER+4, numMsgs * DSPMSGLENMAX);
      /* write the number of messages */
      hpi2dsp(dspNum, &numMsgs, DSPDNBUFFER, 4);

#ifdef STAND_ALONE
      l1proxyDbg.Trace("Send to Dsp %d:\n",dspNum);
      dspDump(dspBuffer,16, L1PROXY_LOG_TRACE);
#endif
 
     if (g_l1proxyDiagPrint && 
          (
           ((((msgStruc*)dspBuffer)->typeHiByte << 8) | 
            ((msgStruc*)dspBuffer)->typeLoByte
           ) != DONW_LINK_TCH
          )
         )
      {
         l1proxyDbg.Trace("Send to Dsp %d (%d Messages):\n", dspNum, numMsgs);    
         for(i=0; i<numMsgs; i++)
            dspDump(&dspBuffer[DSPMSGLENMAX*i], dspBuffer[DSPMSGLENMAX*i]+4,
               L1PROXY_LOG_TRACE);
      }

      /*
      l1proxyDbg.Trace("sending %d messages\n", n);
      */

  }  /* for (;;) */
  taskDspDnObjs[port]->JCTaskNormExit();
}


/*
 * taskDspUp is the upstream handler task. It blocks on a semaphore
 * Upon awakening, it will do a buffer copy from the HPI buffer to a 
 * temporary buffer, unblocking messages. 
 */

void taskDspUp ( void )
{
    STATUS status;
    unsigned char buffer[DSPMSGLENMAX];
    msgStruc* pMsg;             /* received message */
    UINT   msgDisc;             /* messsage discriminator */
    unsigned int offset;        /* offset into DSPUPBUFFER */
    UINT n, nbuff, len;
    int i;
#ifndef STAND_ALONE
    int sendMsMsgToL2L3;
#endif
    unsigned int port;

    taskDspUpObj.JCTaskEnterLoop();   /* entering loop, klim 3-2-00 */
    for (;;)
    {
        if ((status = semTake(semDspUp, WAIT_FOREVER)) != OK)
        {
          l1proxyDbg.Error("\nL1proxy -> semTake on semDspUp returns ERROR\n");
        }

        for (port = 0; port < 2; port++)
        {
          if (dspMsgUpRdy[port] == 0)
            continue;  
          dspMsgUpRdy[port] = 0;   
          
          if (!singleModeSet || (port == singleModePort))
          {          
                                 
            /* read number of messages available */
            dsp2hpi(port, DSPUPBUFFER, &nbuff, 0x4);
            if (nbuff > DSPMSGMAX)
            {
              l1proxyDbg.Warning("\nL1proxy -> illegal Number of Messages from DSP: %d\n",nbuff);
              nbuff = 8;
            }

            /* read all messages */  
            for (n = 0, offset = DSPUPBUFFER+4; n < nbuff; n++, offset+=DSPMSGLENMAX)
            {
                /* read the messege size */
                dsp2hpi_swap(port, offset, buffer, DSPHEADERLEN); 
                len = buffer[DSPLENOFFSET];
                if (len > (DSPMSGLENMAX - DSPHEADERLEN))
                {
                  l1proxyDbg.Warning("\n\nL1proxy -> illegal Size of Message from DSP: %d\n" ,len);            
                  len = DSPMSGLENMAX - DSPHEADERLEN;
                }

                if(!g_l1proxyDiagPrint)
                {
                   /* read a message into the buffer according actual message length*/
                   dsp2hpi_swap(port, offset+DSPHEADERLEN, buffer+DSPHEADERLEN, len);
                }
                else
                {
                   /* read a message into the buffer includes GSM frame number at the end of the DSP message */
                   dsp2hpi_swap(port, offset+DSPHEADERLEN, buffer+DSPHEADERLEN, DSPMSGLENMAX - DSPHEADERLEN);
                }

                if(g_l1proxyPrintAllDspMsg)
				{
                   l1proxyDbg.Trace("From Dsp %d (%d Messages):\n", port, nbuff);    
                   for(i=0; i<nbuff; i++)
                       dspDump((char *)&buffer[DSPMSGLENMAX*i], buffer[DSPMSGLENMAX*i]+4,L1PROXY_LOG_TRACE);
				}

#ifdef STAND_ALONE 
                l1proxyDbg.Trace("Rececived from Dsp %d:\n",port);
                dspDump((char *)buffer, 8, L1PROXY_LOG_TRACE);
#endif
                pMsg = (msgStruc*) buffer;     /* received message */

                /* build trx field of the message */
                pMsg->trx &= 0x0F;
                if (!singleModeSet)                
                  pMsg->trx |= (port ^ (l1pTrxSwapState & 1));                

#ifdef STAND_ALONE
                l1proxyDbg.Trace("Mesasge for Upper Layers:\n");
                dspDump((char *)buffer, 8, L1PROXY_LOG_TRACE);
#endif
                /*
                 * check if message is need to send to up layer
                */
#ifndef STAND_ALONE
                sendMsMsgToL2L3 = ulMobileMessagePreProc(buffer);
#endif

#ifndef STAND_ALONE

                /*
                * logging received MS messages
                */
                if (sendMsMsgToL2L3)
                {
                  /* call the appropriate API to send the message */
                  msgDisc = pMsg->function;
                  if (msgDisc == RR_MANAGE_MSG)
                  {                     
                  
                    CNI_LAPDM_Ph_Callback(len + DSPHEADERLEN, buffer);                                             
                         
                  }   
                  else if ((msgDisc == CCCH_MANAGE_MSG) || (msgDisc == DCCH_MANAGE_MSG))
                  {
   
                    // api_SendMsgFromL1ToRm(*(unsigned short *)buffer, (unsigned char *)&buffer[4]);             

                    api_SendMsgFromL1ToRm(len, (unsigned char *)&buffer[DSPHEADERLEN]);                                
                    
                    if (g_l1proxyDiagPrint)
                    {
                      l1proxyDbg.Trace("From DSP To RRM");
                      dspDump((char *)&buffer[0], buffer[0]+4, L1PROXY_LOG_TRACE);          
                    }           

                  }
                  else if (msgDisc == TRX_MANAGE_MSG)
                  {

                    grr_SendMsgFromL1ToGrr(len, &buffer[DSPHEADERLEN]);
                    if (g_l1proxyDiagPrint)
                    {
                      l1proxyDbg.Trace("From DSP To RRM");
                      dspDump((char *)&buffer[0], buffer[0]+4, L1PROXY_LOG_TRACE);          
                    }           

                  }
                  else if (msgDisc == PDCH_MANAGE_MSG)
                  {

                    RlcMacL1MsgInd(&buffer[DSPHEADERLEN], len);
                    if (g_l1proxyDiagPrint)
                    {
                      l1proxyDbg.Trace("From DSP To RLC/MAC");
                      dspDump((char *)&buffer[0], buffer[0]+4, L1PROXY_LOG_TRACE);          
                    }           

                  }
                }  /* if (sendMsMsgToL2L3) */     
#endif
            }  /* for (n = 0, offset */
          } /* if (!singleModeSet */
          /* clear 'Number of Messages' field to inform dsp messages are read */
          nbuff = 0;
          hpi2dsp(port, &nbuff, DSPUPBUFFER, 4);
        } /* for (port */

    }  /* for (;;) */
    taskDspUpObj.JCTaskNormExit();
}


/*
 * initDsp intializes the DSP hardware
 */

void initDsp
    (
    unsigned int port
    )
    {
/*
    unsigned int resetBit = (port == 0) ? 0x00000800 : 0x00000400;
    AUX_PORT = ~resetBit;
    taskDelay(1);
    AUX_PORT = 0xffffffff;
*/
    HPIC(port)  = 0x00000000;
    hpi2dspD(port, 0x1800000, 0x00003020);
    hpi2dspD(port, 0x1800008, 0x00000040);
    hpi2dspD(port, 0x1800004, 0x20E30322);
    hpi2dspD(port, 0x1800010, 0x00000040);
    hpi2dspD(port, 0x1800014, 0x00000040);
    hpi2dspD(port, 0x1800018, 0x07116000);
    hpi2dspD(port, 0x180001c, 0x00000410);
    }

/*
 * initL1 intializes the layer 1 interrupt routine, semaphores,
 * tasks and message queues.
 */

void initL1
    (
    void
    )
{
    unsigned int i;

    semDspUp = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    if (semDspUp == NULL)
    {
        logMsg("ERROR cannot create semDspUp\n",0,0,0,0,0,0);
    }

    dspLoadSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
    if (dspLoadSem == NULL)
    {
        logMsg("ERROR cannot create dspLoadSem\n",0,0,0,0,0,0);
    }

    for (i = 0; i < NUM_DSPS;  i++)
    {    
      msgDspDn[i] = msgQCreate(DN_Q_SIZE, DSPMSGLENMAX, MSG_Q_FIFO);
      if (msgDspDn == NULL)
          {
          logMsg("ERROR cannot create msgDspDn No. %d\n",i,0,0,0,0,0);
          }
    }
#ifdef STAND_ALONE 
   taskDspUpId = taskSpawn(UPNAME, TASKUPPRIORITY, 0, TASKUPSTK,
        (FUNCPTR) taskDspUp,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#else
    taskDspUpId = taskDspUpObj.JCTaskSpawn( 
        L1UP_TASK_PRIORITY, L1UP_TASK_OPTION, L1UP_TASK_STACK_SIZE,
        (FUNCPTR) taskDspUp,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        MODULE_L1,JC_CRITICAL_TASK);
#endif
    if (taskDspUpId == ERROR)
        {
        logMsg("ERROR cannot create taskDspUp\n",0,0,0,0,0,0);
        }
    for (i = 0; i < NUM_DSPS;  i++)
    {
#ifdef STAND_ALONE
      taskDspDnId[i] = taskSpawn(DNNAME, TASKDNPRIORITY, 0, TASKDNSTK,
          (FUNCPTR) taskDspDn,
          i, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#else 
      char taskName[30];
      sprintf(taskName, "L1TaskDn%d", i);  
      taskDspDnObjs[i] = new JCTask(taskName);
      taskDspDnId[i] = taskDspDnObjs[i]->JCTaskSpawn( 
          L1DN_TASK_PRIORITY, L1DN_TASK_OPTION, L1DN_TASK_STACK_SIZE,
          (FUNCPTR) taskDspDn,
          i, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          MODULE_L1,JC_CRITICAL_TASK);
#endif
      if (taskDspDnId[i] == ERROR)
          {
          logMsg("ERROR cannot create taskDspDn No. %d\n",i,0,0,0,0,0);
          }
    }
    
    /*
     * Setup a DbgOutput object for logging.
     */
    l1proxyDbg.Func("l1proxy Logging",L1PROXY_LAYER);
}

/*
 * go enables DSP interrupts and releases the DSP from the HPI boot
 * reset state.
 */

void go
    (
    unsigned int port
    )
    {
    int nbuff = 0;
    hpi2dsp(port, &nbuff, DSPUPBUFFER, 4);
    HPIC(port) |= 0x00020002;
    }

void initInt(void)
    {
    intConnect((VOIDFUNCPTR*)IV_IRQ1, (VOIDFUNCPTR)hpiInt, 0);
    intConnect((VOIDFUNCPTR*)IV_IRQ2, (VOIDFUNCPTR)hpiInt, 1);
    intEnable(IVEC_TO_INUM(IV_IRQ1));
    intEnable(IVEC_TO_INUM(IV_IRQ2));
    }

void xdump(unsigned int host, unsigned int p, unsigned int count)
{
   unsigned int n, m, addr;
   char buf[DSPBUFFERMAX];
   int curPtr = 0;

   for(n = 0; n < count; n++)
   {
      addr = p + 4 * n;
      m = dsp2hpiD(host, addr);
      curPtr += sprintf(buf+curPtr, "%08x->%08x\n", addr, m); 
   }
   l1proxyDbg.Trace(buf);
}

void zdump(unsigned int host, unsigned int p, unsigned int count)
{
   unsigned int n, addr, *data;
   char buf[DSPBUFFERMAX];
   int curPtr = 0;

   data = (unsigned int*)malloc(4*count);
   dsp2hpi(host, p, data, 4*count);
   for(n = 0; n < count; n++)
   {
      addr = p + 4 * n;
      curPtr += sprintf(buf+curPtr, "%08x->%08x\n", addr, data[n]); 
   }
   l1proxyDbg.Trace(buf);
   free(data);
}

void xtest(unsigned int host, unsigned int p, unsigned int count)
{
   unsigned int n, m, addr;
   char buf[DSPBUFFERMAX];
   int curPtr = 0;

   for(n = 0; n < count; n++)
   {
      addr = p + 4 * n;
      hpi2dspD(host, addr, addr);
   }
   for(n = 0; n < count; n++)
   {
      addr = p + 4 * n;
      m = dsp2hpiD(host, addr);
      if (m != addr)
      {
         curPtr += sprintf(buf+curPtr, "%08x->%08x\n", addr, m); 
      }
   }
   l1proxyDbg.Trace(buf);
}

void ytest(unsigned int host, unsigned int p, unsigned int count)
{
   unsigned int n, *q, addr;
   char buf[DSPBUFFERMAX];
   int curPtr = 0;

   q = (unsigned int*)malloc(4 * count);
   for(n = 0; n < count; n++)
   {
      q[n] = p + 4 * n;
   }
   hpi2dsp(host, q, p, 4 * count);
   dsp2hpi(host, p, q, 4 * count);
   for(n = 0; n < count; n++)
   {
      addr = p + 4 * n;
      if (q[n] != addr)
      {
         curPtr += sprintf(buf+curPtr, "%08x->%08x\n", addr, q[n]); 
      }
   }
   l1proxyDbg.Trace(buf);
}


/***********************************************************/
/* The new root task spawns l1 up/down tasks.              */
/* In that case, we want to maintain to do some pre-       */
/* procedures and post-procedures                          */
/* These functions replace L1Init procedures yet called    */
/* only when using new script that brings up the root task */
/*                                            4-14-99 KLim */
/***********************************************************/
void L1_PreInit(void)
{
    unsigned int port;

    dspLoadSem = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
    if (dspLoadSem == NULL)
    {
        logMsg("ERROR cannot create dspLoadSem\n",0,0,0,0,0,0);
    }

    semDspUp = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    if (semDspUp == NULL)
    {
        logMsg("ERROR cannot create semDspUp\n",0,0,0,0,0,0);
    }

    for (port = 0; port < NUM_DSPS;  port++)
    {    
      msgDspDn[port] = msgQCreate(DN_Q_SIZE, DSPMSGLENMAX, MSG_Q_FIFO);
      if (msgDspDn == NULL)
          {
          logMsg("ERROR cannot create msgDspDn No. %d\n",port,0,0,0,0,0);
          }
    }
}

void L1_PostInit(void)
{

}


/*******************************************************************************
l1pSingleModeStart

Starts running l1proxy in single dsp mode. 
The parameter specifies which dsp to communicate with.

*/
int l1pSingleModeStart
(
  unsigned char port
)
{
  if (port < NUM_DSPS)
  {
    singleModeSet = TRUE;
    singleModePort = port;
    return OK;   
  }
  return ERROR;
}


/*******************************************************************************
l1pSingleModeStop

Stops l1proxy from running in single dsp mode. 
*/

void l1pSingleModeStop()
{
  singleModeSet = FALSE;
}



/*******************************************************************************
SysCommand_L1Proxy
*/

int SysCommand_L1Proxy (T_SYS_CMD action) 
{
  switch(action)
  {
    case SYS_SHUTDOWN:
      l1proxyDbg.Trace("[L1Proxy] shutdown notification received\n");
      break;

    case SYS_START:
      initL1();
      break;

    default:
      l1proxyDbg.Trace("[L1Proxy] Unknown system command received\n");
      
  }
  return 0;
}





