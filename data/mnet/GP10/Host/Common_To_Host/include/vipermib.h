/*-------------------------------------------------------------------------
 *
 * ViperMib.h  ViperCell MIB support header file
 *
 ***************************************************************************
 * Modification history
 * ----------------------------
 * 03/16/00  Bhawani   removeed build id realted unused constants
 * 02/08/01  Bhawani   Fi 
 *--------------------------------------------------------------------------
 */

#ifndef	CISCO_GP10_MIB
#define CISCO_GP10_MIB

#include <stdarg.h>
#include "table.h"
#include "mibtags.h"

extern void  snmp_mibInitialize();		/* Function prototypes */
extern int snmp_getValue( MibTag, ... );
extern Status snmp_getEntry( MibTag, ... );
extern Status snmp_setValue( MibTag, ... );

extern Status snmp_setEntry(MibTag, int, void*, int);
extern void *snmp_getAddress( MibTag, ... );
extern Table *snmp_getTableAddress( MibTag );
extern int snmp_setItem ( char*, ...);
extern int snmp_getItem ( char *, ...);

typedef	char	*TBCD_STRING;
typedef char	*OCTET;
typedef int	INTEGER;
typedef INTEGER	AbsoluteRFChannelNo;
typedef INTEGER	AveragingNumber;
typedef INTEGER	CauseCode;
typedef INTEGER	CellIdentity;
typedef INTEGER	CellReselectHysteresis;
typedef INTEGER	ChannelID;
typedef INTEGER	ClassNumber;
typedef INTEGER	FrequencyUsage;
typedef INTEGER	GSMGeneralObjectID;
typedef INTEGER	HoMargin;
typedef INTEGER	HoPriorityLevel ;
typedef INTEGER	HoppingSequenceNumber;
typedef INTEGER	IpAddress;
typedef INTEGER	L2Timer;
typedef INTEGER	L3Timer;
typedef	INTEGER	HandoverTimer;
typedef INTEGER	LocationAreaCode;
typedef INTEGER	MaxQueueLength;
typedef INTEGER	MobileCountryCode;
typedef INTEGER	MobileNetworkCode;
typedef INTEGER	MSRangeMax;
typedef INTEGER	NetworkColourCode;
typedef INTEGER	NoOfBlocksForAccessGrant;
typedef INTEGER	NoOfMultiframesBetweenPaging;
typedef INTEGER	Ny1;
typedef INTEGER	PlmnPermitted;
typedef INTEGER	PowerControlInterval;
typedef INTEGER	PowerIncrStepSize;
typedef INTEGER	PowerRedStepSize;
typedef INTEGER	QueueTimeLimit;
typedef INTEGER	RadioLinkTimeout;
typedef INTEGER	RxLev;
typedef INTEGER	RxQual;
typedef INTEGER	TimerPeriodicUpdateMS;
typedef INTEGER	Tsc;
typedef INTEGER	TxInteger;
typedef INTEGER	TxPower;
typedef INTEGER	TxPwrMaxReduction;
typedef INTEGER	Weighting;
typedef INTEGER	Si13Position;
typedef INTEGER	CbchTrxSlot;	
typedef INTEGER	CbchTrx;			
typedef INTEGER	BcchChangeMark;
typedef INTEGER	SiChangeField;	

typedef union MibTableInfo_t {
	Int32	value;
	struct {
		unsigned	unused	:28;
		unsigned	lock	:1;
		unsigned	getTrap	:1;
		unsigned	setTrap :1;
		unsigned	dirty	:1;
	} info;
} MibTableInfo;

typedef enum AdministrativeState_t	{ locked=0,
					  unlocked=1,
					  shuttingDown=2
} AdministrativeState;

typedef enum AlarmStatus_t		{ underRepair=0,
					  critical=1,
					  major=2,
					  minor=3,
					  alarmOutstanding=4
} AlarmStatus;


typedef enum Boolean_t			{ bfalse=0,
					  btrue=1
} Boolean;

typedef enum ChannelCombination_t	{ tCHFull=1,	  /* TCH/F + FACCH/F + SACCH/F */
					  tCHHalf=2,	  /* TCH/H(0,1) + FACCH/H(0,1) + SACCH/H(0,1) */
					  tCHHalf2=3,	  /* TCH/H(0,0) + FACCH/H(0,1) + SACCH/H(0,1) + TCH/H(1,1) */
					  /* sDCCH=3,	   SDCCH/8(0..7) + SACCH/C8(0..7) */
					  mainBCCH=4,	  /* FCCH + SCH + BCCH + CCCH */
					  bCCHCombined=5, /*FCCH + SCH + BCCH + CCCH + SDCCH/4(0..3) + SACCH/C4(0..3) */
					  cCH=6,	  /* BCCH + CCCH */
					  bCCHwithCBCH=7, /* combination 5 with CBCH as 05.02 Note 1 */
					  sDCCHwithCBCH=8, /* combination 3 with CBCH as 05.02 Note 1 */
					  pBCCH=11,		   /* combination XI for GPRS */
					  pCCCH=12,		   /* combination XII for GPRS */
					  pDCHData=13      /* combination XIII for GPRS */
} ChannelCombination;

typedef enum DtxUplink_t		{ msMayUseDTx=0,
					  msShallUseDTx=1,
					  msShallNotUseDTx=2
} DtxUplink;

typedef enum EnableoType_t		{ enablePwrBudget=0,
					  enableMSDistanceProcess=1
} EnableHoType;

typedef enum EnableDisable_t		{ opStateDisabled=0,
					  opStateEnabled=1
} EnableDisable;

typedef enum OperationalState_t		{ disabled=0,
					  enabled=1
} OperationalState;

typedef enum GsmdcsIndicator_t 		{ gsm=0,
					  extendedgsm=1,
					  dcs=2,
					  pcs1900=3
} GsmdcsIndicator;

typedef enum MaxRetrans_t		{ one=1,
					  two=2,
					  four=4,
					  seven=7
} MaxRetrans;

typedef enum PowerClass_t 		{
					  m1 = 1,  /* M1 (0.2W - GSM 900) */ 
					  m3 = 2,  /* M3 (0.1W - DCS 1800) */
					  m2 = 3,  /* M2 (0.2W - PCS 1900) */
} PowerClass;

typedef enum PowerStatus_t		{
					  power_off=0,
					  power_on=1
} PowerStatus;

typedef enum PowerSupplyType_t		{
					  basic=0,
					  ups=1
} PowerSupplyType;

typedef enum RecoveryMode_t		{
					  disable=0,
					  enable=1,
					  now=2
} RecoveryMode;

typedef enum OamOperation_t { 
                 clearStatus=0,
					  reloadMib=1,	  /* Load in the default MIB file. */
					  flushMib=2,	  /* Save the active MIB variables. */
					  loadMibFile=3,  /* Load in an Alternate MIB file. */
					  saveMibFile=4,	   /* Save to an Alternate file name. */
					  loadAsciiMib=5,
					  saveAsciiMib=6
} OamOperation;

typedef enum OamOperationStatus_t	{ clear=0,
					  inProgress=1,	 /* Load in the default MIB file. */
					  sucessful=2,	 /* The last opertion was sucessful. */
					  failed=3	 /* The last operation failed. */
} OamOperationStatus;

