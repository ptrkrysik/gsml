// *******************************************************************
//
// (c) Copyright Cisco Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// Status      : Under development
// File        : ssl_intf.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : Implementation of the service functions required by 
//               SSL library
//
// *******************************************************************

#include "socket\GenSocketSSL.h"
#include "CallBackSSL.h"


// Global variables
// random key data
#define PASSWORD_KEY    {0xA5,0x30,0x1F,0x3C,0x8D,0x1B,0x39,0xE5}

////////////////////////////////////////////////////////////////
// Open SSL support functions
////////////////////////////////////////////////////////////////

#ifdef _OPEN_SSL_LIB_
// OpenSSL library does not have it
int isSSL ()
{
    return (1);
}
#endif // _OPEN_SSL_LIB_


// callback functions used by SSL library
// note OpenSSL allows passing a void user data pointer,and can do a per session check
// may need to modify RSA source to do that
// We will use one password for all the sockets on the host
int password_cb (char *buf, int bufSize, int rwflag
#ifdef _OPEN_SSL_LIB_
                 , void *pData
#endif // _OPEN_SSL_LIB_
                 )
{
  AM_FUNC  ("password_cb", SOC_LAYER_ID);
  AM_ENTER ();

  int    length  = 0;
  LPCSTR pPasswd = GenSocketSSL::getPassword();

    if (pPasswd && (length = strlen(pPasswd)) <= bufSize)
        strcpy (buf, pPasswd);

  AM_RETURN (length);
}


int verify_callback (int ok, X509_STORE_CTX *pStoreCtx)
{
  AM_FUNC  ("verify_callback", SOC_LAYER_ID);
  AM_ENTER ();

    char *s, buf[256] = "";

    // read subject line from the certificate
    s = X509_NAME_oneline (X509_get_subject_name(pStoreCtx->current_cert), buf, 256);

    if (s == NULL)
    {
        if (ok)
        {
            AM_TRACE (("Certificate depth=%d %s\n",pStoreCtx->error_depth, buf));
        }
        else
        {
            AM_WARNING (("Certificate depth=%d error=%d %s\n",
                         pStoreCtx->error_depth, pStoreCtx->error,buf));
        }
    }

    if (!ok)
    { // Certificate got a problem, parse it and decide whether to continue
        ok = GenSocketSSL::verifyCert (pStoreCtx->error);
    }

  AM_RETURN (ok);
}


// Encrypt and save password to the specified file
bool encryptPassword (const char *pwd, const char *filename)
{
  int               stat    ;
  unsigned          idx     ;
  const unsigned    blockNum = SSL_MAX_PASSWORD_LENGTH/LENGTH_OF_CBLOCK;
  des_cblock        out     [blockNum];
  des_key_schedule  ks      ;
  FILE             *fd      = NULL;
  bool              retVal  = false;

  AM_FUNC  ("SocLib: encryptPassword", SOC_LAYER_ID);
  AM_ENTER ();

    if (isSSL ())
    {
        const unsigned char key_data [] = PASSWORD_KEY;

        // Always encode SSL_MAX_PASSWORD_LENGTH bytes
        if (strlen (pwd) > SSL_MAX_PASSWORD_LENGTH)
        {
            AM_WARNING (("SocLib: Provided password is longer than allowed.\n Use first %u characters\n",
                         SSL_MAX_PASSWORD_LENGTH    ));
        }

        if (fd = fopen (filename, "wb"))
        {
            memset (out, 0, sizeof(out));

            for (idx = 0; idx < blockNum; idx++)
            {
                if ((stat = des_key_sched ((C_Block *)(key_data),ks)) != 0)
                {
                    AM_TRACE  (("SocLib: Key error %d\n", stat));
                    AM_RETURN (retVal);
                }

                des_ecb_encrypt ((C_Block *)(pwd+idx*LENGTH_OF_CBLOCK), // source
                                 (C_Block *)out[idx]                  , // dest
                                 ks, DES_ENCRYPT                      );
            }

            unsigned temp = 0;

            if ((temp = fwrite (out, SSL_MAX_PASSWORD_LENGTH, 1, fd)) == 1)
            {
                retVal  = true;
            }
            else
            {
                AM_TRACE (("SocLib: Unable to save password file %s\n", filename));
            }

            fclose (fd);
        }
        else
        {
            AM_TRACE (("SocLib: Unable to open password file %s\n", filename));
        }
    }

  AM_RETURN (retVal);
}


bool decryptPassword (char *pwdout, const char *filename)
{
  int               stat    ,
                    idx     ;
  const unsigned    blockNum= SSL_MAX_PASSWORD_LENGTH/LENGTH_OF_CBLOCK;
  des_cblock        in      [blockNum];
  des_key_schedule  ks      ;
  FILE             *fd      = NULL ;
  bool              retVal  = false;
  char passwd [SSL_MAX_PASSWORD_LENGTH+2];

  AM_FUNC  ("decryptPassword", SOC_LAYER_ID);
  AM_ENTER ();

    if (isSSL ())
    {
      const unsigned char key_data [] = PASSWORD_KEY;

        if (fd = fopen (filename, "rb"))
        {
            memset (passwd, 0, (SSL_MAX_PASSWORD_LENGTH+2));

            // Always encode SSL_MAX_PASSWORD_LENGTH bytes
            if (fread (in, SSL_MAX_PASSWORD_LENGTH, 1, fd) == 1)
            {
                for (idx = 0; idx < blockNum; idx++)
                { // decrypt it
                    if ((stat = des_key_sched((C_Block *)(key_data), ks)) != 0)
                    {
                        AM_TRACE  (("SocLib: Key error %d\n", stat));
                        AM_RETURN (retVal);
                    }

                    des_ecb_encrypt ((des_cblock *)in[idx],                           // source
                                     (des_cblock *)(passwd + idx * LENGTH_OF_CBLOCK), // dest
                                     ks, DES_DECRYPT      );
                }

                // verify decrypted password
                if ((stat = strlen (passwd)) != 0 && stat <= SSL_MAX_PASSWORD_LENGTH)
                {
                    for (idx = 0; idx < stat; idx++)
                    {
                        if (!isprint(passwd[idx]))
                            break;
                    }

                    if (idx == stat)
                    { // Password is OK
                        strcpy (pwdout, passwd);
                        retVal = true;
                    }
                }

                fclose (fd);

                if (!retVal)
                {
                    AM_TRACE (("Password verification failed\n"));
                }
            }
            else
            {
                AM_TRACE (("Invalid password file\n"));
            }
        }
        else
        {
            AM_TRACE (("Unable to open password file %s.\n", filename));
        }
    }

  AM_RETURN (retVal);
}

