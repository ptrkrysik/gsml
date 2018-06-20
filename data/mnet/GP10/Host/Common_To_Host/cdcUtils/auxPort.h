/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

#ifndef AUXPORT_H
#define AUXPORT_H

/* Prototypes for AuxPort Functions */
void auxPortOutSet
(
  int val,           /* value to write to aux out port */
  int mask           /* bit mask with which to set the value */
);


UINT auxPortOutGet(void);


/* Output Auxillary port bit definitionas */

#define CF_RESET		0x80000000
#define LED1_OFF		0x40000000	/* led 1 active low */
#define LED2_OFF		0x20000000	/* led 2 active low */
#define LED3_OFF		0x10000000	/* led 3 active low */
#define FAULT_LED_OFF		0x08000000	/* fault led active low */
#define NET_ID_MASK		0x06000000

#define RF1_EN			0x00800000	/* RF1 enable */	
#define RF2_EN			0x00400000	/* RF2 enable */
#define GSM_SYNC1		0x00200000
#define GSM_SYNC2		0x00100000

#define WATCH_DOG		0x00040000
#define GPS_RESET		0x00020000
#define RF_RESET_NOT		0x00010000


#define EXT_RST			0x00002000

#define DSPA_RST_NOT		0x00000800	/* 0 - DSPA reset */
#define DSPB_RST_NOT		0x00000400      /* 0 - DSPB reset */
#define CPU2BBA1		0x00000200
#define CPU2BBA2		0x00000100
#define FPGA_DCLK		0x00000080	/* clock for FPGA download */
#define FPGA_DATA		0x00000040	/* data for FPGA download */		
#define FPGA_NCONFIG		0x00000020	/* signal for FPGA download */
#define CDC_POWER_OFF           0x00000010      /* 1 - cdc power off */
#define LED5_ON                 0x00000008      /* led 5 active high */
#define CPU2BBB1		0x00000004	/* signal 1 to base band FPGA */
#define CPU2BBB2		0x00000002	/* signal 2 to base band FPGA */
#define SEL_DEBUG		0x00000001	/* SMC2 use: 0 - GPS, 1 - debug */

/* Input Auxillary port bit definitionas */

#define SYNTH1_LOCKED		0x80000000	/* synthesizer1 locked */
#define SYNTH2_LOCKED		0x40000000	/* synthesizer1 locked */
#define RF1_PRESENT		0x20000000	/* RF Module 1 present */
#define RF2_PRESENT		0x10000000	/* RF Module 2 present */
#define LIU_ID_MASK		0x0E000000
#define FPGA1A			0x01000000	/* signal 1 from fpga 1 */
#define FPGA2A			0x00800000	/* signal 2 from fpga 1 */
#define FRDY			0x00400000	/* Flash Ready */
#define BCF_CD1			0x00100000	/* compactFlash card detect 1 */
#define BCF_CD2			0x00080000	/* compactFlash card detect 1 */
#define BCF_RDY			0x00040000	/* compactFlash ready */
#define FPGA1B			0x00020000	/* signal 1 from fpga 2 */
#define FPGA2B			0x00010000	/* signal 2 from fpga 2 */



#endif /* AUXPORT_H */
