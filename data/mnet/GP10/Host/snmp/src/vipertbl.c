
#include <mib.h>
#include <snmpdefs.h>
#include <snmpdLib.h>
#include <ioLib.h>
#include "mibhand.h"
#include "vipermib.h"		/* Get the MIB definitions. */
#include "vipertbl.h"		/* Control Table definitions.	*/

Table			*mnetMibDataTypeTable;
int			snmpRemoteIP;
int			snmpRemotePort;
int			snmpProxyIP = 0;

MnetMib		mnetMib;
MnetMibIndex	*mibFileIndex;
MnetMibIndex	mnetMibIndex[] = {
	/* Basic package addresses. */
	{MIB_btsBasicPackage,		(void *)&mnetMib.btsBasicPackage,
	 sizeof(BtsBasicPackage),	 0, 0 },	/*  0 */
	{MIB_btsOptionsPackage, 	(void *)&mnetMib.btsOptionsPackage,
	 sizeof(BtsOptionsPackage),	 0, 0 },	/*  1 */
	{MIB_firstTransceiverPackage, 	(void *)&mnetMib.firstTransceiverPackage,
	 sizeof(FirstTransceiverPackage), 0, 0 },	/*  2 */
	{MIB_secondTransceiverPackage, 	(void *)&mnetMib.secondTransceiverPackage,
	 sizeof(SecondTransceiverPackage), 0, 0 },	/*  3 */
	{MIB_firstRadioCarrierPackage, 	(void *)&mnetMib.firstRadioCarrierPackage,
	 sizeof(FirstRadioCarrierPackage), 0, 0 },	/*  4 */
	{MIB_secondRadioCarrierPackage, (void *)&mnetMib.secondRadioCarrierPackage,
	 sizeof(SecondRadioCarrierPackage), 0, 0 },	/*  5 */
	{MIB_btsTimerPackage,	 	(void *)&mnetMib.btsTimerPackage,
	 sizeof(BtsTimerPackage),	 0, 0 },	/*  6 */
	{MIB_t200,		 	(void *)&mnetMib.t200,
	 sizeof(T200),			 0, 0 },	/*  7 */
	{MIB_t31xx, 			(void *)&mnetMib.t31xx,
	 sizeof(T31xx),			 0, 0 },	/*  8 */
	{MIB_adjacentCellPackage, 	(void *)&mnetMib.adjacentCellPackage,
	 sizeof(AdjacentCellPackage),	 0, 0 },	/*  9 */
	{MIB_frequencyHoppingPackage, 	(void *)&mnetMib.frequencyHoppingPackage,
	 sizeof(FrequencyHoppingPackage), 0, 0 },	/* 10 */
	{MIB_handoverControlPackage, 	(void *)&mnetMib.handoverControlPackage,
	 sizeof(HandoverControlPackage), 0, 0 },	/* 11 */
	{MIB_hoAveragingAdjCellParam, 	(void *)&mnetMib.hoAveragingAdjCellParam,
	 sizeof(HoAveragingAdjCellParam), 0, 0 },	/* 12 */
	{MIB_hoAveragingDistParam, 	(void *)&mnetMib.hoAveragingDistParam,
	 sizeof(HoAveragingDistParam),	 0, 0 },	/* 13 */
	{MIB_hoAveragingLevParam, 	(void *)&mnetMib.hoAveragingLevParam,
	 sizeof(HoAveragingLevParam),	 0, 0 },	/* 14 */
	{MIB_hoAveragingQualParam, 	(void *)&mnetMib.hoAveragingQualParam,
	 sizeof(HoAveragingQualParam),	 0, 0 },	/* 15 */
	{MIB_hoThresholdDistParam, 	(void *)&mnetMib.hoThresholdDistParam,
	 sizeof(HoThresholdDistParam),	 0, 0 },	/* 16 */
	{MIB_hoThresholdInterfaceParam, (void *)&mnetMib.hoThresholdInterfaceParam,
	 sizeof(HoThresholdInterfaceParam), 0, 0 },	/* 17 */
	{MIB_hoThresholdLevParam, 	(void *)&mnetMib.hoThresholdLevParam,
	 sizeof(HoThresholdLevParam),	 0, 0 },	/* 18 */
	{MIB_hoThresholdQualParam, 	(void *)&mnetMib.hoThresholdQualParam,
	 sizeof(HoThresholdQualParam),	 0, 0 },	/* 19 */
	{MIB_interferenceAveragingParam,(void *)&mnetMib.interferenceAveragingParam,
	 sizeof(InterferenceAveragingParam), 0, 0 }, /* 20 */
	{MIB_powerControlPackage, 	(void *)&mnetMib.powerControlPackage,
	 sizeof(PowerControlPackage),	 0, 0  },	/* 21 */
	{MIB_pcAveragingLev, 		(void *)&mnetMib.pcAveragingLev,
	 sizeof(PcAveragingLev),	 0, 0 },	/* 22 */
	{MIB_pcAveragingQual,	 	(void *)&mnetMib.pcAveragingQual,
	 sizeof(PcAveragingQual),	 0, 0 },	/* 23 */
	{MIB_pcLowerThresholdLevParam, 	(void *)&mnetMib.pcLowerThresholdLevParam,
	 sizeof(PcLowerThresholdLevParam), 0, 0 },	/* 24 */
	{MIB_pcLowerThresholdQualParam, (void *)&mnetMib.pcLowerThresholdQualParam,
	 sizeof(PcLowerThresholdQualParam), 0, 0 },	/* 25 */
	{MIB_pcUpperThresholdLevParam, 	(void *)&mnetMib.pcUpperThresholdLevParam,
	 sizeof(PcUpperThresholdLevParam), 0, 0 },	/* 26 */
	{MIB_pcUpperThresholdQualParam, (void *)&mnetMib.pcUpperThresholdQualParam,
	 sizeof(PcUpperThresholdQualParam), 0, 0 },	/* 27 */
	{MIB_cellMeasurmentPackage, 	(void *)&mnetMib.cellMeasurmentPackage,
	 sizeof(CellMeasurmentPackage),	 0, 0 },	/* 28 */
	{MIB_callControlMeasurmentPackage, (void *)&mnetMib.callControlMeasurmentPackage,
	 sizeof(CallControlMeasurmentPackage), 0, 0 },/* 29 */
	{MIB_callConfigData, 		(void *)&mnetMib.callConfigData,
	 sizeof(CallConfigData),	 0, 0 },	/* 30 */
	{MIB_mobilityConfigData, 	(void *)&mnetMib.mobilityConfigData,
	 sizeof(MobilityConfigData),	 0, 0 },	/* 31 */
	{MIB_resourceConfigData, 	(void *)&mnetMib.resourceConfigData,
	 sizeof(ResourceConfigData),	 0, 0 },	/* 32 */
	{MIB_oamConfigData, 		(void *)&mnetMib.oamConfigData,
	 sizeof(OamConfigData),		 0, 0 },	/* 33 */
	{MIB_h323Data,	 		(void *)&mnetMib.h323Data,
	 sizeof(H323Data),		 0, 0 },		/* 34 */
	{MIB_dualTRXSCard,		(void *)&mnetMib.dualTRXSCard,
	 sizeof(DualTRXSCard),		 0, 0 },	/* 35 */
	{MIB_arfcnRssiFunction,		(void *)&mnetMib.arfcnRssiFunction,
	 sizeof(ArfcnRssiFunction),	 0, 0 },	/* 36 */
	{MIB_clockCard,			(void *)&mnetMib.clockCard,
	 sizeof(ClockCard),		 0, 0 },	/* 37 */
	{MIB_gpsCard,			(void *)&mnetMib.gpsCard,
	 sizeof(GpsCard),		 0, 0 },	/* 38 */
	{MIB_powerCard,			(void *)&mnetMib.powerCard,
	 sizeof(PowerCard),		 0, 0 },	/* 39 */
	{MIB_externalPowerSupply,	(void *)&mnetMib.externalPowerSupply,
	 sizeof(ExternalPowerSupply),	 0, 0 },	/* 40 */
	{MIB_cdcBoard,			(void *)&mnetMib.cdcBoard,
	 sizeof(CdcBoard),		 0, 0 },	/* 41 */
	{MIB_automaticParameterAllocation, (void *)&mnetMib.automaticParameterAllocation,
	 sizeof(AutomaticParameterAllocation), 0, 0 },	/* 42 */
	{MIB_gp10MgmtInfoMib,	(void *)&mnetMib.gp10MgmtInfoMib,
	 sizeof(Gp10MgmtInfoMib),	 0, 0 },		/* 43 */
	{MIB_maintenanceConfigData,	(void *)&mnetMib.maintenanceConfigData,
	 sizeof(MaintenanceConfigData),	 0, 0 },	/* 44 */
	{MIB_gp10ErrorInfoMib,	(void *)&mnetMib.gp10ErrorInfoMib,
	 sizeof(Gp10ErrorInfoMib),	 0, 0 },		/* 45 */
	{MIB_viperAccessMib,		(void *)&mnetMib.viperAccessMib,
	 sizeof(ViperAccessMib),	 0, 0 },		/* 46 */
	{MIB_trapFwdData,		(void *)&mnetMib.trapFwdData,
	 sizeof(TrapFwdData),	 0, 0 },		/* 47 */

	/* New Indexes to support Gs */
	{MIB_btsGsBasicPackage,		(void *)&mnetMib.btsGsBasicPackage,
	 sizeof(BtsGsBasicPackage),	 0, 0 },		/* 48 */
	{MIB_btsGsOptionPackage,		(void *)&mnetMib.btsGsOptionPackage,
	 sizeof(BtsGsOptionPackage),	 0, 0 },		/* 49 */
	{MIB_rss,		(void *)&mnetMib.rss,
	 sizeof(Rss),	 0, 0 },				/* 50 */
	{MIB_rlcMac,		(void *)&mnetMib.rlcMac,
	 sizeof(RlcMac),	 0, 0 },				/* 51 */
	{MIB_bssgp,		(void *)&mnetMib.bssgp,
	 sizeof(Bssgp),	 0, 0 },				/* 52 */
	{MIB_btsGsMeasPackage,		(void *)&mnetMib.btsGsMeasPackage,
	 sizeof(BtsGsMeasPackage),	 0, 0 },		/* 53 */

	{MIB_gp10CdrMib,		(void *) &mnetMib.gp10CdrMib,
	 sizeof(Gp10CdrMib), 0, 0 }, /* 54 */
	
	{MIB_handoverTimerMib, (void *) &mnetMib.handoverTimerMib,
	sizeof(HandoverTimerMib), 0, 0},	/* 55 */

	{MIB_gp10ServiceStatusMib, (void *) &mnetMib.gp10ServiceStatusMib,
	sizeof(Gp10ServiceStatusMib), 0, 0}, /* 56 */

	{MIB_cellAllocationTable,	 0,					/* Start of Table */
	 sizeof(CellAllocationEntry),	 CellAllocationTableLimit, 0},		/* Descriptors.   */
	{MIB_plmnPermittedTable,	 0,
	 sizeof(PlmnPermittedEntry),	 PlmnPermittedTableLimit, 0},
	{MIB_notAllowedAccessClassTable, 0,
	 sizeof(NotAllowedAccessClassEntry), NotAllowedAccessClassTableLimit, 0},
	{MIB_channel_0_Table,		 0,
	 sizeof(ChannelEntry),		 ChannelTableLimit, 0},
	{MIB_channel_1_Table,		 0,
	 sizeof(ChannelEntry),		 ChannelTableLimit, 0},
	{MIB_carrierFrequency_0_Table,	 0,
	 sizeof(CarrierFrequencyEntry),	 CarrierFrequencyTableLimit, 0},
	{MIB_carrierFrequency_1_Table,	 0,
	 sizeof(CarrierFrequencyEntry),	 CarrierFrequencyTableLimit, 0},
	{MIB_adjCell_handoverTable,	 0,
	 sizeof(AdjCell_HandoverEntry),	 HandoverTableLimit, 0},
	{MIB_adjCell_reselectionTable,	 0,
	 sizeof(AdjCell_ReselectionEntry), ReselectionTableLimit, 0},
	{MIB_mobileAllocationTable,	 0,
	 sizeof(MobileAllocationEntry),	 MobileAllocationTableLimit, 0},
	{MIB_attImmediateAssingProcsPerCauseTable, 0,
	 sizeof (AttImmediateAssingProcsPerCauseEntry), AttImmediateAssingProcsPerCauseTableLimit, 0},
	{MIB_succImmediateAssingProcsPerCauseTable, 0,
	 sizeof (SuccImmediateAssingProcsPerCauseEntry), SuccImmediateAssingProcsPerCauseTableLimit, 0},
	{MIB_externalHDOsPerCauseTable,	 0,
	 sizeof (ExternalHDOsPerCauseEntry), ExternalHDOsPerCauseTableLimit, 0},
	{MIB_arfcnRssiClockControlTable, 0,
	 sizeof(ArfcnRssiClockControlEntry), ArfcnRssiClockControlTableLimit, 0},
	{MIB_apaInvalidFrequencyTable,	 0,
	 sizeof(ApaInvalidFrequencyEntry), ApaInvalidFrequencyTableLimit, 0},
	{MIB_apaClockSourceFrequencyTable, 0,
	 sizeof(ApaClockSourceFrequencyEntry), ApaClockSourceFrequencyTableLimit, 0},
	{MIB_trapFwdTable,		 0,
	 sizeof(TrapFwdEntry), TrapFwdTableLimit, 0},

	/* New Tables to support Gs */
	{MIB_ncFrequencyListTable,		 0,
	 sizeof(NcFrequencyListEntry), NcFrequencyListTableLimit, 0},
	
	/* String buffers. */
	{MIB_oamAlternateMibFileName,	   0, SIZE_DisplayString, 0, 0},
	{MIB_oamCommandLastError,	      0, SIZE_DisplayString, 0, 0},
                                    
   {MIB_trxSerialNumber,		      0, SIZE_DisplayString, 0, 0},
	{MIB_trxSoftwareVersion,	      0, SIZE_DisplayString, 0, 0},
	{MIB_clockCardSoftwareVersion,   0, SIZE_DisplayString, 0, 0},
	{MIB_clockCardCrystalUpTime,	   0, SIZE_DisplayString, 0, 0},
	{MIB_gpsCardSoftwareVersion,	   0, SIZE_DisplayString, 0, 0},
	{MIB_gpsPosition,		            0, SIZE_DisplayString, 0, 0},
	{MIB_gpsTime,			            0, SIZE_DisplayString, 0, 0},
	{MIB_utcTime,			            0, SIZE_DisplayString, 0, 0},
	{MIB_cdcBoardSerialNumber,	      0, SIZE_DisplayString, 0, 0},
	{MIB_cdcBoardMACAddress,	      0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellName,		         0, SIZE_DisplayString, 0, 0},
   {MIB_viperCellLocation,		      0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellSerialNumber,	   0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellAssetNumber,	      0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellSoftwareBuild,	   0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellCustomerName,	   0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellCustomerAddress,   0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellCustomerPhone,	   0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellCustomerEmail,	   0, SIZE_DisplayString, 0, 0},
	{MIB_viperCellCustomerInfo,	   0, SIZE_DisplayString, 0, 0},
	{MIB_maintenance_operation,	   0, SIZE_DisplayString, 0, 0},
	{MIB_maintenance_response,	      0, SIZE_DisplayString, 0, 0},
	{MIB_readCommunity,		         0, SIZE_DisplayString, 0, 0},
  	{MIB_writeCommunity,		         0, SIZE_DisplayString, 0, 0},
	{0, 0, 0, 0, 0}
};

