/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
JetCell modification history
----------------------------
99/07/12  ck   Add I2Cread2 and I2Cwrite2. Perform read operation twice 
               on certain error to work around MPC860T bug.
99/06/01  ck   Initial Version
*/

/* i2c.c - I2C interface library */

#include <vxWorks.h> 
#include <logLib.h> 
#include <taskLib.h> 
#include <intLib.h> 
#include <arch/ppc/vxPpcLib.h> 
#include <drv/multi/ppc860Cpm.h> 
#include <drv/multi/ppc860Siu.h> 
#include <arch/ppc/ivPpc.h> 
#include <drv/intrCtl/ppc860Intr.h> 

#include "i2c.h"

/* Rx Buffer Area */
int even_align;    /* align buffer rxbuf on even boundary */
unsigned char rxbuf[I2C_RX_LEN];

/* Tx Buffer Area */
unsigned char txbuf0[I2C_TX_LEN];
unsigned char txbuf1[I2C_TX_LEN];


/* vxWorks I2C communications semaphores */
SEM_ID i2cIntSem;    /* semaphore for task sychronization */  
SEM_ID i2cSem;       /* semaphore for i2c read/write mutual exclusion */

static unsigned int prevTxbEvCnt = 0;
static unsigned int rxbEvCnt = 0;
static unsigned int txbEvCnt = 0;
static unsigned int txeEvCnt = 0;
static unsigned int bsyEvCnt = 0;
static unsigned int multEvCnt = 0;
static unsigned int totEvCnt = 0;

/*******************************************************************************
i2cIntHandler  - i2c Interrupt Handler

Interrupt service routine for i2c interrupts. 

*/
static void i2cIntHandler(void) {
  void *vxImmr = (void *) vxImmrGet();
  unsigned char cer, flag = 0;
  STATUS status;
  unsigned int eventChange = 0;

  while ((cer = *I2CER(vxImmr)))
  {
    flag |= cer;
    *I2CER(vxImmr) = cer;
  }
#ifdef notnow 
  logMsg("->%02x\n", flag, 0, 0, 0, 0, 0);
#endif

  totEvCnt++;

  if (flag & 0x01) {
      rxbEvCnt++;
      eventChange++;
      if ((status = semGive(i2cIntSem)) != OK)     /* give the semaphore to wake 
                                                     the blocked tasks */
      {
        logMsg("semGive on i2cIntSem returns ERROR %d\n", errno, 0, 0, 0, 0, 0);
      }
  }
  if (flag & 0x02) {
      txbEvCnt++;
      eventChange++;
      if ((status = semGive(i2cIntSem)) != OK)     /* give the semaphore to wake 
                                                     the blocked tasks */
      {
        logMsg("semGive on i2cIntSem returns ERROR %d\n", errno, 0, 0, 0, 0, 0);
      }
  }
  if (flag & 0x04) {
      bsyEvCnt++;
      eventChange++;
  }
  if (flag & 0x10) {
      txeEvCnt++;
      eventChange++;
  }

  if (eventChange > 1)
      multEvCnt++;
}


/*******************************************************************************
bdGet

Returns addres of Buffer Descriptors used for I2C operations
*/

static void *bdGet(void)
{
  void *vxImmr = (void *) vxImmrGet();
  return ( void * ) ((unsigned char *)vxImmr + 0x2000 + 0x290);
}


/*******************************************************************************
 * Function Name: I2Cinit
 *
 * Description:  Setup I2C Controller as follows:
 *               1) Master Operation Mode
 *               2) Clock Filter OFF (recommended for RevA,B silicon)
 *               3) **** I2C NOT Enabled ****
 *               4) Clock Division factor 96 (260 KHz @ 25MHz)
 *
 */

