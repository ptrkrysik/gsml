/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/********************************************************************
*                          
*  FILE NAME:	CPU_Mon.c    
*                      
*  DESCRIPTION: This file contains routines for the DS1780.               
*                                  
*  NOTES:                              
*                                  
*  VERSION:  1.0  10/06/99  14:45:15
*
*  SCCS ID:  "@(#)CPU_Mon.c"
*
********************************************************************/

#include <vxWorks.h> 
#include "CDC_BSP/i2c.h"
#include "cdcUtils/CPU_Mon_IF.h"

/* if defined it will include test modules. */
#define DS1780_TEST  


/************* Static Globals *****************/

static  UINT8	Chassis_Intrusion		= 0x00;

static  UINT8	DS1780_ADDR				= 0x2C;
static  int		_1_CHAR					= 0x01;
 
static	UINT8	CONFIG_REG			    = 0x40;
static	UINT8	INIT_REGS				= 0x80;
static	UINT8	MONITOR_MODE			= 0x01;

static  UINT8	INTR_STAT_1_REG			= 0x41;
static  UINT8	INTR_1_STATUS_MASK		= 0x1D;

static  UINT8	INTR_STAT_2_REG			= 0x42;
static  UINT8	INTR_2_STATUS_MASK		= 0x10;

static  UINT8	INTR_1_MASK_REG			= 0x43;
static  UINT8	INTR_1_MASK				= 0xC2;

static	UINT8	INTR_2_MASK_REG			= 0x44;
static	UINT8	INTR_2_MASK				= 0x03;

static	UINT8	CHASSIS_INTR_CLR_REG	= 0x46;
static	UINT8	CLR_CHASSIS_INTR		= 0x80;

static	UINT8	SER_ADDR_REG			= 0x48;
static	UINT8	DEFAULT_ADDR			= 0x2C;

static	UINT8	TEMP_CFG_REG			= 0x4B;
static	UINT8	DISABLE_INTRS			= 0x01;
static  UINT8	DEFAULT_INTR_MODE		= 0x00;
static	UINT8	NINTH_TEMP_BIT			= 0x80;

static	UINT8	_2_5_V_Thres_ID			= 0x01;
static	UINT8	_1_7_VOLTS				= 0x82;
static	UINT8	_2_0_VOLTS				= 0x99;
static	UINT8	_3_3_V_Thres_ID			= 0x02;
static	UINT8	_3_1_VOLTS				= 0xB4;
static	UINT8	_3_5_VOLTS				= 0xCB;
static	UINT8	_5_V_Thres_ID			= 0x04;
static	UINT8	_4_75_VOLTS				= 0xB6;
static	UINT8	_5_2_VOLTS				= 0xC7;

/***** Value RAM Definitions ******************/

static	UINT8	_2_5_V_READING_ADDR		= 0x20;
static  UINT8	_3_3_V_READING_ADDR		= 0x22;
static  UINT8	_5_V_READING_ADDR		= 0x23;

static	UINT8	MSB_TEMP_READING_ADDR	= 0x27;

static	UINT8	_2_5_HIGH_LIMIT_ADDR	= 0x2B;
static	UINT8	_2_5_LOW_LIMIT_ADDR		= 0x2C;
static	UINT8	_3_3_HIGH_LIMIT_ADDR	= 0x2F;
static	UINT8	_3_3_LOW_LIMIT_ADDR		= 0x30;
static	UINT8	_5_HIGH_LIMIT_ADDR		= 0x31;
static	UINT8	_5_LOW_LIMIT_ADDR		= 0x32;

static	UINT8	HOT_TEMP_HIGH_ADDR		= 0x39;
static	UINT8	_0_DEGREES				= 0x00;
static	UINT8	_45_DEGREES				= 0x2D;
static	UINT8	HOT_TEMP_HYST_LOW_ADDR	= 0x3A;

static	UINT8	CO_ID_NUMBER_ADDR		= 0x3E;
static	UINT8	STEPPING_ID_ADDR		= 0x3F;
 

/********** Local Prototypes ******************/

STATUS	DS1780_Poll(void);
STATUS  DS1780_Init(void);
STATUS	DS1780_Intr_Setup(void);
STATUS  DS1780_Init_Thresholds(void);
STATUS  DS1780_Set_Voltage_Thresholds(UINT8	Threshold_ID, UINT8 High, UINT8 Low);
STATUS  DS1780_Set_Temp_Thresholds(UINT8 High, UINT8 Low);
STATUS	DS1780_I2C_Wr(unsigned char *outch, unsigned char devAddr, 
					  unsigned char  wAddr, int numChars);
