/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*******************************************************************
*/

#ifndef __RM_CONST_H__
#define __RM_CONST_H__

// Options for RM Task 

#define RM_TASK_PRIORITY			70
#define RM_STACK_SIZE				20000
#define RM_TASK_OPTION          		0

//General purpose constants

#define RM_TRUE					1 
#define RM_FALSE				0
#define RM_U8_0X01				0x01
#define RM_U8_0X08				0x08
#define RM_U8_0X07				0x07
#define RM_U8_0X0F				0x0F
#define RM_U8_0X80				0x80
#define RM_U8_0X88				0x88
#define RM_U8_0X90				0x90
#define RM_U8_0X04				0x04

// Constants for internal use

#define RM_T3101				0
#define RM_T3111				1
#define RM_T3109				2
#define RM_T3L01				3
#define RM_T3105				4
#define RM_TWDOG				5
#define RM_T3L02				6

// Constants for misc purposes

#define RM_LINKED		        	0
#define RM_RSLIND				1
#define RM_MDLERR				1
#define RM_LOCKED				10
#define RM_SAPI0				0
#define RM_SAPI3				3
#define RM_SACCHBAD_POS				0
#define RM_RSLCOUNT_BOUND			10
#define RM_SI		    			0			
#define RM_PHYCHAN_QUALITY_PREALPHA		0xff

//AGCH groups as per BCCH-SDCCH comb or not

#define RM_L1MAX_AGCH_GROUPS_COMB	        2
#define RM_L1MAX_AGCH_GROUPS_UNCOMB		8

// Establishe causes specified by GSM 04.08 Channel Request message

#define RM_CHANREQ_COS_LUP 			0x00
#define RM_CHANREQ_COS_MOC 			0xE0
#define RM_CHANREQ_COS_MTC			0x80
#define RM_CHANREQ_COS_EMC			0xA0
#define RM_CALLTYPE_LUP	  			0x00
#define RM_CALLTYPE_MOC	 			0xE0
#define RM_CALLTYPE_MTC   			0x80

// Max # of physical channel resources

#define RM_L1MAX_BCCCHS			        4
#define RM_L1MAX_SDCCH4S			4
#define RM_L1MAX_TCHFS				15
#define RM_MAX_SMCTXTS				0x30

// Index to rm_PhyChanBcchCcch[]

#define RM_BCCH_IDX				0
#define RM_RACH_IDX				1
#define RM_PCH_IDX				2
#define RM_AGCH_IDX				3

// Physical channel status

#define RM_PHYCHAN_UNUSABLE			0
#define RM_PHYCHAN_USABLE			1
#define RM_PHYCHAN_CBCH_USABLE			2
#define RM_PHYCHAN_FREE				0
#define RM_PHYCHAN_ASSIGNED			1
#define RM_PHYCHAN_RELEASING			2

// SM Ctxt types

#define RM_MAIN_SM							1
#define RM_ACCH_SM							2

// States of DSP initialization

#define RM_TRXMGMT_SNULL			0
#define RM_TRXMGMT_S0_BOOTSTRAP_IND		1
#define RM_TRXMGMT_S1_TRXCONFIG_ACK		2
#define RM_TRXMGMT_S2_SLOTACTIV_ACK		3
#define RM_TRXMGMT_S3_TRXCONFIG_END		4

// RM State Machine states

#define RM_S0NULL			        0
#define RM_S1RM_RUNNING				1
#define RM_S2ACTIVATING_SIGTRAFFIC_CHAN		2
#define RM_S3SIGCHAN_ACTIVATED			3
#define RM_S4TRAFFICCHAN_ACTIVATED		4
#define RM_S5SIGMODE_ESTABLISHED		5
#define RM_S6ACTIVATING_TRAFFICMODE		6
#define RM_S7RELEASING_SIGTRAFFICCHAN		7
#define RM_S8TRAFFICMODE_ACTIVATED		8
#define RM_S9DEACTIVATING_SIGTRAFFICCHAN	9
#define RM_S10TRAFFICMODE_ESTABLISHED		10
#define RM_S11TRAFFICCHAN_SUSPENDING		11

