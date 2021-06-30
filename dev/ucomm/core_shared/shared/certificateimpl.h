
#ifndef _UDAPARTS_COMM_CLIENT_CERTIFICATE_IMPL_H_
#define _UDAPARTS_COMM_CLIENT_CERTIFICATE_IMPL_H_

#include "../../include/ucomm.h"
#include "uschannel.h"

namespace SPA {

    class CCertificateImpl : public IUcert {
    public:
        CCertificateImpl(CSChannelPtr pSspi, const char *serverName);
        CCertificateImpl(PCCERT_CONTEXT pCertContext, const char *serverName);
        virtual ~CCertificateImpl();

    public:
        //implementation for interface IUcert
        virtual const char* Verify(int *errCode) const;
        virtual PCCERT_CONTEXT const GetCertContext() const;
        static bool SetVerifyLocation(const char *caFile);
        static const char* VerifyOne(PCCERT_CONTEXT pCertContext, int *errCode, HCERTSTORE hCertStore = nullptr);
        static const char* MapErrorMessage(DWORD errCode);

    private:
        //disable copy constructor and assignment operator
        CCertificateImpl(const CCertificateImpl &cert) = delete;
        CCertificateImpl& operator=(const CCertificateImpl &cert) = delete;

        void SetPerm();
        void SetSessionInfo();
        void SetPublickey();
        void SetPointers();

    public:
        static CScCert CertStore;

    private:
        PCCERT_CONTEXT m_pCertContext;
        PSecHandle m_sc;
        std::string m_strIssuer;
        std::string m_strSubject;
        std::string m_strPerm;
        std::string m_strSessionInfo;
        std::vector<unsigned char> m_vPublicKey;
        std::wstring m_strServerName;
        std::string m_strSigAlg;
        std::string m_strNotAfter;
        std::string m_strNotBefore;
    };
#ifndef WINCE
    typedef std::shared_ptr<CCertificateImpl> CCertificateImplPtr;
#else
    typedef boost::shared_ptr<CCertificateImpl> CCertificateImplPtr;
#endif
} //namespace SPA

#endif