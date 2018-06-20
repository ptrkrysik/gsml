#ifndef MIB_TAG
#include "mibleaf.h"

/*  A MIB tag is a 32 bit variable that uniquely defines the
 *  location of a MIB variable within the SNMP database.
 *  It contains several indicies and has the following format:
 */
#if 0                   /* Can't pass the structure easily as an arg */
typedef struct MibTag_t 
{       /* so use the int definition for MibTag. */
    unsigned    type:4;     /* Type of MIB item */
    unsigned     size:12;    /* Size or Item number of a table */
    unsigned    table:8;    /* MIB Table this item is in. */
    unsigned    item:8;     /* element # of the item. */
} MibTag;
#else
typedef int MibTag;
#endif

#define MTYPE_STRUCT    0
#define MTYPE_NUMBER    1
#define MTYPE_IPADDR    2
#define MTYPE_STRING    3
#define MTYPE_TABLE     4

#define MTYPE_TENTRY    8
#define MTYPE_TNUMBER   9   
#define MTYPE_TIPADDR   10
#define MTYPE_TSTRING   11


#define MTYPE_IS_TABLE(T)     (T>=8)
#define MTYPE_IS_SCALAR(T)    (T<8)
#define MTYPE_IS_INT(T)       ((T==MTYPE_NUMBER)  || (T== MTYPE_IPADDR))
#define MTYPE_IS_TINT(T)      ((T==MTYPE_TNUMBER) || (T== MTYPE_TIPADDR))
#define MTYPE_IS_STR(T)       (T==MTYPE_STRING)
#define MTYPE_IS_TSTR(T)      (T==MTYPE_TSTRING)

#define MIB_TAG(typ, siz, tbl, itm)    ((typ<<28) | (siz<<16) | (tbl<<8) | (itm))

#define MIB_STRUCT(tbl, itm, siz)    MIB_TAG (MTYPE_STRUCT,  siz, tbl, itm)
#define MIB_ITEM(tbl,   itm)         MIB_TAG (MTYPE_NUMBER,  4,   tbl, itm)
#define MIB_IPADDR(tbl, itm)         MIB_TAG (MTYPE_IPADDR,  4,   tbl, itm)
#define MIB_STRING(tbl, itm, siz)    MIB_TAG (MTYPE_STRING,  siz, tbl, itm)
#define MIB_TABLE(tbl,  itm)         MIB_TAG (MTYPE_TABLE,   4,   tbl, itm)

#define MIB_TENTRY(tbl, itm, siz)    MIB_TAG (MTYPE_TENTRY,  siz, tbl, itm)
#define MIB_TITEM(tbl,  itm, elmt)   MIB_TAG (MTYPE_TNUMBER, elmt,tbl, itm)
#define MIB_TIPADDR(tbl,itm, elmt)   MIB_TAG (MTYPE_TIPADDR, elmt,tbl, itm)
#define MIB_TSTRING(tbl,itm, elmt)   MIB_TAG (MTYPE_TSTRING, elmt,tbl, itm)



#define MIB_TYP(tag)    ((tag>>28)&0xF)
#define MIB_SIZ(tag)    ((tag>>16)&0xFFF)
#define MIB_TBL(tag)    ((tag>>8)&0xFF)
#define MIB_ITM(tag)    (tag&0xFF)

#define MTBL_btsBasicPackage                    0
#define MTBL_btsOptionsPackage                  1
#define MTBL_firstTransceiverPackage            2
#define MTBL_secondTransceiverPackage           3
#define MTBL_firstRadioCarrierPackage           4
#define MTBL_secondRadioCarrierPackage          5
#define MTBL_btsTimerPackage                    6
#define MTBL_t200                               7
#define MTBL_t31xx                              8
#define MTBL_adjacentCellPackage                9
#define MTBL_frequencyHoppingPackage            10
#define MTBL_handoverControlPackage             11
#define MTBL_hoAveragingAdjCellParam            12
#define MTBL_hoAveragingDistParam               13
#define MTBL_hoAveragingLevParam                14
#define MTBL_hoAveragingQualParam               15
#define MTBL_hoThresholdDistParam               16
#define MTBL_hoThresholdInterfaceParam          17
#define MTBL_hoThresholdLevParam                18
#define MTBL_hoThresholdQualParam               19
#define MTBL_interferenceAveragingParam         20
#define MTBL_powerControlPackage                21
#define MTBL_pcAveragingLev                     22
#define MTBL_pcAveragingQual                    23
#define MTBL_pcLowerThresholdLevParam           24
#define MTBL_pcLowerThresholdQualParam          25
#define MTBL_pcUpperThresholdLevParam           26
#define MTBL_pcUpperThresholdQualParam          27
#define MTBL_cellMeasurmentPackage              28
#define MTBL_callControlMeasurmentPackage       29
#define MTBL_callConfigData                     30
#define MTBL_mobilityConfigData                 31
#define MTBL_resourceConfigData                 32
#define MTBL_oamConfigData                      33
#define MTBL_h323Data                           34
#define MTBL_dualTRXSCard                       35
#define MTBL_arfcnRssiFunction                  36
#define MTBL_clockCard                          37
#define MTBL_gpsCard                            38
#define MTBL_powerCard                          39
#define MTBL_externalPowerSupply                40
#define MTBL_cdcBoard                           41
#define MTBL_automaticParameterAllocation       42
#define MTBL_gp10MgmtInfoMib                    43
#define MTBL_maintenanceConfigData              44
#define MTBL_gp10ErrorInfoMib                   45
#define MTBL_viperAccessMib                     46
#define MTBL_trapFwdData                        47

/*  New Defines for supporting  GS  */
#define MTBL_btsGsBasicPackage                  48
#define MTBL_btsGsOptionPackage                 49
#define MTBL_rss                                50
#define MTBL_rlcMac                             51
#define MTBL_bssgp                              52
#define MTBL_btsGsMeasPackage                   53

#define	MTBL_gp10CdrMib							54	
#define	MTBL_handoverTimerMib					55
#define MTBL_gp10ServiceStatusMib				56

#define MTBL_last                               56

#ifndef LEAF_bts_ncc    /*  Grab the leaf definitions */
#include    "mibleaf.h"
#endif

/* The mibcomp -leaf option does not create a complete list of the leaf
 * nodes. In particular, it does not generate them for all structure and
 * table entries. The following is is a list of the missing ones. The
 * LEAF_... definitions contain the final digit of the object OID string.
 */

#define LEAF_btsBasicPackage                            0
#define LEAF_cellAllocationTable                        4
#define LEAF_cellAllocationEntry                        0
#define LEAF_plmnPermittedTable                         12
#define LEAF_plmnPermittedEntry                         0
#define LEAF_btsOptionsPackage                          0
#define LEAF_cellBarred                                 3
#define LEAF_notAllowedAccessClassTable                 7
#define LEAF_notAllowedAccessClassEntry                 0
#define LEAF_firstTransceiverPackage                    0
#define LEAF_channel_0_Table                            6
#define LEAF_channel_0_Entry                            0
#define LEAF_secondTransceiverPackage                   0
#define LEAF_channel_1_Table                            6
#define LEAF_channel_1_Entry                            0
#define LEAF_firstRadioCarrierPackage                   0
#define LEAF_carrierFrequency_0_Table                   1
#define LEAF_carrierFrequency_0_Entry                   0
#define LEAF_secondRadioCarrierPackage                  0
#define LEAF_carrierFrequency_1_Table                   1
#define LEAF_carrierFrequency_1_Entry                   0
#define LEAF_btsTimerPackage                            0
#define LEAF_t200                                       1
#define LEAF_t31xx                                      2
#define LEAF_adjacentCellPackage                        0
#define LEAF_adjCell_handoverTable                      1
#define LEAF_adjCell_handoverEntry                      0
#define LEAF_adjCell_reselectionTable                   2
#define LEAF_adjCell_reselectionEntry                   0
#define LEAF_frequencyHoppingPackage                    0
#define LEAF_mobileAllocationTable                      3
#define LEAF_mobileAllocationEntry                      0
#define LEAF_handoverControlPackage                     0
#define LEAF_hoAveragingAdjCellParam                    3
#define LEAF_hoAveragingDistParam                       4
#define LEAF_hoAveragingLevParam                        5
#define LEAF_hoAveragingQualParam                       6
#define LEAF_hoThresholdDistParam                       8
#define LEAF_hoThresholdInterfaceParam                  9
#define LEAF_hoThresholdLevParam                        10
#define LEAF_hoThresholdQualParam                       11
#define LEAF_interferenceAveragingParam                 12
#define LEAF_powerControlPackage                        0
#define LEAF_pcAveragingLev                             1
#define LEAF_pcAveragingQual                            2
#define LEAF_pcLowerThresholdLevParam                   3
#define LEAF_pcLowerThresholdQualParam                  4
#define LEAF_pcUpperThresholdLevParam                   5
#define LEAF_pcUpperThresholdQualParam                  6
#define LEAF_cellMeasurmentPackage                      0
#define LEAF_attImmediateAssingProcsPerCauseTable       6
#define LEAF_attImmediateAssingProcsPerCauseEntry       0
#define LEAF_succImmediateAssingProcsPerCauseTable      7
#define LEAF_succImmediateAssingProcsPerCauseEntry      0
#define LEAF_callControlMeasurmentPackage               0
#define LEAF_externalHDOsPerCauseTable                  29
#define LEAF_externalHDOsPerCauseEntry                  0
#define LEAF_callConfigData                             0
#define LEAF_mobilityConfigData                         0
#define LEAF_resourceConfigData                         0
#define LEAF_oamConfigData                              0
#define LEAF_h323Data                                   0
                                                        
#define LEAF_dualTRXSCard                               0
#define LEAF_arfcnRssiFunction                          0
#define LEAF_arfcnRssiClockControlTable                 3
#define LEAF_arfcnRssiClockControlEntry                 0
#define LEAF_clockCard                                  0
#define LEAF_gpsCard                                    0
#define LEAF_powerCard                                  0
#define LEAF_externalPowerSupply                        0
#define LEAF_cdcBoard                                   0
#define LEAF_automaticParameterAllocation               0
#define LEAF_apaInvalidFrequencyTable                   8
#define LEAF_apaInvalidFrequencyEntry                   0
#define LEAF_apaClockSourceFrequencyTable               9
#define LEAF_apaClockSourceFrequencyEntry               0
#define LEAF_gp10MgmtInfoMib                            0
                                                        
#define LEAF_maintenanceConfigData                      0
#define LEAF_gp10ErrorInfoMib                           0

#define LEAF_viperAccessMib                             0
#define LEAF_readCommunity			                       1
#define LEAF_writeCommunity			                    2

										                  
#define	LEAF_gp10CdrMib									0	
#define	LEAF_handoverTimerMib							0
#define LEAF_gp10ServiceStatusMib						0

/*  New Defines for supporting  GS  */                  
#define LEAF_btsGsBasicPackage                          0
#define LEAF_btsGsOptionPackage                         0
#define LEAF_rss                                        0
#define LEAF_ncFrequencyListTable                       16
#define LEAF_ncFrequencyListEntry                       0
#define LEAF_rlcMac                                     0
#define LEAF_bssgp                                      0
#define LEAF_btsGsMeasPackage                           0
                                                        
                                                        
#define SIZE_DisplayString                              255
                                                        