// Definitions referring to Frontier L1 Interface Spec

#define RM_SAVCHANREQ_LEN 			8
#define RM_L1MIN_MSG_LEN			4

// L1 message group
#define RM_L1MG_TRXMGMT  			0x10
#define RM_L1MG_COMCHAN				0x0C
#define RM_L1MG_DEDMGMT				0x08
#define RM_L1MG_LNKMGMT				0x02
#define RM_L1MG_OAM 				0x80

// L1 message types 
#define RM_L1MT_CBCHRTS_MSB			0x03
#define RM_L1MT_CBCHRTS_LSB			0x00
#define RM_L1MT_PING_MSB			0x13
#define RM_L1MT_PING_LSB			0x00
#define RM_L1MT_PINGACK_MSB			0x13
#define RM_L1MT_PINGACK_LSB			0x01
#define RM_L1MT_ARFCN_MSB			0x29
#define RM_L1MT_ARFCN_LSB			0x00

#define RM_L1MT_NULL				0xFFFF
#define RM_L1MT_NULL_MSB			0XFF
#define RM_L1MT_NULL_LSB			0xFF
#define RM_L1MT_BCCHINFO			0x0011
#define RM_L1MT_BCCHINFO_MSB			0x00
#define RM_L1MT_BCCHINFO_LSB			0x11
#define RM_L1MT_SYNCH				0x001D
#define RM_L1MT_SYNCH_MSB			0x00
#define RM_L1MT_SYNCH_LSB			0x1D
#define RM_L1MT_SYNCHACK			0x001E
#define RM_L1MT_SYNCHACK_MSB			0x00
#define RM_L1MT_SYNCHACK_LSB			0x1E

