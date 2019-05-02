// MyOpenSSL.h: interface for the CMyOpenSSL class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __UDAPARTS_MY_OPENSSL_H___
#define __UDAPARTS_MY_OPENSSL_H___


#include "../shared/streamhead.h"
#include "../../include/membuffer.h"
#include <openssl/ssl.h> 

#define STATIC_BUFFER_SIZE (IO_BUFFER_SIZE + 256)

class CMyOpenSSL  
{
public:
	static unsigned int Encrypt(SSL *ssl, const void *pBuffer, unsigned int nLen, SPA::CUQueue &qOut);
	static unsigned int Decrypt(SSL *ssl, const void *pBuffer, unsigned int nLen, SPA::CUQueue &qOut);
};

#endif // __UDAPARTS_MY_OPENSSL_H___
