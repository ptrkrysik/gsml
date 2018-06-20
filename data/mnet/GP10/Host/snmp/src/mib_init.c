/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************
**                          
**  FILE NAME: mib_init.c   
**                      
**  DESCRIPTION: This file contains mib initialization routines
**      
**  COMPONENTS:                      
**                              
**                                  
**  NOTES:                              
**                             
**  REVISION HISTORY                            
**  __________________________________________________________________
**  ----------+--------+----------------------------------------------
**  Name      |  Date  |  Reason                
**  ----------+--------+----------------------------------------------
**  Bhawani   |02/28/01| Change to use unify mib change part
**  ----------+--------+----------------------------------------------
**********************************************************************
*/

/*
**********************************************************************
** Include Files. 
**********************************************************************
*/
#include "oam_api.h"

/* Add default initialization routine here */

STATUS snmp_initDefaultMib() 
{
	return STATUS_OK;
}

/* porduct specific post initialization mib routine */
STATUS snmp_postLoadMibHandler()
{
   UINT32 value;
   if (snmp_getValue(MIB_viperCellDefGateway) == 0)
   {
      value = snmp_getValue(MIB_h323_Q931ConnectTimeOut);
      
      if ( value > 0x01000000)
      {
         snmp_setEntry(MIB_viperCellDefGateway, 0, (void *) value, 4);
         snmp_setEntry(MIB_h323_Q931ConnectTimeOut, 0, (void *) 100, 4);
      }
   }
   return STATUS_OK;
}