typedef enum SnmpTrapType_t		{ none=0,
					  snmpv1Trap=1,	 /* Send a Version 1 style Trap. */
					  snmpv2Trap=2,	 /* Send a Version 2 style Trap. */
} SnmpTrapType;

/* First structures that are used within the base structures @ vipertbl.h */


typedef struct CellAllocationEntry_t {
	INTEGER			cellAllocationIndex;	/* {cellAllocationEntry 1} */
	AbsoluteRFChannelNo	cellAllocation;		/* {cellAllocationEntry 2} */
} CellAllocationEntry;

typedef struct	PlmnPermittedEntry_t {
	INTEGER			plmnPermittedIndex;	/* {plmnPermittedEntry 1} */
	PlmnPermitted		plmnPermitted;		/* {plmnPermittedEntry 2} */
} PlmnPermittedEntry;

typedef struct	CarrierFrequencyEntry_t {
	INTEGER			carrierFrequencyIndex;	/* {carrierFrequencyEntry-x 1} */
	AbsoluteRFChannelNo	carrierFrequency;	/* {carrierFrequencyEntry-x 2} */
} CarrierFrequencyEntry;

typedef CarrierFrequencyEntry	CarrierFrequencyEntry_0;	/* Different names for this table */
typedef CarrierFrequencyEntry	CarrierFrequencyEntry_1;	/* One for each transmitter.      */

typedef struct	ChannelEntry_t {
	INTEGER		 	channelIndex;		/* {channelEntry-x 1} */
	ChannelID		channelID;		/* {channelEntry-x 2} */
	ChannelCombination	channelCombination;	/* {channelEntry-x 3} */
	FrequencyUsage		channelFrequencyUsage;	/* {channelEntry-x 4} */
	Tsc			channelTsc;		/* {channelEntry-x 5} */
	AdministrativeState	channelAdministrativeState;	/* {channelEntry-x 6} */
	EnableDisable		channelOperationalState;	/* {channelEntry-x 7} */
} ChannelEntry;

typedef ChannelEntry ChannelEntry_0;
typedef ChannelEntry ChannelEntry_1;

typedef struct	T200_t {
	MibTableInfo		info;			/* Current Status of table. */
	L2Timer			sdcchSAPI0;		/* {t200 1} */
	L2Timer			facchTCHF;		/* {t200 2} */
	L2Timer			facchTCHH;		/* {t200 3} */
	L2Timer			sacchTCHSAPI0;		/* {t200 4} */
	L2Timer			sacchSDCCH;		/* {t200 5} */
	L2Timer			sdcchSAPI3;		/* {t200 6} */
	L2Timer			sdcchTCHSAPI3;		/* {t200 7} */
} T200;
typedef struct T31xx_t {
	MibTableInfo		info;			/* Current Status of table. */
	L3Timer			t3101;			/* {t31xx 1} */
	L3Timer			t3103;			/* {t31xx 2} */
	L3Timer			t3105;			/* {t31xx 3} */
	L3Timer			t3107;			/* {t31xx 4} */
	L3Timer			t3109;			/* {t31xx 5} */
	L3Timer			t3111;			/* {t31xx 6} */
	L3Timer			t3113;			/* {t31xx 7} */
} T31xx;

typedef struct AdjCell_HandoverEntry_t {
	INTEGER			adjCell_handoverIndex;		/* {adjCell-handoverEntry 1} */
	GSMGeneralObjectID	adjCell_handoverCellID;		/* {adjCell-handoverEntry 2} */
	MobileCountryCode	adjCell_mcc;			/* {adjCell-handoverEntry 3} */
	MobileNetworkCode	adjCell_mnc;			/* {adjCell-handoverEntry 4} */
	LocationAreaCode	adjCell_lac;			/* {adjCell-handoverEntry 5} */
	CellIdentity		adjCell_ci;			/* {adjCell-handoverEntry 6} */
	AbsoluteRFChannelNo	adjCell_bCCHFrequency;		/* {adjCell-handoverEntry 7} */
	NetworkColourCode	adjCell_ncc;			/* {adjCell-handoverEntry 8} */
	CellIdentity		adjCell_cid;			/* {adjCell-handoverEntry 9} */
	Boolean			adjCell_synchronized;		/* {adjCell-handoverEntry 10} */
	HoPriorityLevel		adjCell_hoPriorityLevel;	/* {adjCell-handoverEntry 11} */
	HoMargin		adjCell_hoMargin;		/* {adjCell-handoverEntry 12} */
	TxPower			adjCell_msTxPwrMaxCell;		/* {adjCell-handoverEntry 13} */
	RxLev			adjCell_rxLevMinCell;		/* {adjCell-handoverEntry 14} */
	Boolean			adjCell_isExternal;			/* {adjCell-handoverEntry 15} */
} AdjCell_HandoverEntry;

typedef struct	AdjCell_ReselectionEntry_t {
	INTEGER			adjCell_reselectionIndex;	/* {adjCell-reselectionEntry 1} */
	GSMGeneralObjectID	adjCell_reselectionCellID;	/* {adjCell-reselectionEntry 2} */
	AbsoluteRFChannelNo	adjCell_reselectionBCCHFrequency; /* {adjCell-reselectionEntry 3} */
} AdjCell_ReselectionEntry;

typedef struct	MobileAllocationEntry_t {
	INTEGER			mobileAllocationIndex;		/* {mobileAllocationEntry 1} */
	AbsoluteRFChannelNo	mobileAllocation;		/* {mobileAllocationEntry 2} */
} MobileAllocationEntry;

typedef struct BtsBasicPackage_t {
	MibTableInfo		info;				/* Current Status of table. */
	NetworkColourCode	bts_ncc;			/* {btsBasicPackage 1} */
	CellIdentity		bts_cid;			/* {btsBasicPackage 2} */
	GSMGeneralObjectID	btsID;				/* {btsBasicPackage 3} */
	Table			*cellAllocationTable;		/* {btsBasicPackage 4} */
	GsmdcsIndicator		gsmdcsIndicator	;		/* {btsBasicPackage 5} */
	MobileCountryCode	bts_mcc;			/* {btsBasicPackage 6} */
	MobileNetworkCode	bts_mnc;			/* {btsBasicPackage 7} */
	LocationAreaCode	bts_lac;			/* {btsBasicPackage 8} */
	CellIdentity		bts_ci;				/* {btsBasicPackage 9} */
	CellReselectHysteresis	cellReselectHysteresis;		/* {btsBasicPackage 10} */
	Ny1			ny1;				/* {btsBasicPackage 11} */
	Table			*plmnPermittedTable;		/* {btsBasicPackage 12} */
	RadioLinkTimeout	radioLinkTimeout;		/* {btsBasicPackage 13} */
	GSMGeneralObjectID	relatedTranscoder;		/* {btsBasicPackage 14} */
	RxLev			rxLevAccessMin;			/* {btsBasicPackage 15} */
	AdministrativeState	bts_administrativeState;	/* {btsBasicPackage 16} */
	AlarmStatus		bts_alarmStatus;		/* {btsBasicPackage 17} */
	EnableDisable		bts_operationalState;		/* {btsBasicPackage 18} */
#if 0	/* Make this an integer for the RM module. */
	MaxRetrans		maxNumberRetransmissions;	/* {btsBasicPackage 19} */
#else
	int			maxNumberRetransmissions;	/* {btsBasicPackage 19} */
#endif
	TxPower			mSTxPwrMaxCCH;			/* {btsBasicPackage 20} */
	TxInteger		numberOfSlotsSpreadTrans;	/* {btsBasicPackage 21} */
	NoOfBlocksForAccessGrant noOfBlocksForAccessGrant;	/* {btsBasicPackage 22} */
	NoOfMultiframesBetweenPaging noOfMultiframesBetweenPaging; /* {btsBasicPackage 23} */
} BtsBasicPackage;

