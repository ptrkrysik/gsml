/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*******************************************************************************
sysUser

This file contains a set of functions to manage system users. It extends 
VxWorks login user feature by providing persistance records of user name and 
passwords and multiple permission levels.

User records which consists of user name, encrypted password, and permission 
level are stored in a user record file. Each user name is unique. User with 
permission level 0 are considered 'super' users. These users are added to 
VxWorks login table to provide them permission to remote login by telnet 
and FTP. This makes VxWorks login table subset of user record file.

It is ensured that three is at least one user with permission level 0. If there
is no user with permission level 0 as might be the case at start up, a deafult 
user is considered a valid user with permission level 0. These default user is
disabled when a user with permission level 0 is added. When the default user is
a valid user of the system only 'super' user can be added. In other words the 
first user to be added to the system must have permission level 0.

The user record file is a text file with a header and user records. If a file 
exists it is read at initialization. If it does not exist it is created with
no user records. 

Each line in the file can have maximum 200 characters. The header consists of 
some inforation about the file sturcture. The number of lines in header and a 
record is 3. Future versions can increase the number of lines per record. 
A file created with future version is compatible with this version as the file 
structure infomation is obtained from the header. An exception is additon of 
new records.

File Format:

     -------------------------      -
                                     |
     -------------------------
     3  3                            >   Header
     -------------------------
                                     |
     -------------------------      -  
     Name                            |
     -------------------------
     Encrypted Password              >   Record
     -------------------------
     level                           |    
     -------------------------      -  
          :         :
          :         :
          :         :
     -------------------------      -  
     Name                            |
     -------------------------
     Encrypted Password              >   Record
     -------------------------
     level                           |   
     -------------------------      -  

Available Functions:

  sysUserInit  -  Initialize the sub system. Should be called once.
  sysUserAdd   -  Adds a user.
  sysUserDelete - Delets a user.
  sysUserPasswdChange - Changes user password.
  sysUserVerify - Verfies a user.
  sysUserAllGet - Returns a list of user names.
  sysUserShow - Prints the list of user names on stdio.
*/

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usrLib.h>
#include <sysLib.h>
#include <semLib.h>
#include <errnoLib.h>
#include "config.h"
#include "sysUser.h"

#define USER_FILE "vx/sysUser"      /* User Record File */

#define MAX_LINE_LEN  200           /* Maximum line size in the file */

#define LINES_IN_HEADER  3         
#define LINES_IN_RECORD  3

#define MAX_ENCRPT_PASSWD_LEN  80   /* Maximum length of encrypted password */  

     /* User Record stucture */
typedef struct 
{
  char name[MAX_USER_NAME_LEN + 1];
  char passwdEncrpt[MAX_ENCRPT_PASSWD_LEN + 1];
  int level;
} UserRecord;

static SEM_ID sysUserSem;      /* Mutex semaphore to access user records */
static char sysUserFileName[100];   /* User Record File */
static int headerNumLines;     /* number of lines in header (read from file) */
static int recNumLines;        /* number of lines in record (read from file) */
static int numUsers;           /* number of current users */
static int numSuperUsers;      /* number of current super user */
BOOL defaultUserInUse;  /* True if defualt user in usre */
static char* passwdEncrptList[MAX_NUM_USERS]; /* Encrypted passwords for */
                                            /* loginUserAdd should be static */

STATUS sysUserPasswdEncrypt(char* in, char* out);
STATUS sysUserAddFirstUser (const char *,const char *);

/*******************************************************************************
fSkipLines - File Skip Lines

Moves given file pointer to skip given number of lines
*/
static STATUS fSkipLines   /* RETURN: OK/ERROR */
   (
   FILE* fp,               /* IN/OUT: file pointer */
   int numLines            /* IN: number of lines to skip */  
   )
{
  const int BuffSize = 80;
  char buff[BuffSize];
  char* pData = buff;
  char lastChar;
  int i;

  for (i = 0; i < numLines; i++)
  {
    do
    {
      pData = fgets(buff, BuffSize, fp);
    }
    while (lastChar = buff[strlen(buff) -1], 
           ((lastChar != '\n') && (lastChar != '\r')) && (pData != NULL));
  }
  return (pData == NULL) ? ERROR : OK;
}


