/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_MARCO_H__
#define __RM_MARCO_H__

#define RM_ISTCHF(X)		(((X)&0xF8)==0x08)
#define RM_ISSDCCH4(X)		(((X)&0xE0)==0x20)

#define RM_TCHF_ENTRYID(X,Y)	(   (((X)<<3)&0x08) | ((Y)&0x07) ) 
#define RM_SDCCH4_ENTRYID(X,Y)  ((( (((X)<<5)&0x20) | ((Y)&0x07) ) << 2) | (((Y)>>3)&0x03) )

#define RM_MEMSET( X,Y )	memset(X, (int) 0, (size_t) Y )
#define RM_MEMCPY( X,Y,Z )	memcpy(X, Y, Z)

#define RM_BOOTSTRAPCNF(X)	(1<<(X))

#define RM_TCHF_ENTRYIDX(X,Y)	    ((((X)<<3)&0x08)|((Y)&0x07)) 
#define RM_SDCCH4_ENTRYIDX(X,Y)   (((((X)<<5)&0x20)|((Y)&0x07))<<2)|(((Y)>>3)&0x03) 

#define RM_L1MSGCMP(X,Y,Z)  \
        ((X==rm_pItcRxL1Msg->l3_data.buffer[0])&& \
         (Y==rm_pItcRxL1Msg->l3_data.buffer[1])&& \
         (Z==rm_pItcRxL1Msg->l3_data.buffer[2]) )

#define RM_DSPA52(X)			(X & 2)
#define RM_DSPA51(X)			(X & 1)

#endif //__RM_MARCO_H__
