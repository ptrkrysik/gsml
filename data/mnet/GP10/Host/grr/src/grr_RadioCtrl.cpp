/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __GRR_RADIOCTRL_CPP__
#define __GRR_RADIOCTRL_CPP__

#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"
#include "grr/grr_head.h"

extern int l1pTrxSwapState;              /* 1/0 - swap/don't swap trx-dsp mapping */
extern "C" int DS1780_getCdcTemp(void);
extern "C" STATUS I2Cwrite(unsigned char*, unsigned char, unsigned char, int);
extern "C" STATUS I2Cread(unsigned char*, unsigned char, unsigned char, int);
extern "C" void   I2Coperation(int);

#define MONITOR_INTERVAL 60  // in seconds

#define ON  1
#define OFF 0

/* Added to support ConfigTxPwrValues in l1Proxy.cpp */
#define  PWR_TABLE_ADDR     0x00
#define  TX_STEP_TABLE_SIZE   36
#define  TX_FREQ_SWEEP_SIZE   15
#define  RX_GAIN_CHAR_SIZE     6
#define  END_MARKER_SIZE       1
#define  END_MARKER_VALUE   0x2b

#define  PWR_TABLE_SIZE  (2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + RX_GAIN_CHAR_SIZE + END_MARKER_SIZE)

#define EEPROM_STATUS_OK            0
#define EEPROM_STATUS_UPDATE        1
#define EEPROM_STATUS_REPORT_ERROR  2

int grrPowerValsConfigured = 0;

/*****************************************************************************
 *
 *  Module Name: pwrtempadjust
 *  
 *  Purpose:    Query system temperature, determine Tx DAC offset due to
 *              temperature and send the offset to both DSPs
 *
 *****************************************************************************/
#define CLIP_VALUE 75

/* globals for all functions */

int lowTempStart = 10;
float lowTempSlope = 0.99;
int highTempStart = 44;
float highTempSlope = 1.02;
int lowestTempRead;
int highestTempRead;
int deltaDAC = 0;    
int PwrAdjDiag=0;
int gEnablePwrTempAdjust=1;

void InitializeTempAdjust(int interval)
{
    DBG_FUNC("InitializeTempAdjust",GRR_LAYER_DSP);
    DBG_ENTER(); 
      
    int temperatureValue;
      
    temperatureValue=DS1780_getCdcTemp();
    lowestTempRead=temperatureValue;
    highestTempRead=temperatureValue;

    DBG_TRACE("Power Temperature Adjust Active with interval time of %d\n",interval);
    DBG_TRACE("   Low temp start point:%d    Low temp slope:%f\n",lowTempStart,lowTempSlope);
    DBG_TRACE("   High temp start point:%d   High temp slope:%f\n",highTempStart,highTempSlope);
    
    DBG_LEAVE(); 
}