typedef struct NotAllowedAccessClassEntry_t {
	INTEGER			notAllowedAccessClassIndex;	/* {notAllowedAccessClassEntry 1} */
	ClassNumber		notAllowedAccessClass;		/* {notAllowedAccessClassEntry 2} */
} NotAllowedAccessClassEntry;

typedef struct BtsOptionsPackage_t {				/* {viperCellGsmMib 2} */
	MibTableInfo		info;				/* Current Status of table. */
	Boolean			allowIMSIAttachDetach;		/* {btsOptionsPackage 1} */
	Boolean			callReestablishmentAllowed;	/* {btsOptionsPackage 2} */
	Boolean			cellBarred;			/* {btsOptionsPackage 3} */
	Boolean			dtxDownlink;			/* {btsOptionsPackage 4} */
	DtxUplink		dtxUplink;			/* {btsOptionsPackage 5} */
	Boolean			emergencyCallRestricted;	/* {btsOptionsPackage 6} */
	Table			*notAllowedAccessClassTable;	/* {btsOptionsPackage 7} */
	TimerPeriodicUpdateMS	timerPeriodicUpdateMS;		/* {btsOptionsPackage 8} */
	MaxQueueLength		maxQueueLength;			/* {btsOptionsPackage 9} */
	Boolean			msPriorityUsedInQueueing;	/* {btsOptionsPackage 10} */
	QueueTimeLimit		timeLimitCall;			/* {btsOptionsPackage 11} */
	QueueTimeLimit		timeLimitHandover;		/* {btsOptionsPackage 12} */
} BtsOptionsPackage;

typedef struct TransceiverPackage_t {				/* firstTra... {viperCellGsmMib 3} */
	MibTableInfo		info;				/* secondTr... {viperCellGsmMib 4} */
	GSMGeneralObjectID	basebandTransceiverID;		/* {....TransceiverPackage 1} */
	GSMGeneralObjectID	relatedRadioCarrier;		/* {....TransceiverPackage 2} */
	AdministrativeState	basebandAdministrativeState;	/* {....TransceiverPackage 3} */
	AlarmStatus		basebandAlarmStatus;		/* {....TransceiverPackage 4} */
	EnableDisable		basebandOperationalState;	/* {....TransceiverPackage 5} */
	Table			*channelTable;			/* {....TransceiverPackage 6} */
} TransceiverPackage;

typedef TransceiverPackage FirstTransceiverPackage;	/* The entire package is duplicated */
typedef TransceiverPackage SecondTransceiverPackage;	/* since a vipercell has 2 radios.  */

typedef struct RadioCarrierPackage_t {				/* firstRadio... {viperCellGsmMib 5} */
	MibTableInfo		info;				/* secondRadio.. {viperCellGsmMib 6} */
	Table			*carrierFrequencyList;		/* {....RadioCarrierPackage 1} */
	PowerClass		powerClass;			/* {....RadioCarrierPackage 2} */
	GSMGeneralObjectID	radioCarrierID;			/* {....RadioCarrierPackage 3} */
	TxPwrMaxReduction	txPwrMaxReduction;		/* {....RadioCarrierPackage 4} */
	AdministrativeState	carrier_administrativeState;	/* {....RadioCarrierPackage 5} */
	AlarmStatus		carrier_alarmStatus;		/* {....RadioCarrierPackage 6} */
	EnableDisable		carrier_operationalState;	/* {....RadioCarrierPackage 7} */
} RadioCarrierPackage;

typedef RadioCarrierPackage FirstRadioCarrierPackage;
typedef RadioCarrierPackage SecondRadioCarrierPackage;

typedef struct BtsTimerPackage_t {				/* {viperCellGsmMib 7} */
	MibTableInfo		info;				/* Current Status of table. */
	T200			*t200;				/* {btsTimerPackage 1} */
	T31xx			*t31xx;				/* {btsTimerPackage 2} */
} BtsTimerPackage;

typedef struct AdjacentCellPackage_t {				/* {viperCellGsmMib 8} */
	MibTableInfo		info;				/* Current Status of table. */
	Table			*adjCell_handoverTable;		/* {adjacentCellPackage 1} */
	Table			*adjCell_reselectionTable;	/* {adjacentCellPackage 2} */
} AdjacentCellPackage;

typedef struct HandoverTimerMib_t {
	MibTableInfo		info;				/* Current Status of table. */
	HandoverTimer		t101;
	HandoverTimer		t102;
	HandoverTimer		t103;
	HandoverTimer		t104;
	HandoverTimer		t201;
	HandoverTimer		t202;
	HandoverTimer		t204;
	HandoverTimer		t210;
	HandoverTimer		t211;
} HandoverTimerMib;

typedef struct Gp10ServiceStatusMib_t {
	MibTableInfo		info;
	INTEGER				snmpServiceEnabled;
	INTEGER				httpServiceEnabled;
	INTEGER				telnetServiceEnabled;
	INTEGER				sshServiceEnabled;
	INTEGER				ftpServiceEnabled;
} Gp10ServiceStatusMib;


typedef struct FrequencyHoppingPackage_t {			/* {viperCellGsmMib 9} */
	MibTableInfo		info;				/* Current Status of table. */
	GSMGeneralObjectID	frequencyHoppingSystemID;	/* {frequencyHoppingPackage 1} */
	HoppingSequenceNumber	hoppingSequenceNumber;		/* {frequencyHoppingPackage 2} */
	Table			*mobileAllocationTable;		/* {frequencyHoppingPackage 3} */
} FrequencyHoppingPackage;

