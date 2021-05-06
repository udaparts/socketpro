#pragma once

#include "../../include/ucomm.h"
#include "includes.h"

class U_MODULE_HIDDEN CUCertImpl : public SPA::IUcert {
public:
    CUCertImpl(SSL *pSsl);
    CUCertImpl(X509 *cert);
    virtual ~CUCertImpl();

public:
    const char* Verify(int *errCode) const;
    static bool SetVerifyLocation(const char *caFile);

private:
    std::string GetIssuer() const;
    std::string GetSubject() const;
    std::string GetNotBefore() const;
    std::string GetNotAfter() const;
    const char* GetSigAlg() const;
    std::string GetCertPem() const;
    std::string GetSessionInfo() const;
    bool GetValidity() const;
    std::vector<unsigned char> GetPublicKey() const;
    std::vector<unsigned char> GetAlgorithm() const;
    std::vector<unsigned char> GetSerialNumber() const;
    static void asn1time_to_string(const ASN1_TIME *tm, char *buf);

private:
    CUCertImpl(const CUCertImpl &ci);
    CUCertImpl& operator=(const CUCertImpl &ci);
    void Set();
    void ReleaseCert();

private:
    SSL *m_pSsl;
    std::mutex m_mutex;
    std::string m_Issuer;
    std::string m_Subject;
    std::string m_NotBefore;
    std::string m_NotAfter;
    std::string m_CertPem;
    std::string m_SessionInfo;
    std::vector<unsigned char> m_PublicKey;
    std::vector<unsigned char> m_Algorithm;
    std::vector<unsigned char> m_SerialNumber;
    X509 *m_cert;
    static const char *m_empty;
};

#ifndef WINCE
typedef std::shared_ptr<CUCertImpl> CCertificateImplPtr;
#else
typedef boost::shared_ptr<CCertificateImpl> CCertificateImplPtr;
#endif
