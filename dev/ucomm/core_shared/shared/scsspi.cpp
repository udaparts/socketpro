
#include "scsspi.h"
#include "../pinc/base64.h"
#include <boost/algorithm/string.hpp>
#include <fstream>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Secur32.lib")

//Thank you for the hint about "schannel.dll". CompleteAuthToken() is actually exported by that library, and I could load it dynamically via GetProcAddress().
//Note that this whole problem is resolved in Windows CE 7.0.

namespace SPA
{

#if defined(_WIN32_WCE) && _WIN32_WCE < 0x690
    PCompleteAuthToken CScCert::CompleteAuthToken = nullptr;
    HMODULE CScCert::m_hSchannel = nullptr;
#endif

    void CScCert::SetCertGetCertificateChainFunc() {
#if defined(_WIN32_WCE) && _WIN32_WCE < 0x690
        if (!CompleteAuthToken) {
            m_hSchannel = ::LoadLibrary(L"schannel.dll");
            CompleteAuthToken = (PCompleteAuthToken)::GetProcAddress(m_hSchannel, L"CompleteAuthToken");

            //::FreeLibrary(m_hSchannel); //not freed at the moment as we make sure it is loaded into process
        }
#endif
    }

    CScCert::CScCert()
            : m_hCertStore(nullptr),
            m_cst(cstUnknown) {
        CScCert::SetCertGetCertificateChainFunc();
    }

    CScCert::~CScCert() {
        Close();
    }

    bool CScCert::IsOpened() {
        CAutoLock al(m_cs);
        return m_hCertStore != nullptr;
    }

    void CScCert::Close() {
        CAutoLock al(m_cs);
        if (m_hCertStore) {
            ::CertCloseStore(m_hCertStore, 0);
            m_hCertStore = nullptr;
        }
        m_cst = cstUnknown;
    }

    tagCertStoreType CScCert::GetCertStoreType() {
        CAutoLock al(m_cs);
        return m_cst;
    }