#define VALUE_Default                                   0
#define VALUE_Average                                   1
#define VALUE_Deviation                                 2
#define VALUE_Maximum                                   3
#define VALUE_Minimum                                   4
#define VALUE_Reset                                     5

#define MIB_btsBasicPackage             MIB_STRUCT  (MTBL_btsBasicPackage,  LEAF_btsBasicPackage,   sizeof(BtsBasicPackage))
#define MIB_bts_ncc                     MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_ncc)
#define MIB_bts_cid                     MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_cid)
#define MIB_btsID                       MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_btsID)
#define MIB_cellAllocationTable         MIB_TABLE   (MTBL_btsBasicPackage,  LEAF_cellAllocationTable)
#define MIBT_cellAllocationEntry        MIB_TENTRY  (MTBL_btsBasicPackage,  LEAF_cellAllocationTable,   sizeof(CellAllocationEntry))
#define MIBT_cellAllocationIndex        MIB_TITEM   (MTBL_btsBasicPackage,  LEAF_cellAllocationTable,   LEAF_cellAllocationIndex)
#define MIBT_cellAllocation             MIB_TITEM   (MTBL_btsBasicPackage,  LEAF_cellAllocationTable,   LEAF_cellAllocation)
#define MIB_gsmdcsIndicator             MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_gsmdcsIndicator)
#define MIB_bts_mcc                     MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_mcc)
#define MIB_bts_mnc                     MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_mnc)
#define MIB_bts_lac                     MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_lac)
#define MIB_bts_ci                      MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_ci)
#define MIB_cellReselectHysteresis      MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_cellReselectHysteresis)
#define MIB_ny1                         MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_ny1)
#define MIB_plmnPermittedTable          MIB_TABLE   (MTBL_btsBasicPackage,  LEAF_plmnPermittedTable)
#define MIBT_plmnPermittedEntry         MIB_TENTRY  (MTBL_btsBasicPackage,  LEAF_plmnPermittedTable,    sizeof(PlmnPermittedEntry))
#define MIBT_plmnPermittedIndex         MIB_TITEM   (MTBL_btsBasicPackage,  LEAF_plmnPermittedTable,    LEAF_plmnPermittedIndex)
#define MIBT_plmnPermitted              MIB_TITEM   (MTBL_btsBasicPackage,  LEAF_plmnPermittedTable,    LEAF_plmnPermitted)
#define MIB_radioLinkTimeout            MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_radioLinkTimeout)
#define MIB_relatedTranscoder           MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_relatedTranscoder)
#define MIB_rxLevAccessMin              MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_rxLevAccessMin)
#define MIB_bts_administrativeState     MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_administrativeState)
#define MIB_bts_alarmStatus             MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_alarmStatus)
#define MIB_bts_operationalState        MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_bts_operationalState)
#define MIB_maxNumberRetransmissions    MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_maxNumberRetransmissions)
#define MIB_mSTxPwrMaxCCH               MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_mSTxPwrMaxCCH)
#define MIB_numberOfSlotsSpreadTrans    MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_numberOfSlotsSpreadTrans)
#define MIB_noOfBlocksForAccessGrant    MIB_ITEM    (MTBL_btsBasicPackage,  LEAF_noOfBlocksForAccessGrant)
#define MIB_noOfMultiframesBetweenPaging MIB_ITEM   (MTBL_btsBasicPackage,  LEAF_noOfMultiframesBetweenPaging)

#define MIB_btsOptionsPackage           MIB_STRUCT  (MTBL_btsOptionsPackage,    LEAF_btsOptionsPackage, sizeof(BtsOptionsPackage))
#define MIB_allowIMSIAttachDetach       MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_allowIMSIAttachDetach)
#define MIB_callReestablishmentAllowed  MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_callReestablishmentAllowed)
#define MIB_cellBarred                  MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_cellBarred)
#define MIB_dtxDownlink                 MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_dtxDownlink)
#define MIB_dtxUplink                   MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_dtxUplink)
#define MIB_emergencyCallRestricted     MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_emergencyCallRestricted)
#define MIB_notAllowedAccessClassTable  MIB_TABLE   (MTBL_btsOptionsPackage,    LEAF_notAllowedAccessClassTable)
#define MIBT_notAllowedAccessClassEntry MIB_TENTRY  (MTBL_btsOptionsPackage,    LEAF_notAllowedAccessClassTable,    sizeof(NotAllowedAccessClassEntry))
#define MIBT_notAllowedAccessClassIndex MIB_TITEM   (MTBL_btsOptionsPackage,    LEAF_notAllowedAccessClassTable,    LEAF_notAllowedAccessClassIndex)
#define MIBT_notAllowedAccessClass      MIB_TITEM   (MTBL_btsOptionsPackage,    LEAF_notAllowedAccessClassTable,    LEAF_notAllowedAccessClass)
#define MIB_timerPeriodicUpdateMS       MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_timerPeriodicUpdateMS)
#define MIB_maxQueueLength              MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_maxQueueLength)
#define MIB_msPriorityUsedInQueueing    MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_msPriorityUsedInQueueing)
#define MIB_timeLimitCall               MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_timeLimitCall)
#define MIB_timeLimitHandover           MIB_ITEM    (MTBL_btsOptionsPackage,    LEAF_timeLimitHandover)

#define MIB_firstTransceiverPackage     MIB_STRUCT  (MTBL_firstTransceiverPackage,  LEAF_firstTransceiverPackage,   sizeof(FirstTransceiverPackage))
#define MIB_basebandTransceiverID_0     MIB_ITEM    (MTBL_firstTransceiverPackage,  LEAF_basebandTransceiverID_0)
#define MIB_relatedRadioCarrier_0       MIB_ITEM    (MTBL_firstTransceiverPackage,  LEAF_relatedRadioCarrier_0)
#define MIB_basebandAdministrativeState_0   MIB_ITEM    (MTBL_firstTransceiverPackage,  LEAF_basebandAdministrativeState_0)
#define MIB_basebandAlarmStatus_0       MIB_ITEM    (MTBL_firstTransceiverPackage,  LEAF_basebandAlarmStatus_0)
#define MIB_basebandOperationalState_0  MIB_ITEM    (MTBL_firstTransceiverPackage,  LEAF_basebandOperationalState_0)
#define MIB_channel_0_Table             MIB_TABLE   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table)
#define MIBT_channel_0_Entry            MIB_TENTRY  (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   sizeof(ChannelEntry))
#define MIBT_channelIndex_0             MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelIndex_0)
#define MIBT_channelCombination_0       MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelCombination_0)
#define MIBT_channelID_0                MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelID_0)
#define MIBT_channelFrequencyUsage_0    MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelFrequencyUsage_0)
#define MIBT_channelTsc_0               MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelTsc_0)
#define MIBT_channelAdministrativeState_0   MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelAdministrativeState_0)
#define MIBT_channelOperationalState_0  MIB_TITEM   (MTBL_firstTransceiverPackage,  LEAF_channel_0_Table,   LEAF_channelOperationalState_0)
#define MIB_secondTransceiverPackage    MIB_STRUCT  (MTBL_secondTransceiverPackage, LEAF_secondTransceiverPackage,  sizeof(SecondTransceiverPackage))
#define MIB_basebandTransceiverID_1     MIB_ITEM    (MTBL_secondTransceiverPackage, LEAF_basebandTransceiverID_1)
#define MIB_relatedRadioCarrier_1       MIB_ITEM    (MTBL_secondTransceiverPackage, LEAF_relatedRadioCarrier_1)
#define MIB_basebandAdministrativeState_1   MIB_ITEM    (MTBL_secondTransceiverPackage, LEAF_basebandAdministrativeState_1)
#define MIB_basebandAlarmStatus_1       MIB_ITEM    (MTBL_secondTransceiverPackage, LEAF_basebandAlarmStatus_1)
#define MIB_basebandOperationalState_1  MIB_ITEM    (MTBL_secondTransceiverPackage, LEAF_basebandOperationalState_1)
#define MIB_channel_1_Table             MIB_TABLE   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table)
#define MIBT_channel_1_Entry            MIB_TENTRY  (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   sizeof(ChannelEntry))
#define MIBT_channelIndex_1             MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelIndex_1)
#define MIBT_channelID_1                MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelID_1)
#define MIBT_channelCombination_1       MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelCombination_1)
#define MIBT_channelFrequencyUsage_1    MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelFrequencyUsage_1)
#define MIBT_channelTsc_1               MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelTsc_1)
#define MIBT_channelAdministrativeState_1   MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelAdministrativeState_1)
#define MIBT_channelOperationalState_1  MIB_TITEM   (MTBL_secondTransceiverPackage, LEAF_channel_1_Table,   LEAF_channelOperationalState_1)

#define MIB_firstRadioCarrierPackage    MIB_STRUCT  (MTBL_firstRadioCarrierPackage, LEAF_firstRadioCarrierPackage,  sizeof(RadioCarrierPackage))

#define MIB_carrierFrequency_0_Table    MIB_TABLE   (MTBL_firstRadioCarrierPackage, LEAF_carrierFrequency_0_Table)
#define MIBT_carrierFrequency_0_Entry   MIB_TENTRY  (MTBL_firstRadioCarrierPackage, LEAF_carrierFrequency_0_Table,  sizeof(CarrierFrequencyEntry))
#define MIBT_carrierFrequencyIndex_0    MIB_TITEM   (MTBL_firstRadioCarrierPackage, LEAF_carrierFrequency_0_Table,  LEAF_carrierFrequencyIndex_0)
#define MIBT_carrierFrequency_0         MIB_TITEM   (MTBL_firstRadioCarrierPackage, LEAF_carrierFrequency_0_Table,  LEAF_carrierFrequency_0)
#define MIB_powerClass_0                MIB_ITEM    (MTBL_firstRadioCarrierPackage, LEAF_powerClass_0)
#define MIB_radioCarrierID_0            MIB_ITEM    (MTBL_firstRadioCarrierPackage, LEAF_radioCarrierID_0)
#define MIB_txPwrMaxReduction_0         MIB_ITEM    (MTBL_firstRadioCarrierPackage, LEAF_txPwrMaxReduction_0)
#define MIB_carrier_administrativeState_0   MIB_ITEM    (MTBL_firstRadioCarrierPackage, LEAF_carrier_administrativeState_0)
#define MIB_carrier_alarmStatus_0       MIB_ITEM    (MTBL_firstRadioCarrierPackage, LEAF_carrier_alarmStatus_0)
#define MIB_carrier_operationalState_0  MIB_ITEM    (MTBL_firstRadioCarrierPackage, LEAF_carrier_operationalState_0)