void I2Cinit( void )
{
  void *vxImmr = (void *) vxImmrGet();
  RTXBD *rtx = ( RTXBD * ) bdGet();
  /*
   * Configure Port Pins
   */

  /*
   * PB27: I2CSDA -- I2C Serial Data In/Out -- Tri-State/Pullup
   * PB26: I2CSCL -- I2C Serial Clock -- Tri-State/Pullup
   */

  *PBPAR(vxImmr) |= (0x30);
  *PBDIR(vxImmr) |= (0x30);
  *PBODR(vxImmr) |= (0x30);

  /* Set RXBD tbl start at Dual Port */
  *I2C_RBASE(vxImmr) = (unsigned short) ((unsigned int) &(rtx->rxbd) & 0xffff);

  /* Set TXBD tbl start at Dual Port */
  *I2C_TBASE(vxImmr) = (unsigned short) ((unsigned int) &(rtx->txbd[0]) & 0xffff);

  /*
   * Issue Init RX & TX Parameters Command for I2C.
   */

  while( *CPCR(vxImmr) & CPM_CR_FLG );  /* SPIN UNTIL READY TO ISSUE COMMAND */
  *CPCR(vxImmr) = CPM_CR_CHANNEL_I2C | CPM_CR_OPCODE_INIT_RT;
  while( *CPCR(vxImmr) & CPM_CR_FLG );  /* SPIN UNTIL READY TO ISSUE COMMAND */
  *CPCR(vxImmr) = CPM_CR_CHANNEL_I2C | CPM_CR_OPCODE_CLOSE;
  while( *CPCR(vxImmr) & CPM_CR_FLG );  /* SPIN UNTIL COMMAND PROCESSED */

  /* Initialize SDMA Configuration Register */
  *SDCR(vxImmr) = 1;

  /*
   * Set RFCR,TFCR -- Rx,Tx Function Code
   */

  /* Normal Operation and Mot byte ordering */
  *I2C_RFCR(vxImmr) = 0x10;

  /* Mot byte ordering, Normal access */
  *I2C_TFCR(vxImmr) = 0x10;
  
  /*
   * Set MRBLR -- Max. Receive Buffer Length
   */

  *I2C_MRBLR(vxImmr)   = I2C_RX_LEN;
  *I2C_RSTATE(vxImmr)  = 0;
  *I2C_RBPTR(vxImmr)   = *I2C_RBASE(vxImmr);
  *I2C_TSTATE(vxImmr)  = 0;
  *I2C_TBPTR(vxImmr)   = *I2C_TBASE(vxImmr);

  /*
   * Initialize RxBD.
   *
   * Settings:    Wrap, Interrupt
   */

  rtx->rxbd.bd_addr = rxbuf;
  rtx->rxbd.bd_cstatus = 0x3000;    /* Wrap, Interrupt */
  rtx->rxbd.bd_length = 0;

  /*
   * Initialize TxBDs.
   *
   */

  rtx->txbd[0].bd_addr = txbuf0;
  rtx->txbd[0].bd_cstatus = 0;

  rtx->txbd[1].bd_addr = txbuf1;
  rtx->txbd[1].bd_cstatus = 0;

  /*
   * Setup MPC860 I2C address.
   */
   
  *I2ADD(vxImmr) = MPC860_I2C_ADDR; 
  /*
   * Clear I2C Event Register by writing all 1's.
   */

  *I2CER(vxImmr) = ~0;

  /*
   * Set Master Mode in I2C Command Register
   */

  *I2COM(vxImmr) = 1;

  /*
   * Configure I2C Mode Register
   * 
   * REVD=0 -- For Normal I2C operation the UM is incorrect.
   *           0 is normal operation.
   */

  *I2MOD(vxImmr) = 0;

  /*
   * Set the I2C BRG Clock Division Factor
   *
   * NOTE:  ** Assume MPC8XX Clock Frequency of 24MHz.
   * NOTE:  ** Assume that sccr dfbrg field == 0 (divide BRGCLK by 1) **
   *
   * AT24C01 Data Sheet indicates a maximum Serial Clock Frequency of
   *     400 Khz at 5V operation.  Thus, a total division factor of >= 60 is
   *     needed.
   */

  *I2MOD(vxImmr) |= (BAUD_PREDIV_AT24C01)<<1;
  *I2MOD(vxImmr) |=  1; /* Enable I2C Operation */
  *I2BRG(vxImmr)  = BAUD_DIV_AT24C01;
  i2cIntSem = 0; 
}


