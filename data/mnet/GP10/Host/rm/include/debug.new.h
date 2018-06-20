/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "stdio.h"

#define ROW_LEN			10

/*
** MARCO definitions for dubug purpose
*/

#define MAREA__			0
#define RXMSG__			1
#define TXMSG__			2


#define RDEBUG__(X)		if (__RDEBUG__) \
					    printf X;   \
					else            \
					    __DEBUGVAR__ = 0	

#define EDEBUG__(X)		if (__EDEBUG__) \
                                  printf X;   \
					else		    \	
					    __DEBUGVAR__ = 0

#define DDEBUG__(X)		if (__DDEBUG__) \
                                  printf X;   \
					else		    \
					    __DEBUGVAR__ = 0

#define UDEBUG__(X)		if (__UDEBUG__)  \
					    printf X;    \
					else		     \
					    __DEBUGVAR__ = 0

#define PDEBUG__(X)		if (__PDEBUG__)  \
                                  printf X;    \
                              else             \
                                  __DEBUGVAR__ = 0

#define TDEBUG__(X)		if (__TDEBUG__)  \
                                  printf X;    \
                              else             \
                                  __DEBUGVAR__ = 0

#define BDUMP__(X)		if (__BDUMP__)   \
                                  db_BDump X;  \
				      else            \
                                  __DEBUGVAR__ = 0
      
#define EDUMP__(X)		if (__EDUMP__)  \
                                   db_BDump X; \
                              else            \
                                   __DEBUGVAR__ = 0

#define UDUMP__(X)		if (__UDUMP__)   \
                                  db_BDump X;  \
                              else             \
                                  __DEBUGVAR__ = 0

#define TDUMP__(X)	      if (__TDUMP__)   \
                                  db_BDump X;  \
                              else             \
                                  __DEBEGVAR__ = 0

void    db_BDump ( unsigned char, unsigned char *, unsigned short );

#endif /* __DEBUG_H__ */