OidPrefix	oidPrefix[] = {
	{MIB_btsBasicPackage,			"1.3.6.1.4.1.3583.1.1.1"},	/*  0 */
	{MIB_btsOptionsPackage,			"1.3.6.1.4.1.3583.1.1.2"},	/*  1 */
	{MIB_firstTransceiverPackage,		"1.3.6.1.4.1.3583.1.1.3"},	/*  2 */
	{MIB_secondTransceiverPackage,		"1.3.6.1.4.1.3583.1.1.4"},	/*  3 */
	{MIB_firstRadioCarrierPackage,		"1.3.6.1.4.1.3583.1.1.5"},	/*  4 */
	{MIB_secondRadioCarrierPackage,		"1.3.6.1.4.1.3583.1.1.6"},	/*  5 */
	{MIB_btsTimerPackage,			"1.3.6.1.4.1.3583.1.1.7"},	/*  6 */
	{MIB_t200,				"1.3.6.1.4.1.3583.1.1.7.1"},	/*  7 */
	{MIB_t31xx,				"1.3.6.1.4.1.3583.1.1.7.2"},	/*  8 */
	{MIB_adjacentCellPackage,		"1.3.6.1.4.1.3583.1.1.8"},	/*  9 */
	{MIB_frequencyHoppingPackage,		"1.3.6.1.4.1.3583.1.1.9"},	/* 10 */
	{MIB_handoverControlPackage,		"1.3.6.1.4.1.3583.1.1.10"},	/* 11 */
	{MIB_hoAveragingAdjCellParam,		"1.3.6.1.4.1.3583.1.1.10.3"},	/* 12 */
	{MIB_hoAveragingDistParam,		"1.3.6.1.4.1.3583.1.1.10.4"},	/* 13 */
	{MIB_hoAveragingLevParam,		"1.3.6.1.4.1.3583.1.1.10.5"},	/* 14 */
	{MIB_hoAveragingQualParam,		"1.3.6.1.4.1.3583.1.1.10.6"},	/* 15 */
	{MIB_hoThresholdDistParam,		"1.3.6.1.4.1.3583.1.1.10.8"},	/* 16 */
	{MIB_hoThresholdInterfaceParam,		"1.3.6.1.4.1.3583.1.1.10.9"},	/* 17 */
	{MIB_hoThresholdLevParam,		"1.3.6.1.4.1.3583.1.1.10.10"},	/* 18 */
	{MIB_hoThresholdQualParam,		"1.3.6.1.4.1.3583.1.1.10.11"},	/* 19 */
	{MIB_interferenceAveragingParam,	"1.3.6.1.4.1.3583.1.1.10.12"},	/* 20 */
	{MIB_powerControlPackage,		"1.3.6.1.4.1.3583.1.1.11" },	/* 21 */
	{MIB_pcAveragingLev,			"1.3.6.1.4.1.3583.1.1.11.1"},	/* 22 */
	{MIB_pcAveragingQual,			"1.3.6.1.4.1.3583.1.1.11.2"},	/* 23 */
	{MIB_pcLowerThresholdLevParam,		"1.3.6.1.4.1.3583.1.1.11.3"},	/* 24 */
	{MIB_pcLowerThresholdQualParam,		"1.3.6.1.4.1.3583.1.1.11.4"},	/* 25 */
	{MIB_pcUpperThresholdLevParam,		"1.3.6.1.4.1.3583.1.1.11.5"},	/* 26 */
	{MIB_pcUpperThresholdQualParam,		"1.3.6.1.4.1.3583.1.1.11.6"},	/* 27 */
	{MIB_cellMeasurmentPackage,		"1.3.6.1.4.1.3583.1.1.12"},	/* 28 */
	{MIB_callControlMeasurmentPackage,	"1.3.6.1.4.1.3583.1.1.13"},	/* 29 */
	{MIB_callConfigData,			"1.3.6.1.4.1.3583.1.3.1"},	/* 30 */
	{MIB_mobilityConfigData,		"1.3.6.1.4.1.3583.1.3.2"},	/* 31 */
	{MIB_resourceConfigData,		"1.3.6.1.4.1.3583.1.3.3"},	/* 32 */
	{MIB_oamConfigData,			"1.3.6.1.4.1.3583.1.3.4"},	/* 33 */
	{MIB_h323Data,				"1.3.6.1.4.1.3583.1.2"},	/* 34 */
	{MIB_dualTRXSCard,			"1.3.6.1.4.1.3583.1.4.1"},	/* 35 */
	{MIB_arfcnRssiFunction,			"1.3.6.1.4.1.3583.1.4.2"},	/* 36 */
	{MIB_clockCard,				"1.3.6.1.4.1.3583.1.4.3"},	/* 37 */
	{MIB_gpsCard,				"1.3.6.1.4.1.3583.1.4.4"},	/* 38 */
	{MIB_powerCard,				"1.3.6.1.4.1.3583.1.4.5"},	/* 39 */
	{MIB_externalPowerSupply,		"1.3.6.1.4.1.3583.1.4.6"},	/* 40 */
	{MIB_cdcBoard,				"1.3.6.1.4.1.3583.1.4.7"},	/* 41 */
	{MIB_automaticParameterAllocation,	"1.3.6.1.4.1.3583.1.4.8"},	/* 42 */
	{MIB_gp10MgmtInfoMib,		"1.3.6.1.4.1.3583.1.5"},	/* 43 */
	{MIB_maintenanceConfigData,		"1.3.6.1.4.1.3583.1.3.5"},	/* 44 */
	{MIB_gp10ErrorInfoMib,		"1.3.6.1.4.1.3583.1.6"},	/* 45 */
	{MIB_viperAccessMib,			"1.3.6.1.4.1.3583.3"},		/* 46 */
	{MIB_trapFwdData,			"1.3.6.1.4.1.3583.1.3.6"},	/* 47 */
	
	/* New OIDs to support GS */
	{MIB_btsGsBasicPackage,			"1.3.6.1.4.1.3583.1.7.1"},	/* 48 */
	{MIB_btsGsOptionPackage,		"1.3.6.1.4.1.3583.1.7.2"},	/* 49 */
	{MIB_rss,					"1.3.6.1.4.1.3583.1.7.3.1"},	/* 50 */
	{MIB_rlcMac,				"1.3.6.1.4.1.3583.1.7.3.2"},	/* 51 */
	{MIB_bssgp,					"1.3.6.1.4.1.3583.1.7.3.3"},	/* 52 */
	{MIB_btsGsMeasPackage,			"1.3.6.1.4.1.3583.1.7.4"},	/* 53 */
	{MIB_gp10CdrMib,			"1.3.6.1.4.1.3583.1.8"},	/* 54 */
	{MIB_handoverTimerMib,		"1.3.6.1.4.1.3583.1.9"},	/* 55 */
	{MIB_gp10ServiceStatusMib,	"1.3.6.1.4.1.3583.1.10"},	/* 56 */

	{0, 0}
};