typedef struct	HoAveragingAdjCellParam_t {	/* {handoverControlPackage 3}*/
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		hreqave;	/*{hoAveragingAdjCellParam 1}*/
	AveragingNumber		hreqt;		/*{hoAveragingAdjCellParam 2}*/
	Weighting		weighting;	/*{hoAveragingAdjCellParam 3}*/
} HoAveragingAdjCellParam;
typedef struct	HoAveragingDistParam_t {	/* {handoverControlPackage 4}*/
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		hreqave;	/* {hoAveragingDistParam 1} */
	AveragingNumber		hreqt;		/* {hoAveragingDistParam 2} */
} HoAveragingDistParam;
typedef struct	HoAveragingLevParam_t {		/* {handoverControlPackage 5}*/
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		hreqave;	/* {hoAveragingLevParam 1} */
	AveragingNumber		hreqt;		/* {hoAveragingLevParam 2} */
	Weighting		weighting;	/* {hoAveragingLevParam 3} */
} HoAveragingLevParam;
typedef struct	HoAveragingQualParam_t {	/* {handoverControlPackage 6}*/
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		hreqave;	/* {hoAveragingQualParam 1} */
	AveragingNumber		hreqt;		/* {hoAveragingQualParam 2} */
	Weighting		weighting;	/* {hoAveragingQualParam 3} */
} HoAveragingQualParam;
typedef struct	HoThresholdDistParam_t {	/* {handoverControlPackage 8}*/
	MibTableInfo		info;		/* Current Status of table. */
	MSRangeMax		timeadv;	/* {hoThresholdDistParam 1} */
	AveragingNumber		p8;		/* {hoThresholdDistParam 2} */
	AveragingNumber		n8;		/* {hoThresholdDistParam 3} */
} HoThresholdDistParam;
typedef struct	HoThresholdInterfaceParam_t {	/* {handoverControlPackage 9}*/
	MibTableInfo		info;		/* Current Status of table. */
	RxLev			rxLevelUL;	/* {hoThresholdInterfaceParam 1} */
	RxLev			rxLevelDL;	/* {hoThresholdInterfaceParam 2} */
	AveragingNumber		px;		/* {hoThresholdInterfaceParam 3} */
	AveragingNumber		nx;		/* {hoThresholdInterfaceParam 4} */
} HoThresholdInterfaceParam;
typedef struct	HoThresholdLevParam_t {		/* {handoverControlPackage 10} */
	MibTableInfo		info;		/* Current Status of table. */
	RxLev			rxLevelUL;	/* {hoThresholdLevParam 1} */
	RxLev			rxLevelDL;	/* {hoThresholdLevParam 2} */
	AveragingNumber		px;		/* {hoThresholdLevParam 3} */
	AveragingNumber		nx;		/* {hoThresholdLevParam 4} */
} HoThresholdLevParam;
typedef struct	HoThresholdQualParam_t {	/* {handoverControlPackage 11} */
	MibTableInfo		info;		/* Current Status of table. */
	RxQual			rxQualUL;	/* {hoThresholdQualParam 1} */
	RxQual			rxQualDL;	/* {hoThresholdQualParam 2} */
	AveragingNumber		px;		/* {hoThresholdQualParam 3} */
	AveragingNumber		nx;		/* {hoThresholdQualParam 4} */
} HoThresholdQualParam;
typedef struct	InterferenceAveragingParam_t {	/* {handoverControlPackage 12} */
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		interferenceAveragingParamAveragingPeriod;	/* {interferenceAveragingParam 1}*/
	RxLev			interferenceAveragingParamThresholdBoundary0;	/* {interferenceAveragingParam 2}*/
	RxLev			interferenceAveragingParamThresholdBoundary1;	/* {interferenceAveragingParam 3}*/
	RxLev			interferenceAveragingParamThresholdBoundary2;	/* {interferenceAveragingParam 4}*/
	RxLev			interferenceAveragingParamThresholdBoundary3;	/* {interferenceAveragingParam 5}*/
	RxLev			interferenceAveragingParamThresholdBoundary4;	/* {interferenceAveragingParam 6}*/
	RxLev			interferenceAveragingParamThresholdBoundary5;	/* {interferenceAveragingParam 7}*/
} InterferenceAveragingParam;

typedef struct HandoverControlPackage_t {	/* {viperCellGsmMib 10} */
	MibTableInfo		info;		/* Current Status of table. */
	GSMGeneralObjectID	handoverControlID;			/* {handoverControlPackage 1} */
	EnableHoType		enableOptHandoverProcessing;		/* {handoverControlPackage 2} */
	HoAveragingAdjCellParam	*hoAveragingAdjCellParam;		/* {handoverControlPackage 3} */
	HoAveragingDistParam	*hoAveragingDistParam;			/* {handoverControlPackage 4} */
	HoAveragingLevParam	*hoAveragingLevParam;			/* {handoverControlPackage 5} */
	HoAveragingQualParam	*hoAveragingQualParam;			/* {handoverControlPackage 6} */
	HoMargin		hoMarginDef;				/* {handoverControlPackage 7} */
	HoThresholdDistParam	*hoThresholdDistParam;			/* {handoverControlPackage 8} */
	HoThresholdInterfaceParam	*hoThresholdInterfaceParam;		/* {handoverControlPackage 9} */
	HoThresholdLevParam	*hoThresholdLevParam;			/* {handoverControlPackage 10} */
	HoThresholdQualParam	*hoThresholdQualParam;			/* {handoverControlPackage 11} */
	InterferenceAveragingParam	*interferenceAveragingParam;		/* {handoverControlPackage 12} */
	TxPower			mxTxPwrMaxCellDef;			/* {handoverControlPackage 13} */
	RxLev			rxLevMinCellDef;			/* {handoverControlPackage 14} */
} HandoverControlPackage;

typedef struct PcAveragingLev_t {		/* {powerControlPackage 1} */
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		hreqave;	/* {pcAveragingLev 1} */
	AveragingNumber		hreqt;		/* {pcAveragingLev 2} */
	Weighting		weighting;	/* {pcAveragingLev 3} */
} PcAveragingLev;

typedef struct PcAveragingQual_t {		/* {powerControlPackage 2} */
	MibTableInfo		info;		/* Current Status of table. */
	AveragingNumber		hreqave;	/* {pcAveragingQual 1} */
	AveragingNumber		hreqt;		/* {pcAveragingQual 2} */
	Weighting		weighting;	/* {pcAveragingQual 3} */
} PcAveragingQual;

typedef struct	PcLowerThresholdLevParam_t {	/* {powerControlPackage 3} */
	MibTableInfo		info;		/* Current Status of table. */
	RxLev			rxLevelUL;	/* {pcLowerThresholdLevParam 1} */
	RxLev			rxLevelDL;	/* {pcLowerThresholdLevParam 2} */
	AveragingNumber		px;		/* {pcLowerThresholdLevParam 3} */
	AveragingNumber		nx;		/* {pcLowerThresholdLevParam 4} */
} PcLowerThresholdLevParam;

typedef struct	PcLowerThresholdQualParam_t {	/* {powerControlPackage 4} */
	MibTableInfo		info;		/* Current Status of table. */
	RxQual			rxQualUL;	/* {pcLowerThresholdQualParam 1} */
	RxQual			rxQualDL;	/* {pcLowerThresholdQualParam 2} */
	AveragingNumber		px;		/* {pcLowerThresholdQualParam 3} */
	AveragingNumber		nx;		/* {pcLowerThresholdQualParam 4} */
} PcLowerThresholdQualParam;

typedef struct	PcUpperThresholdLevParam_t {	/* {powerControlPackage 5} */
	MibTableInfo		info;		/* Current Status of table. */
	RxLev			rxLevelUL;	/* {pcUpperThresholdLevParam 1} */
	RxLev			rxLevelDL;	/* {pcUpperThresholdLevParam 2} */
	AveragingNumber		px;		/* {pcUpperThresholdLevParam 3} */
	AveragingNumber		nx;		/* {pcUpperThresholdLevParam 4} */
} PcUpperThresholdLevParam;

typedef struct	PcUpperThresholdQualParam_t {	/* {powerControlPackage 6} */
	MibTableInfo		info;		/* Current Status of table. */
	RxQual			rxQualUL;	/* {pcUpperThresholdQualParam 1} */
	RxQual			rxQualDL;	/* {pcUpperThresholdQualParam 2} */
	AveragingNumber		px;		/* {pcUpperThresholdQualParam 3} */
	AveragingNumber		nx;		/* {pcUpperThresholdQualParam 4} */
} PcUpperThresholdQualParam;