/*******************************************************************************
sysUserRecGet - User Record Get

Gets the user recoed pointed by fp. fp poines to the next record upon exit.
*/
static STATUS sysUserRecGet   /* RETUEN: OK/ERROR */
   (
   FILE* fp,                  /* IN: file pointer */
   UserRecord* rec            /* OUT: user recoed */
   )
{
  int retStat = ERROR;
  const int MaxNameLen = MAX_USER_NAME_LEN + 1;  
  const int MaxPasswdEncrptLen = MAX_ENCRPT_PASSWD_LEN + 1;
  char* name = rec->name;
  char* passwdEncrpt = rec->passwdEncrpt;
  char* pChar;

  if (fgets(name, MaxNameLen, fp) != NULL)      /* read name */
  {
    pChar = &name[strlen(name) - 2];
    if ((*pChar == '\n') || (*pChar == '\r'))
      *pChar = EOS;      /* discard NL/CR */ 
    pChar++; 
    if ((*pChar == '\n') || (*pChar == '\r'))
      *pChar = EOS;      /* discard NL/CR */
    if (*pChar != EOS) 
      fSkipLines(fp, 1);  /* lines is too long */
    if (fgets(passwdEncrpt, MaxPasswdEncrptLen, fp) != NULL)   /* read password */
    {
      pChar = &passwdEncrpt[strlen(passwdEncrpt) - 2];
      if ((*pChar == '\n') || (*pChar == '\r'))
        *pChar = EOS;    /* discard NL/CR */
      pChar++; 
      if ((*pChar == '\n') || (*pChar == '\r'))
        *pChar = EOS;    /* discard NL/CR */
      if (*pChar != EOS)
        fSkipLines(fp, 1);   /* lines is too long */ 
      if (fscanf(fp, "%d", &rec->level) != EOF)   /* rad permission level */
      {
        fSkipLines(fp, 1 + recNumLines - 3);    /* move fp to next record */
        retStat = OK;
      } 
    } 
  }
  return retStat; 
}


/*******************************************************************************
sysUserInit - User Initialize

This function initializes the System User sub system. It should be called once
during vxWorks initialization.
*/
void sysUserInit()
{
  FILE* fp;
  BOOL stop;
  UserRecord usrRec; 
 
  loginEncryptInstall(sysUserPasswdEncrypt, NULL);  

  sysUserSem = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);

  sprintf(sysUserFileName, "%s/%s", getenv("VIPERCALL_BASE"), USER_FILE);
  fp = fopen(sysUserFileName, "r");
  if (fp != NULL)
  {
    if (fSkipLines(fp, 1) == OK)
      if (fscanf(fp, "%d%d", &headerNumLines, &recNumLines) == 2)
        if (fSkipLines(fp, headerNumLines - 1) == OK)
        {
          for (stop = FALSE, numUsers = 0, numSuperUsers = 0; 
               (stop == FALSE) && (numUsers < MAX_NUM_USERS);)
          {  
            if (sysUserRecGet(fp, &usrRec) == OK)
            {
              if (usrRec.level == 0) 
              {    /* super user */
                /* Encrypted passwords for loginUserAdd should be static */
                passwdEncrptList[numUsers] = malloc(MAX_ENCRPT_PASSWD_LEN + 1);
                strcpy(passwdEncrptList[numUsers], usrRec.passwdEncrpt);
                /* add to login table */
                loginUserAdd(usrRec.name, passwdEncrptList[numUsers]);
                numSuperUsers++;
              }
              numUsers++;
            } 
            else
              stop = TRUE;
          }
          if (numSuperUsers == 0)
          {     /* Add default user */
            loginUserAdd(LOGIN_USER_NAME, LOGIN_PASSWORD);
            defaultUserInUse = TRUE;  /* make default user valid user */
          }
          else
            defaultUserInUse = FALSE;
        }
    fclose(fp);
  }
  else 
  {   /* No user record file */
    /* create the file */    
    fp = fopen(sysUserFileName, "w");
    /* write header */
    fprintf(fp,"\n%d %d\n\n", LINES_IN_HEADER, LINES_IN_RECORD);
    fclose(fp);
    headerNumLines = LINES_IN_HEADER;
    recNumLines = LINES_IN_HEADER;
    /* Add default user to login table */
    loginUserAdd(LOGIN_USER_NAME, LOGIN_PASSWORD);
    defaultUserInUse = TRUE;   /* make default user valid user */ 
    numSuperUsers = 0;
  }    
}


