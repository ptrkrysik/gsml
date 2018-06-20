/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*******************************************************************
*/

#ifndef __GRR_CONST_H__
#define __GRR_CONST_H__

#define GRR_OFF							0
#define GRR_ON							1

#define GRR_WDOG						1
#define GRR_DSP_WDOG_EXPIRED			2
#define GRR_INIT_WATCH_TIME				36000


//GRR Task Options
//
#define GRR_TASK_PRIORITY					100
#define GRR_TASK_STACK_SIZE					10000
#define GRR_TASK_OPTION						0
#define GRR_MAX_RXMSGS						100	// Max # of messages
#define GRR_MAX_RXQMSG_LENGTH				sizeof(T_CNI_RIL3MD_RRM_MSG)
#define GRR_MAX_TXQMSG_LENGTH				sizeof(T_CNI_RIL3MD_RRM_MSG)
#define GRR_MAX_L1MSG_LEN					100
#define GRR_MAX_GRRMSG_LEN					100

//DSP initialization states	
#define GRR_TRXMGMT_S0_BOOTSTRAP_IND			1	
#define GRR_TRXMGMT_S1_TRXCONFIG_ACK  			2
#define GRR_TRXMGMT_S2_SLOTACTIV_ACK			3
#define GRR_TRXMGMT_S3_TRXCONFIG_END			4

//L1 message group defs
#define GRR_L1MG_TRXMGMT  					0x10
#define GRR_L1MG_COMCHAN					0x0C
#define GRR_L1MG_DEDMGMT					0x08
#define GRR_L1MG_LNKMGMT					0x02
#define GRR_L1MG_OAMMGMT		 			0x18

//L1 message types 

#define GRR_L1MT_CBCHRTS					0x0300	
#define GRR_L1MT_CBCHRTS_MSB					0x03
#define GRR_L1MT_CBCHRTS_LSB					0x00

#define GRR_L1MT_PING						0x1300
#define GRR_L1MT_PING_MSB					0x13
#define GRR_L1MT_PING_LSB					0x00

#define GRR_L1MT_PINGACK					0x1301
#define GRR_L1MT_PINGACK_MSB					0x13
#define GRR_L1MT_PINGACK_LSB					0x01

#define GRR_L1MT_ARFCN						0x2900
#define GRR_L1MT_ARFCN_MSB					0x29
#define GRR_L1MT_ARFCN_LSB					0x00

#define GRR_L1MT_BCCHINFO					0x0011
#define GRR_L1MT_BCCHINFO_MSB					0x00
#define GRR_L1MT_BCCHINFO_LSB					0x11

#define GRR_L1MT_SYNCH						0x001D
#define GRR_L1MT_SYNCH_MSB					0x00
#define GRR_L1MT_SYNCH_LSB					0x1D

#define GRR_L1MT_SYNCHACK					0x001E
#define GRR_L1MT_SYNCHACK_MSB					0x00
#define GRR_L1MT_SYNCHACK_LSB					0x1E

#define GRR_L1MT_TRXCONFIG					0x1500
#define GRR_L1MT_TRXCONFIG_MSB				0x15
#define GRR_L1MT_TRXCONFIG_LSB				0x00

#define GRR_L1MT_TRXCONFIGACK					0x1600
#define GRR_L1MT_TRXCONFIGACK_MSB				0x16
#define GRR_L1MT_TRXCONFIGACK_LSB				0x00

#define GRR_L1MT_SACCHFILL					0x001A
#define GRR_L1MT_SACCHFILL_MSB				0x00
#define GRR_L1MT_SACCHFILL_LSB				0x1A

#define GRR_L1MT_RFRESIND					0x0019
#define GRR_L1MT_RFRESIND_MSB					0x00
#define GRR_L1MT_RFRESIND_LSB					0x19

#define GRR_L1MT_OVERLOAD					0x001B
#define GRR_L1MT_OVERLOAD_MSB					0x00
#define GRR_L1MT_OVERLOAD_LSB					0x1B

#define GRR_L1MT_SLOTACTIV					0x1700
#define GRR_L1MT_SLOTACTIV_MSB				0x17
#define GRR_L1MT_SLOTACTIV_LSB				0x00

#define GRR_L1MT_SLOTACTIVACK					0x1800
#define GRR_L1MT_SLOTACTIVACK_MSB				0x18
#define GRR_L1MT_SLOTACTIVACK_LSB				0x00

#define GRR_L1MT_SLOTACTIVNACK				0x1900
#define GRR_L1MT_SLOTACTIVNACK_MSB				0x19
#define GRR_L1MT_SLOTACTIVNACK_LSB				0x00