void pwrtempadjust ( )
{
    int temperatureValue;
    int deltaDAC;    
    unsigned char  buffer[40];       

    DBG_FUNC("pwrtempadjust",GRR_LAYER_DSP);
    DBG_ENTER();   
        temperatureValue=DS1780_getCdcTemp();
        
    /*
    * If temperature compensation is enabled and measured temperature is within legal range...
    */
    if ( gEnablePwrTempAdjust && CLIP_VALUE>temperatureValue )
    {
        if (temperatureValue>highestTempRead)
            highestTempRead=temperatureValue;

        if (temperatureValue<lowestTempRead)
            lowestTempRead=temperatureValue;

        if (PwrAdjDiag>=2) {
            DBG_TRACE("   pwrtempadjust: Temperature is %d\n",temperatureValue);
        }

        deltaDAC = 0;
        if (temperatureValue>highTempStart) {
            deltaDAC = (int)(highTempSlope*(float)(temperatureValue-highTempStart));
            if (PwrAdjDiag>=1) {
                DBG_TRACE("   pwrtempadjust: High temperature is %d, Adjustment is %d\n", temperatureValue, deltaDAC);
            }
        } else if (temperatureValue<lowTempStart) {
            deltaDAC = (int)(lowTempSlope*(float)(temperatureValue-lowTempStart));
            if (PwrAdjDiag>=1) {
                DBG_TRACE("   pwrtempadjust: Low temperature is %d, Adjustment is %d\n", temperatureValue, deltaDAC);
            }
        }

        /*-- Send Temp Compensation message to DSP 0 --*/            
        buffer[0] = TRX_MANAGE_MSG;     /* function type */
        buffer[1] = TEMPERATURE_COMPENSATION>>8;
        buffer[2] = TEMPERATURE_COMPENSATION;
        buffer[3] = 0;
        buffer[4] = deltaDAC & 0xff;    /* DAC offset due to temperature */
        api_SendMsgFromRmToL1(5, buffer);

        /*-- Send Temp Compensation message to DSP 1 --*/            
        buffer[3] = 1;
        api_SendMsgFromRmToL1(5, buffer);

        if (PwrAdjDiag>=2) {
            DBG_TRACE("   pwrtempadjust: out of control loop\n");
        }   
    }
    else if ( gEnablePwrTempAdjust )
    {
        if (PwrAdjDiag>=1) {
           DBG_ERROR("   pwrtempadjust: TEMPERATURE OUT OF RANGE: %d\n",temperatureValue);
        }
    }
    else
    {
        if (PwrAdjDiag>=2) {
           DBG_TRACE("   pwrtempadjust: Temperature compensation is disabled (gEnablePwrTempAdjust=0)\n");
        }
    }
            
   DBG_LEAVE(); 
}

/*****************************************************************************
 *
 *  Module Name: pwrtempadjustPrintStats
 *  
 *****************************************************************************/
int pwrtempadjustPrintStats(void)
{
    printf("     pwrtempadjust: lowest temp, highest temp, current deltaDAC: %d, %d, %d\n",
      lowestTempRead, highestTempRead, deltaDAC);
}

/*****************************************************************************
 *
 *  Module Name: EnablePwrTempAdjust
 *  
 *****************************************************************************/
void EnablePwrTempAdjust(void)
{
   gEnablePwrTempAdjust = 1;
   printf("    Temperature Compensation is now Enabled\n");
}

/*****************************************************************************
 *
 *  Module Name: DisablePwrTempAdjust
 *  
 *****************************************************************************/
void DisablePwrTempAdjust(void)
{
   unsigned char  buffer[40]; 

   gEnablePwrTempAdjust = 0;
            
   /*-- Send Temp Compensation message to DSP 0 --*/            
   buffer[0] = TRX_MANAGE_MSG;     /* function type */
   buffer[1] = TEMPERATURE_COMPENSATION>>8;
   buffer[2] = TEMPERATURE_COMPENSATION;
   buffer[3] = 0;    /* trx 0 */
   buffer[4] = 0;    /* DAC offset due to temperature */
   api_SendMsgFromRmToL1(5, buffer);

   /*-- Send Temp Compensation message to DSP 1 --*/            
   buffer[3] = 1;  /* trx 1 */
   api_SendMsgFromRmToL1(5, buffer);
   
   printf("    Temperature Compensation is now Disabled\n"); 
}


/*****************************************************************************
 *
 *  Module Name: grr_RadioMonitor
 *  
 *****************************************************************************/
void grr_RadioMonitor()
{
    RMDBG_ENTER(("ENTER@rm_RadioMonitor()",GRR_LAYER_DSP));

    int interval = sysClkRateGet() * MONITOR_INTERVAL; 

    InitializeTempAdjust(interval);
    
    grrMonTaskObj.JCTaskEnterLoop();
   
    /* spin here until some power values have been configured */
    while (grrPowerValsConfigured <= 1)
    {
        taskDelay(10);
    }
   
    while (1)
    {
        taskDelay(interval);
        
        pwrtempadjust();
    }

    grrMonTaskObj.JCTaskNormExit();

}


