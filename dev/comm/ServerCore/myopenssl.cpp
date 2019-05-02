// MyOpenSSL.cpp: implementation of the CMyOpenSSL class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "myopenssl.h"

unsigned int CMyOpenSSL::Encrypt(SSL *ssl, const void *pBuffer, unsigned int nSize, SPA::CUQueue &qOut)
{
	unsigned int nLen = 0;
	int res = ::SSL_write(ssl, pBuffer, nSize);
	BIO *wbio = ::SSL_get_wbio(ssl);
	while(::BIO_ctrl_pending(wbio) || res > 0)
	{
		if(qOut.GetTailSize() < STATIC_BUFFER_SIZE)
		{
			qOut.SetHeadPosition();
			qOut.ReallocBuffer(qOut.GetMaxSize() + STATIC_BUFFER_SIZE);
		}
		res = ::BIO_read(wbio, (unsigned char*)qOut.GetBuffer() + qOut.GetSize(), qOut.GetMaxSize() - qOut.GetSize());
		if(res > 0)
		{
			qOut.SetSize(qOut.GetSize() + res);
			nLen += res;
		}
		else
		{
			break;
		}
		res = 0;
	}
	return nLen;
}

unsigned int CMyOpenSSL::Decrypt(SSL *ssl, const void *pBuffer, unsigned int nSize, SPA::CUQueue &qOut)
{
	unsigned int nLen = 0;
	BIO *rbio = ::SSL_get_rbio(ssl);
	int res = ::BIO_write(rbio, pBuffer, nSize);
	while(::BIO_ctrl_pending(rbio))
	{
		if(qOut.GetTailSize() < STATIC_BUFFER_SIZE)
		{
			qOut.SetHeadPosition();
			qOut.ReallocBuffer(qOut.GetMaxSize() + STATIC_BUFFER_SIZE);
		}
		res = ::SSL_read(ssl, (unsigned char*)qOut.GetBuffer() + qOut.GetSize(), (qOut.GetMaxSize() - qOut.GetSize()));
		if(res > 0)
		{
			qOut.SetSize(qOut.GetSize() + res);
			nLen += res;
		}
		else
		{
			break;
		}
		res = 0;
	}
	return nLen;	
}