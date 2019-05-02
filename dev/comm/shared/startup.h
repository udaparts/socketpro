
#ifndef _SOCKETPRO_OPENSSL_START_UP_H_
#define _SOCKETPRO_OPENSSL_START_UP_H_

#include "../../include/commutil.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>

typedef int (*PSSL_library_init) (void);
typedef SSL_METHOD* (*PSSLv23_method) (void);
typedef const SSL_METHOD* (*PTLSv1_method) (void);
typedef const SSL_METHOD* (*PTLSv1_server_method) (void);
typedef const SSL_METHOD* (*PTLSv1_client_method) (void);
typedef void(*PSSL_load_error_strings) (void);

typedef SSL_CTX*(*PSSL_CTX_new) (const SSL_METHOD* pMethod);
typedef void(*PSSL_CTX_free) (SSL_CTX *pCTX);
typedef long (*PSSL_CTX_ctrl) (SSL_CTX *ctx, int cmd, long larg, void *parg);

typedef void(*PSSL_set_connect_state) (SSL *pSSL);
typedef void(*PSSL_set_accept_state) (SSL *pSSL);
typedef int (*PSSL_CTX_use_certificate_file) (SSL_CTX *pCTX, const char *strFile, int nType);
typedef int (*PSSL_CTX_use_PrivateKey_file) (SSL_CTX *pCTX, const char *strFile, int nType);
typedef int (*PSSL_CTX_check_private_key) (SSL_CTX *pCTX);
typedef int (*PSSL_do_handshake) (SSL *pSSL);

typedef int (*PSSL_set_fd) (SSL *pSSL, int hSocket);
typedef int (*PSSL_accept) (SSL *pSSL);
typedef int (*PSSL_get_error) (SSL *pSSL, int nRet);
typedef int (*PSSL_write) (SSL *pSSL, const void *pBuf, int nLen);
typedef int (*PSSL_read) (SSL *pSSL, void *pBuf, int nLen);
typedef int (*PSSL_peek) (SSL *pSSL, void *pBuf, int nLen);

typedef SSL*(*PSSL_new) (SSL_CTX *pCTX);
typedef void(*PSSL_free) (SSL* pSSL);
typedef int (*PSSL_shutdown) (SSL *pSSL);

typedef int (*PSSL_state) (SSL *pSSL);

typedef SSL_CIPHER* (*PSSL_get_current_cipher) (SSL *s);
typedef const char* (*PSSL_CIPHER_get_name) (SSL_CIPHER *pCipher);
typedef X509* (*PSSL_get_peer_certificate) (SSL *pSSL);
typedef char* (*PSSL_CIPHER_get_version) (SSL_CIPHER *pCipher);
typedef const char* (*PSSL_get_version) (SSL *pSSL);
typedef void (*PX509_free) (X509 *pX509);
typedef void (*PCRYPTO_free) (void *);
typedef char* (*PX509_NAME_oneline) (X509_NAME *pX509Name, char *pBuf, int nSize);
typedef X509_NAME* (*PX509_get_subject_name) (X509 *pX509);
typedef X509_NAME* (*PX509_get_issuer_name) (X509 *pX509);
typedef int (*PBIO_socket_ioctl) (int hSocket, long nType, void *nArg);
typedef int (*PSSL_want) (SSL *pSSL);

typedef const char*(*PSSL_state_string_long) (const SSL *pSSL);
typedef int (*PSSL_pending) (SSL *pSSL);
typedef int (*PSSL_connect) (SSL *pSSL);

typedef void (*PSSL_set_bio) (SSL *pSSL, BIO *bioRead, BIO *bioWrite);
typedef BIO* (*PSSL_get_rbio) (SSL *pSSL);
typedef BIO* (*PSSL_get_wbio) (SSL *pSSL);
typedef int (*PBIO_read) (BIO *pBIO, void *pData, int nLen);
typedef int (*PBIO_write) (BIO *pBIO, const void *pData, int nLen);
typedef long (*PBIO_ctrl) (BIO *pBIO, int nCmd, long lArg, void *pArg);

typedef void (*PERR_clear_error) (void);
typedef int (*PSSL_clear) (SSL *pSSL);
typedef void* (*PCRYPTO_malloc) (int nNum, const char *strFile, int nLine);
typedef void (*PCRYPTO_set_locking_callback) (void (*func)(int mode, int type, const char *file, int line));
typedef int (*PCRYPTO_num_locks) (void);
typedef void (*PSSL_set_read_ahead) (SSL *pSSL, int nYes);
typedef void (*PSSL_set_info_callback) (SSL *pSSL, void (*cb) (const SSL *pSSL, int nWhere, int nRtn));
typedef void (*PCRYPTO_set_id_callback)(unsigned long (*func)(void));