/*****************************************************************************
 *
 *  Module Name: ConfigTxPwrValues
 *  
 *  Purpose:     Handler for the Request to Configure Radio Tx Pwr Values msg.
 *               It reads the Power Tables for both Tx's and sends them to the 
 *               associated DSP.
 *
 *****************************************************************************/
void ConfigTxPwrValues(unsigned char trx)
{
   STATUS         I2CStatus;
   unsigned char  pwrCalTable[PWR_TABLE_SIZE];
   unsigned char  *pwrcal_ptr = &pwrCalTable[0];
   unsigned char  radioEEpromDevAddr = 0x53;
   UINT8          MsgLen = 0;
   int            i;
   unsigned short length;     
   unsigned char  buffer[40];       

   DBG_FUNC("ConfigTxPwrValues",GRR_LAYER_DSP);
   DBG_ENTER();   
   
   I2Coperation(ON);
   I2CStatus = I2Cread(pwrcal_ptr, radioEEpromDevAddr, (unsigned char)PWR_TABLE_ADDR,  (int)PWR_TABLE_SIZE);
   I2Coperation(OFF);

   if ( I2CStatus != OK )
   {
      DBG_ERROR( "Error in I2Cread() from ConfigTxPwrValues()\n" );
   }
   else
   {
      /*
      * Construct a temporary packed calibration table for the appropriate DSP. Start with the
      * appropriate 36-byte step table, followed by the appropriate 15-byte frequency sweep,
      * followed by the 6-byte rx gain table. These sub-tables will be sent to the DSP below.
      */
      if((!l1pTrxSwapState && trx == 0) || (l1pTrxSwapState && trx == 1))
      {
         /*
         * Construct packed calibration table for hopping radio on DSP A
         */       
         for ( i=0; i<RX_GAIN_CHAR_SIZE ; i++ )  // move Rx gain table to follow step and sweep tables
         {
            pwrCalTable[TX_STEP_TABLE_SIZE + TX_FREQ_SWEEP_SIZE + i]
            = pwrCalTable[2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + i];
         }
         
       }
      else
      {
         /*
         * Construct packed calibration table for non-hopping radio on DSP B
         */         
         for ( i=0; i<TX_STEP_TABLE_SIZE ; i++ )  // move step table to beginning
         {
            pwrCalTable[i] = pwrCalTable[TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + i];
         }
         for ( i=0; i<TX_FREQ_SWEEP_SIZE ; i++ )  // move sweep table to follow step table
         {
            pwrCalTable[TX_STEP_TABLE_SIZE + i] = pwrCalTable[TX_STEP_TABLE_SIZE + TX_FREQ_SWEEP_SIZE + i];
         }
         for ( i=0; i<RX_GAIN_CHAR_SIZE ; i++ )  // move Rx gain table to follow sweep table
         {
            pwrCalTable[TX_STEP_TABLE_SIZE + TX_FREQ_SWEEP_SIZE + i]
            = pwrCalTable[2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + i];
         }
      }
      /*
      * Send Tx Pwr Step Table to this DSP
      */         
      length = 0;
      buffer[length++] = TRX_MANAGE_MSG;     /* function type */
      buffer[length++] = RADIO_TX_PWR_CHAR_CONFIG>>8;
      buffer[length++] = RADIO_TX_PWR_CHAR_CONFIG;
      buffer[length++] = trx;
      for(i=0; i<TX_STEP_TABLE_SIZE; i++)
      {
         buffer[length++] = pwrCalTable[i];
      }
      api_SendMsgFromRmToL1(length, buffer);

      /*
      * Send Tx Offset vs. Frequency Table to this DSP
      */         
      length = 0;
      buffer[length++] = TRX_MANAGE_MSG;     /* function type */
      buffer[length++] = TX_PWR_OFFSETS_CONFIG>>8;
      buffer[length++] = TX_PWR_OFFSETS_CONFIG;
      buffer[length++] = trx;
      for(i=0; i<TX_FREQ_SWEEP_SIZE; i++)
      {
         buffer[length++] = pwrCalTable[TX_STEP_TABLE_SIZE + i];
      }
      api_SendMsgFromRmToL1(length, buffer);
      
      /*
      * Send Tx Offset vs. Frequency Table to this DSP
      */         
      length = 0;
      buffer[length++] = TRX_MANAGE_MSG;     /* function type */
      buffer[length++] = RADIO_GAIN_CHAR_CONFIG>>8;
      buffer[length++] = RADIO_GAIN_CHAR_CONFIG;
      buffer[length++] = trx;
      for(i=0; i<RX_GAIN_CHAR_SIZE; i++)
      {
       buffer[length++] = pwrCalTable[TX_STEP_TABLE_SIZE + TX_FREQ_SWEEP_SIZE + i];
      }

     api_SendMsgFromRmToL1(length, buffer);
   }

   grrPowerValsConfigured++;
   
   DBG_LEAVE();
}