#define MIB_secondRadioCarrierPackage   MIB_STRUCT  (MTBL_secondRadioCarrierPackage,    LEAF_secondRadioCarrierPackage, sizeof(RadioCarrierPackage))
#define MIB_carrierFrequency_1_Table    MIB_TABLE   (MTBL_secondRadioCarrierPackage,    LEAF_carrierFrequency_1_Table)
#define MIBT_carrierFrequency_1_Entry   MIB_TENTRY  (MTBL_secondRadioCarrierPackage,    LEAF_carrierFrequency_1_Table,  sizeof(CarrierFrequencyEntry))
#define MIBT_carrierFrequencyIndex_1    MIB_TITEM   (MTBL_secondRadioCarrierPackage,    LEAF_carrierFrequency_1_Table,  LEAF_carrierFrequencyIndex_1)
#define MIBT_carrierFrequency_1         MIB_TITEM   (MTBL_secondRadioCarrierPackage,    LEAF_carrierFrequency_1_Table,  LEAF_carrierFrequency_1)
#define MIB_powerClass_1                MIB_ITEM    (MTBL_secondRadioCarrierPackage,    LEAF_powerClass_1)
#define MIB_radioCarrierID_1            MIB_ITEM    (MTBL_secondRadioCarrierPackage,    LEAF_radioCarrierID_1)
#define MIB_txPwrMaxReduction_1         MIB_ITEM    (MTBL_secondRadioCarrierPackage,    LEAF_txPwrMaxReduction_1)
#define MIB_carrier_administrativeState_1   MIB_ITEM    (MTBL_secondRadioCarrierPackage,    LEAF_carrier_administrativeState_1)
#define MIB_carrier_alarmStatus_1       MIB_ITEM    (MTBL_secondRadioCarrierPackage,    LEAF_carrier_alarmStatus_1)
#define MIB_carrier_operationalState_1  MIB_ITEM    (MTBL_secondRadioCarrierPackage,    LEAF_carrier_operationalState_1)

#define MIB_btsTimerPackage             MIB_STRUCT  (MTBL_btsTimerPackage,  LEAF_btsTimerPackage,   sizeof(BtsTimerPackage))
#define MIB_t200                        MIB_STRUCT  (MTBL_btsTimerPackage,  LEAF_t200,  sizeof(T200))

#define MIB_t200Package                 MIB_STRUCT  (MTBL_t200, 0,  sizeof(T200))   /*  added   on  Aug 9   */
#define MIB_sdcchSAPI0                  MIB_ITEM    (MTBL_t200, LEAF_sdcchSAPI0)
#define MIB_facchTCHF                   MIB_ITEM    (MTBL_t200, LEAF_facchTCHF)
#define MIB_facchTCHH                   MIB_ITEM    (MTBL_t200, LEAF_facchTCHH)
#define MIB_sacchTCHSAPI0               MIB_ITEM    (MTBL_t200, LEAF_sacchTCHSAPI0)
#define MIB_sacchSDCCH                  MIB_ITEM    (MTBL_t200, LEAF_sacchSDCCH)
#define MIB_sdcchSAPI3                  MIB_ITEM    (MTBL_t200, LEAF_sdcchSAPI3)
#define MIB_sdcchTCHSAPI3               MIB_ITEM    (MTBL_t200, LEAF_sdcchTCHSAPI3)
#define MIB_t31xx                       MIB_STRUCT  (MTBL_btsTimerPackage,  LEAF_t31xx, sizeof(T31xx))

/*  Modified    to  solve   nested  table   reference   problem.    When    an  application gets    the address of  the
    *   structure,  it  should  get the actual  address of  the sturcture   rather  than    address to  the pointer
    *   to  the structure.  Similar change  is  needed  for all the nested  struct. This    applies only    for the
    *   nested  struct, outermost   struct  is  fine.
    */

#define MIB_t31xxPackage    MIB_STRUCT  (MTBL_t31xx,    0,  sizeof(T31xx))
#define MIB_t3101   MIB_ITEM    (MTBL_t31xx,    LEAF_t3101)
#define MIB_t3103   MIB_ITEM    (MTBL_t31xx,    LEAF_t3103)
#define MIB_t3105   MIB_ITEM    (MTBL_t31xx,    LEAF_t3105)
#define MIB_t3107   MIB_ITEM    (MTBL_t31xx,    LEAF_t3107)
#define MIB_t3109   MIB_ITEM    (MTBL_t31xx,    LEAF_t3109)
#define MIB_t3111   MIB_ITEM    (MTBL_t31xx,    LEAF_t3111)
#define MIB_t3113   MIB_ITEM    (MTBL_t31xx,    LEAF_t3113)
#define MIB_adjacentCellPackage MIB_STRUCT  (MTBL_adjacentCellPackage,  LEAF_adjacentCellPackage,   sizeof(AdjacentCellPackage))
#define MIB_adjCell_handoverTable   MIB_TABLE    (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable)
#define MIBT_adjCell_handoverEntry  MIB_TENTRY  (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, sizeof(AdjCell_HandoverEntry))
#define MIBT_adjCell_handoverIndex  MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_handoverIndex)
#define MIBT_adjCell_handoverCellID MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_handoverCellID)
#define MIBT_adjCell_mcc    MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_mcc)
#define MIBT_adjCell_mnc    MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_mnc)
#define MIBT_adjCell_lac    MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_lac)
#define MIBT_adjCell_ci MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_ci)
#define MIBT_adjCell_bCCHFrequency  MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_bCCHFrequency)
#define MIBT_adjCell_ncc    MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_ncc)
#define MIBT_adjCell_cid    MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_cid)
#define MIBT_adjCell_synchronized   MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_synchronized)
#define MIBT_adjCell_hoPriorityLevel    MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_hoPriorityLevel)
#define MIBT_adjCell_hoMargin   MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_hoMargin)
#define MIBT_adjCell_msTxPwrMaxCell MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_msTxPwrMaxCell)
#define MIBT_adjCell_rxLevMinCell   MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_rxLevMinCell)
#define MIBT_adjCell_isExternal   MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_handoverTable, LEAF_adjCell_isExternal)
#define MIB_adjCell_reselectionTable    MIB_TABLE   (MTBL_adjacentCellPackage,  LEAF_adjCell_reselectionTable)
#define MIBT_adjCell_reselectionEntry   MIB_TENTRY  (MTBL_adjacentCellPackage,  LEAF_adjCell_reselectionTable,  sizeof(AdjCell_ReselectionEntry))
#define MIBT_adjCell_reselectionIndex   MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_reselectionTable,  LEAF_adjCell_reselectionIndex)
#define MIBT_adjCell_reselectionCellID  MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_reselectionTable,  LEAF_adjCell_reselectionCellID)
#define MIBT_adjCell_reselectionBCCHFrequency   MIB_TITEM   (MTBL_adjacentCellPackage,  LEAF_adjCell_reselectionTable,  LEAF_adjCell_reselectionBCCHFrequency)
#define MIB_frequencyHoppingPackage MIB_STRUCT  (MTBL_frequencyHoppingPackage,  LEAF_frequencyHoppingPackage,   sizeof(FrequencyHoppingPackage))
#define MIB_frequencyHoppingSystemID    MIB_ITEM    (MTBL_frequencyHoppingPackage,  LEAF_frequencyHoppingSystemID)
#define MIB_hoppingSequenceNumber   MIB_ITEM    (MTBL_frequencyHoppingPackage,  LEAF_hoppingSequenceNumber)
#define MIB_mobileAllocationTable   MIB_TABLE   (MTBL_frequencyHoppingPackage,  LEAF_mobileAllocationTable)
#define MIBT_mobileAllocationEntry  MIB_TENTRY  (MTBL_frequencyHoppingPackage,  LEAF_mobileAllocationTable, sizeof(MobileAllocationEntry))
#define MIBT_mobileAllocationIndex  MIB_TITEM   (MTBL_frequencyHoppingPackage,  LEAF_mobileAllocationTable, LEAF_mobileAllocationIndex)
#define MIBT_mobileAllocation   MIB_TITEM   (MTBL_frequencyHoppingPackage,  LEAF_mobileAllocationTable, LEAF_mobileAllocation)
#define MIB_handoverControlPackage  MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_handoverControlPackage,    sizeof(HandoverControlPackage))
#define MIB_handoverControlID   MIB_ITEM    (MTBL_handoverControlPackage,   LEAF_handoverControlID)
#define MIB_enableOptHandoverProcessing MIB_ITEM    (MTBL_handoverControlPackage,   LEAF_enableOptHandoverProcessing)
#define MIB_hoAveragingAdjCellParam MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoAveragingAdjCellParam,   sizeof(HoAveragingAdjCellParam))
#define MIB_hoAveragingAdjCellPackage   MIB_STRUCT  (MTBL_hoAveragingAdjCellParam,  0,  sizeof(HoAveragingAdjCellParam))    /*  added   on  Aug 9   */

#define MIB_hoAveragingAdjCellParamHreqave  MIB_ITEM    (MTBL_hoAveragingAdjCellParam,  LEAF_hoAveragingAdjCellParamHreqave)
#define MIB_hoAveragingAdjCellParamHreqt    MIB_ITEM    (MTBL_hoAveragingAdjCellParam,  LEAF_hoAveragingAdjCellParamHreqt)
#define MIB_hoAveragingAdjCellParamWeighting    MIB_ITEM    (MTBL_hoAveragingAdjCellParam,  LEAF_hoAveragingAdjCellParamWeighting)
#define MIB_hoAveragingDistParam    MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoAveragingDistParam,  sizeof(HoAveragingDistParam))
#define MIB_hoAveragingDistPackage  MIB_STRUCT  (MTBL_hoAveragingDistParam, 0,  sizeof(HoAveragingDistParam))   /*  added   on  Aug 9   */

#define MIB_hoAveragingDistParamHreqave MIB_ITEM    (MTBL_hoAveragingDistParam, LEAF_hoAveragingDistParamHreqave)
#define MIB_hoAveragingDistParamHreqt   MIB_ITEM    (MTBL_hoAveragingDistParam, LEAF_hoAveragingDistParamHreqt)
#define MIB_hoAveragingLevParam MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoAveragingLevParam,   sizeof(HoAveragingLevParam))
#define MIB_hoAveragingLevPackage   MIB_STRUCT  (MTBL_hoAveragingLevParam,  0,  sizeof(HoAveragingLevParam))    /*  added   on  Aug 9   */

#define MIB_hoAveragingLevParamHreqave  MIB_ITEM    (MTBL_hoAveragingLevParam,  LEAF_hoAveragingLevParamHreqave)
#define MIB_hoAveragingLevParamHreqt    MIB_ITEM    (MTBL_hoAveragingLevParam,  LEAF_hoAveragingLevParamHreqt)
#define MIB_hoAveragingLevParamWeighting    MIB_ITEM    (MTBL_hoAveragingLevParam,  LEAF_hoAveragingLevParamWeighting)
#define MIB_hoAveragingQualParam    MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoAveragingQualParam,  sizeof(HoAveragingQualParam))
#define MIB_hoAveragingQualPackage  MIB_STRUCT  (MTBL_hoAveragingQualParam, 0,  sizeof(HoAveragingQualParam))   /*  added   on  Aug 9   */

#define MIB_hoAveragingQualParamHreqave MIB_ITEM    (MTBL_hoAveragingQualParam, LEAF_hoAveragingQualParamHreqave)
#define MIB_hoAveragingQualParamHreqt   MIB_ITEM    (MTBL_hoAveragingQualParam, LEAF_hoAveragingQualParamHreqt)
#define MIB_hoAveragingQualParamWeighting   MIB_ITEM    (MTBL_hoAveragingQualParam, LEAF_hoAveragingQualParamWeighting)
#define MIB_hoMarginDef MIB_ITEM    (MTBL_handoverControlPackage,   LEAF_hoMarginDef)
#define MIB_hoThresholdDistParam    MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoThresholdDistParam,  sizeof(HoThresholdDistParam))
#define MIB_hoThresholdDistPackage  MIB_STRUCT  (MTBL_hoThresholdDistParam, 0,  sizeof(HoThresholdDistParam))   /*  added   on  Aug 9   */

