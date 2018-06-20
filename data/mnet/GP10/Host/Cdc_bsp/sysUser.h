/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

#include <loginLib.h>


#define MAX_NUM_USERS  100

#define MAX_USER_NAME_LEN  40    /* cannot exceed MAX_LOGIN_NAME_LEN */
#define MAX_USER_PASSWD_LEN  40  /* cannot exceed 40 */


/*******************************************************************************
sysUserInit - User Initialize

This function initializes the System User sub system. It should be called once
during vxWorks initialization.
*/
void sysUserInit();

/*******************************************************************************
sysUserAdd - User Add

Adds a user to the list of valid system user.

Adds a user name and password entry to user record file and optionally adds the 
user to login table. If user level is 0 ('super' user) user is added to 
vxWorks login table.
*/
STATUS sysUserAdd          /* RETURN: OK/ERROR */ 
    (
    const char *name,      /* IN: user name. Max.len. MAX_USER_NAME_LEN */
    const char *passwd,    /* IN: user password. Max. len MAX_USER_PASSWD_LEN */
    int level              /* IN: user permission level */
    );


/*******************************************************************************
sysUserDelete - User Delete

Deletes the user with given user name from list of valid users,

Deletes from user info. file and if user level is 0 from the vxWorks login 
table. Last user with permission level 0 cannot be deleted.
*/
STATUS sysUserDelete     /* RETURN: OK/ERROR */   
    (
    const char  *name    /* IN: user name */
    );


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
    );


/*******************************************************************************
sysUserPasswdChange - User Password Channge 

Chnages the password of a user.
*/
STATUS sysUserPasswdChange  /* RETURN: OK/ERROR */
    (
    char  *name,      /* IN: user name. Max.len. MAX_USER_NAME_LEN  */
    char  *oldPasswd, /* IN: old user password. */
    char  *passwd     /* OUT: new user password. Max. len MAX_USER_PASSWD_LEN */
    );


/*******************************************************************************
sysUserAllGet - User All Get 

Provides the list of current valid system user names.
*/
STATUS sysUserAllGet /* RETURN: OK/ERROR */
    (
    char  *names     /* OUT: user names seperated by \n */ 
    );

