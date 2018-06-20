/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_MARCO_H__
#define __GRR_MARCO_H__

#define GRR_MEMSET( X,Y )			memset(X, (int) 0, (size_t) Y )
#define GRR_MEMCPY( X,Y,Z )			memcpy(X, Y, Z)
#define GRR_L1MSGCMP(X,Y,Z)  	    \
									((X==grr_pItcRxDspMsg->buffer[0])&& \
									(Y==grr_pItcRxDspMsg->buffer[1])&& \
									(Z==grr_pItcRxDspMsg->buffer[2]) )

#define GRR_DSPA52(X)				(X & 2)
#define GRR_DSPA51(X)				(X & 1)


#endif //__GRR_MARCO_H__
