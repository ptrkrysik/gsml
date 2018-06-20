/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/* *******************************************************************/

/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : cdcUtils.h                                          */

/* *******************************************************************/

#ifndef cdcUtils_H
#define cdcUtils_H


/*---- Function Declarations ----*/

/*******************************************************************************
cdcSerialNumReturn - CDC Serial Number Return

*/
char* cdcSerialNumReturn();

/*******************************************************************************
cdcMacAddrGet - CDC MAC Address Get

*/
char* cdcMacAddrGet();

void dspReset(void);

int ldCoff(char *file, char *base_prefix);

STATUS  LoadDSPCode(char *fileName, char *base_prefix);


#endif /* cdcUtils_H */
