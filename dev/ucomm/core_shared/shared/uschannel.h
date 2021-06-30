#ifndef ___UDAPARTS_COMM_SCHANNEL_I_H__
#define ___UDAPARTS_COMM_SCHANNEL_I_H__

#include "../../include/membuffer.h"

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#ifdef _WIN32_WCE
#include <schnlsp.h>
#include <boost/shared_ptr.hpp>
#endif
#include <security.h>
#include <schannel.h>
#include <memory>
#include "storetype.h"

#include "../pinc/random_crash.h"
#ifdef BAD_COMM_ENVIRONMENT
#include <time.h>
#endif

namespace SPA {

    typedef SECURITY_STATUS(SEC_ENTRY *PCompleteAuthToken)(PCtxtHandle phContext, PSecBufferDesc pToken);

    class CScCert {
    public:
        CScCert();
        ~CScCert();

    public:
        bool IsOpened();
        bool OpenFromPem(const wchar_t *filePerm);
        bool OpenFromPfxFile(const wchar_t *filePfx, const wchar_t *password);
        void Close();
        HCERTSTORE GetCertStore();
        bool OpenStore(tagCertStoreType cst = cstRoot);
        bool OpenStore(bool currentUser, tagCertStoreType cst = cstRoot);
        bool OpenStore(const wchar_t *certFile);
        tagCertStoreType GetCertStoreType();

    public:
#if defined(_WIN32_WCE) && _WIN32_WCE < 0x690
        static PCompleteAuthToken CompleteAuthToken;
        static HMODULE m_hSchannel;
#endif
    private:
        static void SetCertGetCertificateChainFunc();

    private:
        CScCert(const CScCert &cert);
        CScCert& operator=(const CScCert &cert);
        HCERTSTORE m_hCertStore;
        SPA::CUCriticalSection m_cs;
        tagCertStoreType m_cst;
    };

    enum tagSslHandshakeState {
        hsStart = 0,
        hsShaking = 1,
        hsDone = 2
    };

    class CSChannel {
    public:
        CSChannel(bool client, PCredHandle pCredHandle, bool clientCertAuth);
        ~CSChannel();

    public:
        bool IsInitialized() const;
        bool DoHandshake(const unsigned char *buffer, DWORD dwSize, CUQueue &token);
        tagSslHandshakeState GetHandshakeState() const;
        SECURITY_STATUS GetLastStatus() const;
        bool Decrypt(const unsigned char *buffer, DWORD dwSize, CUQueue &read);
        SECURITY_STATUS Encrypt(const unsigned char *buffer, DWORD dwSize, CUQueue &write);
        PCCERT_CONTEXT GetCertContext() const;
        PSecHandle GetCtxHandle() const;
        void ResetCredHandle(PCredHandle pCredHandle);

    private:
        CSChannel(const CSChannel &sspi);
        CSChannel& operator=(const CSChannel &sspi);
        void DeleteContext();
        void DeleteCertContext();

    private:
        const static unsigned int INITIAL_CRYPTION_BUFFER_SIZE = 20 * 1024;
        CScopeUQueue m_sbIn;
        CScopeUQueue m_sbOut;
        bool m_client;
        PCredHandle m_pCredHandle;
        CtxtHandle m_CtxHandle;
        SecPkgContext_StreamSizes m_StreamSizes;
        tagSslHandshakeState m_hs;
        SECURITY_STATUS m_ss;
        TimeStamp m_tsExpiry;
        CUQueue &m_qIn;
        CUQueue &m_qOut;
        PCCERT_CONTEXT m_pCertContext;
        bool m_bClientReset;
        bool m_clientCertAuth;
    };
#ifndef WINCE
    typedef std::shared_ptr<CSChannel> CSChannelPtr;
#else
    typedef boost::shared_ptr<CSChannel> CSChannelPtr;
#endif
}

#endif