/*******************************************************************************
sysUserFind - User Find

This functions checks if an entry with a given user name exsists in the user 
record file. It optionally returns index of the user in the file and the record.
fp should point to begnning of the file.
*/
static STATUS sysUserFind     /* RETURN: OK if user found else ERROR */ 
  (
  FILE* fp,                   /* IN: file pointer */
  const char* userName,       /* IN: name of user to find */
  int* pUserNum,              /* OUT: index of user in file if not NULL*/
  UserRecord* pUserRec        /* OUT: user record if not NULL*/
  ) 
{
  STATUS retStat = ERROR;
  UserRecord userRec;
  int count;
  BOOL stop;

  fSkipLines(fp, headerNumLines);
  if (pUserRec == NULL) pUserRec = &userRec; 
  for (count = 0, stop = FALSE; 
       (stop == FALSE) && (sysUserRecGet(fp, pUserRec) == OK); count++)
  {
    if (strcmp(pUserRec->name, userName) == 0)
    {                       /* Found the user */
      stop = TRUE;
      retStat = OK;
    }
  }
  if (pUserNum != NULL) *pUserNum = count - 1;
  return retStat;
}


/*******************************************************************************
sysUserAdd - User Add

Adds a user to the list of valid system user.

Adds a user name and password entry to user record file and optionally adds the 
user to login table. If user level is 0 ('super' user) user is added to 
vxWorks login table.
*/
STATUS sysUserAdd         /* RETURN: OK/ERROR */ 
    (
    const char *name,     /* IN: user name. Max.len. MAX_USER_NAME_LEN */
    const char *passwd,   /* IN: user password. Max. len MAX_USER_PASSWD_LEN */
    int level             /* IN: user permission level */
    )
{
  STATUS retStat = ERROR;
  FILE* fp;
  char passwdEncrpt[MAX_ENCRPT_PASSWD_LEN + 1];

  if ((numUsers > MAX_NUM_USERS) || 
      (strlen(name) > MAX_USER_NAME_LEN) ||
      (strlen(passwd) > MAX_USER_PASSWD_LEN))
    return ERROR;       

  if ((defaultUserInUse == TRUE) && (level != 0))
    return ERROR;   /* first user must be 'super' user */

  if (semTake(sysUserSem, sysClkRateGet() * 5) != OK)
    return ERROR;   /* only one access at a time */

  fp = fopen(sysUserFileName, "r+");
  if (fp != NULL)
  {
    if (sysUserFind(fp, name, NULL, NULL) != OK)
    {       /* user name is not in use */
      if (sysUserPasswdEncrypt((char*)passwd, passwdEncrpt) == OK)
      {        /* password is long enough */
        fseek(fp, 0, SEEK_END);   /* write at the end */
        if (fprintf(fp, "%s\n%s\n%d\n", name, passwdEncrpt, level) > 0)
        {         
          if (level == 0)
          {        /* super user */
            if (defaultUserInUse == TRUE)
            {       /* remove default user */
              loginUserDelete(LOGIN_USER_NAME, LOGIN_PASSWORD);
              defaultUserInUse = FALSE;
            } 
            /* Encrypted passwords for loginUserAdd should be static */
            passwdEncrptList[numUsers] = malloc(MAX_ENCRPT_PASSWD_LEN +1);
            strcpy(passwdEncrptList[numUsers], passwdEncrpt);
            loginUserAdd((char*)name, passwdEncrptList[numUsers]);
            numSuperUsers++;
          } 
          numUsers++;
          retStat = OK;
        }
      }
    }
    fclose(fp);
  }
  semGive(sysUserSem);
  return retStat;
}


