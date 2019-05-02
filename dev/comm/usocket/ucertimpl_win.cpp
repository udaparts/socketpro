#include "stdafx.h"
#include "ucertimpl_win.h"

CUCertImpl::CUCertImpl(SPA::CMsSsl *pSsl)
: m_pSsl(pSsl) {
    SigAlg = GetSigAlg();
    Validity = GetValidity();

    m_Issuer = GetIssuer();
    Issuer = m_Issuer.c_str();

    m_Subject = GetSubject();
    Subject = m_Subject.c_str();

    m_NotBefore = GetNotBefore();
    NotBefore = m_NotBefore.c_str();

    m_NotAfter = GetNotAfter();
    NotAfter = m_NotAfter.c_str();

    m_CertPem = GetCertPem();
    CertPem = m_CertPem.c_str();

    m_SessionInfo = GetSessionInfo();
    SessionInfo = m_SessionInfo.c_str();

    m_PublicKey = GetPublicKey();
    PublicKey = &m_PublicKey.front();
    PKSize = (unsigned int) m_PublicKey.size();

    m_Algorithm = GetAlgorithm();
    Algorithm = &m_Algorithm.front();
    AlgSize = (unsigned int) m_Algorithm.size();

    m_SerialNumber = GetSerialNumber();
    SerialNumber = &m_SerialNumber.front();
    SNSize = (unsigned int) m_SerialNumber.size();
}

std::string CUCertImpl::GetIssuer() const {
    
    return "";
}

std::string CUCertImpl::GetSubject() const {
    return "";
}

bool CUCertImpl::SetVerifyLocation(const char *caFile) const {
    return false;
}

const char* CUCertImpl::Verify(int *errCode) const {
    return "";
}

std::string CUCertImpl::GetCertPem() const {
    return "";
}

std::vector<unsigned char> CUCertImpl::GetPublicKey() const {
    std::vector<unsigned char> vPublic;
    
    return vPublic;
}

std::string CUCertImpl::GetNotAfter() const {
    char strNotAfter[256] = {0};
    return strNotAfter;
}

std::string CUCertImpl::GetNotBefore() const {
    char strNotBefore[256] = {0};
    return strNotBefore;
}

bool CUCertImpl::GetValidity() const {
    bool bOK = false;
    return bOK;
}

const char* CUCertImpl::GetSigAlg() const {
    return "";
}

std::vector<unsigned char> CUCertImpl::GetSerialNumber() const {
    std::vector<unsigned char> sn;
    return sn;
}

std::vector<unsigned char> CUCertImpl::GetAlgorithm() const {
    std::vector<unsigned char> alg;
    return alg;
}

std::string CUCertImpl::GetSessionInfo() const {
    return "";
}




