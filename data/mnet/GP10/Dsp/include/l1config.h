/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
** Filename: l1config.h
**
** Target Platform: 
**   TMS320C6201 
**
** Description:   
**   This file contains L1 defaule configuration values
** 
****************************************************************************/ 

#define RF_LINK_LOST_THRESHOLD 12   /* Lost SACCH Frame threshold for Radio link loss */

#define RCV_AGC_DYNAMIC_RANGE  60   /* Current Radio Board DYMANIC_RANGE is 30 dB */                         

#define TOA_DETECTION_MAX      4    /* maximum TOA for a Valid RACH detection */


#define RACH_BIT_ERROR_MAX     5    /* maximum BIT ERROR for a Valid RACH detection */


#define SNR_RAW_MININUM        6    /* mininum raw snr for a Valid RACH detection */


#define MIN_RCV_POWER  (SNR_RAW_MININUM - RCV_AGC_DYNAMIC_RANGE)

/* MAX_RCV_POWER is not dependent upon R47's presence or absence on the radio board */
#define MAX_RCV_POWER          0
                                       
#define  NUM_RX_SIGNAL_SAMPLES	8

#define DBM_2_RXLEV_CONV_FACTOR    110  /* subtract 110 from dBm to get RXLEV */ 
#define RXLEV_2_DBM_CONV_FACTOR   -110  /* converse operation */