typedef int (*PSSL_get_fd) (SSL *pSSL);
typedef unsigned long (*PERR_get_error) (void);
typedef char*(*PERR_error_string) (unsigned long e, char *buf);
typedef void (*PERR_free_strings) (void);
typedef void (*PERR_remove_state) (unsigned long pid); /* if zero we look it up */

typedef int (*PCRYPTO_set_mem_debug_functions) (void (*m)(void *, int, const char *, int, int),
        void (*r)(void *, void *, int, const char *, int, int),
        void (*f)(void *, int),
        void (*so)(long),
        long (*go)(void));

typedef void (*PCRYPTO_dbg_malloc) (void *addr, int num, const char *file, int line, int before_p);
typedef void (*PCRYPTO_dbg_realloc) (void *addr1, void *addr2, int num, const char *file, int line, int before_p);
typedef void (*PCRYPTO_dbg_free) (void *addr, int before_p);
typedef void (*PCRYPTO_dbg_set_options) (long bits);
typedef long (*PCRYPTO_dbg_get_options) (void);
typedef int (*PCRYPTO_mem_ctrl) (int mode);
typedef void (*PCRYPTO_mem_leaks_fp) (FILE *);
typedef int (*PPKCS12_parse) (PKCS12 *p12, const char *pass, EVP_PKEY **pkey, X509 **cert, STACK_OF(X509) **ca);
typedef PKCS12* (*Pd2i_PKCS12_fp) (FILE *fp, PKCS12 **p12);
typedef void (*PPKCS12_free)(PKCS12 *p12);
typedef int (*PSSL_use_PrivateKey) (SSL *ssl, EVP_PKEY *pkey);
typedef int (*PSSL_use_certificate) (SSL *ssl, X509 *x);
typedef void (*POPENSSL_add_all_algorithms_noconf)();
typedef void (*PERR_load_crypto_strings) ();

typedef PKCS12* (*Pd2i_PKCS12_bio) (BIO *bp, PKCS12 **p12);
typedef BIO* (*PBIO_new_file) (const char *filename, const char *mode);
typedef int (*PBIO_free) (BIO *a);
typedef void (*PEVP_PKEY_free) (EVP_PKEY *pkey);
typedef void(*PX509_free) (X509 *a);

typedef BIO* (*PBIO_new) (BIO_METHOD *type);
typedef BIO_METHOD* (*PBIO_s_mem) (void);
typedef void (*PSSL_set_bio) (SSL *s, BIO *rbio, BIO *wbio);
typedef int (*PBIO_read) (BIO *b, void *data, int len);
typedef int (*PBIO_write) (BIO *b, const void *data, int len);
typedef size_t(*PBIO_ctrl_pending) (BIO *b);
typedef BIO* (*PSSL_get_rbio)(SSL *s);
typedef BIO* (*PSSL_get_wbio)(SSL *s);
typedef int (*PSSL_CTX_use_certificate_chain_file)(SSL_CTX *ctx, const char *file);
typedef void (*PSSL_CTX_set_default_passwd_cb)(SSL_CTX *ctx, pem_password_cb *cb);
typedef void (*PCRYPTO_set_dynlock_create_callback)(struct CRYPTO_dynlock_value *(*dyn_create_function)(const char *file, int line));
typedef void (*PCRYPTO_set_dynlock_lock_callback)(void (*dyn_lock_function)(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line));
typedef void (*PCRYPTO_set_dynlock_destroy_callback)(void (*dyn_destroy_function)(struct CRYPTO_dynlock_value *l, const char *file, int line));
typedef void (*PERR_load_BIO_strings)(void);
typedef long (*PSSL_ctrl)(SSL *ssl, int cmd, long larg, void *parg);

class CStartup {
public:
    CStartup(bool client);
    ~CStartup();

private:
    CStartup(const CStartup &s);
    CStartup& operator=(const CStartup &s);