/*******************************************************************************
sysUserDelete - User Delete

Deletes the user with given user name from list of valid users,

Deletes from user info. file and if user level is 0 from the vxWorks login 
table. Last user with permission level 0 cannot be deleted.
*/
STATUS sysUserDelete     /* RETURN: OK/ERROR */   
    (
    const char  *name    /* IN: user name */
    )
{
  STATUS retStat = ERROR;
  FILE* fp;
  FILE* fpTmp;
  char tmpFile[100];
  int userNum;
  UserRecord userRec;
  char buff[MAX_LINE_LEN + 1];
  int i, j;
  BOOL stop;
  
 
  if (semTake(sysUserSem, sysClkRateGet() * 5) != OK)
    return ERROR;     /* only one access at a time */
 
  fp = fopen(sysUserFileName, "r");
  if (fp != NULL)
  {
    if (sysUserFind(fp, name, &userNum, &userRec) == OK)
    {         /* user found */
      if ((userRec.level != 0) || (numSuperUsers > 1))
      {       /* cannot remove last super user */
        sprintf(tmpFile, "%s.tmp", sysUserFileName);
        fpTmp = fopen(tmpFile, "w");
        if (fpTmp != NULL)
        {
          rewind(fp);
          for (i = 0; i < headerNumLines; i++)
          {
            fgets(buff, MAX_LINE_LEN, fp);
            fputs(buff, fpTmp);
          }
          for (i = 0, stop = FALSE; stop == FALSE; i++)
          {
            for (j = 0; (j < recNumLines) && (stop == FALSE); j++)
            {
              if (fgets(buff, MAX_LINE_LEN, fp) != NULL)
              {
                if (i != userNum)
                  fputs(buff, fpTmp);
              }
              else
                stop = TRUE;
            }
          }
          fclose(fp);
          fclose(fpTmp);
          if (copy(tmpFile, sysUserFileName) == OK)
          {
            numUsers--;
            if (userRec.level == 0)
            {
              loginUserDelete(userRec.name, userRec.passwdEncrpt);
              numSuperUsers--;
            }
            free(passwdEncrptList[userNum]);
            for (i = userNum; i < MAX_NUM_USERS; i++)
            {
              passwdEncrptList[i] = passwdEncrptList[i + 1];
            }
            retStat = OK;
          }
          remove(tmpFile);
        }
        else
          fclose(fp);
      }
      else 
        fclose(fp);
    }
    else 
      fclose(fp);
  }
  semGive(sysUserSem);
  return retStat;
}

/*******************************************************************************
sysUserVerify - User Verify

Verifies if given user is valid system user. Checks for user name and password
in the user recoed file. If default user name is in use it is returned as valid
user name. If there us erro reading user record file default user is considerd
a valid user.
*/
int sysUserVerify  /* RETURN: permission level, -1 if not a user */ 
    (
    char  *name,   /* IN: user name    */
    char  *passwd  /* IN: user password */
    )
{
  int retVal = -1;
  FILE* fp;
  UserRecord userRec;
  char passwdEncrpt[MAX_ENCRPT_PASSWD_LEN + 1];

  if((defaultUserInUse == TRUE) && (strcmp(name,LOGIN_USER_NAME)==0))
	  return 0;

  if (semTake(sysUserSem, sysClkRateGet() * 5) != OK)
    return ERROR;    /* only one access at a time */

  if (sysUserPasswdEncrypt((char*)passwd, passwdEncrpt) == OK)
  {
    fp = fopen(sysUserFileName, "r");
    if (fp != NULL)
    {
      if (sysUserFind(fp, name, NULL, &userRec) == OK)
      {
        if (strcmp(passwdEncrpt, userRec.passwdEncrpt) == 0)
          retVal = userRec.level;       
      }
      fclose(fp);
    }

    /* if user not found default user may be valid user */
/*  if (retVal == -1)   
    {
      if ((defaultUserInUse == TRUE) || (fp == NULL))   
      {
        if ((strcmp(name, LOGIN_USER_NAME) == 0) && 
            (strcmp(passwdEncrpt, LOGIN_PASSWORD) == 0))
          retVal = 0;
      }
    }
*/ 
  } 
  semGive(sysUserSem);
  return retVal;  
}



