#ifndef	JETCELL_VIPERCELL_MIB_CONTROL_TABLES
#define JETCELL_VIPERCELL_MIB_CONTROL_TABLES

#include <sys/times.h>	/* For the timeval structure */

#define	USECS_PER_SEC	1000000			/* Timers for delayed callbacks */
#define	NSECS_PER_SEC	1000000000		/* Timers for delayed callbacks */
#define NSECS_TO_USECS	(NSECS_PER_SEC/USECS_PER_SEC)
#define	TICKS_PER_SEC	64

#define	SECONDS		USECS_PER_SEC		/* Use microsecond based timers */
#define	IN_HALF_SECOND	(SECONDS/2)
#define	IN_1_SECOND	(SECONDS)
#define	IN_2_SECONDS	(2*SECONDS)
#define	IN_3_SECONDS	(3*SECONDS)
#define	IN_5_SECONDS	(5*SECONDS)
#define IN_10_SECONDS	(10*SECONDS)
#define IN_15_SECONDS	(15*SECONDS)
#define IN_30_SECONDS	(30*SECONDS)
Status snmp_loadMib( char * );
Status snmp_saveMib( char * );
STATUS snmp_initDefaultMib() ;
int snmpMibWriteDelayTimeoutHandler(int arg);
void snmp_initIoTask();
int snmp_reboot(int);


/* defined in vipermib.c */
extern char*    int2ip(UINT32 nIp);
Status snmp_lockMib( int );
void snmp_fixupTable( Table * );
int strncasecmp(char *, char *, int);
int snmp_retryLoad(int, int, int, int);
void snmp_log(int flag, char *name, char *, ...);
char * snmp_tagName(MibTag tag);
void snmp_mibTimerInit();
void snmp_mibTimerDelete();
int snmpMibWriteDelayTimeoutHandler(int arg);
void snmp_mibTimerSet(char* arg);
void snmp_trapBroadcast(MibTag tag);
int snmp_findTag(const void *, const void *);
Status snmp_validTag( MibTag );

extern Table		*mnetMibDataTypeTable;
extern int		snmpRemoteIP;

typedef struct MnetMib_t {					  /* Mib Table ID*/
	BtsBasicPackage			btsBasicPackage;		/* 0 */
	BtsOptionsPackage			btsOptionsPackage;		/* 1 */
	FirstTransceiverPackage		firstTransceiverPackage;	/* 2 */
	SecondTransceiverPackage	secondTransceiverPackage;	/* 3 */
	FirstRadioCarrierPackage	firstRadioCarrierPackage;	/* 4 */
	SecondRadioCarrierPackage	secondRadioCarrierPackage;	/* 5 */
	BtsTimerPackage			btsTimerPackage;		/* 6 */
	T200				t200;				/* 7 */
	T31xx				t31xx;				/* 8 */
	AdjacentCellPackage		adjacentCellPackage;		/* 9 */
	FrequencyHoppingPackage		frequencyHoppingPackage;	/* 10 */
	HandoverControlPackage		handoverControlPackage;		/* 11 */
	HoAveragingAdjCellParam		hoAveragingAdjCellParam;	/* 12 */
	HoAveragingDistParam		hoAveragingDistParam;		/* 13 */
	HoAveragingLevParam		hoAveragingLevParam;		/* 14 */
	HoAveragingQualParam		hoAveragingQualParam;		/* 15 */
	HoThresholdDistParam		hoThresholdDistParam;		/* 16 */
	HoThresholdInterfaceParam	hoThresholdInterfaceParam;	/* 17 */
	HoThresholdLevParam		hoThresholdLevParam;		/* 18 */
	HoThresholdQualParam		hoThresholdQualParam;		/* 19 */
	InterferenceAveragingParam	interferenceAveragingParam;	/* 20 */
	PowerControlPackage		powerControlPackage;		/* 21 */
	PcAveragingLev			pcAveragingLev;			/* 22 */
	PcAveragingQual			pcAveragingQual;		/* 23 */
	PcLowerThresholdLevParam	pcLowerThresholdLevParam;	/* 24 */
	PcLowerThresholdQualParam	pcLowerThresholdQualParam;	/* 25 */
	PcUpperThresholdLevParam	pcUpperThresholdLevParam;	/* 26 */
	PcUpperThresholdQualParam	pcUpperThresholdQualParam;	/* 27 */
	CellMeasurmentPackage		cellMeasurmentPackage;		/* 28 */
	CallControlMeasurmentPackage	callControlMeasurmentPackage;	/* 29 */
	CallConfigData			callConfigData;			/* 30 */
	MobilityConfigData		mobilityConfigData;		/* 31 */
	ResourceConfigData		resourceConfigData;		/* 32 */
	OamConfigData			oamConfigData;			/* 33 */
	H323Data			h323Data;			/* 34 */
	DualTRXSCard			dualTRXSCard;			/* 35 */
	ArfcnRssiFunction		arfcnRssiFunction;		/* 36 */
	ClockCard			clockCard;			/* 37 */
	GpsCard				gpsCard;			/* 38 */
	PowerCard			powerCard;			/* 39 */
	ExternalPowerSupply		externalPowerSupply;		/* 40 */
	CdcBoard			cdcBoard;			/* 41 */
	AutomaticParameterAllocation	automaticParameterAllocation;	/* 42 */
	Gp10MgmtInfoMib		gp10MgmtInfoMib;		/* 43 */
	MaintenanceConfigData		maintenanceConfigData;		/* 44 */
	Gp10ErrorInfoMib		gp10ErrorInfoMib;		/* 45 */
	ViperAccessMib			viperAccessMib;			/* 46 */
	TrapFwdData			trapFwdData;			/* 47 */

	/* New Structures for supporting GS */
	BtsGsBasicPackage			btsGsBasicPackage;
	BtsGsOptionPackage		btsGsOptionPackage;
	Rss					rss;
	RlcMac				rlcMac;
	Bssgp					bssgp;
	BtsGsMeasPackage			btsGsMeasPackage;
	Gp10CdrMib			 gp10CdrMib;	/* 54 */
	HandoverTimerMib	handoverTimerMib;	/* 55 */
	Gp10ServiceStatusMib	gp10ServiceStatusMib;	/* 56 */
} MnetMib;

