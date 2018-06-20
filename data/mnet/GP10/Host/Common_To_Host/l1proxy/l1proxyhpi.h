/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/****************************************************************************
* File: l1proxyhpi.h
*
* Description:   
*   Definitions & Prototypes for the hpi communications
*
*****************************************************************************/

#ifndef L1PROXYHPI_H
#define L1PROXYHPI_H

/*
 * HPI Hardware register definitions
 */
#define PORT0_BASE_ADDR 0xe0000000
#define PORT1_BASE_ADDR 0xe0010000
#define AUX_PORT        (*(volatile unsigned long *) 0xe1000000)
#define PORT_BASE(port) (port == 0 ? PORT0_BASE_ADDR : PORT1_BASE_ADDR)
#define HPIC(port)      (*(volatile unsigned int *)(PORT_BASE(port)+0x00))
#define HPIA(port)      (*(volatile unsigned int *)(PORT_BASE(port)+0x08))
#define HPID(port)      ( (volatile         void *)(PORT_BASE(port)+0x0c))
#define HPIS(port)      ( (volatile         void *)(PORT_BASE(port)+0x04))


/*
 * hpi2dspD is a layer 1 utility routine, transferring a 32-bit word
 * from the vxWorks host to the DSP.
 */

void hpi2dspD(unsigned int port, unsigned int addr, unsigned int data);


/*
 * dsp2hpiD is a layer 1 utility routine, transferring a 32-bit word
 * from the DSP to the vxWorks host.
 */

unsigned int dsp2hpiD(unsigned int port, unsigned int addr);


#endif /* L1PROXYHPI_H */