    bool CScCert::OpenFromPem(const wchar_t * filePerm) {
        BOOL ok;
        USES_CONVERSION;
        CAutoLock al(m_cs);
        if (m_hCertStore) {
            return true;
        }
        /*
        HCERTCHAINENGINE hChainEngine = nullptr;
        CERT_CHAIN_ENGINE_CONFIG ChainConfig;
        ::memset(&ChainConfig, 0, sizeof(ChainConfig));
        ChainConfig.cbSize = sizeof(CERT_CHAIN_ENGINE_CONFIG);
        ChainConfig.dwFlags = CERT_CHAIN_CACHE_END_CERT;
        ok = CertCreateCertificateChainEngine(&ChainConfig, &hChainEngine);
        if (!ok)
                return false;
         */
        m_hCertStore = ::CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, nullptr);
        if (!m_hCertStore)
            return false;
        bool append = false;
        std::ifstream infile(W2CA(filePerm));
        std::string line;
        std::string pem;
        while (std::getline(infile, line)) {
            if (line == "-----BEGIN CERTIFICATE-----") {
                append = true;
                pem = "";
            } else if (line == "-----END CERTIFICATE-----") {
                SPA::CScopeUQueue sb;
                if (sb->GetMaxSize() < pem.size()) {
                    sb->ReallocBuffer((unsigned int) pem.size());
                }
                unsigned int res = CBase64::decode(pem.c_str(), (unsigned int) pem.size(), (unsigned char *) sb->GetBuffer());
                sb->SetSize(res);
                PCCERT_CONTEXT context = ::CertCreateCertificateContext((PKCS_7_ASN_ENCODING | X509_ASN_ENCODING), sb->GetBuffer(), sb->GetSize());
                if (context) {
                    ok = ::CertAddCertificateContextToStore(m_hCertStore, context, CERT_STORE_ADD_REPLACE_EXISTING, nullptr);
                    ::CertFreeCertificateContext(context);
                    if (!ok) {
                        ok = ::CertCloseStore(m_hCertStore, 0);
                        //::CertFreeCertificateChainEngine(hChainEngine);
                        m_hCertStore = nullptr;
                        return false;
                    }
                }
                append = false;
                pem = "";
            } else if (append) {
                pem += line;
            }
        }
        /*
        PCCERT_CHAIN_CONTEXT pChainContext = nullptr;
		CERT_CHAIN_PARA ChainPara;
        ::memset(&ChainPara, 0, sizeof(ChainPara));
		ChainPara.cbSize = sizeof (CERT_CHAIN_PARA);
		ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

        PCCERT_CONTEXT context = nullptr;
        while ((context = ::CertEnumCertificatesInStore(m_hCertStore, context))) {
                ok = CertGetCertificateChain(hChainEngine, context, nullptr, nullptr, &ChainPara, 0, nullptr, &pChainContext);
                if (!ok) {
                        ok = ::CertCloseStore(m_hCertStore, 0);
                        ::CertFreeCertificateChainEngine(hChainEngine);
                        m_hCertStore = nullptr;
                        return false;
                }
                ::CertFreeCertificateChain(pChainContext);
        }
        ::CertFreeCertificateChainEngine(hChainEngine);
        */
        m_cst = cstCertFile;
        return true;
    }

    bool CScCert::OpenFromPfxFile(const wchar_t *filePfx, const wchar_t * password) {
        ULONG ulRead;
        CAutoLock al(m_cs);
        if (m_hCertStore) {
            return true;
        }
#ifdef _WIN32_WCE

#else
        if (!password) {
            password = L"";
        }
        HANDLE hfile = ::CreateFile(filePfx, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if (INVALID_HANDLE_VALUE == hfile) {
            return false;
        }
        CRYPT_DATA_BLOB blob;
        blob.cbData = GetFileSize(hfile, 0);
        blob.pbData = (BYTE*)::malloc(blob.cbData + 16);
        do {
            if (::ReadFile(hfile, blob.pbData, blob.cbData, &ulRead, nullptr) == FALSE) {
                break;
            }
            if (::PFXIsPFXBlob(&blob) == FALSE) {
                break;
            }
            m_hCertStore = ::PFXImportCertStore(&blob, password, CRYPT_USER_KEYSET | CRYPT_EXPORTABLE);
            if (m_hCertStore) {
                m_cst = cstPfx;
            }
        } while (false);
        if (blob.pbData) {
            ::free(blob.pbData);
        }
        ::CloseHandle(hfile);
#endif
        return (m_hCertStore != nullptr);
    }

    bool CScCert::OpenStore(bool currentUser, tagCertStoreType ct) {
        CAutoLock al(m_cs);
        if (m_hCertStore) {
            return true;
        }
        m_cst = ct;
        switch (ct) {
            case SPA::cstRoot:
                m_hCertStore = ::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, currentUser ? CERT_SYSTEM_STORE_CURRENT_USER : CERT_SYSTEM_STORE_LOCAL_MACHINE, L"ROOT");
                break;
            case SPA::cstMy:
                m_hCertStore = ::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, currentUser ? CERT_SYSTEM_STORE_CURRENT_USER : CERT_SYSTEM_STORE_LOCAL_MACHINE, L"MY");
                break;
            default:
                assert(false);
                break;
        }
        return (m_hCertStore != nullptr);
    }

    bool CScCert::OpenStore(tagCertStoreType ct) {
        CAutoLock al(m_cs);
        if (m_hCertStore) {
            return true;
        }
        m_cst = ct;
        switch (ct) {
            case SPA::cstRoot:
                m_hCertStore = CertOpenSystemStore(0, L"ROOT");
                break;
            case SPA::cstCa:
                m_hCertStore = CertOpenSystemStore(0, L"CA");
                break;
            case SPA::cstMy:
                m_hCertStore = CertOpenSystemStore(0, L"MY");
                break;
            case SPA::cstSpc:
                m_hCertStore = CertOpenSystemStore(0, L"SPC");
                break;
            default:
                assert(false);
                break;
        }
        return (m_hCertStore != nullptr);
    }

    bool CScCert::OpenStore(const wchar_t * certFile) {
        CAutoLock al(m_cs);
        if (m_hCertStore) {
            return true;
        }
        DWORD dwStoreOpenFlag = (CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG);
        m_hCertStore = CertOpenStore(CERT_STORE_PROV_FILENAME_W,
                0,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                dwStoreOpenFlag,
                certFile);
        if (m_hCertStore) {
            m_cst = cstCertFile;
        }
        return (m_hCertStore != nullptr);
    }

    HCERTSTORE CScCert::GetCertStore() {
        CAutoLock al(m_cs);
        return m_hCertStore;
    }

    CSspi::CSspi(bool client, PCredHandle pCredHandle, bool clientCertAuth)
            : m_sbIn(MY_OPERATION_SYSTEM, IsBigEndian(), INITIAL_CRYPTION_BUFFER_SIZE),
            m_sbOut(MY_OPERATION_SYSTEM, IsBigEndian(), INITIAL_CRYPTION_BUFFER_SIZE),
            m_client(client),
            m_pCredHandle(pCredHandle),
            m_hs(hsStart),
            m_ss(SEC_E_OK),
            m_qIn(*m_sbIn),
            m_qOut(*m_sbOut),
            m_pCertContext(nullptr),
            m_bClientReset(false),
            m_clientCertAuth(clientCertAuth) {
        assert(pCredHandle);
        ::memset(&m_CtxHandle, 0, sizeof (m_CtxHandle));
        ::memset(&m_StreamSizes, 0, sizeof (m_StreamSizes));
        ::memset(&m_tsExpiry, 0, sizeof (m_tsExpiry));
        if (m_qIn.GetMaxSize() > DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
            m_qIn.ReallocBuffer(DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
        }
    }

    CSspi::~CSspi() {
        DeleteCertContext();
        DeleteContext();
    }

    void CSspi::ResetCredHandle(PCredHandle pCredHandle) {
        assert(m_client);
        m_bClientReset = true;
        m_pCredHandle = pCredHandle;
    }

    tagSslHandshakeState CSspi::GetHandshakeState() const {
        return m_hs;
    }

    bool CSspi::IsInitialized() const {
        return ((m_CtxHandle.dwLower || m_CtxHandle.dwUpper) && m_ss != SEC_E_INCOMPLETE_MESSAGE && m_ss != SEC_I_COMPLETE_AND_CONTINUE && m_ss != SEC_I_CONTINUE_NEEDED);
    }

    PCCERT_CONTEXT CSspi::GetCertContext() const {
        return m_pCertContext;
    }

    PSecHandle CSspi::GetCtxHandle() const {
        return (PSecHandle)&m_CtxHandle;
    }

    void CSspi::DeleteCertContext() {
        if (m_pCertContext) {
            ::CertFreeCertificateContext(m_pCertContext);
            m_pCertContext = nullptr;
        }
    }

    void CSspi::DeleteContext() {
        if (m_CtxHandle.dwLower || m_CtxHandle.dwUpper) {
            SECURITY_STATUS status = ::DeleteSecurityContext(&m_CtxHandle);
            ::memset(&m_CtxHandle, 0, sizeof (m_CtxHandle));
        }
    }

    SECURITY_STATUS CSspi::GetLastStatus() const {
        return m_ss;
    }

    SECURITY_STATUS CSspi::Encrypt(const unsigned char *buffer, DWORD dwSize, CUQueue & write) {
        SECURITY_STATUS ss;
        assert(m_hs == hsDone);
        SecBufferDesc Message;
        SecBuffer Buffers[4];
        if (!buffer) {
            dwSize = 0;
        }
        do {
            unsigned int size = (dwSize > m_StreamSizes.cbMaximumMessage) ? m_StreamSizes.cbMaximumMessage : dwSize;
            unsigned int requiredSize = (unsigned int) (m_StreamSizes.cbHeader + m_StreamSizes.cbTrailer + size);
            if (requiredSize > m_qOut.GetMaxSize()) {
                m_qOut.ReallocBuffer(requiredSize);
            }
            Buffers[0].pvBuffer = (void*) m_qOut.GetBuffer();
            Buffers[0].cbBuffer = m_StreamSizes.cbHeader;
            Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
            ::memset(Buffers[0].pvBuffer, 0, m_StreamSizes.cbHeader);
            m_qOut.SetSize((unsigned int) m_StreamSizes.cbHeader);
            m_qOut.Push(buffer, size);
            Buffers[1].pvBuffer = (void*) m_qOut.GetBuffer(m_StreamSizes.cbHeader);
            Buffers[1].cbBuffer = m_qOut.GetSize() - (unsigned int) m_StreamSizes.cbHeader;
            Buffers[1].BufferType = SECBUFFER_DATA;
            Buffers[2].pvBuffer = (void*) m_qOut.GetBuffer(m_qOut.GetSize());
            Buffers[2].cbBuffer = m_StreamSizes.cbTrailer;
            Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
            ::memset(Buffers[2].pvBuffer, 0, m_StreamSizes.cbTrailer);
            Buffers[3].BufferType = SECBUFFER_EMPTY;
            Buffers[3].cbBuffer = 0;
            Buffers[3].pvBuffer = nullptr;
            m_qOut.SetSize(0);
            Message.ulVersion = SECBUFFER_VERSION;
            Message.cBuffers = sizeof (Buffers) / sizeof (SecBuffer);
            Message.pBuffers = Buffers;
            ss = ::EncryptMessage(&m_CtxHandle, 0, &Message, 0);
            if (ss != SEC_E_OK) {
                return ss;
            }
            assert(Buffers[0].cbBuffer == m_StreamSizes.cbHeader);
            assert(Buffers[1].cbBuffer == size);
            unsigned int obtained = (unsigned int) (Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer);
            write.Push(m_qOut.GetBuffer(), obtained);
            dwSize -= size;
            buffer += size;
        } while (dwSize > 0);
        return ss;
    }

    bool CSspi::Decrypt(const unsigned char *buffer, DWORD dwSize, CUQueue & read) {
        int i;
        bool keep;
        bool backup = false;
        assert(m_hs == hsDone);
        SecBuffer Buffers[4];
        SecBufferDesc Message;
        if (!buffer) {
            dwSize = 0;
        }
        m_qIn.SetHeadPosition();
        do {
            keep = false;
            SecBuffer *pDataBuffer = nullptr;
            SecBuffer *pExtraBuffer = nullptr;
            ::memset(Buffers, 0, sizeof (Buffers));
            // Attempt to decrypt the received data.
            if (m_qIn.GetSize()) {
                if (buffer && dwSize) {
                    m_qIn.Push((const unsigned char*) buffer, (unsigned int) dwSize);
                }
                Buffers[0].pvBuffer = (void*) m_qIn.GetBuffer();
                Buffers[0].cbBuffer = m_qIn.GetSize();
            } else {
                Buffers[0].pvBuffer = (void*) buffer;
                Buffers[0].cbBuffer = dwSize;
                backup = true;
            }
            Buffers[0].BufferType = SECBUFFER_DATA;
            /*
            Buffers[1].BufferType   = SECBUFFER_EMPTY;
            Buffers[2].BufferType   = SECBUFFER_EMPTY;
            Buffers[3].BufferType   = SECBUFFER_EMPTY;
             */
            Message.ulVersion = SECBUFFER_VERSION;
            Message.cBuffers = sizeof (Buffers) / sizeof (SecBuffer);
            Message.pBuffers = Buffers;

            m_ss = ::DecryptMessage(&m_CtxHandle, &Message, 0, nullptr);
            switch (m_ss) {
                case SEC_E_OK:
                    m_qIn.SetSize(0);
                    for (i = 0; i<sizeof (Buffers) / sizeof (SecBuffer); ++i) {
                        if (!pDataBuffer && Buffers[i].BufferType == SECBUFFER_DATA) {
                            pDataBuffer = Buffers + i;
                        } else if (!pExtraBuffer && Buffers[i].BufferType == SECBUFFER_EXTRA) {
                            pExtraBuffer = Buffers + i;
                        }
                    }
                    if (pDataBuffer) {
                        read.Push((const unsigned char*) pDataBuffer->pvBuffer, (unsigned int) pDataBuffer->cbBuffer);
                    }
                    if (pExtraBuffer) {
                        m_qIn.Push((const unsigned char*) pExtraBuffer->pvBuffer, (unsigned int) pExtraBuffer->cbBuffer);
                    }
                    if (m_qIn.GetSize() > (unsigned int) m_StreamSizes.cbHeader) {
                        keep = true;
                        buffer = nullptr;
                        dwSize = 0;
                    }
                    break;
                case SEC_E_INCOMPLETE_MESSAGE:
                    if (backup) {
                        m_qIn.Push(buffer, dwSize);
                    }
                    break;
                default:
                    return false;
            }
        } while (keep);
        return true;
    }

    bool CSspi::DoHandshake(const unsigned char *buffer, DWORD dwSize, CUQueue & token) {
        //assert(!IsInitialized());
        SecBufferDesc InBuffer;
        SecBuffer InBuffers[2];
        SecBufferDesc OutBuffer;
        SecBuffer OutBuffers[1];
        DWORD dwSSPIOutFlags = 0;
        SECURITY_STATUS ss = SEC_E_OK;

        DWORD dwSSPIFlags;
        if (m_client) {
            dwSSPIFlags =
                    ISC_REQ_SEQUENCE_DETECT |
                    ISC_REQ_MANUAL_CRED_VALIDATION |
                    ISC_REQ_EXTENDED_ERROR |
                    ISC_REQ_ALLOCATE_MEMORY |
                    ISC_REQ_REPLAY_DETECT |
                    ISC_REQ_CONFIDENTIALITY |
                    ISC_REQ_STREAM;
            if (m_bClientReset) {
                dwSSPIFlags |= ISC_REQ_USE_SUPPLIED_CREDS;
            }
        } else {
            dwSSPIFlags =
                    ASC_REQ_SEQUENCE_DETECT |
                    ASC_REQ_REPLAY_DETECT |
                    ASC_REQ_CONFIDENTIALITY |
                    ASC_REQ_EXTENDED_ERROR |
                    ASC_REQ_ALLOCATE_MEMORY |
                    ASC_REQ_STREAM;
            if (m_clientCertAuth) {
                dwSSPIFlags |= ASC_REQ_MUTUAL_AUTH;
            }
        }

        // Set up the input buffers. Buffer 0 is used to pass in data
        // received from the server. Schannel will consume some or all
        // of this. Leftover data (if any) will be placed in buffer 1 and
        // given a buffer type of SECBUFFER_EXTRA.
        InBuffers[0].pvBuffer = (void*) buffer;
        InBuffers[0].cbBuffer = dwSize;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer = nullptr;
        InBuffers[1].cbBuffer = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers = 2;
        InBuffer.pBuffers = InBuffers;
        InBuffer.ulVersion = SECBUFFER_VERSION;

        // Set up the output buffers. These are initialized to NULL
        // so as to make it less likely we'll attempt to free random
        // garbage later.
        OutBuffers[0].pvBuffer = nullptr;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;

        OutBuffer.cBuffers = 1;
        OutBuffer.pBuffers = OutBuffers;
        OutBuffer.ulVersion = SECBUFFER_VERSION;

        bool bInit = (m_CtxHandle.dwLower == 0 && m_CtxHandle.dwUpper == 0);
        if (m_client) {
            m_ss = ::InitializeSecurityContext(m_pCredHandle,
                    (bInit ? nullptr : &m_CtxHandle),
                    nullptr,
                    dwSSPIFlags,
                    0,
                    0,
                    &InBuffer,
                    0,
                    (bInit ? &m_CtxHandle : nullptr),
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &m_tsExpiry);
        } else {
            m_ss = ::AcceptSecurityContext(m_pCredHandle,
                    (bInit ? nullptr : &m_CtxHandle),
                    &InBuffer,
                    dwSSPIFlags,
                    SECURITY_NATIVE_DREP,
                    (bInit ? &m_CtxHandle : nullptr),
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &m_tsExpiry);
        }

        if (!bInit && !m_pCertContext && SUCCEEDED(m_ss)) {
            ss = ::QueryContextAttributes(&m_CtxHandle, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &m_pCertContext);
        }
        switch (m_ss) {
            case SEC_E_OK:
                ss = ::QueryContextAttributes(&m_CtxHandle, SECPKG_ATTR_STREAM_SIZES, &m_StreamSizes);
                if (OutBuffers[0].cbBuffer && OutBuffers[0].pvBuffer) {
                    token.Push((const unsigned char*) (OutBuffers[0].pvBuffer), (unsigned int) (OutBuffers[0].cbBuffer));
                    ss = ::FreeContextBuffer(OutBuffers[0].pvBuffer);
                }
                m_hs = hsDone;
                break;
            case SEC_I_COMPLETE_AND_CONTINUE:
            case SEC_I_COMPLETE_NEEDED:
                m_hs = hsShaking;
#if defined(_WIN32_WCE) && _WIN32_WCE < 0x690
                m_ss = CScCert::CompleteAuthToken(&m_CtxHandle, &OutBuffer);
#else
                m_ss = ::CompleteAuthToken(&m_CtxHandle, &OutBuffer);
#endif
                if (OutBuffers[0].cbBuffer && OutBuffers[0].pvBuffer) {
                    token.Push((const unsigned char*) (OutBuffers[0].pvBuffer), (unsigned int) (OutBuffers[0].cbBuffer));
                    ss = ::FreeContextBuffer(OutBuffers[0].pvBuffer);
                }
                if (!SUCCEEDED(m_ss)) {
                    return false;
                }
                break;
            case SEC_I_CONTINUE_NEEDED:
                if (OutBuffers[0].cbBuffer && OutBuffers[0].pvBuffer) {
                    token.Push((const unsigned char*) (OutBuffers[0].pvBuffer), (unsigned int) (OutBuffers[0].cbBuffer));
                    ss = ::FreeContextBuffer(OutBuffers[0].pvBuffer);
                }
                m_hs = hsShaking;
                break;
            case SEC_E_INCOMPLETE_MESSAGE:
                m_hs = hsShaking;
                break;
            case SEC_I_INCOMPLETE_CREDENTIALS:
                m_hs = hsShaking;
                break;
            default:
                DeleteContext();
                return false;
        }
        return true;
    }
}
