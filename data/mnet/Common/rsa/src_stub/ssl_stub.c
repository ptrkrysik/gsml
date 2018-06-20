/* *******************************************************************
//
// (c) Copyright Cisco Systems, Inc 2001
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// Status      : Under development
// File        : ssl_stub.cpp

// Description : Implementation stub functions SSL 
//
// *******************************************************************/
#define FLAT_INC
#define OPT_32_BIT
#include "rsa\ssl.h"
#include "rsa\des.h"
#include "rsa\x509.h"

int isSSL (void)
{ 
    return (0);
}

int SSL_library_init (void)
{
    return 0;
}


SSL_METHOD *SSLv3_method (void)
{
    return (SSL_METHOD *)NULL;
}

SSL_CTX *SSL_CTX_new (SSL_METHOD *meth)
{
    return (SSL_CTX *)NULL;
}

int SSL_CTX_use_certificate_file (SSL_CTX *ctx, char *file, int type)
{
    return 0;
}

void SSL_CTX_set_default_passwd_cb (SSL_CTX *ctx,int (*cb)())
{
}

int SSL_CTX_use_PrivateKey_file (SSL_CTX *ctx, char *file, int type)
{
    return 0;
}

int SSL_CTX_load_verify_locations (SSL_CTX *ctx,char *CAfile,char *CApath)
{
    return 0;
}

void    SSL_set_verify_depth (SSL *ssl, int depth)
{
    return;
}

void RAND_seed ( unsigned char *buf,int num)
{
    return;
}

int SSL_CTX_set_cipher_list (SSL_CTX *ctx, char *str)
{
    return 0;
}

int SSL_CTX_set_session_id_context (SSL_CTX *ctx,const unsigned char *sid_ctx,
                                    unsigned int sid_ctx_len)
{
    return 0;
}

RSA * RSA_generate_key (int bits, unsigned long e,
                        void (*callback)(int,int,char *),char *cb_arg)
{
    return (RSA *)NULL;
}

long SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, char *parg)
{
    return 0;
}


void RSA_free(RSA *rsa)
{
    return;
}


void SSL_CTX_free (SSL_CTX *ctx)
{
    return;
}


SSL * SSL_new(SSL_CTX *ctx)
{
    return (SSL *) NULL;
}

int SSL_set_session(SSL *ssl, SSL_SESSION *sess)
{
    return 0;
}

BIO *BIO_new_socket(SIO_SOCK sock, int close_flag)
{
    return (BIO *)NULL;
}

void SSL_set_bio(SSL *ssl, BIO *rbio,BIO *wbio)
{
    return;
}


int SSL_shutdown(SSL *ssl)
{
    return 0;
}

int SSL_clear(SSL *s)
{
    return 0;
}

void SSL_free(SSL *ssl)
{
    return;
}

void SSL_set_connect_state(SSL *ssl)
{
}

void SSL_set_accept_state(SSL *ssl)
{
}

int SSL_set_cipher_list(SSL *ssl, char *str)
{
    return 0;
}

long SSL_get_verify_result(SSL *ssl)
{
    return 0;
}

void SSL_CTX_set_verify(SSL_CTX *ctx,int mode,int (*callback)())
{
}

int SSL_connect(SSL *ssl)
{
    return 0;
}

int SSL_get_error(SSL *ssl,int ret_code)
{
    return 0;
}

char *    SSL_CIPHER_get_name(SSL_CIPHER *cipher)
{
    return (char *)0;
}


int SSL_accept(SSL *ssl)
{
    return 0;
}

int SSL_read(SSL *ssl,char *buffer,int num)
{
    return 0;
}

int SSL_write(SSL *ssl,char *buffer,int num)
{
    return 0;
}

SSL_CIPHER *SSL_get_current_cipher(SSL *ssl)
{
    return (SSL_CIPHER *)0;
}

int des_key_sched(des_cblock *key,des_key_schedule schedule)
{
    return 0;
}

void des_ecb_encrypt (des_cblock *input,des_cblock *output,
                      des_key_schedule ks,int enc)
{
    return;
}

void ERR_print_errors_fp(FILE *fp)
{
    return;
}

X509_NAME *    X509_get_subject_name(X509 *x509)
{
    return (X509_NAME *) 0;
}

char *X509_NAME_oneline(X509_NAME *xn,char *buf,int size)
{
    return (char *) 0;
}

int    SSL_SESSION_print_fp(FILE *fp, SSL_SESSION *sess)
{
    return (0);
}

SSL_SESSION *SSL_get_session(SSL *ssl)
{
    return (NULL);
}

void SSL_CTX_set_verify_depth (SSL_CTX *ctx, int depth)
{
    return;
}

int SSL_CTX_check_private_key(SSL_CTX *ctx)
{
    return (0);
}


void BIO_free_all(BIO *bio)
{
    return;
}

int BIO_free(BIO *bio)
{
    return (0);
}