#define RM_L1MT_TRXCONFIG			0x1500
#define RM_L1MT_TRXCONFIG_MSB			0x15
#define RM_L1MT_TRXCONFIG_LSB			0x00
#define RM_L1MT_TRXCONFIGACK			0x1600
#define RM_L1MT_TRXCONFIGACK_MSB		0x16
#define RM_L1MT_TRXCONFIGACK_LSB		0x00
#define RM_L1MT_SACCHFILL			0x001A
#define RM_L1MT_SACCHFILL_MSB			0x00
#define RM_L1MT_SACCHFILL_LSB			0x1A
#define RM_L1MT_RFRESIND			0x0019
#define RM_L1MT_RFRESIND_MSB			0x00
#define RM_L1MT_RFRESIND_LSB			0x19
#define RM_L1MT_OVERLOAD			0x001B
#define RM_L1MT_OVERLOAD_MSB			0x00
#define RM_L1MT_OVERLOAD_LSB			0x1B
#define RM_L1MT_SLOTACTIV			0x1700
#define RM_L1MT_SLOTACTIV_MSB			0x17
#define RM_L1MT_SLOTACTIV_LSB			0x00
#define RM_L1MT_SLOTACTIVACK			0x1800
#define RM_L1MT_SLOTACTIVACK_MSB		0x18
#define RM_L1MT_SLOTACTIVACK_LSB		0x00
#define RM_L1MT_SLOTACTIVNACK			0x1900
#define RM_L1MT_SLOTACTIVNACK_MSB		0x19
#define RM_L1MT_SLOTACTIVNACK_LSB		0x00
#define RM_L1MT_SETUGAIN			0x1000
#define RM_L1MT_SETUGAIN_MSB			0x10
#define RM_L1MT_SETUGAIN_LSB			0x00
#define RM_L1MT_SETDGAIN			0x1100
#define RM_L1MT_SETDGAIN_MSB			0x11
#define RM_L1MT_SETDGAIN_LSB			0x00
#define RM_L1MT_TUNESYNTH			0x1200
#define RM_L1MT_TUNESYNTH_MSB			0x12
#define RM_L1MT_TUNESYNTH_LSB			0x00
#define RM_L1MT_CONFIGFPGADELAY			0x14
#define RM_L1MT_CONFIGFPGADELAY_MSB		0x14
#define RM_L1MT_CONFIGFPGADELAY_LSB		0x00
#define RM_L1MT_BOOTSTRAPCNF			0x2000
#define RM_L1MT_BOOTSTRAPCNF_MSB  		0x20
#define RM_L1MT_BOOTSTRAPCNF_LSB  		0x00
#define RM_L1MT_CHANACTIV			0x0021
#define RM_L1MT_CHANACTIV_MSB			0x00
#define RM_L1MT_CHANACTIV_LSB			0x21
#define RM_L1MT_CHANACTIVACK			0x0022
#define RM_L1MT_CHANACTIVACK_MSB		0x00
#define RM_L1MT_CHANACTIVACK_LSB		0x22
#define RM_L1MT_CHANACTIVNACK			0x0023
#define RM_L1MT_CHANACTIVNACK_MSB		0x00
#define RM_L1MT_CHANACTIVNACK_LSB		0x23
#define RM_L1MT_RFCHANREL			0x002E
#define RM_L1MT_RFCHANREL_MSB			0x00
#define RM_L1MT_RFCHANREL_LSB			0x2E
#define RM_L1MT_RFCHANRELACK			0x0033
#define RM_L1MT_RFCHANRELACK_MSB		0x00
#define RM_L1MT_RFCHANRELACK_LSB		0x33
#define RM_L1MT_RSL_MSB				0x00
#define RM_L1MT_RSL_LSB				0x35
#define RM_L1MT_MEASRES				0x0028
#define RM_L1MT_MEASRES_MSB			0x00
#define RM_L1MT_MEASRES_LSB			0x28
#define RM_L1MT_MODEMODIFY			0x0029
#define RM_L1MT_MODEMODIFY_MSB			0x00
#define RM_L1MT_MODEMODIFY_LSB			0x29
#define RM_L1MT_ENCRYPTION			0x0026
#define RM_L1MT_ENCRYPTION_MSB			0x00
#define RM_L1MT_ENCRYPTION_LSB			0x26
#define RM_L1MT_MODEMODIFYACK			0x002A
#define RM_L1MT_MODEMODIFYACK_MSB		0x00
#define RM_L1MT_MODEMODIFYACK_LSB		0x2A
#define RM_L1MT_MODEMODIFYNACK			0x002B
#define RM_L1MT_MODEMODIFYNACK_MSB		0x00
#define RM_L1MT_MODEMODIFYNACK_LSB		0x2B
#define RM_L1MT_DEACTIVSACCH			0x0025
#define RM_L1MT_DEACTIVSACCH_MSB		0x00
#define RM_L1MT_DEACTIVSACCH_LSB		0x25
#define RM_L1MT_PHDATAREQ_MSB			0x00
#define RM_L1MT_PHDATAREQ_LSB			0x01
#define RM_L1MT_ASYNCHO_REQUIRED_MSB		0x00
#define RM_L1MT_ASYNCHO_REQUIRED_LSB		0x36
#define RM_L1MT_AYSNCHO_REQ_MSB			0x00
#define RM_L1MT_AYSNCHO_REQ_LSB			0x37

// Channel Number IE 
#define RM_L1TCHF_CHANNUMBER_TYPE		0x08
#define RM_L1SDCCH4_CHANNUMBER_TYPE		0x20
#define RM_L1RACH_CHANNUMBER_MSB		0x88
#define RM_L1RACH_CHANNUMBER_LSB		0x00 
#define RM_L1PCH_CHANNUMBER_MSB 		0x90 
#define RM_L1PCH_CHANNUMBER_LSB 		0x00 
#define RM_L1AGCH_CHANNUMBER_MSB		0x90
#define RM_L1AGCH_CHANNUMBER_LSB		0x00
#define RM_L1BCCH_CHANNUMBER_MSB		0x80
#define RM_L1BCCH_CHANNUMBER_LSB		0x00
#define RM_L1CBCH_CHANNUMBER_MSB		0xa0
#define RM_L1CBCH_CHANNUMBER_LSB		0x00