typedef struct PowerControlPackage_t {		/* {viperCellGsmMib 11} */
	MibTableInfo		info;		/* Current Status of table. */
	PcAveragingLev			*pcAveragingLev;			/* {powerControlPackage 1} */
	PcAveragingQual			*pcAveragingQual;			/* {powerControlPackage 2} */
	PcLowerThresholdLevParam	*pcLowerThresholdLevParam;		/* {powerControlPackage 3} */
	PcLowerThresholdQualParam	*pcLowerThresholdQualParam;		/* {powerControlPackage 4} */
	PcUpperThresholdLevParam	*pcUpperThresholdLevParam;		/* {powerControlPackage 5} */
	PcUpperThresholdQualParam	*pcUpperThresholdQualParam;		/* {powerControlPackage 6} */
	PowerControlInterval		powerControlInterval;			/* {powerControlPackage 7} */
	PowerIncrStepSize		powerIncrStepSize;			/* {powerControlPackage 8} */
	PowerRedStepSize		powerRedStepSize;			/* {powerControlPackage 9} */
} PowerControlPackage;

typedef struct	AttImmediateAssingProcsPerCauseEntry_t {
	CauseCode		attImmediateAssingProcsCause;	/* {attImmediateAssingProcsPerCauseEntry 1} */
	INTEGER			attImmediateAssingProcsValue;	/* {attImmediateAssingProcsPerCauseEntry 2} */
} AttImmediateAssingProcsPerCauseEntry;

typedef struct	SuccImmediateAssingProcsPerCauseEntry_t {
	CauseCode		succImmediateAssingProcsCause;	/* {succImmediateAssingProcsPerCauseEntry 1} */
	INTEGER			succImmediateAssingProcsValue;	/* {succImmediateAssingProcsPerCauseEntry 2} */
} SuccImmediateAssingProcsPerCauseEntry;

typedef struct CellMeasurmentPackage_t {				/* {viperCellGsmMib 12} */
	MibTableInfo		info;		/* Current Status of table. */
	INTEGER			meanPCHAGCHQueueLength;			/* {cellMeasurmentPackage 1} */
	INTEGER			attTransOfPagingMessagesThePCH;		/* {cellMeasurmentPackage 2} */
	INTEGER			unsuccTransOfPagingMessagesThePCH;	/* {cellMeasurmentPackage 3} */
	INTEGER			attImmediateAssingProcs;		/* {cellMeasurmentPackage 4} */
	INTEGER			succImmediateAssingProcs;		/* {cellMeasurmentPackage 5} */
	Table			*attImmediateAssingProcsPerCauseTable;	/* {cellMeasurmentPackage 6} */
	Table			*succImmediateAssingProcsPerCauseTable;	/* {cellMeasurmentPackage 7} */
	INTEGER			nbrOfPagesDiscardedFromPCHQueue;	/* {cellMeasurmentPackage 8} */
	INTEGER			meanDurationOfSuccPagingProcs;		/* {cellMeasurmentPackage 9} */
	INTEGER			nbrOfAvailableTCHs;			/* {cellMeasurmentPackage 10} */
	INTEGER			meanNbrOfBusyTCHs;			/* {cellMeasurmentPackage 11} */
	INTEGER			maxNbrOfBusyTCHs;			/* {cellMeasurmentPackage 12} */
	INTEGER			meanNbrOfIdleTCHsPerInterferenceBand;	/* {cellMeasurmentPackage 13} */
	INTEGER			attTCHSeizures;				/* {cellMeasurmentPackage 14} */
	INTEGER			succTCHSeizures;			/* {cellMeasurmentPackage 15} */
	INTEGER			attTCHSeizuresMeetingTCHBlockedState;	/* {cellMeasurmentPackage 16} */
	INTEGER			allAvailableTCHAllocatedTime;		/* {cellMeasurmentPackage 17} */
	INTEGER			meanTCHBusyTime;			/* {cellMeasurmentPackage 18} */
	INTEGER			meanTCHQueueLength;			/* {cellMeasurmentPackage 19} */
	INTEGER			nbrOfLostRadioLinksTCH;			/* {cellMeasurmentPackage 20} */
	INTEGER			nbrOfAvailableSDCCHs;			/* {cellMeasurmentPackage 21} */
	INTEGER			meanNbrOfBusySDCCHs;			/* {cellMeasurmentPackage 22} */
	INTEGER			maxNbrOfBusySDCCHs;			/* {cellMeasurmentPackage 23} */
	INTEGER			attSDCCHSeizuresMeetingSDCCHBlockedState; /* {cellMeasurmentPackage 24} */
	INTEGER			allAvailableSDCCHAllocatedTime;		/* {cellMeasurmentPackage 25} */
	INTEGER			meanSDCCHQueueLength;			/* {cellMeasurmentPackage 26} */
	INTEGER			nbrOfLostRadioLinksSDCCH;		/* {cellMeasurmentPackage 27} */
	INTEGER			relativeTimeDLPowerControlAtMax;	/* {cellMeasurmentPackage 28} */
	INTEGER			relativeTimeULPowerControlAtMax;	/* {cellMeasurmentPackage 29} */
	INTEGER			succInternalHDOsIntraCell;		/* {cellMeasurmentPackage 30} */
	INTEGER			unsuccInternalHDOsIntraCell;		/* {cellMeasurmentPackage 31} */
#if THESE_ARE_NOT_USED_IN_VIPERCELL
	INTEGER			attIncomingInternalInterCellHDOs;	/* {cellMeasurmentPackage 32} */
	INTEGER			succIncomingInternalInterCellHDOs;	/* {cellMeasurmentPackage 33} */
	INTEGER			attOutgoingInternalInterCellHDOs;	/* {cellMeasurmentPackage 34} */
	INTEGER			succOutgoingInternalInterCellHDOs;	/* {cellMeasurmentPackage 35} */
	INTEGER			unsuccHDOsWithReconnection;		/* {cellMeasurmentPackage 36} */
	INTEGER			unsuccHDOsWithLossOfConnection;		/* {cellMeasurmentPackage 37} */
#endif
} CellMeasurmentPackage;

