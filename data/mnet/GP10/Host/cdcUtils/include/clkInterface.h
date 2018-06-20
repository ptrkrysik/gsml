/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*---- Function Declarations ----*/

/**********************************************************************
clkSerialNumReturn  -  Serial Number Return

Returns the Serial Number of the clock module.
*/
char* clkSerialNumReturn();


/**********************************************************************
clkSoftVersionReturn  -  Software Version Return

Returns the Software Version of the clock module.
*/
char* clkSoftVersionReturn();


/**********************************************************************
clkStatGet - Status Get

Sends a command to clock module to read the status. 
Returns the status or -1 for error. 

Thre returned status is:

     0 - No Alarm conditions
     1 - Burst Alarm Active
     2 - Frame Alarm Active
     3 - Borth Burst and Fram Alarms active
*/
int clkStatGet();         /* RETURN: status or -1 for error */


/**********************************************************************
clkNumDaysTuneGet  -  Number of Days Tune Get

Sends a command to clock module to read the number of days since last tuneup. 
Returns the no. of days or -1 for error.
*/
int clkNumDaysTuneGet();       /* RETURN: number of days or -1 for error */


/**********************************************************************
clkNumDaysRunGet  -  Number of Days Run Get

Sends a command to clock module to read the number of days since it is running. 
Returns the no. of days or -1 for error. 
*/

int clkNumDaysRunGet();          /* RETURN: number of days or -1 for error */


/**********************************************************************
clkGetDateOfLastTuning  -  Get the date of the last tuning of the clock

Sends a command to clock module to read the last tuning date.
Returns OK or ERROR. 
*/

STATUS clkGetDateOfLastTuning(unsigned char *month, unsigned char *day, unsigned short *year);