#define MIB_hoThresholdDistParamTimeadv MIB_ITEM    (MTBL_hoThresholdDistParam, LEAF_hoThresholdDistParamTimeadv)
#define MIB_hoThresholdDistParamP8  MIB_ITEM    (MTBL_hoThresholdDistParam, LEAF_hoThresholdDistParamP8)
#define MIB_hoThresholdDistParamN8  MIB_ITEM    (MTBL_hoThresholdDistParam, LEAF_hoThresholdDistParamN8)
#define MIB_hoThresholdInterfaceParam   MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoThresholdInterfaceParam, sizeof(HoThresholdInterfaceParam))
#define MIB_hoThresholdInterfacePackage MIB_STRUCT  (MTBL_hoThresholdInterfaceParam,    0,  sizeof(HoThresholdInterfaceParam))  /*  added   on  Aug 9   */

#define MIB_hoThresholdInterfaceParamRxLevelUL  MIB_ITEM    (MTBL_hoThresholdInterfaceParam,    LEAF_hoThresholdInterfaceParamRxLevelUL)
#define MIB_hoThresholdInterfaceParamRxLevelDL  MIB_ITEM    (MTBL_hoThresholdInterfaceParam,    LEAF_hoThresholdInterfaceParamRxLevelDL)
#define MIB_hoThresholdInterfaceParamPx MIB_ITEM    (MTBL_hoThresholdInterfaceParam,    LEAF_hoThresholdInterfaceParamPx)
#define MIB_hoThresholdInterfaceParamNx MIB_ITEM    (MTBL_hoThresholdInterfaceParam,    LEAF_hoThresholdInterfaceParamNx)
#define MIB_hoThresholdLevParam MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoThresholdLevParam,   sizeof(HoThresholdLevParam))
#define MIB_hoThresholdLevPackage   MIB_STRUCT  (MTBL_hoThresholdLevParam,  0,  sizeof(HoThresholdLevParam))    /*  added   on  Aug 9   */

#define MIB_hoThresholdLevParamRxLevelUL    MIB_ITEM    (MTBL_hoThresholdLevParam,  LEAF_hoThresholdLevParamRxLevelUL)
#define MIB_hoThresholdLevParamRxLevelDL    MIB_ITEM    (MTBL_hoThresholdLevParam,  LEAF_hoThresholdLevParamRxLevelDL)
#define MIB_hoThresholdLevParamPx   MIB_ITEM    (MTBL_hoThresholdLevParam,  LEAF_hoThresholdLevParamPx)
#define MIB_hoThresholdLevParamNx   MIB_ITEM    (MTBL_hoThresholdLevParam,  LEAF_hoThresholdLevParamNx)
#define MIB_hoThresholdQualParam    MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_hoThresholdQualParam,  sizeof(HoThresholdQualParam))
#define MIB_hoThresholdQualPackage  MIB_STRUCT  (MTBL_hoThresholdQualParam, 0,  sizeof(HoThresholdQualParam))   /*  added   on  Aug 9   */

#define MIB_hoThresholdQualParamRxQualUL    MIB_ITEM    (MTBL_hoThresholdQualParam, LEAF_hoThresholdQualParamRxQualUL)
#define MIB_hoThresholdQualParamRxQualDL    MIB_ITEM    (MTBL_hoThresholdQualParam, LEAF_hoThresholdQualParamRxQualDL)
#define MIB_hoThresholdQualParamPx  MIB_ITEM    (MTBL_hoThresholdQualParam, LEAF_hoThresholdQualParamPx)
#define MIB_hoThresholdQualParamNx  MIB_ITEM    (MTBL_hoThresholdQualParam, LEAF_hoThresholdQualParamNx)
#define MIB_interferenceAveragingParam  MIB_STRUCT  (MTBL_handoverControlPackage,   LEAF_interferenceAveragingParam,    sizeof(InterferenceAveragingParam))
#define MIB_interferenceAveragingPackage    MIB_STRUCT  (MTBL_interferenceAveragingParam,   0,  sizeof(InterferenceAveragingParam)) /*  added   on  Aug 9   */

#define MIB_interferenceAveragingParamAveragingPeriod   MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamAveragingPeriod)
#define MIB_interferenceAveragingParamThresholdBoundary0    MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamThresholdBoundary0)
#define MIB_interferenceAveragingParamThresholdBoundary1    MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamThresholdBoundary1)
#define MIB_interferenceAveragingParamThresholdBoundary2    MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamThresholdBoundary2)
#define MIB_interferenceAveragingParamThresholdBoundary3    MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamThresholdBoundary3)
#define MIB_interferenceAveragingParamThresholdBoundary4    MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamThresholdBoundary4)
#define MIB_interferenceAveragingParamThresholdBoundary5    MIB_ITEM    (MTBL_interferenceAveragingParam,   LEAF_interferenceAveragingParamThresholdBoundary5)
#define MIB_msTxPwrMaxCellDef   MIB_ITEM    (MTBL_handoverControlPackage,   LEAF_msTxPwrMaxCellDef)
#define MIB_rxLevMinCellDef MIB_ITEM    (MTBL_handoverControlPackage,   LEAF_rxLevMinCellDef)
#define MIB_powerControlPackage MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_powerControlPackage,   sizeof(PowerControlPackage))
#define MIB_pcAveragingLev  MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_pcAveragingLev,    sizeof(PcAveragingLev))
#define MIB_pcAveragingLevPackage   MIB_STRUCT  (MTBL_pcAveragingLev,   0,  sizeof(PcAveragingLev)) /*  added   on  Aug 9   */

#define MIB_pcAveragingLevHreqave   MIB_ITEM    (MTBL_pcAveragingLev,   LEAF_pcAveragingLevHreqave)
#define MIB_pcAveragingLevHreqt MIB_ITEM    (MTBL_pcAveragingLev,   LEAF_pcAveragingLevHreqt)
#define MIB_pcAveragingLevWeighting MIB_ITEM    (MTBL_pcAveragingLev,   LEAF_pcAveragingLevWeighting)
#define MIB_pcAveragingQual MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_pcAveragingQual,   sizeof(PcAveragingQual))
#define MIB_pcAveragingQualPackage  MIB_STRUCT  (MTBL_pcAveragingQual,  0,  sizeof(PcAveragingQual))    /*  added   on  Aug 9   */

#define MIB_pcAveragingQualHreqave  MIB_ITEM    (MTBL_pcAveragingQual,  LEAF_pcAveragingQualHreqave)
#define MIB_pcAveragingQualHreqt    MIB_ITEM    (MTBL_pcAveragingQual,  LEAF_pcAveragingQualHreqt)
#define MIB_pcAveragingQualWeighting    MIB_ITEM    (MTBL_pcAveragingQual,  LEAF_pcAveragingQualWeighting)
#define MIB_pcLowerThresholdLevParam    MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_pcLowerThresholdLevParam,  sizeof(PcLowerThresholdLevParam))
#define MIB_pcLowerThresholdLevPackage  MIB_STRUCT  (MTBL_pcLowerThresholdLevParam, 0,  sizeof(PcLowerThresholdLevParam))   /*  added   on  Aug 9   */

#define MIB_pcLowerThresholdLevParamRxLevelUL   MIB_ITEM    (MTBL_pcLowerThresholdLevParam, LEAF_pcLowerThresholdLevParamRxLevelUL)
#define MIB_pcLowerThresholdLevParamRxLevelDL   MIB_ITEM    (MTBL_pcLowerThresholdLevParam, LEAF_pcLowerThresholdLevParamRxLevelDL)
#define MIB_pcLowerThresholdLevParamPx  MIB_ITEM    (MTBL_pcLowerThresholdLevParam, LEAF_pcLowerThresholdLevParamPx)
#define MIB_pcLowerThresholdLevParamNx  MIB_ITEM    (MTBL_pcLowerThresholdLevParam, LEAF_pcLowerThresholdLevParamNx)
#define MIB_pcLowerThresholdQualParam   MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_pcLowerThresholdQualParam, sizeof(PcLowerThresholdQualParam))
#define MIB_pcLowerThresholdQualPackage MIB_STRUCT  (MTBL_pcLowerThresholdQualParam,    0,  sizeof(PcLowerThresholdQualParam))  /*  added   on  Aug 9   */

#define MIB_pcLowerThresholdQualParamRxQualUL   MIB_ITEM    (MTBL_pcLowerThresholdQualParam,    LEAF_pcLowerThresholdQualParamRxQualUL)
#define MIB_pcLowerThresholdQualParamRxQualDL   MIB_ITEM    (MTBL_pcLowerThresholdQualParam,    LEAF_pcLowerThresholdQualParamRxQualDL)
#define MIB_pcLowerThresholdQualParamPx MIB_ITEM    (MTBL_pcLowerThresholdQualParam,    LEAF_pcLowerThresholdQualParamPx)
#define MIB_pcLowerThresholdQualParamNx MIB_ITEM    (MTBL_pcLowerThresholdQualParam,    LEAF_pcLowerThresholdQualParamNx)
#define MIB_pcUpperThresholdLevParam    MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_pcUpperThresholdLevParam,  sizeof(PcUpperThresholdLevParam))
#define MIB_pcUpperThresholdLevPackage  MIB_STRUCT  (MTBL_pcUpperThresholdLevParam, 0,  sizeof(PcUpperThresholdLevParam))   /*  added   on  Aug 9   */

#define MIB_pcUpperThresholdLevParamRxLevelUL   MIB_ITEM    (MTBL_pcUpperThresholdLevParam, LEAF_pcUpperThresholdLevParamRxLevelUL)
#define MIB_pcUpperThresholdLevParamRxLevelDL   MIB_ITEM    (MTBL_pcUpperThresholdLevParam, LEAF_pcUpperThresholdLevParamRxLevelDL)
#define MIB_pcUpperThresholdLevParamPx  MIB_ITEM    (MTBL_pcUpperThresholdLevParam, LEAF_pcUpperThresholdLevParamPx)
#define MIB_pcUpperThresholdLevParamNx  MIB_ITEM    (MTBL_pcUpperThresholdLevParam, LEAF_pcUpperThresholdLevParamNx)
#define MIB_pcUpperThresholdQualParam   MIB_STRUCT  (MTBL_powerControlPackage,  LEAF_pcUpperThresholdQualParam, sizeof(PcUpperThresholdQualParam))
#define MIB_pcUpperThresholdQualPackage MIB_STRUCT  (MTBL_pcUpperThresholdQualParam,    0,  sizeof(PcUpperThresholdQualParam))  /*  added   on  Aug 9   */