typedef struct	ExternalHDOsPerCauseEntry_t {
	CauseCode		externalHDOsPerCauseCode;		/* {externalHDOsPerCauseEntry 1} */
	INTEGER			externalHDOsPerCauseValue;		/* {externalHDOsPerCauseEntry 2} */
} ExternalHDOsPerCauseEntry;
typedef struct CallControlMeasurmentPackage_t {				/* {viperCellGsmMib 13} */
	MibTableInfo		info;		/* Current Status of table. */
	INTEGER			nbrOfClassMarkUpdates;			/* {callControlMeasurmentPackage 1} */
	INTEGER			attMobileOriginatingCalls;		/* {callControlMeasurmentPackage 2} */
	INTEGER			succMobileOriginatingCalls;		/* {callControlMeasurmentPackage 3} */
	INTEGER			ansMobileOriginatingCalls;		/* {callControlMeasurmentPackage 4} */
	INTEGER			attMobileTerminatingCalls;		/* {callControlMeasurmentPackage 5} */
	INTEGER			succMobileTerminatingCalls;		/* {callControlMeasurmentPackage 6} */
	INTEGER			ansMobileTerminatingCalls;		/* {callControlMeasurmentPackage 7} */
	INTEGER			attMobileEmergencyCalls;		/* {callControlMeasurmentPackage 8} */
	INTEGER			succMobileEmergencyCalls;		/* {callControlMeasurmentPackage 9} */
	INTEGER			ansMobileEmergencyCalls;		/* {callControlMeasurmentPackage 10} */
	INTEGER			attCipheringModeControlProcs;		/* {callControlMeasurmentPackage 11} */
	INTEGER			succCipheringModeControlProcs;		/* {callControlMeasurmentPackage 12} */
	INTEGER			attInterrogationOfHLRsForRouting;	/* {callControlMeasurmentPackage 13} */
	INTEGER			succInterrogationOfHLRsMSRNObtained;	/* {callControlMeasurmentPackage 14} */
	INTEGER			succInterrogationOfHLRsCallForwarding;	/* {callControlMeasurmentPackage 15} */
	INTEGER			attOpForMobileOriginatingPointToPointSMs;  /* {callControlMeasurmentPackage 16} */
	INTEGER			succOpForMobileOriginatingPointToPointSMs; /* {callControlMeasurmentPackage 17} */
	INTEGER			attOpForMobileTerminatingPointToPointSMs;  /* {callControlMeasurmentPackage 18} */
	INTEGER			succOpForMobileTerminatingPointToPointSMs; /* {callControlMeasurmentPackage 19} */
	INTEGER			meanTimeToCallSetupService;		/* {callControlMeasurmentPackage 20} */
	INTEGER			meanTimeToLocationUpdateService;	/* {callControlMeasurmentPackage 21} */
	INTEGER			transSubIdentifiedWithIMSI;		/* {callControlMeasurmentPackage 22} */
	INTEGER			imsiDetachProcs;			/* {callControlMeasurmentPackage 23} */
	INTEGER			imsiAttachProcs;			/* {callControlMeasurmentPackage 24} */
	INTEGER			attIncomingInterMSCHDOs;		/* {callControlMeasurmentPackage 25} */
	INTEGER			succIncomingInterMSCHDOs;		/* {callControlMeasurmentPackage 26} */
	INTEGER			attOutgoingInterMSCHDOs;		/* {callControlMeasurmentPackage 27} */
	INTEGER			succOutgoingInterMSCHDOs;		/* {callControlMeasurmentPackage 28} */
	Table			*externalHDOsPerCauseTable;		/* {callControlMeasurmentPackage 29} */
	INTEGER			unsuccExternHDOsWithReconnectionPerMSC;	/* {callControlMeasurmentPackage 30} */
	INTEGER			unsuccExternHDOsWithLossOfConnectionPerMSC; /* {callControlMeasurmentPackage 31} */
} CallControlMeasurmentPackage;

typedef struct CallConfigData {					/* {viperCellConfigMib 1} */
	MibTableInfo		info;		/* Current Status of table. */
	INTEGER			cc_max_L3_msgs;			/* {callConfigData 1} */
	INTEGER			cc_l3_msg_q_priority;		/* {callConfigData 2} */
	INTEGER			cc_task_priority;		/* {callConfigData 3} */
	INTEGER			cc_task_stack_size;		/* {callConfigData 4} */
	INTEGER			cc_mm_conn_est_T999;		/* {callConfigData 5} */
	INTEGER			cc_alerting_T301;		/* {callConfigData 6} */
	INTEGER			cc_setup_T303;			/* {callConfigData 7} */
	INTEGER			cc_call_confirmed_T310;		/* {callConfigData 8} */
	INTEGER			cc_connect_T313;		/* {callConfigData 9} */
	INTEGER			cc_disconnect_T305;		/* {callConfigData 10} */
	INTEGER			cc_release_T308;		/* {callConfigData 11} */
	INTEGER			cc_safety_timer;		/* {callConfigData 12} */
	INTEGER			cc_max_calls;			/* {callConfigData 13} */
} CallConfigData;
typedef struct mobilityConfigData_t {				/* {viperCellConfigMib 2} */
	MibTableInfo		info;		/* Current Status of table. */
	INTEGER			mm_max_mm_connections;		/* {mobilityConfigData 1} */
	INTEGER			mm_max_L3_msgs;			/* {mobilityConfigData 2} */
	INTEGER			task_priority;			/* {mobilityConfigData 3} */
	INTEGER			task_stack_size;		/* {mobilityConfigData 4} */
	INTEGER			mm_paging_t3113;		/* {mobilityConfigData 5} */
	Boolean			mm_authenticationRequired;	/* {mobilityConfigData 6} */
	Boolean			mm_cipheringRequired;		/* {mobilityConfigData 7} */
	Boolean			mm_IMEICheckRequired;		/* {mobilityConfigData 8} */
} MobilityConfigData;
typedef struct ResourceConfigData_t {				/* {viperCellConfigMib 3} */
	MibTableInfo		info;		/* Current Status of table. */
	INTEGER			rm_t3L01;			/* {resourceConfigData 1} */
	INTEGER			rm_bsCcChans;			/* {resourceConfigData 2} */
	INTEGER			rm_bsCcchSdcchComb;		/* {resourceConfigData 3} */
	INTEGER			rm_sysInfoOnOffMap;		/* {resourceConfigData 4} */
	INTEGER			rm_airInterface;		/* {resourceConfigData 5} */
	INTEGER			rm_viperCellTsc;		/* {resourceConfigData 6} */
	INTEGER			rm_maxSlotPerTrx;		/* {resourceConfigData 7} */
	INTEGER			rm_maxTrxs;			/* {resourceConfigData 8} */
	INTEGER			rm_maxTchfs;			/* {resourceConfigData 9} */
	INTEGER			rm_maxSdcch4s;			/* {resourceConfigData 10} */
	INTEGER			rm_trxOnOffMap;			/* {resourceConfigData 11} */
	INTEGER			rm_networkIfConfig;		/* {resourceConfigData 12} */
	INTEGER			rm_bcchTrx;			/* {resourceConfigData 13} */
	INTEGER			rm_preAlphaTrx;			/* {resourceConfigData 14} */
	INTEGER			rm_ccchBcchComb;		/* {resourceConfigData 15} */
	INTEGER			rm_ccchConf;			/* {resourceConfigData 16} */
	INTEGER			rm_nim[2][8];			/* {resourceConfigData 17 - 32} */
	INTEGER			rm_trxSynthSetting[2][4];	/* {resourceConfigData 33 - 40} */
	INTEGER			rm_trxFpgaDelay[2][2];		/* {resourceConfigData 41 - 44} */
	INTEGER			rm_SGainCtrlSetting[2][2];	/* {resourceConfigData 45 - 48} */
	INTEGER			rm_synchChanInfo;		/* {resourceConfigData 49} */
} ResourceConfigData;

typedef struct OamConfigData_t {                /* {viperCellConfigMib 4}   */
   MibTableInfo   info;                         /* Current Status of table. */
   INTEGER        oamCommand;                   /* {oamConfigData 1} */
   INTEGER        oamCommandExecutionStatus;    /* {oamConfigData 2} */
   OCTET          *oamAlternateMibFileName;     /* {oamConfigData 3} */
   INTEGER        *oamCommandLastError;         /* {oamConfigData 4} */
   Boolean        mibAccessControlFlag;         /* {oamConfigData 5} */
} OamConfigData;