/*******************************************************************************
 * Function Name: I2CvxInit
 *
 * Description:  Setup I2C Controller for vxWorks tasking environment:
 *               1) Call I2Cinit
 *               2) Create i2cIntSem semaphore
 *               3) Setup I2c interrupt vectors
 *
 */

STATUS I2CvxInit(void)
{
  void *vxImmr = (void *) vxImmrGet();
  I2Cinit();
  i2cIntSem = semCCreate(SEM_Q_FIFO, SEM_EMPTY);
  if (i2cIntSem == NULL)
  {
      logMsg("ERROR cannot create i2cIntSem\n",0,0,0,0,0,0);
      return ERROR;
  }
  i2cSem = semMCreate(SEM_Q_FIFO | SEM_DELETE_SAFE);
  if (i2cSem == NULL)
  {
      logMsg("ERROR cannot create i2cSem\n",0,0,0,0,0,0);
      return ERROR;
  }
  intConnect(IV_I2C, i2cIntHandler, 0);
  *I2CMR(vxImmr) = 0x17;
  *CIMR(vxImmr) |= CIMR_I2C;
  return OK;
}


/*******************************************************************************
 * I2Coperation -- Enable/Disable I2C Operation.
 *
 * Input Parameter:  setting -- Disable I2C Operation iff == OFF;
 *                              Enable I2C Operation iff == ON
 *
 * Note:  Useful for reducing power consumption when access to I2C is not
 *        needed.
 */

void I2Coperation(setting)
int setting;
{
#if 0 /* Disable turning on and off of I2C because it causes crashes from lack of Txb interrupt */
  void *vxImmr = (void *) vxImmrGet();
  if (setting == ON) {
      *I2MOD(vxImmr) |=  1; /* Enable I2C Operation */
  } else if (setting == OFF) {
      *I2MOD(vxImmr) &= ~1; /* Disable I2C Operation */
  }
#endif
}


/*******************************************************************************
 * RxBD -- Turn ON (Ready) or turn OFF (Disable) Receive Buffer Descriptor
 *         to receive data.
 */

void RxBD(setting)
int setting;
{
  RTXBD *rtx = ( RTXBD * ) bdGet();

  if(setting == ON)  
      rtx->rxbd.bd_cstatus |=  RXBD_E;    /* Ready */
  else if(setting == OFF)  
      rtx->rxbd.bd_cstatus &= ~RXBD_E;    /* Disable */
}


/*******************************************************************************
 * TXBDcstatusInit -- Initialized Transmit Buffer Descriptor Control and
 *                      Status Register.
 *
 * Description:  Initialize Transmit Buffer Descriptor indexed by bd_indx
 *               with bd_flags value.
 *
 * Input Parameters:  bdIndx  -- Transmit Buffer Descriptor Index
 *                    bdFlags -- Transmit Buffer Descriptor Flags Settings
 *
 */

static void TXBDcstatusInit(bdIndx, bdFlags)
int bdIndx, bdFlags;
{
  RTXBD *rtx = ( RTXBD * ) bdGet();

  /* Initialize TX BD Control and Status Field */
  rtx->txbd[bdIndx].bd_cstatus = bdFlags;
}


/*******************************************************************************
 * I2CwriteArr -- Write array contents to Transmit Buffer Descriptor
 *
 * Description:  Write len elements of tx_arr character array to transmit
 *               buffer descriptor indexed by bd_indx.
 *
 * Input Parameters:  bd_indx -- Transmit Buffer Descriptor Index
 *                    txArr   -- Array of characters to be transmitted
 *                    len     -- Number (length) of characters to be transmitted
 *
 */