#define MIB_pcUpperThresholdQualParamRxQualUL   MIB_ITEM    (MTBL_pcUpperThresholdQualParam,    LEAF_pcUpperThresholdQualParamRxQualUL)
#define MIB_pcUpperThresholdQualParamRxQualDL   MIB_ITEM    (MTBL_pcUpperThresholdQualParam,    LEAF_pcUpperThresholdQualParamRxQualDL)
#define MIB_pcUpperThresholdQualParamPx MIB_ITEM    (MTBL_pcUpperThresholdQualParam,    LEAF_pcUpperThresholdQualParamPx)
#define MIB_pcUpperThresholdQualParamNx MIB_ITEM    (MTBL_pcUpperThresholdQualParam,    LEAF_pcUpperThresholdQualParamNx)
#define MIB_powerControlInterval    MIB_ITEM    (MTBL_powerControlPackage,  LEAF_powerControlInterval)
#define MIB_powerIncrStepSize   MIB_ITEM    (MTBL_powerControlPackage,  LEAF_powerIncrStepSize)
#define MIB_powerRedStepSize    MIB_ITEM    (MTBL_powerControlPackage,  LEAF_powerRedStepSize)
#define MIB_cellMeasurmentPackage   MIB_STRUCT  (MTBL_cellMeasurmentPackage,    LEAF_cellMeasurmentPackage, sizeof(CellMeasurmentPackage))
#define MIB_meanPCHAGCHQueueLength  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanPCHAGCHQueueLength)
#define MIB_attTransOfPagingMessagesThePCH  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attTransOfPagingMessagesThePCH)
#define MIB_unsuccTransOfPagingMessagesThePCH   MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_unsuccTransOfPagingMessagesThePCH)
#define MIB_attImmediateAssingProcs MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attImmediateAssingProcs)
#define MIB_succImmediateAssingProcs    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_succImmediateAssingProcs)
#define MIB_attImmediateAssingProcsPerCauseTable    MIB_TABLE   (MTBL_cellMeasurmentPackage,    LEAF_attImmediateAssingProcsPerCauseTable)
#define MIBT_attImmediateAssingProcsPerCauseEntry   MIB_TENTRY  (MTBL_cellMeasurmentPackage,    LEAF_attImmediateAssingProcsPerCauseTable,  sizeof(AttImmediateAssingProcsPerCauseEntry))
#define MIBT_attImmediateAssingProcsCause   MIB_TITEM   (MTBL_cellMeasurmentPackage,    LEAF_attImmediateAssingProcsPerCauseTable,  LEAF_attImmediateAssingProcsCause)
#define MIBT_attImmediateAssingProcsValue   MIB_TITEM   (MTBL_cellMeasurmentPackage,    LEAF_attImmediateAssingProcsPerCauseTable,  LEAF_attImmediateAssingProcsValue)
#define MIB_succImmediateAssingProcsPerCauseTable   MIB_TABLE   (MTBL_cellMeasurmentPackage,    LEAF_succImmediateAssingProcsPerCauseTable)
#define MIBT_succImmediateAssingProcsPerCauseEntry  MIB_TENTRY  (MTBL_cellMeasurmentPackage,    LEAF_succImmediateAssingProcsPerCauseTable, sizeof(SuccImmediateAssingProcsPerCauseEntry))
#define MIBT_succImmediateAssingProcsCause  MIB_TITEM   (MTBL_cellMeasurmentPackage,    LEAF_succImmediateAssingProcsPerCauseTable, LEAF_succImmediateAssingProcsCause)
#define MIBT_succImmediateAssingProcsValue  MIB_TITEM   (MTBL_cellMeasurmentPackage,    LEAF_succImmediateAssingProcsPerCauseTable, LEAF_succImmediateAssingProcsValue)
#define MIB_nbrOfPagesDiscardedFromPCHQueue MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_nbrOfPagesDiscardedFromPCHQueue)
#define MIB_meanDurationOfSuccPagingProcs   MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanDurationOfSuccPagingProcs)
#define MIB_nbrOfAvailableTCHs  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_nbrOfAvailableTCHs)
#define MIB_meanNbrOfBusyTCHs   MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanNbrOfBusyTCHs)
#define MIB_maxNbrOfBusyTCHs    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_maxNbrOfBusyTCHs)
#define MIB_meanNbrOfIdleTCHsPerInterferenceBand    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanNbrOfIdleTCHsPerInterferenceBand)
#define MIB_attTCHSeizures  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attTCHSeizures)
#define MIB_succTCHSeizures MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_succTCHSeizures)
#define MIB_attTCHSeizuresMeetingTCHBlockedState    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attTCHSeizuresMeetingTCHBlockedState)
#define MIB_allAvailableTCHAllocatedTime    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_allAvailableTCHAllocatedTime)
#define MIB_meanTCHBusyTime MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanTCHBusyTime)
#define MIB_meanTCHQueueLength  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanTCHQueueLength)
#define MIB_nbrOfLostRadioLinksTCH  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_nbrOfLostRadioLinksTCH)
#define MIB_nbrOfAvailableSDCCHs    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_nbrOfAvailableSDCCHs)
#define MIB_meanNbrOfBusySDCCHs MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanNbrOfBusySDCCHs)
#define MIB_maxNbrOfBusySDCCHs  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_maxNbrOfBusySDCCHs)
#define MIB_attSDCCHSeizuresMeetingSDCCHBlockedState    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attSDCCHSeizuresMeetingSDCCHBlockedState)
#define MIB_allAvailableSDCCHAllocatedTime  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_allAvailableSDCCHAllocatedTime)
#define MIB_meanSDCCHQueueLength    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_meanSDCCHQueueLength)
#define MIB_nbrOfLostRadioLinksSDCCH    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_nbrOfLostRadioLinksSDCCH)
#define MIB_relativeTimeDLPowerControlAtMax MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_relativeTimeDLPowerControlAtMax)
#define MIB_relativeTimeULPowerControlAtMax MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_relativeTimeULPowerControlAtMax)
#define MIB_succInternalHDOsIntraCell   MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_succInternalHDOsIntraCell)
#define MIB_unsuccInternalHDOsIntraCell MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_unsuccInternalHDOsIntraCell)
#if THESE_ARE_NOT_USED_IN_VIPERCELL
#define MIB_attIncomingInternalInterCellHDOs    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attIncomingInternalInterCellHDOs)
#define MIB_succIncomingInternalInterCellHDOs   MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_succIncomingInternalInterCellHDOs)
#define MIB_attOutgoingInternalInterCellHDOs    MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_attOutgoingInternalInterCellHDOs)
#define MIB_succOutgoingInternalInterCellHDOs   MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_succOutgoingInternalInterCellHDOs)
#define MIB_unsuccHDOsWithReconnection  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_unsuccHDOsWithReconnection)
#define MIB_unsuccHDOsWithLossOfConnection  MIB_ITEM    (MTBL_cellMeasurmentPackage,    LEAF_unsuccHDOsWithLossOfConnection)
#endif
#define MIB_callControlMeasurmentPackage    MIB_STRUCT  (MTBL_callControlMeasurmentPackage, LEAF_callControlMeasurmentPackage,  sizeof(CallControlMeasurmentPackage))
#define MIB_nbrOfClassMarkUpdates   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_nbrOfClassMarkUpdates)
#define MIB_attMobileOriginatingCalls   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attMobileOriginatingCalls)
#define MIB_succMobileOriginatingCalls  MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succMobileOriginatingCalls)
#define MIB_ansMobileOriginatingCalls   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_ansMobileOriginatingCalls)
#define MIB_attMobileTerminatingCalls   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attMobileTerminatingCalls)
#define MIB_succMobileTerminatingCalls  MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succMobileTerminatingCalls)
#define MIB_ansMobileTerminatingCalls   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_ansMobileTerminatingCalls)
#define MIB_attMobileEmergencyCalls MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attMobileEmergencyCalls)
#define MIB_succMobileEmergencyCalls    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succMobileEmergencyCalls)
#define MIB_ansMobileEmergencyCalls MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_ansMobileEmergencyCalls)
#define MIB_attCipheringModeControlProcs    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attCipheringModeControlProcs)
#define MIB_succCipheringModeControlProcs   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succCipheringModeControlProcs)
#define MIB_attInterrogationOfHLRsForRouting    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attInterrogationOfHLRsForRouting)
#define MIB_succInterrogationOfHLRsMSRNObtained MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succInterrogationOfHLRsMSRNObtained)
#define MIB_succInterrogationOfHLRsCallForwarding   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succInterrogationOfHLRsCallForwarding)
#define MIB_attOpForMobileOriginatingPointToPointSMs    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attOpForMobileOriginatingPointToPointSMs)
#define MIB_succOpForMobileOriginatingPointToPointSMs   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succOpForMobileOriginatingPointToPointSMs)
#define MIB_attOpForMobileTerminatingPointToPointSMs    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attOpForMobileTerminatingPointToPointSMs)
#define MIB_succOpForMobileTerminatingPointToPointSMs   MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succOpForMobileTerminatingPointToPointSMs)
#define MIB_meanTimeToCallSetupService  MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_meanTimeToCallSetupService)
#define MIB_meanTimeToLocationUpdateService MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_meanTimeToLocationUpdateService)
#define MIB_transSubIdentifiedWithIMSI  MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_transSubIdentifiedWithIMSI)
#define MIB_imsiDetachProcs MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_imsiDetachProcs)
#define MIB_imsiAttachProcs MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_imsiAttachProcs)
#define MIB_attIncomingInterMSCHDOs MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attIncomingInterMSCHDOs)
#define MIB_succIncomingInterMSCHDOs    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succIncomingInterMSCHDOs)
#define MIB_attOutgoingInterMSCHDOs MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_attOutgoingInterMSCHDOs)
#define MIB_succOutgoingInterMSCHDOs    MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_succOutgoingInterMSCHDOs)
#define MIB_externalHDOsPerCauseTable   MIB_TABLE   (MTBL_callControlMeasurmentPackage, LEAF_externalHDOsPerCauseTable)
#define MIBT_externalHDOsPerCauseEntry  MIB_TENTRY  (MTBL_callControlMeasurmentPackage, LEAF_externalHDOsPerCauseTable, sizeof(ExternalHDOsPerCauseEntry))
#define MIBT_externalHDOsPerCauseCode   MIB_TITEM   (MTBL_callControlMeasurmentPackage, LEAF_externalHDOsPerCauseTable, LEAF_externalHDOsPerCauseCode)
#define MIBT_externalHDOsPerCauseValue  MIB_TITEM   (MTBL_callControlMeasurmentPackage, LEAF_externalHDOsPerCauseTable, LEAF_externalHDOsPerCauseValue)
#define MIB_unsuccExternHDOsWithReconnectionPerMSC  MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_unsuccExternHDOsWithReconnectionPerMSC)
#define MIB_unsuccExternHDOsWithLossOfConnectionPerMSC  MIB_ITEM    (MTBL_callControlMeasurmentPackage, LEAF_unsuccExternHDOsWithLossOfConnectionPerMSC)
#define MIB_callConfigData  MIB_STRUCT  (MTBL_callConfigData,   LEAF_callConfigData,    sizeof(CallConfigData))
#define MIB_cc_max_L3_msgs  MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_max_L3_msgs)
#define MIB_cc_l3_msg_q_priority    MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_l3_msg_q_priority)
#define MIB_cc_task_priority    MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_task_priority)
#define MIB_cc_task_stack_size  MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_task_stack_size)
#define MIB_cc_mm_conn_est_T999 MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_mm_conn_est_T999)
#define MIB_cc_alerting_T301    MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_alerting_T301)
#define MIB_cc_setup_T303   MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_setup_T303)
#define MIB_cc_call_confirmed_T310  MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_call_confirmed_T310)
#define MIB_cc_connect_T313 MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_connect_T313)
#define MIB_cc_disconnect_T305  MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_disconnect_T305)
#define MIB_cc_release_T308 MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_release_T308)
#define MIB_cc_safety_timer MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_safety_timer)
#define MIB_cc_max_calls    MIB_ITEM    (MTBL_callConfigData,   LEAF_cc_max_calls)
#define MIB_mobilityConfigData  MIB_STRUCT  (MTBL_mobilityConfigData,   LEAF_mobilityConfigData,    sizeof(MobilityConfigData))
#define MIB_mm_max_mm_connections   MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_mm_max_mm_connections)
#define MIB_mm_max_L3_msgs  MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_mm_max_L3_msgs)
#define MIB_task_priority   MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_task_priority)
#define MIB_task_stack_size MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_task_stack_size)
#define MIB_mm_paging_t3113 MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_mm_paging_t3113)
#define MIB_mm_authenticationRequired   MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_mm_authenticationRequired)
#define MIB_mm_cipheringRequired    MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_mm_cipheringRequired)
#define MIB_mm_IMEICheckRequired    MIB_ITEM    (MTBL_mobilityConfigData,   LEAF_mm_IMEICheckRequired)
#define MIB_resourceConfigData  MIB_STRUCT  (MTBL_resourceConfigData,   LEAF_resourceConfigData,    sizeof(ResourceConfigData))
#define MIB_rm_t3L01    MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_t3L01)
#define MIB_rm_bsCcChans    MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_bsCcChans)
#define MIB_rm_bsCcchSdcchComb  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_bsCcchSdcchComb)
#define MIB_rm_sysInfoOnOffMap  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_sysInfoOnOffMap)
#define MIB_rm_airInterface MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_airInterface)
#define MIB_rm_viperCellTsc MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_viperCellTsc)
#define MIB_rm_maxSlotPerTrx    MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_maxSlotPerTrx)
#define MIB_rm_maxTrxs  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_maxTrxs)
#define MIB_rm_maxTchfs MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_maxTchfs)
#define MIB_rm_maxSdcch4s   MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_maxSdcch4s)
#define MIB_rm_trxOnOffMap  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxOnOffMap)
#define MIB_rm_networkIfConfig  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_networkIfConfig)
#define MIB_rm_bcchTrx  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_bcchTrx)
#define MIB_rm_preAlphaTrx  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_preAlphaTrx)
#define MIB_rm_ccchBcchComb MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_ccchBcchComb)
#define MIB_rm_ccchConf MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_ccchConf)
#define MIB_rm_nim_0_0  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_0)
#define MIB_rm_nim_0_1  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_1)
#define MIB_rm_nim_0_2  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_2)
#define MIB_rm_nim_0_3  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_3)
#define MIB_rm_nim_0_4  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_4)
#define MIB_rm_nim_0_5  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_5)
#define MIB_rm_nim_0_6  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_6)
#define MIB_rm_nim_0_7  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_0_7)
#define MIB_rm_nim_1_0  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_0)
#define MIB_rm_nim_1_1  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_1)
#define MIB_rm_nim_1_2  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_2)
#define MIB_rm_nim_1_3  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_3)
#define MIB_rm_nim_1_4  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_4)
#define MIB_rm_nim_1_5  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_5)
#define MIB_rm_nim_1_6  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_6)
#define MIB_rm_nim_1_7  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_nim_1_7)
#define MIB_rm_trxSynthSetting_0_0  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_0_0)
#define MIB_rm_trxSynthSetting_0_1  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_0_1)
#define MIB_rm_trxSynthSetting_0_2  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_0_2)
#define MIB_rm_trxSynthSetting_0_3  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_0_3)
#define MIB_rm_trxSynthSetting_1_0  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_1_0)
#define MIB_rm_trxSynthSetting_1_1  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_1_1)
#define MIB_rm_trxSynthSetting_1_2  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_1_2)
#define MIB_rm_trxSynthSetting_1_3  MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxSynthSetting_1_3)
#define MIB_rm_trxFpgaDelay_0_0 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxFpgaDelay_0_0)
#define MIB_rm_trxFpgaDelay_0_1 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxFpgaDelay_0_1)
#define MIB_rm_trxFpgaDelay_1_0 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxFpgaDelay_1_0)
#define MIB_rm_trxFpgaDelay_1_1 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_trxFpgaDelay_1_1)
#define MIB_rm_SGainCtrlSetting_0_0 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_SGainCtrlSetting_0_0)
#define MIB_rm_SGainCtrlSetting_0_1 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_SGainCtrlSetting_0_1)
#define MIB_rm_SGainCtrlSetting_1_0 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_SGainCtrlSetting_1_0)
#define MIB_rm_SGainCtrlSetting_1_1 MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_SGainCtrlSetting_1_1)
#define MIB_rm_synchChanInfo    MIB_ITEM    (MTBL_resourceConfigData,   LEAF_rm_synchChanInfo)

