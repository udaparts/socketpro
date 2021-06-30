

#include "certificateimpl.h"
#include <boost/algorithm/string.hpp>
#include "../pinc/base64.h"

namespace SPA
{

    CScCert CCertificateImpl::CertStore;

    CCertificateImpl::CCertificateImpl(CSChannelPtr pSspi, const char *serverName)
            : m_pCertContext(pSspi->GetCertContext()),
            m_sc(pSspi->GetCtxHandle()),
            m_strSessionInfo("SSL version: ") {
        std::string s(serverName);
        m_strServerName.assign(s.begin(), s.end());
        SetSessionInfo();
        SetPerm();
        SetPublickey();
        SetPointers();
    }

    CCertificateImpl::CCertificateImpl(PCCERT_CONTEXT pCertContext, const char *serverName)
            : m_pCertContext(pCertContext),
            m_sc(nullptr),
            m_strSessionInfo("SSL version: ") {
        std::string s(serverName);
        m_strServerName.assign(s.begin(), s.end());
        SetSessionInfo();
        SetPerm();
        SetPublickey();
        SetPointers();
    }

    CCertificateImpl::~CCertificateImpl() {

    }

    void CCertificateImpl::SetPointers() {
#ifdef _WIN32_WCE
        PublicKey = &m_vPublicKey.front();
#else
        PublicKey = m_vPublicKey.data();
#endif
        PKSize = (unsigned int) m_vPublicKey.size();
        Subject = m_strSubject.c_str();
        Issuer = m_strIssuer.c_str();
        CertPem = m_strPerm.c_str();
        SessionInfo = m_strSessionInfo.c_str();
        SNSize = (unsigned int) m_pCertContext->pCertInfo->SerialNumber.cbData;
        SerialNumber = m_pCertContext->pCertInfo->SerialNumber.pbData;
        AlgSize = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData;
        Algorithm = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData;
        SigAlg = m_pCertContext->pCertInfo->SignatureAlgorithm.pszObjId;
        Validity = (::CertVerifyTimeValidity(nullptr, m_pCertContext->pCertInfo) == 0);
        char str[128] = {0};
        LPCSTR map[] = {nullptr, "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        SYSTEMTIME st;
        ::FileTimeToSystemTime(&m_pCertContext->pCertInfo->NotBefore, &st);
#ifdef _WIN32_WCE
        ::sprintf(str, "%s %2d %02d:%02d:%02d %d GMT", map[st.wMonth], st.wDay, st.wHour, st.wMinute, st.wSecond, st.wYear);
#else
        ::sprintf_s(str, sizeof (str), "%s %2d %02d:%02d:%02d %d GMT", map[st.wMonth], st.wDay, st.wHour, st.wMinute, st.wSecond, st.wYear);
#endif
        m_strNotBefore = str;
        ::memset(str, 0, sizeof (str));
        ::FileTimeToSystemTime(&m_pCertContext->pCertInfo->NotAfter, &st);
#ifdef _WIN32_WCE
        ::sprintf(str, "%s %2d %02d:%02d:%02d %d GMT", map[st.wMonth], st.wDay, st.wHour, st.wMinute, st.wSecond, st.wYear);
#else
        ::sprintf_s(str, sizeof (str), "%s %2d %02d:%02d:%02d %d GMT", map[st.wMonth], st.wDay, st.wHour, st.wMinute, st.wSecond, st.wYear);
#endif
        m_strNotAfter = str;
        NotAfter = m_strNotAfter.c_str();
        NotBefore = m_strNotBefore.c_str();
    }

    PCCERT_CONTEXT const CCertificateImpl::GetCertContext() const {
        return m_pCertContext;
    }

    bool CCertificateImpl::SetVerifyLocation(const char *caFile) {
        if (!caFile)
            return false;
        if (boost::iequals(caFile, "my")) {
            return SPA::CCertificateImpl::CertStore.OpenStore(SPA::cstMy);
        } else if (boost::iequals(caFile, "root")) {
            return SPA::CCertificateImpl::CertStore.OpenStore(SPA::cstRoot);
        } else if (boost::iequals(caFile, "my@currentuser")) {
            return SPA::CCertificateImpl::CertStore.OpenStore(true, SPA::cstMy);
        } else if (boost::iequals(caFile, "root@currentuser")) {
            return SPA::CCertificateImpl::CertStore.OpenStore(true, SPA::cstRoot);
        } else if (boost::iequals(caFile, "my@localmachine")) {
            return SPA::CCertificateImpl::CertStore.OpenStore(false, SPA::cstMy);
        } else if (boost::iequals(caFile, "root@localmachine")) {
            return SPA::CCertificateImpl::CertStore.OpenStore(false, SPA::cstRoot);
        }
        return false; //file store not supported in windows
        /*
            USES_CONVERSION;
            return SPA::CCertificateImpl::CertStore.OpenFromPem(A2CW(caFile));
         */
    }

    void CCertificateImpl::SetPublickey() {
        unsigned char *p = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData;
        unsigned int len = m_pCertContext->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData;
        m_vPublicKey.assign(p, p + len);
    }

    void CCertificateImpl::SetSessionInfo() {
        char szName[4096] = {0};
        PCCERT_CONTEXT p = m_pCertContext;
        ::CertNameToStrA(p->dwCertEncodingType, &p->pCertInfo->Issuer, CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG, szName, sizeof (szName));
        m_strIssuer = szName;
        ::CertNameToStrA(p->dwCertEncodingType, &p->pCertInfo->Subject, CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG, szName, sizeof (szName));
        m_strSubject = szName;
        if (!m_sc) {
            return;
        }
        SecPkgContext_ConnectionInfo ConnectionInfo;
        PSecHandle sc = m_sc;

        SECURITY_STATUS ss = ::QueryContextAttributes(sc, SECPKG_ATTR_CONNECTION_INFO, &ConnectionInfo);
        if (ss == SEC_E_OK) {
            switch (ConnectionInfo.dwProtocol) {
#ifndef _WIN32_WCE
                case SP_PROT_TLS1_2_CLIENT:
                case SP_PROT_TLS1_2_SERVER:
                    m_strSessionInfo += "TLS1.2";
                    break;
                case SP_PROT_TLS1_1_CLIENT:
                case SP_PROT_TLS1_1_SERVER:
                    m_strSessionInfo += "TLS1.1";
                    break;
#endif
                case SP_PROT_TLS1_CLIENT:
                case SP_PROT_TLS1_SERVER:
                    m_strSessionInfo += "TLS1.0";
                    break;
                case SP_PROT_SSL3_CLIENT:
                case SP_PROT_SSL3_SERVER:
                    m_strSessionInfo += "SSL3.0";
                    break;
                default:
                    break;
            }

#ifndef _WIN32_WCE
            SecPkgContext_CipherInfo ci;
            ::memset(&ci, 0, sizeof (ci));
            ss = ::QueryContextAttributes(sc, SECPKG_ATTR_CIPHER_INFO, &ci);
            if (ci.szCipherSuite) {
                m_strSessionInfo += ", ";
                m_strSessionInfo += SPA::Utilities::ToUTF8(ci.szCipherSuite);
            }
            m_strSessionInfo += (", Cipher strength: " + std::to_string((SPA::UINT64)ConnectionInfo.dwCipherStrength) + " bits");
#else
            char num[64] = {0};
            m_strSessionInfo += ", Cipher strength: ";
            ::sprintf(num, "%d", (int) ConnectionInfo.dwCipherStrength);
            m_strSessionInfo += num;
            m_strSessionInfo += " bits";
#endif
            m_strSessionInfo += ", Key size: ";
#ifndef _WIN32_WCE
            m_strSessionInfo += std::to_string((SPA::UINT64)ConnectionInfo.dwExchStrength);
#else
            ::sprintf(num, "%d", (int) ConnectionInfo.dwExchStrength);
            m_strSessionInfo += num;
#endif
            m_strSessionInfo += " bits";
        }
    }

    void CCertificateImpl::SetPerm() {
        if (m_pCertContext) {
            const char *start;
            const char *end;
            SPA::CScopeUQueue sb;
            if (m_pCertContext->cbCertEncoded * 2 > sb->GetMaxSize()) {
                sb->ReallocBuffer(m_pCertContext->cbCertEncoded * 2);
            }
            unsigned int res = CBase64::encode(m_pCertContext->pbCertEncoded, m_pCertContext->cbCertEncoded, (char*) sb->GetBuffer());
            sb->SetSize(res);
            sb->SetNull();
            m_strPerm += "-----BEGIN CERTIFICATE-----\n";
            unsigned int lines = (res / 64);
            unsigned int remain = (res % 64);
            const char *s = (const char*) sb->GetBuffer();
            for (unsigned int row = 0; row < lines; ++row) {
                start = s + row * 64;
                end = start + 64;
                std::string str(start, end);
                m_strPerm += str;
                m_strPerm += "\n";
            }
            if (remain) {
                start = s + lines * 64;
                end = start + remain;
                std::string str(start, end);
                m_strPerm += str;
                m_strPerm += "\n";
            }
            m_strPerm += "-----END CERTIFICATE-----\n";
        }
    }

    const char* CCertificateImpl::MapErrorMessage(DWORD errCode) {
        const char *errMsg = "";
        switch (errCode) {
            case CERT_TRUST_NO_ERROR:
                break;
            case CERT_TRUST_IS_NOT_TIME_VALID:
                errMsg = "This certificate or one of the certificates in the certificate chain is not time-valid";
                break;
            case CERT_TRUST_IS_REVOKED:
                errMsg = "Trust for this certificate or one of the certificates in the certificate chain has been revoked";
                break;
            case CERT_TRUST_IS_NOT_SIGNATURE_VALID:
                errMsg = "The certificate or one of the certificates in the certificate chain does not have a valid signature";
                break;
            case CERT_TRUST_IS_NOT_VALID_FOR_USAGE:
                errMsg = "The certificate or certificate chain is not valid in its proposed usage";
                break;
            case CERT_TRUST_IS_UNTRUSTED_ROOT:
                errMsg = "The certificate or certificate chain is based on an untrusted root";
                break;
            case CERT_TRUST_REVOCATION_STATUS_UNKNOWN:
                errMsg = "The revocation status of the certificate or one of the certificates in the certificate chain is unknown";
                break;
            case CERT_TRUST_IS_CYCLIC:
                errMsg = "One of the certificates in the chain was issued by a certification authority that the original certificate had certified";
                break;
            case CERT_TRUST_IS_PARTIAL_CHAIN:
                errMsg = "The certificate chain is not complete";
                break;
            case CERT_TRUST_CTL_IS_NOT_TIME_VALID:
                errMsg = "A CTL used to create this chain was not time-valid";
                break;
            case CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID:
                errMsg = "A CTL used to create this chain did not have a valid signature";
                break;
            case CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE:
                errMsg = "A CTL used to create this chain is not valid for this usage";
                break;
            default:
                break;
        }
        return errMsg;
    }

    const char* CCertificateImpl::VerifyOne(PCCERT_CONTEXT pCertContext, int *errCode, HCERTSTORE hCertStore) {
        const char *errMsg = "";
        DWORD dwFlags = 0;
        PCCERT_CHAIN_CONTEXT pChainContext = nullptr;
        CERT_CHAIN_PARA ChainPara;
        ::memset(&ChainPara, 0, sizeof (ChainPara));
        ChainPara.cbSize = sizeof (CERT_CHAIN_PARA);
        ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        BOOL ok = ::CertGetCertificateChain(
                nullptr, // use the default chain engine
                pCertContext, // pointer to the end certificate
                nullptr, // use the default time
                hCertStore, // ? hCertStore : pCertContext->hCertStore, // search no additional stores
                &ChainPara, // use AND logic and enhanced key usage 
                dwFlags,
                nullptr, // currently reserved
                &pChainContext); // return a pointer to the chain created
        if (!ok) {
            if (errCode)
                *errCode = (int) ::GetLastError();
            errMsg = "Can't create certificate chain";
        }

        if (errCode) {
            *errCode = (int) pChainContext->TrustStatus.dwErrorStatus;
        }

        if (pChainContext->TrustStatus.dwErrorStatus == CERT_TRUST_NO_ERROR) {
            if (pChainContext->rgpChain[0]->cElement == 1) {
                if (errCode) {
                    *errCode = CERT_TRUST_IS_UNTRUSTED_ROOT;
                }
                errMsg = "The certificate or certificate chain is based on an untrusted root";
                ::CertFreeCertificateChain(pChainContext);
                return errMsg;
            }
        }
        errMsg = MapErrorMessage(pChainContext->TrustStatus.dwErrorStatus);
        ::CertFreeCertificateChain(pChainContext);
        return errMsg;
    }

    const char* CCertificateImpl::Verify(int *errCode) const {
        return VerifyOne(m_pCertContext, errCode, CertStore.GetCertStore());
    }
} //namespace SPA