static void I2CwriteArr(bdIndx, txArr, len)
int bdIndx;
unsigned char *txArr;
int len;
{
  RTXBD *rtx = ( RTXBD * ) bdGet();
  int i;

  /* Transmit length in bytes */
  rtx->txbd[bdIndx].bd_length = len;

  /* Copy in bytes to be transmitted */
  for (i = 0; i < len; i++) rtx->txbd[bdIndx].bd_addr[i] = txArr[i];

  /* Ready to Transmit */
  rtx->txbd[bdIndx].bd_cstatus |= TXBD_R;
}


/*******************************************************************************
 * I2Ctransmit -- Start I2C Transmission
 *
 * Description: Initiate I2C Transmit, and check the number of Tx BDs
 *              specified.
 *
 * Input Parameter:  numTxbds -- number of Tx BDs to be transmitted.
 *
 */

static STATUS I2Ctransmit(numTxbds)
int numTxbds;
{
  void *vxImmr = (void *) vxImmrGet();
  RTXBD *rtx = ( RTXBD * ) bdGet();
  int i;
/*  unsigned int n; */
  STATUS status;
  
  semTake(i2cIntSem, NO_WAIT); 

  *I2COM(vxImmr) |= 0x80; /* Transmit */

  /* Spin until all transmit buffers used have finished transmitting */
  for (i = 0; i < numTxbds; i++)
  {
#if 0    
    if (i2cIntSem == 0)
    {
      n = 0;
      while(rtx->txbd[i].bd_cstatus & TXBD_R)
      {
        if (++n > LONGTIME) 
        {
          logMsg("ERROR: Timeout waiting for TXBD_R\n", 0,0,0,0,0,0);
	  return -1;
        }
      }
    } 
    else 
#endif
    {
      /* wait upto one second */
      if ((status = semTake(i2cIntSem, 10 * sysClkRateGet())) != OK) 
      {
        if (errno == S_objLib_OBJ_TIMEOUT)
          logMsg("I2C transmit timed out %d curTxbds %d curTxbCnt %d prevTxbCnt %d\n", errno, i, txbEvCnt, prevTxbEvCnt, 0, 0);
        else
          logMsg("semTake on i2cIntSem in I2Ctransmit returns ERROR %d\n", 
                  errno, 0, 0, 0, 0, 0);
	    return -1;
      }

      if (rtx->txbd[i].bd_cstatus & TXBD_R)
      {
          /* Note MPC860T bug:
             After a not acknowledged read on I2C, this error is generated 
             on next acknowledged read.
          */
          logMsg("TXBD_R and i2cIntSem inconsistent\n", 0, 0, 0, 0, 0, 0);
          return -2; 
      }
      if (rtx->txbd[i].bd_cstatus & TXBD_N)
      {
          logMsg("No Acknowledge on I2C Transmit\n", 0, 0, 0, 0, 0, 0);
	  return -3;
      }

    }
  }
  return OK;
}


/*******************************************************************************
 * readBD -- Retrieve characters from buffer descriptor starting with MS byte.
 *           Return number of characters read.
 */