STATUS	DS1780_I2C_Rd(unsigned char *outch, unsigned char devAddr, 
					  unsigned char  wAddr, int numChars);

#ifdef DS1780_TEST

STATUS	DS1780_Rd_Regs(void);
STATUS	DS1780_Rd_Loc(unsigned char  Addr);
STATUS	DS1780_Wr_Loc(unsigned char  Addr, unsigned char Value);
STATUS	DS1780_Test_Read(void);

#endif

	
/*****************************************************************************
 *
 *  Module Name: DS1780_Init
 *  
 *  Purpose:     Initializes the DS1780 and sets it to its operating mode.
 *
 *****************************************************************************/

STATUS DS1780_Init(void)
{
	STATUS		Result;

	Result = DS1780_I2C_Wr(&INIT_REGS, DS1780_ADDR, CONFIG_REG, _1_CHAR);

	if ( Result == OK )
	{
		Result = DS1780_Intr_Setup();
		
		if ( Result == OK )
		{
			Result = DS1780_Init_Thresholds();

			if ( Result == OK )
			{
				Result   = DS1780_I2C_Wr(&MONITOR_MODE, DS1780_ADDR, 
						                  CONFIG_REG,   _1_CHAR);
			}
		}
	}

	return(Result);
}


/*****************************************************************************
 *
 *  Module Name: DS1780_Poll
 *  
 *  Purpose:     Routine to poll the status of the DS1780 measurements.
 *
 *****************************************************************************/

STATUS	DS1780_Poll(void)
{
	STATUS	Result;  
	UINT8	Value;

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, INTR_STAT_1_REG, _1_CHAR);

	if ( Result == OK )
	{
		if ( (Value & INTR_1_STATUS_MASK) != OK )
			Result = ERROR;
		else
		{
			Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, INTR_STAT_2_REG, _1_CHAR);

			if ( (Value & INTR_2_STATUS_MASK) != OK )
			{
				Chassis_Intrusion = INTR_2_STATUS_MASK;

				Result = ERROR;
			}
		}
	}

	return(Result);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Read
 *  
 *  Purpose:     Reads the monitored values of the DS1780 Measurements.
 *
 *****************************************************************************/

STATUS	DS1780_Read(t_DS1780 *data)
{
	STATUS	Result;
	UINT8	Value;

	Result = DS1780_I2C_Rd(&data->_2_5_V_Mon_Value, DS1780_ADDR, _2_5_V_READING_ADDR, _1_CHAR);

	if ( Result == OK )
	{
		Result = DS1780_I2C_Rd(&data->_3_3_V_Mon_Value, DS1780_ADDR, _3_3_V_READING_ADDR, _1_CHAR);
	
		if ( Result == OK )
		{
			Result = DS1780_I2C_Rd(&data->_5_V_Mon_Value,   DS1780_ADDR, _5_V_READING_ADDR,   _1_CHAR);

			if ( Result == OK )
			{
				Result = DS1780_I2C_Rd(&data->Temp_Mon_Value, DS1780_ADDR, MSB_TEMP_READING_ADDR, _1_CHAR);
	
				if ( Result == OK )
				{
					data->Chassis_Intrusion_Status = Chassis_Intrusion;
					Chassis_Intrusion			   = OK;
				}
			}
		}
	}

	return(Result);
}


/*****************************************************************************
 *
 *  Module Name: DS1780_Intr_Setup
 *  
 *  Purpose:     Initializes the DS1780 voltage and temperature thresholds.
 *
 *****************************************************************************/

