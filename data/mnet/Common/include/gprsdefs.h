#ifndef __GPRSDEFS_H__
#define __GPRSDEFS_H__
/*********************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : gprsefs.h
// Author(s)   : Igal Gutkin
// Create Date : 10/31/00
// Description : Shared definitions for GPRS modules
//
// Notes       : IMPORTANT - DO NOT use C++ style comments "//" in this file 
//
// *******************************************************************/


#define MAX_L3_PDU_LENGTH           (1520)      /* max byte size in a L3 GPRS PDU */
#define MAX_LLC_OVERHEAD_LENGTH     (36)        /* max byte size for LLC frame overhead */

/* max bytes in a DL GPRS PDU */
#define MAX_DL_PDU_LENGTH           (MAX_L3_PDU_LENGTH+MAX_LLC_OVERHEAD_LENGTH)

#define TLLI_UNASSIGN_VALUE         (0xFFFFFFFF)

typedef UINT32 TLLI_t;

#endif /* __GPRSDEFS_H__ */