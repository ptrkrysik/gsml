
/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : AlarmTask.h                                         */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 07/11/99                                            */
/* Description : This file contains routines to perform disk I/O to  */
/*               save alarm to the disk file.                        */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                            
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |09/28/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/
#include "AlarmTask.h"

#include "dirent.h"
#include "stat.h"
#include "time.h"
#include "string.h"
static unsigned long  CyclicFileRecordIndex = 0;

STATUS AlarmTask::saveAlarm(const char* msg)
{
	FILE *alarmfd;
	
	alarm_fenter("AlarmTask::saveAlarm");
	
	// Opens for reading and writing; 
    alarmfd = fopen(_logfilename, "r+");
	if (alarmfd == NULL)
	{
		alarm_error(("Could not open alarm log file %s (Reason: %s)\n", _logfilename, strerror(errno)));
		alarm_fexit();
		return ERROR;
	}

    long fpos = ALARM_record_length*CyclicFileRecordIndex++;
    CyclicFileRecordIndex = CyclicFileRecordIndex%ALARM_max_records;

    fseek(alarmfd, fpos, SEEK_SET);
    
    fputs(msg, alarmfd);
	fclose(alarmfd);
	alarm_debug(("Alarm Message successfully saved\n"));
	alarm_fexit();
    return OK;
}

STATUS AlarmTask::renameOldLogFile()
{	
	struct stat   fileStat;

	alarm_fenter("alarm_renameOldLogFile");
	
	/* check if old Alarrm log file exists */
	if (stat (_logfilename, &fileStat) == OK)
	{
		/* The file exist */
		struct tm localTime;
		struct tm *ltime = &localTime;
		time_t  longTime;
		char *pdest;
		char newFileName[ALARM_logFileMaxSize];
		
		longTime = time(NULL);
		
		ltime = localtime(&longTime);
		
		strcpy(newFileName, _logfilename);

		/* Assumes that file name is long enough */
		pdest = strrchr(newFileName, ALARM_dirChar);
		if( pdest != NULL )
		{
			sprintf(pdest+1, "AlarmLogAsOf_%d-%d-%d,%d:%d", ltime->tm_mon+1, ltime->tm_mday, 
				ltime->tm_year+1900,ltime->tm_hour, ltime->tm_min);
		} else {
			sprintf(newFileName, "AlarmLogAsOf_%d-%d-%d,%d:%d", ltime->tm_mon+1, ltime->tm_mday, 
				ltime->tm_year+1900,ltime->tm_hour, ltime->tm_min);
		}
		alarm_debug(("Renaming %s file to %s\n", _logfilename, newFileName));
		if (rename(_logfilename, newFileName) == ERROR)
		{
			
			/* rename failed */
			FILE *alarmfd;

			alarm_error(("Could not rename old file (Reason: %s)\n", strerror(errno))); 
			alarm_warning(("Truncating the old alarm file: %s\n", _logfilename));
			
			alarmfd = fopen(_logfilename, "w");
			if (alarmfd == NULL)
			{
				alarm_error(("Could not truncate file(Reason: %s)\n",strerror(errno)));
			} else {
				fclose(alarmfd);
			}
			
		} else {
			alarm_debug(("Old alarm file successfully renamed to %s\n", newFileName));
		}
		
	} else {
		alarm_warning(("Old alarm log file \"%s\" does not exist.\n", _logfilename));
	}
	
	alarm_fexit();
	return OK;

}


STATUS AlarmTask::getLastId(unsigned long *id)
{
    FILE *alarmfd;
   	struct stat   statBuff; 
    alarm_fenter("AlarmTask::getLastId");
    STATUS status;
    
    
    status = stat(_logfilename, &statBuff);
    
    
    if (  status == ERROR  // file does not exist
        || statBuff.st_size == 0                                    // newly created
        || statBuff.st_size % ALARM_record_length != 0              // corrupt file
        || statBuff.st_size > ALARM_max_records*ALARM_record_length // corrupt file
        )
    {
        // if size is a zero, there was no alarm file
        *id = 1;
        CyclicFileRecordIndex = 0;
        alarmfd = fopen(_logfilename, "w");   // truncate file
        if (alarmfd != NULL)
            fclose(alarmfd);
        else 
        {
            alarm_error(("Could not create %s for writing: %s\n", _logfilename, strerror(errno)));
        }
        
    } else {
        
        alarm_debug(("Current Alarm disk file size = %d\n", statBuff.st_size));
        
        if (statBuff.st_size < ALARM_max_records*ALARM_record_length)
        {
            
            // the size less then max size
            CyclicFileRecordIndex = statBuff.st_size / ALARM_record_length;
            *id = CyclicFileRecordIndex+1;
        } else {
            unsigned long id1, id2, i;
            
            alarmfd= fopen(_logfilename, "r");
            if (alarmfd == NULL)
            {
                alarm_error(("Cannot open() Alarm disk file %s to read (Reason: %s)\n", _logfilename, strerror(errno)));	
                *id =1;
                CyclicFileRecordIndex = 0;
                status = ERROR;
            } else {
                // file is ALARM_max_records*ALARM_record_length size
                fseek(alarmfd, 0, SEEK_SET);
                fscanf(alarmfd, "%u", &id1);
                for(i=1; i< ALARM_max_records; i++)
                {
                    fseek(alarmfd, i*ALARM_record_length, SEEK_SET);       
                    fscanf(alarmfd, "%u", &id2);
                    if(id1> id2)
                    {
                        break;
                    }  else {
                        id1 = id2;
                    }
                }
                CyclicFileRecordIndex = i % ALARM_max_records;
                *id = id1+1; 
            }
        }
    }
    alarm_fexit();
    return status;
}