/*******************************************************************************
sysUserPasswdChange - User Password Channge 

Changes the password of a user.
*/
STATUS sysUserPasswdChange  /* RETURN: OK/ERROR */
    (
    char  *name,      /* IN: user name. Max.len. MAX_USER_NAME_LEN */
    char  *oldPasswd, /* IN: old user password */
    char  *passwd     /* OUT: new user password. Max. len MAX_USER_PASSWD_LEN */
    )
{
  STATUS retStat = ERROR;
  FILE* fp;
  FILE* fpTmp;
  char tmpFile[100];
  int userNum;
  UserRecord userRec;
  char buff[MAX_LINE_LEN + 1];
  int i, j;
  BOOL stop;
  char passwdEncrpt[MAX_ENCRPT_PASSWD_LEN + 1];


  if ((strlen(name) > MAX_USER_NAME_LEN) ||
      (strlen(passwd) > MAX_USER_PASSWD_LEN))
    return ERROR;       

  if (semTake(sysUserSem, sysClkRateGet() * 5) != OK)
    return ERROR;  /* only one access at a time */

  fp = fopen(sysUserFileName, "r");
  if (fp != NULL)
  {
    sysUserPasswdEncrypt(oldPasswd, passwdEncrpt);
    if ((sysUserFind(fp, name, &userNum, &userRec) == OK) &&
        (strcmp(passwdEncrpt, userRec.passwdEncrpt) == 0))
    {       /* found the user */
      if (sysUserPasswdEncrypt(passwd, passwdEncrpt) == OK)
      {      /* new password is long enough */
        sprintf(tmpFile, "%s.tmp", sysUserFileName);
        fpTmp = fopen(tmpFile, "w");
        if (fpTmp != NULL)
        {
          rewind(fp);
          for (i = 0; i < headerNumLines; i++)
          {
            fgets(buff, MAX_LINE_LEN, fp);
            fputs(buff, fpTmp);
          }
          for (i = 0, stop = FALSE; stop == FALSE; i++)
          {
            for (j = 0; (j < recNumLines) && (stop == FALSE); j++)
            {
              if (fgets(buff, MAX_LINE_LEN, fp) != NULL)
              {
                if ((i == userNum) && (j == 1))
                  fprintf(fpTmp, "%s\n", passwdEncrpt);
                else
                  fputs(buff, fpTmp);
              }
              else
                stop = TRUE;
            }
          }
          fclose(fp);
          fclose(fpTmp);
          if (copy(tmpFile, sysUserFileName) == OK)
          {
            if (userRec.level == 0)
            {    /* super user */
              loginUserDelete(userRec.name, userRec.passwdEncrpt);
              strcpy(passwdEncrptList[userNum], passwdEncrpt);
              loginUserAdd(name, passwdEncrptList[userNum]);
            }
            retStat = OK;
          }
          remove(tmpFile);
        }
        else 
          fclose(fp);
      }
      else
        fclose(fp);
    }
    else
      fclose(fp);
  }
  semGive(sysUserSem);
  return retStat;
}


