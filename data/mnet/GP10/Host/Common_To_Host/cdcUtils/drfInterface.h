/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/* drf status bits */
#define EEPROM_REQUIRED      0x8000     /* EEPROM contents required */
#define EEPROM_DEFAULT_USED  0x4000     /* Default EEPROM values in use */
#define FPGA_SYNC_LOST       0x2000     /* FPGA sychronization lost */
#define PLL_IF_LOCK          0x0080     /* Intermediate Frequencey PLL Lock Detect */
#define PLL_2B_LOCK          0x0040     /* Transceiver 2B PLL Lock Detect */
#define PLL_2A_LOCK          0x0020     /* Transceiver 2A PLL Lock Detect */
#define PLL_1_LOCK           0x0010     /* Transceiver 1 PLL Lock Detect */
#define PLL_REF_LOCK         0x0008     /* Reference Frequency PLL Lock Detect */
#define PLL_MON_LOCK         0x0004     /* Monitor Frequency PLL Lock Detect */


typedef enum
{
  Mon, 
  Ref, 
  T1, 
  T2A,
  T2B,
  IF
}PLLType;


extern STATUS drfPllRefFreqSet    /* RETURN : OK, ERROR */
  (
    unsigned int freq   /* IN : the freq to set */
  );

extern int drfPllRefFreqGet();    /* RETURN : 0 - 10Mhz, 1 - 13Mhz, -1 - Error */

extern STATUS drfNullAttenSet  /* RETURN : OK, ERROR */
  (
    int attenLevel      /* IN : attenuation Level * 10 */
  );

extern STATUS drfNullAttenGet  /* RETURN : OK, ERROR */
  (
    int* level          /* OUT : Attenuator level * 10 */
  );

extern STATUS drfNullAttenMin();  /* RETUEN : OK, ERROR */

extern int drfLockStatGet     /* RETUEN : 0 - not locked, 1 - locked, -1 - error */
  (
    PLLType pll        /* IN : pll type */
  );

extern int drfRSSIGet();     /* RETURN : RSSI or -1 for error */

extern STATUS drfLoopbackSet  /* RETURN : OK, ERROR */
  (
    int txNum,       /* IN : transmitter, 0 or 1  */
    int lowHigh      /* IN : 0 - set low, !0 - set high */
  );

extern STATUS drfTxPwrSet     /* RETURN : OK, ERROR */
  (
    int txNum,         /* IN : transmitter, 0 or 1  */
    int lowHigh        /* IN : 0 - set low, !0 - set high */
  );

extern STATUS drfFpgaTxPwrEnable     /* RETURN : OK, ERROR */
  (
    int txNum                 /* IN : transmitter, 0 or 1  */
  );

extern STATUS drfFpgaTxPwrDisable     /* RETURN : OK, ERROR */
  (
    int txNum                 /* IN : transmitter, 0 or 1  */
  );

extern STATUS drfFreqHopEnable();     /* RETURN : OK, ERROR */

extern STATUS drfFreqHopDisable();     /* RETURN : OK, ERROR */

extern STATUS drfStatGet                 /* RETURN: OK ro ERROR */
                  ( 
                   int* pStatus   /* OUT: drf status here */
                  );

extern char* drfVersionReturn();

extern char* drfSerialNumReturn();

extern int drfTypeGet();       /* RETUEN: 0,1,2, or -1 */



extern int drfFreqHopGet        /* RETURN: TRUE /FALSE */
  (
    UINT  trx            /* IN: trx number */
  );