#define MIB_oamConfigData              MIB_STRUCT  (MTBL_oamConfigData,    LEAF_oamConfigData, sizeof(OamConfigData))
#define MIB_oamCommand                 MIB_ITEM    (MTBL_oamConfigData,    LEAF_oamCommand)
#define MIB_oamCommandExecutionStatus  MIB_ITEM    (MTBL_oamConfigData,    LEAF_oamCommandExecutionStatus)
#define MIB_oamAlternateMibFileName    MIB_STRING  (MTBL_oamConfigData,    LEAF_oamAlternateMibFileName,   SIZE_DisplayString)
#define MIB_oamCommandLastError        MIB_STRING  (MTBL_oamConfigData,    LEAF_oamCommandLastError,   SIZE_DisplayString)
#define MIB_mibAccessControlFlag       MIB_ITEM    (MTBL_oamConfigData,    LEAF_mibAccessControlFlag)



#define MIB_maintenanceConfigData   MIB_STRUCT  (MTBL_maintenanceConfigData,    LEAF_maintenanceConfigData, sizeof(MaintenanceConfigData))
#define MIB_maintenance_operation   MIB_STRING  (MTBL_maintenanceConfigData,    LEAF_maintenance_operation, SIZE_DisplayString)
#define MIB_maintenance_response    MIB_STRING  (MTBL_maintenanceConfigData,    LEAF_maintenance_response,  SIZE_DisplayString)

/*  For h323    tags    */

#define MIB_h323Data    MIB_STRUCT  (MTBL_h323Data, LEAF_h323Data,  sizeof(H323Data))
#define MIB_h323_GKPort MIB_ITEM    (MTBL_h323Data, LEAF_h323_GKPort)
#define MIB_h323_GKIPAddress    MIB_IPADDR    (MTBL_h323Data, LEAF_h323_GKIPAddress)
#define MIB_h323_Q931ResponseTimeOut    MIB_ITEM    (MTBL_h323Data, LEAF_h323_Q931ResponseTimeOut)
#define MIB_h323_Q931ConnectTimeOut MIB_ITEM    (MTBL_h323Data, LEAF_h323_Q931ConnectTimeOut)
#define MIB_h323_Q931CallSignalingPort  MIB_ITEM    (MTBL_h323Data, LEAF_h323_Q931CallSignalingPort)
#define MIB_h323_t35CountryCode MIB_ITEM    (MTBL_h323Data, LEAF_h323_t35CountryCode)
#define MIB_h323_rasRequestTimeout  MIB_ITEM    (MTBL_h323Data, LEAF_h323_rasRequestTimeout)
#define MIB_h323_viperbaseHeartbeatTimeout  MIB_ITEM    (MTBL_h323Data, LEAF_h323_viperbaseHeartbeatTimeout)

/*  ViperSpecific   MIB items   */

#define MIB_dualTRXSCard    MIB_STRUCT  (MTBL_dualTRXSCard, LEAF_dualTRXSCard,  sizeof(DualTRXSCard))
#define MIB_monitorReceiverPLL_LockDetectStatus MIB_ITEM    (MTBL_dualTRXSCard, LEAF_monitorReceiverPLL_LockDetectStatus)
#define MIB_referenceFrequencyPLL_LockDetectStatus  MIB_ITEM(MTBL_dualTRXSCard, LEAF_referenceFrequencyPLL_LockDetectStatus)
#define MIB_transceiver_1_PLL_LockDetectStatus  MIB_ITEM    (MTBL_dualTRXSCard, LEAF_transceiver_1_PLL_LockDetectStatus)
#define MIB_transceiver_2A_PLL_LockDetectStatus MIB_ITEM    (MTBL_dualTRXSCard, LEAF_transceiver_2A_PLL_LockDetectStatus)
#define MIB_transceiver_2B_PLL_LockDetectStatus MIB_ITEM    (MTBL_dualTRXSCard, LEAF_transceiver_2B_PLL_LockDetectStatus)
#define MIB_intermediateFrequency_LockDetectStatus  MIB_ITEM    (MTBL_dualTRXSCard, LEAF_intermediateFrequency_LockDetectStatus)
#define MIB_trxSerialNumber MIB_STRING  (MTBL_dualTRXSCard, LEAF_trxSerialNumber,   SIZE_DisplayString)
#define MIB_trxSoftwareVersion  MIB_STRING  (MTBL_dualTRXSCard, LEAF_trxSoftwareVersion,    SIZE_DisplayString)
#define MIB_trxLoopBackMode MIB_ITEM    (MTBL_dualTRXSCard, LEAF_trxLoopBackMode)

