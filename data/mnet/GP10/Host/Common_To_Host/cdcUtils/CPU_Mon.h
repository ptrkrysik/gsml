/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/********************************************************************
*                          
*  FILE NAME:	CPU_Mon.h    
*                      
*  DESCRIPTION: This file contains definitions for the DS1780 operation.               
*                                  
*  NOTES:                              
*                                                                   
*  VERSION:  1.0  10/06/99  14:45:15
*
*  SCCS ID:  "@(#)CPU_Mon.h"
*
********************************************************************/

#if !defined(CPU_MON_H)
#define CPU_MON_H

typedef struct
{
	UINT8	_2_5_V_Mon_Value;
	UINT8	_3_3_V_Mon_Value;
	UINT8	_5_V_Mon_Value;

	UINT8	Temp_Mon_Value;
	UINT8	Chassis_Intrusion_Status;

} t_DS1780;


#endif  /* CPU_MON_H */