    void StopOpenSSL();
    bool StartOpenSSL();
    void UnloadOpenSSL();
    bool LoadOpenSSL();
    static void LockingCallback(int nMode, int nType, const char *strFile, int nLine);
    static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line);
    static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *lock, const char *file, int line);
    static void dyn_destroy_function(struct CRYPTO_dynlock_value *lock, const char *file, int line);
    static unsigned long openssl_id_func();

public:
    PX509_free X509_free;
    PEVP_PKEY_free EVP_PKEY_free;
    PBIO_free BIO_free;
    PBIO_new_file BIO_new_file;
    Pd2i_PKCS12_bio d2i_PKCS12_bio;
    PERR_load_crypto_strings ERR_load_crypto_strings;
    POPENSSL_add_all_algorithms_noconf OPENSSL_add_all_algorithms_noconf;
    PSSL_use_certificate SSL_use_certificate;
    PSSL_use_PrivateKey SSL_use_PrivateKey;
    PPKCS12_free PKCS12_free;
    PPKCS12_parse PKCS12_parse;
    PSSL_library_init SSL_library_init;
    PSSLv23_method SSLv23_method;
    PTLSv1_method TLSv1_method;
    PSSL_load_error_strings SSL_load_error_strings;
    PSSL_CTX_new SSL_CTX_new;
    PSSL_CTX_free SSL_CTX_free;
    PSSL_new SSL_new;
    PSSL_free SSL_free;
    PSSL_set_fd SSL_set_fd;
    PSSL_do_handshake SSL_do_handshake;
    PSSL_accept SSL_accept;
    PSSL_get_error SSL_get_error;
    PSSL_write SSL_write;
    PSSL_read SSL_read;
    PSSL_shutdown SSL_shutdown;
    PSSL_state SSL_state;
    PSSL_want SSL_want;
    PSSL_pending SSL_pending;
    PSSL_set_connect_state SSL_set_connect_state;
    PSSL_set_accept_state SSL_set_accept_state;
    PSSL_CTX_use_certificate_file SSL_CTX_use_certificate_file;
    PSSL_CTX_use_PrivateKey_file SSL_CTX_use_PrivateKey_file;
    PSSL_CTX_check_private_key SSL_CTX_check_private_key;
    PSSL_set_read_ahead SSL_set_read_ahead;
    PSSL_state_string_long SSL_state_string_long;
    PSSL_connect SSL_connect;
    PSSL_peek SSL_peek;
    PERR_clear_error ERR_clear_error;
    PSSL_clear SSL_clear;
    PSSL_set_info_callback SSL_set_info_callback;
    PSSL_get_fd SSL_get_fd;
    PCRYPTO_free CRYPTO_free;
    PCRYPTO_malloc CRYPTO_malloc;
    PCRYPTO_set_locking_callback CRYPTO_set_locking_callback;
    PCRYPTO_num_locks CRYPTO_num_locks;
    PERR_get_error ERR_get_error;
    PERR_error_string ERR_error_string;
    PSSL_CTX_ctrl SSL_CTX_ctrl;

    PSSL_set_bio SSL_set_bio;
    PBIO_read BIO_read;
    PBIO_write BIO_write;
    PBIO_ctrl_pending BIO_ctrl_pending;
    PSSL_get_rbio SSL_get_rbio;
    PSSL_get_wbio SSL_get_wbio;
    PBIO_new BIO_new;
    PBIO_s_mem BIO_s_mem;
    PSSL_CTX_use_certificate_chain_file SSL_CTX_use_certificate_chain_file;
    PSSL_CTX_set_default_passwd_cb SSL_CTX_set_default_passwd_cb;
    PCRYPTO_set_dynlock_create_callback CRYPTO_set_dynlock_create_callback;
    PCRYPTO_set_dynlock_lock_callback CRYPTO_set_dynlock_lock_callback;
    PCRYPTO_set_dynlock_destroy_callback CRYPTO_set_dynlock_destroy_callback;
    PTLSv1_server_method TLSv1_server_method;
    PTLSv1_client_method TLSv1_client_method;
    PCRYPTO_set_id_callback CRYPTO_set_id_callback;
    PERR_load_BIO_strings ERR_load_BIO_strings;
    PSSL_ctrl SSL_ctrl;

    SSL_CTX *m_pCtx;
    HINSTANCE m_hSSLEay;
    HINSTANCE m_hLibeay;

private:
    const SSL_METHOD* m_pMeth;
    SPA::CUCriticalSection *m_pSSLLockHandle;
    bool m_bClient;
};

extern CStartup *g_pStartup;

#endif