STATUS	DS1780_Intr_Setup(void)
{
	STATUS	Result;

	Result = DS1780_I2C_Wr(&INTR_1_MASK, DS1780_ADDR, INTR_1_MASK_REG, _1_CHAR);

	if ( Result == OK ) 
	{
		Result = DS1780_I2C_Wr(&INTR_2_MASK, DS1780_ADDR, INTR_2_MASK_REG, _1_CHAR);

		if ( Result == OK )    
			Result = DS1780_I2C_Wr(&DEFAULT_INTR_MODE, DS1780_ADDR, TEMP_CFG_REG, _1_CHAR);
	}

	return(Result);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Init_Thresholds
 *  
 *  Purpose:     Initializes the DS1780 voltage and temperature thresholds.
 *
 *****************************************************************************/

STATUS  DS1780_Init_Thresholds(void)
{
	STATUS	Result;

	Result = DS1780_Set_Voltage_Thresholds(_2_5_V_Thres_ID, _2_0_VOLTS, _1_7_VOLTS);

	if ( Result == OK )
	{
		Result = DS1780_Set_Voltage_Thresholds(_3_3_V_Thres_ID, _3_5_VOLTS, _3_1_VOLTS);

		if ( Result == OK )
				
			Result = DS1780_Set_Voltage_Thresholds(_5_V_Thres_ID, _5_2_VOLTS, _4_75_VOLTS);
	}

	if ( Result == OK )
		
		Result = DS1780_Set_Temp_Thresholds( _45_DEGREES, _0_DEGREES);

	return(Result);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Set_Voltage_Thresholds
 *  
 *  Purpose:     Sets the DS1780 voltage threshold.
 *
 *****************************************************************************/

STATUS  DS1780_Set_Voltage_Thresholds(UINT8	Threshold_ID, UINT8 High, UINT8 Low)
{
	STATUS		Result = OK;
	UINT8		wHiAddr, wLoAddr;

	if ( Threshold_ID == _2_5_V_Thres_ID )
	{
		wHiAddr = _2_5_HIGH_LIMIT_ADDR;
		wLoAddr = _2_5_LOW_LIMIT_ADDR;
	}
	else if ( Threshold_ID == _3_3_V_Thres_ID )
	{
		wHiAddr = _3_3_HIGH_LIMIT_ADDR;
		wLoAddr = _3_3_LOW_LIMIT_ADDR;
	}
	else if ( Threshold_ID == _5_V_Thres_ID )
	{
		wHiAddr = _5_HIGH_LIMIT_ADDR;
		wLoAddr = _5_LOW_LIMIT_ADDR;
	}
	else 
		Result = ERROR;


	if ( Result == OK )
	{
		Result = DS1780_I2C_Wr( &High, DS1780_ADDR, wHiAddr, _1_CHAR );

		if ( Result == OK )

			Result = DS1780_I2C_Wr( &Low, DS1780_ADDR, wLoAddr, _1_CHAR );
	}

	return(Result);
}


/*****************************************************************************
 *
 *  Module Name: DS1780_Set_Temp_Thresholds
 *  
 *  Purpose:     Sets the DS1780 temperature thresholds.
 *
 *****************************************************************************/

STATUS  DS1780_Set_Temp_Thresholds(UINT8 High, UINT8 Low)
{
	STATUS	Result;

	Result = DS1780_I2C_Wr( &High, DS1780_ADDR, HOT_TEMP_HIGH_ADDR, _1_CHAR );

	if ( Result == OK )

		Result = DS1780_I2C_Wr( &Low, DS1780_ADDR, HOT_TEMP_HYST_LOW_ADDR, _1_CHAR );

	return(Result);

}


/*****************************************************************************
 *
 *  Module Name: DS1780_Read_Company_Id
 *  
 *  Purpose:     Reads the DS1780 Company I.D. value.
 *
 *****************************************************************************/

STATUS	DS1780_Read_Company_Id(unsigned char *outch)
{
	STATUS		Result;

	Result = DS1780_I2C_Rd(outch, DS1780_ADDR, CO_ID_NUMBER_ADDR, _1_CHAR);

	return(Result);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Stepping_Id
 *  
 *  Purpose:     Reads the DS1780 Stepping I.D. value.
 *
 *****************************************************************************/

STATUS	DS1780_Stepping_Id(unsigned char *outch)
{
	STATUS		Result;

	Result = DS1780_I2C_Rd(outch, DS1780_ADDR, STEPPING_ID_ADDR, _1_CHAR);

	return(Result);
}


/*****************************************************************************
 *
 *  Module Name: DS1780_I2C_Rd
 *  
 *  Purpose:     Reads data from the DS1780 via I2C bus.
 *
 *****************************************************************************/

STATUS	DS1780_I2C_Rd(unsigned char *outch, unsigned char devAddr, 
					  unsigned char  wAddr, int numChars)
{
	STATUS	Results;
    int i;

	I2Coperation(ON); 

    for (i = 0, Results = ERROR; (i < 10) && (Results == ERROR); i++)
    {    /* if error try muliple times since I2Cread can fail occasionally */
      Results = I2Cread(outch, devAddr, wAddr, numChars);
      taskDelay(2);
    }

	I2Coperation(OFF); 

	return(Results); 
}


/*****************************************************************************
 *
 *  Module Name: DS1780_I2C_Wr
 *  
 *  Purpose:     Writes data to the DS1780 via I2C bus.
 *
 *****************************************************************************/

STATUS	DS1780_I2C_Wr(unsigned char *outch, unsigned char devAddr, 
					  unsigned char  wAddr, int numChars)
{
	STATUS	Results;

	I2Coperation(ON);

	Results = I2Cwrite(outch, devAddr, wAddr, numChars);

	I2Coperation(OFF);

	return(Results);
}


/*****************************************************************************
 *
 *      T E S T   C O D E
 *
 *****************************************************************************/

#ifdef DS1780_TEST

/*****************************************************************************
 *
 *  Module Name: DS1780_Rd_Regs
 *  
 *  Purpose:     Test Module to read the DS1780 registers.
 *
 *****************************************************************************/

STATUS	DS1780_Rd_Regs(void)
{
	STATUS			Result;
	unsigned char   Value;

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, CONFIG_REG, _1_CHAR);
	printf("CONFIG_REG = %d\n", Value);	

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, INTR_STAT_1_REG, _1_CHAR);
	printf("INTR_STAT_1_REG = %d\n", Value);	

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, INTR_STAT_2_REG, _1_CHAR);
	printf("INTR_STAT_2_REG = %d\n", Value);

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, INTR_1_MASK_REG, _1_CHAR);
	printf("INTR_1_MASK_REG = %d\n", Value);

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, INTR_2_MASK_REG, _1_CHAR);
	printf("INTR_2_MASK_REG = %d\n", Value);

	return(Result);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Rd_Loc
 *  
 *  Purpose:     Test Module to read the DS1780 memory locations.
 *
 *****************************************************************************/

