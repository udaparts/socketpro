#include "pch.h"
#include "rawsession.h"
#include "rawthread.h"

namespace SPA
{

    CSpinLock CRawSession::m_csBuffer;
    std::vector<unsigned char*> CRawSession::m_aBuffer;

    unsigned char* CRawSession::GetIoBuffer() {
        unsigned char *s = nullptr;
        {
            CAutoLock sl(m_csBuffer);
            size_t size = m_aBuffer.size();
            if (size) {
                s = m_aBuffer[size - 1];
                m_aBuffer.pop_back();
            }
        }
        if (s == nullptr)
            s = (unsigned char*) ::malloc(IO_BUFFER_SIZE + IO_ENCRYPTION_PADDING); //add extra space for encryption padding
        return s;
    }

    void CRawSession::ReleaseIoBuffer(unsigned char *buffer) {
        if (buffer == nullptr)
            return;
        m_csBuffer.lock();
        m_aBuffer.push_back(buffer);
        m_csBuffer.unlock();
    }

    CRawSession::CRawSession(CIoService &IoService, CRawThread &rt, PDataArrive da) : m_qWrite(*m_sbWrite), m_qSsl(*m_sbSsl), m_io(IoService), m_rt(rt), m_da(da), m_socket(IoService),
            m_nPort(0), m_b6(false), m_bSync(false), m_ss(tagSessionState::ssClosed), m_secure(tagEncryptionMethod::NoEncryption), m_ReadBuffer(GetIoBuffer()),
            m_bRBLocked(false), m_WriteBuffer(GetIoBuffer()), m_bWBLocked(0), m_bWaiting(false) {
#ifdef WIN32_64
        ::memset(&m_hCreds, 0, sizeof (m_hCreds));
#else

#endif
        auto sc = rt.GetSessionCallback();
        if (sc) {
            sc(&rt, tagSessionPoolEvent::seSessionCreated, this);
        }
    }

    CRawSession::~CRawSession() {
        auto sc = m_rt.GetSessionCallback();
        if (sc) {
            //bool chatting = false;
            //CRAutoLock sl(m_mutex, chatting);
            sc(&m_rt, tagSessionPoolEvent::seSessionDestroyed, this);
        }
        ReleaseIoBuffer(m_WriteBuffer);
        ReleaseIoBuffer(m_ReadBuffer);
    }

    void CRawSession::PostProcessing(unsigned int hint, UINT64 data) {
        m_io.post(std::bind(&CRawSession::OnPostProcessing, this, hint, data));
    }

    bool CRawSession::IsSameThread() {
#ifdef WIN32_64
        auto id = ::GetCurrentThreadId();
#else
        auto id = ::pthread_self();
#endif
        return (id == m_rt.GetThreadId());
    }