static int readBD(unsigned char *chr, int numChars)
{
  RTXBD *rtx = ( RTXBD * ) bdGet();
  int i;
/*  unsigned int n; */
  STATUS status;

  /* limit number of characters read by length of receive data buffer */
  if (numChars > (rtx->rxbd.bd_length))
      numChars = (rtx->rxbd.bd_length);

  /* Wait until buffer is ready to read */
#if 0
  if (i2cIntSem == 0)
  {
    n = 0;
    while(rtx->rxbd.bd_cstatus & RXBD_E)
    {
      if (++n > LONGTIME) 
      {
	logMsg("ERROR: Timeout waiting for RXBD_E\n", 0,0,0,0,0,0);
	return 0;
      }
    }
  } 
  else 
#endif 
  {
    /* wait upto one second */
    if ((status = semTake(i2cIntSem, 10 * sysClkRateGet())) != OK)
    {
        if (errno == S_objLib_OBJ_TIMEOUT)
          logMsg("I2C readBD timed out %d\n", errno, 0, 0, 0, 0, 0);
        else
          logMsg("semTake on i2cIntSem in readBD returns ERROR %d\n", 
                  errno, 0, 0, 0, 0, 0);
    	return 0;
    }

    if (rtx->rxbd.bd_cstatus & RXBD_E)
    {
        logMsg("RXBD_E and i2cIntSem inconsistent\n", 0, 0, 0, 0, 0, 0);
	return 0;
    }
  }

  for(i = 0; i < numChars; i++)
      chr[i] = rtx->rxbd.bd_addr[i];

  return numChars;
}


/*******************************************************************************
 * readRxBD -- Retrieve characters from receive buffer descriptor.
 *           Return number of characters read.
 */

int readRxBD(unsigned char *chr, int numChars)
{
  RTXBD *rtx = ( RTXBD * ) bdGet();
  int i;

  /* limit number of characters read by length of receive data buffer */
  if (numChars > (rtx->rxbd.bd_length))
      numChars = (rtx->rxbd.bd_length);


  for(i = 0; i < numChars; i++)
      chr[i] = rtx->rxbd.bd_addr[i];

  return numChars;
}




/*******************************************************************************
 * Function name: I2Cwrite
 *
 * Description:  Transmit n Characters using I2C
 *
 * Input Parameters:
 *                   *chr     -- character array
 *                   devAddr  -- i2c slave device general address
 *                   wAddr    -- i2c slave device input port address
 *                   numChars -- number of characters to be written
 *
 */

STATUS I2Cwrite( chr, devAddr, wAddr, numChars )
unsigned char *chr, devAddr, wAddr;
int numChars;
{
  int i;
  unsigned char txArr[I2C_TX_LEN];  /* assume numChars <= 126 */
  STATUS status = ERROR;
  int myOldTaskPriority;

  /* prevent reentrancy to guard the resource */
  if ((semTake(i2cSem, WAIT_FOREVER)) != OK)
  {
     logMsg("semTake returns ERROR %d\n", errno, 0, 0, 0, 0, 0);
     return ERROR;
  }

    /* Set the task to very high priority while transmitting */
  if (taskPriorityGet(taskIdSelf(),&myOldTaskPriority) != OK)
      logMsg("I2Cwrite: taskPriorityGet failed \n",0,0,0,0,0,0);
  if (taskPrioritySet(taskIdSelf(),0) != OK)
      logMsg("I2Cwrite: taskPrioritySet 1 failed\n",0,0,0,0,0,0);

  TXBDcstatusInit(TXBD_0, TXBD_W | TXBD_L | TXBD_I);

  /*
   * Transmit:
   *     7-bit device address, write bit, and
   *     8-bit word address, and
   *     character data
   */

  /* Store 7-bit device address and write-bit */
  txArr[0] = (devAddr << 1) | WRITE_BIT;

  /* Store 8-bit word address */
  txArr[1] = wAddr;

  /* Store num_chars characters */
  for(i = 0; i < numChars; i++)
      txArr[i + 2] = chr[i];

  I2CwriteArr(TXBD_0, txArr, 2 + numChars);

  /* Transmit Byte Write Message (single TXBD) */
  if (I2Ctransmit(1) == OK) 
    status = OK;

  /* Set the task back to old task priority */
  if (taskPrioritySet(taskIdSelf(),myOldTaskPriority) != OK)
      logMsg("I2Cwrite: taskPrioritySet myOldTaskPriority failed\n",0,0,0,0,0,0);

  semGive(i2cSem);


  return status;
}


