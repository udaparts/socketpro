// MyOpenSSL.h: interface for the CMyOpenSSL class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __UDAPARTS_MY_OPENSSL_H___
#define __UDAPARTS_MY_OPENSSL_H___



#include "../../include/membuffer.h"
#include <openssl/ssl.h> 

#define STATIC_BUFFER_SIZE (1460 * 12)

class CMyOpenSSL {
public:
    CMyOpenSSL(SSL_CTX *pContext, bool bClient = false);
    virtual ~CMyOpenSSL();

public:
    bool DoHandshake(const void *pBuffer, unsigned int nLen, SPA::CUQueue &qOut);
    unsigned int Encrypt(const void *pBuffer, unsigned int nLen, SPA::CUQueue &qOut);
    unsigned int Decrypt(const void *pBuffer, unsigned int nLen, SPA::CUQueue &qOut);
    //int GetSSLState() const;
    bool Done() const;
    SSL* GetSSL() const;
    static bool IsFatal(int res);
    BIO* GetRBio() const;

private:
    CMyOpenSSL(const CMyOpenSSL &openssl);
    CMyOpenSSL& operator=(const CMyOpenSSL &openssl);

private:
    SSL *m_pSSL;
    BIO *m_pRBio;
    BIO *m_pWBio;
    bool m_bClient;
};

#endif // __UDAPARTS_MY_OPENSSL_H___