/*****************************************************************************
 *
 *  Module Name: ReadEeprom
 *  
 *  Purpose: Read Tx calibration tables and uplink amp characteristics from
 *           EEPROM on the RF board     
 *
 *****************************************************************************/
STATUS   ReadEeprom(void)
{
   STATUS         I2CStatus;
   unsigned char  PwrCalTable[PWR_TABLE_SIZE];
   unsigned char  *pwrcal_ptr = &PwrCalTable[0];  // pointless?
   unsigned char  radioEEpromDevAddr = 0x53;
   UINT8          MsgLen = 0;
   int            i, j = 0;

   DBG_FUNC("ReadEeprom",GRR_LAYER_DSP);
   DBG_ENTER();   

   I2Coperation(ON);
   I2CStatus = I2Cread(pwrcal_ptr, radioEEpromDevAddr, (unsigned char)PWR_TABLE_ADDR, (int)PWR_TABLE_SIZE);
   I2Coperation(OFF);   

   printf("After EEPROM read, I2CStatus = %d\n", I2CStatus);
   for ( i=0; i<PWR_TABLE_SIZE; i++ )
   {
      printf("%02x ", PwrCalTable[i]);
      if ( i%10 == 9 )
      {
         printf("\n");
      }
   }
   printf("\n");
   DBG_LEAVE();
}


/*****************************************************************************
 *
 *  Module Name: WriteEeprom
 *  
 *  Purpose: Write Tx calibration tables and uplink amp characteristics to
 *           EEPROM on the RF board     
 *
 *****************************************************************************/