// Channel Mode IE
#define RM_L1ACTTYPE_IMMASS			0x00
#define RM_L1ACTTYPE_ASYNC_HO		      0x02
#define RM_L1ACTTYPE_CBCH			0x06

// DTX & Date Rate 
#define RM_L1DTXCTRL_D0U0			0x00
#define RM_L1DATATTR_SPEECH			0x01	
#define RM_L1DATATTR_DATA			0x02
#define RM_L1DATATTR_SIGNALING			0x03
#define RM_L1RATTYPE_SDCCH			0x01
#define RM_L1RATTYPE_TCHF			0x08
#define RM_L1CODALGO_NORES			0x00
#define RM_L1CODALGO_GSMV1			0x01
#define RM_L1CODALGO_GSMV2			0x21
#define RM_L1CODALGO_GSMV3			0x41

// Encryption
#define RM_L1CIPHTYPE_NCIPH			0x01
#define RM_L1CIPHKEY_LEN			9
	
// Power control and TA
#define RM_L1BSPOWER_PREALPHA			0  
#define RM_L1MSPOWER_PREALPHA			0

// Trx number IE
#define RM_L1MAX_TRXS			        2
#define RM_L1TRX_NULL				255
#define RM_L1TRX_0			  	0
#define RM_L1TRX_1				1
#define RM_PREALPHA_TRX				0

// System information IE
#define RM_L1MAX_SYSINFOS			8
#define RM_L1MAX_SYSINFO_LEN			24
#define RM_L1SYSINFO_TYPE_8			0x00
#define RM_L1SYSINFO_TYPE_1			0x01
#define RM_L1SYSINFO_TYPE_2			0x02
#define RM_L1SYSINFO_TYPE_3			0x03
#define RM_L1SYSINFO_TYPE_4			0x04
#define RM_L1SYSINFO_TYPE_5			0x05
#define RM_L1SYSINFO_TYPE_6			0x06
#define RM_L1SYSINFO_TYPE_7			0x07

// Synthesizer setting 
#define RM_L1UL_SYNTH_A				0
#define RM_L1UL_SYNTH_B				1
#define RM_L1DL_SYNTH_A				2
#define RM_L1DL_SYNTH_B				3

// Link Identifer IE
#define RM_L1LINK_MAIN				0x00				
#define RM_L1LINK_ACCH				0x40
#define RM_L1LINK_VOID				0x20

// Air Interface standard IE
#define RM_L1PCS1900				0x02

// Channel Sync 
#define RM_L1SYNCH_CHANINFO_LEN			4


// Definition added for OAM message group
#define TOTAL_AVAILABLE_CELL 			3
#define TOTAL_AVAILABLE_TRX  			1

#define OAM_MANAGE_MSG   			0x0C<<1
#define OAM_DPC_CONFIG             		0x5000
#define OAM_DPC_CONFIG_ACK         		0x5001
#define OAM_HO_SCELL_CONFIG        		0x5002
#define OAM_HO_SCELL_CONFIG_ACK    		0x5003
#define OAM_HO_NCELL_CONFIG        		0x5004
#define OAM_HO_NCELL_CONFIG_ACK    		0x5005
#define OAM_BTS_PACKAGE		  		0x5008
#define OAM_BTS_PACKAGE_MSB	  		0x50
#define OAM_BTS_PACKAGE_LSB	  		0x08

// SMS-CB table size
#define	  SmsCbc_TableRow_MAX			100

#endif // __RM_CONST_H__