STATUS	DS1780_Rd_Loc(unsigned char  Addr)
{
	STATUS			Result;
	unsigned char   Value;

	Result = DS1780_I2C_Rd(&Value, DS1780_ADDR, Addr, _1_CHAR);

	printf("Value = %d\n", Value);

	return(Result);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Wr_Loc
 *  
 *  Purpose:     Test Module to write the DS1780 locations.
 *
 *****************************************************************************/

STATUS	DS1780_Wr_Loc(unsigned char  Addr, unsigned char Value)
{
	STATUS		Result;

	Result = DS1780_I2C_Wr(&Value, DS1780_ADDR, Addr, _1_CHAR);

	return(Result);
}


/*****************************************************************************
 *
 *  Module Name: DS1780_get1_8_V
 *  
 *  Purpose:     Returns 1.8 Voltage status
 *
 *****************************************************************************/

float DS1780_get1_8_V(void)
{
	t_DS1780	data;

	DS1780_Read(&data);

   return  (float)((2.5 * (float)data._2_5_V_Mon_Value)/192);
}
/*****************************************************************************
 *
 *  Module Name: DS1780_get3_3_V
 *  
 *  Purpose:     Returns 3.3 Voltage status.
 *
 *****************************************************************************/

float DS1780_get3_3_V(void)
{
	t_DS1780	data;

	DS1780_Read(&data);

   return (float)((3.3 * (float)data._3_3_V_Mon_Value)/192);
}
/*****************************************************************************
 *
 *  Module Name: DS1780_get5_0_V
 *  
 *  Purpose:     returns 5 voltage status
 *
 *****************************************************************************/

float DS1780_get5_0_V(void)
{
	t_DS1780	data;
	DS1780_Read(&data);

   return (float)((5.0 * (float)data._5_V_Mon_Value)/192);
}

/*****************************************************************************
 *
 *  Module Name: DS1780_getCdcTemp
 *  
 *  Purpose:     return cdc temperature.
 *
 *****************************************************************************/

int DS1780_getCdcTemp(void)
{
	t_DS1780	data;

	DS1780_Read(&data);

   return data.Temp_Mon_Value;
}

/*****************************************************************************
 *
 *  Module Name: DS1780_Test_Read
 *  
 *  Purpose:     Test the DS1780_Read Module.
 *
 *****************************************************************************/

STATUS	DS1780_Test_Read(void)
{
	STATUS		Results;
	t_DS1780	data;

	Results = DS1780_Read(&data);

	printf("2.5_V_Mon_Value = %d", data._2_5_V_Mon_Value);
	printf(" = %f volts\n", ((2.5 * (float)data._2_5_V_Mon_Value)/192));

	printf("3.3_V_Mon_Value = %d", data._3_3_V_Mon_Value);
	printf(" = %f volts\n", ((3.3 * (float)data._3_3_V_Mon_Value)/192));

	printf("5 V_Mon_Value   = %d", data._5_V_Mon_Value);
	printf(" = %f volts\n", ((5 * (float)data._5_V_Mon_Value)/192));

	printf("Temp_Mon_Value = %d degrees C\n", data.Temp_Mon_Value);

	printf("Chassis_Intrusion_Status = %d\n", data.Chassis_Intrusion_Status);

	return(Results);
}

#endif
