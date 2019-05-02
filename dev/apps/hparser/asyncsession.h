#pragma once

#include "httpcontext.h"

namespace UHTTP {

#define ASYNC_BUFFER_SIZE		(2*1460)
#define MID_SIZE_BUFFER			(25*1024)
#define LARGE_SESSION_BUFFER	(1024*1024)

    typedef SPA::CScopeUQueueEx<ASYNC_BUFFER_SIZE, SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CAsyncSessionScopeUQueue;
    typedef SPA::CScopeUQueueEx<LARGE_SESSION_BUFFER, LARGE_SESSION_BUFFER> CLargeSessionBuffer;

    class CAsyncSession {
    public:
        CAsyncSession(CIoService &io, CSslContext &sslContext);
        ~CAsyncSession(void);

    public:
        bool IsSecure();
        CSocket& GetSocket();

    protected:
        virtual void OnClose(const CErrorCode &ec);

    private:
        CAsyncSession(const CAsyncSession &as);
        CAsyncSession& operator=(const CAsyncSession &as);
        void Start();
        void OnSSLHandleShake(const CErrorCode &ec);
        void OnWriteCompleted(const CErrorCode &ec, size_t bytes_transferred);
        void OnRead(const CErrorCode &ec, size_t bytes_transferred);
        void SendData();
        bool IsSending();
        void InternalShutDown();
        void ProcessHttpRequest();

    private:
        SPA::CScopeUQueue m_sqRecv;
        SPA::CScopeUQueue m_sqSend;
        SPA::CUQueue &m_qRecv;
        SPA::CUQueue &m_qSend;

        CSslSocket m_ssl;
        CSslContext &m_sslContext;
        CSocket &m_socket;

        CAsyncSessionScopeUQueue m_asRead;
        CAsyncSessionScopeUQueue m_asWrite;
        unsigned char *m_BufferRead;
        unsigned char *m_BufferWrite;

        CErrorCode m_ec;
        UHTTP::CHttpContext *m_pHttpContext;

        static const unsigned int MULTIPLE_CONTEXT_LENGTH = 30 * 1024;

        friend class CAsyncServer;
    };

};

