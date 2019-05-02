#pragma once

#include "../../include/uclient.h"
#include "../shared/includes.h"


class CUCertImpl : public SPA::ClientSide::IUcert {
public:
    CUCertImpl(SPA::CMsSsl *pSsl);

public:
    const char* Verify(int *errCode) const;
    bool SetVerifyLocation(const char *caFile) const;

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

private:
    CUCertImpl(const CUCertImpl &ci);
    CUCertImpl& operator=(const CUCertImpl &ci);

private:
    SPA::CMsSsl *m_pSsl;
    boost::mutex m_mutex;
    std::string m_Issuer;
    std::string m_Subject;
    std::string m_NotBefore;
    std::string m_NotAfter;
    std::string m_CertPem;
    std::string m_SessionInfo;
    std::vector<unsigned char> m_PublicKey;
    std::vector<unsigned char> m_Algorithm;
    std::vector<unsigned char> m_SerialNumber;
};