    void CRawSession::OnPostProcessing(unsigned int hint, UINT64 data) {

        switch (hint) {
            case HINT_CLOSE:
                Close();
                break;
            case HINT_CONNECT:
            {
                tagSessionPoolEvent se = tagSessionPoolEvent::seConnected;
                PSessionCallback sc = m_rt.GetSessionCallback();
                CErrorCode ec;
                {
                    do {
                        CAutoLock sl(m_cs);
                        CResolver::query ipAddr(m_b6 ? nsIP::tcp::v6() : nsIP::tcp::v4(), m_strhost, std::to_string(m_nPort));
                        CResolver r(m_io);
                        CResolver::iterator iterator = r.resolve(ipAddr, ec);
                        if (ec || iterator == CResolver::iterator()) {
                            m_ec = ec;
                            m_ss = tagSessionState::ssClosed;
                            break;
                        }
                        m_socket.connect(iterator->endpoint(), ec);
                        if (ec) {
                            m_ec = ec;
                            m_ss = tagSessionState::ssClosed;
                            break;
                        }
                        boost::asio::socket_base::keep_alive option(false);
                        boost::asio::ip::tcp::no_delay nodelay(true);
                        m_socket.set_option(option, ec);
                        m_socket.set_option(nodelay, ec);
                        m_qWrite.SetSize(0);
                        m_bRBLocked = false;
                        m_bWBLocked = 0;
                        m_pCert.reset();
                        if (m_secure == tagEncryptionMethod::TLSv1) {
#ifdef WIN32_64
                            SECURITY_STATUS ss = OpenCred();
                            if (ss != SEC_E_OK) {
                                m_ec.assign(ss, boost::system::system_category());
                                m_ss = tagSessionState::ssClosed;
                                m_socket.shutdown(nsIP::tcp::socket::shutdown_type::shutdown_both, ec);
                                m_socket.close(ec);
                                break;
                            }
                            m_pSspi.reset(new CSspi(true, &m_hCreds, false));
                            if (!m_pSspi->DoHandshake(nullptr, 0, m_qWrite)) {
                                m_ec.assign(m_pSspi->GetLastStatus(), boost::system::system_category());
                                m_ss = tagSessionState::ssClosed;
                                m_socket.shutdown(nsIP::tcp::socket::shutdown_type::shutdown_both, ec);
                                m_socket.close(ec);
                                break;
                            }
#else
                            m_pSsl.reset(new CMyOpenSSL(m_rt.GetSslContext().native_handle(), true));
                            if (!m_pSsl->DoHandshake(nullptr, 0, m_qWrite)) {
                                m_ec.assign(SSL_ERROR_SSL, boost::asio::error::get_ssl_category());
                                m_ss = tagSessionState::ssClosed;
                                m_socket.shutdown(nsIP::tcp::socket::shutdown_type::shutdown_both, ec);
                                m_socket.close(ec);
                                break;
                            }
#endif
                            m_ss = tagSessionState::ssSslShaking;
                            se = tagSessionPoolEvent::seSslShaking;
                            SendInternal(nullptr, 0);
                            Read();
                            break;
                        }
                        m_ss = tagSessionState::ssConnected;
                        m_ec.clear();
                        Read();
                    } while (false);
                    std::unique_lock<std::mutex> sl(m_mutex);
                    if (m_bWaiting && se != tagSessionPoolEvent::seSslShaking) {
                        m_cv.notify_all();
                    }
                }
                if (sc) {
                    sc(&m_rt, se, this);
                }
            }
                break;
            default:
                assert(false);
                break;
        }
    }

    bool CRawSession::Shutdown(tagShutdownType st) {
        CAutoLock sl(m_cs);
        if (m_ss < tagSessionState::ssSslShaking) {
            m_ec.assign(boost::system::errc::not_connected, boost::system::generic_category());
            return false;
        }
        m_ec.clear();
        m_socket.shutdown((nsIP::tcp::socket::shutdown_type)st, m_ec);
        return (!m_ec.failed());
    }

    bool CRawSession::Connect(const char *strHost, unsigned int nPort, tagEncryptionMethod secure, bool b6, bool bSync, unsigned int timeout) {
        {
            CAutoLock sl(m_cs);
            if (m_ss > tagSessionState::ssClosed) {
                m_ec.assign(boost::system::errc::already_connected, boost::system::generic_category());
                return false;
            }
            m_strhost = strHost ? strHost : "";
            SPA::Trim(m_strhost);
            m_nPort = nPort;
            m_bSync = bSync;
            m_b6 = b6;
            m_secure = secure;
            PostProcessing(HINT_CONNECT, 0);
        }
        if (bSync) {
            if (IsSameThread()) {
                m_ec.assign(boost::system::errc::not_supported, boost::system::generic_category());
                return false;
            }
            if (m_ss >= tagSessionState::ssConnected)
                return true;
            std::unique_lock<std::mutex> sl(m_mutex);
            if (m_ss >= tagSessionState::ssConnected)
                return true;
            m_bWaiting = true;
            bool b = false;
            do {
                if (m_cv.wait_for(sl, ms(timeout)) == std::cv_status::timeout) {
                    m_ec.assign(boost::system::errc::timed_out, boost::system::generic_category());
                    break;
                }
                b = (m_ss >= tagSessionState::ssConnected);
            } while (false);
            m_bWaiting = false;
            return b;
        }
        return true;
    }

    bool CRawSession::IsConnected() {
        return (m_ss >= tagSessionState::ssConnected);
    }

    IUcert * CRawSession::GetUCert() {
        CAutoLock sl(m_cs);
#ifdef WIN32_64
        if (!m_pCert && m_pSspi && m_pSspi->GetHandshakeState() == tagSslHandshakeState::hsDone)
            m_pCert.reset(new CCertificateImpl(m_pSspi, ""));
#else
        if (!m_pCert && m_pSsl && m_pSsl->Done())
            m_pCert.reset(new CUCertImpl(m_pSsl->GetSSL()));
#endif
        return m_pCert.get();
    }