#if 0
Mib2Access	mib2Access[] = {
	{}	/* 0 = System */
	{}	/* 1 = Interfaces */
	{}	/* 2 = At */
	{}	/* 3 = Ip */
	{}	/* 4 = Icmp */
	{}	/* 5 = Tcp */
	{}	/* 6 = Udp */
	{}	/* 7 = Egp */
	{}	/* 8 = Transmisison */
	{}	/* 9 = Snmp */
	{0,0}
}
#endif

/*   WARNING!!	At runtime, this table gets sorted to permit more efficient access. */

MnetMibDataType mnetMibDataType[] = {
	DTYPE (MIB_btsBasicPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_bts_ncc, TYPE_NetworkColourCode, 0, 0),
	DTYPE (MIB_bts_cid, TYPE_CellIdentity, 0, 0),
	DTYPE (MIB_btsID, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_cellAllocationTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_cellAllocationEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_cellAllocationIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_cellAllocation, TYPE_AbsoluteRFChannelNo, 0, 0),
	DTYPE (MIB_gsmdcsIndicator, TYPE_GsmdcsIndicator, 0, 0),
	DTYPE (MIB_bts_mcc, TYPE_MobileCountryCode, 0, 0),
	DTYPE (MIB_bts_mnc, TYPE_MobileNetworkCode, 0, 0),
	DTYPE (MIB_bts_lac, TYPE_LocationAreaCode, 0, 0),
	DTYPE (MIB_bts_ci, TYPE_CellIdentity, 0, 0),
	DTYPE (MIB_cellReselectHysteresis, TYPE_CellReselectHysteresis, 7, 0),
	DTYPE (MIB_ny1, TYPE_Ny1, 3, 0),
	DTYPE (MIB_plmnPermittedTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_plmnPermittedEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_plmnPermittedIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_plmnPermitted, TYPE_PlmnPermitted, 0, 0),
	DTYPE (MIB_radioLinkTimeout, TYPE_RadioLinkTimeout, 4, 0),		/* aloha 15 */
	DTYPE (MIB_relatedTranscoder, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_rxLevAccessMin, TYPE_RxLev, 10, 0),
	DTYPE (MIB_bts_administrativeState, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIB_bts_alarmStatus, TYPE_AlarmStatus, 0, 0),
	DTYPE (MIB_bts_operationalState, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_maxNumberRetransmissions, TYPE_MaxRetrans, 4, 0),
	DTYPE (MIB_mSTxPwrMaxCCH, TYPE_TxPower, 0, 0),
	DTYPE (MIB_numberOfSlotsSpreadTrans, TYPE_TxInteger, 12, 0),
	DTYPE (MIB_noOfBlocksForAccessGrant, TYPE_NoOfBlocksForAccessGrant, 0, 0),
	DTYPE (MIB_noOfMultiframesBetweenPaging, TYPE_NoOfMultiframesBetweenPaging, 9, 0),
	DTYPE (MIB_btsOptionsPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_allowIMSIAttachDetach, TYPE_Boolean, 1, 0),
	DTYPE (MIB_callReestablishmentAllowed, TYPE_Boolean, 1, 0),
	DTYPE (MIB_cellBarred, TYPE_Boolean, 0, 0),
	DTYPE (MIB_dtxDownlink, TYPE_Boolean, 0, 0),
	DTYPE (MIB_dtxUplink, TYPE_DtxUplink, 2, 0),
	DTYPE (MIB_emergencyCallRestricted, TYPE_Boolean, 0, 0),
	DTYPE (MIB_notAllowedAccessClassTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_notAllowedAccessClassEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_notAllowedAccessClassIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_notAllowedAccessClass, TYPE_ClassNumber, 0, 0),
	DTYPE (MIB_timerPeriodicUpdateMS, TYPE_TimerPeriodicUpdateMS, 30, 0),
	DTYPE (MIB_maxQueueLength, TYPE_MaxQueueLength, 0, 0),
	DTYPE (MIB_msPriorityUsedInQueueing, TYPE_Boolean, 0, 0),
	DTYPE (MIB_timeLimitCall, TYPE_QueueTimeLimit, 0, 0),
	DTYPE (MIB_timeLimitHandover, TYPE_QueueTimeLimit, 0, 0),
	DTYPE (MIB_firstTransceiverPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_basebandTransceiverID_0, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_relatedRadioCarrier_0, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_basebandAdministrativeState_0, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIB_basebandAlarmStatus_0, TYPE_AlarmStatus, 0, 0),
	DTYPE (MIB_basebandOperationalState_0, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_channel_0_Table, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_channel_0_Entry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_channelIndex_0, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_channelID_0, TYPE_ChannelID, 0, 0),
	DTYPE (MIBT_channelCombination_0, TYPE_ChannelCombination, 0, 0),
	DTYPE (MIBT_channelFrequencyUsage_0, TYPE_FrequencyUsage, 0, 0),
	DTYPE (MIBT_channelTsc_0, TYPE_Tsc, 0, 0),
	DTYPE (MIBT_channelAdministrativeState_0, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIBT_channelOperationalState_0, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_secondTransceiverPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_basebandTransceiverID_1, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_relatedRadioCarrier_1, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_basebandAdministrativeState_1, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIB_basebandAlarmStatus_1, TYPE_AlarmStatus, 0, 0),
	DTYPE (MIB_basebandOperationalState_1, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_channel_1_Table, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_channel_1_Entry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_channelIndex_1, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_channelID_1, TYPE_ChannelID, 0, 0),
	DTYPE (MIBT_channelCombination_1, TYPE_ChannelCombination, 0, 0),
	DTYPE (MIBT_channelFrequencyUsage_1, TYPE_FrequencyUsage, 0, 0),
	DTYPE (MIBT_channelTsc_1, TYPE_Tsc, 0, 0),
	DTYPE (MIBT_channelAdministrativeState_1, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIBT_channelOperationalState_1, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_firstRadioCarrierPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_carrierFrequency_0_Table, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_carrierFrequency_0_Entry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_carrierFrequencyIndex_0, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_carrierFrequency_0, TYPE_AbsoluteRFChannelNo, 0, 0),
	DTYPE (MIB_powerClass_0, TYPE_PowerClass, 2, 0),
	DTYPE (MIB_radioCarrierID_0, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_txPwrMaxReduction_0, TYPE_TxPwrMaxReduction, 0, 0),
	DTYPE (MIB_carrier_administrativeState_0, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIB_carrier_alarmStatus_0, TYPE_AlarmStatus, 0, 0),
	DTYPE (MIB_carrier_operationalState_0, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_secondRadioCarrierPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_carrierFrequency_1_Table, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_carrierFrequency_1_Entry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_carrierFrequencyIndex_1, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_carrierFrequency_1, TYPE_AbsoluteRFChannelNo, 0, 0),
	DTYPE (MIB_powerClass_1, TYPE_PowerClass, 2, 0),
	DTYPE (MIB_radioCarrierID_1, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_txPwrMaxReduction_1, TYPE_TxPwrMaxReduction, 0, 0),
	DTYPE (MIB_carrier_administrativeState_1, TYPE_AdministrativeState, 0, 0),
	DTYPE (MIB_carrier_alarmStatus_1, TYPE_AlarmStatus, 0, 0),
	DTYPE (MIB_carrier_operationalState_1, TYPE_OperationalState, 0, 0),
	DTYPE (MIB_btsTimerPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_t200, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_sdcchSAPI0, TYPE_L2Timer, 103, 0),
	DTYPE (MIB_facchTCHF, TYPE_L2Timer, 77, 0),
	DTYPE (MIB_facchTCHH, TYPE_L2Timer, 77, 0),
	DTYPE (MIB_sacchTCHSAPI0, TYPE_L2Timer, 448, 0),
	DTYPE (MIB_sacchSDCCH, TYPE_L2Timer, 448, 0),
	DTYPE (MIB_sdcchSAPI3, TYPE_L2Timer, 103, 0),
	DTYPE (MIB_sdcchTCHSAPI3, TYPE_L2Timer, 103, 0),

	DTYPE (MIB_handoverTimerMib, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_t101, TYPE_INTEGER, 200, 0),
	DTYPE (MIB_t102, TYPE_INTEGER, 500, 0),
	DTYPE (MIB_t103, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_t104, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_t201, TYPE_INTEGER, 200, 0),
	DTYPE (MIB_t202, TYPE_INTEGER, 500, 0),
	DTYPE (MIB_t204, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_t210, TYPE_INTEGER, 500, 0),
	DTYPE (MIB_t211, TYPE_INTEGER, 2000, 0),

	DTYPE (MIB_gp10ServiceStatusMib, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_snmpServiceEnabled, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_httpServiceEnabled, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_telnetServiceEnabled, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_sshServiceEnabled, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_ftpServiceEnabled, TYPE_INTEGER, 1, 0),

	DTYPE (MIB_t31xx, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_t3101, TYPE_L3Timer, 767, 0),
	DTYPE (MIB_t3103, TYPE_L3Timer, 0, 0),
	DTYPE (MIB_t3105, TYPE_L3Timer, 1023, 0),
	DTYPE (MIB_t3107, TYPE_L3Timer, 0, 0),
	DTYPE (MIB_t3109, TYPE_L3Timer, 511, 0),
	DTYPE (MIB_t3111, TYPE_L3Timer, 767, 0),
	DTYPE (MIB_t3113, TYPE_L3Timer, 1800, 0),
	DTYPE (MIB_adjacentCellPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_adjCell_handoverTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_adjCell_handoverEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_adjCell_handoverIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_adjCell_handoverCellID, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIBT_adjCell_mcc, TYPE_MobileCountryCode, 0, 0),
	DTYPE (MIBT_adjCell_mnc, TYPE_MobileNetworkCode, 0, 0),
	DTYPE (MIBT_adjCell_lac, TYPE_LocationAreaCode, 0, 0),
	DTYPE (MIBT_adjCell_ci, TYPE_CellIdentity, 0, 0),
	DTYPE (MIBT_adjCell_bCCHFrequency, TYPE_AbsoluteRFChannelNo, 0, 0),
	DTYPE (MIBT_adjCell_ncc, TYPE_NetworkColourCode, 0, 0),
	DTYPE (MIBT_adjCell_cid, TYPE_CellIdentity, 0, 0),
	DTYPE (MIBT_adjCell_synchronized, TYPE_Boolean, 0, 0),
	DTYPE (MIBT_adjCell_hoPriorityLevel, TYPE_HoPriorityLevel, 0, 0),
	DTYPE (MIBT_adjCell_hoMargin, TYPE_HoMargin, 0, 0),
	DTYPE (MIBT_adjCell_msTxPwrMaxCell, TYPE_TxPower, 0, 0),
	DTYPE (MIBT_adjCell_rxLevMinCell, TYPE_RxLev, 0, 0),
	DTYPE (MIBT_adjCell_isExternal, TYPE_Boolean, 0, 0),
	DTYPE (MIB_adjCell_reselectionTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_adjCell_reselectionEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_adjCell_reselectionIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_adjCell_reselectionCellID, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIBT_adjCell_reselectionBCCHFrequency, TYPE_AbsoluteRFChannelNo, 0, 0),
	DTYPE (MIB_frequencyHoppingPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_frequencyHoppingSystemID, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_hoppingSequenceNumber, TYPE_HoppingSequenceNumber, 0, 0),
	DTYPE (MIB_mobileAllocationTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_mobileAllocationEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_mobileAllocationIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_mobileAllocation, TYPE_AbsoluteRFChannelNo, 0, 0),
	DTYPE (MIB_handoverControlPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_handoverControlID, TYPE_GSMGeneralObjectID, 0, 0),
	DTYPE (MIB_enableOptHandoverProcessing, TYPE_EnableHoType, 0, 0),
	DTYPE (MIB_hoAveragingAdjCellParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoAveragingAdjCellParamHreqave, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingAdjCellParamHreqt, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingAdjCellParamWeighting, TYPE_Weighting, 0, 0),
	DTYPE (MIB_hoAveragingDistParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoAveragingDistParamHreqave, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingDistParamHreqt, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingLevParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoAveragingLevParamHreqave, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingLevParamHreqt, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingLevParamWeighting, TYPE_Weighting, 0, 0),
	DTYPE (MIB_hoAveragingQualParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoAveragingQualParamHreqave, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingQualParamHreqt, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoAveragingQualParamWeighting, TYPE_Weighting, 0, 0),
	DTYPE (MIB_hoMarginDef, TYPE_HoMargin, 6, 0),		/* aloha 0 */
	DTYPE (MIB_hoThresholdDistParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoThresholdDistParamTimeadv, TYPE_MSRangeMax, 0, 0),
	DTYPE (MIB_hoThresholdDistParamP8, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoThresholdDistParamN8, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoThresholdInterfaceParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoThresholdInterfaceParamRxLevelUL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_hoThresholdInterfaceParamRxLevelDL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_hoThresholdInterfaceParamPx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoThresholdInterfaceParamNx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_hoThresholdLevParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoThresholdLevParamRxLevelUL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_hoThresholdLevParamRxLevelDL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_hoThresholdLevParamPx, TYPE_AveragingNumber, 6, 0),
	DTYPE (MIB_hoThresholdLevParamNx, TYPE_AveragingNumber, 8, 0),
	DTYPE (MIB_hoThresholdQualParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_hoThresholdQualParamRxQualUL, TYPE_RxQual, 0, 0),
	DTYPE (MIB_hoThresholdQualParamRxQualDL, TYPE_RxQual, 0, 0),
	DTYPE (MIB_hoThresholdQualParamPx, TYPE_AveragingNumber, 3, 0),
	DTYPE (MIB_hoThresholdQualParamNx, TYPE_AveragingNumber, 3, 0),
	DTYPE (MIB_interferenceAveragingParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_interferenceAveragingParamAveragingPeriod, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_interferenceAveragingParamThresholdBoundary0, TYPE_RxLev, 0, 0),
	DTYPE (MIB_interferenceAveragingParamThresholdBoundary1, TYPE_RxLev, 0, 0),
	DTYPE (MIB_interferenceAveragingParamThresholdBoundary2, TYPE_RxLev, 0, 0),
	DTYPE (MIB_interferenceAveragingParamThresholdBoundary3, TYPE_RxLev, 0, 0),
	DTYPE (MIB_interferenceAveragingParamThresholdBoundary4, TYPE_RxLev, 0, 0),
	DTYPE (MIB_interferenceAveragingParamThresholdBoundary5, TYPE_RxLev, 0, 0),
	DTYPE (MIB_msTxPwrMaxCellDef, TYPE_TxPower, 0, 0),
	DTYPE (MIB_rxLevMinCellDef, TYPE_RxLev, 0, 0),
	DTYPE (MIB_powerControlPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcAveragingLev, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcAveragingLevHreqave, TYPE_AveragingNumber, 2, 0),
	DTYPE (MIB_pcAveragingLevHreqt, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcAveragingLevWeighting, TYPE_Weighting, 0, 0),
	DTYPE (MIB_pcAveragingQual, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcAveragingQualHreqave, TYPE_AveragingNumber, 2, 0),
	DTYPE (MIB_pcAveragingQualHreqt, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcAveragingQualWeighting, TYPE_Weighting, 0, 0),
	DTYPE (MIB_pcLowerThresholdLevParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcLowerThresholdLevParamRxLevelUL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_pcLowerThresholdLevParamRxLevelDL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_pcLowerThresholdLevParamPx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcLowerThresholdLevParamNx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcLowerThresholdQualParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcLowerThresholdQualParamRxQualUL, TYPE_RxQual, 0, 0),
	DTYPE (MIB_pcLowerThresholdQualParamRxQualDL, TYPE_RxQual, 0, 0),
	DTYPE (MIB_pcLowerThresholdQualParamPx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcLowerThresholdQualParamNx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcUpperThresholdLevParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcUpperThresholdLevParamRxLevelUL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_pcUpperThresholdLevParamRxLevelDL, TYPE_RxLev, 0, 0),
	DTYPE (MIB_pcUpperThresholdLevParamPx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcUpperThresholdLevParamNx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcUpperThresholdQualParam, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pcUpperThresholdQualParamRxQualUL, TYPE_RxQual, 0, 0),
	DTYPE (MIB_pcUpperThresholdQualParamRxQualDL, TYPE_RxQual, 0, 0),
	DTYPE (MIB_pcUpperThresholdQualParamPx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_pcUpperThresholdQualParamNx, TYPE_AveragingNumber, 0, 0),
	DTYPE (MIB_powerControlInterval, TYPE_PowerControlInterval, 0, 0),
	DTYPE (MIB_powerIncrStepSize, TYPE_PowerIncrStepSize, 0, 0),
	DTYPE (MIB_powerRedStepSize, TYPE_PowerRedStepSize, 0, 0),
	DTYPE (MIB_cellMeasurmentPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_meanPCHAGCHQueueLength, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attTransOfPagingMessagesThePCH, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_unsuccTransOfPagingMessagesThePCH, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attImmediateAssingProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succImmediateAssingProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attImmediateAssingProcsPerCauseTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_attImmediateAssingProcsPerCauseEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_attImmediateAssingProcsCause, TYPE_CauseCode, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_attImmediateAssingProcsValue, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succImmediateAssingProcsPerCauseTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_succImmediateAssingProcsPerCauseEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_succImmediateAssingProcsCause, TYPE_CauseCode, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_succImmediateAssingProcsValue, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_nbrOfPagesDiscardedFromPCHQueue, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanDurationOfSuccPagingProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_nbrOfAvailableTCHs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanNbrOfBusyTCHs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_maxNbrOfBusyTCHs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanNbrOfIdleTCHsPerInterferenceBand, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attTCHSeizures, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succTCHSeizures, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attTCHSeizuresMeetingTCHBlockedState, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_allAvailableTCHAllocatedTime, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanTCHBusyTime, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanTCHQueueLength, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_nbrOfLostRadioLinksTCH, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_nbrOfAvailableSDCCHs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanNbrOfBusySDCCHs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_maxNbrOfBusySDCCHs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attSDCCHSeizuresMeetingSDCCHBlockedState, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_allAvailableSDCCHAllocatedTime, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanSDCCHQueueLength, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_nbrOfLostRadioLinksSDCCH, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_relativeTimeDLPowerControlAtMax, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_relativeTimeULPowerControlAtMax, TYPE_INTEGER, 0, 0),
#if THESE_ARE_NOT_USED_IN_VIPERCELL
	DTYPE (MIB_succInternalHDOsIntraCell, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_unsuccInternalHDOsIntraCell, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attIncomingInternalInterCellHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succIncomingInternalInterCellHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attOutgoingInternalInterCellHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succOutgoingInternalInterCellHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_unsuccHDOsWithReconnection, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_unsuccHDOsWithLossOfConnection, TYPE_INTEGER, 0, 0),
#endif
	DTYPE (MIB_callControlMeasurmentPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_nbrOfClassMarkUpdates, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attMobileOriginatingCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succMobileOriginatingCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_ansMobileOriginatingCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attMobileTerminatingCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succMobileTerminatingCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_ansMobileTerminatingCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attMobileEmergencyCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succMobileEmergencyCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_ansMobileEmergencyCalls, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attCipheringModeControlProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succCipheringModeControlProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attInterrogationOfHLRsForRouting, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succInterrogationOfHLRsMSRNObtained, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succInterrogationOfHLRsCallForwarding, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attOpForMobileOriginatingPointToPointSMs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succOpForMobileOriginatingPointToPointSMs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attOpForMobileTerminatingPointToPointSMs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succOpForMobileTerminatingPointToPointSMs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanTimeToCallSetupService, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_meanTimeToLocationUpdateService, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_transSubIdentifiedWithIMSI, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_imsiDetachProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_imsiAttachProcs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attIncomingInterMSCHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succIncomingInterMSCHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_attOutgoingInterMSCHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_succOutgoingInterMSCHDOs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_externalHDOsPerCauseTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_externalHDOsPerCauseEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_externalHDOsPerCauseCode, TYPE_CauseCode, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_externalHDOsPerCauseValue, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_unsuccExternHDOsWithReconnectionPerMSC, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_unsuccExternHDOsWithLossOfConnectionPerMSC, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_callConfigData, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_cc_max_L3_msgs, TYPE_INTEGER, 32, 0),
	DTYPE (MIB_cc_l3_msg_q_priority, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_cc_task_priority, TYPE_INTEGER, 100, 0),
	DTYPE (MIB_cc_task_stack_size, TYPE_INTEGER, 10000, 0),
	DTYPE (MIB_cc_mm_conn_est_T999, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_alerting_T301, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_setup_T303, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_call_confirmed_T310, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_connect_T313, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_disconnect_T305, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_release_T308, TYPE_INTEGER, 2000, 0),
	DTYPE (MIB_cc_safety_timer, TYPE_INTEGER, 1000, 0),
	DTYPE (MIB_cc_max_calls, TYPE_INTEGER, 7, 0),
	DTYPE (MIB_mobilityConfigData, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_mm_max_mm_connections, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_mm_max_L3_msgs, TYPE_INTEGER, 32, 0),
	DTYPE (MIB_task_priority, TYPE_INTEGER, 100, 0),
	DTYPE (MIB_task_stack_size, TYPE_INTEGER, 20000, 0),
	DTYPE (MIB_mm_paging_t3113, TYPE_INTEGER, 3000, 0),
	DTYPE (MIB_mm_authenticationRequired, TYPE_Boolean, 0, 0),
	DTYPE (MIB_mm_cipheringRequired, TYPE_Boolean, 0, 0),
	DTYPE (MIB_mm_IMEICheckRequired, TYPE_Boolean, 0, 0),
	DTYPE (MIB_resourceConfigData, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_rm_t3L01, TYPE_INTEGER, 0x1FF, 0),
	DTYPE (MIB_rm_bsCcChans, TYPE_INTEGER, 0x01, 0),
	DTYPE (MIB_rm_bsCcchSdcchComb, TYPE_INTEGER, 0x01, 0),
	DTYPE (MIB_rm_sysInfoOnOffMap, TYPE_INTEGER, 0x3E, 0),
	DTYPE (MIB_rm_airInterface, TYPE_INTEGER, 0x02, 0),
	DTYPE (MIB_rm_viperCellTsc, TYPE_INTEGER, 0x05, 0),
	DTYPE (MIB_rm_maxSlotPerTrx, TYPE_INTEGER, 0x08, 0),
	DTYPE (MIB_rm_maxTrxs, TYPE_INTEGER, 0x02, 0),
	DTYPE (MIB_rm_maxTchfs, TYPE_INTEGER, 0x0F, 0),
	DTYPE (MIB_rm_maxSdcch4s, TYPE_INTEGER, 0x04, 0),
	DTYPE (MIB_rm_trxOnOffMap, TYPE_INTEGER, 0x01, 0),
	DTYPE (MIB_rm_networkIfConfig, TYPE_INTEGER, 0x00, 0),
	DTYPE (MIB_rm_bcchTrx, TYPE_INTEGER, 0x00, 0),
	DTYPE (MIB_rm_preAlphaTrx, TYPE_INTEGER, 0x00, 0),
	DTYPE (MIB_rm_ccchBcchComb, TYPE_INTEGER, 0x01, 0),
	DTYPE (MIB_rm_ccchConf, TYPE_INTEGER, 0x01, 0),
	DTYPE (MIB_rm_nim_0_0, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_1, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_2, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_3, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_4, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_5, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_6, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_0_7, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_0, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_1, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_2, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_3, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_4, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_5, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_6, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_nim_1_7, TYPE_INTEGER, 0xFF, 0),
	DTYPE (MIB_rm_trxSynthSetting_0_0, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_0_1, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_0_2, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_0_3, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_1_0, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_1_1, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_1_2, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxSynthSetting_1_3, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxFpgaDelay_0_0, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxFpgaDelay_0_1, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxFpgaDelay_1_0, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_trxFpgaDelay_1_1, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_rm_SGainCtrlSetting_0_0, TYPE_INTEGER, 0x380, 0),
	DTYPE (MIB_rm_SGainCtrlSetting_0_1, TYPE_INTEGER, 0xc0, 0),
	DTYPE (MIB_rm_SGainCtrlSetting_1_0, TYPE_INTEGER, 0x380, 0),
	DTYPE (MIB_rm_SGainCtrlSetting_1_1, TYPE_INTEGER, 0xc0, 0),
	DTYPE (MIB_rm_synchChanInfo, TYPE_INTEGER, 0, 0),

	DTYPE (MIB_oamConfigData, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_oamCommand, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_oamCommandExecutionStatus, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_oamAlternateMibFileName, TYPE_String, 0, 0),
	DTYPE (MIB_oamCommandLastError, TYPE_String, 0, SAVE_ASCII_OMIT),
   DTYPE (MIB_mibAccessControlFlag, TYPE_INTEGER, 0, 0),

   DTYPE (MIB_maintenance_operation, TYPE_String, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_maintenance_response, TYPE_String, 0, SAVE_ASCII_OMIT),

	DTYPE (MIB_trapFwdData,       TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_trapFwdTable,      TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_trapFwdEntry,     TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_trapFwdIndex,     TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_trapType,         TYPE_SnmpTrapType, 0, 0),
   DTYPE (MIBT_trapIpAddress,    TYPE_IpAddress, 0, 0),
	DTYPE (MIBT_trapPort,         TYPE_INTEGER, 162, 0),
   DTYPE (MIBT_trapCommunity,    TYPE_String, 0, 0),

	DTYPE (MIB_h323Data, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_h323_GKPort,	TYPE_INTEGER, 1720, 0),
	DTYPE (MIB_h323_GKIPAddress, TYPE_IpAddress, 0, 0),
	DTYPE (MIB_h323_Q931ResponseTimeOut, TYPE_INTEGER, 100, 0),
	DTYPE (MIB_h323_Q931ConnectTimeOut, TYPE_INTEGER, 150, 0),
	DTYPE (MIB_h323_Q931CallSignalingPort, TYPE_INTEGER, 500, 0),
	DTYPE (MIB_h323_t35CountryCode,	TYPE_INTEGER, 0, 0),
	DTYPE (MIB_h323_rasRequestTimeout, TYPE_INTEGER, 20, 0),
	DTYPE (MIB_h323_viperbaseHeartbeatTimeout, TYPE_INTEGER, 180, 0),

	DTYPE (MIB_dualTRXSCard, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_monitorReceiverPLL_LockDetectStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_referenceFrequencyPLL_LockDetectStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_transceiver_1_PLL_LockDetectStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_transceiver_2A_PLL_LockDetectStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_transceiver_2B_PLL_LockDetectStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_intermediateFrequency_LockDetectStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_trxSerialNumber, TYPE_String, 0, 0),
	DTYPE (MIB_trxSoftwareVersion, TYPE_String, 0, 0),
	DTYPE (MIB_trxLoopBackMode, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_arfcnRssiFunction, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_arfcnRssiControlMode, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_clockRecoveryControlMode, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_arfcnRssiClockControlTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_arfcnRssiClockControlEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_arfcnRssiClockControlIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_arfcnRssiClockControlArfcn, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_arfcnRssiClockRecoveryMode, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_clockCard, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_clockCardType, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_clockCardSoftwareVersion, TYPE_String, 0, 0),
	DTYPE (MIB_clockCardStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_clockCardCrystalUpTime, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_clockCardDAC, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gpsCard, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_gpsCardType, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gpsCardSoftwareVersion, TYPE_String, 0, 0),
	DTYPE (MIB_gpsCardStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gpsPosition, TYPE_String, 0, 0),
	DTYPE (MIB_gpsTime, TYPE_String, 0, 0),
	DTYPE (MIB_utcTime, TYPE_String, 0, 0),
	DTYPE (MIB_clockStatusFrequencyError, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_clockStatusDACVoltage, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gpsAntennaStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gpsSatelliteStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_powerCard, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_powerCardType, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_powerCardStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_powerOn, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_externalPowerSupply, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_powerSupplyType, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_powerSupplyStatus, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_cdcBoard, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_cdcBoardSerialNumber, TYPE_String, 0, 0),
	DTYPE (MIB_cdcBoardMACAddress, TYPE_String, 0, 0),
	DTYPE (MIB_automaticParameterAllocation, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_apaEnable, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaScanFrequencyLowBound, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaScanFrequencyHighBound, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaMaxPowerSetting, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaMinRSSIThresholdForNeighborId, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaScanRate, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaHysteresisForBCCHPower, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_apaInvalidFrequencyTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_apaInvalidFrequencyEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_apaInvalidFrequencyIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_apaInvalidFrequencyArfcn, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_apaClockSourceFrequencyTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_apaClockSourceFrequencyEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_apaClockSourceFrequencyIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_apaClockSourceFrequencyArfcn, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_gp10MgmtInfoMib, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_viperCellName, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellIPAddress, TYPE_IpAddress, 0, 0),
	DTYPE (MIB_viperCellDefGateway, TYPE_IpAddress, 0, 0),
   DTYPE (MIB_viperCellLocation, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellSerialNumber, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellAssetNumber, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellSoftwareBuild, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellCustomerName, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellCustomerAddress, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellCustomerPhone, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellCustomerEmail, TYPE_String, 0, 0),
	DTYPE (MIB_viperCellCustomerInfo, TYPE_String, 0, 0),

   DTYPE (MIB_readCommunity, TYPE_String, 0, 0),
   DTYPE (MIB_writeCommunity, TYPE_String, 0, 0),


	DTYPE (MIB_gp10ErrorInfoMib, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_L1, TYPE_INTEGER,       0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_MD, TYPE_INTEGER,       0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_RM, TYPE_INTEGER,       0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_MM, TYPE_INTEGER,       0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_CC, TYPE_INTEGER,       0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_L2, TYPE_INTEGER,       0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_SMS, TYPE_INTEGER,      0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_OAM, TYPE_INTEGER,      0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_H323, TYPE_INTEGER,     0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_LUDB, TYPE_INTEGER,     0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_EXT_LC, TYPE_INTEGER,   0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_LOG, TYPE_INTEGER,      0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_MCH, TYPE_INTEGER,      0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_ALARM, TYPE_INTEGER,    0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_SYSINIT, TYPE_INTEGER,  0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_GRR, TYPE_INTEGER,      0, SAVE_ASCII_OMIT),
	DTYPE (MIB_errorCode_CDR, TYPE_INTEGER,      0, SAVE_ASCII_OMIT),

	DTYPE (MIB_cdrClientSSLEnable, TYPE_INTEGER,      0, 0),	/* disabled by default */
	DTYPE (MIB_cdrLongCallTimer, TYPE_INTEGER, 10, 0),	/* Default value is 15 mins */

	/* New Data Types to Support Gs */
	DTYPE (MIB_btsGsBasicPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_gprsServerIpAddress, TYPE_IpAddress, 0, 0),
	DTYPE (MIB_gprsRac, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_enableGprs, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gprsOperationalStatus, TYPE_INTEGER, 0, 0),

	DTYPE (MIB_btsGsOptionPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_nmo, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_t3168, TYPE_INTEGER, 4, 0),
	DTYPE (MIB_t3192, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_drxTimerMax, TYPE_INTEGER, 7, 0),
	DTYPE (MIB_accessBurstType, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_controlAckType, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_bsCvMax, TYPE_INTEGER, 7, 0),
	DTYPE (MIB_panDec, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_panInc, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_panMax, TYPE_INTEGER, 4, 0),
	DTYPE (MIB_raColour, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_si13Position, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_cbchTrxSlot, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_cbchTrx, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_bcchChangeMark, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_siChangeField, TYPE_INTEGER, 0, 0),

	DTYPE (MIB_rss, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_gprsMsTxpwrMaxCch, TYPE_INTEGER, 30, 0),
	DTYPE (MIB_gprsRexLevAccessMin, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_gprsReselectOffset, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_priorityClass, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_lsaId, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_hcsThreshold, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gprsTemporaryOffset, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gprsPenaltyTime, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_gprsCellReselectHysteresis, TYPE_INTEGER, 4, 0),
	DTYPE (MIB_gprsRaReselectHysteresis, TYPE_INTEGER, 4, 0),
	DTYPE (MIB_c32Qual, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_c31Hysteresis, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_alpha, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_pB, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_networkControlOrder, TYPE_INTEGER, 1, 0),

	DTYPE (MIB_ncFrequencyListTable, TYPE_TablePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_ncFrequencyListEntry, TYPE_TableEntryPointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_ncFrequencyListIndex, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIBT_ncFrequency, TYPE_INTEGER, 0, 0),

	DTYPE (MIB_ncReportingPeriodI, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_ncReportingPeriodT, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_extMeasurementOrder, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_extReportingPeriod, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_extReportingType, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_intFrequency, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_nccPermitted, TYPE_INTEGER, 0, 0),
	DTYPE (MIB_pcMeasChan, TYPE_INTEGER, 1, 0),
	DTYPE (MIB_tavgW, TYPE_INTEGER, 15, 0),
	DTYPE (MIB_tavgT, TYPE_INTEGER, 5, 0),
	DTYPE (MIB_navgI, TYPE_INTEGER, 4, 0),
	DTYPE (MIB_gprsMsTargetPower, TYPE_INTEGER, 30, 0),
	DTYPE (MIB_spgcCcchSup, TYPE_INTEGER, 0, 0),

	DTYPE (MIB_rlcMac, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_t3169, TYPE_INTEGER, 5, 0),
	DTYPE (MIB_t3191, TYPE_INTEGER, 5, 0),
	DTYPE (MIB_t3193, TYPE_INTEGER, 5, 0),
	DTYPE (MIB_t3195, TYPE_INTEGER, 5, 0),

	DTYPE (MIB_bssgp, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_blockTimer, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_resetTimer, TYPE_INTEGER, 60, 0),
	DTYPE (MIB_suspendTimer, TYPE_INTEGER, 5, 0),
	DTYPE (MIB_resumeTimer, TYPE_INTEGER, 5, 0),
	DTYPE (MIB_capUpdateTimer, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_blockRetries, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_unBlockRetries, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_resetRetries, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_suspendRetries, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_resumeRetries, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_raCapUpdateRetries, TYPE_INTEGER, 3, 0),
	DTYPE (MIB_timeInterval, TYPE_INTEGER, 10, 0),
	DTYPE (MIB_networkPort, TYPE_INTEGER, 22240, 0),

	DTYPE (MIB_btsGsMeasPackage, TYPE_StructurePointer, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_flushReqRecvd, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pagingReqRecvd, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_meanPsInterArrivalTime, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_availablePdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_meanAvailablePdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_maxAvailablePdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_minAvailablePdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_meanOccupiedPdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_maxOccupiedPdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_minOccupiedPdch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_availablePdchAllocTime, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_packetPagingMessages, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_pagchQueueLength, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),

	DTYPE (MIB_pagesDiscardedFromPpch, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_assignmentRequest, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_succAssignmentProc, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_succPdtchSeizures, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_meanPacketQueueLength, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),
	DTYPE (MIB_serviceChanges, TYPE_INTEGER, 0, SAVE_ASCII_OMIT),

	DTYPE (0, 0, 0, 0)
};