#define MIB_arfcnRssiFunction   MIB_STRUCT  (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiFunction, sizeof(ArfcnRssiFunction))
#define MIB_arfcnRssiControlMode    MIB_ITEM    (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiControlMode)
#define MIB_clockRecoveryControlMode    MIB_ITEM    (MTBL_arfcnRssiFunction,    LEAF_clockRecoveryControlMode)
#define MIB_arfcnRssiClockControlTable  MIB_TABLE   (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiClockControlTable)
#define MIBT_arfcnRssiClockControlEntry MIB_TENTRY  (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiClockControlTable,    sizeof(ArfcnRssiClockControlEntry))
#define MIBT_arfcnRssiClockControlIndex MIB_TITEM   (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiClockControlTable,    LEAF_arfcnRssiClockControlIndex)
#define MIBT_arfcnRssiClockControlArfcn MIB_TITEM   (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiClockControlTable,    LEAF_arfcnRssiClockControlArfcn)
#define MIBT_arfcnRssiClockRecoveryMode MIB_TITEM   (MTBL_arfcnRssiFunction,    LEAF_arfcnRssiClockControlTable,    LEAF_arfcnRssiClockRecoveryMode)
#define MIB_clockCard   MIB_STRUCT  (MTBL_clockCard,    LEAF_clockCard, sizeof(ClockCard))
#define MIB_clockCardType   MIB_ITEM    (MTBL_clockCard,    LEAF_clockCardType)
#define MIB_clockCardSoftwareVersion    MIB_STRING  (MTBL_clockCard,    LEAF_clockCardSoftwareVersion,  SIZE_DisplayString)
#define MIB_clockCardStatus MIB_ITEM    (MTBL_clockCard,    LEAF_clockCardStatus)
#define MIB_clockCardCrystalUpTime  MIB_STRING  (MTBL_clockCard,    LEAF_clockCardCrystalUpTime,    SIZE_DisplayString)
#define MIB_clockCardDAC    MIB_ITEM    (MTBL_clockCard,    LEAF_clockCardDAC)
#define MIB_gpsCard MIB_STRUCT  (MTBL_gpsCard,  LEAF_gpsCard,   sizeof(GpsCard))
#define MIB_gpsCardType MIB_ITEM    (MTBL_gpsCard,  LEAF_gpsCardType)
#define MIB_gpsCardSoftwareVersion  MIB_STRING  (MTBL_gpsCard,  LEAF_gpsCardSoftwareVersion,    SIZE_DisplayString)
#define MIB_gpsCardStatus   MIB_ITEM    (MTBL_gpsCard,  LEAF_gpsCardStatus)
#define MIB_gpsPosition MIB_STRING  (MTBL_gpsCard,  LEAF_gpsPosition,   SIZE_DisplayString)
#define MIB_gpsTime MIB_STRING  (MTBL_gpsCard,  LEAF_gpsTime,   SIZE_DisplayString)
#define MIB_utcTime MIB_STRING  (MTBL_gpsCard,  LEAF_utcTime,   SIZE_DisplayString)
#define MIB_clockStatusFrequencyError   MIB_ITEM    (MTBL_gpsCard,  LEAF_clockStatusFrequencyError)
#define MIB_clockStatusDACVoltage   MIB_ITEM    (MTBL_gpsCard,  LEAF_clockStatusDACVoltage)
#define MIB_gpsAntennaStatus    MIB_ITEM    (MTBL_gpsCard,  LEAF_gpsAntennaStatus)
#define MIB_gpsSatelliteStatus  MIB_ITEM    (MTBL_gpsCard,  LEAF_gpsSatelliteStatus)
#define MIB_powerCard   MIB_STRUCT  (MTBL_powerCard,    LEAF_powerCard, sizeof(PowerCard))
#define MIB_powerCardType   MIB_ITEM    (MTBL_powerCard,    LEAF_powerCardType)
#define MIB_powerCardStatus MIB_ITEM    (MTBL_powerCard,    LEAF_powerCardStatus)
#define MIB_powerOn MIB_ITEM    (MTBL_powerCard,    LEAF_powerOn)
#define MIB_externalPowerSupply MIB_STRUCT  (MTBL_externalPowerSupply,  LEAF_externalPowerSupply,   sizeof(ExternalPowerSupply))
#define MIB_powerSupplyType MIB_ITEM    (MTBL_externalPowerSupply,  LEAF_powerSupplyType)
#define MIB_powerSupplyStatus   MIB_ITEM    (MTBL_externalPowerSupply,  LEAF_powerSupplyStatus)
#define MIB_cdcBoard    MIB_STRUCT  (MTBL_cdcBoard, LEAF_cdcBoard,  sizeof(CdcBoard))
#define MIB_cdcBoardSerialNumber    MIB_STRING  (MTBL_cdcBoard, LEAF_cdcBoardSerialNumber,  SIZE_DisplayString)
#define MIB_cdcBoardMACAddress  MIB_STRING  (MTBL_cdcBoard, LEAF_cdcBoardMACAddress,    SIZE_DisplayString)
#define MIB_automaticParameterAllocation    MIB_STRUCT  (MTBL_automaticParameterAllocation, LEAF_automaticParameterAllocation,  sizeof(AutomaticParameterAllocation))
#define MIB_apaEnable   MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaEnable  )
#define MIB_apaScanFrequencyLowBound    MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaScanFrequencyLowBound   )
#define MIB_apaScanFrequencyHighBound   MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaScanFrequencyHighBound  )
#define MIB_apaMaxPowerSetting  MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaMaxPowerSetting)
#define MIB_apaMinRSSIThresholdForNeighborId    MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaMinRSSIThresholdForNeighborId)
#define MIB_apaScanRate MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaScanRate)
#define MIB_apaHysteresisForBCCHPower   MIB_ITEM    (MTBL_automaticParameterAllocation, LEAF_apaHysteresisForBCCHPower)
#define MIB_apaInvalidFrequencyTable    MIB_TABLE   (MTBL_automaticParameterAllocation, LEAF_apaInvalidFrequencyTable)
#define MIBT_apaInvalidFrequencyEntry   MIB_TENTRY  (MTBL_automaticParameterAllocation, LEAF_apaInvalidFrequencyTable,  sizeof(ApaInvalidFrequencyEntry))
#define MIBT_apaInvalidFrequencyIndex   MIB_TITEM   (MTBL_automaticParameterAllocation, LEAF_apaInvalidFrequencyTable,  LEAF_apaInvalidFrequencyIndex)
#define MIBT_apaInvalidFrequencyArfcn   MIB_TITEM   (MTBL_automaticParameterAllocation, LEAF_apaInvalidFrequencyTable,  LEAF_apaInvalidFrequencyArfcn)
#define MIB_apaClockSourceFrequencyTable    MIB_TABLE   (MTBL_automaticParameterAllocation, LEAF_apaClockSourceFrequencyTable)
#define MIBT_apaClockSourceFrequencyEntry   MIB_TENTRY  (MTBL_automaticParameterAllocation, LEAF_apaClockSourceFrequencyTable,  sizeof(ApaClockSourceFrequencyEntry))
#define MIBT_apaClockSourceFrequencyIndex   MIB_TITEM   (MTBL_automaticParameterAllocation, LEAF_apaClockSourceFrequencyTable,  LEAF_apaClockSourceFrequencyIndex)
#define MIBT_apaClockSourceFrequencyArfcn   MIB_TITEM   (MTBL_automaticParameterAllocation, LEAF_apaClockSourceFrequencyTable,  LEAF_apaClockSourceFrequencyArfcn)

#define MIB_gp10MgmtInfoMib         MIB_STRUCT     (MTBL_gp10MgmtInfoMib,  LEAF_gp10MgmtInfoMib,         sizeof(Gp10MgmtInfoMib))
#define MIB_viperCellName           MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellName,           SIZE_DisplayString)
#define MIB_viperCellIPAddress      MIB_IPADDR     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellIPAddress)
#define MIB_viperCellDefGateway     MIB_IPADDR     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellDefGateway)
#define MIB_viperCellLocation       MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellLocation,       SIZE_DisplayString)
#define MIB_viperCellSerialNumber   MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellSerialNumber,   SIZE_DisplayString)
#define MIB_viperCellAssetNumber    MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellAssetNumber,    SIZE_DisplayString)
#define MIB_viperCellSoftwareBuild  MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellSoftwareBuild,  SIZE_DisplayString)
#define MIB_viperCellCustomerName   MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellCustomerName,   SIZE_DisplayString)
#define MIB_viperCellCustomerAddress    MIB_STRING (MTBL_gp10MgmtInfoMib,  LEAF_viperCellCustomerAddress,SIZE_DisplayString)
#define MIB_viperCellCustomerPhone  MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellCustomerPhone,  SIZE_DisplayString)
#define MIB_viperCellCustomerEmail  MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellCustomerEmail,  SIZE_DisplayString)
#define MIB_viperCellCustomerInfo   MIB_STRING     (MTBL_gp10MgmtInfoMib,  LEAF_viperCellCustomerInfo,   SIZE_DisplayString)

#define MIB_viperAccessMib  MIB_STRUCT  (MTBL_viperAccessMib,   LEAF_viperAccessMib,    sizeof(ViperAccessMib))
#define MIB_readCommunity			MIB_STRING (MTBL_viperAccessMib, LEAF_readCommunity, SIZE_DisplayString)
#define MIB_writeCommunity			MIB_STRING (MTBL_viperAccessMib, LEAF_writeCommunity, SIZE_DisplayString)


#define MIB_gp10ErrorInfoMib    MIB_STRUCT  (MTBL_gp10ErrorInfoMib, LEAF_gp10ErrorInfoMib,  sizeof(Gp10ErrorInfoMib))
#define MIB_errorCode_L1    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_L1)
#define MIB_errorCode_MD    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_MD)
#define MIB_errorCode_RM    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_RM)
#define MIB_errorCode_MM    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_MM)
#define MIB_errorCode_CC    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_CC)
#define MIB_errorCode_L2    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_L2)
#define MIB_errorCode_SMS   MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_SMS)
#define MIB_errorCode_OAM   MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_OAM)
#define MIB_errorCode_H323  MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_H323)
#define MIB_errorCode_LUDB  MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_LUDB)
#define MIB_errorCode_EXT_LC    MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_EXT_LC)
#define MIB_errorCode_LOG   MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_LOG)
#define MIB_errorCode_MCH   MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_MCH)
#define MIB_errorCode_ALARM MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_ALARM)
#define MIB_errorCode_SYSINIT   MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_SYSINIT)

#define MIB_gp10CdrMib	MIB_STRUCT (MTBL_gp10CdrMib, LEAF_gp10CdrMib, sizeof(Gp10CdrMib))
#define	MIB_cdrClientSSLEnable	MIB_ITEM (MTBL_gp10CdrMib, LEAF_cdrClientSSLEnable)
#define MIB_cdrLongCallTimer	MIB_ITEM (MTBL_gp10CdrMib, LEAF_cdrLongCallTimer)

#define	MIB_handoverTimerMib	MIB_STRUCT(MTBL_handoverTimerMib, LEAF_handoverTimerMib, sizeof(HandoverTimerMib))
#define	MIB_t101	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t101)
#define	MIB_t102	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t102)
#define	MIB_t103	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t103)
#define	MIB_t104	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t104)
#define	MIB_t201	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t201)
#define	MIB_t202	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t202)
#define	MIB_t204	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t204)
#define	MIB_t210	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t210)
#define	MIB_t211	MIB_ITEM(MTBL_handoverTimerMib, LEAF_t211)

#define	MIB_gp10ServiceStatusMib	MIB_STRUCT(MTBL_gp10ServiceStatusMib, LEAF_gp10ServiceStatusMib, sizeof(Gp10ServiceStatusMib))
#define	MIB_snmpServiceEnabled		MIB_ITEM(MTBL_gp10ServiceStatusMib, LEAF_snmpServiceEnabled)
#define	MIB_httpServiceEnabled		MIB_ITEM(MTBL_gp10ServiceStatusMib, LEAF_httpServiceEnabled)
#define	MIB_telnetServiceEnabled	MIB_ITEM(MTBL_gp10ServiceStatusMib, LEAF_telnetServiceEnabled)
#define	MIB_sshServiceEnabled		MIB_ITEM(MTBL_gp10ServiceStatusMib, LEAF_sshServiceEnabled)
#define	MIB_ftpServiceEnabled		MIB_ITEM(MTBL_gp10ServiceStatusMib, LEAF_ftpServiceEnabled)

/*  New Defines for supporting  GS  */
#define MIB_btsGsBasicPackage			MIB_STRUCT  (MTBL_btsGsBasicPackage,    LEAF_btsGsBasicPackage, sizeof(BtsGsBasicPackage))
#define MIB_gprsServerIpAddress			MIB_IPADDR	(MTBL_btsGsBasicPackage,    LEAF_gprsServerIpAddress)
#define MIB_gprsRac						MIB_ITEM    (MTBL_btsGsBasicPackage,    LEAF_gprsRac)
#define MIB_enableGprs					MIB_ITEM    (MTBL_btsGsBasicPackage,    LEAF_enableGprs)
#define MIB_gprsOperationalStatus		MIB_ITEM    (MTBL_btsGsBasicPackage,    LEAF_gprsOperationalStatus)
										