/*******************************************************************************
 * Function name:  I2Cread
 *
 * Description:  Issue current address read (random read) to get 4 characters
 *               from e2prom.
 *
 */

STATUS I2Cread(outch, devAddr, wAddr, numChars)
unsigned char *outch, devAddr, wAddr;
int numChars;
{
  unsigned char txArr[I2C_RX_LEN]; /* assume numChars <= 127 */
  STATUS status = ERROR;
  int transmitStatus;
  int n;
  int myOldTaskPriority;

  /* prevent reentrancy to guard the resource */
  if ((semTake(i2cSem, WAIT_FOREVER)) != OK)
  {
     logMsg("semTake returns ERROR %d\n", errno, 0, 0, 0, 0, 0);
     return ERROR;
  }

    prevTxbEvCnt = txbEvCnt;
  /* Set the task to very high priority while transmitting */
    if (taskPriorityGet(taskIdSelf(),&myOldTaskPriority) != OK)
      logMsg("I2Cread: taskPriorityGet failed \n",0,0,0,0,0,0);
    if (taskPrioritySet(taskIdSelf(),0) != OK)
      logMsg("I2Cread: taskPrioritySet 1 failed\n",0,0,0,0,0,0);

  /* On '-2' error perform the operation twice to work around */ 
  for (transmitStatus = -2, n = 0; (transmitStatus == -2) && (n < 2); n++)
  {
    /* Ready Receive Buffer Descriptor */
    RxBD(ON);

    /*
     * Step 1: Setup Dummy Write
     *     Configure Tx BD 0 for Interrupt
     *     Byte0: Device Address with Write Bit
     *     Byte1: Word Address
     */

    TXBDcstatusInit(TXBD_0, TXBD_I);
  
    /* Store 7-bit device address and write-bit */
    txArr[0] = (devAddr << 1) | WRITE_BIT;

    /* Store word address */
    txArr[1] = wAddr;

    /* Store Dummy Write in Tx BD 0 */
    I2CwriteArr (TXBD_0, txArr, 2);

    /*
     * Step 2: Setup Read Request
     *     Configure Tx BD 1 for: Wrap, Interrupt, Last, Start bits
     *     Byte0: Device Address and Read Bit
     *     numChars: Unintialized Dummy Bytes to Toggle Serial Clock during Read
     */

    TXBDcstatusInit(TXBD_1, TXBD_W | TXBD_I | TXBD_S | TXBD_L);

    /* Store 7-bit device address and write-bit */
    txArr[0] = (devAddr << 1) | READ_BIT;

    /* Store Read Request in Tx BD 1 */
    I2CwriteArr(TXBD_1, txArr, 1 + numChars);

    /* Transmit Random Read Message:  Dummy Write + Read Request */
    if ((transmitStatus = I2Ctransmit(2)) == OK) 
    {
      /* Read Data */
      if (readBD(outch, numChars) == numChars) 
        status = OK;
    }
    else
        logMsg("I2Cread: I2Ctransmit returned error\n",0,0,0,0,0,0);
  }

  /* Set the task back to old task priority */
  if (taskPrioritySet(taskIdSelf(),myOldTaskPriority) != OK)
      logMsg("I2Cread: taskPrioritySet myOldTaskPriority failed\n",0,0,0,0,0,0);

  semGive(i2cSem);

  return status;
}


/*******************************************************************************
 * Function name: I2Cwrite2
 *
 * Description:  Transmit n Characters using I2C
 *
 * Input Parameters:
 *                   *chr     -- character array
 *                   devAddr  -- i2c slave device general address
 *                   numChars -- number of characters to be written
 *
 */

