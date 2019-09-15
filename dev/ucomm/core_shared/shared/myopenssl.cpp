
#include "myopenssl.h"


#ifndef NDEBUG
#include <iostream>
#endif

CMyOpenSSL::CMyOpenSSL(SSL_CTX *pContext, bool bClient)
: m_pSSL(::SSL_new(pContext)),
m_pRBio(::BIO_new(::BIO_s_mem())),
m_pWBio(::BIO_new(::BIO_s_mem())),
m_bClient(bClient) {
    int res;
    ::SSL_set_bio(m_pSSL, m_pRBio, m_pWBio);
    if (bClient) {
        ::SSL_set_connect_state(m_pSSL);
    } else {
        res = ::SSL_accept(m_pSSL);
        ::SSL_set_accept_state(m_pSSL);
    }
    res = SSL_set_mode(m_pSSL, SSL_MODE_AUTO_RETRY);
}

CMyOpenSSL::~CMyOpenSSL() {
    if (m_pSSL) {
        ::SSL_free(m_pSSL);
    }
}

bool CMyOpenSSL::Done() const {
    return SSL_is_init_finished(m_pSSL);
}

/*
int CMyOpenSSL::GetSSLState() const {
    return ::SSL_state(m_pSSL);
}
 */
BIO* CMyOpenSSL::GetRBio() const {
    return ::SSL_get_rbio(m_pSSL);
}

SSL* CMyOpenSSL::GetSSL() const {
    return m_pSSL;
}

bool CMyOpenSSL::IsFatal(int res) {
    switch (res) {
        case SSL_ERROR_WANT_WRITE:
        case SSL_ERROR_NONE:
        case SSL_ERROR_WANT_READ:
            return false;
            break;
        default:
            break;
    }
    return true;
}

bool CMyOpenSSL::DoHandshake(const void *pBuffer, unsigned int nLen, SPA::CUQueue &qOut) {
    if (!SSL_is_init_finished(m_pSSL)) {
        int res = BIO_write(m_pRBio, pBuffer, nLen);
        res = SSL_do_handshake(m_pSSL);
        if (res < 0) {
            res = SSL_get_error(m_pSSL, res);
            if (res == SSL_ERROR_WANT_READ) {
                if (BIO_ctrl_pending(m_pWBio)) {
                    if (qOut.GetTailSize() < 4096) {
                        qOut.ReallocBuffer(qOut.GetMaxSize() + 4096);
                    }
                    res = ::BIO_read(m_pWBio, (unsigned char*) qOut.GetBuffer(qOut.GetSize()), qOut.GetTailSize());
                    if (res < 0)
                        res = SSL_get_error(m_pSSL, res);
                    else {
                        qOut.SetSize(qOut.GetSize() + (unsigned int) res);
                        return true;
                    }
                }
            }
        } else if (res == 0) {
            res = SSL_get_error(m_pSSL, res);
            if (BIO_ctrl_pending(m_pWBio)) {
                if (qOut.GetTailSize() < 4096) {
                    qOut.ReallocBuffer(qOut.GetMaxSize() + 4096);
                }
                res = ::BIO_read(m_pWBio, (unsigned char*) qOut.GetBuffer(qOut.GetSize()), qOut.GetTailSize());
                if (res < 0)
                    res = SSL_get_error(m_pSSL, res);
                else {
                    qOut.SetSize(qOut.GetSize() + (unsigned int) res);
                    return true;
                }
            }
        } else {
            res = 0;
            if (BIO_ctrl_pending(m_pWBio)) {
                if (qOut.GetTailSize() < 4096) {
                    qOut.ReallocBuffer(qOut.GetMaxSize() + 4096);
                }
                res = ::BIO_read(m_pWBio, (unsigned char*) qOut.GetBuffer(qOut.GetSize()), qOut.GetTailSize());
                if (res < 0)
                    res = SSL_get_error(m_pSSL, res);
                else {
                    qOut.SetSize(qOut.GetSize() + (unsigned int) res);
                    return true;
                }
            }
        }
        if (IsFatal(res))
            return false;
        return true;
    }
    return false;
}

unsigned int CMyOpenSSL::Encrypt(const void *pBuffer, unsigned int nSize, SPA::CUQueue &qOut) {
    unsigned int nLen = 0;
    int res = ::SSL_write(m_pSSL, pBuffer, nSize);
    while (::BIO_ctrl_pending(m_pWBio) || res > 0) {
        if (qOut.GetTailSize() < STATIC_BUFFER_SIZE) {
            qOut.ReallocBuffer(qOut.GetMaxSize() + STATIC_BUFFER_SIZE);
        }
        res = ::BIO_read(m_pWBio, (unsigned char*) qOut.GetBuffer() + qOut.GetSize(), qOut.GetMaxSize() - qOut.GetSize());
        if (res > 0) {
            qOut.SetSize(qOut.GetSize() + res);
            nLen += res;
        } else {
            break;
        }
        res = 0;
    }
    return nLen;
}

unsigned int CMyOpenSSL::Decrypt(const void *pBuffer, unsigned int nSize, SPA::CUQueue &qOut) {
    int res;
    unsigned int nLen = 0;
    if (pBuffer && nSize) {
        res = ::BIO_write(m_pRBio, pBuffer, nSize);
    }
    while (::BIO_ctrl_pending(m_pRBio)) {
        if (qOut.GetTailSize() < STATIC_BUFFER_SIZE) {
            qOut.ReallocBuffer(qOut.GetMaxSize() + STATIC_BUFFER_SIZE);
        }
        res = ::SSL_read(m_pSSL, (unsigned char*) qOut.GetBuffer() + qOut.GetSize(), qOut.GetMaxSize() - qOut.GetSize());
        if (res > 0) {
            qOut.SetSize(qOut.GetSize() + res);
            nLen += res;
        } else {
            break;
        }
    }
    return nLen;
}