    bool CRawSession::GetServerName(char *name, unsigned int chars) {
        int res;
        if (!name || !chars) return true;
        name[0] = 0;
        --chars;
        if (!chars) return true;
        std::string host;
        boost::asio::ip::tcp::endpoint ep;
        do {
            CAutoLock sl(m_cs);
            m_ec.clear();
            ep = m_socket.remote_endpoint(m_ec);
            if (m_ec) {
                return false;
            }
            nsIP::address addr = ep.address();
            std::string saddr = addr.to_string();
            if (saddr != m_strhost) {
                host = m_strhost;
                break;
            }
            char hostname[NI_MAXHOST] = {0};
            if (addr.is_v4()) {
                struct sockaddr_in saGNI;
                ::memset(&saGNI, 0, sizeof (struct sockaddr));
                memset(&saGNI.sin_addr, 0, sizeof (struct in_addr));
                res = inet_pton(AF_INET, saddr.c_str(), &saGNI.sin_addr);
                saGNI.sin_family = AF_INET;
                if (res == 1) {
                    res = ::getnameinfo((struct sockaddr *) &saGNI, sizeof (struct sockaddr), hostname, sizeof (hostname), nullptr, 0, NI_NAMEREQD);
                }
            } else {
                nsIP::address_v6::bytes_type bytes = addr.to_v6().to_bytes();
                struct sockaddr_in6 saGNI;
                saGNI.sin6_family = AF_INET6;
#ifdef WIN32_64
                memcpy(saGNI.sin6_addr.u.Byte, bytes.data(), bytes.size());
#else
                memcpy(saGNI.sin6_addr.__in6_u.__u6_addr8, bytes.data(), bytes.size());
#endif
                res = ::getnameinfo((struct sockaddr *) &saGNI, sizeof (struct sockaddr_in6), hostname, sizeof (hostname), nullptr, 0, NI_NAMEREQD);
            }
            if (res == 0) {
                host = hostname;
            } else {
                host = m_strhost;
            }
        } while (false);
        unsigned int n = 0, len = (unsigned int) host.size();
        for (; n < len && n < chars; ++n) {
            name[n] = host[n];
        }
        name[n] = 0;
        return true;
    }

    bool CRawSession::GetPeerName(unsigned int *port, char *addr, unsigned int chars) {
        unsigned int n = 0;
        boost::asio::ip::tcp::endpoint ep;
        CAutoLock sl(m_cs);
        m_ec.clear();
        ep = m_socket.remote_endpoint(m_ec);
        if (m_ec) {
            return false;
        }
        if (port) {
            *port = ep.port();
        }
        if (!addr || !chars) {
            return true;
        }
        --chars;
        if (chars) {
            std::string str = ep.address().to_string();
            unsigned int max = (unsigned int) str.size();
            for (n = 0; n < max && n < chars; ++n) {
                addr[n] = str[n];
            }
        }
        addr[n] = 0;
        return true;
    }

    int CRawSession::GetErrorCode(char *em, unsigned int len) {
        CAutoLock sl(m_cs);
        int ec = m_ec.value();
        if (!ec) {
            if (em) {
                *em = 0;
            }
        } else if (em && len) {
            m_ec.message(em, len);
        }
        return ec;
    }