#define GRR_L1MT_SETUGAIN					0x1000
#define GRR_L1MT_SETUGAIN_MSB					0x10
#define GRR_L1MT_SETUGAIN_LSB					0x00

#define GRR_L1MT_SETDGAIN					0x1100
#define GRR_L1MT_SETDGAIN_MSB					0x11
#define GRR_L1MT_SETDGAIN_LSB					0x00

#define GRR_L1MT_TUNESYNTH					0x1200
#define GRR_L1MT_TUNESYNTH_MSB				0x12
#define GRR_L1MT_TUNESYNTH_LSB				0x00

#define GRR_L1MT_CONFIGFPGADELAY				0x14
#define GRR_L1MT_CONFIGFPGADELAY_MSB			0x14
#define GRR_L1MT_CONFIGFPGADELAY_LSB			0x00

#define GRR_L1MT_BOOTSTRAPCNF					0x2000
#define GRR_L1MT_BOOTSTRAPCNF_MSB  				0x20
#define GRR_L1MT_BOOTSTRAPCNF_LSB  				0x00

#define GRR_L1MT_CHANACTIV					0x0021
#define GRR_L1MT_CHANACTIV_MSB				0x00
#define GRR_L1MT_CHANACTIV_LSB				0x21

#define GRR_L1MT_CHANACTIVACK					0x0022
#define GRR_L1MT_CHANACTIVACK_MSB				0x00
#define GRR_L1MT_CHANACTIVACK_LSB				0x22

#define GRR_L1MT_CHANACTIVNACK				0x0023
#define GRR_L1MT_CHANACTIVNACK_MSB				0x00
#define GRR_L1MT_CHANACTIVNACK_LSB				0x23

#define GRR_L1MT_RFCHANREL					0x002E
#define GRR_L1MT_RFCHANREL_MSB				0x00
#define GRR_L1MT_RFCHANREL_LSB				0x2E

#define GRR_L1MT_RFCHANRELACK					0x0033
#define GRR_L1MT_RFCHANRELACK_MSB				0x00
#define GRR_L1MT_RFCHANRELACK_LSB				0x33

#define GRR_L1MT_RSL						0x0035
#define GRR_L1MT_RSL_MSB					0x00
#define GRR_L1MT_RSL_LSB					0x35

#define GRR_L1MT_MEASRES					0x0028
#define GRR_L1MT_MEASRES_MSB					0x00
#define GRR_L1MT_MEASRES_LSB					0x28

#define GRR_L1MT_MODEMODIFY					0x0029
#define GRR_L1MT_MODEMODIFY_MSB				0x00
#define GRR_L1MT_MODEMODIFY_LSB				0x29

#define GRR_L1MT_ENCRYPTION					0x0026
#define GRR_L1MT_ENCRYPTION_MSB				0x00
#define GRR_L1MT_ENCRYPTION_LSB				0x26

#define GRR_L1MT_MODEMODIFYACK				0x002A
#define GRR_L1MT_MODEMODIFYACK_MSB				0x00
#define GRR_L1MT_MODEMODIFYACK_LSB				0x2A

#define GRR_L1MT_MODEMODIFYNACK				0x002B
#define GRR_L1MT_MODEMODIFYNACK_MSB				0x00
#define GRR_L1MT_MODEMODIFYNACK_LSB				0x2B

#define GRR_L1MT_DEACTIVSACCH					0x0025
#define GRR_L1MT_DEACTIVSACCH_MSB				0x00
#define GRR_L1MT_DEACTIVSACCH_LSB				0x25

#define GRR_L1MT_PHDATAREQ					0x0001
#define GRR_L1MT_PHDATAREQ_MSB				0x00
#define GRR_L1MT_PHDATAREQ_LSB				0x01

#define GRR_L1MT_ASYNCHO_REQUIRED				0x0036
#define GRR_L1MT_ASYNCHO_REQUIRED_MSB			0x00
#define GRR_L1MT_ASYNCHO_REQUIRED_LSB			0x36

#define GRR_L1MT_AYSNCHO_REQ					0x0037
#define GRR_L1MT_AYSNCHO_REQ_MSB				0x00
#define GRR_L1MT_AYSNCHO_REQ_LSB				0x37

#define GRR_L1MT_DPCCFG						0x5000
#define GRR_L1MT_DPCCFG_MSB					0x50
#define GRR_L1MT_DPCCFG_LSB					0x00

#define GRR_L1MT_DPCCFGACK					0x5001
#define GRR_L1MT_DPCCFGACK_MSB				0x50
#define GRR_L1MT_DPCCFGACK_LSB				0x01