extern MnetMib	mnetMib;

/*	The following are used to process the varbinds for the SNMP PDU's
 */

#define VB_TAG (int)VB_TO_COOKIE
#define VB_TYPE(vb) ((((int)VB_TAG(vb))>>28)&0xF)
#define VB_SIZE(vb) ((((int)VB_TAG(vb))>>16)&0xFFF)
#define VB_TBL(vb)  ((((int)VB_TAG(vb))>>8)&0xFF)
#define VB_ITM(vb)  (((int)VB_TAG(vb))&0xFF)

#define  VB_STATIC   MTYPE_STATIC 
#define  VB_NUMBER   MTYPE_NUMBER 
#define  VB_STRING   MTYPE_STRING 
#define  VB_TABLE    MTYPE_TABLE  
#define  VB_TENTRY   MTYPE_TENTRY 
#define  VB_TSTRING  MTYPE_TSTRING



/*	In addition to the root structure, there are several tables that are
 *	used inthe MIB. These tables are allocated at runtime in the initialization
 *	routine. The following variables define the limits of these tables.
 */

#define	CellAllocationTableLimit			            32
#define	PlmnPermittedTableLimit				            16
#define	NotAllowedAccessClassTableLimit	            16
#define	CarrierFrequencyTableLimit			            16
#define	ChannelTableLimit				                  8
#define	HandoverTableLimit				               16
#define	ReselectionTableLimit				            16
#define	MobileAllocationTableLimit			            16
#define	AttImmediateAssingProcsPerCauseTableLimit	   16
#define	SuccImmediateAssingProcsPerCauseTableLimit	16
#define	ExternalHDOsPerCauseTableLimit			      16
#define	Oam_TestTableLimit				               4
#define	ArfcnRssiClockControlTableLimit			      64
#define	ApaInvalidFrequencyTableLimit			         64
#define	ApaClockSourceFrequencyTableLimit		      64
#define  TrapFwdTableLimit				                  8

/* New defines to support Gs */
#define 	NcFrequencyListTableLimit			16


#define	LIMIT_oam_testValue	MIN_oam_testValue

/* Note: This table is written out to the MIB data file as an index
	 to the contents. Because of this, it is difficult to change
	 the contents of this table. Once can use the SNMP MIB version
	 number to indicate a different format of the MIB data and do
	 a conversion upon loading the data I suppose. */

typedef struct MnetMibIndex_t {
	MibTag		tag;		   /* Tag for this table address.   */
	void		   *addr;		/* Address of the struct         */
	int		   size;		   /* Size of the structure         */
	int		   limit;		/* Table limit.	               */
	int		   offset;		/* Seek offset.                  */
} MnetMibIndex;

extern MnetMibIndex	mnetMibIndex[];

typedef	struct	OidPrefix_t {
	MibTag		tag;
	char		*prefix;
} OidPrefix;

typedef struct Mib2Access_t {
	STATUS		(*get)(char *);
	STATUS		(*set)(unsigned int, char *);
} Mib2Access;

extern	MnetMibIndex	*mibFileIndex;

typedef enum Access_t {
	no_access=0,
	read_only=1,
	write_only=2,
	read_write=3,
} Access;

typedef struct ViewAccess_t {
	struct ViewAccess_t *next;	/* Null = Last item in list. */
	int		     view;	/* The view this applies to. */
	Access		     allowed;	/* the access for this view. */
} ViewAccess;

/*	The following table is used to supply the data types for each of the
 *	items in the MIB. It would be nice if this information were stored
 *	in the SNMP control information, but that would require changes to
 *	mibcomp. We can extract most of the TYPE_... flags with the -array
 *	switch to mibcomp. Unfortunately, it does not include any structural
 *	elements. Since we need an entry for each MIB_TAG, we cannot use
 *	mibcomp to directly generate this table.
 */