typedef struct MaintenanceConfigData_t {				/* {viperCellConfigMib 4} */
	MibTableInfo		info;				/* Current Status of table. */
	char			*maintenance_operation;		/* {maintenanceConfigData 1} */
	char			*maintenance_response;		/* {maintenanceConfigData 2} */
} MaintenanceConfigData;

typedef struct TrapFwdData_t {
	MibTableInfo		info;				/* {viperCellConfigMib 6} */
	Table			*trapFwdTable;		/* {trapFwdData 1} */
} TrapFwdData;


typedef struct TrapFwdEntry_t
{
   INTEGER			trapFwdIndex;  	/* Index into the Table. */
  	SnmpTrapType   trapType;			/* Type of trap SNMPv1 / SNMPv2 */
   IpAddress      trapIpAddress;    /* Trap Manager's IP Address */
   INTEGER        trapPort;         /* Trap Manager's Trap Port  */
   char*          trapCommunity;    /* Expected Community string */
}TrapFwdEntry;

/* Structure used to store H323 Management informaiton */
typedef struct H323Data_t {
	MibTableInfo		info;	/* Current Status of table. */
	INTEGER	GKPort;			/* Gate Keeper's listline Port */
	IpAddress   GKIPAddress;		/* Gate Keeper's IP address */
	INTEGER	Q931ResponseTimeOut;	/* Q931 Response Time Out */
	INTEGER	Q931ConnectTimeOut;	/* Q931 Connect time out */
	INTEGER	Q931CallSignalingPort;	/* Q931 Call Singaling Port */
	INTEGER	t35CountryCode;		/* t35 Country Code */
	INTEGER rasRequestTimeout;	/* Ras Request Timeout */
	INTEGER	viperbaseHeartbeatTimeout;	/*...*/
} H323Data;

/* Structures for the Viper Specific MIB */
typedef struct DualTRXSCard_t {
	MibTableInfo		info;				/* Current Status of table. */
	INTEGER		monitorReceiverPLL_LockDetectStatus;	/* Lock Detect Status of the Monitor Receiver's PLL */
	INTEGER		referenceFrequencyPLL_LockDetectStatus;	/* Lock Detect Status of the Reference Frequency's PLL */
	INTEGER		transceiver_1_PLL_LockDetectStatus;	/* Lock Detect Status of the Transceiver #1 PLL */
	INTEGER		transceiver_2A_PLL_LockDetectStatus;	/* Lock Detect Status of the Transceiver #2A PLL */
	INTEGER		transceiver_2B_PLL_LockDetectStatus;	/* Lock Detect Status of the Transceiver #2B PLL */
	INTEGER		intermediateFrequency_LockDetectStatus;	/* Lock Detect Status of the Intermediate Frequency's PLL */
	char		*trxSerialNumber;	/* Hardware Serial Number of the TRX Card */
	char		*trxSoftwareVersion;	/* Software Version Number of the TRX Card */
	EnableDisable	trxLoopBackMode;	/* Loop Back Mode */
} DualTRXSCard;

typedef struct ArfcnRssiFunction_t {
	MibTableInfo		info;				/* Current Status of table. */
	EnableDisable	arfcnRssiControlMode;		/* ARFCN/RSSI Function Control */
	RecoveryMode	clockRecoveryControlMode;	/* Clock Recovery Feature Control */
	Table		*arfcnRssiClockControlTable;	/* This entry is required to define a table of values. */
} ArfcnRssiFunction;

typedef struct	ArfcnRssiClockControlEntry_t {
	INTEGER			arfcnRssiClockControlIndex;	/* Index to the ARFCN/RSSI/ClockControl table. */
	AbsoluteRFChannelNo	arfcnRssiClockControlArfcn;	/* RF Channel No */
	EnableDisable		arfcnRssiClockRecoveryMode;	/* Clock Syncronization */
} ArfcnRssiClockControlEntry;

typedef struct ClockCard_t {
	MibTableInfo		info;				/* Current Status of table. */
	INTEGER	clockCardType;			/* Clock Card types */
	char	*clockCardSoftwareVersion;	/* Software Version of the Clock Card */
	INTEGER	clockCardStatus;		/* Clock Card Status - validity of clock output */
	char	*clockCardCrystalUpTime;	/* Crystal Up-time of the Clock Card */
	INTEGER	clockCardDAC;			/* Clock Card DAC value */
} ClockCard;

typedef struct GpsCard_t {
	MibTableInfo		info;				/* Current Status of table. */
	INTEGER	gpsCardType;			/* GPS Card types */
	char	*gpsCardSoftwareVersion;	/* Software Version of the GPS Card */
	INTEGER	gpsCardStatus;			/* GPS Card General Status */
	char	*gpsPosition;			/* Longitude and Latitude */
	char	*gpsTime;			/* GPS Time */
	char	*utcTime;			/* UTC Time */
	INTEGER	clockStatusFrequencyError;	/* Clock Status - Frequency Error */
	INTEGER	clockStatusDACVoltage;		/* Clock Status - DAC Voltage */
	INTEGER	gpsAntennaStatus;		/* Antenna Status */
	INTEGER	gpsSatelliteStatus;		/* Satellite Status */
} GpsCard;

typedef struct PowerCard_t {
	MibTableInfo		info;				/* Current Status of table. */
	INTEGER		powerCardType;		/* powerCard  types */
	INTEGER		powerCardStatus;	/* Power Card General Status */
	PowerStatus	powerOn;		/* Switch of the power Card (none-reversible action when turn-off) */
} PowerCard;

typedef struct ExternalPowerSupply_t {
	MibTableInfo		info;				/* Current Status of table. */
	INTEGER	powerSupplyType;		/* External Power Supply type */
	INTEGER	powerSupplyStatus;		/* Power Supply General Status */
} ExternalPowerSupply;

typedef struct CdcBoard_t {
	MibTableInfo		info;				/* Current Status of table. */
	char	*cdcBoardSerialNumber;		/* cdcBoard Serial Number */
	char	*cdcBoardMACAddress;		/* CDC board MAC Address */
} CdcBoard;

typedef struct AutomaticParameterAllocation_t {
	MibTableInfo		info;				/* Current Status of table. */
	INTEGER	apaEnable ;			/* Feature Enable */
	INTEGER	apaScanFrequencyLowBound ;	/* Low bound of the Feaquencies to be scaned */
	INTEGER	apaScanFrequencyHighBound ;	/* High bound of the Feaquencies to be scaned */
	INTEGER	apaMaxPowerSetting;		/* Max power setting for ViperCell */
	INTEGER	apaMinRSSIThresholdForNeighborId;	/* Minimum RSSI value below which the unit is not a neighbor */
	INTEGER	apaScanRate;			/* Scan Rate for checking RSSI value (in unit of TBD) */
	INTEGER	apaHysteresisForBCCHPower;	/* Hysteresis for change the power on the BCCH */
	Table	*apaInvalidFrequencyTable;	/* This attribute defines the set of invalid frequencies within the scan frequency range. */
	Table	*apaClockSourceFrequencyTable;	/* This attribute defines the set of clock source frequencies. */
} AutomaticParameterAllocation;