STATUS   WriteEeprom(void)
{
   STATUS         I2CStatus;
   unsigned char  radioEEpromDevAddr = 0x53;
   UINT8          MsgLen = 0;
   int            i, k;
   FILE           *fp;
   int            pwrCalTable[PWR_TABLE_SIZE];
   UINT8          usByte;

   DBG_FUNC("WriteEeprom",GRR_LAYER_DSP);
   DBG_ENTER();   
   
   if ( fp = fopen("PowerCalTable.txt", "r") )
   {
   
      /*
      * Read in 1 table of DAC values vs. Static Power Level (step) 
      * Note: this assumes TX_STEP_TABLE_SIZE is 36, format 9 across
      */
      for ( i=0; i<4; i++ ) 
      {
         k=9*i;
         fscanf(fp, "%x %x %x %x %x %x %x %x %x",
         pwrCalTable + k,
         pwrCalTable + k + 1,
         pwrCalTable + k + 2,
         pwrCalTable + k + 3,
         pwrCalTable + k + 4,
         pwrCalTable + k + 5,
         pwrCalTable + k + 6,
         pwrCalTable + k + 7,
         pwrCalTable + k + 8 );
      }
      /*
      * Read in 2 tables of DAC values vs. Static Power Level (sweep) 
      * Note: this assumes TX_FREQ_SWEEP_SIZE is 15, format 15 across
      */
      for ( i=0; i<2; i++ )
      {
         k=15*i;
         fscanf(fp, "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
         pwrCalTable + TX_STEP_TABLE_SIZE + k,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 1,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 2,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 3,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 4,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 5,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 6,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 7,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 8,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 9,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 10,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 11,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 12,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 13,
         pwrCalTable + TX_STEP_TABLE_SIZE + k + 14 );
      }
      /*
      * Read in 2nd table of DAC values vs. Static Power Level (step) 
      * Note: this assumes TX_STEP_TABLE_SIZE is 36, format 9 across
      */
      for ( i=0; i<4; i++ ) 
      {
         k=9*i;
         fscanf(fp, "%x %x %x %x %x %x %x %x %x",
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 1,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 2,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 3,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 4,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 5,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 6,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 7,
         pwrCalTable + TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + k + 8 );
      }
      /*
      * Read in Amplifier Gain Characteristic 
      * Note: this assumes RX_GAIN_CHAR_SIZE is 6
      */
      fscanf(fp, "%x %x %x %x %x %x",
         pwrCalTable + 2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE,
         pwrCalTable + 2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + 1,
         pwrCalTable + 2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + 2,
         pwrCalTable + 2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + 3,
         pwrCalTable + 2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + 4,
         pwrCalTable + 2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + 5 );

      fclose(fp);

      /*
      * Append marker byte(s)
      */
      for ( i=0; i<END_MARKER_SIZE; i++ )
      {
         pwrCalTable[2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + RX_GAIN_CHAR_SIZE + i] = END_MARKER_VALUE;
      }

      I2Coperation(ON);
      /* EEPROM on radio board needs to be fed 1 byte at a time  */
      for ( i=0; i<PWR_TABLE_SIZE; i++ )
      {
         usByte = (UINT8)pwrCalTable[i]; 
         printf("%02x ", usByte);
         if ( i%10 == 9 )
         {
             printf("\n");
         }
         I2CStatus = I2Cwrite(&usByte, radioEEpromDevAddr, (unsigned char)i, 1);
         taskDelay(2);    /* Limit how fast the EEPROM gets the bytes */
      }
      I2Coperation(OFF); 
      printf("\nAfter EEPROM write, I2CStatus = %d\n", I2CStatus);
   }
   else
   {
      printf("\nError: Could not open PowerCalTable.txt\n");
   } 
        
   DBG_LEAVE();
}



/*****************************************************************************
 *
 *  Module Name: CheckEeprom
 *  
 *  Purpose: Read Tx calibration tables form EEPROM. If marker byte(s) missing,
 *           then (1) set output status to generate alarm, (2) construct table
 *           in Rel 1.5+ format by duplicating step table and appending uplink
 *           amplifier characteristics and marker, and (3) write table back
 *           to EEPROM.
 *
 *     NOTE: The EEPROM update portion of CheckEeprom is intended to catch up
 *           pre-FCS GP10s and pre-Rel-1.5 GP10s to the EEPROM convention used
 *           in Release 1.5 and beyond. If more parameters are added to EEPROM 
 *           in the future, then CheckEeprom is an ideal place to add updating
 *           code for the new format.
 *
 * Pre-FCS Format:
 *           36-byte step table, two 15-byte sweep tables
 *
 * Pre-Rel-1.5 Format: 
 *           36-byte step table, two 15-byte sweep tables,
 *           6-byte rx gain table, 1-byte marker
 *
 * Rel-1.5+ Format:
 *           one 36-byte step table, two 15-byte sweep tables,
 *           another 36-byte step table, 6-byte rx gain table, 1-byte marker
 *
 *****************************************************************************/
