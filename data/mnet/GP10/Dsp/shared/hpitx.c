/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: hpirx.c
*
* Description:
*   This file allocates memory for variables and buffers used for Tx HPI.
*   The linker command file assigns these to the appropriate memory location. 
*
******************************************************************************/
#include "dsp/dsptypes.h"

volatile Int32 DspMsgNumWord;
Uint8          txMsgBuf[0x200];