typedef struct MnetMibDataType_T
 {
	int	    tag;
	int	    type;
	int	    value;
	int	    flag;
   ViewAccess *access;
	char	   *name;
} MnetMibDataType;

#define DTYPE(tag, typ, val, flg) { tag, typ, val, flg, 0, #tag }

extern	MnetMibDataType mnetMibDataType[];

#define SAVE_ASCII_OMIT	1

#define TYPE_INTEGER 0
#define TYPE_OBJECT_IDENTIFIER 1
#define TYPE_BITS 2
#define TYPE_OCTET_STRING 3
#define TYPE_ObjectName 4
#define TYPE_NetworkAddress 5
#define TYPE_IpAddress 6
#define TYPE_Counter 7
#define TYPE_Gauge 8
#define TYPE_TimeTicks 9
#define TYPE_Opaque 10
/* #define TYPE_ObjectName 11 */
#define TYPE_NotificationName 12
#define TYPE_Integer32 13
/* #define TYPE_IpAddress 14 */
#define TYPE_Counter32 15
#define TYPE_Gauge32 16
/* #define TYPE_TimeTicks 17 */
/* #define TYPE_Opaque 18 */
#define TYPE_Counter64 19
#define TYPE_Unsigned32 20
#define TYPE_DisplayString 21
#define TYPE_PhysAddress 22
#define TYPE_MacAddress 23
#define TYPE_TruthValue 24
#define TYPE_TestAndIncr 25
#define TYPE_AutonomousType 26
#define TYPE_InstancePointer 27
#define TYPE_VariablePointer 28
#define TYPE_RowPointer 29
#define TYPE_RowStatus 30
#define TYPE_TimeStamp 31
#define TYPE_TimeInterval 32
#define TYPE_DateAndTime 33
#define TYPE_StorageType 34
#define TYPE_TDomain 35
#define TYPE_TAddress 36
#define TYPE_SnmpUDPAddress 37
#define TYPE_SnmpOSIAddress 38
#define TYPE_SnmpNBPAddress 39
#define TYPE_SnmpIPXAddress 40
#define TYPE_TBCD_STRING 41
#define TYPE_AbsoluteRFChannelNo 42
#define TYPE_AveragingNumber 43
#define TYPE_CauseCode 44
#define TYPE_CellIdentity 45
#define TYPE_CellReselectHysteresis 46
#define TYPE_ChannelID 47
#define TYPE_ClassNumber 48
#define TYPE_FrequencyUsage 49
#define TYPE_GSMGeneralObjectID 50
#define TYPE_HoMargin 51
#define TYPE_HoPriorityLevel 52
#define TYPE_HoppingSequenceNumber 53
#define TYPE_L2Timer 54
#define TYPE_L3Timer 55
#define TYPE_LocationAreaCode 56
#define TYPE_MaxQueueLength 57
#define TYPE_MobileCountryCode 58
#define TYPE_MobileNetworkCode 59
#define TYPE_MSRangeMax 60
#define TYPE_NetworkColourCode 61
#define TYPE_NoOfBlocksForAccessGrant 62
#define TYPE_NoOfMultiframesBetweenPaging 63
#define TYPE_Ny1 64
#define TYPE_PlmnPermitted 65
#define TYPE_PowerControlInterval 66
#define TYPE_PowerIncrStepSize 67
#define TYPE_PowerRedStepSize 68
#define TYPE_QueueTimeLimit 69
#define TYPE_RadioLinkTimeout 70
#define TYPE_RxLev 71
#define TYPE_RxQual 72
#define TYPE_TimerPeriodicUpdateMS 73
#define TYPE_Tsc 74
#define TYPE_TxInteger 75
#define TYPE_TxPower 76
#define TYPE_TxPwrMaxReduction 77
#define TYPE_Weighting 78
#define TYPE_AdministrativeState 79
#define TYPE_AlarmStatus 80
#define TYPE_Boolean 81
#define TYPE_ChannelCombination 82
#define TYPE_DtxUplink 83
#define TYPE_EnableHoType 84
#define TYPE_GsmdcsIndicator 85
#define TYPE_MaxRetrans 86
#define TYPE_OperationalState 87
#define TYPE_PowerClass 88
#define	TYPE_SnmpTrapType 89
#define TYPE_Si13Position	90
#define TYPE_CbchTrxSlot	91	
#define TYPE_CbchTrx		92			
#define TYPE_BcchChangeMark	93
#define TYPE_SiChangeField	94	


#define	TYPE_StructurePointer	1000
#define	TYPE_TablePointer	1001
#define	TYPE_TableEntryPointer	1002
#define	TYPE_String		1003
#define	TYPE_Complex		1004
#define	TYPE_Average		1005
#define	TYPE_Deviation		1006
#define	TYPE_Maximum		1007
#define	TYPE_Minimum		1008
#define	TYPE_ResetValue		1009
#define	TYPE_MIB2_Value		1010

#endif	/* defined JETCELL_VIPERCELL_MIB_CONTROL_TABLES */