STATUS   CheckEeprom(void)
{
   STATUS         I2CStatus;
   unsigned char  pwrCalTable[PWR_TABLE_SIZE];
   unsigned char  *pwrcal_ptr = &pwrCalTable[0];  // pointless?
   unsigned char  radioEEpromDevAddr = 0x53;
   UINT8          usByte;
   int            i; 
   long           ais;
   int            foundMarkerNewFormat;
   int            foundMarkerOldFormat;
   int            writeToEeprom = FALSE;
   int            appendUplinkInfo = FALSE;
   STATUS  returnValue;
   /*--- Uplink amp characteristics for 900, 1800, 1900 as of 8/21/00 ---*/
   UINT8          gainChar[3][RX_GAIN_CHAR_SIZE] = {
                  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                  {0x0d, 0x80, 0x01, 0x90, 0x02, 0x80},
                  {0x0f, 0x0f, 0x00, 0xd0, 0x01, 0x80} };
   char           aisname[3][9] = { {"GSM-900"}, {"DCS-1800"}, {"PCS-1900"} };
   UINT8          defaultCalTable[PWR_TABLE_SIZE] = {   
                  0xd0,0xc8,0x54,0x43,0x37,0x22,0x32,0x22,0x23, 
                  0x23,0x33,0x24,0x34,0x43,0x55,0x44,0x54,0x54, 
                  0x44,0x34,0x33,0x42,0x33,0x23,0x32,0xc2,0x22, 
                  0x12,0x22,0x22,0x22,0x22,0x22,0x32,0x43,0x72, 
                  6, 6, 6, 7, 5, 5, 2, 0, 0, 2, 7, 9, 6, 5, 4,
                  5, 5, 5, 5, 5, 3, 0, 0, 0, 0, 5, 7, 9, 3, 2, 
                  0xd0,0xc8,0x54,0x43,0x37,0x22,0x32,0x22,0x23, 
                  0x23,0x33,0x24,0x34,0x43,0x55,0x44,0x54,0x54, 
                  0x44,0x34,0x33,0x42,0x33,0x23,0x32,0xc2,0x22, 
                  0x12,0x22,0x22,0x22,0x22,0x22,0x32,0x43,0x72, 
                  0x0d,0x80,0x01,0x90,0x02,0x80,0x2b };       

   DBG_FUNC("CheckEeprom",RM_LAYER);
   DBG_ENTER();   

   /*--------------------------------------------------------------------------
   *  Read power cal table from EEPROM
   *--------------------------------------------------------------------------*/
   I2Coperation(ON);
   I2CStatus = I2Cread(pwrcal_ptr, radioEEpromDevAddr, (unsigned char)PWR_TABLE_ADDR, (int)PWR_TABLE_SIZE);
   I2Coperation(OFF);   
   printf("After EEPROM read, I2CStatus = %d\n", I2CStatus);
   
   /*--------------------------------------------------------------------------
   *  Check whether marker byte(s) are already in EEPROM at end of table in
   *  Release 1.5+ Format and also whether marker byte(s) are present at 
   *  location used by earlier formats. 
   *--------------------------------------------------------------------------*/
   foundMarkerNewFormat = TRUE;
   foundMarkerOldFormat = TRUE;
   for ( i=0; i<END_MARKER_SIZE; i++ )
   {
      if ( pwrCalTable[TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + RX_GAIN_CHAR_SIZE + i] != END_MARKER_VALUE )
      {
         foundMarkerOldFormat = FALSE;
      }
   }
   for ( i=0; i<END_MARKER_SIZE; i++ )
   {
      if ( pwrCalTable[2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + RX_GAIN_CHAR_SIZE + i] != END_MARKER_VALUE )
      {
         foundMarkerNewFormat = FALSE;
      }
   }
   
   /*--------------------------------------------------------------------------
   *  Check first two values of first step table. Both must be larger than
   *  0x40 and in order of decreasing value (or equal). If not, copy default
   *  table and set flags to look up AIS, write to EEPROM and send alarm. 
   *--------------------------------------------------------------------------*/
   if ( pwrCalTable[0]>0x40 && pwrCalTable[1]>0x40 && pwrCalTable[0]>=pwrCalTable[1] )
   {
      returnValue = EEPROM_STATUS_OK;
   }
   else
   {
      for ( i=0; i<PWR_TABLE_SIZE; i++ )
      {
         pwrCalTable[i] = defaultCalTable[i];
      }
      appendUplinkInfo = TRUE;
      writeToEeprom = TRUE;
      returnValue = EEPROM_STATUS_REPORT_ERROR;
   }

   /*--------------------------------------------------------------------------
   *  If marker is correct for Release 1.5+ and not for earlier format, then
   *  no action is required. Otherwise, update EEPROM format by duplicating
   *  the step table. For details, see format description in function header.
   *  NOTE: If  markers are present for both formats, the GP could have been
   *  loaded with Release 1.5+, then reverted to an earlier release. In this
   *  case, we still need to update EEPROM. 
   *--------------------------------------------------------------------------*/
   if ( returnValue==EEPROM_STATUS_OK && (!foundMarkerNewFormat || foundMarkerOldFormat) )
   {
      appendUplinkInfo = TRUE;
      writeToEeprom = TRUE;
      returnValue = EEPROM_STATUS_UPDATE;
      /*--------------------------------------------------------------------------
      *  Duplicate existing step table to produce one for each Tx path
      *--------------------------------------------------------------------------*/
      for ( i=0; i<TX_STEP_TABLE_SIZE; i++ )
      {
         pwrCalTable[TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + i] = pwrCalTable[i];
      }
   }
   
   /*--------------------------------------------------------------------------
   *  If indicated, look up Air Interface Std (AIS) in MIB and append table
   *--------------------------------------------------------------------------*/
   if ( appendUplinkInfo )
   { 
      oam_getMibIntVar(MIB_gsmdcsIndicator, &ais);
      if ( ais < 0 )
      {
         returnValue = EEPROM_STATUS_REPORT_ERROR;
         printf("Error: MIB contains AIS=%d. Should be 0-2. Defaulting to 1 (DCS1800).\n", ais);
         ais = 1;
      }
      else if ( ais > 2 )
      {
         returnValue = EEPROM_STATUS_REPORT_ERROR;
         printf("Error: MIB contains AIS=%d. Should be 0-2. Defaulting to 1 (DCS1800).\n", ais);
         ais = 1;
      }
      printf("Updating EEPROM for %s system\n", aisname[ais]);
   
      /*--------------------------------------------------------------------------
      *  Append uplink amplifier information specific to AIS, plus marker byte(s)
      *--------------------------------------------------------------------------*/
      for ( i=0; i<RX_GAIN_CHAR_SIZE; i++ )
      {
         pwrCalTable[2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + i] = gainChar[ais][i];
      }
      for ( i=0; i<END_MARKER_SIZE; i++ )
      {
         pwrCalTable[2*TX_STEP_TABLE_SIZE + 2*TX_FREQ_SWEEP_SIZE + RX_GAIN_CHAR_SIZE + i] = END_MARKER_VALUE;
      }
   }
   
   /*--------------------------------------------------------------------------
   *  If indicated, write power cal table to EEPROM and report values to shell
   *--------------------------------------------------------------------------*/
   if ( writeToEeprom )
   {
      I2Coperation(ON);
      /*--EEPROM on radio board needs to be fed 1 byte at a time--*/
      for ( i=0; i<PWR_TABLE_SIZE; i++ )
      {
         usByte = (UINT8)pwrCalTable[i]; 
         printf("%02x ", usByte);
         if ( i%10 == 9 )
         {
            printf("\n");
         }
         I2CStatus = I2Cwrite(&usByte, radioEEpromDevAddr, (unsigned char)i, 1);
         taskDelay(2);    /* Limit how fast the EEPROM gets the bytes */
      }
      I2Coperation(OFF);
      printf("\nAfter EEPROM write, I2CStatus = %d\n", I2CStatus);
   }  
   DBG_LEAVE();
   
   return(returnValue);
}



#endif /* __GRR_RADIOCTRL_CPP__ */