    void CRawSession::OnReadCompleted(const CErrorCode& ec, size_t nLen) {
        if (ec || !nLen) {
            m_cs.lock();
            m_ec = ec;
            m_cs.unlock();
            Close();
        } else {
            if (m_secure == tagEncryptionMethod::TLSv1) {
#ifdef WIN32_64
                if (m_pSspi->GetHandshakeState() == hsDone) {
                    if (!m_pSspi->Decrypt(m_ReadBuffer, (unsigned int) nLen, m_rt.m_buff)) {
                        m_cs.lock();
                        m_ec.assign(m_pSspi->GetLastStatus(), boost::system::system_category());
                        m_cs.unlock();
                        Close();
                        return;
                    }
                    if (m_rt.m_buff.GetSize()) {
                        m_da(&m_rt, this, m_rt.m_buff.GetBuffer(), m_rt.m_buff.GetSize());
                        m_rt.m_buff.SetSize(0);
                    }
                } else {
                    CScopeUQueue sb;
                    if (!m_pSspi->DoHandshake(m_ReadBuffer, (unsigned int) nLen, *sb)) {
                        m_cs.lock();
                        m_ec.assign(m_pSspi->GetLastStatus(), boost::system::system_category());
                        m_cs.unlock();
                        Close();
                        return;
                    }
                    if (sb->GetSize()) {
                        Send(sb->GetBuffer(), sb->GetSize());
                    }
                    if (m_pSspi->GetHandshakeState() == hsDone) {
                        g_mutexCvc.lock();
                        PCertificateVerifyCallback cvc = g_cvc;
                        g_mutexCvc.unlock();
                        m_cs.lock();
                        m_ec.clear();
                        m_ss = tagSessionState::ssConnected;
                        m_cs.unlock();
                        if (cvc) {
                            DWORD dwFlags = 0;
                            PCCERT_CHAIN_CONTEXT pChainContext = nullptr;
                            CERT_ENHKEY_USAGE EnhkeyUsage;
                            CERT_USAGE_MATCH CertUsage;
                            CERT_CHAIN_PARA ChainPara;
                            EnhkeyUsage.cUsageIdentifier = 0;
                            EnhkeyUsage.rgpszUsageIdentifier = nullptr;
                            CertUsage.dwType = USAGE_MATCH_TYPE_AND;
                            CertUsage.Usage = EnhkeyUsage;
                            ChainPara.cbSize = sizeof (CERT_CHAIN_PARA);
                            ChainPara.RequestedUsage = CertUsage;
                            PCCERT_CONTEXT pCertContext = m_pSspi->GetCertContext();
                            if (!::CertGetCertificateChain(nullptr, // use the default chain engine
                                    pCertContext, // pointer to the end certificate
                                    NULL, // use the default time
                                    CCertificateImpl::CertStore.GetCertStore(), // search no additional stores
                                    &ChainPara, // use AND logic and enhanced key usage 
                                    dwFlags,
                                    NULL, // currently reserved
                                    &pChainContext)) // return a pointer to the chain created
                            {
                                m_cs.lock();
                                m_ec.assign(::GetLastError(), boost::system::system_category());
                                m_cs.unlock();
                                Close();
                                return;
                            } else {
                                int errCode = 0;
                                const char *errMsg = CCertificateImpl::VerifyOne(pCertContext, &errCode, CCertificateImpl::CertStore.GetCertStore());
                                bool ok = (errCode == CERT_TRUST_NO_ERROR || ::strlen(errMsg) == 0);
                                PCERT_SIMPLE_CHAIN chains = pChainContext->rgpChain[0];
                                DWORD total = chains->cElement;
                                for (DWORD n = 0; n < total; ++n) {
                                    PCERT_CHAIN_ELEMENT one = chains->rgpElement[n];
                                    CCertificateImplPtr pCert(new CCertificateImpl(one->pCertContext, ""));
                                    ok = cvc(ok, n, one->TrustStatus.dwErrorStatus, CCertificateImpl::MapErrorMessage(one->TrustStatus.dwErrorStatus), pCert.get());
                                    if (!ok)
                                        break;
                                }
                                ::CertFreeCertificateChain(pChainContext);
                                if (!ok) {
                                    m_cs.lock();
                                    m_ec.assign(SEC_E_UNTRUSTED_ROOT, boost::system::system_category());
                                    m_cs.unlock();
                                    Close();
                                    return;
                                }
                            }
                        }
                        PSessionCallback sc = m_rt.GetSessionCallback();
                        if (sc) {
                            sc(&m_rt, tagSessionPoolEvent::seConnected, this);
                        }
                        std::unique_lock<std::mutex> sl(m_mutex);
                        if (m_bWaiting) {
                            m_cv.notify_all();
                        }
                    } else {
                        auto sc = m_rt.GetSessionCallback();
                        if (sc) {
                            sc(&m_rt, tagSessionPoolEvent::seSslShaking, this);
                        }
                    }
                }
#else
                if (m_pSsl->Done()) {
                    m_pSsl->Decrypt(m_ReadBuffer, nLen, m_rt.m_buff);
                    if (m_rt.m_buff.GetSize()) {
                        m_da(&m_rt, this, m_rt.m_buff.GetBuffer(), m_rt.m_buff.GetSize());
                        m_rt.m_buff.SetSize(0);
                    }
                } else {
                    CScopeUQueue sb;
                    if (!m_pSsl->DoHandshake(m_ReadBuffer, nLen, *sb)) {
                        m_cs.lock();
                        m_ec.assign(SSL_ERROR_SSL, boost::asio::error::get_ssl_category());
                        m_cs.unlock();
                        Close();
                        return;
                    }
                    if (sb->GetSize()) {
                        Send(sb->GetBuffer(), sb->GetSize());
                    }
                    if (m_pSsl->Done()) {
                        m_cs.lock();
                        m_ec.clear();
                        m_ss = tagSessionState::ssConnected;
                        m_cs.unlock();
                        PSessionCallback sc = m_rt.GetSessionCallback();
                        if (sc) {
                            sc(&m_rt, tagSessionPoolEvent::seConnected, this);
                        }
                        std::unique_lock<std::mutex> sl(m_mutex);
                        if (m_bWaiting) {
                            m_cv.notify_all();
                        }
                    } else {
                        auto sc = m_rt.GetSessionCallback();
                        if (sc) {
                            sc(&m_rt, tagSessionPoolEvent::seSslShaking, this);
                        }
                    }
                }
#endif
            } else {
                m_da(&m_rt, this, m_ReadBuffer, (unsigned int) nLen);
            }
            CAutoLock sl(m_cs);
            m_bRBLocked = false;
            Read();
        }
    }

