
#include "stdafx.h"
#include "asyncsession.h"
#include "../../include/membuffer.h"
#include "asyncserver.h"
#include <assert.h>
#include "jsloader.h"

namespace UHTTP {

    CAsyncSession::CAsyncSession(CIoService &io, CSslContext &sslContext)
    : m_qRecv(*m_sqRecv),
    m_qSend(*m_sqSend),
    m_ssl(io, sslContext),
    m_sslContext(sslContext),
    m_socket(m_ssl.next_layer()),
    m_BufferRead((unsigned char*) m_asRead->GetBuffer()),
    m_BufferWrite((unsigned char*) m_asWrite->GetBuffer()),
    m_pHttpContext(NULL) {
        m_asWrite->CleanTrack();
        m_asRead->CleanTrack();
    }

    CAsyncSession::~CAsyncSession(void) {

    }

    CSocket& CAsyncSession::GetSocket() {
        return m_socket;
    }

    void CAsyncSession::OnSSLHandleShake(const CErrorCode &ec) {
        if (ec) {
            std::string errMsg = ec.message();
            int errCode = ec.value();
            CErrorCode ErrorCode;
            m_ssl.shutdown(ErrorCode);
            m_socket.shutdown(nsAsio::socket_base::shutdown_both, ErrorCode);
            m_socket.close(ErrorCode);
            g_pAsyncServer->m_vDeadAsyncSession.push_back(this);
            m_ec = ec;
        } else {
            m_ssl.async_read_some(boost::asio::buffer(m_BufferRead, ASYNC_BUFFER_SIZE), boost::bind(&CAsyncSession::OnRead, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
        }
    }

    void CAsyncSession::OnClose(const CErrorCode &ec) {
        CErrorCode ErrorCode;
        bool secure = IsSecure();
        if (secure)
            m_ssl.shutdown(ErrorCode);
        m_socket.shutdown(nsAsio::socket_base::shutdown_both, ErrorCode);
        m_socket.close(ErrorCode);
        UHTTP::CHttpContext::Unlock(m_pHttpContext);
        m_pHttpContext = NULL;
        g_pAsyncServer->m_vDeadAsyncSession.push_back(this);
    }

    void CAsyncSession::ProcessHttpRequest() {
        if (m_pHttpContext == NULL)
            m_pHttpContext = CHttpContext::Lock();

        if (m_pHttpContext->IsWebSocket()) {
            const unsigned char *end = m_pHttpContext->ParseWSMsg(m_qRecv.GetBuffer(), m_qRecv.GetSize());
            unsigned int len = (unsigned int) (end - m_qRecv.GetBuffer());
            CWebSocketMsg *pWebSocketMsg = m_pHttpContext->GetWebSocketMsg();
            if (pWebSocketMsg->ParseStatus == psComplete && len > 0 && pWebSocketMsg->IsFin()) {
                m_qRecv.Pop(len);
                tagWSOpCode wsOpCode = pWebSocketMsg->GetOpCode();
                SPA::CUQueue &q = pWebSocketMsg->Content;
                switch (wsOpCode) {
                    case ocTextMsg:
                    {
                        const UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
                        SPA::CScopeUQueue jBuffer;
                        if (ur.IsBatching()) {
                            m_pHttpContext->PrepareBatchWSResponseMessage(ur, wsOpCode, m_qSend);
                        } else {
                            SPA::CScopeUQueue jBuffer;
                            m_pHttpContext->ProcessSpRequest(ur, *jBuffer);
                            m_pHttpContext->PrepareWSResponseMessage(jBuffer->GetBuffer(), jBuffer->GetSize(), wsOpCode, m_qSend);
                        }
                    }
                        break;
                    case ocMsgContinuation:
                        m_qRecv.Insert(q.GetBuffer(), q.GetSize());
                        break;
                    case ocConnectionClose:
                    case ocPing:
                    case ocPong:
                        m_pHttpContext->PrepareWSResponseMessage(q.GetBuffer(), q.GetSize(), wsOpCode, m_qSend);
                        break;
                    case ocBinaryMsg:
                    default:
                        assert(false);
                        break;
                }
                q.SetSize(0);
            } else {
                return;
            }
            return;
        }

        UHTTP::tagParseStatus ps = m_pHttpContext->GetPS();
        do {
            if (ps >= psHeaders)
                break;
            const char *start = (const char*) m_qRecv.GetBuffer();
            const char *end = m_pHttpContext->ParseHeaders(start);
            ps = m_pHttpContext->GetPS();
            if (ps >= psHeaders) {
                unsigned int parsed = (unsigned int) (end - start);
                m_qRecv.Pop(parsed);
                m_qRecv.SetHeadPosition();
                m_qRecv.SetNull();
            } else
                return; //if headers are not available, stop here!
        } while (false);

        SPA::UINT64 content_length = m_pHttpContext->GetContentLength();
        if (content_length != CONTENT_LEN_UNKNOWN) {
            unsigned int ms = m_qRecv.GetMaxSize();
            if (content_length > ms && MULTIPLE_CONTEXT_LENGTH > ms)
                m_qRecv.ReallocBuffer(MULTIPLE_CONTEXT_LENGTH + 128);
            if (m_qRecv.GetSize() < content_length && m_qRecv.GetSize() < MULTIPLE_CONTEXT_LENGTH) {
                return;
            }
        }

        SPA::ServerSide::tagHttpMethod hm = m_pHttpContext->GetMethod();
        if (m_pHttpContext->IsWebSocket()) {
            m_pHttpContext->PrepareResponse(NULL, 0, m_qSend, UHTTP::hrfpAll);
        } else if (hm == SPA::ServerSide::hmGet) {
            if (m_pHttpContext->GetHttpRequestType() == hrtDownloadAdapter) {
                unsigned int len;
                UHTTP::CUJsLoader loader(m_pHttpContext->GetUserAgent(),
                        m_pHttpContext->GetParams().Start,
                        m_pHttpContext->GetHost(),
                        m_pHttpContext->IsCrossDomain(),
                        IsSecure());
                const char *code = loader.GetSPACode(len);
                m_pHttpContext->PrepareResponse((const unsigned char*) code, len, m_qSend, UHTTP::hrfpAll);
            } else if (m_pHttpContext->GetHttpRequestType() == hrtJsRequest) {
                const UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
                SPA::CScopeUQueue jBuffer;
                m_pHttpContext->ProcessSpRequest(ur, *jBuffer);
                m_pHttpContext->PrepareResponse(jBuffer->GetBuffer(), jBuffer->GetSize(), m_qSend, UHTTP::hrfpAll);
            } else {
                const char *s = m_pHttpContext->GetUrl().Start;
                const char *e = s + m_pHttpContext->GetUrl().Length;
                std::string file(s + 1, e);
                m_pHttpContext->StartDownloadFile(file.c_str(), m_qSend);
            }
            if (m_pHttpContext->GetResponseProgress().Status == hrsCompleted) {
                CHttpContext::Unlock(m_pHttpContext);
                m_pHttpContext = NULL;
            }
        } else if (hm == SPA::ServerSide::hmPost) {
            SPA::ServerSide::tagContentMultiplax cm = m_pHttpContext->GetCM();
            if (cm != SPA::ServerSide::cmUnknown) {
                unsigned int size = m_qRecv.GetSize();
                const char *start = (const char*) m_qRecv.GetBuffer();
                const char *stop = m_pHttpContext->ParseMultipart(start, size);
                unsigned int len = (unsigned int) (stop - start);
                m_qRecv.Pop(len);
                m_qRecv.SetHeadPosition();
                if (m_pHttpContext->GetMultiplaxContext()->ParseStatus == UHTTP::psComplete) {
                    const UHTTP::CHeaderValue* pHV = m_pHttpContext->GetMultiplaxContext()->GetHeaderValue(size);
                    for (len = 0; len < size; ++len, ++pHV) {

                    }
                    m_pHttpContext->PrepareResponse(NULL, 0, m_qSend, UHTTP::hrfpAll);
                    assert(m_qRecv.GetSize() == 0);
                    assert(m_pHttpContext->GetContentLength() == 0 || m_pHttpContext->GetContentLength() == CONTENT_LEN_UNKNOWN);
                } else {

                }
            } else {
                if (m_pHttpContext->GetContentLength() <= m_qRecv.GetSize()) {
                    m_pHttpContext->SetContent(m_qRecv.GetBuffer(), (unsigned int) m_pHttpContext->GetContentLength());
                    const UHttpRequest &ur = m_pHttpContext->ParseWebRequest();
                    if (ur.IsBatching()) {
                        m_pHttpContext->PrepareBatchResponses(ur, m_qSend, UHTTP::hrfpAll);
                    } else {
                        SPA::CScopeUQueue jBuffer;
                        m_pHttpContext->ProcessSpRequest(ur, *jBuffer);
                        m_pHttpContext->PrepareResponse(jBuffer->GetBuffer(), jBuffer->GetSize(), m_qSend);
                    }
                    m_qRecv.Pop((unsigned int) m_pHttpContext->GetContentLength());
                    m_qRecv.SetHeadPosition();
                    m_qRecv.SetNull();
                    CHttpContext::Unlock(m_pHttpContext);
                    m_pHttpContext = NULL;
                }
            }
        }
        m_qSend.SetNull();
    }

    void CAsyncSession::OnRead(const CErrorCode &ec, size_t bytes_transferred) {
        m_ec = ec;
        if (bytes_transferred == 0) {
            OnClose(ec);
            return;
        }
        if (m_qRecv.GetHeadPosition() > 1024)
            m_qRecv.SetHeadPosition();
        m_qRecv.Push(m_BufferRead, (unsigned int) bytes_transferred);
        m_qRecv.SetNull();
        const char *flash_policy = (const char*) m_qRecv.GetBuffer();
        bool bFlash = (strcmp(flash_policy, "<policy-file-request/>") == 0);
        if (bFlash) {

            const char *policy = "<?xml version=\"1.0\"?>" \
            "<!DOCTYPE cross-domain-policy SYSTEM \"http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\">" \
            "<cross-domain-policy>" \
            "<site-control permitted-cross-domain-policies=\"all\"/>" \
            "<allow-access-from domain=\"*\"/>" \
            "</cross-domain-policy>";

            //"<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>"

            m_qSend.Push(policy);
            m_qSend.SetNull();
        }

        if (IsSecure())
            m_ssl.async_read_some(boost::asio::buffer(m_BufferRead, ASYNC_BUFFER_SIZE), boost::bind(&CAsyncSession::OnRead, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
        else
            m_socket.async_read_some(boost::asio::buffer(m_BufferRead, ASYNC_BUFFER_SIZE), boost::bind(&CAsyncSession::OnRead, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));

        if (!bFlash)
            ProcessHttpRequest();
        SendData();
    }

    void CAsyncSession::InternalShutDown() {
        CErrorCode ErrorCode;
        bool secure = IsSecure();
        if (secure)
            m_ssl.shutdown(ErrorCode);
        m_socket.shutdown(nsAsio::socket_base::shutdown_both, ErrorCode);
        CHttpContext::Unlock(m_pHttpContext);
        m_pHttpContext = NULL;
    }

    void CAsyncSession::OnWriteCompleted(const CErrorCode &ec, size_t bytes_transferred) {
        m_BufferWrite = (unsigned char*) m_asWrite->GetBuffer();
        do {
            if (m_qSend.GetSize() >= ASYNC_BUFFER_SIZE)
                break;
            if (m_pHttpContext == NULL) {
                if (m_qRecv.GetSize())
                    ProcessHttpRequest();
                break;
            }
            if (m_pHttpContext->IsWebSocket()) {
                if (m_qRecv.GetSize())
                    ProcessHttpRequest();
                break;
            }
            m_pHttpContext->DownloadFile(m_qSend);
            if (m_pHttpContext->GetResponseProgress().Status == hrsCompleted) {
                CHttpContext::Unlock(m_pHttpContext);
                m_pHttpContext = NULL;
            }
        } while (false);
        SendData();
        //InternalShutDown();
    }

    bool CAsyncSession::IsSending() {
        return (m_BufferWrite == NULL);
    }

    void CAsyncSession::SendData() {
        if (IsSending())
            return;
        unsigned int available = ASYNC_BUFFER_SIZE;
        if (available > m_qSend.GetSize())
            available = m_qSend.GetSize();
        if (!available)
            return;
        unsigned int res = m_qSend.Pop(m_BufferWrite, available);
        if (IsSecure())
            m_ssl.async_write_some(boost::asio::buffer(m_BufferWrite, res), boost::bind(&CAsyncSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
        else
            m_socket.async_write_some(boost::asio::buffer(m_BufferWrite, res), boost::bind(&CAsyncSession::OnWriteCompleted, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
        m_BufferWrite = NULL;
    }

    void CAsyncSession::Start() {
        m_qRecv.SetSize(0);
        m_qSend.SetSize(0);
        if (IsSecure()) {
            m_ssl.async_handshake(boost::asio::ssl::stream_base::server, boost::bind(&CAsyncSession::OnSSLHandleShake, this, nsPlaceHolders::error));
        } else {
            m_socket.async_read_some(boost::asio::buffer(m_BufferRead, ASYNC_BUFFER_SIZE), boost::bind(&CAsyncSession::OnRead, this, nsPlaceHolders::error, nsPlaceHolders::bytes_transferred));
        }
    }

    bool CAsyncSession::IsSecure() {
        return (m_sslContext.native_handle()->default_passwd_callback != NULL);
    }

}