#define MIB_btsGsOptionPackage			MIB_STRUCT  (MTBL_btsGsOptionPackage,   LEAF_btsGsOptionPackage,    sizeof(BtsGsOptionPackage))
#define MIB_nmo							MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_nmo)
#define MIB_t3168						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_t3168)
#define MIB_t3192						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_t3192)
#define MIB_drxTimerMax					MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_drxTimerMax)
#define MIB_accessBurstType				MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_accessBurstType)
#define MIB_controlAckType				MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_controlAckType)
#define MIB_bsCvMax						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_bsCvMax)
#define MIB_panDec						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_panDec)
#define MIB_panInc						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_panInc)
#define MIB_panMax						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_panMax)
#define MIB_raColour					MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_raColour)
#define MIB_si13Position				MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_si13Position)
#define MIB_cbchTrxSlot					MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_cbchTrxSlot)
#define MIB_cbchTrx						MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_cbchTrx)	
#define MIB_bcchChangeMark				MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_bcchChangeMark)
#define MIB_siChangeField				MIB_ITEM    (MTBL_btsGsOptionPackage,   LEAF_siChangeField)

#define MIB_rss							MIB_STRUCT  (MTBL_rss,  LEAF_rss,   sizeof(Rss))
#define MIB_gprsMsTxpwrMaxCch			MIB_ITEM    (MTBL_rss,  LEAF_gprsMsTxpwrMaxCch)
#define MIB_gprsRexLevAccessMin			MIB_ITEM    (MTBL_rss,  LEAF_gprsRexLevAccessMin)
#define MIB_gprsReselectOffset			MIB_ITEM    (MTBL_rss,  LEAF_gprsReselectOffset)
#define MIB_priorityClass				MIB_ITEM    (MTBL_rss,  LEAF_priorityClass)
#define MIB_lsaId						MIB_ITEM    (MTBL_rss,  LEAF_lsaId)
#define MIB_hcsThreshold				MIB_ITEM    (MTBL_rss,  LEAF_hcsThreshold)
#define MIB_gprsTemporaryOffset			MIB_ITEM    (MTBL_rss,  LEAF_gprsTemporaryOffset)
#define MIB_gprsPenaltyTime				MIB_ITEM    (MTBL_rss,  LEAF_gprsPenaltyTime)
#define MIB_gprsCellReselectHysteresis  MIB_ITEM    (MTBL_rss,  LEAF_gprsCellReselectHysteresis)
#define MIB_gprsRaReselectHysteresis    MIB_ITEM    (MTBL_rss,  LEAF_gprsRaReselectHysteresis)
#define MIB_c32Qual						MIB_ITEM    (MTBL_rss,  LEAF_c32Qual)
#define MIB_c31Hysteresis				MIB_ITEM    (MTBL_rss,  LEAF_c31Hysteresis)
#define MIB_alpha						MIB_ITEM    (MTBL_rss,  LEAF_alpha)
#define MIB_pB							MIB_ITEM    (MTBL_rss,  LEAF_pB)
#define MIB_networkControlOrder			MIB_ITEM    (MTBL_rss,  LEAF_networkControlOrder)
    
#define MIB_ncFrequencyListTable		MIB_TABLE   (MTBL_rss,  LEAF_ncFrequencyListTable)
#define MIBT_ncFrequencyListEntry		MIB_TENTRY  (MTBL_rss,  LEAF_ncFrequencyListTable,  sizeof(NcFrequencyListEntry))
#define MIBT_ncFrequencyListIndex		MIB_TITEM   (MTBL_rss,  LEAF_ncFrequencyListTable,  LEAF_ncFrequencyListIndex)
#define MIBT_ncFrequency				MIB_TITEM   (MTBL_rss,  LEAF_ncFrequencyListTable,  LEAF_ncFrequency)

#define MIB_ncReportingPeriodI			MIB_ITEM    (MTBL_rss,  LEAF_ncReportingPeriodI)
#define MIB_ncReportingPeriodT			MIB_ITEM    (MTBL_rss,  LEAF_ncReportingPeriodT)
#define MIB_extMeasurementOrder			MIB_ITEM    (MTBL_rss,  LEAF_extMeasurementOrder)
#define MIB_extReportingPeriod			MIB_ITEM    (MTBL_rss,  LEAF_extReportingPeriod)
#define MIB_extReportingType			MIB_ITEM    (MTBL_rss,  LEAF_extReportingType)
#define MIB_intFrequency				MIB_ITEM    (MTBL_rss,  LEAF_intFrequency)
#define MIB_nccPermitted				MIB_ITEM    (MTBL_rss,  LEAF_nccPermitted)
#define MIB_pcMeasChan					MIB_ITEM    (MTBL_rss,  LEAF_pcMeasChan)
#define MIB_tavgW						MIB_ITEM    (MTBL_rss,  LEAF_tavgW)
#define MIB_tavgT						MIB_ITEM    (MTBL_rss,  LEAF_tavgT)
#define MIB_navgI						MIB_ITEM    (MTBL_rss,  LEAF_navgI)
#define MIB_gprsMsTargetPower			MIB_ITEM    (MTBL_rss,  LEAF_gprsMsTargetPower)
#define MIB_spgcCcchSup					MIB_ITEM    (MTBL_rss,  LEAF_spgcCcchSup)

#define MIB_rlcMac						MIB_STRUCT  (MTBL_rlcMac,   LEAF_rlcMac,    sizeof(RlcMac))
#define MIB_t3169						MIB_ITEM    (MTBL_rlcMac,   LEAF_t3169)
#define MIB_t3191						MIB_ITEM    (MTBL_rlcMac,   LEAF_t3191)
#define MIB_t3193						MIB_ITEM    (MTBL_rlcMac,   LEAF_t3193)
#define MIB_t3195						MIB_ITEM    (MTBL_rlcMac,   LEAF_t3195)

#define MIB_bssgp						MIB_STRUCT  (MTBL_bssgp,    LEAF_bssgp, sizeof(Bssgp))
#define MIB_blockTimer					MIB_ITEM    (MTBL_bssgp,    LEAF_blockTimer)
#define MIB_resetTimer					MIB_ITEM    (MTBL_bssgp,    LEAF_resetTimer)
#define MIB_suspendTimer				MIB_ITEM    (MTBL_bssgp,    LEAF_suspendTimer)
#define MIB_resumeTimer					MIB_ITEM    (MTBL_bssgp,    LEAF_resumeTimer)
#define MIB_capUpdateTimer				MIB_ITEM    (MTBL_bssgp,    LEAF_capUpdateTimer)
#define MIB_blockRetries				MIB_ITEM    (MTBL_bssgp,    LEAF_blockRetries)
#define MIB_unBlockRetries				MIB_ITEM    (MTBL_bssgp,    LEAF_unBlockRetries)
#define MIB_resetRetries				MIB_ITEM	(MTBL_bssgp,    LEAF_resetRetries)
#define MIB_suspendRetries				MIB_ITEM	(MTBL_bssgp,    LEAF_suspendRetries)
#define MIB_resumeRetries				MIB_ITEM	(MTBL_bssgp,    LEAF_resumeRetries)
#define MIB_raCapUpdateRetries			MIB_ITEM	(MTBL_bssgp,    LEAF_raCapUpdateRetries)
#define MIB_timeInterval				MIB_ITEM	(MTBL_bssgp,    LEAF_timeInterval)
#define MIB_networkPort					MIB_ITEM	(MTBL_bssgp,    LEAF_networkPort)

#define MIB_btsGsMeasPackage			MIB_STRUCT	(MTBL_btsGsMeasPackage, LEAF_btsGsMeasPackage,  sizeof(BtsGsMeasPackage))
#define MIB_flushReqRecvd				MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_flushReqRecvd)
#define MIB_pagingReqRecvd				MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_pagingReqRecvd)
#define MIB_meanPsInterArrivalTime		MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_meanPsInterArrivalTime)
#define MIB_availablePdch				MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_availablePdch)
#define MIB_meanAvailablePdch			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_meanAvailablePdch)
#define MIB_maxAvailablePdch			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_maxAvailablePdch)
#define MIB_minAvailablePdch			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_minAvailablePdch)
#define MIB_meanOccupiedPdch			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_meanOccupiedPdch)
#define MIB_maxOccupiedPdch				MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_maxOccupiedPdch)
#define MIB_minOccupiedPdch				MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_minOccupiedPdch)
#define MIB_availablePdchAllocTime		MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_availablePdchAllocTime)
#define MIB_packetPagingMessages		MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_packetPagingMessages)
#define MIB_pagchQueueLength			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_pagchQueueLength)
#define MIB_pagesDiscardedFromPpch		MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_pagesDiscardedFromPpch)
#define MIB_assignmentRequest			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_assignmentRequest)
#define MIB_succAssignmentProc			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_succAssignmentProc)
#define MIB_succPdtchSeizures			MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_succPdtchSeizures)
#define MIB_meanPacketQueueLength		MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_meanPacketQueueLength)
#define MIB_serviceChanges				MIB_ITEM	(MTBL_btsGsMeasPackage, LEAF_serviceChanges)


#define MIB_errorCode_GRR   MIB_ITEM    (MTBL_gp10ErrorInfoMib, LEAF_errorCode_RM)
#define MIB_errorCode_CDR	MIB_ITEM (MTBL_gp10ErrorInfoMib, LEAF_errorCode_CDR)


/*  special section to  redefine    some    common  MIB objects */
#define MIB_mnetIpAddress           MIB_viperCellIPAddress
#define MIB_mnetName                MIB_viperCellName
#define MIB_mnetSoftwareBuild       MIB_viperCellSoftwareBuild
#define MIBL_mnetErrorInfoMib       MTBL_gp10ErrorInfoMib

/*  Trap    Forwarding  table   */
#define LEAF_trapFwdData    0
#define LEAF_trapFwdTable   1
#define LEAF_trapFwdEntry   0


#define MIB_trapFwdData MIB_STRUCT  (MTBL_trapFwdData,  LEAF_trapFwdData,   sizeof(TrapFwdData))
#define MIB_trapFwdTable    MIB_TABLE   (MTBL_trapFwdData,  LEAF_trapFwdTable)
#define MIBT_trapFwdEntry   MIB_TENTRY  (MTBL_trapFwdData,  LEAF_trapFwdTable,  sizeof(TrapFwdEntry))
    
#define MIBT_trapFwdIndex   MIB_TITEM   (MTBL_trapFwdData,  LEAF_trapFwdTable,  LEAF_trapFwdIndex)
#define MIBT_trapType   MIB_TITEM   (MTBL_trapFwdData,  LEAF_trapFwdTable,  LEAF_trapType)
#define MIBT_trapIpAddress  MIB_TIPADDR (MTBL_trapFwdData,  LEAF_trapFwdTable,  LEAF_trapIpAddress)
#define MIBT_trapPort   MIB_TITEM   (MTBL_trapFwdData,  LEAF_trapFwdTable,  LEAF_trapPort)
#define MIBT_trapCommunity  MIB_TSTRING (MTBL_trapFwdData,  LEAF_trapFwdTable,  LEAF_trapCommunity)

#define SNMP_AGENT_MODULE_ID MODULE_OAM

#endif  /*  MIB_TAG */