STATUS I2Cwrite2( chr, devAddr, numChars )
unsigned char *chr, devAddr;
int numChars;
{
  int i;
  unsigned char txArr[I2C_TX_LEN];  /* assume numChars <= 126 */
  STATUS status = ERROR;
  int myOldTaskPriority;

  /* prevent reentrancy to guard the resource */
  if ((semTake(i2cSem, WAIT_FOREVER)) != OK)
  {
     logMsg("semTake returns ERROR %d\n", errno, 0, 0, 0, 0, 0);
     return ERROR;
  }

    /* Set the task to very high priority while transmitting */
  if (taskPriorityGet(taskIdSelf(),&myOldTaskPriority) != OK)
      logMsg("I2Cwrite: taskPriorityGet failed \n",0,0,0,0,0,0);
  if (taskPrioritySet(taskIdSelf(),0) != OK)
      logMsg("I2Cwrite: taskPrioritySet 1 failed\n",0,0,0,0,0,0);

  TXBDcstatusInit(TXBD_0, TXBD_W | TXBD_L | TXBD_I);

  /*
   * Transmit:
   *     7-bit device address, write bit, and
   *     character data
   */

  /* Store 7-bit device address and write-bit */
  txArr[0] = (devAddr << 1) | WRITE_BIT;

  /* Store num_chars characters */
  for(i = 0; i < numChars; i++)
      txArr[i + 1] = chr[i];

  I2CwriteArr(TXBD_0, txArr, 1 + numChars);

  /* Transmit Byte Write Message (single TXBD) */
  if (I2Ctransmit(1) == OK) 
    status = OK;

  /* Set the task back to old task priority */
  if (taskPrioritySet(taskIdSelf(),myOldTaskPriority) != OK)
      logMsg("I2Cwrite: taskPrioritySet myOldTaskPriority failed\n",0,0,0,0,0,0);

  semGive(i2cSem);

  return status;
}


/*******************************************************************************
 * Function name:  I2Cread2
 *
 * Description:  Issue current address read (random read) to get 4 characters
 *               from e2prom.
 *
 */

STATUS I2Cread2(outch, devAddr, numChars)
unsigned char *outch, devAddr;
int numChars;
{
  unsigned char txArr[I2C_RX_LEN]; /* assume numChars <= 127 */
  STATUS status = ERROR;
  int transmitStatus;
  int n;
  int myOldTaskPriority;

  /* prevent reentrancy to guard the resource */
  if ((semTake(i2cSem, WAIT_FOREVER)) != OK)
  {
     logMsg("semTake returns ERROR %d\n", errno, 0, 0, 0, 0, 0);
     return ERROR;
  }

    /* Set the task to very high priority while transmitting */
    if (taskPriorityGet(taskIdSelf(),&myOldTaskPriority) != OK)
      logMsg("I2Cwrite: taskPriorityGet failed \n",0,0,0,0,0,0);
    if (taskPrioritySet(taskIdSelf(),0) != OK)
      logMsg("I2Cwrite: taskPrioritySet 1 failed\n",0,0,0,0,0,0);

  /* On '-2' error perform the operation twice to work around */ 
  for (transmitStatus = -2, n = 0; (transmitStatus == -2) && (n < 2); n++)
  {
    /* Ready Receive Buffer Descriptor */
    RxBD(ON);

    /*
     *     Setup Read Request
     *     Configure Tx BD 0 for: Wrap, Interrupt, Last, Start bits
     *     Byte0: Device Address and Read Bit
     *     numChars: Unintialized Dummy Bytes to Toggle Serial Clock during Read
     */

    TXBDcstatusInit(TXBD_0, TXBD_W | TXBD_I | TXBD_S | TXBD_L);

    /* Store 7-bit device address and write-bit */
    txArr[0] = (devAddr << 1) | READ_BIT;

    /* Store Read Request in Tx BD 0 */
    I2CwriteArr(TXBD_0, txArr, 1 + numChars);

    /* Transmit Random Read Message: Read Request */
    if ((transmitStatus = I2Ctransmit(1)) == OK) 
    { 
      /* Read Data */
      if (readBD(outch, numChars) == numChars) 
        status = OK;
    }
  }

    /* Set the task back to old task priority */
    if (taskPrioritySet(taskIdSelf(),myOldTaskPriority) != OK)
      logMsg("I2Cwrite: taskPrioritySet myOldTaskPriority failed\n",0,0,0,0,0,0);

/*  taskDelay(10); */
  semGive(i2cSem);
/*  return OK; */
/*
  if (outch[0] != '*')
    printf("I2Cread2: %c,%c,%c,%c\n",outch[0],outch[1],outch[2],outch[3]);
*/
  return status;
}


