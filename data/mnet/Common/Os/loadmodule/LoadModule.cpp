// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : LoadModule.cpp
// Author(s)   : Kevin Lim
// Create Date : 04-14-99
// Description : Functions to load modules
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "vxWorks.h"
#include "dosFsLib.h"
#include "loadLib.h"
#include "ioLib.h"
#include "stdio.h"
#include "string.h"

#ifdef GMC_VOB_SPECIFIC
#include <oam_api.h>
#include <vipermib.h>
#endif /* GMC_VOB_SPECIFIC */

#define UNIX_FILE_EOFS

#ifdef UNIX_FILE_EOFS
#define EOL_TRUNC_LEN	1
#else
#define EOL_TRUNC_LEN	2
#endif

// *******************************************************************
int viperLoad(char * filename)
{
    int fd;
    MODULE_ID	mId;
    
#ifdef NEVER // changed from GMC_VOB_SPECIFIC by oleg
    Ss7Type*    networkType;
    char*       fn;

    if ((fn = strstr(filename, "gmccpapp.out"))
         != NULL)
      {
        if( NULL != ( networkType = (Ss7Type*)oam_getMibAddress( MIB_ss7NetworkType ) ))
          {
            
            if (*networkType == ansi)
              {
                printf ("Mib returned ansi for loading.\n");
                strcpy(fn, "gmccpapp-ansi.out");
              }
            else
              {
                printf ("Mib returned itu for loading.\n");
                strcpy(fn, "gmccpapp-itu.out");
              }
          }
        else
          {
            printf ("Major Error reading network type from the Mib. Cannot load GMC CP.\n");
            return (-1);
          }
      }

#endif /* GMC_VOB_SPECIFIC */

    printf("Loading %s\n", filename);
    fd = open (filename, 0, 0);
    if(fd == ERROR)
    {
        // error on open object file
        printf("[loadMod] Fatal Error on fopen(%s) return ERROR\n", filename);
        return ERROR;
    }
    mId = loadModule (fd, LOAD_ALL_SYMBOLS);
    if(mId == NULL)
    {
        // error on loading module
        printf("[loadMod] Fatal Error on loadModule() return NULL\n");
        return (-1);
    }
    close (fd);
    return (int)mId;
}

// *******************************************************************
#define VIPER_MAX_BASE_PREFIX		100
#define VIPER_MAX_MODULE_LINE		180
// *******************************************************************
extern "C"
{
void viperLoadList(char * filename, char * base, int append)
{
	char base_line[VIPER_MAX_MODULE_LINE];
	char line[VIPER_MAX_MODULE_LINE];
	FILE * fd;
	char * lp;
	int base_len = strlen(base);
	int i, line_len;
	bool done = false;
	
	if(base_len > VIPER_MAX_BASE_PREFIX){
		printf("[viperLoadList] base string too long %d\n", base_len);
		return;
	}
	else{
		for(i=0; i<base_len; i++) base_line[i] = base[i];
		base_line[i]='/';
		base_len++;
	}

	if(append){
		int filename_len = strlen(filename);
		if((base_len+filename_len)>VIPER_MAX_MODULE_LINE){
				printf("[viperLoadList] base + module list name too long: %d\n", base_line+filename_len);
				printf("%s%s\n", base_line, filename);
				return;
		}
		for(i=0; i<filename_len; i++) base_line[base_len+i] = filename[i];
		// mark null
		base_line[base_len+i] = '\0';
		fd = fopen(base_line, "r");
		if(fd == NULL){ 
			printf("[viperLoadList] Error on openning module list file: %s\n", base_line); 
			return; 
		}
		else{
			printf("[viperLoadList] Loading modules from: %s\n", base_line);
		}
	}
	else{
		fd = fopen(filename, "r");
		if(fd == NULL){ 
			printf("[viperLoadList] Error on openning module list file: %s\n", filename); 
			return; 
		}
		else{
			printf("[viperLoadList] Loading modules from: %s\n", filename);
		}
	}

	while(!done){
		lp = fgets(line, 80, fd);
		if(lp == NULL){
			done = true;
		}
		else{ // parse the line
			line_len = strlen(lp);
			if(lp[0] == '#'){ // do not append base
				if(line_len>VIPER_MAX_MODULE_LINE){
					printf("[viperLoadList] module line too long: %d\n", line_len);
					printf("%s\n", line);
					fclose(fd);
					return;
				}
				// mark null at eol
				lp[line_len-EOL_TRUNC_LEN] = '\0';
				viperLoad(&line[1]);	// exclude #
			}
			else if(lp[0] == '!'){ // comment fields
				printf("%s", lp);
			}
			else if(line_len<3){ // probably blank line
			
			}
			else{ //append to the line
				if((base_len+line_len)>VIPER_MAX_MODULE_LINE){
					printf("[viperLoadList] base + module line too long: %d\n", base_line+line_len);
					printf("%s%s\n", base_line, line);
					fclose(fd);
					return;
				}
				for(i=0; i<line_len; i++) base_line[base_len+i] = lp[i];
				// mark null
				base_line[base_len+i-EOL_TRUNC_LEN] = '\0';
				viperLoad(base_line);
			}
		} // end parse
	}
	fclose(fd);
}
}


