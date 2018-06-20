/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*******************************************************************************
Altera FPGA configuration 

fpgaConfig - Configures FPGAs
*******************************************************************************/

/* 
Revision History 
---------------- 
99/10/17  ck  Modified to use auxPortOutSet function
99/05/03  ck  Initial Version 
*/

#include <vxworks.h>
#include <stdlib.h>
#include <stdio.h>
#include <taskLib.h>
#include <vxLib.h>
#include <drv/multi/ppc860siu.h>
#include "cdcUtils/auxPort.h"

#define CONFIG_DONE_BIT 0x00010000

/******************************************************************************
fpgaConfig

Configure FPGAs. This routine is called to configure the FPGAs. It is called
once after system boots. It reads the data to be programmed from file fpga.rbf

RETURNS
   0  -  Success
         Errors
  -1  -  CONFIG_DONE does not go low 
  -2  -  CONFIG_DONE does not go high at the end of configuration

 -100 -  Error Reading file
 -200 -  Cannot allocate memory

External Signals
   In  - CONFIG_DONE on port B
   Out - NCONFIG on Aux. Port
   Out - DCLK on Aux. Port
   Out - DATA on Aux. Port

******************************************************************************/

int fpgaConfig
  (
    const char* fileName
  )
{

  FILE*  file;           /* input file */
  char*  buff;
  int    buffSize;
  int    byteIndex;
  int    bitNum;
  char   outByte;
  int    returnVal;
  char   i;
  int    immrVal = vxImmrGet();
      
  /* read the file */
  file = fopen(fileName,"rb");
  if (file == 0)
  {
    returnVal = -100;
    goto exit2;
  } 
  fseek(file,0,SEEK_END);
  buffSize = ftell(file);
  fseek(file,0,SEEK_SET); 
  buff = (char *)malloc(buffSize);
  if (buff == 0)
  {
    returnVal = -200;
    fclose(file);
    goto exit2;
  }
  fread(buff,1,buffSize,file);
  fclose(file);

  auxPortOutSet(FPGA_NCONFIG, FPGA_NCONFIG);  /* NCONFIG high */
  for(i=0;i<100;i++);        /* keep NCONFIG high for a while */

 /* NCONFIG low,DCLK low */
  auxPortOutSet(~(FPGA_NCONFIG|FPGA_DCLK), FPGA_NCONFIG|FPGA_DCLK); 
  for(i=0;i<100;i++);   /* wait at least 2us */
  auxPortOutSet(FPGA_NCONFIG, FPGA_NCONFIG); /* NCONFIG high */
  for(i=0;i<250;i++);   /* wait at least 5us */

  if (*PBDAT(immrVal) & CONFIG_DONE_BIT)
  {        
    returnVal = -1;
    goto exit1;  
  }
 
  for (byteIndex = 0; byteIndex < buffSize; byteIndex++)  
  {

    /* check if CONFIG_DONE goes high early */
    /* commented out because CONFIG_DONE goes hiag a few bits before 
       is transferred */
/*
    if ((*PBDAT(immrVal) & CONFIG_DONE_BIT)) 
    {     
      returnVal = -3;
      goto exit;
    }
*/
    outByte = buff[byteIndex];
    for (bitNum = 0; bitNum < 8; bitNum++)
    {
      auxPortOutSet((outByte << 6) & ~FPGA_DCLK, FPGA_DATA|FPGA_DCLK); /* DATA, DCLK low */ 
                /* wait minimum 50ns */
      for(i=0;i<4;i++);        
      auxPortOutSet(FPGA_DCLK, FPGA_DCLK); /* DCLK high */
                /* wait minimum 50ns */
      for(i=0;i<4;i++); 

      outByte = outByte >> 1;

    }
  }
  
  /* at this point CONFIG_DONE should be high */
  if (!(*PBDAT(immrVal) & CONFIG_DONE_BIT))
  {          /* CONFIG_DONE low */
    returnVal = -2;
    goto exit1;
  }

  /* clock ten extra cycles */
  for (byteIndex = 0; byteIndex < 10; byteIndex++) 
  {
    auxPortOutSet(~FPGA_DCLK, FPGA_DCLK) ; /* DCLK low */ 
    for (i=0;i<4;i++);     /* wait minimum 50ns */
    auxPortOutSet(FPGA_DCLK, FPGA_DCLK); /* DCLK high */
    for (i=0;i<4;i++);     /* wait minimum 50ns */;
  }
  returnVal = 0;

  exit1: 
  free(buff);
  exit2:
  return(returnVal);
}
