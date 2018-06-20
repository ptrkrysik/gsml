/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: gplink_prot.h													*
 *																						*
 *	Description			: Definitions for the GPLink Protocol							*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |01/25/01 | DSN    | File created												*
 *		 |		   |		|															*
 ****************************************************************************************
 */
#ifndef GPLINK_PROT_HDR_INCLUDE
#define GPLINK_PROT_HDR_INCLUDE


#define GPLINK_TCP_PORT             	52172


typedef enum {

    GPLINK_MSG_TYPE_REG_REQ         = 1,
    GPLINK_MSG_TYPE_REG_CNF         = 2,
    GPLINK_MSG_TYPE_REG_REJ         = 3,
    GPLINK_MSG_TYPE_REG_UPDATE_REQ  = 4,
    GPLINK_MSG_MAX_TYPES            = 5

} GPLINK_PROTOCOL_MSG_TYPE;

typedef enum {

    GPLINK_IE_CELL_ID               = 1,
    GPLINK_IE_LOCATION_AREA_ID      = 2,
    GPLINK_IE_ROUTING_AREA_ID       = 3,
    GPLINK_IE_CAUSE                 = 4,
    GPLINK_MAX_IE_TYPES             = 5

} GPLINK_IE_TYPE;


typedef enum {

    GPLINK_EVENT_CELL_CONNECTED		= 0,
    GPLINK_EVENT_CELL_DISCONNECTED	= 1,
    GPLINK_EVENT_CELL_INFO_UPDATE	= 2,
	GPLINK_MAX_EVENTS				= 3

} GPLINK_EVENT;


typedef enum {
	GPLINK_GS_CAPACITY_EXCEEDED		= 0
} GPLINK_REJECT_CAUSE;


#endif /* #ifndef GPLINK_PROT_HDR_INCLUDE */