void dumpI2CBD(void)
{
  RTXBD *rtx = ( RTXBD * ) bdGet();
  int i;

  printf("\nRx buffer\n");
  printf("---------\n");
  printf("Status(%x)\n", rtx->rxbd.bd_cstatus);
  printf("Buffer : ");
  for(i = 0; i < rtx->rxbd.bd_length; i++)
      printf("%#04x  ",  rtx->rxbd.bd_addr[i]);
      
  printf("\n\nTx buffer 0\n");
  printf("-------------\n");
  printf("Status(%x)\n", rtx->txbd[0].bd_cstatus);
  printf("Buffer : ");
  for(i = 0; i < rtx->txbd[0].bd_length; i++)
      printf("%#04x  ",  rtx->txbd[0].bd_addr[i]);
  
  printf("\n\nTx buffer 1\n");
  printf("-------------\n");
  printf("Status(%x)\n", rtx->txbd[1].bd_cstatus);
  printf("Buffer : ");
  for(i = 0; i < rtx->txbd[1].bd_length; i++)
      printf("%#04x  ",  rtx->txbd[1].bd_addr[i]);
  printf("\n");
}

void dumpI2CRegs(void)
{
  void *vxImmr = (void *) vxImmrGet();

  printf("I2C Registers\n");
  printf("-------------\n");
  printf("I2MOD(%#x)\n", *I2MOD(vxImmr));
  printf("I2ADD(%#x)\n", *I2ADD(vxImmr));
  printf("I2BRG(%#x)\n", *I2BRG(vxImmr));
  printf("I2COM(%#x)\n", *I2COM(vxImmr));
  printf("I2CER(%#x)\n", *I2CER(vxImmr));
  printf("I2CMR(%#x)\n", *I2CMR(vxImmr));
  printf("I2C_RBASE(%#x)\n", *I2C_RBASE(vxImmr));
  printf("I2C_TBASE(%#x)\n", *I2C_TBASE(vxImmr));
  printf("I2C_RFCR(%#x)\n", *I2C_RFCR(vxImmr));
  printf("I2C_TFCR(%#x)\n", *I2C_TFCR(vxImmr));
  printf("I2C_MRBLR(%#x)\n", *I2C_MRBLR(vxImmr));
  printf("I2C_RSTATE(%#x)\n", *I2C_RSTATE(vxImmr));
  printf("I2C_RBPTR(%#x)\n", *I2C_RBPTR(vxImmr));
  printf("I2C_TSTATE(%#x)\n", *I2C_TSTATE(vxImmr));
  printf("I2C_TBPTR(%#x)\n", *I2C_TBPTR(vxImmr));
}

void dumpI2CCnts()
{
  printf("I2C Counters\n");
  printf("---------------\n");
  printf("rxbEvCnt %d\n", rxbEvCnt);
  printf("txbEvCnt %d\n", txbEvCnt);
  printf("txeEvCnt %d\n", txeEvCnt);
  printf("bsyEvCnt %d\n", bsyEvCnt);
  printf("multEvCnt %d\n", multEvCnt);
  printf("totEvCnt %d\n", totEvCnt);
  printf("prevTxbEvCnt %d\n", prevTxbEvCnt);
}

