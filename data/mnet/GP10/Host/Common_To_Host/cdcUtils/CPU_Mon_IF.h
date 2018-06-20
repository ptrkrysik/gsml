/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/********************************************************************
*                          
*  FILE NAME:	CPU_Mon_IF.h    
*                      
*  DESCRIPTION: This file contains extern prototypes for the DS1780 code.               
*                                  
*  NOTES:                              
*                                  
*  VERSION:  1.0  10/06/99  14:45:15
*
*  SCCS ID:  "@(#)CPU_Mon_IF.h"
*
********************************************************************/

#if !defined(CPU_MON_IF_H)
#define CPU_MON_IF_H

#include "dsp/dsptypes.h"
#include "CPU_Mon.h"

/*  CPU_Mon.c Extern Prototypes  */

extern STATUS  DS1780_Init					(void);
extern STATUS  DS1780_Poll				    (void);
extern STATUS  DS1780_Read                  (t_DS1780 *data);
extern STATUS  DS1780_Set_Voltage_Thresholds(UINT8	Threshold_ID, UINT8 High, UINT8 Low);
extern STATUS  DS1780_Set_Temp_Thresholds   (UINT8 High, UINT8 Low);

#endif  /* CPU_MON_IF_H */
