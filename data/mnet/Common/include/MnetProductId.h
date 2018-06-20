/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : ProductId.h                                         */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 9/18/2000                                           */
/* Description : This file contains MNET product IDs                 */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |10/16/00| Initial Draft                                 */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


#if !defined(_MNETRODUCTID_H_)
#define _MNETRODUCTID_H_

typedef enum {
    MNET_PRODUCT_GP10,
    MNET_PRODUCT_GMC,
    MNET_PRODUCT_GS,
    MNET_PRODUCT_ITP,
    MNET_PRODUCT_MAXID          // always needs to be the last in this 
}MNETProductId_t;


#ifdef  __cplusplus
inline 
#else
static 
#endif
char *GetMnetProductName(MNETProductId_t id)
{

    switch (id)
    {
    case MNET_PRODUCT_GP10:
        return "Gsm Port 10 Series (GP10)";
        break;
        
    case MNET_PRODUCT_GMC:
        return "GSM Mobility Controller (GMC)";
        break;
        
    case MNET_PRODUCT_GS:
        return "Serving GPRS Support Node (SGSN)";
        break;
        
    case MNET_PRODUCT_ITP:
        return "IP Transfer Point (ITP)";
        break;
    }
 
        return("Unknown Product ID");
}


#ifdef  __cplusplus
inline 
#else
static 
#endif

char *GetMnetProductRelativeDirName(MNETProductId_t id)
{
    
    switch (id)
    {
    case MNET_PRODUCT_GP10:
        return "Gp10";
        break;
        
    case MNET_PRODUCT_GMC:
        return "Gmc";
        break;

    case MNET_PRODUCT_GS:
        return "Gs";
        break;
        
    case MNET_PRODUCT_ITP:
        return "Itp";
        break;
    }
    
    return("Unknown");
}

#endif /* _MNETRODUCTID_H_ */
