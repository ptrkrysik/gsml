/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

#include <vxWorks.h>
#include <semLib.h>
#include "cdcUtils/auxPort.h"

/* external */
extern int sysAuxPortAddr;

/* global */

/* file local */
static int auxPortOutCopy;

void auxPortInit()
{
  auxPortOutCopy = LED1_OFF | LED2_OFF | LED3_OFF | FAULT_LED_OFF |
                   RF1_EN | RF2_EN | RF_RESET_NOT |
                   DSPA_RST_NOT | DSPB_RST_NOT |
                   FPGA_DCLK | FPGA_DATA;
  *(UINT*)sysAuxPortAddr = auxPortOutCopy;
}


/******************************************************************************
auxPortOutSet - aux. port output set

This routine provides a mechanism to set certain bits on the 'write only' 
auxillary output port.
Set the given vaue at the auxillary output port.
A bit mask is supplied to indicate which bits are to be set. A '1' in the mask
indiactes that value for that bit position is to be set. A '0' means that the 
value for that bit position is unchanged.

This routine provides a mechanism to set certain bits on the 'write only' 
auxillary output port. This fuction maintance a copy of the aux. output and 
provides mutual exclusion.

This routine should be called to write to the auxillary output port. This is 
*/
void auxPortOutSet
(
  int val,           /* value to write to aux out port */
  int mask           /* bit mask with which to set the value */
)
{
  int lock;


/*  semTake(auxPortSem, WAIT_FOREVER); */
  taskLock();

  lock = intLock();  /* lock interrupts */ 
               /* This mechanism is used for mutual exclusion instead of using 
                  a semaphore to avoid overhead to guard very small code */
  auxPortOutCopy = (auxPortOutCopy & ~mask) | (val & mask); /* save the copy */
  *(UINT*)sysAuxPortAddr = auxPortOutCopy;   /* write to the port */
  intUnlock(lock);   /* unlock interrupts */

/*  semGive(auxPortSem); */
  taskUnlock();

  
}

/******************************************************************************
auxPortOutGet - aux Port Output Get

Returns the value of aux. output port
*/
UINT auxPortOutGet()
{
  return auxPortOutCopy;
}