#define GRR_L1MT_HOSCELLCFG					0x5002
#define GRR_L1MT_HOSCELLCFG_MSB				0x50
#define GRR_L1MT_HOSCELLCFG_LSB				0x02

#define GRR_L1MT_HOSCELLCFGACK				0x5003
#define GRR_L1MT_HOSCELLCFGACK_MSB				0x50
#define GRR_L1MT_HOSCELLCFGACK_LSB				0x03

#define GRR_L1MT_HONCELLCFG					0x5004
#define GRR_L1MT_HONCELLCFG_MSB				0x50
#define GRR_L1MT_HONCELLCFG_LSB				0x04

#define GRR_L1MT_HONCELLCFGACK				0x5005
#define GRR_L1MT_HONCELLCFGACK_MSB				0x50
#define GRR_L1MT_HONCELLCFGACK_LSB				0x05

#define GRR_L1MT_BTSPACKAGE					0x5008
#define GRR_L1MT_BTSPACKAGE_MSB				0x50
#define GRR_L1MT_BTSPACKAGE_LSB				0x08

#define GRR_L1MT_TXPWMAXRDCTCFG  				0x5006
#define GRR_L1MT_TXPWMAXRDCTCFG_MSB  				0x50
#define GRR_L1MT_TXPWMAXRDCTCFG_LSB  				0x06

#define OAM_L1MT_TXPWRMAXRDCTCFGACK 			0x5007
#define OAM_L1MT_TXPWRMAXRDCTCFGACK_MSB			0x50
#define OAM_L1MT_TXPWRMAXRDCTCFGACK_LSB			0x07

//Channel Number
#define GRR_L1CH_TCHF						0x0800
#define GRR_L1CH_TCHF_MSB					0x08
#define GRR_L1CH_TCHF_LSB					0x00

#define GRR_L1CH_SDCCH4						0x2000
#define GRR_L1CH_SDCCH4_MSB					0x20
#define GRR_L1CH_SDCCH4_LSB					0x00

#define GRR_L1CH_RACH						0x8800
#define GRR_L1CH_RACH_MSB					0x88
#define GRR_L1CH_RACH_LSB					0x00

#define GRR_L1CH_PCH			 			0x9000 
#define GRR_L1CH_PCH_MSB					0x90 
#define GRR_L1CH_PCH_LSB			 		0x00 

#define GRR_L1CH_AGCH						0x9000
#define GRR_L1CH_AGCH_MSB					0x90
#define GRR_L1CH_AGCH_LSB					0x00

#define GRR_L1CH_BCCH						0x8000
#define GRR_L1CH_BCCH_MSB					0x80
#define GRR_L1CH_BCCH_LSB					0x00

#define GRR_L1CH_CBCH						0xa000
#define GRR_L1CH_CBCH_MSB					0xa0
#define GRR_L1CH_CBCH_LSB					0x00

//Link Identifer
#define GRR_L1CH_MAIN						0x00				
#define GRR_L1CH_ACCH						0x40
#define GRR_L1CH_VOID						0x20

//Channel Mode
#define GRR_L1CH_ACTTYPE_IMMASS				0x00
#define GRR_L1CH_ACTTYPE_ASYNC_HO			      0x02
#define GRR_L1CH_ACTTYPE_CBCH					0x06
#define GRR_L1CH_DTXCTRL_D0U0					0x00
#define GRR_L1CH_DATATTR_SPEECH				0x01
#define GRR_L1CH_DATATTR_DATA					0x02
#define GRR_L1CH_DATATTR_SIGNALING				0x03
#define GRR_L1CH_RATTYPE_SDCCH				0x01
#define GRR_L1CH_RATTYPE_TCHF					0x08
#define GRR_L1CH_CODALGO_NORES				0x00
#define GRR_L1CH_CODALGO_GSMV1				0x01
#define GRR_L1CH_CODALGO_GSMV2				0x21
#define GRR_L1CH_CODALGO_GSMV3				0x41

//Encription
#define GRR_L1CH_CIPHNUL					0x01
#define GRR_L1CH_CIPHKEY_LEN					9
	
//System information
#define GRR_L1SI_TYPE_8						0x00
#define GRR_L1SI_TYPE_1						0x01
#define GRR_L1SI_TYPE_2						0x02
#define GRR_L1SI_TYPE_3						0x03
#define GRR_L1SI_TYPE_4						0x04
#define GRR_L1SI_TYPE_5						0x05
#define GRR_L1SI_TYPE_6						0x06
#define GRR_L1SI_TYPE_7						0x07
#define GRR_L1SI_TYPE_13					0x0D

// Air Interface standard
#define GRR_L1ST_PCS1900	  			      0x02


#endif //__GRR_CONST_H__