typedef struct	ApaInvalidFrequencyEntry_t {
	INTEGER			apaInvalidFrequencyIndex;	/* Index to the apa Invalid Frequency table. */   
	AbsoluteRFChannelNo	apaInvalidFrequencyArfcn;	/* Index to the apa Invalid Frequency table. */   
} ApaInvalidFrequencyEntry;

typedef struct 	ApaClockSourceFrequencyEntry_t {
	INTEGER			apaClockSourceFrequencyIndex;	      /* Index to the apa Invalid Frequency table. */
	AbsoluteRFChannelNo	apaClockSourceFrequencyArfcn;	/* Apa Invalid Frequency. */
} ApaClockSourceFrequencyEntry;

typedef struct Gp10MgmtInfoMib_t {
	MibTableInfo		info;				   /* Current Status of table. */
	char	*viperCellName;		         /* The name of the ViperCell */
	IpAddress   viperCellIPAddress;	   /* IP Address of the ViperCell*/
	IpAddress	viperCellIDefGateway;	/* Default gateway*/
   char	*viperCellLocation;	         /* Location of the ViperCell */
	char	*viperCellSerialNumber;	      /* Serial Number of the ViperCell */
	char	*viperCellAssetNumber;  	   /* Asset Number or other Customer tracking information */
	char	*viperCellSoftwareBuild;	   /* Date upon which the ViperCell software was created */
	char	*viperCellCustomerName;	      /* Name of the Customer who owns the ViperCell */
	char	*viperCellCustomerAddress;	   /* Address of who Customer who owns the ViperCell */
	char	*viperCellCustomerPhone;	   /* Phone Number of who Customer who owns the ViperCell */
	char	*viperCellCustomerEmail;	   /* Email of who to Customer who owns the ViperCell */
	char	*viperCellCustomerInfo;	      /* For any general information the customer wishes to store */
} Gp10MgmtInfoMib;

typedef struct 	ViperAccessMib_t
{
	MibTableInfo		info;				            /* Current Status of table. */
	char	            *readCommunity;		      /* Read Community */
   char              *writeCommunity;           /* write Community */
} ViperAccessMib;


typedef struct Gp10ErrorInfoMib_t {
	MibTableInfo		info;				/* Current Status of table. */
	char	*errorCode_L1;
	char	*errorCode_MD;
	char	*errorCode_RM;
	char	*errorCode_MM;
	char	*errorCode_CC;
	char	*errorCode_L2;
	char	*errorCode_SMS;
	char	*errorCode_OAM;
	char	*errorCode_H323;
	char	*errorCode_LUDB;
	char	*errorCode_EXT;
	char	*errorCode_LOG;
	char	*errorCode_MCH;
	char	*errorCode_ALARM;
	char	*errorCode_SYSINIT;
	char	*errorCode_GRR;
	char	*errorCode_CDR;
} Gp10ErrorInfoMib;

typedef struct Gp10CdrMib_t {
	MibTableInfo		info;				/* Current Status of table. */
	Boolean		cdrClientSSLEnable;
	INTEGER		cdrLongCallTimer;
} Gp10CdrMib;

/* New Structures to support GPRS */

typedef struct BtsGsBasicPackage_t {
	IpAddress	gprsServerIpAddress ;			/* Ip Address of GPRS Server */
	INTEGER	gprsRac ;				/* Routing Area Code Number */
	EnableDisable	enableGprs ;	/* Enable / Disable GPRS */
	OperationalState	gprsOperationalStatus;		/* Operational Status of GPRS Server */
} BtsGsBasicPackage;


typedef struct BtsGsOptionPackage_t {
	INTEGER	nmo;
	INTEGER	t3168;
	INTEGER	t3192;
	INTEGER	drxTimerMax;
	INTEGER	accessBurstType;
	INTEGER	controlAckType;
	INTEGER	nmbsCvMax;
	INTEGER	panDec;
	INTEGER	panInc;
	INTEGER	panMax;
	NetworkColourCode	raColour;
	Si13Position		si13Position;
	CbchTrxSlot			cbchTrxSlot;
	CbchTrx				cbchTrx;	
	BcchChangeMark		bcchChangeMark;
	SiChangeField		siChangeField;
}BtsGsOptionPackage;


typedef struct NcFrequencyListEntry_t {
	INTEGER	ncFrequencyListIndex;
	AbsoluteRFChannelNo	ncFrequency;
} NcFrequencyListEntry;

typedef struct Rss_t {
	INTEGER	gprsMsTxpwrMaxCch;
	INTEGER	gprsRexLevAccessMin;
	INTEGER	gprsReselectOffset;
	INTEGER	priorityClass;
	INTEGER	lsaId;
	INTEGER	hcsThreshold;
	INTEGER	gprsTemporaryOffset;
	INTEGER	gprsPenaltyTime;
	INTEGER	gprsCellReselectHysteresis;
	INTEGER	gprsRaReselectHysteresis;
	INTEGER	c32Qual;
	INTEGER	c31Hysteresis;
	INTEGER	alpha;
	INTEGER	pB;
	INTEGER	networkControlOrder;
	Table*	ncFrequencyListTable;
	INTEGER	ncReportingPeriodI;
	INTEGER	ncReportingPeriodT;
	INTEGER	extMeasurementOrder;
	INTEGER	extReportingPeriod;
	INTEGER	extReportingType;
	INTEGER	intFrequency;
	INTEGER	nccPermitted;
	INTEGER	pcMeasChan;
	INTEGER	tavgW;
	INTEGER	tavgT;
	INTEGER	navgI;
	RxLev		gprsMsTargetPower;
	Boolean	spgcCcchSup;
}Rss;

typedef struct RlcMac_t {
	INTEGER	t3169;
	INTEGER	t3191;
	INTEGER	t3193;
	INTEGER	t3195;
} RlcMac;

typedef struct Bssgp_t {
	INTEGER	blockTimer;
	INTEGER	resetTimer;
	INTEGER	suspendTimer;
	INTEGER	resumeTimer;
	INTEGER	capUpdateTimer;
	INTEGER	blockRetries;
	INTEGER	unBlockRetries;
	INTEGER	resetRetries;
	INTEGER	suspendRetries;
	INTEGER	resumeRetries;
	INTEGER	raCapUpdateRetries;
	INTEGER	timeInterval;
	INTEGER	networkPort;
} Bssgp;

typedef struct BtsGsMeasPackage_t {
	INTEGER	flushReqRecvd;
	INTEGER	pagingReqRecvd;
	INTEGER	meanPsInterArrivalTime;
	INTEGER	availablePdch;
	INTEGER	meanAvailablePdch;
	INTEGER	maxAvailablePdch;
	INTEGER	minAvailablePdch;
	INTEGER	meanOccupiedPdch;
	INTEGER	maxOccupiedPdch;
	INTEGER	minOccupiedPdch;
	INTEGER	availablePdchAllocTime;
	INTEGER	packetPagingMessages;
	INTEGER	pagchQueueLength;
	INTEGER	pagesDiscardedFromPpch;
	INTEGER	assignmentRequest;
	INTEGER	succAssignmentProc;
	INTEGER	succPdtchSeizures;
	INTEGER	meanPacketQueueLength;
	INTEGER	serviceChanges;
} BtsGsMeasPackage;


#endif	/* defined CISCO_GP10_MIB */