    void CRawSession::Read() {
        if (m_bRBLocked && m_ss < tagSessionState::ssSslShaking)
            return;
        m_bRBLocked = true;
        m_socket.async_read_some(boost::asio::buffer(m_ReadBuffer, IO_BUFFER_SIZE), std::bind(&CRawSession::OnReadCompleted, this, std::placeholders::_1, std::placeholders::_2));
    }

    void CRawSession::OnWriteCompleted(const CErrorCode& ec, size_t bytes_transferred) {
        do {
            m_cs.lock();
            if (m_ss < tagSessionState::ssSslShaking) {
                m_cs.unlock();
                break;
            }
            if (ec) {
                m_ec = ec;
                m_cs.unlock();
                Close();
            } else {
                unsigned int bytes = m_qWrite.GetSize();
                if (m_ss >= tagSessionState::ssConnected && bytes > IO_BUFFER_SIZE && bytes <= (IO_BUFFER_SIZE >> 1)) {
                    std::unique_lock<std::mutex> sl(m_mutex);
                    if (m_bWaiting) {
                        m_cv.notify_all();
                    }
                }
                if (m_bWBLocked > bytes_transferred) {
                    //m_bWBLocked -= (unsigned int)bytes_transferred;
                    unsigned int ulLen = (unsigned int) (m_bWBLocked - bytes_transferred);
                    ::memmove(m_WriteBuffer, m_WriteBuffer + bytes_transferred, ulLen);
                    m_bWBLocked = ulLen;
                    m_socket.async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), std::bind(&CRawSession::OnWriteCompleted, this, std::placeholders::_1, std::placeholders::_2));
                } else {
                    assert(m_bWBLocked == bytes_transferred);
                    m_bWBLocked = 0;
                    if (bytes) {
                        SendInternal(nullptr, 0);
                    }
                }
                m_cs.unlock();
            }
        } while (false);
    }

    unsigned int CRawSession::GetOutBufferSize() {
        CAutoLock sl(m_cs);
        return m_qWrite.GetSize();
    }

    int CRawSession::SendInternal(const unsigned char *s, unsigned int nSize) {
        if (m_ss < tagSessionState::ssSslShaking) {
            if (!m_ec) {
                m_ec.assign(boost::system::errc::not_connected, boost::system::generic_category());
            }
            return m_ec.value();
        }
        if (m_bWBLocked) {
            m_qWrite.Push(s, nSize);
            return 0;
        }
        unsigned int ulLen = m_qWrite.GetSize();
        if (ulLen == 0 && s && nSize) {
            if (nSize <= IO_BUFFER_SIZE) {
                ::memcpy(m_WriteBuffer, s, nSize);
                ulLen = nSize;
            } else {
                ::memcpy(m_WriteBuffer, s, IO_BUFFER_SIZE);
                ulLen = IO_BUFFER_SIZE;

                //remaining
                m_qWrite.Push(s + IO_BUFFER_SIZE, nSize - IO_BUFFER_SIZE);
            }
        } else {
            m_qWrite.Push(s, nSize);
            ulLen = m_qWrite.GetSize();
            if (ulLen == 0) {
                return 0;
            }
            if (ulLen > IO_BUFFER_SIZE)
                ulLen = IO_BUFFER_SIZE;
            m_qWrite.Pop(m_WriteBuffer, ulLen);
        }
#ifdef WIN32_64
        if (m_secure == tagEncryptionMethod::TLSv1 && m_pSspi->GetHandshakeState() == tagSslHandshakeState::hsDone && m_ss >= tagSessionState::ssConnected) {
            m_qSsl.SetSize(0);
            m_pSspi->Encrypt(m_WriteBuffer, ulLen, m_qSsl);
            ulLen = m_qSsl.GetSize();
            assert(ulLen <= IO_BUFFER_SIZE + IO_ENCRYPTION_PADDING);
            ::memcpy(m_WriteBuffer, m_qSsl.GetBuffer(), ulLen);
        }
#else
        if (m_secure == tagEncryptionMethod::TLSv1 && m_pSsl->Done() && m_ss >= tagSessionState::ssConnected) {
            m_qSsl.SetSize(0);
            ulLen = m_pSsl->Encrypt(m_WriteBuffer, ulLen, m_qSsl);
            assert(ulLen);
            assert(ulLen == m_qSsl.GetSize());
            assert(ulLen <= IO_BUFFER_SIZE + IO_ENCRYPTION_PADDING);
            ::memcpy(m_WriteBuffer, m_qSsl.GetBuffer(), ulLen);
        }
#endif
        m_bWBLocked = ulLen;
        m_socket.async_write_some(boost::asio::buffer(m_WriteBuffer, ulLen), std::bind(&CRawSession::OnWriteCompleted, this, std::placeholders::_1, std::placeholders::_2));
        return 0;
    }

    int CRawSession::Send(const unsigned char *data, unsigned int bytes) {
        if (!IsSameThread() && GetOutBufferSize() >= LARGE_SENDING_BUFFER) {
            std::unique_lock<std::mutex> sl(m_mutex);
            m_bWaiting = true;
            while (m_cv.wait_for(sl, ms(10)) == std::cv_status::timeout && GetOutBufferSize() >= (LARGE_SENDING_BUFFER >> 4)) {

            }
            m_bWaiting = false;
        }
        CAutoLock sl(m_cs);
        return SendInternal(data, bytes);
    }

    void CRawSession::Close() {
        PSessionCallback sc = nullptr;
#ifdef WIN32_64
        auto id = ::GetCurrentThreadId();
#else
        auto id = ::pthread_self();
#endif
        tagSessionPoolEvent se = tagSessionPoolEvent::seSessionClosed;
        {
            m_cs.lock();
            do {
                if (m_ss == tagSessionState::ssClosed) {
                    break;
                }
                if (id == m_rt.GetThreadId()) {
                    CErrorCode ec;
                    m_socket.shutdown(nsIP::tcp::socket::shutdown_type::shutdown_both, ec);
                    m_socket.close(ec);
                    m_ss = tagSessionState::ssClosed;
                    sc = m_rt.GetSessionCallback();
                } else {
                    PostProcessing(HINT_CLOSE, 0);
                }
            } while (false);
            m_cs.unlock();
            std::unique_lock<std::mutex> sl(m_mutex);
            if (m_bWaiting) {
                m_cv.notify_all();
            }
        }
        if (sc) {
            sc(&m_rt, se, this);
        }
    }

#ifdef WIN32_64

    void CRawSession::FreeCredHandle() {
        if (m_hCreds.dwLower || m_hCreds.dwUpper) {
            ::FreeCredentialsHandle(&m_hCreds);
            ::memset(&m_hCreds, 0, sizeof (m_hCreds));
        }
    }

    SECURITY_STATUS CRawSession::OpenCred() {
        FreeCredHandle();
        SECURITY_STATUS Status = SEC_E_OK;
        SCHANNEL_CRED SchannelCred;
        ::memset(&SchannelCred, 0, sizeof (SchannelCred));

        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
        SchannelCred.grbitEnabledProtocols = SP_PROT_TLS1_X_CLIENT;
        SchannelCred.dwFlags = (SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_REVOCATION_CHECK_CHAIN/* | SCH_SEND_ROOT_CERT*/);
        SECURITY_STATUS ss = ::AcquireCredentialsHandle(nullptr,
                (LPWSTR) UNISP_NAME,
                SECPKG_CRED_OUTBOUND,
                nullptr,
                &SchannelCred,
                nullptr,
                nullptr,
                &m_hCreds,
                nullptr);
        return ss;
    }
#else

#endif

}; //namespace SPA;