/* platform specific object ids */
OIDC_T   enterpriseOid [] = {1, 3, 6, 1, 4, 1, 3583, 1, 3, 6, 3};
OIDC_T   sysTrapOid [] = { 1, 3, 6, 1, 4, 1, 3583, 1, 3, 6, 2 };
OIDC_T   snmpTrapEventOid[] = { 1, 3, 6, 1, 4, 1, 3583, 1, 3, 6, 3,0,0};
OIDC_T   errorCodeOamOid[] = { 1, 3, 6, 1, 4, 1, 3583, 1, 6, 0};

OIDC_T   trapAlarmModuleNameOid[] = { 1, 3, 6, 1, 4, 1, 3583, 1, 3, 6, 4};
OIDC_T   trapAlarmCodeOid[] =       { 1, 3, 6, 1, 4, 1, 3583, 1, 3, 6, 5};
OIDC_T   trapAlarmDescOid[] =       { 1, 3, 6, 1, 4, 1, 3583, 1, 3, 6, 6};

int SIZE_enterpriseOid = sizeof enterpriseOid / sizeof enterpriseOid [0];
int SIZE_sysTrapOid = sizeof sysTrapOid / sizeof sysTrapOid [0];
int SIZE_snmpTrapEventOid = sizeof snmpTrapEventOid / sizeof snmpTrapEventOid [0];
int SIZE_errorCodeOamOid =  sizeof errorCodeOamOid / sizeof errorCodeOamOid [0];

int SIZE_trapAlarmModuleNameOid =  sizeof trapAlarmModuleNameOid / sizeof trapAlarmModuleNameOid[0];
int SIZE_trapAlarmCodeOid       =  sizeof trapAlarmCodeOid / sizeof trapAlarmCodeOid[0];
int SIZE_trapAlarmDescOid       =  sizeof trapAlarmDescOid / sizeof trapAlarmDescOid[0];