/*******************************************************************************
sysUserAllGet - User All Get 

Provides the list of current valid system user names.
*/
STATUS sysUserAllGet /* RETURN: OK/ERROR */
    (
    char  *names     /* OUT: user names seperated by \n */ 
    )
{
  STATUS retStat = ERROR;
  FILE* fp;
  UserRecord userRec;

  if (semTake(sysUserSem, sysClkRateGet() * 5) != OK)
    return ERROR;    /* only one access at a time */

  *names = EOS;
  fp = fopen(sysUserFileName, "r");
  if (fp != NULL)
  {
    fSkipLines(fp,headerNumLines);
    for ( ;sysUserRecGet(fp, &userRec) == OK; )
    {
      strcat(names, userRec.name); 
      strcat(names, "\n");
    }
    fclose(fp);
    retStat = OK;
  } 
  /* default user may be valid user */
  /* if there is error reading usrer record file, 
     default user is valid user*/
  if ((defaultUserInUse == TRUE) || (fp == NULL))
  {
    strcat(names, LOGIN_USER_NAME);
    strcat(names, "\n");
  }
  semGive(sysUserSem);
  return retStat;
}


/*******************************************************************************
sysUserShow - User Show 

Prints the list of current system user names.
*/
void sysUserShow()
{
  char* names;

  names = malloc(((MAX_USER_NAME_LEN + 1) * MAX_NUM_USERS) + 1);
  sysUserAllGet(names);
  printf("%s", names);
  free(names);
}


/*******************************************************************************
sysUserPasswdEncrpt - Password Encrypt

Password Encryption routine from Tornado 2, patch 1.
*/
/******************************************************************************
*
* loginDefaultEncrypt - default password encryption routine
*
* This routine provides default encryption for login passwords. It employs
* a simple encryption algorithm.  It takes as arguments a string <in> and a
* pointer to a buffer <out>.  The encrypted string is then stored in the
* buffer.
*
* The input strings must be at least 8 characters and no more than 40
* characters.
*
* If a more sophisticated encryption algorithm is needed, this routine can
* be replaced, as long as the new encryption routine retains the same
* declarations as the default routine.  The routine vxencrypt
* in \f3host/<hostOs>/bin\fP
* should also be replaced by a host version of <encryptionRoutine>. For more
* information, see the manual entry for loginEncryptInstall().
*
* RETURNS: OK, or ERROR if the password is invalid.
*
* SEE ALSO: loginEncryptInstall(), vxencrypt
*
* INTERNAL
* The encryption is done by summing the password and multiplying it by
* a magic number.
*/

STATUS sysUserPasswdEncrypt
    (
    char *in,                           /* input string */
    char *out                           /* encrypted string */
    )
    {
    int            ix;
    unsigned long  magic     = 31695317;
    unsigned long  passwdInt = 0;

   if (strlen (in) < 8 || strlen (in) > 40)
        {
        errnoSet (S_loginLib_INVALID_PASSWORD);
        return (ERROR);
        }

    for (ix = 0; ix < strlen(in); ix++)         /* sum the string */
        passwdInt += (in[ix]) * (ix+1) ^ (ix+1);

    sprintf (out, "%lu", (passwdInt * magic)); /* convert interger 
                                                        to string */
    /* make encrypted passwd printable */

    for (ix = 0; ix < strlen (out); ix++)
        {
        if (out[ix] < '3')
            out[ix] = out[ix] + '!';    /* arbitrary */

        if (out[ix] < '7')
            out[ix] = out[ix] + '/';    /* arbitrary */

        if (out[ix] < '9')
            out[ix] = out[ix] + 'B';    /* arbitrary */
        }

    return (OK);
    }




int sysUserGetUserLevelByName  
    (
    char  *name   /* IN: user name    */
    )
{
  int retVal = -1;
  FILE* fp;
  UserRecord userRec;
  
  if (semTake(sysUserSem, sysClkRateGet() * 5) != OK)
    return ERROR;    /* only one access at a time */

    fp = fopen(sysUserFileName, "r");
    if (fp != NULL)
    {
      if (sysUserFind(fp, name, NULL, &userRec) == OK)
      {
          retVal = userRec.level;       
      }
      fclose(fp);
    }
  
  semGive(sysUserSem);
  return retVal;  
}